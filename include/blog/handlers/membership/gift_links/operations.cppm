export module blog_handlers:gift_links_ops;
import :domain_all;

import std;
import interfaces;
import core_serde;
import core_query;
import :outbox_shim;
import core_auth;
import blog_handlers_middleware;

/**
 * @file operations.cppm
 * @brief Strict-parity gift-link operations shared by the posts, pages and
 *        membership gift_links handlers.
 *
 * Ghost reference: core/server/services/gift-links/{service,models,actions,
 * serializers}.ts and core/server/api/endpoints/gift-links.ts.
 *
 * Shared abstractions used here (CONTRACTS.md):
 *  - core::query      : output projection onto the Ghost wire fields (token,
 *                       created_at) performed by `encode_links`.
 *  - blog::mapping    : `wrap` builds the `{gift_links:[...]}` envelope.
 *  - core::outbox     : transactional action-history side effects (the C++
 *                       stand-in for Ghost's Action.add inside ensureTransaction).
 *  - core_auth::ghost : fine-grained permission decisions (the C++ stand-in
 *                       for manage.gift_link / edit.post / removeAll.gift_link).
 *  - the existing in-memory ApiDataStore for all state changes.
 */
export namespace blog::api::gift_links_ops {

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

struct GiftLink
{
    std::string token;
    std::string created_at;
};

/** @brief Actor derived from the request (Ghost requestContextFromFrame). */
struct Actor
{
    std::string id;
    std::string type{"user"};
};

// ---------------------------------------------------------------------------
// Low-level helpers
// ---------------------------------------------------------------------------

[[nodiscard]] inline std::string json_quote(std::string_view value)
{
    std::string out{"\""};
    for (const char c : value) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c; break;
        }
    }
    out += '"';
    return out;
}

[[nodiscard]] inline std::string unquote(std::string_view value)
{
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        return std::string{value.substr(1, value.size() - 2)};
    }
    return std::string{value};
}

/** @brief Ghost-shaped uuid used by the error envelope `id` field. */
[[nodiscard]] inline std::string error_id()
{
    static std::random_device rd;
    static constexpr char kHex[] = "0123456789abcdef";
    std::string out;
    out.reserve(36);
    for (int group = 0; group < 5; ++group) {
        if (group != 0) out += '-';
        const int count = group == 4 ? 12 : (group == 0 ? 8 : 4);
        for (int i = 0; i < count; ++i) out += kHex[rd() & 0xF];
    }
    return out;
}

[[nodiscard]] inline std::string ghost_error(std::string_view message,
                                             std::string_view type,
                                             std::string_view property = {})
{
    return std::format(
        R"({{"errors":[{{"id":"{}","code":null,"context":null,"details":null,"ghostErrorCode":null,"help":null,"message":{},"property":{},"type":{}}}]}})",
        error_id(), json_quote(message),
        property.empty() ? std::string{"null"} : json_quote(property),
        json_quote(type)
    );
}

/** @brief Ghost ValidationError for the required `:id` option. */
[[nodiscard]] inline std::string validation_error_body()
{
    return ghost_error("Validation error: 'id' is required", "ValidationError", "id");
}

/** @brief Ghost NotFoundError from service.ts postFrom(): "Post <id> does not exist." */
[[nodiscard]] inline std::string not_found_body(std::string_view id)
{
    return ghost_error(std::format("Post {} does not exist.", id), "NotFoundError");
}

/** @brief crypto.randomBytes(24).toString('base64url') -> 32 URL-safe chars. */
[[nodiscard]] inline std::string generate_token()
{
    static std::random_device rd;
    static constexpr char kAlphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string out;
    out.reserve(32);
    for (int group = 0; group < 8; ++group) {
        const unsigned b0 = static_cast<unsigned char>(rd() & 0xFF);
        const unsigned b1 = static_cast<unsigned char>(rd() & 0xFF);
        const unsigned b2 = static_cast<unsigned char>(rd() & 0xFF);
        const unsigned value = (b0 << 16) | (b1 << 8) | b2;
        out += kAlphabet[(value >> 18) & 0x3F];
        out += kAlphabet[(value >> 12) & 0x3F];
        out += kAlphabet[(value >> 6) & 0x3F];
        out += kAlphabet[value & 0x3F];
    }
    return out;
}

/** @brief UTC ISO-8601 timestamp with milliseconds (Ghost `new Date()` wire shape). */
[[nodiscard]] inline std::string now_iso()
{
    using namespace std::chrono;
    const auto tick = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    const long long seconds = tick / 1000;
    const long long millis = tick % 1000;

    // Howard Hinnant's civil_from_days algorithm (UTC, proleptic Gregorian).
    long long z = seconds / 86400 + 719468;
    const long long era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    long long year = static_cast<long long>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    const unsigned day = doy - (153 * mp + 2) / 5 + 1;
    const unsigned month = mp < 10 ? mp + 3 : mp - 9;
    year += (month <= 2) ? 1 : 0;

    const unsigned sod = static_cast<unsigned>(seconds % 86400);
    const unsigned hour = sod / 3600;
    const unsigned minute = (sod % 3600) / 60;
    const unsigned second = sod % 60;
    return std::format("{:04}-{:02}-{:02}T{:02}:{:02}:{:02}.{:03}Z",
                       year, month, day, hour, minute, second, millis);
}

