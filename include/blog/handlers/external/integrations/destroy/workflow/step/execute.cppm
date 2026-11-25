export module blog_handlers:integrations_destroy_step;
import :domain_all;
import :integrations_dto_more_resources;
import :outbox_shim;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::integrations::destroy::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"integrations_destroy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        namespace support = blog::handlers::integrations::support;
        auto& ctx = *s_ctx;
        const std::string_view path{in.get_path()};
        const std::string id = support::tail_id(path);

        // options:['id'] required, require:true (integrations.js:118-128).
        auto record = ctx.store().resources().get("integrations", id);
        if (id.empty() || record.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"}, {"body", support::not_found_integration()}}
            };
        }

        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        ctx.store().resources().destroy("integrations", id);
        for (const auto& key: support::array(support::api_keys_for(ctx.store(), id))) {
            const auto key_id = support::unquote(support::field(key, "id"));
            if (!key_id.empty()) {
                ctx.store().resources().destroy("api_keys", key_id);
            }
        }
        for (const auto& hook: support::array(support::webhooks_for(ctx.store(), id))) {
            const auto hook_id = support::unquote(support::field(hook, "id"));
            if (!hook_id.empty()) {
                ctx.store().resources().destroy("webhooks", hook_id);
            }
        }
        outbox.emit(
            blog::handlers::outbox::Event{
                .type = "integration.deleted",
                .payload = std::format(R"({{"id":{}}})", support::quote(id)),
                .aggregate_id = id
            }
        );
        outbox.commit();

        return core::workflow::StepResponse<Output>{{{"status", "204"}, {"body", ""}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::integrations::destroy::workflow
