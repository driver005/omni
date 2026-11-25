export module blog_handlers:automated_emails_automated_emails_senders_put_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import :outbox_shim;
import interfaces;

export namespace blog::handlers::automated_emails::automated_emails_senders_put::workflow {

namespace local_mapping {
    [[nodiscard]] inline std::string
    wrap(std::string_view resource, std::string_view rows, std::string_view meta = {})
    {
        return "{\"" + std::string{resource} + "\":" + std::string{rows} +
               (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
    }

    [[nodiscard]] inline std::string clean(std::string_view json, std::string_view)
    {
        return std::string{json};
    }

    [[nodiscard]] inline std::string pagination(
        std::size_t total,
        std::size_t limit,
        std::size_t page,
        std::size_t pages,
        std::string_view prev,
        std::string_view next
    )
    {
        return std::format(
            R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
            total,
            limit,
            page,
            pages,
            prev.empty() ? std::string{"null"} : std::string{prev},
            next.empty() ? std::string{"null"} : std::string{next}
        );
    }
} // namespace local_mapping

/** Outbox adapter standing in for the shared transactional mailer used by Ghost's
 *  #sendEmailVerificationMagicLink. Records the outbound verification message in the
 *  context-local `emails` resource and keeps an in-process log for verification. */
namespace mail_side_effect {
    using Event = blog::handlers::outbox::Event;

    inline std::vector<Event>& log()
    {
        static std::vector<Event> v;
        return v;
    }

    inline blog::BlogContext*& ctx_ptr()
    {
        static blog::BlogContext* p{nullptr};
        return p;
    }

    class Adapter final : public blog::handlers::outbox::Adapter
    {
    public:
        void dispatch(const Event& event) override
        {
            if (event.type != "email.send_verification") {
                return;
            }
            log().push_back(event);
            if (ctx_ptr() != nullptr) {
                ctx_ptr()->store().resources().create("emails", event.payload);
            }
        }
    };

    inline void ensure()
    {
        static const bool registered = []
        {
            blog::handlers::outbox::register_adapter(std::make_shared<Adapter>());
            return true;
        }();
        static_cast<void>(registered);
    }
} // namespace mail_side_effect

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"automated_emails_automated_emails_senders_put_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        static constexpr std::string_view kFields[]{
            "sender_name",
            "sender_email",
            "sender_reply_to"
        };
        std::string raw{in.body_as_string()};
        if (raw.empty()) {
            raw = "{}";
        }
        Object sender;
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(raw);
        if (parsed) {
            if (const auto* root = std::get_if<Object>(&parsed->get())) {
                const auto* source = root;
                const auto* data = value_for(root, "data");
                if (data != nullptr) {
                    if (const auto* dobj = std::get_if<Object>(&data->get())) {
                        source = dobj;
                    }
                }
                for (const auto key: kFields) {
                    const auto* value = value_for(source, key);
                    if (value == nullptr) {
                        continue;
                    }
                    if (!std::holds_alternative<std::string>(value->get())) {
                        const std::string message = std::string{key} == "sender_name"
                                                        ? "Sender name must be a string"
                                                        : (std::string{key} == "sender_email"
                                                               ? "Sender email must be a string"
                                                               : "Reply-to email must be a string");
                        return core::workflow::StepResponse<Output>{
                            {{"status", "422"},
                             {"body", blog::api::ApiError{}.validation(message, "", key)}}
                        };
                    }
                    sender[std::string{key}] = *value;
                }
            }
        }
        // Normalize: trim; empty string -> null (Ghost #normalizeSharedSenderValue).
        for (auto& [key, value]: sender) {
            if (const auto* text = std::get_if<std::string>(&value.get())) {
                std::string trimmed = trim(*text);
                value = trimmed.empty() ? serde::Value{} : serde::Value{trimmed};
            }
        }