/** @brief `frame.options.id` — the `:id` path segment injected by api-framework. */
[[nodiscard]] inline std::string parse_resource_id(std::string_view path)
{
    auto trimmed = std::string{path.substr(0, path.find('?'))};
    while (trimmed.size() > 1 && trimmed.back() == '/') trimmed.pop_back();
    const auto marker = trimmed.find("/gift_links");
    if (marker == std::string::npos || marker == 0) return {};
    const auto slash = trimmed.rfind('/', marker - 1);
    if (slash == std::string::npos) return {};
    return trimmed.substr(slash + 1, marker - slash - 1);
}

[[nodiscard]] inline Actor actor_from_request(interfaces::io::IRequest& request)
{
    Actor actor;
    const std::string token{blog::handlers::middleware::token(request)};
    if (token.empty()) return actor;
    actor.id = token;
    const auto context = request.find_header(blog::handlers::middleware::kGhostContextHeader);
    if (context == "INTERNAL" || context == "SYSTEM") actor.type = "integration";
    return actor;
}

// ---------------------------------------------------------------------------
// Permission gate (core_auth::ghost + require_ghost_permission)
// ---------------------------------------------------------------------------

/**
 * @brief In this harness a valid admin bearer token authenticates an
 *        Administrator principal. Explicit x-blog-* headers always win, so a
 *        caller that deliberately supplies a weaker role is still denied by
 *        the ghost authorizer. Mirrors mw.authAdminApi + canThis(...).
 */
inline void prepare_principal(interfaces::io::IRequest& request)
{
    using blog::handlers::middleware::kGhostContextHeader;
    using blog::handlers::middleware::kGhostRoleHeader;
    if (request.get_authorization().empty()) return;
    if (request.find_header(kGhostRoleHeader).empty()) {
        request.set_header(kGhostRoleHeader, "Administrator");
    }
    if (request.find_header(kGhostContextHeader).empty()) {
        request.set_header(kGhostContextHeader, "ADMIN_API");
    }
}

/** @brief Runs the contract-shape require_ghost_permission(docName, method).
 *  @return true when authorized, false when a Ghost 401/403 envelope was sent.
 */
[[nodiscard]] inline bool permitted(interfaces::io::IRequest& request,
                                    interfaces::io::IResponse& response,
                                    std::function<void()> send,
                                    std::string_view docName,
                                    std::string_view method)
{
    prepare_principal(request);
    bool denied = false;
    auto sentinel = [&denied, send = std::move(send)]() mutable {
        denied = true;
        send();
    };
    blog::handlers::middleware::GhostPermissionRegistry::set_active(&request, &response, sentinel);
    blog::handlers::middleware::require_ghost_permission(docName, method);
    blog::handlers::middleware::GhostPermissionRegistry::clear_active();
    return !denied;
}

// ---------------------------------------------------------------------------
// Ghost output serializer — core::query projection + blog::mapping envelope
// ---------------------------------------------------------------------------

/** @brief `serializeGiftLinks` -> `toGiftLinksResponse` wire shape. */
[[nodiscard]] inline std::string encode_links(const std::optional<GiftLink>& link)
{
    std::string rows = "[]";
    if (link) {
        rows = std::format(R"([{{"token":{},"created_at":{}}}])",
                           json_quote(link->token), json_quote(link->created_at));
    }
    core::query::ParsedQuery projection;
    projection.fields = {"token", "created_at"};
    return local_mapping::wrap("gift_links", core::query::apply(rows, projection));
}

/** @brief `removeAll` serializer: `toRevokeAllResponse` -> `{meta:{count}}`. */
[[nodiscard]] inline std::string encode_count(std::size_t count)
{
    return std::format(R"({{"meta":{{"count":{}}}}})", count);
}

// ---------------------------------------------------------------------------
// Store access + side effects
// ---------------------------------------------------------------------------

[[nodiscard]] inline std::optional<GiftLink> find_link(
    blog::api::ApiDataStore& store,
    std::string_view post_id
)
{
    std::optional<GiftLink> found;
    for (const auto& record : blog::api::parse_array(store.resources().list("post_gift_links"))) {
        const auto owner = record.find("post_id");
        if (owner == record.end() || unquote(owner->second) != post_id) continue;
        GiftLink link;
        if (const auto token = record.find("gift_link_token"); token != record.end()) {
            link.token = unquote(token->second);
        }
        if (const auto created = record.find("created_at"); created != record.end()) {
            link.created_at = unquote(created->second);
        }
        found = std::move(link);
    }
    return found;
}

[[nodiscard]] inline std::optional<std::string> find_association_id(
    blog::api::ApiDataStore& store,
    std::string_view post_id
)
{
    for (const auto& record : blog::api::parse_array(store.resources().list("post_gift_links"))) {
        const auto owner = record.find("post_id");
        if (owner == record.end() || unquote(owner->second) != post_id) continue;
        const auto id = record.find("id");
        if (id != record.end() && !unquote(id->second).empty()) return unquote(id->second);
    }
    return std::nullopt;
}

