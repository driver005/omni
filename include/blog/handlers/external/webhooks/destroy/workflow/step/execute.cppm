export module blog_handlers:webhooks_destroy_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import :webhooks_create_step;
import :outbox_shim;

export namespace blog::handlers::webhooks::destroy::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"webhooks_destroy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
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

        // Ghost `permissions.before` owner check, unconditional when a caller
        // integration context is present.
        const std::string caller{in.find_header(support::kIntegrationHeader)};
        if (!caller.empty()) {
            const auto stored = blog::api::JsonBody{}.parse_raw(stored_raw);
            const auto owner = stored.find("integration_id");
            const std::string stored_owner = owner == stored.end()
                                           ? std::string{} : support::unquote(owner->second);
            if (stored_owner != caller) {
                return core::workflow::StepResponse<Output>{"__FORBIDDEN__"};
            }
        }

        // Ghost Webhook.destroy runs in a transaction and emits `webhook.deleted`
        // through emitChange; the shared outbox supplies both.
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        const auto result = ctx.store().destroy("webhooks", id);
        if (!result.ok() || result.status() == 404) {
            outbox.rollback();
            return core::workflow::StepResponse<Output>{"__NOT_FOUND__"};
        }
        const std::string payload = "{\"webhooks\":[" + stored_raw + "]}";
        outbox.emit(blog::handlers::outbox::Event{"webhook.deleted", payload, id});
        outbox.commit();

        // Ghost statusCode:204 — no body on success (never the generic 200 "{}").
        return core::workflow::StepResponse<Output>{""};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::webhooks::destroy::workflow
