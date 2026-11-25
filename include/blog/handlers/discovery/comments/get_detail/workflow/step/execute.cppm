export module blog_handlers:comments_get_detail_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import interfaces;

export namespace blog::handlers::comments::get_detail::workflow {

namespace local_mapping {
    [[nodiscard]] inline std::string
    wrap(std::string_view resource, std::string_view rows, std::string_view meta = {})
    {
        return "{\"" + std::string{resource} + "\":" + std::string{rows} +
               (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
    }

    [[nodiscard]] inline std::string clean(std::string_view json, std::string_view)
    {
        return std::string{json};
    }

    [[nodiscard]] inline std::string pagination(
        std::size_t total,
        std::size_t limit,
        std::size_t page,
        std::size_t pages,
        std::string_view prev,
        std::string_view next
    )
    {
        return std::format(
            R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
            total,
            limit,
            page,
            pages,
            prev.empty() ? std::string{"null"} : std::string{prev},
            next.empty() ? std::string{"null"} : std::string{next}
        );
    }
} // namespace local_mapping

namespace detail {

    inline constexpr std::string_view kOptions[]{
        "post_id",
        "include",
        "page",
        "limit",
        "fields",
        "filter",
        "order",
        "debug",
        "impersonate_member_uuid"
    };

    [[nodiscard]] inline std::string unquote(std::string_view raw)
    {
        auto s = raw;
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) {
            s.remove_prefix(1);
        }
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
            s.remove_suffix(1);
        }
        if (s.size() >= 2 &&
            ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\''))) {
            s = s.substr(1, s.size() - 2);
        }
        return std::string{s};
    }

    [[nodiscard]] inline std::string post_id_from_path(std::string_view path)
    {
        const auto bare = path.substr(0, path.find('?'));
        const auto pos = bare.find("/post/");
        if (pos == std::string_view::npos) {
            return {};
        }
        const auto start = pos + 6;
        const auto end = bare.find('/', start);
        return std::string{
            end == std::string_view::npos ? bare.substr(start) : bare.substr(start, end - start)
        };
    }

    /** @brief Ghost comments service checkEnabled: settings.cache `comments_enabled`. */
    [[nodiscard]] inline bool comments_disabled(const blog::BlogContext& ctx)
    {
        for (const auto& record: blog::api::parse_array(ctx.store().json_for("settings"))) {
            const auto key = record.find("key");
            const auto value = record.find("value");
            if (key == record.end() || value == record.end()) {
                continue;
            }
            if (unquote(key->second) == "comments_enabled" && unquote(value->second) == "off") {
                return true;
            }
        }
        return false;
    }

    /** @brief Ghost #withImpersonationContext: resolve the acting member by UUID. */
    [[nodiscard]] inline bool
    member_uuid_exists(const blog::BlogContext& ctx, std::string_view uuid)
    {
        for (const auto& record: blog::api::parse_array(ctx.store().json_for("members"))) {
            const auto it = record.find("uuid");
            if (it != record.end() && unquote(it->second) == uuid) {
                return true;
            }
        }
        return false;
    }

} // namespace detail

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"comments_get_detail_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        auto q = core::query::parse(in.get_path(), detail::kOptions);

        std::string post_id = detail::post_id_from_path(in.get_path());
        if (const auto it = q.options.find("post_id");
            post_id.empty() && it != q.options.end() && !it->second.empty()) {
            post_id = it->second;
        }
        if (post_id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body",
                  blog::api::ApiError{}
                      .validation("Validation error", "Field value is required", "post_id")}}
            };
        }

        if (const auto it = q.options.find("impersonate_member_uuid");
            it != q.options.end() && !it->second.empty() &&
            !detail::member_uuid_exists(ctx, it->second)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body", blog::api::ApiError{}.json("Unable to find member", "Not Found")}}
            };
        }

        if (detail::comments_disabled(ctx)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "405"},
                 {"body",
                  blog::api::ApiError{}.json("Comments are not enabled", "Method Not Allowed")}}
            };
        }

        // adminBrowse: #withPostFilter ANDs `post_id:<id>` with the caller filter,
        // then service.getAdminComments forces parentId:null and pinned-first.
        q.filters.emplace_back("post_id", post_id);
        q.filters.emplace_back("parent_id", "null");

        const auto cleaned = local_mapping::clean(ctx.store().json_for("comments"), "comment");
        std::string wire{"["};
        bool first = true;
        for (auto record: blog::api::parse_array(cleaned)) {
            const auto id_it = record.find("id");
            const auto parent_it = record.find("parent_id");
            if (id_it != record.end() && parent_it != record.end() &&
                id_it->second == parent_it->second) {
                parent_it->second = "null";
            }
            if (!first) {
                wire.push_back(',');
            }
            first = false;
            wire += blog::api::record_to_json(record);
        }
        wire.push_back(']');
        std::string page_rows;
        std::size_t total{0};
        try {
            auto all = q;
            all.limit = std::numeric_limits<std::size_t>::max();
            all.page = 1;
            all.offset = 0;
            total = blog::api::parse_array(core::query::apply(wire, all)).size();
            page_rows = core::query::apply(wire, q);
        } catch (const std::exception&) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body", blog::api::ApiError{}.json("Invalid Ghost query", "Bad Request")}}
            };
        }

        const auto limit = q.limit == 0 || q.limit == std::numeric_limits<std::size_t>::max()
                               ? (total == 0 ? std::size_t{15} : total)
                               : q.limit;
        const auto page = q.page == 0 ? std::size_t{1} : q.page;
        const auto pages = limit == 0 ? std::size_t{0} : (total + limit - 1) / limit;
        const std::string prev = page > 1 ? std::to_string(page - 1) : "null";
        const std::string next = page < pages ? std::to_string(page + 1) : "null";
        const auto meta = local_mapping::pagination(total, limit, page, pages, prev, next);

        return core::workflow::StepResponse<Output>{
            {{"status", "200"},
             {"body", local_mapping::wrap("comments", page_rows, "{\"pagination\":" + meta + "}")}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::comments::get_detail::workflow
