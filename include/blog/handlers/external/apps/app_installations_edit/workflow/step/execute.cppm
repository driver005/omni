export module blog_handlers:apps_app_installations_edit_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import blog_handlers_middleware;
import :outbox_shim;
import :apps_app_installations_browse_step;

// Strict Ghost parity for `appInstallations.edit` (app-installations.ts:91-103):
// permissions:{method:'add'}, reviewedChange(frame) (manifest_url+digest+
// non-negative revision), service.approve(context,id,{...}) -> repository.find
// -> 404/409 uninstalled -> fetchReviewed -> revision check -> approvalWrite
// -> whileUnchanged write trx -> recordAction('changes_approved') -> read(id).
export namespace blog::handlers::apps::app_installations_edit::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"apps_app_installations_edit_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        const auto id = path_id(in.get_path());
        if (id.empty()) return result(invalid("id is required", "id"));

        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(std::string{in.body_as_string()});
        const auto* item = parsed ? first_data_item(*parsed, "app_installations") : nullptr;
        if (item == nullptr) return result(invalid("app_installations is required", "app_installations"));
        const auto manifest_url = scalar_of(*item, "manifest_url");
        const auto digest = scalar_of(*item, "digest");
        const auto revision = scalar_of(*item, "revision");
        if (manifest_url.empty()) return result(invalid("manifest_url is required", "manifest_url"));
        if (digest.empty()) return result(invalid("digest is required", "digest"));
        long long rev{-1};
        try {
            std::size_t used{0};
            rev = std::stoll(revision, &used);
            if (used != revision.size()) rev = -1;
        } catch (...) {
        }
        if (rev < 0) return result(invalid("revision must be a non-negative integer", "revision"));

        return result(approve(*s_ctx, blog::handlers::outbox::global_outbox(), id,
                              manifest_url, digest, revision, actor_from_request(in)));
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    [[nodiscard]] static core::workflow::StepResponse<Output> result(const support::Result& value)
    {
        return core::workflow::StepResponse<Output>{
            {{"status", std::to_string(value.status)}, {"body", value.body}}};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::apps::app_installations_edit::workflow
