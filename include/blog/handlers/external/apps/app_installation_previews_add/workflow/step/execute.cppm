export module blog_handlers:apps_app_installation_previews_add_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import blog_handlers_middleware;
import :outbox_shim;
import :apps_app_installations_browse_step;

// Strict Ghost parity for `appInstallationPreviews.add`
// (app-installation-previews.ts:12-28): PreviewRequest zod check on
// data.app_installation_previews[0].manifest_url, then service.preview(url)
// -> load(challenge + signed-manifest fetch/verify) -> review(identity check)
// -> previewOf(icon fetch, approval diff). Nothing is stored. Full
// AppInstallationPreview body; 422 ValidationError on bad input.
export namespace blog::handlers::apps::app_installation_previews_add::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"apps_app_installation_previews_add_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(std::string{in.body_as_string()});
        const auto* item = parsed ? first_data_item(*parsed, "app_installation_previews") : nullptr;
        if (item == nullptr) {
            return result({422, validation_error("Validation error",
                "Expected the manifest_url to preview.", "manifest_url")});
        }
        const auto manifest_url = scalar_of(*item, "manifest_url");
        if (manifest_url.empty()) {
            return result({422, validation_error("Validation error",
                "Expected the manifest_url to preview.", "manifest_url")});
        }
        return result(preview(*s_ctx, blog::handlers::outbox::global_outbox(), manifest_url));
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

} // namespace blog::handlers::apps::app_installation_previews_add::workflow
