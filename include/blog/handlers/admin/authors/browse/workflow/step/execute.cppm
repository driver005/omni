export module blog_handlers:authors_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import interfaces;

export namespace blog::handlers::authors::browse::workflow {

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

// Shared author-parity helpers. The read steps import this partition so all three
// Ghost Content API routes use the same input guard and wire mapper.
namespace detail {

inline constexpr std::string_view kAllowedBrowseOptions[]{
    "include", "filter", "fields", "limit", "order", "page"};

inline constexpr std::string_view kAllowedReadOptions[]{
    "include", "filter", "fields"};

// Ghost's Content API author restriction set: api-filter-utils.ts
// CONTENT_API_RESTRICTED_FIELDS / _RELATIONS / USER_RESTRICTED_FIELDS plus the
// Admin API secret set. A restricted field may not be used in `filter` or
// `order` even though it is withheld from the wire model.
inline constexpr std::string_view kRestrictedAuthorFields[]{
    "password", "token", "gift_link_token", "secret",
    "email", "email_only", "html", "lexical", "locale", "mobiledoc",
    "newsletter_id", "plaintext", "email_recipient_filter", "published_by",
    "mobiledoc_revisions", "post_revisions", "email_subject",
    "show_title_and_feature_image",
    "status", "last_seen", "created_at", "updated_at", "roles", "roles_users",
    "visibility", "accessibility", "paid_subscription_started_notification",
    "paid_subscription_canceled_notification", "tour", "comment_notifications",
    "recommendation_notifications", "gift_subscription_notifications",
    "mention_notifications", "donation_notifications", "milestone_notifications",
    "free_member_signup_notification"};

[[nodiscard]] inline std::string_view trim(std::string_view s) noexcept
{
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.remove_prefix(1);
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.remove_suffix(1);
    return s;
}

[[nodiscard]] inline std::string lower(std::string_view s)
{
    std::string out;
    out.reserve(s.size());
    for (const unsigned char c : s) out.push_back(static_cast<char>(std::tolower(c)));
    return out;
}

/** @brief Ghost segment check: any dotted segment equal to a restricted field. */
[[nodiscard]] inline bool restricted_segment(std::string_view token)
{
    const std::string t = lower(token);
    std::size_t start = 0;
    for (;;) {
        const auto dot = t.find('.', start);
        const auto segment = t.substr(start, dot == std::string_view::npos
                                                   ? std::string_view::npos
                                                   : dot - start);
        if (!segment.empty()) {
            for (const auto restricted : kRestrictedAuthorFields) {
                if (segment == restricted) return true;
            }
        }
        if (dot == std::string_view::npos) break;
        start = dot + 1;
    }
    // bookshelf-order resolves an unqualified field by suffix, so a suffix of
    // a withheld column is restricted too (see isAdminApiRestrictedOrderField).
    if (!t.contains('.')) {
        for (const auto restricted : kRestrictedAuthorFields) {
            if (restricted.ends_with(t)) return true;
        }
    }
    return false;
}

/** @brief True when an NQL clause / order clause targets a withheld author field. */
[[nodiscard]] inline bool restricted_clause(std::string_view clause)
{
    if (core::query::has_restricted_field(clause)) return true;
    const auto colon = clause.find(':');
    auto field = trim(colon == std::string_view::npos ? clause : clause.substr(0, colon));
    while (!field.empty() && (field.front() == '-' || field.front() == '!')) field.remove_prefix(1);
    const auto ws = field.find_first_of(" \t");
    if (ws != std::string_view::npos) field = trim(field.substr(0, ws));
    return !field.empty() && restricted_segment(field);
}

/** @brief Drop restricted clauses while preserving Ghost's `,` (OR) / `+` (AND) separators. */
[[nodiscard]] inline std::string strip_restricted(std::string_view nql)
{
    std::string out;
    std::size_t start = 0;
    int depth = 0;
    char quote = 0;
    bool escaped = false;
    char sep = 0;

    const auto flush = [&](std::size_t from, std::size_t to) {
        const auto clause = trim(nql.substr(from, to - from));
        if (clause.empty() || restricted_clause(clause)) return;
        if (!out.empty() && sep != 0) out.push_back(sep);
        out += clause;
    };

    for (std::size_t i = 0; i < nql.size(); ++i) {
        const char c = nql[i];
        if (quote != 0) {
            if (escaped) escaped = false;
            else if (c == '\\') escaped = true;
            else if (c == quote) quote = 0;
            continue;
        }
        if (c == '\'' || c == '"') { quote = c; continue; }
        if (c == '[') { ++depth; continue; }
        if (c == ']') { if (depth > 0) --depth; continue; }
        if (depth == 0 && (c == ',' || c == '+')) {
            flush(start, i);
            sep = c;
            start = i + 1;
        }
    }
    flush(start, nql.size());
    return out;
}

/** @brief Apply Ghost's mongoTransformer + restricted-order guard to a parsed query. */
inline void sanitize(core::query::ParsedQuery& q)
{
    q.raw_filter = strip_restricted(q.raw_filter);
    q.order = strip_restricted(q.order);
    std::erase_if(q.filters, [](const auto& pair) { return restricted_clause(pair.first + ":" + pair.second); });
    // Keep the allow-listed option mirror consistent with the sanitized surfaces.
    if (q.options.contains("filter")) q.options["filter"] = q.raw_filter;
    if (q.options.contains("order")) q.options["order"] = q.order;
}

/** @brief Ghost input/authors.js#setDefaultOrder: slug-list autoOrder or `name asc`. */
[[nodiscard]] inline bool slug_filter_order(std::string_view filter, std::vector<std::string>& slugs)
{
    const auto key = filter.find("slug:");
    if (key == std::string_view::npos) return false;
    auto list = trim(filter.substr(key + 5));
    if (list.size() < 2 || list.front() != '[' || list.back() != ']') return false;
    list = trim(list.substr(1, list.size() - 2));
    for (auto part : list | std::views::split(',')) {
        const std::string text{std::ranges::begin(part), std::ranges::end(part)};
        auto item = std::string{trim(text)};
        if (item.size() >= 2 && ((item.front() == '"' && item.back() == '"') ||
                                 (item.front() == '\'' && item.back() == '\''))) {
            item = item.substr(1, item.size() - 2);
        }
        if (!item.empty()) slugs.push_back(std::move(item));
    }
    return !slugs.empty();
}

[[nodiscard]] inline std::string unquote(std::string_view raw)
{
    auto s = trim(raw);
    if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') ||
                          (s.front() == '\'' && s.back() == '\''))) {
        s = s.substr(1, s.size() - 2);
    }
    return std::string{s};
}

