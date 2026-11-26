export module app_user:hierarchical_resources_input;

import std;
import interfaces;
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
    [[nodiscard]] std::expected<
        ::app::dto::hierarchical_resources::UploadCertificateRequest,
        core::error::AppError>
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

    // ── Standard base64 (upstream BASE64_ENGINE; local, no new module deps) ────

    [[nodiscard]] std::string base64_encode(std::string_view input) const
    {
        static constexpr std::string_view table{
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
        };
        std::string out{};
        out.reserve(4 * ((input.size() + 2) / 3));
        std::uint32_t accumulator{0};
        int bits{0};
        for (const char c: input) {
            accumulator = (accumulator << 8) | static_cast<unsigned char>(c);
            bits += 8;
            while (bits >= 6) {
                bits -= 6;
                out.push_back(table[(accumulator >> bits) & 0x3FU]);
            }
        }
        if (bits > 0) {
            out.push_back(table[(accumulator << (6 - bits)) & 0x3FU]);
            while (out.size() % 4 != 0) {
                out.push_back('=');
            }
        }
        return out;
    }

    ::app::context::AppContext& m_context;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old *_core call received.

struct GenerateHierarchicalResourceInput
{
    std::string merchant_id{};
    std::string organization_id{};
    std::string resource_type{};
};

struct UploadHierarchicalResourceInput
{
    std::string resource_id{};
    std::string certificate{};
    std::string merchant_id{};
    std::string organization_id{};
};

struct ListHierarchicalResourcesInput
{
    ::app::dto::hierarchical_resources::ListHierarchicalResourcesRequest request{};
    std::string merchant_id{};
    std::string organization_id{};
};

struct LinkHierarchicalResourceInput
{
    std::string resource_id{};
    std::string requestor_type{};
    std::string requestor_id{};
    std::string merchant_id{};
    std::string organization_id{};
};

} // namespace app::handlers::hierarchical_resources
