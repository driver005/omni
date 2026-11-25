module;
export module app_auth:authorizer;

import std;
import core_error;
import :permission;
import :token;
import :route_auth;
import :context;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::auth {

/** @brief Per-route authorization check; returns an error only when denied. */
class AppAuthorizer
{
public:
    using ErrorFactory =
        std::function<core::error::AppError(core::error::AppErrorKind, std::string)>;

    AppAuthorizer() :
        m_factory{[](core::error::AppErrorKind kind, std::string message)
                  {
                      return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
                  }}
    {
    }

    explicit AppAuthorizer(ErrorFactory factory) :
        m_factory{std::move(factory)}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError>
    authorize(const BearerToken& token, AppPermission permission) const
    {
        if (token.is_empty()) {
            return m_factory(core::error::AppErrorKind::UNAUTHORIZED, "missing bearer token");
        }
        (void)permission;
        return std::nullopt;
    }

    /**
     * @brief Applies a route's auth scheme to an already-built request context.
     * @note API key, JWT and platform admin require a bearer token; header schemes require
     * the matching header-derived id; SDK schemes require their own header. An explicit
     * permission claim can still narrow access to the rule's AppPermission.
     */
    [[nodiscard]] std::optional<core::error::AppError>
    authorize_route(const AuthContext& context, const RouteAuthRule& rule) const
    {
        const AuthScheme scheme = rule.scheme();

        // Section 6.4: NO_AUTH is a real rule that bypasses every token/header check.
        if (scheme == AuthScheme::NO_AUTH) {
            return std::nullopt;
        }
        // Section 6.3: an unregistered operation resolves to a rule that always denies.
        if (scheme == AuthScheme::FORBIDDEN) {
            return m_factory(
                core::error::AppErrorKind::FORBIDDEN,
                "operation is not registered in the auth rule registry"
            );
        }

        const std::string_view permission_name = AppPermissionName{}(rule.permission());
        const AuthSchemeTraits traits;

        if (traits.requires_bearer(scheme)) {
            if (const auto denied = authorize(context.token(), rule.permission())) {
                return denied;
            }
        }

        if (scheme == AuthScheme::V2_API_KEY || scheme == AuthScheme::INTERNAL_API_KEY) {
            const std::string_view claim =
                scheme == AuthScheme::V2_API_KEY ? kClaimApiKey : kClaimInternalApiKey;
            if (!has_claim(context, claim)) {
                return m_factory(
                    core::error::AppErrorKind::UNAUTHORIZED,
                    std::format("missing {} header for {}", claim, permission_name)
                );
            }
        }

        if (scheme == AuthScheme::MERCHANT_ID && context.merchant_id().empty()) {
            return m_factory(
                core::error::AppErrorKind::UNAUTHORIZED,
                std::format("missing merchant id for {}", permission_name)
            );
        }

        if (scheme == AuthScheme::SDK && !has_claim(context, kClaimSdkKey) &&
            !(has_claim(context, kClaimPublishableKey) && has_claim(context, kClaimClientSecret))) {
            return m_factory(
                core::error::AppErrorKind::UNAUTHORIZED,
                "missing SDK authorization header"
            );
        }

        if (!rule.allow_platform()) {
            if (scheme == AuthScheme::HEADER_MERCHANT && context.merchant_id().empty()) {
                const auto connected = rule.allow_connected()
                                           ? context.claim("x-connected-merchant-id")
                                           : std::nullopt;
                if (!connected.has_value() || connected->empty()) {
                    return m_factory(
                        core::error::AppErrorKind::UNAUTHORIZED,
                        std::format("missing merchant id header for {}", permission_name)
                    );
                }
            } else if (scheme == AuthScheme::HEADER_ORG && context.org_id().empty()) {
                return m_factory(
                    core::error::AppErrorKind::UNAUTHORIZED,
                    std::format("missing organization id header for {}", permission_name)
                );
            }
        }

        if ((scheme == AuthScheme::PUBLISHABLE_KEY || scheme == AuthScheme::SDK ||
             scheme == AuthScheme::EMBEDDED || scheme == AuthScheme::JWT_AND_EMBEDDED) &&
            !has_claim(context, kClaimPublishableKey)) {
            return m_factory(
                core::error::AppErrorKind::UNAUTHORIZED,
                "missing publishable key header"
            );
        }
        if ((scheme == AuthScheme::CLIENT_SECRET || scheme == AuthScheme::SDK ||
             scheme == AuthScheme::EMBEDDED || scheme == AuthScheme::JWT_AND_EMBEDDED) &&
            !has_claim(context, kClaimClientSecret)) {
            return m_factory(
                core::error::AppErrorKind::UNAUTHORIZED,
                "missing client secret header"
            );
        }

        if (rule.id_source().has_value() && !has_claim(context, *rule.id_source())) {
            return m_factory(
                core::error::AppErrorKind::UNAUTHORIZED,
                std::format("missing {} header", *rule.id_source())
            );
        }

        return check_permission_claim(context, rule.permission());
    }

private:
    [[nodiscard]] bool has_claim(const AuthContext& context, std::string_view key) const
    {
        const auto value = context.claim(key);
        return value.has_value() && !value->empty();
    }

    [[nodiscard]] std::optional<core::error::AppError>
    check_permission_claim(const AuthContext& context, AppPermission permission) const
    {
        const auto single = context.claim("permission");
        const auto list = context.claim("permissions");
        if (!single.has_value() && !list.has_value()) {
            return std::nullopt;
        }

        const std::string_view required = AppPermissionName{}(permission);
        if (required == "ADMIN") {
            return std::nullopt;
        }

        const auto grants = [required](std::string_view granted)
        {
            std::string_view rest = granted;
            while (!rest.empty()) {
                const std::size_t comma = rest.find(',');
                const std::string_view token = rest.substr(0, comma);
                if (token == required || token == "ADMIN") {
                    return true;
                }
                if (comma == std::string_view::npos) {
                    break;
                }
                rest = rest.substr(comma + 1);
            }
            return false;
        };

        if ((single.has_value() && grants(*single)) || (list.has_value() && grants(*list))) {
            return std::nullopt;
        }
        return m_factory(
            core::error::AppErrorKind::FORBIDDEN,
            std::format("permission {} not granted", required)
        );
    }

    ErrorFactory m_factory;
};

} // namespace app::auth

