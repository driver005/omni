module;
#include <ctime>
export module blog_handlers:stripe_stripe_checkout_config_get_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_auth;
import core_serde;
import interfaces;
import :stripe;
import :outbox_shim;

// ---------------------------------------------------------------------------
// Shared strict-parity support for the four Stripe Admin API routes. Kept in
// this already-registered partition so no central BUILD / all.cppm edit is
// needed. The project's registered primitives are used throughout:
//   core::query     — NQL filter/pagination pipeline
//   core::auth::ghost + blog_handlers_middleware — Ghost permission service
//   blog::handlers::outbox (outbox_shim) — observable Stripe / action side effects
//   ctx.store()     — context-local persistence
// `blog_handlers:mapping` and `core_adapters` are not registered deps of this
// target; see the parity reports for the new-dependency needs.
// ---------------------------------------------------------------------------
export namespace blog::handlers::stripe::support {

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

using Rows = std::map<std::string, std::string>;

/** @brief Ghost `StripeCheckoutDesign` snake_case fields (models.ts:23-30). */
inline constexpr std::array<std::string_view, 4> kDesignFields{
    "button_color", "background_color", "border_style", "font_family"};

[[nodiscard]] inline std::string json_quote(std::string_view text)
{
    std::string out = "\"";
    for (const unsigned char c : text) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    out += std::format("\\u{:04x}", static_cast<unsigned>(c));
                } else {
                    out.push_back(static_cast<char>(c));
                }
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

[[nodiscard]] inline std::string field_text(const Rows& row, std::string_view key)
{
    const auto it = row.find(std::string{key});
    return it == row.end() ? std::string{} : it->second;
}

/** @brief Ghost output envelope (local_mapping::wrap equivalent, local until the
 *         mapping partitions are registered as a dependency). */
[[nodiscard]] inline std::string wrap(std::string_view resource, std::string_view rows)
{
    return "{\"" + std::string{resource} + "\":[" + std::string{rows} + "]}";
}

/** @brief Admin API principal. A bearer token with no explicit role is Ghost's
 *         Administrator; x-blog-* headers override the default. */
[[nodiscard]] inline core::auth::ghost::Principal principal_for(interfaces::io::IRequest& req)
{
    using core::auth::ghost::ApiContext;
    core::auth::ghost::Principal p;
    const auto context = req.find_header("x-blog-context");
    if (context == "CONTENT_API") p.context = ApiContext::CONTENT_API;
    else if (context == "MEMBERS_APP") p.context = ApiContext::MEMBERS_APP;
    else if (context == "INTERNAL") p.context = ApiContext::INTERNAL;
    else if (context == "SYSTEM") p.context = ApiContext::SYSTEM;
    else if (context == "ADMIN_API") p.context = ApiContext::ADMIN_API;
    else p.context = req.get_authorization().empty() ? ApiContext::ANONYMOUS
                                                      : ApiContext::ADMIN_API;

    p.role = std::string{req.find_header("x-blog-role")};
    if (p.role.empty() && p.context == ApiContext::ADMIN_API) p.role = "Administrator";
    for (auto part : std::string_view{req.find_header("x-blog-permissions")} |
                         std::views::split(',')) {
        const std::string scope{std::ranges::begin(part), std::ranges::end(part)};
        if (!scope.empty()) p.permissions.insert(scope);
    }
    const auto staff = req.find_header("x-blog-staff");
    p.staff = staff == "true" || staff == "1";
    if (!req.get_authorization().empty()) p.id = "bearer";
    return p;
}

/** @brief Ghost `permissions:{docName:'products', method:read|edit}` gate. */
[[nodiscard]] inline core::auth::ghost::AuthResult authorize(
    interfaces::io::IRequest& req, std::string_view method)
{
    core::auth::ghost::GhostAuthorizer authorizer;
    return authorizer.authorize(principal_for(req), true, "products", method);
}

/** @brief STRIPE_CONNECTED=0 models a disconnected Stripe account; all other
 *         values (including unset) model the connected fixture used by Ghost's
 *         e2e suite. Lets the 422 "Connect Stripe" branches be exercised. */
[[nodiscard]] inline bool stripe_connected()
{
    const char* value = std::getenv("STRIPE_CONNECTED");
    if (value == nullptr) return true;
    const std::string_view v{value};
    return !(v == "0" || v == "false" || v == "no");
}

[[nodiscard]] inline std::string env_or(std::string_view name, std::string_view fallback)
{
    const char* value = std::getenv(std::string{name}.c_str());
    return value == nullptr || *value == '\0' ? std::string{fallback} : std::string{value};
}

/**
 * @brief Record one outbound Stripe call in the context-local outbox. The
 *        payload matches core_adapters::OutboundCall::to_json(), so a host that
 *        registers the real adapter can consume it unchanged once the
 *        core_adapters dependency is wired.
 */
inline void outbound(std::string_view operation, std::string_view method,
                     std::string_view url, std::string_view request_body,
                     std::string_view response_body, int status)
{
    const bool ok = status >= 200 && status < 300;
    const auto payload = std::format(
        R"({{"adapter":"stripe","operation":{},"method":{},"url":{},"status":{},"ok":{},"error":{},"request_body":{},"response_body":{}}})",
        json_quote(operation), json_quote(method), json_quote(url), status,
        ok ? "true" : "false",
        ok ? std::string{"\"\""} : json_quote("Stripe upstream error"),
        json_quote(request_body), json_quote(response_body));
    blog::handlers::outbox::global_outbox().emit(
        blog::handlers::outbox::Event{"adapter.outbound", payload, std::string{operation}});
}

inline void domain_event(std::string_view type, std::string_view payload,
                         std::string_view aggregate_id)
{
    blog::handlers::outbox::global_outbox().emit(blog::handlers::outbox::Event{
        std::string{type}, std::string{payload}, std::string{aggregate_id}});
}

/** @brief Ghost's logging.warn shape for an unreadable Stripe design. */
inline void warn(std::string_view event_name)
{
    domain_event("logging.warn", "{\"event\":" + json_quote(event_name) + "}", "stripe");
}

/** @brief Decode a stored snake_case Stripe design into the Ghost wire shape
 *         ({customize:false} or {customize:true, <four snake fields>}). */
[[nodiscard]] inline std::string snake_design_response(std::string_view raw)
{
    if (raw.empty() || raw == "null" || raw == "{}") return R"({"customize":false})";
    const auto doc = core_serde::JsonCodec{}.decode<serde::Value>(raw);
    const auto* object = doc ? std::get_if<serde::Value::Object>(&doc->get()) : nullptr;
    if (object == nullptr) return R"({"customize":false})";

    std::string out = "{\"customize\":true";
    for (const auto field : kDesignFields) {
        for (const auto& [key, value] : *object) {
            if (key == field) {
                out += "," + json_quote(field) + ":" + core_serde::JsonCodec{}.encode(value);
                break;
            }
        }
    }
    out += "}";
    return out;
}

/** @brief Validate a submitted design (discriminated union on `customize`). */
[[nodiscard]] inline bool valid_design_input(const serde::Value& value, std::string& encoded)
{
    const auto* object = std::get_if<serde::Value::Object>(&value.get());
    if (object == nullptr) return false;
    const serde::Value* customize = nullptr;
    for (const auto& [key, field] : *object) {
        if (key == "customize") { customize = &field; break; }
    }
    if (customize == nullptr) return false;
    const auto flag = core_serde::JsonCodec{}.encode(*customize);
    if (flag == "false") { encoded = R"({"customize":false})"; return true; }
    if (flag != "true") return false;

    encoded = "{\"customize\":true";
    for (const auto field : kDesignFields) {
        const serde::Value* found = nullptr;
        for (const auto& [key, value] : *object) {
            if (key == field) { found = &value; break; }
        }
        if (found == nullptr) return false;
        const auto text = core_serde::JsonCodec{}.encode(*found);
        if (text.size() < 2 || text.front() != '"') return false;
        encoded += "," + json_quote(field) + ":" + text;
    }
    encoded += "}";
    return true;
}

/** @brief Read the single `slug=default` config row through core::query. */
[[nodiscard]] inline std::string config_design(blog::BlogContext& ctx)
{
    const auto rows = ctx.store().json_for("stripe_checkout_config", {});
    static constexpr std::array<std::string_view, 1> kFilter{{"filter"}};
    const auto q = core::query::parse("?filter[slug]=default", kFilter);
    auto filtered = rows;
    try {
        filtered = core::query::apply(rows, q, 15);
    } catch (...) { // NOLINT(bugprone-empty-catch) - restricted-field guard is not used here
        filtered = rows;
    }
    const auto records = blog::api::parse_array(filtered);
    if (records.empty()) return R"({"customize":false})";

    const auto design = field_text(records.front(), "design");
    const auto result = snake_design_response(design);
    if (!design.empty() && design != "null" && design != "{}" &&
        result == R"({"customize":false})") {
        warn("stripe_checkout.design.unreadable");
    }
    return result;
}

/** @brief `{"checkout_config":[{"design":...}]}` — Ghost config serializer. */
[[nodiscard]] inline std::string config_response(blog::BlogContext& ctx)
{
    return wrap("checkout_config", "{\"design\":" + config_design(ctx) + "}");
}

[[nodiscard]] inline std::string now_iso()
{
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm utc{};
#if defined(_WIN32)
    ::gmtime_s(&utc, &now);
#else
    ::gmtime_r(&now, &utc);
#endif
    char buffer[32]{};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &utc);
    return std::string{buffer};
}

} // namespace blog::handlers::stripe::support

export namespace blog::handlers::stripe::stripe_checkout_config_get::workflow {

class ExecuteStep {
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"stripe_stripe_checkout_config_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::stripe::support;
        auto& ctx = *s_ctx;

        // Ghost: permissions:{docName:'products',method:'read'} on config.read.
        const auto authorized = authorize(in, "read");
        if (!authorized.allowed) {
            return core::workflow::StepResponse<Output>{{{"status", std::to_string(authorized.status)},
                                                          {"body", authorized.error_json}}};
        }

        // Ghost config.read is uncached and makes no Stripe call; the single
        // `slug='default'` row is the authoritative source.
        return core::workflow::StepResponse<Output>{{{"status", "200"},
                                                      {"body", config_response(ctx)}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
    static blog::BlogContext* s_ctx;
    static blog::api::StripeHandler* s_stripe;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::StripeHandler* ExecuteStep::s_stripe{nullptr};

} // namespace blog::handlers::stripe::stripe_checkout_config_get::workflow
