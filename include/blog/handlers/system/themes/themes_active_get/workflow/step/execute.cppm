export module blog_handlers:themes_themes_active_get_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::themes::themes_active_get::workflow {

namespace active_detail {

    [[nodiscard]] inline std::string unquote(std::string_view raw)
    {
        const auto start = raw.find_first_not_of(" \t\r\n");
        if (start == std::string_view::npos) {
            return {};
        }
        const auto t = raw.substr(start);
        if (t.size() >= 2 && t.front() == '"') {
            if (auto s = core_serde::JsonCodec{}.decode<std::string>(std::string{t})) {
                return *s;
            }
        }
        return std::string{t};
    }

} // namespace active_detail

/** @brief Ghost `settingsCache.get('active_theme')` -> `getThemeErrors(name)` ->
 * `getJSON(name, errors)` (`themes.js:23-33`). The context-local `settings` and
 * `themes` resource namespaces are the parity source; a theme record carrying
 * `warnings`/`errors` fields passes them through as `to-json.js:33-39` does. */
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"themes_themes_active_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        (void)in;
        auto& ctx = *s_ctx;
        auto& rs = ctx.store().resources();

        std::string active;
        for (const auto& rec: blog::api::parse_array(rs.list("settings"))) {
            const auto k = rec.find("key");
            const auto v = rec.find("value");
            if (k != rec.end() && v != rec.end() &&
                active_detail::unquote(k->second) == "active_theme") {
                active = active_detail::unquote(v->second);
                break;
            }
        }

        std::string theme;
        if (!active.empty()) {
            for (const auto& rec: blog::api::parse_array(rs.list("themes"))) {
                const auto it = rec.find("name");
                if (it != rec.end() && active_detail::unquote(it->second) == active) {
                    theme = blog::api::record_to_json(rec);
                    break;
                }
            }
        }
        return core::workflow::StepResponse<Output>{
            Output{{"body", "{\"themes\":[" + theme + "]}"}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::themes::themes_active_get::workflow
