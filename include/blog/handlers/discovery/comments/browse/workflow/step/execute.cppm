export module blog_handlers:comments_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import interfaces;

export namespace blog::handlers::comments::browse::workflow {

namespace local_mapping {
[[nodiscard]] inline std::string wrap(std::string_view resource, std::string_view rows,
                                      std::string_view meta = {})
{
    return "{\"" + std::string{resource} + "\":" + std::string{rows} +
           (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
}
[[nodiscard]] inline std::string clean(std::string_view json, std::string_view) { return std::string{json}; }
[[nodiscard]] inline std::string pagination(std::size_t total, std::size_t limit, std::size_t page,
                                            std::size_t pages, std::string_view prev,
                                            std::string_view next)
{
    return std::format(R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
                       total, limit, page, pages,
                       prev.empty() ? std::string{"null"} : std::string{prev},
                       next.empty() ? std::string{"null"} : std::string{next});
}
} // namespace local_mapping

namespace detail {

inline constexpr std::string_view kOptions[]{
    "page", "limit", "filter", "order", "include_nested"};

struct ReportCount
{
    std::string op{"="};
    long long value{0};
};

[[nodiscard]] inline std::string_view trim(std::string_view s) noexcept
{
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.remove_prefix(1);
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.remove_suffix(1);
    return s;
}

/** @brief Parse the `count.reports` operator Ghost resolves to raw SQL. */
[[nodiscard]] inline std::optional<ReportCount> report_count(std::string_view value)
{
    value = trim(value);
    ReportCount rc;
    if (value.starts_with(">=")) { rc.op = ">="; value.remove_prefix(2); }
    else if (value.starts_with("<=")) { rc.op = "<="; value.remove_prefix(2); }
    else if (value.starts_with("!=")) { rc.op = "!="; value.remove_prefix(2); }
    else if (value.starts_with('>')) { rc.op = ">"; value.remove_prefix(1); }
    else if (value.starts_with('<')) { rc.op = "<"; value.remove_prefix(1); }
    else if (value.starts_with('!') || value.starts_with('-')) { rc.op = "!="; value.remove_prefix(1); }
    else if (value.starts_with('=')) { value.remove_prefix(1); }
    value = trim(value);
    if (value.size() >= 2 && ((value.front() == '\'' && value.back() == '\'') ||
                              (value.front() == '"' && value.back() == '"'))) {
        value = value.substr(1, value.size() - 2);
    }
    if (value.empty()) return std::nullopt;
    for (const char c : value) {
        if (c < '0' || c > '9') return std::nullopt;
    }
    try {
        rc.value = std::stoll(std::string{value});
    } catch (...) {
        return std::nullopt;
    }
    return rc;
}

struct ExtractedReports
{
    std::optional<ReportCount> value;
    bool unsupported{false};
};

/** @brief Split `count.reports` out of the NQL filter, keeping `+`/`,` separators. */
[[nodiscard]] inline ExtractedReports extract_report_count(core::query::ParsedQuery& q)
{
    ExtractedReports result;
    for (auto it = q.filters.begin(); it != q.filters.end();) {
        if (it->first != "count.reports") { ++it; continue; }
        auto rc = report_count(it->second);
        if (!rc) { result.unsupported = true; return result; }
        result.value = rc;
        it = q.filters.erase(it);
    }
    if (!q.raw_filter.empty() && q.raw_filter.find("count.reports") != std::string::npos) {
        std::string rebuilt;
        std::size_t start = 0;
        char sep = 0;
        const auto flush = [&](std::size_t from, std::size_t to) {
            const auto clause = trim(q.raw_filter.substr(from, to - from));
            if (clause.empty()) return;
            if (clause.find("count.reports") == std::string_view::npos) {
                if (!rebuilt.empty() && sep != 0) rebuilt.push_back(sep);
                rebuilt += clause;
                return;
            }
            const auto colon = clause.find(':');
            auto rc = colon == std::string_view::npos
                          ? std::optional<ReportCount>{}
                          : report_count(clause.substr(colon + 1));
            if (!rc) { result.unsupported = true; return; }
            result.value = rc;
        };
        for (std::size_t i = 0; i < q.raw_filter.size(); ++i) {
            const char c = q.raw_filter[i];
            if (c == '+' || c == ',') {
                flush(start, i);
                sep = c;
                start = i + 1;
            }
        }
        flush(start, q.raw_filter.size());
        if (result.unsupported) return result;
        q.raw_filter = rebuilt;
        if (q.options.contains("filter")) q.options["filter"] = q.raw_filter;
    }
    return result;
}

[[nodiscard]] inline long long reports_of(const blog::api::RecordDto& row)
{
    const auto it = row.find("count");
    if (it == row.end()) return 0;
    const auto pos = it->second.find("\"reports\":");
    if (pos == std::string::npos) return 0;
    std::size_t i = pos + 10;
    while (i < it->second.size() && std::isspace(static_cast<unsigned char>(it->second[i]))) ++i;
    const auto begin = i;
    while (i < it->second.size() && ((it->second[i] >= '0' && it->second[i] <= '9') ||
                                      it->second[i] == '-')) ++i;
    if (i == begin) return 0;
    try { return std::stoll(it->second.substr(begin, i - begin)); } catch (...) { return 0; }
}

[[nodiscard]] inline bool report_match(long long actual, const ReportCount& rc)
{
    if (rc.op == ">") return actual > rc.value;
    if (rc.op == ">=") return actual >= rc.value;
    if (rc.op == "<") return actual < rc.value;
    if (rc.op == "<=") return actual <= rc.value;
    if (rc.op == "!=") return actual != rc.value;
    return actual == rc.value;
}

} // namespace detail

class ExecuteStep {
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"comments_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        auto q = core::query::parse(in.get_path(), detail::kOptions);

