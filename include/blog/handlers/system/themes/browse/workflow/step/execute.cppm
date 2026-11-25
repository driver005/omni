export module blog_handlers:themes_browse_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::themes::browse::workflow {

/** @brief Ghost `themeService.api.getJSON()` -> `{themes:[…]}`, no pagination meta
 * (`services/themes/to-json.js:47`, `utils/serializers/output/themes.js:4-8`). */
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"themes_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        (void)in;
        auto& ctx = *s_ctx;
        const auto rows = ctx.store().json_for("themes", {});
        return core::workflow::StepResponse<Output>{Output{{"body", "{\"themes\":" + rows + "}"}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::themes::browse::workflow
