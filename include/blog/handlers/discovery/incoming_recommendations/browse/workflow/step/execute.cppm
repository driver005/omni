export module blog_handlers:incoming_recommendations_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import interfaces;

export namespace blog::handlers::incoming_recommendations::browse::workflow {

// Local output helpers (the shared blog_handlers:mapping partitions are not a
// registered dependency of blog_handlers). Mirrors the other strict-parity
// browse steps in this tree.
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

inline constexpr std::size_t kDefaultLimit{5};

// Ghost: #getMentionFilter() -> source:~$'/.well-known/recommendations.json'
inline constexpr std::string_view kMentionFilter{"source:~$'/.well-known/recommendations.json'"};

/** @brief Ghost NQL `$` marks a case-sensitive literal. core_query's `~`
 *  (contains) is already case-insensitive; dropping the no-op marker keeps the
 *  same match for this fixed lowercase well-known path. */
[[nodiscard]] inline std::string mention_filter()
{
    std::string filter{kMentionFilter};
    if (const auto at = filter.find("$'"); at != std::string::npos) filter.erase(at, 1);
    return filter;
}

[[nodiscard]] inline std::string_view trim(std::string_view text)
{
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) text.remove_prefix(1);
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) text.remove_suffix(1);
    return text;
}

/** @brief All raw query pairs (used only for the route-specific integer guard). */
[[nodiscard]] inline std::map<std::string, std::string> query_params(std::string_view path)
{
    std::map<std::string, std::string> out;
    const auto q = path.find('?');
    if (q == std::string_view::npos) return out;
    for (auto part : std::views::split(path.substr(q + 1), '&')) {
        const std::string text{std::ranges::begin(part), std::ranges::end(part)};
        if (const auto eq = text.find('='); eq != std::string::npos) {
            out[text.substr(0, eq)] = text.substr(eq + 1);
        }
    }
    return out;
}

/** @brief Ghost UnsafeData.integer: parseInt(raw).toString() === raw. */
[[nodiscard]] inline bool ghost_integer(std::string_view raw)
{
    if (raw.empty()) return false;
    std::size_t i = (raw.front() == '-' || raw.front() == '+') ? 1 : 0;
    if (i == raw.size()) return false;
    if (raw.size() - i > 1 && raw[i] == '0') return false;
    for (; i < raw.size(); ++i) {
        if (raw[i] < '0' || raw[i] > '9') return false;
    }
    return true;
}

[[nodiscard]] inline std::string unquote(std::string_view raw)
{
    auto text = trim(raw);
    if (text.size() >= 2 && ((text.front() == '"' && text.back() == '"') ||
                             (text.front() == '\'' && text.back() == '\''))) {
        text = text.substr(1, text.size() - 2);
    }
    std::string out;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\\' && i + 1 < text.size()) ++i;
        out.push_back(text[i]);
    }
    return out;
}

[[nodiscard]] inline std::string raw_field(const blog::api::RecordDto& record, std::string_view key)
{
    if (const auto it = record.find(std::string{key}); it != record.end()) return it->second;
    return "null";
}

[[nodiscard]] inline bool is_null(std::string_view raw)
{
    return raw.empty() || raw == "null";
}

/** @brief mention.source.toString().replace(/\/.well-known\/recommendations\.json$/, '') */
[[nodiscard]] inline std::string strip_well_known(std::string_view url)
{
    constexpr std::string_view kSuffix{"/.well-known/recommendations.json"};
    if (url.ends_with(kSuffix)) url.remove_suffix(kSuffix.size());
    return std::string{url};
}

/** @brief new URL(...) in #mentionToIncomingRecommendation; bad URLs are dropped. */
[[nodiscard]] inline bool parseable_url(std::string_view url)
{
    return url.starts_with("http://") || url.starts_with("https://");
}

/** @brief Stored outgoing recommendation URLs, the in-store stand-in for
 *  recommendationService.readRecommendationByUrl(url). */
[[nodiscard]] inline std::set<std::string> outgoing_urls(blog::BlogContext& ctx)
{
    std::set<std::string> urls;
    for (const auto& record : blog::api::parse_array(ctx.store().json_for("recommendations", {}))) {
        const auto url = unquote(raw_field(record, "url"));
        if (!url.empty()) urls.insert(url);
    }
    return urls;
}

