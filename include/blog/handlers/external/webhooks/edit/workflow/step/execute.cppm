export module blog_handlers:webhooks_edit_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import :webhooks_create_step;
import :outbox_shim;

export namespace blog::handlers::webhooks::edit::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"webhooks_edit_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        namespace support = blog::handlers::webhooks::support;
        auto& ctx = *s_ctx;

        const std::string id = support::id_tail(in.get_path());
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{"__VALIDATION__"};
        }

        const std::string stored_raw = ctx.store().resources().get("webhooks", id);
        if (stored_raw.empty()) {
            return core::workflow::StepResponse<Output>{"__NOT_FOUND__"};
        }
        const auto stored = blog::api::JsonBody{}.parse_raw(stored_raw);

        // Ghost `permissions.before`: when the caller carries an integration context,
        // findOne + owner check runs unconditionally (even when integration_id is null).
        const std::string caller{in.find_header(support::kIntegrationHeader)};
        if (!caller.empty()) {
            const auto owner = stored.find("integration_id");
            const std::string stored_owner =
                owner == stored.end() ? std::string{} : support::unquote(owner->second);
            if (stored_owner != caller) {
                return core::workflow::StepResponse<Output>{"__FORBIDDEN__"};
            }
        }

        // Ghost `data: ['name','event','target_url','secret','api_version']`.
        // Presence, not non-emptiness, decides: null/empty values clear a field.
        const std::string raw =
            in.body_as_string().empty() ? std::string{"{}"} : std::string{in.body_as_string()};
        const auto fields = support::unwrap(raw);
        blog::api::RecordDto patch;
        for (const char* key: {"name", "event", "target_url", "secret", "api_version"}) {
            if (const auto it = fields.find(key); it != fields.end()) {
                patch[key] = it->second;
            }
        }

        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        const auto result = ctx.store().edit("webhooks", id, blog::api::record_to_json(patch));
        if (!result.ok() || result.status() >= 400) {
            outbox.rollback();
            return core::workflow::StepResponse<Output>{"__NOT_FOUND__"};
        }

        const auto after =
            blog::api::JsonBody{}.parse_raw(ctx.store().resources().get("webhooks", id));
        const std::string body =
            "{\"webhooks\":[" + support::webhook_model(after, patch, id, true) + "]}";
        // webhook.edited == Ghost Webhook.onUpdated -> emitChange('edited').
        outbox.emit(blog::handlers::outbox::Event{"webhook.edited", body, id});
        outbox.commit();
        return core::workflow::StepResponse<Output>{body};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::webhooks::edit::workflow
