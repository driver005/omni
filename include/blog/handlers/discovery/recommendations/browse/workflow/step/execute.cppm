export module blog_handlers:recommendations_browse_step;
import :domain_all;
import :outbox_shim;
import std;
import core_workflow;
import core_query;
import core_auth;
import core_serde;
import interfaces;

// ---------------------------------------------------------------------------
// Shared Ghost-parity helpers for every recommendations handler/step. Kept in
// this already-registered partition so no central BUILD/all.cppm wiring is
// needed. Other recommendation partitions `import :recommendations_browse_step`
// and `using namespace blog::handlers::recommendations::support;`.
// ---------------------------------------------------------------------------
export namespace blog::handlers::recommendations::support {

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
            R"({{"page":{},"limit":{},"total":{},"pages":{},"prev":{},"next":{}}})",
            page,
            limit,
            total,
            pages,
            prev.empty() ? std::string{"null"} : std::string{prev},
            next.empty() ? std::string{"null"} : std::string{next}
        );
    }
} // namespace local_mapping

using core::auth::ghost::ApiContext;
using Object = serde::Value::Object;
using Array = serde::Value::Array;

// Ghost controller options (recommendations.js:11,23,36,51,66,85;
// recommendations-public.js:11,23,42).
inline constexpr std::array<std::string_view, 8> kBrowseOptions{
    "limit",
    "page",
    "offset",
    "include",
    "withRelated",
    "filter",
    "order",
    "fields"
};
inline constexpr std::array<std::string_view, 3> kReadOptions{"id", "fields", "include"};
inline constexpr std::array<std::string_view, 4>
    kWriteOptions{"id", "fields", "include", "formats"};
inline constexpr std::array<std::string_view, 2> kIncludes{"count.clicks", "count.subscribers"};
inline constexpr std::array<std::string_view, 8> kOrderFields{
    "title",
    "description",
    "excerpt",
    "one_click_subscribe",
    "created_at",
    "updated_at",
    "count.clicks",
    "count.subscribers"
};
inline constexpr std::array<std::string_view, 7> kInputFields{
    "title",
    "url",
    "one_click_subscribe",
    "description",
    "excerpt",
    "featured_image",
    "favicon"
};

[[nodiscard]] inline bool in_list(std::string_view value, std::span<const std::string_view> list)
{
    return std::ranges::find(list, value) != list.end();
}

/**
 * @brief Build a ghost::Principal from x-blog-* headers. A bearer token with no
 *        explicit role is Ghost's Administrator; no token is an anonymous
 *        Content API reader (the public recommendations browse surface).
 */