[[nodiscard]] inline std::string author_url(std::string_view scheme,
                                            std::string_view authority,
                                            std::string_view slug)
{
    const std::string s = scheme.empty() ? std::string{"http"} : std::string{scheme};
    const std::string a = authority.empty() ? std::string{"127.0.0.1:2369"} : std::string{authority};
    return s + "://" + a + "/author/" + std::string{slug} + "/";
}

/** @brief Apply the Ghost author mapper + clean.author allow-list to raw store rows. */
[[nodiscard]] inline std::string author_rows(std::string_view rows_json,
                                             bool with_count,
                                             std::size_t post_count,
                                             std::string_view scheme,
                                             std::string_view authority)
{
    const auto records = blog::api::parse_array(rows_json);
    std::string out{"["};
    bool first_row = true;
    for (const auto& record : records) {
        if (!first_row) out.push_back(',');
        first_row = false;

        std::string slug;
        if (const auto it = record.find("slug"); it != record.end()) slug = unquote(it->second);
        const auto raw = [&](std::string_view key) -> std::string {
            const auto it = record.find(std::string{key});
            return it == record.end() ? std::string{"null"} : it->second;
        };

        std::string object{"{"};
        bool first_field = true;
        const auto put = [&](std::string_view key, std::string value) {
            if (!first_field) object.push_back(',');
            first_field = false;
            object += '"' + std::string{key} + "\":" + value;
        };

        put("id", raw("id"));
        put("name", raw("name"));
        put("slug", raw("slug"));
        put("profile_image", "null");
        put("cover_image", "null");
        std::string bio = raw("bio");
        if (bio == "\"\"" || bio.empty()) bio = "null";
        put("bio", bio);
        put("website", "null");
        put("location", "null");
        put("facebook", "null");
        put("twitter", "null");
        put("threads", "null");
        put("bluesky", "null");
        put("mastodon", "null");
        put("tiktok", "null");
        put("youtube", "null");
        put("instagram", "null");
        put("linkedin", "null");
        put("meta_title", "null");
        put("meta_description", "null");
        put("url", '"' + author_url(scheme, authority, slug) + '"');
        if (with_count) put("count", "{\"posts\":" + std::to_string(post_count) + '}');
        object.push_back('}');
        out += object;
    }
    out.push_back(']');
    return local_mapping::clean(out, "author");
}

