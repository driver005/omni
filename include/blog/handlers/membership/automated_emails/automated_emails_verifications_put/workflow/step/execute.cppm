export module blog_handlers:automated_emails_automated_emails_verifications_put_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import :outbox_shim;
import interfaces;

export namespace blog::handlers::automated_emails::automated_emails_verifications_put::workflow {

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

/** Outbox adapter for the verification side effect. Ghost calls
 *  MagicLink.getDataFromToken -> EmailDesignSetting.edit -> collection reload; the
 *  mail acknowledgement is observable here as an `email.sender_verified` event. */
namespace verification_side_effect {
    using Event = blog::handlers::outbox::Event;

    inline std::vector<Event>& log()
    {
        static std::vector<Event> v;
        return v;
    }

    class Adapter final : public blog::handlers::outbox::Adapter
    {
    public:
        void dispatch(const Event& event) override
        {
            if (event.type == "email.sender_verified") {
                log().push_back(event);
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
} // namespace verification_side_effect

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{
        "automated_emails_automated_emails_verifications_put_execute"
    };

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        std::string raw{in.body_as_string()};
        if (raw.empty()) {
            raw = "{}";
        }
        std::string token;
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(raw);
        if (parsed) {
            if (const auto* root = std::get_if<Object>(&parsed->get())) {
                const auto* data = value_for(root, "data");
                if (data != nullptr) {
                    if (const auto* dobj = std::get_if<Object>(&data->get())) {
                        token = field(dobj, "token");
                    }
                }
                if (token.empty()) {
                    token = field(root, "token");
                }
            }
        }
        if (token.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body", blog::api::ApiError{}.validation("Token is required", "", "token")}}
            };
        }

        // Fixture-backed stand-in for MagicLink.getDataFromToken(). A real signed,
        // single-use token codec (expiry + signature) is a new-partition need.
        Object payload;
        bool matched = false;
        const auto pending = ctx.store().resources().list("sender_verification_tokens");
        const auto decoded = core_serde::JsonCodec{}.decode<serde::Value>(pending);
        if (decoded) {
            if (const auto* list = std::get_if<serde::Value::Array>(&decoded->get())) {
                for (const auto& item: *list) {
                    if (const auto* object = std::get_if<Object>(&item.get())) {
                        if (field(object, "token") != token) {
                            continue;
                        }
                        for (const auto& [key, value]: *object) {
                            if (key != "token") {
                                payload[key] = value;
                            }
                        }
                        matched = true;
                        break;
                    }
                }
            }
        }
        if (!matched) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body", blog::api::ApiError{}.validation("Token is invalid", "", "token")}}
            };
        }

        const std::string property = field(&payload, "property");
        // VERIFIED_SENDER_PROPERTIES allowlist (IncorrectUsageError in Ghost).
        if (property != "sender_reply_to") {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}.json(
                      "Not allowed to update this sender setting via token",
                      "Incorrect Usage"
                  )}}
            };
        }
        const std::string value = field(&payload, "value");
        if (!valid_email(value)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}
                      .validation("You cannot set " + property + " to " + value, "", property)}}
            };
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
        Object edit;
        edit[property] = serde::Value{value};
        (void)ctx.store().edit(
            "email_design_settings",
            field(&design, "id"),
            core_serde::JsonCodec{}.encode(serde::Value{edit})
        );

        verification_side_effect::ensure();
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.emit(
            blog::handlers::outbox::Event{
                "email.sender_verified",
                "{\"property\":\"" + property + "\",\"value\":\"" + escape(value) + "\"}",
                field(&design, "id")
            }
        );
        [[maybe_unused]] const auto drained = outbox.drain();

        const auto rows = ctx.store().json_for("automated_emails", {});
        const auto updated = first_row(ctx.store().json_for(
            "email_design_settings",
            {{"filter", "slug:default-automated-email"}}
        ));
        const auto data = flatten_all(rows, updated);
        const auto body = local_mapping::wrap(
            "automated_emails",
            data,
            "{\"email_verified\":\"" + property + "\"}"
        );
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
} // namespace blog::handlers::automated_emails::automated_emails_verifications_put::workflow
