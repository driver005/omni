export module blog_handlers:custom_theme_settings_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::custom_theme_settings::browse::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"custom_theme_settings_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input&, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        // Ghost `query()` takes no frame and `listSettings()` has no arguments
        // (custom-theme-settings.js:12-14; service.js:89-95): no options/filter/fields/
        // limit/page are read. The context-local namespace is the stand-in for the
        // in-process `_activeThemeSettings` map; no per-request DB read.
        const auto rows = ctx.store().json_for("custom_theme_settings", {});
        // Output serializer sets frame.response[docName] = full setting objects, with
        // no pagination meta (output/custom-theme-settings.js:2-6).
        const auto body = envelope(rows);
        return core::workflow::StepResponse<Output>{body};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    // Ghost output/default.js: frame.response[docName] = rows, no meta key.
    // Kept local: blog_handlers:mapping partitions are not registered in BUILD.
    static std::string envelope(std::string_view rows)
    {
        while (!rows.empty() && std::isspace(static_cast<unsigned char>(rows.front()))) {
            rows.remove_prefix(1);
        }
        while (!rows.empty() && std::isspace(static_cast<unsigned char>(rows.back()))) {
            rows.remove_suffix(1);
        }
        const auto data = rows.size() >= 2 && rows.front() == '[' && rows.back() == ']'
                              ? std::string{rows}
                              : std::string{"[]"};
        return "{\"custom_theme_settings\":" + data + "}";
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::custom_theme_settings::browse::workflow