[[nodiscard]] inline core::auth::ghost::Principal principal_for(interfaces::io::IRequest& req)
{
    core::auth::ghost::Principal p;
    const auto context = req.find_header("x-blog-context");
    if (context == "ADMIN_API") {
        p.context = ApiContext::ADMIN_API;
    } else if (context == "CONTENT_API") {
        p.context = ApiContext::CONTENT_API;
    } else if (context == "MEMBERS_APP") {
        p.context = ApiContext::MEMBERS_APP;
    } else if (context == "INTERNAL") {
        p.context = ApiContext::INTERNAL;
    } else if (context == "SYSTEM") {
        p.context = ApiContext::SYSTEM;
    } else {
        p.context =
            req.get_authorization().empty() ? ApiContext::CONTENT_API : ApiContext::ADMIN_API;
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

[[nodiscard]] inline bool is_public(const core::auth::ghost::Principal& p)
{
    return p.context == ApiContext::CONTENT_API;
}

/** @brief Ghost `permissions:true` / `{method:'add'}` via the shared authorizer.
 *         Members-app tracking is authorized by loadMemberSession (checked by
 *         member_id() on the required-member path), so it is admitted here. */
[[nodiscard]] inline core::auth::ghost::AuthResult
authorize(const core::auth::ghost::Principal& p, std::string_view method)
{
    if (p.context == ApiContext::MEMBERS_APP) {
        return core::auth::ghost::AuthResult{.allowed = true, .status = 200, .error_json = {}};
    }
    core::auth::ghost::GhostAuthorizer authorizer;
    return authorizer.authorize(p, true, "recommendations", method);
}

/** @brief Member id populated by loadMemberSession (optional for clicks). */
[[nodiscard]] inline std::string member_id(interfaces::io::IRequest& req)
{
    auto id = std::string{req.find_header("x-blog-member-id")};
    if (id.empty()) {
        id = std::string{req.find_header("x-blog-member")};
    }
    return id;
}

/** @brief `:id` is the last non-empty path segment (query and trailing `/` stripped). */
[[nodiscard]] inline std::string path_id(std::string_view path)
{
    auto view = path.substr(0, path.find('?'));
    while (view.size() > 1 && view.back() == '/') {
        view.remove_suffix(1);
    }
    const auto slash = view.find_last_of('/');
    return std::string{slash == std::string_view::npos ? view : view.substr(slash + 1)};
}

/** @brief core::query with Ghost's browse defaults: limit 5, order created_at desc. */
[[nodiscard]] inline core::query::ParsedQuery
ghost_query(std::string_view path, std::span<const std::string_view> options)
{
    auto q = core::query::parse(path, options);
    if (!q.options.contains("limit")) {
        q.limit = 5;
    }
    if (q.order.empty()) {
        q.order = "created_at desc";
    }
    return q;
}

[[nodiscard]] inline std::set<std::string> include_set(const core::query::ParsedQuery& q)
{
    std::set<std::string> out{q.include.begin(), q.include.end()};
    if (const auto it = q.options.find("withRelated"); it != q.options.end()) {
        for (auto part : std::string_view{it->std::views::split(second}, ','){)
            {
                const std::string token{std::ranges::begin(part), std::ranges::end(part)};
                if (!token.empty()) {
                    out.insert(token);
                }
            }
        }
    }
    return out;
}

[[nodiscard]] inline bool includes_ok(const core::query::ParsedQuery& q)
{
    for (const auto& token: include_set(q)) {
        if (!in_list(token, kIncludes)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline bool order_ok(const core::query::ParsedQuery& q)
{
    for (auto clause: std::views::split(std::string_view{q.order}, ',')) {
        const std::string text{std::ranges::begin(clause), std::ranges::end(clause)};
        const auto space = text.find_first_of(" \t");
        const auto field = space == std::string::npos ? text : text.substr(0, space);
        const auto dir =
            space == std::string::npos ? std::string{"desc"} : std::string{text.substr(space + 1)};
        if (!in_list(field, kOrderFields) || (dir != "asc" && dir != "desc")) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline bool bad_paging(const core::query::ParsedQuery& q)
{
    if (q.options.contains("limit") && q.limit == 0) {
        return true;
    }
    const auto it = q.options.find("page");
    return it != q.options.end() && it->second == "0";
}

[[nodiscard]] inline std::string
validation_error(std::string_view message, std::string_view context, std::string_view property)
{
    return blog::api::ApiError{}.validation(message, context, property);
}

[[nodiscard]] inline std::string not_found_error(std::string_view id)
{
    return blog::api::ApiError{}.json(
        "Recommendation with id " + std::string{id} + " not found",
        "Not Found"
    );
}

[[nodiscard]] inline std::string now_iso()
{
    using namespace std::chrono;
    const auto epoch_ms =
        duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    std::int64_t days = epoch_ms / 86'400'000;
    std::int64_t remainder = epoch_ms % 86'400'000;
    if (remainder < 0) {
        remainder += 86'400'000;
        --days;
    }
    const int hour = static_cast<int>(remainder / 3'600'000);
    const int minute = static_cast<int>((remainder / 60'000) % 60);
    const int second = static_cast<int>((remainder / 1'000) % 60);
    const int millis = static_cast<int>(remainder % 1'000);
    // Howard Hinnant's civil-from-days conversion (no <ctime> dependency).
    const std::int64_t z = days + 719'468;
    const std::int64_t era = (z >= 0 ? z : z - 146'096) / 146'097;
    const unsigned doe = static_cast<unsigned>(z - era * 146'097);
    const unsigned yoe = (doe - doe / 1'460 + doe / 36'524 - doe / 146'096) / 365;
    std::int64_t year = static_cast<std::int64_t>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    const unsigned day = doy - (153 * mp + 2) / 5 + 1;
    const unsigned month = mp + (mp < 10 ? 3 : -9);
    if (month <= 2) {
        ++year;
    }
    return std::format(
        "{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}.{:03d}Z",
        year,
        month,
        day,
        hour,
        minute,
        second,
        millis
    );
}

// ------------------------------- JSON helpers ------------------------------

[[nodiscard]] inline std::optional<serde::Value> decode(std::string_view text)
{
    return core_serde::JsonCodec{}.decode<serde::Value>(text);
}

[[nodiscard]] inline const serde::Value* value_for(const Object* o, std::string_view key)
{
    if (o == nullptr) {
        return nullptr;
    }
    for (const auto& [name, value]: *o) {
        if (std::string_view{name} == key) {
            return &value;
        }
    }
    return nullptr;
}

[[nodiscard]] inline const Object* as_object(const serde::Value* v)
{
    return v != nullptr ? std::get_if<Object>(&v->get()) : nullptr;
}

[[nodiscard]] inline std::string field_string(const Object* o, std::string_view key)
{
    const auto* v = value_for(o, key);
    if (v != nullptr) {
        if (const auto* s = std::get_if<std::string>(&v->get())) {
            return *s;
        }
    }
    return {};
}

[[nodiscard]] inline std::string
field_any(const Object* o, std::initializer_list<std::string_view> keys)
{
    for (const auto key: keys) {
        const auto value = field_string(o, key);
        if (!value.empty()) {
            return value;
        }
    }
    return {};
}

[[nodiscard]] inline std::string encoded_or_null(const serde::Value* v)
{
    return v != nullptr ? core_serde::JsonCodec{}.encode(*v) : std::string{"null"};
}

[[nodiscard]] inline std::string scalar_or_null(const Object* o, std::string_view key)
{
    return encoded_or_null(value_for(o, key));
}

[[nodiscard]] inline std::optional<std::vector<serde::Value>> decode_array(std::string_view text)
{
    const auto parsed = decode(text);
    if (!parsed) {
        return std::nullopt;
    }
    const auto* arr = std::get_if<Array>(&parsed->get());
    if (arr == nullptr) {
        return std::nullopt;
    }
    return std::vector<serde::Value>{arr->begin(), arr->end()};
}

[[nodiscard]] inline std::string encode_array(const std::vector<serde::Value>& values)
{
    std::string out = "[";
    bool first = true;
    for (const auto& v: values) {
        if (!first) {
            out += ',';
        }
        out += core_serde::JsonCodec{}.encode(v);
        first = false;
    }
    return out + "]";
}

/** @brief Unwrap Ghost's `frame.data.recommendations[0]` (bare `recommendations`
 *         arrays are accepted for legacy callers). */
[[nodiscard]] inline std::optional<serde::Value> unwrap_recommendation(std::string_view raw)
{
    const auto parsed = decode(raw);
    if (!parsed) {
        return std::nullopt;
    }
    const auto* top = as_object(&*parsed);
    if (top == nullptr) {
        return std::nullopt;
    }
    const auto* data = as_object(value_for(top, "data"));
    const auto* holder = value_for(data != nullptr ? data : top, "recommendations");
    if (holder == nullptr) {
        return std::nullopt;
    }
    const auto* arr = std::get_if<Array>(&holder->get());
    if (arr == nullptr || arr->empty()) {
        return std::nullopt;
    }
    return arr->front();
}

// --------------------------- Ghost getByUrl / getById ----------------------

/** @brief Ghost's URL normalization: protocol, www., query, hash and trailing
 *         slash are ignored; the host is case-insensitive (bookshelf-recommendation-
 *         repository.ts:131-147). */
[[nodiscard]] inline std::string normalize_url(std::string_view url)
{
    std::string s{url};
    if (const auto scheme = s.find("://"); scheme != std::string::npos) {
        s.erase(0, scheme + 3);
    }
    if (const auto hash = s.find('#'); hash != std::string::npos) {
        s.erase(hash);
    }
    if (const auto query = s.find('?'); query != std::string::npos) {
        s.erase(query);
    }
    const auto slash = s.find('/');
    std::string host = s.substr(0, slash);
    if (host.starts_with("www.")) {
        host.erase(0, 4);
    }
    std::ranges::transform(
        host,
        host.begin(),
        [](unsigned char c)
        {
            return static_cast<char>(std::tolower(c));
        }
    );
    std::string path = slash == std::string::npos ? std::string{} : s.substr(slash);
    while (path.size() > 1 && path.back() == '/') {
        path.pop_back();
    }
    return host + path;
}

[[nodiscard]] inline std::vector<serde::Value> all_rows(blog::BlogContext& ctx)
{
    return decode_array(ctx.store().resources().list("recommendations"))
        .value_or(std::vector<serde::Value>{});
}

[[nodiscard]] inline std::optional<serde::Value>
find_by_id(blog::BlogContext& ctx, std::string_view id)
{
    for (const auto& row: all_rows(ctx)) {
        const auto* o = as_object(&row);
        if (o != nullptr && field_string(o, "id") == std::string{id}) {
            return row;
        }
    }
    return std::nullopt;
}

[[nodiscard]] inline std::optional<serde::Value>
find_by_url(blog::BlogContext& ctx, std::string_view url)
{
    const auto wanted = normalize_url(url);
    for (const auto& row: all_rows(ctx)) {
        const auto* o = as_object(&row);
        if (o != nullptr && normalize_url(field_string(o, "url")) == wanted) {
            return row;
        }
    }
    return std::nullopt;
}

[[nodiscard]] inline std::string row_id(const serde::Value& row)
{
    return field_string(as_object(&row), "id");
}

// ----------------------------- count subqueries ----------------------------

[[nodiscard]] inline bool include_counts(const core::query::ParsedQuery& q)
{
    const auto includes = include_set(q);
    return includes.contains("count.clicks") || includes.contains("count.subscribers");
}

[[nodiscard]] inline std::size_t
event_count(blog::BlogContext& ctx, std::string_view resource, std::string_view recommendation_id)
{
    std::size_t count = 0;
    for (const auto& row: decode_array(ctx.store().resources().list(resource))
                              .value_or(std::vector<serde::Value>{})) {
        const auto* o = as_object(&row);
        if (o != nullptr &&
            field_string(o, "recommendation_id") == std::string{recommendation_id}) {
            ++count;
        }
    }
    return count;
}

// -------------------------- Ghost output serializer ------------------------

/** @brief recommendation-controller.ts#serialize field set + aliases + count. */
[[nodiscard]] inline std::string
serialize_row(const serde::Value& row, bool with_count, std::size_t clicks, std::size_t subscribers)
{
    const auto* o = as_object(&row);
    if (o == nullptr) {
        return "{}";
    }
    std::string out = "{";
    out += "\"id\":" + scalar_or_null(o, "id");
    out += ",\"title\":" + scalar_or_null(o, "title");
    out += ",\"description\":" + scalar_or_null(o, "description");
    out += ",\"excerpt\":" + scalar_or_null(o, "excerpt");
    const auto featured = value_for(o, "featured_image") != nullptr ? value_for(o, "featured_image")
                                                                    : value_for(o, "featuredImage");
    out += ",\"featured_image\":" + encoded_or_null(featured);
    out += ",\"favicon\":" + scalar_or_null(o, "favicon");
    out += ",\"url\":" + scalar_or_null(o, "url");
    const auto one_click = value_for(o, "one_click_subscribe") != nullptr
                               ? value_for(o, "one_click_subscribe")
                               : value_for(o, "oneClickSubscribe");
    out +=
        ",\"one_click_subscribe\":" +
        (one_click != nullptr ? core_serde::JsonCodec{}.encode(*one_click) : std::string{"false"});
    const auto created = value_for(o, "created_at") != nullptr ? value_for(o, "created_at")
                                                               : value_for(o, "createdAt");
    out += ",\"created_at\":" + encoded_or_null(created);
    const auto updated = value_for(o, "updated_at") != nullptr ? value_for(o, "updated_at")
                                                               : value_for(o, "updatedAt");
    out += ",\"updated_at\":" + encoded_or_null(updated);
    if (with_count) {
        out += std::format(R"(,"count":{{"clicks":{},"subscribers":{}}})", clicks, subscribers);
    }
    return out + "}";
}

[[nodiscard]] inline std::string
serialize_rows(blog::BlogContext& ctx, const std::vector<serde::Value>& rows, bool with_count)
{
    std::string out = "[";
    bool first = true;
    for (const auto& row: rows) {
        if (!first) {
            out += ',';
        }
        const auto id = row_id(row);
        out += serialize_row(
            row,
            with_count,
            with_count ? event_count(ctx, "recommendation_click_events", id) : 0,
            with_count ? event_count(ctx, "recommendation_subscribe_events", id) : 0
        );
        first = false;
    }
    return out + "]";
}

/** @brief Transient preview row for `check` misses (never inserted). */
[[nodiscard]] inline std::string preview_row(std::string_view url)
{
    return "{\"id\":null,\"title\":null,\"description\":null,\"excerpt\":null,"
           "\"featured_image\":null,\"favicon\":null,\"url\":" +
           core_serde::JsonCodec{}.encode(std::string{url}) +
           R"(,"one_click_subscribe":false,"created_at":null,"updated_at":null})";
}

/** @brief Materialize Ghost's AddRecommendation defaults on create. */
[[nodiscard]] inline std::string materialize_input(const serde::Value& inner)
{
    const auto* o = as_object(&inner);
    if (o == nullptr) {
        return "{}";
    }
    std::string out = "{";
    out += "\"title\":" + scalar_or_null(o, "title");
    out += ",\"url\":" + scalar_or_null(o, "url");
    const auto one_click = value_for(o, "one_click_subscribe") != nullptr
                               ? value_for(o, "one_click_subscribe")
                               : value_for(o, "oneClickSubscribe");
    out +=
        ",\"one_click_subscribe\":" +
        (one_click != nullptr ? core_serde::JsonCodec{}.encode(*one_click) : std::string{"false"});
    for (const auto field: {"description", "excerpt", "featured_image", "favicon"}) {
        out += ",\"" + std::string{field} + "\":" + scalar_or_null(o, field);
    }
    const auto stamp = core_serde::JsonCodec{}.encode(now_iso());
    out += ",\"created_at\":" + stamp;
    out += ",\"updated_at\":" + stamp;
    return out + "}";
}

[[nodiscard]] inline std::string merge_input(const Object* existing, const serde::Value& patch)
{
    auto merged = existing != nullptr ? *existing : Object{};
    const auto* incoming = as_object(&patch);
    if (incoming == nullptr) {
        return core_serde::JsonCodec{}.encode(merged);
    }
    for (const auto& [key, value]: *incoming) {
        auto it = std::ranges::find_if(
            merged,
            [&](const auto& member)
            {
                return member.first == key;
            }
        );
        if (it != merged.end()) {
            it->second = value;
        } else {
            merged.insert(key, value);
        }
    }
    return core_serde::JsonCodec{}.encode(merged);
}

/** @brief Keep only the seven typed fields Ghost's edit serializer accepts. */
[[nodiscard]] inline std::string filtered_patch(const serde::Value& patch)
{
    const auto* o = as_object(&patch);
    if (o == nullptr) {
        return "{}";
    }
    Object out;
    for (const auto& [key, value]: *o) {
        if (in_list(key, kInputFields)) {
            out.insert(key, value);
        }
    }
    return core_serde::JsonCodec{}.encode(out);
}

[[nodiscard]] inline std::string
pagination_meta(std::size_t total, const core::query::ParsedQuery& q)
{
    std::size_t limit = q.limit == 0 ? 5 : q.limit;
    if (limit == std::numeric_limits<std::size_t>::max()) {
        limit = total;
    }
    const std::size_t page = q.page == 0 ? 1 : q.page;
    const std::size_t pages = limit == 0 ? 0 : (total + limit - 1) / limit;
    const std::string prev = page > 1 ? std::to_string(page - 1) : "null";
    const std::string next = page < pages ? std::to_string(page + 1) : "null";
    return local_mapping::pagination(total, limit, page, pages, prev, next);
}

/** @brief Transactional side-effect event through the shared outbox. */
inline void emit_event(std::string_view type, std::string_view id, std::string_view payload)
{
    auto& outbox = blog::handlers::outbox::global_outbox();
    outbox.begin_transaction();
    outbox.emit(
        blog::handlers::outbox::Event{
            .type = std::string{type},
            .payload = std::string{payload},
            .aggregate_id = std::string{id}
        }
    );
    outbox.commit();
}

inline void emit_crud_side_effects(
    std::string_view action,
    std::string_view id,
    std::string_view url,
    bool recommendations_remain
)
{
    emit_event(
        "recommendation." + std::string{action},
        id,
        "{\"url\":" + core_serde::JsonCodec{}.encode(std::string{url}) + "}"
    );
    emit_event("recommendations_enabled.set", id, recommendations_remain ? "true" : "false");
    emit_event(
        "recommendation.mention",
        id,
        "{\"url\":" + core_serde::JsonCodec{}.encode(std::string{url}) +
            ",\"action\":" + core_serde::JsonCodec{}.encode(std::string{action}) + "}"
    );
}

} // namespace blog::handlers::recommendations::support

// ---------------------------------------------------------------------------
// GET /recommendations — admin `browse` and public content API `browse`.
// ---------------------------------------------------------------------------
export namespace blog::handlers::recommendations::browse::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"recommendations_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::recommendations::support;
        auto& ctx = *s_ctx;

        auto principal = principal_for(in);
        if (principal.context == ApiContext::ANONYMOUS) {
            principal.context = ApiContext::CONTENT_API;
        }
        const auto authorized = authorize(principal, "browse");
        if (!authorized.allowed) {
            return core::workflow::StepResponse<Output>{authorized.error_json};
        }

        const auto q = ghost_query(in.get_path(), kBrowseOptions);
        if (!includes_ok(q)) {
            return core::workflow::StepResponse<Output>{
                validation_error("Invalid include value", "Validation Error", "include")
            };
        }
        if (!order_ok(q)) {
            return core::workflow::StepResponse<Output>{
                validation_error("Invalid order value", "Validation Error", "order")
            };
        }
        if (bad_paging(q)) {
            return core::workflow::StepResponse<Output>{
                validation_error("Invalid page or limit", "Validation Error", "limit")
            };
        }

        // Correlated count(*) subqueries are emulated from the context-local event store.
        const bool with_count = include_counts(q);
        const auto rows = all_rows(ctx);
        auto serialized = serialize_rows(ctx, rows, with_count);

        // core::query handles NQL filter, fields projection and page/limit pagination.
        // count.clicks/count.subscribers order needs the synthesized nested field, so
        // those clauses are applied on the serialized rows before the shared pipeline.
        auto query = q;
        if (query.order.find("count.") != std::string::npos) {
            std::vector<serde::Value> sorted;
            for (const auto& row: rows) {
                sorted.push_back(row);
            }
            const auto field = query.order.starts_with("count.clicks") ? "clicks" : "subscribers";
            const bool asc = query.order.ends_with("asc");
            std::stable_sort(
                sorted.begin(),
                sorted.end(),
                [&](const auto& a, const auto& b)
                {
                    const auto av = event_count(
                        ctx,
                        std::string{"recommendation_"} +
                            (field == std::string{"clicks"} ? "click_events" : "subscribe_events"),
                        row_id(a)
                    );
                    const auto bv = event_count(
                        ctx,
                        std::string{"recommendation_"} +
                            (field == std::string{"clicks"} ? "click_events" : "subscribe_events"),
                        row_id(b)
                    );
                    return asc ? av < bv : av > bv;
                }
            );
            serialized = serialize_rows(ctx, sorted, with_count);
            query.order.clear();
        }

        std::string data = "[]";
        try {
            const auto run = core::query::apply(serialized, query, 5);
            data = run;
        } catch (const std::exception&) {
            return core::workflow::StepResponse<Output>{validation_error(
                "Restricted field used in Ghost query",
                "Validation Error",
                "filter"
            )};
        }

        const auto meta = "{\"pagination\":" + support::pagination_meta(rows.size(), q) + "}";
        return core::workflow::StepResponse<Output>{local_mapping::wrap("data", data, meta)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::recommendations::browse::workflow

// ---------------------------------------------------------------------------
// POST /members/api/recommendations/:id/clicked — trackClicked (204).
// ---------------------------------------------------------------------------
export namespace blog::handlers::recommendations::track_clicked::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"recommendations_track_clicked_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::recommendations::support;
        auto& ctx = *s_ctx;

        const auto authorized = authorize(principal_for(in), "browse");
        if (!authorized.allowed) {
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(authorized.status)}, {"body", authorized.error_json}}
            };
        }

        const std::string id = path_id(in.get_path());
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body", validation_error("id is required", "Validation Error", "id")}}
            };
        }
        // readRecommendation(id) existence guard (unknown id -> 404).
        if (!find_by_id(ctx, id)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"}, {"body", not_found_error(id)}}
            };
        }

        const auto payload = materialize_click(id, member_id(in));
        ctx.store().resources().create("recommendation_click_events", payload);
        emit_event("recommendation.clicked", id, payload);
        return core::workflow::StepResponse<Output>{{{"status", "204"}, {"body", ""}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static std::string materialize_click(std::string_view id, std::string_view member)
    {
        return "{\"recommendation_id\":" + core_serde::JsonCodec{}.encode(std::string{id}) +
               ",\"member_id\":" +
               (member.empty() ? std::string{"null"}
                               : core_serde::JsonCodec{}.encode(std::string{member})) +
               ",\"created_at\":" + core_serde::JsonCodec{}.encode(support::now_iso()) + "}";
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::recommendations::track_clicked::workflow

// ---------------------------------------------------------------------------
// POST /members/api/recommendations/:id/subscribed — trackSubscribed (204).
// ---------------------------------------------------------------------------
export namespace blog::handlers::recommendations::track_subscribed::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"recommendations_track_subscribed_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::recommendations::support;
        auto& ctx = *s_ctx;

        const auto authorized = authorize(principal_for(in), "browse");
        if (!authorized.allowed) {
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(authorized.status)}, {"body", authorized.error_json}}
            };
        }

        const auto member = member_id(in);
        if (member.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "401"},
                 {"body", core::auth::ghost::unauthorized_error_json("Member not found")}}
            };
        }

        const std::string id = path_id(in.get_path());
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body", validation_error("id is required", "Validation Error", "id")}}
            };
        }

        const auto payload = "{\"recommendation_id\":" + core_serde::JsonCodec{}.encode(id) +
                             ",\"member_id\":" + core_serde::JsonCodec{}.encode(member) +
                             ",\"created_at\":" + core_serde::JsonCodec{}.encode(now_iso()) + "}";
        ctx.store().resources().create("recommendation_subscribe_events", payload);
        emit_event("recommendation.subscribed", id, payload);
        return core::workflow::StepResponse<Output>{{{"status", "204"}, {"body", ""}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::recommendations::track_subscribed::workflow
