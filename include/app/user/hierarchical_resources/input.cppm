export module app_user:hierarchical_resources_input;

import std;
import interfaces;
import core_behavior;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_store;

export namespace app::handlers::hierarchical_resources {

/**
 * @brief Parse/auth/path/query support for the hierarchical resources handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime.
 */
class HierarchicalResourcesInputSupport
{
public:
    explicit HierarchicalResourcesInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    /** @brief routes:22-26 mandatory_profile_id_from_header. */
    [[nodiscard]] std::optional<std::string>
    mandatory_profile_id_from_header(interfaces::io::IRequest& req) const
    {
        const auto header = ::app::http::HeaderReader{}.get(req, "x-profile-id");
        if (header.empty()) {
            return std::nullopt;
        }
        if (!valid_profile_id(header)) {
            return std::nullopt;
        }
        return header;
    }

    /** @brief routes:28-46 resolve_profile_from_header (scoped fetch = auth). */
    [[nodiscard]] std::optional<std::string>
    resolve_profile_from_header(std::string_view profile_id, std::string_view merchant_id) const
    {
        const auto profile = m_context.repository().find_profile_by_string(profile_id);
        if (!profile.has_value()) {
            return std::nullopt;
        }
        if (!merchant_id.empty() && profile->get_merchant_id() != merchant_id) {
            return std::nullopt;
        }
        return std::string{profile_id};
    }

    /** @brief JWTAndEmbeddedAuth + MerchantConnector Write/Read + allow flags. */
    [[nodiscard]] std::optional<core::error::AppError>
    authorize_request(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto base = m_context.auth_rules().rule_for(operation);
        const auto rule = ::app::auth::RouteAuthRuleBuilder{}
                              .scheme(base.scheme())
                              .permission(base.permission())
                              .allow_connected(true)
                              .allow_platform(true)
                              .id_source("x-profile-id")
                              .build();
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    [[nodiscard]] std::string
    resolve_merchant_id(interfaces::io::IRequest& req, const ::app::auth::AuthContext& auth) const
    {
        const auto header = ::app::http::merchant_id_from_header{}(req);
        return header.empty() ? auth.merchant_id() : header;
    }

    /** @brief Authoritative organization of the caller's merchant account. */
    [[nodiscard]] std::string resolve_scope_id(
        interfaces::io::IRequest& req,
        const ::app::auth::AuthContext& auth,
        std::string_view merchant_id
    ) const
    {
        if (!merchant_id.empty()) {
            const auto account =
                m_context.repository().find_merchant_account_by_string(merchant_id);
            if (account.has_value() && !account->get_organization_id().empty()) {
                return account->get_organization_id();
            }
        }
        const auto header_org = ::app::http::org_id_from_header{}(req);
        if (!header_org.empty()) {
            const auto organization =
                m_context.repository().find_organization_by_string(header_org);
            if (organization.has_value() && !organization->get_organization_id().empty()) {
                return organization->get_organization_id();
            }
            return header_org;
        }
        const auto header_org_long = ::app::http::HeaderReader{}.get(req, "x-organization-id");
        if (!header_org_long.empty()) {
            return header_org_long;
        }
        return auth.org_id();
    }

    /** @brief routes:48-71 parse_upload_certificate_body. */
    [[nodiscard]] core::behavior::Outcome<
        ::app::dto::hierarchical_resources::UploadCertificateRequest>
    parse_upload_certificate_body(interfaces::io::IRequest& req, std::string_view body) const
    {
        using Input = ::app::dto::hierarchical_resources::UploadCertificateRequest;
        const auto content_type = ::app::http::HeaderReader{}.get(req, "content-type");
        const bool is_json = content_type.starts_with("application/json");
        if (is_json) {
            auto parsed = serde::Ser::deserialize<Input>("application/json", body);
            if (!parsed) {
                return std::unexpected{core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "invalid JSON body: " + parsed.error()
                )};
            }
            return *parsed;
        }
        Input raw{};
        raw.set_certificate(base64_encode(body));
        return raw;
    }

    [[nodiscard]] bool valid_resource_type(std::string_view value) const
    {
        // common_enums::ResourceType has exactly one snake_case variant.
        return value == "apple_pay_certificate";
    }

    [[nodiscard]] bool valid_requestor_type(std::string_view value) const
    {
        // common_enums::ResourceRequestorType snake_case variants (accounts.rs:145-149).
        return value == "merchant_connector_account" || value == "profile" ||
               value == "merchant_account";
    }

    /** @brief Extract :resource_id next to the route's fixed sibling segment. */
    [[nodiscard]] std::optional<std::string>
    resource_id_from_path(interfaces::io::IRequest& req, std::string_view marker) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        for (std::size_t i{0}; i < segments.count(); ++i) {
            if (segments.at(i).value_or(std::string_view{}) != marker) {
                continue;
            }
            const std::size_t target =
                marker == "apple_pay_certificate" ? i + 1 : (i == 0 ? 0 : i - 1);
            if (target >= segments.count()) {
                break;
            }
            const auto candidate = segments.at(target);
            if (candidate.has_value() && valid_resource_id(*candidate)) {
                return std::string{*candidate};
            }
        }
        return std::nullopt;
    }

private:
    [[nodiscard]] bool valid_profile_id(std::string_view value) const
    {
        return !value.empty() && value.size() <= 36;
    }

