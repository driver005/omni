export module blog_handlers:apps_app_installations_add_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import blog_handlers_middleware;
import :outbox_shim;
import :apps_app_installations_browse_step;

// Strict Ghost parity for `appInstallations.add` (app-installations.ts:77-87):
// reviewed(frame) -> service.create(actingContext, {manifestUrl,digest})
// -> fetchReviewed (identity challenge, manifest fetch/verify, digest check)
// -> fetchIcon -> repository.insert (trx) -> recordAction('installed').
// 201 with an `app_installations` envelope; no cache header.
export namespace blog::handlers::apps::app_installations_add::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"apps_app_installations_add_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(std::string{in.body_as_string()});
        const auto* item = parsed ? first_data_item(*parsed, "app_installations") : nullptr;
        if (item == nullptr) {
            return result(invalid("app_installations is required", "app_installations"));
        }
        const auto manifest_url = scalar_of(*item, "manifest_url");
        const auto digest = scalar_of(*item, "digest");
        if (manifest_url.empty()) return result(invalid("manifest_url is required", "manifest_url"));
        if (digest.empty()) return result(invalid("digest is required", "digest"));

        auto& ctx = *s_ctx;
        auto& outbox = blog::handlers::outbox::global_outbox();
        const auto reviewed = fetch_reviewed(ctx, outbox, manifest_url, digest);
        if (reviewed.status != 200) return result(reviewed);
        CheckedManifest checked;
        checked.loaded.row = reviewed.body;
        checked.loaded.manifest_url = manifest_url;
        auto manifest = raw_member(reviewed.body, "manifest");
        if (manifest.empty() || manifest == "null") manifest = "{}";
        checked.loaded.manifest = manifest;
        checked.loaded.digest = scalar_raw(reviewed.body, "digest");
        auto icon = raw_member(reviewed.body, "icon");
        if (icon.empty() || icon == "null") icon = raw_member(reviewed.body, "icon_data_url");
        checked.loaded.icon = icon.empty() ? std::string{"null"} : icon;
        checked.loaded.app_id = scalar_raw(manifest, "id");
        checked.loaded.identity_key_thumbprint = scalar_raw(reviewed.body, "backend_key_thumbprint");
        checked.installation = current_installation_of(ctx, checked.loaded.app_id);

        return result(install(ctx, outbox, checked, actor_from_request(in)));
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

} // namespace blog::handlers::apps::app_installations_add::workflow