        // Ghost adminBrowseAll: only 'false'/false turn include_nested off.
        const auto nested = q.options.find("include_nested");
        const bool include_nested = !(nested != q.options.end() && nested->second == "false");
        if (!include_nested) q.filters.emplace_back("parent_id", "null");

        // Ghost #extractReportCountFilter: unsupported operators are a 400.
        const auto extracted = detail::extract_report_count(q);
        if (extracted.unsupported) {
            return core::workflow::StepResponse<Output>{{{"status", "400"},
                {"body", blog::api::ApiError{}.json(
                     "Unsupported count.reports filter", "Bad Request")}}};
        }
        const auto& report_count = extracted.value;

        // adminBrowseAll default order is `created_at desc`.
        if (q.order.empty()) q.order = "created_at desc";

        const auto cleaned = local_mapping::clean(ctx.store().json_for("comments"), "comment");
        std::string wire{"["};
        bool first = true;
        for (auto record : blog::api::parse_array(cleaned)) {
            // The shared Comment wire fixture fabricates parent_id = own id for
            // top-level comments; normalize it to Ghost's null before filtering.
            const auto id_it = record.find("id");
            const auto parent_it = record.find("parent_id");
            if (id_it != record.end() && parent_it != record.end() &&
                id_it->second == parent_it->second) {
                parent_it->second = "null";
            }
            if (report_count && !detail::report_match(detail::reports_of(record), *report_count)) {
                continue;
            }
            if (!first) wire.push_back(',');
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
            return core::workflow::StepResponse<Output>{{{"status", "400"},
                {"body", blog::api::ApiError{}.json("Invalid Ghost query", "Bad Request")}}};
        }

        const auto limit = q.limit == 0 || q.limit == std::numeric_limits<std::size_t>::max()
                               ? (total == 0 ? std::size_t{15} : total)
                               : q.limit;
        const auto page = q.page == 0 ? std::size_t{1} : q.page;
        const auto pages = limit == 0 ? std::size_t{0} : (total + limit - 1) / limit;
        const std::string prev = page > 1 ? std::to_string(page - 1) : "null";
        const std::string next = page < pages ? std::to_string(page + 1) : "null";
        const auto meta = local_mapping::pagination(total, limit, page, pages, prev, next);

        return core::workflow::StepResponse<Output>{{{"status", "200"},
            {"body", local_mapping::wrap("comments", page_rows, "{\"pagination\":" + meta + "}")}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
    static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::comments::browse::workflow