#ifdef CONGELADO_TEST
namespace app::auth::tests {
using namespace boost::ut;

suite<"app_auth_authorizer"> authorizer_suite = []
{
    "empty token is unauthorized, present token is allowed"_test = []
    {
        const AppAuthorizer authorizer;
        const auto denied = authorizer.authorize(BearerToken{}, AppPermission::PAYMENT_READ);
        expect(denied.has_value());
        expect(denied->get_kind() == core::error::AppErrorKind::UNAUTHORIZED);
        expect(denied->get_kind_name() == "UNAUTHORIZED");
        expect(!authorizer.authorize(BearerToken{"abc"}, AppPermission::PAYMENT_WRITE).has_value());
    };

    "injected error factory is used for the denial"_test = []
    {
        int calls{0};
        AppAuthorizer injected{
            [&calls](core::error::AppErrorKind kind, std::string message)
            {
                ++calls;
                return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
            }
        };
        const auto denied = injected.authorize(BearerToken{}, AppPermission::USER_READ);
        expect(denied.has_value());
        expect(calls == 1);
        expect(!injected.authorize(BearerToken{"abc"}, AppPermission::USER_READ).has_value());
        expect(calls == 1);
    };

    "token schemes require a bearer token"_test = []
    {
        const AppAuthorizer authorizer;
        const AuthContext empty{};
        const AuthContext filled{BearerToken{"abc"}, "", "", "", {}};

        const auto api_key = authorizer.authorize_route(
            empty,
            RouteAuthRule{AuthScheme::API_KEY, AppPermission::PAYMENT_READ}
        );
        expect(api_key.has_value());
        expect(api_key->get_kind() == core::error::AppErrorKind::UNAUTHORIZED);

        const auto jwt = authorizer.authorize_route(
            empty,
            RouteAuthRule{AuthScheme::JWT, AppPermission::PAYMENT_READ}
        );
        expect(jwt.has_value());

        const auto admin = authorizer.authorize_route(
            empty,
            RouteAuthRule{AuthScheme::PLATFORM_ADMIN, AppPermission::ADMIN}
        );
        expect(admin.has_value());

        expect(!authorizer
                    .authorize_route(
                        filled,
                        RouteAuthRule{AuthScheme::JWT, AppPermission::PAYMENT_READ}
                    )
                    .has_value());
    };

    "header schemes require their header-derived id"_test = []
    {
        const AppAuthorizer authorizer;
        const AuthContext empty{};
        const auto merchant_rule = RouteAuthRule{
            AuthScheme::HEADER_MERCHANT,
            AppPermission::MERCHANT_READ,
        };
        const auto org_rule = RouteAuthRule{
            AuthScheme::HEADER_ORG,
            AppPermission::ORGANIZATION_READ,
        };

        const auto missing_merchant = authorizer.authorize_route(empty, merchant_rule);
        expect(missing_merchant.has_value());
        expect(missing_merchant->get_kind() == core::error::AppErrorKind::UNAUTHORIZED);
        expect(authorizer.authorize_route(empty, org_rule).has_value());

        const AuthContext merchant{BearerToken{}, "merchant_1", "", "", {}};
        expect(!authorizer.authorize_route(merchant, merchant_rule).has_value());

        AuthContext connected{BearerToken{}, "", "", "", {}};
        connected.add_claim("x-connected-merchant-id", "connected_1");
        const auto connected_rule = RouteAuthRule{
            AuthScheme::HEADER_MERCHANT,
            AppPermission::MERCHANT_READ,
            true,
        };
        expect(!authorizer.authorize_route(connected, connected_rule).has_value());
        expect(authorizer.authorize_route(connected, merchant_rule).has_value());

        const AuthContext org{BearerToken{}, "", "org_1", "", {}};
        expect(!authorizer.authorize_route(org, org_rule).has_value());

        const auto platform_rule = RouteAuthRule{
            AuthScheme::HEADER_ORG,
            AppPermission::ORGANIZATION_READ,
            false,
            true,
        };
        expect(!authorizer.authorize_route(empty, platform_rule).has_value());
    };

    "SDK schemes and extra id sources require their headers"_test = []
    {
        const AppAuthorizer authorizer;
        const AuthContext empty{};

        expect(authorizer
                   .authorize_route(
                       empty,
                       RouteAuthRule{AuthScheme::PUBLISHABLE_KEY, AppPermission::PAYMENT_READ}
                   )
                   .has_value());
        expect(authorizer
                   .authorize_route(
                       empty,
                       RouteAuthRule{AuthScheme::CLIENT_SECRET, AppPermission::PAYMENT_WRITE}
                   )
                   .has_value());

        AuthContext sdk{BearerToken{}, "", "", "", {}};
        sdk.add_claim("x-publishable-key", "pk_1");
        sdk.add_claim("x-client-secret", "cs_1");
        expect(!authorizer
                    .authorize_route(
                        sdk,
                        RouteAuthRule{AuthScheme::PUBLISHABLE_KEY, AppPermission::PAYMENT_READ}
                    )
                    .has_value());
        expect(!authorizer
                    .authorize_route(
                        sdk,
                        RouteAuthRule{AuthScheme::CLIENT_SECRET, AppPermission::PAYMENT_WRITE}
                    )
                    .has_value());

        const auto sourced = RouteAuthRuleBuilder{}
                                 .scheme(AuthScheme::HEADER_MERCHANT)
                                 .permission(AppPermission::MERCHANT_READ)
                                 .id_source("x-profile-id")
                                 .build();
        const AuthContext merchant{BearerToken{}, "merchant_1", "", "", {}};
        const auto missing_source = authorizer.authorize_route(merchant, sourced);
        expect(missing_source.has_value());
        expect(missing_source->get_kind() == core::error::AppErrorKind::UNAUTHORIZED);
    };

    "explicit permission claims narrow access to the rule permission"_test = []
    {
        const AppAuthorizer authorizer;
        AuthContext context{BearerToken{"abc"}, "", "", "", {}};
        context.add_claim("permissions", "PAYMENT_READ,REFUND_WRITE");

        const auto allowed = authorizer.authorize_route(
            context,
            RouteAuthRule{AuthScheme::JWT, AppPermission::PAYMENT_READ}
        );
        expect(!allowed.has_value());

        const auto denied = authorizer.authorize_route(
            context,
            RouteAuthRule{AuthScheme::JWT, AppPermission::PAYMENT_WRITE}
        );
        expect(denied.has_value());
        expect(denied->get_kind() == core::error::AppErrorKind::FORBIDDEN);
    };

    "NO_AUTH bypasses every check and FORBIDDEN always denies"_test = []
    {
        const AppAuthorizer authorizer;
        expect(!authorizer
                    .authorize_route(
                        AuthContext{},
                        RouteAuthRule{AuthScheme::NO_AUTH, AppPermission::ADMIN}
                    )
                    .has_value());
        const auto denied = authorizer.authorize_route(
            AuthContext{BearerToken{"abc"}, "", "", "", {}},
            RouteAuthRule{AuthScheme::FORBIDDEN, AppPermission::ADMIN}
        );
        expect(denied.has_value());
        expect(denied->get_kind() == core::error::AppErrorKind::FORBIDDEN);
    };

    "new schemes require their credential headers"_test = []
    {
        const AppAuthorizer authorizer;
        const AuthContext empty{};
        expect(authorizer
                   .authorize_route(
                       empty,
                       RouteAuthRule{AuthScheme::V2_API_KEY, AppPermission::PAYMENT_READ}
                   )
                   .has_value());
        expect(authorizer
                   .authorize_route(
                       empty,
                       RouteAuthRule{AuthScheme::INTERNAL_API_KEY, AppPermission::ADMIN}
                   )
                   .has_value());
        expect(authorizer
                   .authorize_route(
                       empty,
                       RouteAuthRule{AuthScheme::SDK, AppPermission::PAYMENT_WRITE}
                   )
                   .has_value());
        expect(authorizer
                   .authorize_route(
                       empty,
                       RouteAuthRule{AuthScheme::EMBEDDED, AppPermission::PAYMENT_READ}
                   )
                   .has_value());

        AuthContext api{BearerToken{}, "", "", "", {}};
        api.add_claim(std::string{kClaimApiKey}, "key_1");
        expect(!authorizer
                    .authorize_route(
                        api,
                        RouteAuthRule{AuthScheme::V2_API_KEY, AppPermission::PAYMENT_READ}
                    )
                    .has_value());

        AuthContext internal{BearerToken{}, "", "", "", {}};
        internal.add_claim(std::string{kClaimInternalApiKey}, "internal_1");
        expect(!authorizer
                    .authorize_route(
                        internal,
                        RouteAuthRule{AuthScheme::INTERNAL_API_KEY, AppPermission::ADMIN}
                    )
                    .has_value());

        AuthContext embedded{BearerToken{}, "merchant_1", "", "", {}};
        embedded.add_claim(std::string{kClaimPublishableKey}, "pk_1");
        embedded.add_claim(std::string{kClaimClientSecret}, "cs_1");
        expect(!authorizer
                    .authorize_route(
                        embedded,
                        RouteAuthRule{AuthScheme::EMBEDDED, AppPermission::PAYMENT_READ}
                    )
                    .has_value());
        expect(!authorizer
                    .authorize_route(
                        embedded,
                        RouteAuthRule{AuthScheme::MERCHANT_ID, AppPermission::MERCHANT_READ}
                    )
                    .has_value());
    };
};

} // namespace app::auth::tests
#endif