/** @brief Ghost #mentionToIncomingRecommendation + #serialize over a mentions page. */
[[nodiscard]] inline std::string to_incoming_recommendations(blog::BlogContext& ctx,
                                                             std::string_view mentions_json)
{
    const auto back_urls = outgoing_urls(ctx);
    std::string out{"["};
    bool first{true};
    for (const auto& mention : blog::api::parse_array(mentions_json)) {
        const auto url = strip_well_known(unquote(raw_field(mention, "source")));
        if (!parseable_url(url)) continue;  // Ghost logs and drops unparseable mentions
        const auto site_title = raw_field(mention, "source_site_title");
        const auto title = is_null(site_title) || unquote(site_title).empty()
                               ? raw_field(mention, "source_title") : site_title;
        if (!first) out.push_back(',');
        first = false;
        out += "{\"id\":" + raw_field(mention, "id")
             + ",\"title\":" + (is_null(title) ? std::string{"\"\""} : title)
             + ",\"excerpt\":" + raw_field(mention, "source_excerpt")
             + ",\"featured_image\":" + raw_field(mention, "source_featured_image")
             + ",\"favicon\":" + raw_field(mention, "source_favicon")
             + ",\"url\":\"" + url + "\""
             + ",\"recommending_back\":" + (back_urls.contains(url) ? "true" : "false")
             + "}";
    }
    out.push_back(']');
    return out;
}

} // namespace detail

class ExecuteStep {
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"incoming_recommendations_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        // Ghost declares options: ['limit', 'page'] only.
        constexpr std::array<std::string_view, 2> kOptions{"limit", "page"};
        auto q = core::query::parse(in.get_path(), kOptions);

        // Ghost reads page first, then limit, via UnsafeData.integer; a
        // non-integer value throws ValidationError -> HTTP 422.
        const auto raw_params = detail::query_params(in.get_path());
        for (const std::string_view key : {"page", "limit"}) {
            if (const auto it = raw_params.find(std::string{key}); it != raw_params.end()) {
                if (!detail::ghost_integer(it->second)) {
                    return core::workflow::StepResponse<Output>{{{"status", "422"},
                        {"body", blog::api::ApiError{}.validation(
                            std::string{key} + " must be an integer", "", "")}}};
                }
            }
        }

        // Ghost controller defaults: page = 1, limit = 5.
        if (!raw_params.contains("limit")) q.limit = detail::kDefaultLimit;
        if (!raw_params.contains("page")) q.page = 1;

        // Ghost: mentionsApi.listMentions({filter, page, limit}). The filter and
        // real page slicing are applied by core::query (NQL + Ghost pagination).
        const auto all_mentions = ctx.store().json_for("mentions", {});
        auto page_query = q;
        page_query.raw_filter = detail::mention_filter();
        page_query.filters.clear();
        page_query.fields.clear();
        page_query.include.clear();
        page_query.offset = 0;
        page_query.order.clear();
        const auto mention_page = core::query::apply(all_mentions, page_query, detail::kDefaultLimit);

        auto total_query = page_query;
        total_query.limit = std::numeric_limits<std::size_t>::max();
        total_query.page = 1;
        total_query.offset = 0;
        const auto filtered_all = core::query::apply(all_mentions, total_query, detail::kDefaultLimit);
        const std::size_t total = blog::api::parse_array(filtered_all).size();

        // Map each mention exactly as Ghost's #mentionToIncomingRecommendation.
        const auto rows = detail::to_incoming_recommendations(ctx, mention_page);

        const std::size_t limit = q.limit;
        const std::size_t page = q.page == 0 ? std::size_t{1} : q.page;
        const std::size_t pages = limit == 0 ? std::size_t{0} : (total + limit - 1) / limit;
        const std::string prev = page > 1 ? std::to_string(page - 1) : std::string{"null"};
        const std::string next = (page < pages) ? std::to_string(page + 1) : std::string{"null"};
        const std::string meta = "{\"pagination\":" +
            local_mapping::pagination(total, limit, page, pages, prev, next) + "}";
        const std::string body = local_mapping::wrap("data", local_mapping::clean(rows, "incoming_recommendation"), meta);
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
    static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::incoming_recommendations::browse::workflow
