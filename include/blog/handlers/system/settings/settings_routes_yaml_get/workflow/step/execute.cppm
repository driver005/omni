export module blog_handlers:settings_settings_routes_yaml_get_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import :settings_browse_step;

export namespace blog::handlers::settings::settings_routes_yaml_get::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = support::Result;
    static constexpr std::string_view name{"settings_settings_routes_yaml_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input&, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        using namespace support;
        // Ghost routeSettings.api.download() -> DynamicRoutingService.download()
        // -> store.get().yamlSource; direct file I/O is disallowed, so the
        // context-local routes_yaml record is the exact store stand-in.
        const auto rows = blog::api::parse_array(ctx.store().resources().list("routes_yaml"));
        std::string yaml;
        if (!rows.empty()) {
            if (const auto it = rows.back().find("content"); it != rows.back().end()) {
                yaml = core_serde::JsonCodec{}.decode<std::string>(it->second).value_or(it->second);
            }
        }
        if (yaml.empty()) {
            yaml = "# See https://docs.ghost.org/themes/routing/#routes\nroutes: null\n";
        }
        Result result{
            {"status", "200"},
            {"body", yaml},
            {"content_type", "text/plain"},
            {"content_disposition", R"(attachment; filename="routes.yaml")"}
        };
        // Ghost cacheInvalidate:false -> no X-Cache-Invalidate.
        return core::workflow::StepResponse<Output>{result};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::settings::settings_routes_yaml_get::workflow
