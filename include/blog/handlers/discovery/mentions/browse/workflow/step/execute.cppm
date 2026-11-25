export module blog_handlers:mentions_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_auth;
import core_serde;
import :outbox_shim;
import interfaces;

// ---------------------------------------------------------------------------
// Shared Ghost-parity helpers for both mentions routes. They live in this
// already-registered partition (`:mentions_browse_step`) so no central
// BUILD/all.cppm wiring is needed — the receive step imports this partition.
// ---------------------------------------------------------------------------
export namespace blog::handlers::mentions::support {

using core::auth::ghost::ApiContext;

/** @brief Ghost controller options (`mentions.js:10`) plus framework options. */
inline constexpr std::array<std::string_view, 9> kBrowseOptions{
    "filter",
    "fields",
    "limit",
    "order",
    "page",
    "debug",
    "unique",
    "offset",
    "include"
};

/** @brief core_query's internal "no limit" sentinel. */
inline constexpr std::size_t kAll = std::numeric_limits<std::size_t>::max();

/** @brief Admin API principal. A bearer token with no explicit role is Ghost's
 *         Administrator; x-blog-* headers override the default (authAdminApi). */
[[nodiscard]] inline core::auth::ghost::Principal principal_for(interfaces::io::IRequest& req)
{
    core::auth::ghost::Principal p;
    const auto context = req.find_header("x-blog-context");
    if (context == "CONTENT_API") {
        p.context = ApiContext::CONTENT_API;
    } else if (context == "MEMBERS_APP") {
        p.context = ApiContext::MEMBERS_APP;
    } else if (context == "INTERNAL") {
        p.context = ApiContext::INTERNAL;
    } else if (context == "SYSTEM") {
        p.context = ApiContext::SYSTEM;
    } else if (context == "ADMIN_API") {
        p.context = ApiContext::ADMIN_API;
    } else {
        p.context = req.get_authorization().empty() ? ApiContext::ANONYMOUS : ApiContext::ADMIN_API;
    }

    p.role = std::string{req.find_header("x-blog-role")};
    if (p.role.empty() && p.context == ApiContext::ADMIN_API) {
        p.role = "Administrator";
    }
    for (auto part:
         std::string_view{req.find_header("x-blog-permissions")} | std::views::split(',')) {
        const std::string scope{std::ranges::begin(part), std::ranges::end(part)};
        if (!scope.empty()) {
            p.permissions.insert(scope);
        }
    }
    const auto staff = req.find_header("x-blog-staff");
    p.staff = staff == "true" || staff == "1";
    if (!req.get_authorization().empty()) {
        p.id = "bearer";
    }
    return p;
}

/** @brief Read one field out of a flat JSON record, unquoting JSON strings. */
[[nodiscard]] inline std::string field_string(const serde::Value& v, std::string_view key)
{
    const auto object = v.to_object();
    if (!object) {
        return {};
    }
    const auto field = object->get(std::string{key});
    if (!field) {
        return {};
    }
    const auto encoded = core_serde::JsonCodec{}.encode(*field);
    if (!encoded.empty() && encoded.front() == '"') {
        // JSON string: strip the surrounding quotes; the body never contains escapes
        // that matter for the URL/title comparisons performed here.
        return encoded.substr(1, encoded.size() >= 2 ? encoded.size() - 2 : 0);
    }
    return encoded;
}

[[nodiscard]] inline std::string row_field(std::string_view json, std::string_view key)
{
    try {
        if (auto value = core_serde::JsonCodec{}.decode<serde::Value>(json)) {
            return field_string(*value, key);
        }
    } catch (...) {
    }
    return {};
}

/** @brief Ghost NQL regex starts-with value: strip `~^` and matching quotes. */
[[nodiscard]] inline std::string regex_prefix(std::string_view value)
{
    auto text = value;
    if (text.starts_with("~^")) {
        text.remove_prefix(2);
    }
    if (text.size() >= 2 && text.front() == '\'' && text.back() == '\'') {
        text = text.substr(1, text.size() - 2);
    } else if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
        text = text.substr(1, text.size() - 2);
    }
    return std::string{text};
}

/** @brief input/mentions.ts: unconditionally append `source:-~^'https://brid.gy/'`. */
inline void force_bridgy_filter(core::query::ParsedQuery& q)
{
    constexpr std::string_view kBridgy = "source:-~^'https://brid.gy/'";
    if (q.raw_filter.empty()) {
        q.raw_filter = std::string{kBridgy};
    } else if (!q.raw_filter.contains("brid.gy")) {
        q.raw_filter += "+";
        q.raw_filter += kBridgy;
    }
    q.options["filter"] = q.raw_filter;
}

