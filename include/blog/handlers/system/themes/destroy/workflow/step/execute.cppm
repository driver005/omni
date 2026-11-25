export module blog_handlers:themes_destroy_step;
import :domain_all;
import :outbox_shim;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::themes::destroy::workflow {

namespace destroy_detail {

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

    /** @brief `/themes/:name` -> `:name` (the last non-empty path segment). */
    [[nodiscard]] inline std::string theme_name(std::string_view path)
    {
        if (const auto q = path.find('?'); q != std::string_view::npos) {
            path = path.substr(0, q);
        }
        std::vector<std::string> segs;
        for (auto part: std::views::split(path, '/')) {
            std::string s{std::ranges::begin(part), std::ranges::end(part)};
            if (!s.empty()) {
                segs.push_back(std::move(s));
            }
        }
        return segs.empty() ? std::string{} : segs.back();
    }

} // namespace destroy_detail

/** @brief Ghost `destroy` (`themes.js:159-178`): reject default (`casper`/`source`)
 * and active themes, then `list.get` / `getStorage().delete` / `list.del`. Missing
 * theme -> 404 `NotFoundError "Theme does not exist."` (`storage.js:184-208`).
 * `site.changed` is emitted through the outbox. */
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"themes_destroy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const auto path = in.get_path();
        std::string name = destroy_detail::theme_name(path);
        if (name.empty()) {
            name = destroy_detail::query_value(path, "name");
        }
        if (name.empty()) {
            return core::workflow::StepResponse<Output>{
                Output{{"status", "400"}, {"message", "Invalid theme name"}}
            };
        }

        if (name == "casper" || name == "source") {
            return core::workflow::StepResponse<Output>{
                Output{{"status", "400"}, {"message", "Deleting the default theme is not allowed."}}
            };
        }

        auto& rs = ctx.store().resources();
        std::string active;
        for (const auto& rec: blog::api::parse_array(rs.list("settings"))) {
            const auto k = rec.find("key");
            const auto v = rec.find("value");
            if (k != rec.end() && v != rec.end() &&
                destroy_detail::unquote(k->second) == "active_theme") {
                active = destroy_detail::unquote(v->second);
                break;
            }
        }
        if (!active.empty() && active == name) {
            return core::workflow::StepResponse<Output>{
                Output{{"status", "400"}, {"message", "Deleting the active theme is not allowed."}}
            };
        }

        std::string id;
        for (const auto& rec: blog::api::parse_array(rs.list("themes"))) {
            const auto it = rec.find("name");
            if (it == rec.end()) {
                continue;
            }
            if (destroy_detail::unquote(it->second) != name) {
                continue;
            }
            const auto idit = rec.find("id");
            if (idit != rec.end()) {
                id = destroy_detail::unquote(idit->second);
            }
            break;
        }
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                Output{{"status", "404"}, {"message", "Theme does not exist."}}
            };
        }

        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        rs.destroy("themes", id);
        // `X-Cache-Invalidate:'/*'` on the response finish emits `site.changed`.
        outbox.emit(
            blog::handlers::outbox::Event{
                .type = "site.changed",
                .payload = "{}",
                .aggregate_id = "site"
            }
        );
        outbox.commit();

        return core::workflow::StepResponse<Output>{Output{{"status", "204"}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::themes::destroy::workflow