    /** @brief web::Path<ResourceId> substitute: alphanumeric, length 6..36. */
    [[nodiscard]] bool valid_resource_id(std::string_view value) const
    {
        if (value.size() < 6 || value.size() > 36) {
            return false;
        }
        for (const char c: value) {
            const bool ok =
                (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
            if (!ok) {
                return false;
            }
        }
        return true;
    }

    /** @brief Request snapshot shared by every hierarchical_resources runtime method. */
[[nodiscard]] inline ::core::behavior::CallContext make_call_context(interfaces::io::IRequest& req)
{
    ::core::behavior::CallContext ctx{};
    const auto auth = ::app::auth::AuthContextBuilder{}(req);
    ctx.merchant_id = std::string{::app::http::merchant_id_from_header{}(req)};
    if (ctx.merchant_id.empty()) ctx.merchant_id = auth.merchant_id();
    ctx.org_id = std::string{::app::http::org_id_from_header{}(req)};
    if (ctx.org_id.empty()) ctx.org_id = auth.org_id();
    ctx.profile_id = auth.profile_id();
    if (const auto c = auth.claim(::app::auth::kClaimConnectedMerchantId)) ctx.connected_merchant_id = std::string{*c};
    if (const auto s = auth.claim(::app::auth::kClaimClientSecret)) ctx.client_secret = std::string{*s};
    const auto segments = ::app::http::RequestPath{}(req);
    for (std::size_t i{0}; i < segments.count(); ++i) ctx.path_segments.push_back(std::string{segments.at(i).value_or("")});
    const auto raw = req.get_path();
    if (const auto q = raw.find('?'); q != std::string_view::npos) {
        const std::string_view tail = raw.substr(q + 1);
        std::size_t start{0};
        while (start <= tail.size()) {
            const auto amp = tail.find('&', start);
            const auto end = amp == std::string_view::npos ? tail.size() : amp;
            const auto pair = tail.substr(start, end - start);
            const auto eq = pair.find('=');
            if (!pair.empty()) {
                ctx.query.insert_or_assign(
                    std::string{eq == std::string_view::npos ? pair : pair.substr(0, eq)},
                    std::string{eq == std::string_view::npos ? std::string_view{} : pair.substr(eq + 1)}
                );
            }
            if (amp == std::string_view::npos) break;
            start = amp + 1;
        }
    }
    constexpr std::string_view kHeaders[] = {
        "x-merchant-id", "x-org-id", "x-organization-id", "x-profile-id", "authorization",
        "x-client-secret", "x-api-key", "x-connected-merchant-id", "x-forwarded-for"
    };
    for (const auto name: kHeaders) {
        const auto value = req.find_header(name);
        if (!value.empty()) ctx.headers.insert_or_assign(std::string{name}, std::string{value});
    }
    for (const auto& [key, value]: auth.claims()) ctx.claims.insert_or_assign(key, value);
    ctx.claims.insert_or_assign("merchant_id", ctx.merchant_id);
    ctx.claims.insert_or_assign("org_id", ctx.org_id);
    ctx.claims.insert_or_assign("profile_id", ctx.profile_id);
    return ctx;
}

[[nodiscard]] inline std::string ctx_string(const ::core::behavior::CallContext& ctx, std::string_view key)
{
    const auto it = ctx.claims.find(std::string{key});
    return it == ctx.claims.end() ? std::string{} : it->second;
}

/**
 * @brief Domain authorization policy for HierarchicalResources.
 *
 * The route operation string selects the registered auth rule; the scope is
 * resolved from the request once, for every handler in the domain.
 */
class HierarchicalResourcesPolicy
{
public:
    explicit HierarchicalResourcesPolicy(HierarchicalResourcesInputSupport& input) noexcept :
        m_input{input}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const ::core::behavior::AuthSpec&
    ) const
    {
        return m_input.authorize_request(req, operation);
    }

    [[nodiscard]] ::core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        ::core::behavior::Scope out{};
        out.merchant_id = std::string{::app::http::merchant_id_from_header{}(req)};
        if (out.merchant_id.empty()) {
            out.merchant_id = auth.merchant_id();
        }
        out.org_id = std::string{::app::http::org_id_from_header{}(req)};
        if (out.org_id.empty()) {
            out.org_id = auth.org_id();
        }
        out.profile_id = auth.profile_id();
        const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId);
        if (connected.has_value()) {
            out.connected_merchant_id = std::string{*connected};
        }
        return out;
    }

    [[nodiscard]] ::core::behavior::CallContext
    context(interfaces::io::IRequest& req) const
    {
        return make_call_context(req);
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const ::core::behavior::Scope&) const
    {
        return std::nullopt;
    }


private:
    HierarchicalResourcesInputSupport& m_input;
};

using HierarchicalResourcesRequestSupport =
    ::core::behavior::RequestSupport<HierarchicalResourcesPolicy>;

} // namespace app::handlers::hierarchical_resources
