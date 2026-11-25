module;
export module app_auth:route_auth;

import std;
import :permission;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::auth {

/** @brief Credential scheme a route accepts, mirrored from upstream auth extractors. */
enum class AuthScheme : std::uint8_t
{
    API_KEY,
    JWT,
    HEADER_MERCHANT,
    HEADER_ORG,
    PUBLISHABLE_KEY,
    CLIENT_SECRET,
    PLATFORM_ADMIN,
    // Added by section 6.1 parity; appended so existing scheme numbers stay stable.
    SDK,
    V2_API_KEY,
    INTERNAL_API_KEY,
    EMBEDDED,
    JWT_AND_EMBEDDED,
    MERCHANT_ID,
    NO_AUTH,
    /** @brief Sentinel for an unregistered operation; a real rule that always denies. */
    FORBIDDEN,
};

/** @brief Instance functor classifying a scheme for authorization bookkeeping. */
class AuthSchemeTraits
{
public:
    [[nodiscard]] bool requires_bearer(AuthScheme scheme) const noexcept
    {
        switch (scheme) {
            case AuthScheme::API_KEY:
            case AuthScheme::JWT:
            case AuthScheme::PLATFORM_ADMIN:
            case AuthScheme::V2_API_KEY:
            case AuthScheme::JWT_AND_EMBEDDED:
                return true;
            default:
                return false;
        }
    }

    [[nodiscard]] bool bypasses_checks(AuthScheme scheme) const noexcept
    {
        return scheme == AuthScheme::NO_AUTH;
    }

    [[nodiscard]] bool always_denies(AuthScheme scheme) const noexcept
    {
        return scheme == AuthScheme::FORBIDDEN;
    }
};

/** @brief Declarative auth requirement attached to a route. */
class RouteAuthRule
{
public:
    RouteAuthRule() = default;

    RouteAuthRule(
        AuthScheme scheme,
        AppPermission permission,
        bool allow_connected = false,
        bool allow_platform = false,
        std::optional<std::string> id_source = std::nullopt
    ) :
        m_scheme{scheme},
        m_permission{permission},
        m_allow_connected{allow_connected},
        m_allow_platform{allow_platform},
        m_id_source{std::move(id_source)}
    {
    }

    [[nodiscard]] AuthScheme scheme() const noexcept
    {
        return m_scheme;
    }

    [[nodiscard]] AppPermission permission() const noexcept
    {
        return m_permission;
    }

    [[nodiscard]] bool allow_connected() const noexcept
    {
        return m_allow_connected;
    }

    [[nodiscard]] bool allow_platform() const noexcept
    {
        return m_allow_platform;
    }

    /** @brief Optional header name an additional identity check is keyed on. */
    [[nodiscard]] const std::optional<std::string>& id_source() const noexcept
    {
        return m_id_source;
    }

private:
    AuthScheme m_scheme{AuthScheme::JWT};
    AppPermission m_permission{AppPermission::ADMIN};
    bool m_allow_connected{false};
    bool m_allow_platform{false};
    std::optional<std::string> m_id_source{};
};

/** @brief Instance functor building RouteAuthRule values. */
class RouteAuthRuleBuilder
{
public:
    RouteAuthRuleBuilder& scheme(AuthScheme value)
    {
        m_scheme = value;
        return *this;
    }

    RouteAuthRuleBuilder& permission(AppPermission value)
    {
        m_permission = value;
        return *this;
    }

    RouteAuthRuleBuilder& allow_connected(bool value = true)
    {
        m_allow_connected = value;
        return *this;
    }

    RouteAuthRuleBuilder& allow_platform(bool value = true)
    {
        m_allow_platform = value;
        return *this;
    }

    RouteAuthRuleBuilder& id_source(std::string value)
    {
        m_id_source = std::move(value);
        return *this;
    }

    [[nodiscard]] RouteAuthRule build() const
    {
        return RouteAuthRule{
            m_scheme,
            m_permission,
            m_allow_connected,
            m_allow_platform,
            m_id_source,
        };
    }

    [[nodiscard]] RouteAuthRule operator()() const
    {
        return build();
    }

    [[nodiscard]] RouteAuthRule operator()(AuthScheme scheme, AppPermission permission) const
    {
        return RouteAuthRule{
            scheme,
            permission,
            m_allow_connected,
            m_allow_platform,
            m_id_source,
        };
    }

private:
    AuthScheme m_scheme{AuthScheme::JWT};
    AppPermission m_permission{AppPermission::ADMIN};
    bool m_allow_connected{false};
    bool m_allow_platform{false};
    std::optional<std::string> m_id_source{};
};

} // namespace app::auth

#ifdef CONGELADO_TEST
namespace app::auth::tests {
using namespace boost::ut;

suite<"app_auth_route_auth"> route_auth_suite = []
{
    "rule carries scheme, permission, flags and header id source"_test = []
    {
        const RouteAuthRule rule{
            AuthScheme::HEADER_MERCHANT,
            AppPermission::MERCHANT_READ,
            true,
            false,
            std::string{"x-merchant-id"},
        };
        expect(rule.scheme() == AuthScheme::HEADER_MERCHANT);
        expect(rule.permission() == AppPermission::MERCHANT_READ);
        expect(rule.allow_connected());
        expect(!rule.allow_platform());
        expect(rule.id_source().has_value());
        expect(*rule.id_source() == "x-merchant-id");
    };

    "builder produces the configured rule and safe defaults"_test = []
    {
        const RouteAuthRule built = RouteAuthRuleBuilder{}
                                        .scheme(AuthScheme::HEADER_ORG)
                                        .permission(AppPermission::ORGANIZATION_WRITE)
                                        .allow_platform()
                                        .id_source("x-organization-id")
                                        .build();
        expect(built.scheme() == AuthScheme::HEADER_ORG);
        expect(built.permission() == AppPermission::ORGANIZATION_WRITE);
        expect(built.allow_platform());
        expect(!built.allow_connected());
        expect(*built.id_source() == "x-organization-id");

        const RouteAuthRule fallback = RouteAuthRuleBuilder{}.build();
        expect(fallback.scheme() == AuthScheme::JWT);
        expect(fallback.permission() == AppPermission::ADMIN);
        expect(!fallback.allow_connected());
        expect(!fallback.allow_platform());
        expect(!fallback.id_source().has_value());

        const auto called = RouteAuthRuleBuilder{}.allow_connected()(
            AuthScheme::API_KEY,
            AppPermission::API_KEY_READ
        );
        expect(called.scheme() == AuthScheme::API_KEY);
        expect(called.permission() == AppPermission::API_KEY_READ);
        expect(called.allow_connected());
    };

    "section 6.1 schemes and FORBIDDEN sentinel are distinct"_test = []
    {
        const AuthSchemeTraits traits;
        expect(traits.bypasses_checks(AuthScheme::NO_AUTH));
        expect(traits.always_denies(AuthScheme::FORBIDDEN));
        expect(traits.requires_bearer(AuthScheme::V2_API_KEY));
        expect(traits.requires_bearer(AuthScheme::JWT_AND_EMBEDDED));
        expect(!traits.requires_bearer(AuthScheme::SDK));
        expect(static_cast<std::uint8_t>(AuthScheme::API_KEY) == 0);
        expect(static_cast<std::uint8_t>(AuthScheme::PLATFORM_ADMIN) == 6);
    };
};

} // namespace app::auth::tests
#endif
