export module blog_handlers:newsletters_edit_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_serde;
import interfaces;
import :outbox_shim;
import :newsletters_browse_step;

export namespace blog::handlers::newsletters::edit::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"newsletters_edit_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        auto& ctx = *s_ctx;

        constexpr std::array<std::string_view, 2> kOptions{"id", "include"};
        auto q = core::query::parse(in.get_path(), kOptions);
        if (!valid_include(q)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body", validation("Validation error", "Invalid include", "include")}}
            };
        }

        const std::string path{in.get_path()};
        std::string id = path.substr(path.rfind('/') + 1);
        if (const auto pos = id.find('?'); pos != std::string::npos) {
            id = id.substr(0, pos);
        }
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body", validation("Validation error", "An id is required", "id")}}
            };
        }

        // Unwrap frame.data.newsletters[0].
        Object attrs;
        const auto raw = std::string{in.body_as_string()};
        const auto parsed = raw.empty() ? std::nullopt : decode(raw);
        if (parsed) {
            const auto* root = std::get_if<Object>(&parsed->get());
            if (root != nullptr) {
                const auto* data = value_for(root, "data");
                const auto* source = root;
                if (data != nullptr) {
                    if (const auto* d = std::get_if<Object>(&data->get())) {
                        source = d;
                    }
                }
                const auto* list = value_for(source, "newsletters");
                if (list != nullptr) {
                    if (const auto* array = std::get_if<Array>(&list->get())) {
                        if (!array->empty()) {
                            if (const auto* item = std::get_if<Object>(&array->front().get())) {
                                attrs = *item;
                            }
                        }
                    }
                }
            }
        }

        // service.edit: findOne({id}, require:true) -> NotFoundError.
        const auto original = ctx.store().find_by_resource("newsletters", id);
        if (!original) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"}, {"body", not_found("Newsletter not found.")}}
            };
        }
        const Object before = first_row("[" + *original + "]");

        // prepAttrsForEmailVerification(attrs, originalNewsletter).
        for (const auto property:
             {std::string_view{"sender_email"}, std::string_view{"sender_reply_to"}}) {
            const std::string email = field(&attrs, property);
            if (email.empty()) {
                continue;
            }
            if (property == "sender_reply_to" && (email == "newsletter" || email == "support")) {
                continue;
            }
            if (email == field(&before, property)) {
                continue;
            }
            if (!valid_email(email)) {
                const std::string message =
                    property == "sender_email"
                        ? "You cannot set the sender email address to " + email
                        : "You cannot set the reply-to email address to " + email;
                return core::workflow::StepResponse<Output>{
                    {{"status", "422"}, {"body", validation(message, "", property)}}
                };
            }
        }
        const std::string name = field(&attrs, "name");
        if (!name.empty() && name_taken(ctx, name, id)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body", validation("A newsletter with the same name already exists", "", "name")}}
            };
        }

        std::vector<std::pair<std::string, std::string>> to_verify;
        if (managed_email_enabled(ctx)) {
            for (const auto property:
                 {std::string_view{"sender_email"}, std::string_view{"sender_reply_to"}}) {
                const std::string email = field(&attrs, property);
                if (email.empty()) {
                    continue;
                }
                if (field(&before, property) == email) {
                    continue;
                }
                if (property == "sender_reply_to" && email == field(&attrs, "sender_email")) {
                    continue;
                }
                to_verify.emplace_back(property, email);
                Object kept;
                for (const auto& entry: attrs) {
                    if (entry.first != property) {
                        kept[entry.first] = entry.second;
                    }
                }
                attrs = std::move(kept);
            }
        }

        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        const auto result =
            ctx.store().edit("newsletters", id, support::encode(serde::Value{attrs}));
        for (const auto& [property, email]: to_verify) {
            outbox.emit(
                blog::handlers::outbox::Event{
                    "email.send_verification",
                    "{\"property\":\"" + escape(property) + "\",\"email\":\"" + escape(email) +
                        "\",\"subject\":\"Verify email address\"}",
                    id
                }
            );
            static_cast<void>(issue_verification_token(ctx, id, property, email));
        }
        outbox.emit(
            blog::handlers::outbox::Event{
                "newsletter.edited",
                "{\"id\":\"" + escape(id) + "\"}",
                id
            }
        );
        outbox.commit();
        mail_side_effect::ctx_ptr() = &ctx;
        mail_side_effect::ensure();
        [[maybe_unused]] const auto drained = outbox.drain();
        mail_side_effect::ctx_ptr() = nullptr;

        if (result.status() == 404) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"}, {"body", result.body()}}
            };
        }
        int status = result.status();
        if (status == 201) {
            status = 200; // Ghost edit returns 200.
        }

        std::string model;
        const auto reloaded = ctx.store().find_by_resource("newsletters", id);
        if (reloaded) {
            model = *reloaded;
        } else {
            model = first_row(result.body()).empty()
                        ? std::string{"{}"}
                        : encode(serde::Value{first_row(result.body())});
        }

        std::string meta;
        if (!to_verify.empty()) {
            meta = "{\"sent_email_verification\":[";
            for (std::size_t i = 0; i < to_verify.size(); ++i) {
                meta += std::string{i ? "," : ""} + "\"" + to_verify[i].first + "\"";
            }
            meta += "]}";
        }
        const std::string body = wrap("newsletters", model, meta);
        return core::workflow::StepResponse<Output>{
            {{"status", std::to_string(status)}, {"body", body}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static bool
    name_taken(blog::BlogContext& ctx, std::string_view name, std::string_view except_id)
    {
        const auto parsed = support::decode(ctx.store().json_for("newsletters", {}));
        if (!parsed) {
            return false;
        }
        if (const auto* list = std::get_if<support::Array>(&parsed->get())) {
            for (const auto& item: *list) {
                if (const auto* object = std::get_if<support::Object>(&item.get())) {
                    const std::string other_id = support::field(object, "id");
                    std::string wanted_id{except_id};
                    try {
                        wanted_id = serde::ghost_id(std::stoll(std::string{except_id}));
                    } catch (...) {
                    }
                    if (!except_id.empty() && other_id == wanted_id) {
                        continue;
                    }
                    std::string other = support::field(object, "name");
                    std::ranges::transform(
                        other,
                        other.begin(),
                        [](unsigned char c)
                        {
                            return static_cast<char>(std::tolower(c));
                        }
                    );
                    std::string wanted{name};
                    std::ranges::transform(
                        wanted,
                        wanted.begin(),
                        [](unsigned char c)
                        {
                            return static_cast<char>(std::tolower(c));
                        }
                    );
                    if (other == wanted) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::newsletters::edit::workflow
