export module blog_handlers:themes_themes_install_post_step;
import :domain_all;
import :outbox_shim;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::themes::themes_install_post::workflow {

namespace install_detail {

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

    [[nodiscard]] inline std::string lower(std::string_view value)
    {
        std::string out{value};
        for (char& c: out) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return out;
    }

    [[nodiscard]] inline std::string query_value(std::string_view path, std::string_view key)
    {
        const auto q = path.find('?');
        if (q == std::string_view::npos) {
            return {};
        }
        for (auto part: std::views::split(path.substr(q + 1), '&')) {
            std::string_view kv{std::ranges::begin(part), std::ranges::end(part)};
            const auto eq = kv.find('=');
            if (eq != std::string_view::npos && kv.substr(0, eq) == key) {
                return std::string{kv.substr(eq + 1)};
            }
        }
        return {};
    }

    /** @brief Ghost `installer.js:19`: `const [org, repo] = ref.toLowerCase().split('/')`.
     * The installed theme name is the lowercased repo segment (`TryGhost/Test` -> `test`). */
    [[nodiscard]] inline std::string theme_name(std::string_view ref)
    {
        const auto slash = ref.find('/');
        const auto repo = slash == std::string_view::npos ? ref : ref.substr(slash + 1);
        return lower(repo);
    }

} // namespace install_detail

/** @brief Ghost `install` (`themes.js:68-102`): validate `source`/`ref`, simulate
 * `installFromGithub(ref)`, write the context-local theme, emit `theme.uploaded`
 * and (on override) the `site.changed` side effect through `core_outbox`. */
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"themes_themes_install_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const auto path = in.get_path();
        const auto rec =
            blog::api::parse_object(in.body_as_string().empty() ? "{}" : in.body_as_string());

        std::string source = install_detail::query_value(path, "source");
        std::string ref = install_detail::query_value(path, "ref");
        if (const auto it = rec.find("source"); it != rec.end() && source.empty()) {
            source = install_detail::unquote(it->second);
        }
        if (const auto it = rec.find("ref"); it != rec.end() && ref.empty()) {
            ref = install_detail::unquote(it->second);
        }
        if (source.empty() || ref.empty()) {
            std::vector<std::string> segs;
            for (auto part: std::views::split(path, '/')) {
                std::string s{std::ranges::begin(part), std::ranges::end(part)};
                if (!s.empty()) {
                    segs.push_back(std::move(s));
                }
            }
            if (segs.size() >= 3) {
                if (source.empty()) {
                    source = segs[segs.size() - 2];
                }
                if (ref.empty()) {
                    ref = segs.back();
                }
            }
        }
        if (source != "github" || ref.empty()) {
            return core::workflow::StepResponse<Output>{Output{{"status", "400"}}};
        }

        const std::string name = install_detail::theme_name(ref);
        auto& rs = ctx.store().resources();
        blog::api::RecordDto theme;
        bool overridden{false};
        std::string override_id;
        for (const auto& existing: blog::api::parse_array(rs.list("themes"))) {
            const auto it = existing.find("name");
            if (it != existing.end() && install_detail::unquote(it->second) == name) {
                theme = existing;
                overridden = true;
                const auto idit = existing.find("id");
                if (idit != existing.end()) {
                    override_id = install_detail::unquote(idit->second);
                }
                break;
            }
        }
        if (theme.empty()) {
            theme["name"] = core_serde::JsonCodec{}.encode(name);
            theme["ref"] = core_serde::JsonCodec{}.encode(ref);
            theme["source"] = core_serde::JsonCodec{}.encode(source);
        }
        const auto theme_json = blog::api::record_to_json(theme);

        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        if (overridden) {
            rs.edit("themes", override_id, theme_json);
        } else {
            rs.create("themes", theme_json);
        }
        // Ghost `events.emit('theme.uploaded', {name: theme.name})` (`themes.js:97`).
        outbox.emit(
            blog::handlers::outbox::Event{
                .type = "theme.uploaded",
                .payload = "{\"name\":" + core_serde::JsonCodec{}.encode(name) + "}",
                .aggregate_id = name
            }
        );
        if (overridden) {
            // `X-Cache-Invalidate:'/*'` on response finish emits `site.changed`
            // (`web/parent/middleware/emit-events.js:10-15`).
            outbox.emit(
                blog::handlers::outbox::Event{
                    .type = "site.changed",
                    .payload = "{}",
                    .aggregate_id = "site"
                }
            );
        }
        outbox.commit();

        return core::workflow::StepResponse<Output>{Output{
            {"status", "200"},
            {"body", "{\"themes\":[" + theme_json + "]}"},
            {"override", overridden ? "1" : "0"}
        }};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::themes::themes_install_post::workflow