        const auto defaults = ctx.store().json_for(
            "email_design_settings",
            {{"filter", "slug:default-automated-email"}}
        );
        const Object design = first_row(defaults);
        if (design.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body",
                  blog::api::ApiError{}
                      .json("Default automated email design setting not found", "Not Found")}}
            };
        }

        // #prepareSharedSenderUpdate: skip unchanged, validate changed, queue the ones
        // that require verification (managed-email policy), persist the rest.
        Object persist;
        std::vector<std::string> to_verify;
        for (const auto& [key, value]: sender) {
            const std::string text = scalar(value);
            if (trim(field(&design, key)) == text) {
                continue;
            }
            if ((key == "sender_email" || key == "sender_reply_to") && !text.empty() &&
                !valid_email(text)) {
                return core::workflow::StepResponse<Output>{
                    {{"status", "422"},
                     {"body",
                      blog::api::ApiError{}
                          .validation("You cannot set " + key + " to " + text, "", key)}}
                };
            }
            if ((key == "sender_email" || key == "sender_reply_to") && !text.empty() &&
                managed_email_enabled(ctx)) {
                to_verify.push_back(key);
            } else {
                persist[key] = value;
            }
        }
        if (!persist.empty()) {
            (void)ctx.store().edit(
                "email_design_settings",
                field(&design, "id"),
                core_serde::JsonCodec{}.encode(serde::Value{persist})
            );
        }

        // #sendSharedSenderVerifications -> MagicLink + transactional mail: observable
        // only as an outbox event dispatched through the registered mail adapter.
        mail_side_effect::ensure();
        mail_side_effect::ctx_ptr() = &ctx;
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        for (const auto& property: to_verify) {
            const std::string email = scalar(sender.at(property));
            outbox.emit(
                blog::handlers::outbox::Event{
                    "email.send_verification",
                    "{\"property\":\"" + property + "\",\"email\":\"" + escape(email) +
                        "\",\"subject\":\"Verify email address\"}",
                    field(&design, "id")
                }
            );
            ctx.store().resources().create(
                "sender_verification_tokens",
                "{\"token\":\"verify-" + property + "\",\"property\":\"" + property +
                    "\",\"value\":\"" + escape(email) + "\"}"
            );
        }
        outbox.commit();
        [[maybe_unused]] const auto drained = outbox.drain();
        mail_side_effect::ctx_ptr() = nullptr;

        // #loadWelcomeEmailsCollection + flatten with default sender settings.
        const auto rows = ctx.store().json_for("automated_emails", {});
        const auto updated_default = first_row(ctx.store().json_for(
            "email_design_settings",
            {{"filter", "slug:default-automated-email"}}
        ));
        const auto data = flatten_all(rows, updated_default);
        std::string meta = "{}";
        if (!to_verify.empty()) {
            meta = "{\"sent_email_verification\":[";
            for (std::size_t i = 0; i < to_verify.size(); ++i) {
                meta += (i ? std::string{","} : std::string{}) + '"' + to_verify[i] + '"';
            }
            meta += "]}";
        }
        const auto body = local_mapping::wrap("automated_emails", data, meta);
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;

    static const serde::Value* value_for(const Object* object, std::string_view key)
    {
        if (object == nullptr) {
            return nullptr;
        }
        for (const auto& [name, value]: *object) {
            if (name == key) {
                return &value;
            }
        }
        return nullptr;
    }

    static std::string field(const Object* object, std::string_view key)
    {
        const auto* value = value_for(object, key);
        if (value == nullptr) {
            return {};
        }
        if (const auto* text = std::get_if<std::string>(&value->get())) {
            return *text;
        }
        return {};
    }

    static std::string scalar(const serde::Value& value)
    {
        if (const auto* text = std::get_if<std::string>(&value.get())) {
            return *text;
        }
        return {};
    }

    static std::string trim(std::string_view s)
    {
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) {
            s.remove_prefix(1);
        }
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
            s.remove_suffix(1);
        }
        return std::string{s};
    }

    static std::string escape(std::string_view s)
    {
        std::string out;
        for (const char c: s) {
            if (c == '"' || c == '\\') {
                out.push_back('\\');
            }
            out.push_back(c);
        }
        return out;
    }

    static bool valid_email(std::string_view value)
    {
        const auto at = value.find('@');
        if (at == std::string_view::npos || at == 0 || at + 1 >= value.size()) {
            return false;
        }
        if (value.find(' ') != std::string_view::npos) {
            return false;
        }
        const auto domain = value.substr(at + 1);
        return domain.find('.') != std::string_view::npos && domain.front() != '.' &&
               domain.back() != '.';
    }

    static bool managed_email_enabled(blog::BlogContext& ctx)
    {
        const auto settings = ctx.store().json_for("settings", {});
        return settings.find("hostSettings:managedEmail:enabled") != std::string::npos &&
               settings.find("true") != std::string::npos;
    }

    static Object first_row(std::string_view json)
    {
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(json);
        if (parsed) {
            if (const auto* list = std::get_if<serde::Value::Array>(&parsed->get())) {
                if (!list->empty()) {
                    if (const auto* object = std::get_if<Object>(&list->front().get())) {
                        return *object;
                    }
                }
            }
        }
        return {};
    }

    static serde::Value raw_value(const Object* object, std::string_view key)
    {
        const auto* value = value_for(object, key);
        return value != nullptr ? *value : serde::Value{};
    }

    static std::string flatten(const Object* row, const Object* design)
    {
        const auto item = [](std::string_view key, const serde::Value& value)
        {
            return "\"" + std::string{key} + "\":" + core_serde::JsonCodec{}.encode(value);
        };
        return "{" + item("id", raw_value(row, "id")) + "," +
               item("status", raw_value(row, "status")) + "," +
               item("name", raw_value(row, "name")) + "," + item("slug", raw_value(row, "slug")) +
               "," + item("subject", raw_value(row, "subject")) + "," +
               item("lexical", raw_value(row, "lexical")) + "," +
               item("sender_name", raw_value(design, "sender_name")) + "," +
               item("sender_email", raw_value(design, "sender_email")) + "," +
               item("sender_reply_to", raw_value(design, "sender_reply_to")) + "," +
               item("email_design_setting_id", raw_value(row, "email_design_setting_id")) + "," +
               item("created_at", raw_value(row, "created_at")) + "," +
               item("updated_at", raw_value(row, "updated_at")) + "}";
    }

    static std::string flatten_all(std::string_view rows, const Object& design)
    {
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(rows);
        if (!parsed) {
            return "[]";
        }
        if (const auto* list = std::get_if<serde::Value::Array>(&parsed->get())) {
            std::string out = "[";
            bool first = true;
            for (const auto& item: *list) {
                if (const auto* object = std::get_if<Object>(&item.get())) {
                    out += (first ? "" : ",") + flatten(object, &design);
                    first = false;
                }
            }
            return out + "]";
        }
        return "[]";
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::automated_emails::automated_emails_senders_put::workflow
