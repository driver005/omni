export module blog_handlers:themes_themes_name_activate_put_step;
import :domain_all;
import :outbox_shim;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::themes::themes_name_activate_put::workflow {

namespace activate_detail {

[[nodiscard]] inline std::string unquote(std::string_view raw)
{
    const auto start = raw.find_first_not_of(" \t\r\n");
    if (start == std::string_view::npos) return {};
    const auto t = raw.substr(start);
    if (t.size() >= 2 && t.front() == '"') {
        if (auto s = core_serde::JsonCodec{}.decode<std::string>(std::string{t})) return *s;
    }
    return std::string{t};
}

[[nodiscard]] inline std::string query_value(std::string_view path, std::string_view key)
{
    const auto q = path.find('?');
    if (q == std::string_view::npos) return {};
    for (auto part : path.substr(q + 1) | std::views::split('&')) {
        std::string_view kv{std::ranges::begin(part), std::ranges::end(part)};
        const auto eq = kv.find('=');
        if (eq != std::string_view::npos && kv.substr(0, eq) == key) {
            return std::string{kv.substr(eq + 1)};
        }
    }
    return {};
}

/** @brief `/themes/:name/activate` -> `:name` (the segment before the trailing action). */
[[nodiscard]] inline std::string theme_name(std::string_view path)
{
    if (const auto q = path.find('?'); q != std::string_view::npos) path = path.substr(0, q);
    std::vector<std::string> segs;
    for (auto part : path | std::views::split('/')) {
        std::string s{std::ranges::begin(part), std::ranges::end(part)};
        if (!s.empty()) segs.push_back(std::move(s));
    }
    if (segs.size() >= 2) return segs[segs.size() - 2];
    return {};
}

} // namespace activate_detail

/** @brief Ghost `activate`: host-limit guard (accepted no-op), `activate(name)`,
 * `models.Settings.edit([{key:'active_theme',value:name}])`, `getJSON(name, errors)`
 * (`themes.js:35-66`). `site.changed` is emitted through the outbox. */
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"themes_themes_name_activate_put_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const auto path = in.get_path();
        std::string name = activate_detail::theme_name(path);
        if (name.empty()) name = activate_detail::query_value(path, "name");
        if (name.empty()) {
            return core::workflow::StepResponse<Output>{Output{{"status", "400"}}};
        }

        auto& rs = ctx.store().resources();
        const std::string setting = "{\"key\":\"active_theme\",\"value\":" +
                                    core_serde::JsonCodec{}.encode(name) + "}";

        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        std::string setting_id;
        for (const auto& rec : blog::api::parse_array(rs.list("settings"))) {
            const auto it = rec.find("key");
            if (it != rec.end() && activate_detail::unquote(it->second) == "active_theme") {
                const auto idit = rec.find("id");
                if (idit != rec.end()) setting_id = activate_detail::unquote(idit->second);
                break;
            }
        }
        if (!setting_id.empty()) rs.edit("settings", setting_id, setting);
        else rs.create("settings", setting);

        std::string theme;
        for (const auto& rec : blog::api::parse_array(rs.list("themes"))) {
            const auto it = rec.find("name");
            if (it != rec.end() && activate_detail::unquote(it->second) == name) {
                theme = blog::api::record_to_json(rec);
                break;
            }
        }
        // `X-Cache-Invalidate:'/*'` on the response finish emits `site.changed`.
        outbox.emit(blog::handlers::outbox::Event{
            .type = "site.changed", .payload = "{}", .aggregate_id = "site"});
        outbox.commit();

        return core::workflow::StepResponse<Output>{
            Output{{"status", "200"}, {"body", "{\"themes\":[" + theme + "]}"}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::themes::themes_name_activate_put::workflow