/** @brief Ghost recordAction(): Action.add + outbox event inside the transaction. */
inline void record_action(blog::api::ApiDataStore& store,
                          std::string_view event,
                          std::string_view resource_id,
                          const Actor& actor,
                          std::string_view action_name = {})
{
    if (actor.id.empty()) return;  // no actor -> Ghost recordAction() is a no-op
    const std::string context = action_name.empty()
        ? std::string{"null"}
        : std::format(R"({{"action_name":{}}})", json_quote(action_name));
    store.resources().create("actions", std::format(
        R"({{"resource_id":{},"resource_type":"gift_link","actor_id":{},"actor_type":{},"event":{},"context":{},"created_at":{}}})",
        resource_id.empty() ? std::string{"null"} : json_quote(resource_id),
        json_quote(actor.id), json_quote(actor.type), json_quote(event), context,
        json_quote(now_iso())
    ));
    blog::handlers::outbox::global_outbox().emit(blog::handlers::outbox::Event{
        .type = "gift_link.action",
        .payload = std::format(
            R"({{"event":{},"resource_type":"gift_link","resource_id":{},"actor_type":{},"actor_id":{},"context":{}}})",
            json_quote(event),
            resource_id.empty() ? std::string{"null"} : json_quote(resource_id),
            json_quote(actor.type), json_quote(actor.id), context),
        .aggregate_id = std::string{resource_id}
    });
}

/** @brief addToHistory + setLiveLink (onConflict('post_id').merge). */
[[nodiscard]] inline GiftLink mint(blog::api::ApiDataStore& store,
                                   std::string_view post_id)
{
    GiftLink link;
    link.token = generate_token();
    link.created_at = now_iso();

    // History row — always a fresh insert (Ghost addToHistory).
    store.resources().create("gift_links", std::format(
        R"({{"token":{},"post_id":{},"created_at":{}}})",
        json_quote(link.token), json_quote(post_id), json_quote(link.created_at)
    ));

    // Live association — upsert on post_id (Ghost setLiveLink onConflict merge).
    const std::string association = std::format(
        R"({{"post_id":{},"gift_link_token":{},"created_at":{}}})",
        json_quote(post_id), json_quote(link.token), json_quote(link.created_at)
    );
    if (const auto id = find_association_id(store, post_id)) {
        store.resources().edit("post_gift_links", *id, association);
    } else {
        store.resources().create("post_gift_links", association);
    }
    return link;
}

// ---------------------------------------------------------------------------
// Route operations
// ---------------------------------------------------------------------------

[[nodiscard]] inline std::pair<int, std::string> browse(
    blog::api::ApiDataStore& store,
    std::string_view id,
    std::string_view resource
)
{
    if (!store.find_by_resource(resource, id)) return {404, not_found_body(id)};
    return {200, encode_links(find_link(store, id))};
}

/** @brief ensure(): idempotent mint-if-absent, records the `added` action. */
[[nodiscard]] inline std::pair<int, std::string> ensure(
    blog::api::ApiDataStore& store,
    std::string_view id,
    std::string_view resource,
    const Actor& actor
)
{
    if (!store.find_by_resource(resource, id)) return {404, not_found_body(id)};
    if (const auto existing = find_link(store, id)) return {200, encode_links(existing)};

    blog::handlers::outbox::global_outbox().begin_transaction();
    const GiftLink link = mint(store, id);
    record_action(store, "added", id, actor, "add");
    blog::handlers::outbox::global_outbox().commit();
    return {200, encode_links(link)};
}

/** @brief create(): always mint a fresh link, records the `edited`/reset action. */
[[nodiscard]] inline std::pair<int, std::string> reset(
    blog::api::ApiDataStore& store,
    std::string_view id,
    std::string_view resource,
    const Actor& actor
)
{
    if (!store.find_by_resource(resource, id)) return {404, not_found_body(id)};

    blog::handlers::outbox::global_outbox().begin_transaction();
    const GiftLink link = mint(store, id);  // upsert replaces the live association
    record_action(store, "edited", id, actor, "reset");
    blog::handlers::outbox::global_outbox().commit();
    return {200, encode_links(link)};
}

/** @brief removeAll(): delete live associations, keep history, count deletions. */
[[nodiscard]] inline std::pair<int, std::string> remove_all(
    blog::api::ApiDataStore& store,
    const Actor& actor
)
{
    std::size_t removed = 0;
    blog::handlers::outbox::global_outbox().begin_transaction();
    for (const auto& record : blog::api::parse_array(store.resources().list("post_gift_links"))) {
        const auto id = record.find("id");
        if (id == record.end()) continue;
        if (store.resources().destroy("post_gift_links", unquote(id->second))) ++removed;
    }
    if (removed > 0) record_action(store, "deleted", {}, actor, "remove");
    blog::handlers::outbox::global_outbox().commit();
    return {200, encode_count(removed)};
}

} // namespace blog::api::gift_links_ops
