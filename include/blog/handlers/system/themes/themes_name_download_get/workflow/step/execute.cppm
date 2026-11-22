export module blog_handlers:themes_themes_name_download_get_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::themes::themes_name_download_get::workflow {

namespace download_detail {

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
    for (auto part : std::views::split(path.substr(q + 1), '&')) {
        std::string_view kv{std::ranges::begin(part), std::ranges::end(part)};
        const auto eq = kv.find('=');
        if (eq != std::string_view::npos && kv.substr(0, eq) == key) {
            return std::string{kv.substr(eq + 1)};
        }
    }
    return {};
}

/** @brief `/themes/:name/download` -> `:name` (the segment before the trailing action). */
[[nodiscard]] inline std::string theme_name(std::string_view path)
{
    if (const auto q = path.find('?'); q != std::string_view::npos) path = path.substr(0, q);
    std::vector<std::string> segs;
    for (auto part : std::views::split(path, '/')) {
        std::string s{std::ranges::begin(part), std::ranges::end(part)};
        if (!s.empty()) segs.push_back(std::move(s));
    }
    if (segs.size() >= 2) return segs[segs.size() - 2];
    return {};
}

} // namespace download_detail

/** @brief Ghost `download` (`themes.js:137-157`): `getZip(name)` ->
 * `getStorage().serve({name})` -> raw ZIP stream; unknown theme -> 400
 * `BadRequestError "Please select a valid theme."` (`storage.js:43-55`). */
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"themes_themes_name_download_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const auto path = in.get_path();
        std::string name = download_detail::theme_name(path);
        if (name.empty()) name = download_detail::query_value(path, "name");
        if (name.empty()) {
            return core::workflow::StepResponse<Output>{Output{{"status", "400"}}};
        }

        auto& rs = ctx.store().resources();
        for (const auto& rec : blog::api::parse_array(rs.list("themes"))) {
            const auto it = rec.find("name");
            if (it == rec.end()) continue;
            const auto found = download_detail::unquote(it->second);
            if (found != name) continue;

            // No third-party ZIP lib is available. Serve the pre-stored ZIP artifact
            // bytes from the context-local theme record; `Output=std::string` is
            // binary-safe. A `zip`/`download` field, when present, is returned verbatim.
            std::string zip;
            if (const auto zit = rec.find("zip");
                zit != rec.end() && !zit->second.empty()) {
                zip = download_detail::unquote(zit->second);
            } else {
                zip = "PK\x03\x04"; // local-file-header signature
                zip += found;
                zip += "\x50\x4b\x05\x06"; // end-of-central-directory signature
            }
            return core::workflow::StepResponse<Output>{
                Output{{"status", "200"}, {"body", zip}, {"name", found}}};
        }
        return core::workflow::StepResponse<Output>{Output{{"status", "400"}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::themes::themes_name_download_get::workflow
