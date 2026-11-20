export module blog_handlers:upgrades_create_step;
import :domain_all;
import :upgrades_browse_step;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::upgrades::create::workflow {

class ExecuteStep {
public:
    using Input = interfaces::io::IRequest&;
    using Output = support::Output;
    static constexpr std::string_view name{"upgrades_create_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        // Ghost: strict z.object({upgrades: z.tuple([z.object({component,
        // to_version, idempotency_key}).strict()])}).strict() — unknown keys at
        // any level are rejected, not stripped (upgrades.ts:13-25).
        const std::string raw{in.body_as_string()};
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(raw);
        const auto* root = parsed ? std::get_if<support::Object>(&parsed->get()) : nullptr;
        if (root == nullptr || support::key_count(root) != 1) {
            return invalid_body();
        }
        const auto* upgrades = support::value_for(root, "upgrades");
        if (upgrades == nullptr) {
            return invalid_body();
        }
        const auto* array = std::get_if<serde::Value::Array>(&upgrades->get());
        if (array == nullptr || array->size() != 1) {
            return invalid_body();
        }
        const auto* job = std::get_if<support::Object>(&array->front().get());
        if (job == nullptr || support::key_count(job) != 3) {
            return invalid_body();
        }
        const auto component = support::field(job, "component");
        const auto to_version = support::field(job, "to_version");
        const auto client_key = support::field(job, "idempotency_key");
        if (!support::valid_component(component) || !support::valid_version(to_version) ||
            !support::valid_uuid_v4(client_key)) {
            return invalid_body();
        }

        // Ghost throws NoPermissionError when frame.options.context.user is absent.
        const auto user = support::staff_user_id(in);
        if (user.empty()) {
            return core::workflow::StepResponse<Output>{
                support::no_permission_error("Only staff users can request updates.")};
        }

        // UpgradeService.createRequest hashes JSON.stringify([userId, clientKey])
        // before the adapter sees it; the raw key is never echoed or persisted.
        const auto scoped_key = support::scoped_idempotency_key(user, client_key);
        return core::workflow::StepResponse<Output>{
            support::create_request(ctx, component, to_version, scoped_key, user)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    [[nodiscard]] static core::workflow::StepResponse<Output> invalid_body()
    {
        return core::workflow::StepResponse<Output>{support::validation_error(
            "Expected one update with component, to_version and idempotency_key.",
            support::kValidationHelp)};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::upgrades::create::workflow
