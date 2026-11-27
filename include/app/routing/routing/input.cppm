export module app_routing:routing_input;

import std;
import interfaces;
import core_error;
import core_behavior;
import core_serde;
import app_context;
import app_model;
import app_dto;
import app_auth;
import app_http;

import :policy;

export namespace app::handlers::routing {

using RoutingRecord =
    std::remove_cvref_t<decltype(*std::declval<::app::context::AppContext&>()
                                      .repository()
                                      .find_routing_config_by_string(std::string_view{}))>;
using DictionaryRecord = ::app::model::routing::RoutingDictionaryRecord;
using Connectors = std::vector<::app::model::routing::RoutableConnectorChoice>;

/**
 * Local mirror of one upstream route's auth extractor combination:
 *   HeaderAuth(ApiKeyAuth{...}) + JWTAuth{...} / V2ApiKeyAuth + JWTAuthProfileFromRoute /
 *   JWTAuth (dashboard-only) / AdminApiAuth (migration endpoints).
 */
struct AuthSpec
{
    ::app::auth::AppPermission permission{::app::auth::AppPermission::PROFILE_READ};
    std::string_view required_permission{"routing:read"}; // upstream Permission wire name
    bool allow_connected{true};
    bool allow_platform{false};
    bool profile_from_route{false};
    bool jwt_only{false};
    bool admin_api{false};
    bool api_key_v2{false};
};

/**
 * @brief Parse/auth/path/query support for the routing handlers.
 *
 * No business logic lives here: it turns request data into the plain RoutingInput
 * consumed by the routing runtime, and returns auth/validation errors instead of
 * writing them so the handler keeps one response-writing path per route.
 */
class RoutingInputSupport
{
public:
    explicit RoutingInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ───────────────────────── Per-operation auth spec ─────────────────────────

    /** Exact per-operation extractor combination (routes/routing.rs:30-51,63-93,113-151,...). */
    [[nodiscard]] AuthSpec spec_for(std::string_view operation) const
    {
        using P = ::app::auth::AppPermission;
        AuthSpec spec{};

        // AdminApiAuth on both migration endpoints (routing.rs:1592,1615).
        if (operation == "migrate_routing_rules" || operation == "routing_migration_status") {
            const bool write = operation == "migrate_routing_rules";
            spec.permission = write ? P::ROUTING_WRITE : P::ROUTING_READ;
            spec.required_permission = write ? "routing:write" : "routing:read";
            spec.admin_api = true;
            spec.allow_platform = true; // AdminApiAuth is platform-scoped
            return spec;
        }

        // Dashboard routing entry: JWTAuth ProfileRoutingRead, dashboard-only (routing.rs:45-49).
        if (operation == "routing_entry") {
            spec.permission = P::PROFILE_READ;
            spec.required_permission = "routing:read";
            spec.jwt_only = true;
            return spec;
        }

        // 3DS decision-manager family (routing.rs:861-874,961-977,1065-1078; 921-935,1021-1035).
        if (operation == "upsert_decision_manager_config" ||
            operation == "delete_decision_manager_config") {
            spec.permission = P::MERCHANT_WRITE;
            spec.required_permission = "three_ds_decision_manager:write";
            return spec;
        }
        if (operation == "retrieve_decision_manager_config") {
            spec.permission = P::MERCHANT_READ;
            spec.required_permission = "three_ds_decision_manager:read";
            return spec;
        }

        // Surcharge decision-manager family (routing.rs:733-746,773-786,816-829).
        if (operation == "upsert_surcharge_decision_manager_config" ||
            operation == "delete_surcharge_decision_manager_config") {
            spec.permission = P::MERCHANT_WRITE;
            spec.required_permission = "surcharge_decision_manager:write";
            return spec;
        }
        if (operation == "retrieve_surcharge_decision_manager_config") {
            spec.permission = P::MERCHANT_READ;
            spec.required_permission = "surcharge_decision_manager:read";
            return spec;
        }

        // Dynamic routing: ProfileRouting* + JWTAuthProfileFromRoute.
        if (operation == "create_success_based_routing" ||
            operation == "create_elimination_routing" ||
            operation == "contract_based_routing_setup_config" ||
            operation == "set_dynamic_routing_volume_split") {
            spec.permission = P::PROFILE_WRITE;
            spec.required_permission = "routing:write";
            spec.profile_from_route = true;
            return spec;
        }
        if (operation == "get_dynamic_routing_volume_split") {
            spec.permission = P::PROFILE_READ;
            spec.required_permission = "routing:read";
            spec.profile_from_route = true;
            return spec;
        }

        // Core routing family — v1 permissions; v2 overrides applied in authorize_operation().
        const bool write =
            operation == "routing_create_config" || operation == "routing_link_config" ||
            operation == "routing_unlink_config" || operation == "routing_update_default_config" ||
            operation == "routing_update_default_config_for_profile";
        const bool merchant_read = operation == "list_routing_configs" ||
                                   operation == "routing_retrieve_default_config_for_profiles";
        spec.permission = merchant_read ? P::MERCHANT_READ
                          : write       ? P::PROFILE_WRITE
                                        : P::PROFILE_READ;
        spec.required_permission = write ? "routing:write" : "routing:read";
        spec.profile_from_route = operation == "routing_retrieve_linked_config" ||
                                  operation == "routing_update_default_config_for_profile";
        return spec;
    }