/** @brief mention-controller.js:62-89 Ghost browse defaults. */
inline void apply_browse_defaults(core::query::ParsedQuery& q)
{
    const auto order = q.options.find("order");
    if (order == q.options.end() || order->second != "created_at desc") {
        q.order = "created_at asc";
    } else {
        q.order = "created_at desc";
    }
    const auto page = q.options.find("page");
    if (page == q.options.end() || page->second.empty() || q.page == 0) {
        q.page = 1;
    }
    // Ghost treats an absent or `all` limit as unbounded.
    const auto limit = q.options.find("limit");
    if (limit == q.options.end() || limit->second == "all") {
        q.limit = kAll;
    }
}

/** @brief Sort by created_at (asc default, desc only for `created_at desc`) and,
 *         for `unique:true`, keep the highest id per source — the in-memory
 *         equivalent of the repository's `whereRaw` NOT EXISTS clause. */
[[nodiscard]] inline std::vector<blog::api::RecordDto>
order_and_dedupe(std::vector<blog::api::RecordDto> rows, const core::query::ParsedQuery& q)
{
    std::vector<blog::api::RecordDto> out;
    if (q.options.contains("unique") && q.options.at("unique") == "true") {
        std::map<std::string, blog::api::RecordDto> by_source;
        for (auto& row: rows) {
            const std::string source = row_field(blog::api::record_to_json(row), "source");
            const std::string id = row_field(blog::api::record_to_json(row), "id");
            const auto it = by_source.find(source);
            if (it == by_source.end()) {
                by_source.emplace(source, std::move(row));
                continue;
            }
            const std::string seen = row_field(blog::api::record_to_json(it->second), "id");
            const bool newer = (id.size() == seen.size()) ? id > seen : id.size() > seen.size();
            if (newer) {
                it->second = std::move(row);
            }
        }
        for (auto& [key, row]: by_source) {
            out.push_back(std::move(row));
        }
    } else {
        out = std::move(rows);
    }

    const bool desc = q.order == "created_at desc";
    std::stable_sort(
        out.begin(),
        out.end(),
        [desc](const auto& a, const auto& b)
        {
            const auto ta = row_field(blog::api::record_to_json(a), "created_at");
            const auto tb = row_field(blog::api::record_to_json(b), "created_at");
            return desc ? ta > tb : ta < tb;
        }
    );
    return out;
}

/** @brief Drop mentions whose source matches the forced brid.gy exclusion. */
[[nodiscard]] inline std::vector<blog::api::RecordDto>
drop_bridgy(std::vector<blog::api::RecordDto> rows)
{
    std::vector<blog::api::RecordDto> out;
    out.reserve(rows.size());
    for (auto& row: rows) {
        const auto source = row_field(blog::api::record_to_json(row), "source");
        if (source.starts_with("https://brid.gy/")) {
            continue;
        }
        out.push_back(std::move(row));
    }
    return out;
}

/** @brief Map a stored mention row to the Ghost output mapper field set
 *         (`output/mappers/mentions.ts:3-18`). Internal fields are temporary:
 *         `created_at` supports filtering/ordering, `resource_*` supports the
 *         resource enrichment; all are stripped by `finish_row`. */
[[nodiscard]] inline blog::api::RecordDto mention_row(const blog::api::RecordDto& raw)
{
    const std::string json = blog::api::record_to_json(raw);
    blog::api::RecordDto row;
    const auto string_or_null = [&](std::string_view key)
    {
        const auto it = raw.find(std::string{key});
        if (it == raw.end() || it->second == "null" || it->second.empty()) {
            return std::string{"null"};
        }
        return it->second;
    };
    row["id"] = string_or_null("id");
    row["source"] = string_or_null("source");
    row["target"] = string_or_null("target");
    row["timestamp"] = string_or_null("created_at");
    row["payload"] = string_or_null("payload");
    row["resource"] = "null";
    row["source_title"] = string_or_null("source_title");
    row["source_site_title"] = string_or_null("source_site_title");
    row["source_excerpt"] = string_or_null("source_excerpt");
    row["source_author"] = string_or_null("source_author");
    row["source_favicon"] = string_or_null("source_favicon");
    row["source_featured_image"] = string_or_null("source_featured_image");
    row["verified"] = string_or_null("verified");
    // Hidden helpers used before serialization.
    row["created_at"] = string_or_null("created_at");
    const auto type = raw.find("resource_type");
    const auto id = raw.find("resource_id");
    row["resource_type"] = type == raw.end() ? std::string{"null"} : type->second;
    row["resource_id"] = id == raw.end() ? std::string{"null"} : id->second;
    return row;
}

/** @brief Attach the resolved post/page/tag resource, then strip hidden fields. */
[[nodiscard]] inline blog::api::RecordDto
finish_row(blog::BlogContext& ctx, const blog::api::RecordDto& row)
{
    blog::api::RecordDto out = row;
    const auto type = row_field(blog::api::record_to_json(row), "resource_type");
    const auto id = row_field(blog::api::record_to_json(row), "resource_id");
    if (!type.empty() && type != "null" && !id.empty() && id != "null") {
        if (const auto resource = ctx.store().find_by_resource(type, id)) {
            out["resource"] = *resource;
        }
    }
    out.erase("created_at");
    out.erase("resource_type");
    out.erase("resource_id");
    return out;
}