/** @brief blog_handlers:mapping quotes bare link values; Ghost emits numeric pages. */
[[nodiscard]] inline std::string ghost_pagination_links(std::string meta)
{
    for (const std::string_view key : {"prev", "next"}) {
        const std::string open = "\"" + std::string{key} + "\":\"";
        std::size_t pos = 0;
        while ((pos = meta.find(open, pos)) != std::string::npos) {
            const auto begin = pos + open.size();
            const auto end = meta.find('"', begin);
            if (end == std::string::npos) break;
            const auto digits = meta.substr(begin, end - begin);
            if (digits.empty() ||
                !std::ranges::all_of(digits, [](unsigned char c) { return std::isdigit(c) != 0; })) {
                pos = end + 1;
                continue;
            }
            meta.erase(end, 1);
            meta.erase(begin - 1, 1);
            pos = begin - 1 + digits.size();
        }
    }
    return meta;
}

} // namespace detail

class ExecuteStep {
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"authors_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        using namespace detail;

        const auto path = std::string{in.get_path()};
        const auto q = core::query::parse(path, kAllowedBrowseOptions);

        // Ghost validation.options.include.values = ['count.posts'].
        for (const auto& include : q.include) {
            if (include != "count.posts") {
                return core::workflow::StepResponse<Output>{
                    {{"status", "400"},
                     {"body", blog::api::ApiError{}.validation(
                          "Invalid include", "Bad Request", "include", "ValidationError")}}};
            }
        }

        auto query = q;
        sanitize(query);

        // Ghost setDefaultOrder(): slug-list autoOrder, otherwise `name asc`.
        std::vector<std::string> slugs;
        bool auto_order = false;
        if (query.order.empty() && slug_filter_order(query.raw_filter, slugs)) {
            auto_order = true;
        } else if (query.order.empty()) {
            query.order = "name asc";
        }

        // One store read: all authors, no in-store pagination (Ghost slices once in fetchPage).
        auto records = blog::api::parse_array(ctx.store().json_for("authors"));
        if (auto_order) {
            const auto rank = [&](const blog::api::RecordDto& record) {
                const auto it = record.find("slug");
                if (it == record.end()) return slugs.size();
                const auto found = std::ranges::find(slugs, unquote(it->second));
                return found == slugs.end() ? slugs.size()
                                            : static_cast<std::size_t>(found - slugs.begin());
            };
            std::stable_sort(records.begin(), records.end(),
                [&](const blog::api::RecordDto& a, const blog::api::RecordDto& b) {
                    return rank(a) < rank(b);
                });
            // core::query must not re-sort; the slugs are already in Ghost order.
            query.order.clear();
        }

        std::string raw_array{"["};
        for (std::size_t i = 0; i < records.size(); ++i) {
            if (i) raw_array.push_back(',');
            raw_array += blog::api::record_to_json(records[i]);
        }
        raw_array.push_back(']');

        const bool with_count = query.include.contains("count.posts");
        // The local store has no author->posts relation; Ghost's relation loader is
        // not available. Use the post count as the best in-store approximation.
        const std::size_t post_count = with_count ? ctx.store().total("posts") : 0;
        const auto wire = author_rows(raw_array, with_count, post_count,
                                      in.get_scheme(), in.get_authority());

        // core::query applies restricted-field guard, filter, order, fields and
        // pagination in one pass (default limit 15, as in Ghost).
        auto all_query = query;
        all_query.limit = std::numeric_limits<std::size_t>::max();
        all_query.page = 1;
        all_query.offset = 0;
        const auto filtered_all = core::query::apply(wire, all_query);
        const std::size_t total = blog::api::parse_array(filtered_all).size();
        const auto page_rows = core::query::apply(wire, query);

        const std::size_t limit = query.limit == 0 ? std::size_t{15}
                                : query.limit == std::numeric_limits<std::size_t>::max()
                                    ? total : query.limit;
        const std::size_t page = query.page == 0 ? std::size_t{1} : query.page;
        const std::size_t pages = limit ? (total + limit - 1) / limit : std::size_t{0};
        const std::string meta = ghost_pagination_links(local_mapping::pagination(
            total, limit, page, pages,
            page > 1 ? std::to_string(page - 1) : std::string{"null"},
            page < pages ? std::to_string(page + 1) : std::string{"null"}));

        return core::workflow::StepResponse<Output>{
            {{"status", "200"},
             {"body", local_mapping::wrap("authors", page_rows,
                                          "{\"pagination\":" + meta + "}")}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
    static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::authors::browse::workflow