    /** Matches the authorizer's permission-claim check but with upstream grant names. */
    [[nodiscard]] bool
    granular_permission_allowed(interfaces::io::IRequest& req, std::string_view required) const
    {
        const auto single = req.find_header("permission");
        const auto list = req.find_header("permissions");
        if (single.empty() && list.empty()) {
            return true; // no explicit grant list: bearer/scheme check governs
        }
        const auto grants = [required](std::string_view granted)
        {
            std::string_view rest = granted;
            while (!rest.empty()) {
                const auto comma = rest.find(',');
                const auto token = rest.substr(0, comma);
                if (token == required || token == "ADMIN" || token == "all") {
                    return true;
                }
                if (comma == std::string_view::npos) {
                    break;
                }
                rest = rest.substr(comma + 1);
            }
            return false;
        };
        return (!single.empty() && grants(single)) || (!list.empty() && grants(list));
    }

    [[nodiscard]] bool is_v2_route(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        for (std::size_t i = 0; i < segments.count(); ++i) {
            if (segments.at(i).value() == "v2") {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] std::string merchant_id_of(interfaces::io::IRequest& req) const
    {
        return ::app::http::merchant_id_from_header{}(req);
    }

    [[nodiscard]] std::string org_id_of(interfaces::io::IRequest& req) const
    {
        return ::app::http::org_id_from_header{}(req);
    }

    [[nodiscard]] std::string first_path_param_after(
        interfaces::io::IRequest& req,
        std::string_view key,
        std::string_view alt = {}
    ) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        for (std::size_t i = 0; i + 1 < segments.count(); ++i) {
            const auto current = segments.at(i).value();
            if (current == key || (!alt.empty() && current == alt)) {
                return std::string{segments.at(i + 1).value()};
            }
        }
        if (const auto from_query = ::app::http::RequestQuery{}(req).get("profile_id")) {
            return *from_query;
        }
        return ::app::auth::AuthContextBuilder{}(req).profile_id();
    }

    [[nodiscard]] std::string profile_id_of(interfaces::io::IRequest& req) const
    {
        return first_path_param_after(req, "profiles", "business_profile");
    }

    [[nodiscard]] std::string algorithm_id_of(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        for (std::size_t i = 0; i < segments.count(); ++i) {
            const auto current = segments.at(i).value();
            if (current == "routing-algorithms" && i + 1 < segments.count()) {
                return std::string{segments.at(i + 1).value()};
            }
            if (current == "activate" && i > 0) {
                const auto previous = segments.at(i - 1).value();
                if (previous != "routing-algorithms") {
                    return std::string{previous};
                }
            }
        }
        if (const auto from_query = ::app::http::RequestQuery{}(req).get("algorithm_id")) {
            return *from_query;
        }
        return {};
    }

    [[nodiscard]] std::string transaction_type_of(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        for (std::size_t i = 0; i < segments.count(); ++i) {
            if (segments.at(i).value() == "payouts") {
                return "payout";
            }
        }
        if (const auto from_query = ::app::http::RequestQuery{}(req).get("transaction_type")) {
            return *from_query;
        }
        return "payment";
    }

    [[nodiscard]] std::string auth_profile_of(interfaces::io::IRequest& req) const
    {
        return ::app::auth::AuthContextBuilder{}(req).profile_id();
    }

    /** core/utils.rs:2889-2912 validate_profile_id_from_auth_layer (pure given auth_profile). */
    [[nodiscard]] std::optional<std::string>
    profile_validation_error(interfaces::io::IRequest& req, std::string_view profile) const
    {
        const auto auth_profile = ::app::auth::AuthContextBuilder{}(req).profile_id();
        if (!auth_profile.empty() && !profile.empty() && auth_profile != profile) {
            return std::string{
                "Profile id authentication failed. Please use the correct JWT token"
            };
        }
        return std::nullopt;
    }

    /** Returns the auth error instead of writing it (same checks as the original write path). */
    [[nodiscard]] std::optional<core::error::AppError>
    authorize_operation(interfaces::io::IRequest& req, std::string_view operation) const
    {
        auto spec = spec_for(operation);

        // v2 routes: V2ApiKeyAuth + JWTAuthProfileFromRoute, with v2 permission names.
        if (is_v2_route(req)) {
            spec.api_key_v2 = true;
            spec.profile_from_route = true;
            if (operation == "routing_create_config" || operation == "routing_link_config" ||
                operation == "routing_unlink_config" ||
                operation == "routing_update_default_config") {
                spec.permission = ::app::auth::AppPermission::MERCHANT_WRITE;
                spec.required_permission = "routing:write";
            } else if (operation == "routing_retrieve_default_config") {
                spec.permission = ::app::auth::AppPermission::MERCHANT_READ;
                spec.required_permission = "routing:read";
            } else if (
                operation == "upsert_decision_manager_config" ||
                operation == "retrieve_decision_manager_config"
            ) {
                spec.permission = ::app::auth::AppPermission::PROFILE_WRITE;
                spec.required_permission = "three_ds_decision_manager:write";
            } else {
                spec.permission = ::app::auth::AppPermission::PROFILE_READ;
                spec.required_permission = "routing:read";
            }
        }

        const auto context = ::app::auth::AuthContextBuilder{}(req);

        // Upstream auth_type picks API-key when the header is present, otherwise JWT.
        ::app::auth::AuthScheme scheme = ::app::auth::AuthScheme::JWT;
        if (spec.admin_api) {
            scheme = ::app::auth::AuthScheme::PLATFORM_ADMIN;
        } else if (
            !spec.jwt_only && (context.claim(::app::auth::kClaimApiKey) ||
                               context.claim(::app::auth::kClaimInternalApiKey))
        ) {
            scheme = spec.api_key_v2 ? ::app::auth::AuthScheme::V2_API_KEY
                                     : ::app::auth::AuthScheme::API_KEY;
        }
        const ::app::auth::RouteAuthRule
            rule{scheme, spec.permission, spec.allow_connected, spec.allow_platform};
        if (const auto denied = m_context.authorizer().authorize_route(context, rule)) {
            return denied;
        }

        if (!spec.allow_connected && context.claim(::app::auth::kClaimConnectedMerchantId)) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "connected scope not allowed for this operation"
            );
        }
        if (!spec.allow_platform && context.claim(::app::auth::kClaimInternalApiKey)) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "platform scope not allowed for this operation"
            );
        }
        if (!granular_permission_allowed(req, spec.required_permission)) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                std::format("permission {} not granted", spec.required_permission)
            );
        }
        if (spec.profile_from_route) {
            const auto route_profile = profile_id_of(req);
            if (const auto validation_error = profile_validation_error(req, route_profile)) {
                return core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::PRECONDITION_FAILED,
                    *validation_error
                );
            }
        }
        return std::nullopt;
    }

private:
    ::app::context::AppContext& m_context;
};

/**
 * @brief Domain policy for routing routes. The input support already encodes the
 * per-operation extractor combination (v1/v2, admin API, granular grants), so the
 * policy simply forwards the operation unchanged.
 */
class RoutingPolicy
{
public:
    explicit RoutingPolicy(RoutingInputSupport& support) noexcept :
        m_support{support}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const ::core::behavior::AuthSpec&
    ) const
    {
        return m_support.authorize_operation(req, operation);
    }

    [[nodiscard]] ::core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        return ::app::routing::policy::scope_of(req);
    }

    [[nodiscard]] ::core::behavior::CallContext context(interfaces::io::IRequest& req) const
    {
        auto ctx = ::app::routing::policy::context_of(req);
        ctx.merchant_id = m_support.merchant_id_of(req);
        ctx.org_id = m_support.org_id_of(req);
        ctx.profile_id = m_support.profile_id_of(req);
        return ctx;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const ::core::behavior::Scope& scope) const
    {
        return ::app::routing::policy::scope_error(scope);
    }

private:
    RoutingInputSupport& m_support;
};

using RoutingRequestSupport = ::core::behavior::RequestSupport<RoutingPolicy>;

} // namespace app::handlers::routing