/** @brief JSON array from a vector of records. */
[[nodiscard]] inline std::string records_json(const std::vector<blog::api::RecordDto>& rows)
{
    std::string out{"["};
    for (std::size_t i = 0; i < rows.size(); ++i) {
        if (i) {
            out += ',';
        }
        out += blog::api::record_to_json(rows[i]);
    }
    out += ']';
    return out;
}

/** @brief Ghost meta.pagination: `limit` stays the string "all" for an
 *         unbounded browse; prev/next are numbers or null. */
[[nodiscard]] inline std::string
pagination_meta(std::size_t total, std::size_t limit, std::size_t page)
{
    const std::string limit_json = limit == kAll ? std::string{"\"all\""} : std::to_string(limit);
    const std::size_t shown = limit == kAll ? (total == 0 ? 1 : total) : limit;
    const std::size_t pages = shown == 0 ? 1 : (total + shown - 1) / shown;
    const std::string prev = page > 1 ? std::to_string(page - 1) : "null";
    const std::string next = page < pages ? std::to_string(page + 1) : "null";
    return std::format(
        R"({{"limit":{},"page":{},"pages":{},"prev":{},"next":{},"total":{}}})",
        limit_json,
        page,
        pages,
        prev,
        next,
        total
    );
}

/** @brief Transactional side-effect record through the shared outbox partition. */
inline void emit_event(std::string type, std::string payload, std::string aggregate_id)
{
    auto& outbox = blog::handlers::outbox::global_outbox();
    outbox.begin_transaction();
    outbox.emit(
        blog::handlers::outbox::Event{
            .type = std::move(type),
            .payload = std::move(payload),
            .aggregate_id = std::move(aggregate_id)
        }
    );
    outbox.commit();
}

} // namespace blog::handlers::mentions::support

export namespace blog::handlers::mentions::browse::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"mentions_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::mentions::support;
        auto& ctx = *s_ctx;

        // Ghost mw.authAdminApi + permissions:true / docName:'mentions'.
        core::auth::ghost::GhostAuthorizer authorizer;
        const auto authorized = authorizer.authorize(principal_for(in), true, "mentions", "browse");
        if (!authorized.allowed) {
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(authorized.status)}, {"body", authorized.error_json}}
            };
        }

        // Ghost input pipeline: allow-listed options, restricted-field guard, NQL,
        // fields projection and paging all come from core::query.
        auto q = core::query::parse(in.get_path(), kBrowseOptions);
        force_bridgy_filter(q);
        apply_browse_defaults(q);

        std::vector<blog::api::RecordDto> rows;
        try {
            rows = blog::api::parse_array(ctx.store().resources().list("mentions"));
        } catch (...) {
        }
        rows = drop_bridgy(std::move(rows));
        rows = order_and_dedupe(std::move(rows), q);

        std::vector<blog::api::RecordDto> mapped;
        mapped.reserve(rows.size());
        for (const auto& raw: rows) {
            mapped.push_back(mention_row(raw));
        }
        const std::string normalized = records_json(mapped);

        // Apply Ghost filter/fields/page/limit on the mapped rows (created_at is
        // still present here so NQL on the raw column keeps working).
        std::string page_json = "[]", meta{"{}"};
        try {
            page_json = core::query::apply(normalized, q, support::kAll);
            const auto total_json = core::query::run(normalized, q, support::kAll);
            if (auto value = core_serde::JsonCodec{}.decode<serde::Value>(total_json)) {
                if (const auto object = value->to_object()) {
                    if (const auto m = object->get("meta")) {
                        meta = core_serde::JsonCodec{}.encode(*m);
                    }
                }
            }
        } catch (const std::exception&) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}.validation(
                      "Restricted field used in Ghost query",
                      "Validation Error",
                      "filter"
                  )}}
            };
        }

        std::size_t total = 0;
        if (auto value = core_serde::JsonCodec{}.decode<serde::Value>(meta)) {
            if (const auto object = value->to_object()) {
                if (const auto p = object->get("pagination")) {
                    if (const auto object2 = p->to_object()) {
                        if (const auto t = object2->get("total")) {
                            try {
                                total = std::stoull(core_serde::JsonCodec{}.encode(*t));
                            } catch (...) {
                            }
                        }
                    }
                }
            }
        }

        std::vector<blog::api::RecordDto> page_rows;
        try {
            page_rows = blog::api::parse_array(page_json);
        } catch (...) {
        }
        std::vector<blog::api::RecordDto> finished;
        finished.reserve(page_rows.size());
        for (const auto& row: page_rows) {
            finished.push_back(finish_row(ctx, row));
        }

        const std::string body =
            "{\"mentions\":" + records_json(finished) +
            ",\"meta\":{\"pagination\":" + pagination_meta(total, q.limit, q.page) + "}}";
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::mentions::browse::workflow
