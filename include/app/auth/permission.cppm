module;
export module app_auth:permission;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::auth {

/** @brief Per-route permission required by an app handler. */
enum class AppPermission : std::uint8_t
{
    PAYMENT_READ,
    PAYMENT_WRITE,
    REFUND_WRITE,
    API_KEY_READ,
    API_KEY_WRITE,
    USER_READ,
    USER_WRITE,
    ROUTING_READ,
    ROUTING_WRITE,
    WEBHOOK_READ,
    WEBHOOK_WRITE,
    ADMIN,
    MANDATE_READ,
    PAYMENT_METHOD_READ,
    PAYMENT_METHOD_WRITE,
    CONNECTOR_READ,
    CONNECTOR_WRITE,
    MERCHANT_READ,
    MERCHANT_WRITE,
    ORGANIZATION_READ,
    ORGANIZATION_WRITE,
    PROFILE_READ,
    PROFILE_WRITE,
    // Granular values appended after section 6.2 audit; existing numeric values preserved.
    REFUND_READ,
    MANDATE_WRITE,
    FILE_READ,
    FILE_WRITE,
    DISPUTE_READ,
    DISPUTE_WRITE,
    SUBSCRIPTION_READ,
    SUBSCRIPTION_WRITE,
    FRM_WRITE,
};

/** @brief Instance functor mapping a permission to its stable wire name. */
class AppPermissionName
{
public:
    [[nodiscard]] std::string_view operator()(AppPermission permission) const noexcept
    {
        switch (permission) {
            case AppPermission::PAYMENT_READ:
                return "PAYMENT_READ";
            case AppPermission::PAYMENT_WRITE:
                return "PAYMENT_WRITE";
            case AppPermission::REFUND_WRITE:
                return "REFUND_WRITE";
            case AppPermission::API_KEY_READ:
                return "API_KEY_READ";
            case AppPermission::API_KEY_WRITE:
                return "API_KEY_WRITE";
            case AppPermission::USER_READ:
                return "USER_READ";
            case AppPermission::USER_WRITE:
                return "USER_WRITE";
            case AppPermission::ROUTING_READ:
                return "ROUTING_READ";
            case AppPermission::ROUTING_WRITE:
                return "ROUTING_WRITE";
            case AppPermission::WEBHOOK_READ:
                return "WEBHOOK_READ";
            case AppPermission::WEBHOOK_WRITE:
                return "WEBHOOK_WRITE";
            case AppPermission::ADMIN:
                return "ADMIN";
            case AppPermission::MANDATE_READ:
                return "MANDATE_READ";
            case AppPermission::PAYMENT_METHOD_READ:
                return "PAYMENT_METHOD_READ";
            case AppPermission::PAYMENT_METHOD_WRITE:
                return "PAYMENT_METHOD_WRITE";
            case AppPermission::CONNECTOR_READ:
                return "CONNECTOR_READ";
            case AppPermission::CONNECTOR_WRITE:
                return "CONNECTOR_WRITE";
            case AppPermission::MERCHANT_READ:
                return "MERCHANT_READ";
            case AppPermission::MERCHANT_WRITE:
                return "MERCHANT_WRITE";
            case AppPermission::ORGANIZATION_READ:
                return "ORGANIZATION_READ";
            case AppPermission::ORGANIZATION_WRITE:
                return "ORGANIZATION_WRITE";
            case AppPermission::PROFILE_READ:
                return "PROFILE_READ";
            case AppPermission::PROFILE_WRITE:
                return "PROFILE_WRITE";
            case AppPermission::REFUND_READ:
                return "REFUND_READ";
            case AppPermission::MANDATE_WRITE:
                return "MANDATE_WRITE";
            case AppPermission::FILE_READ:
                return "FILE_READ";
            case AppPermission::FILE_WRITE:
                return "FILE_WRITE";
            case AppPermission::DISPUTE_READ:
                return "DISPUTE_READ";
            case AppPermission::DISPUTE_WRITE:
                return "DISPUTE_WRITE";
            case AppPermission::SUBSCRIPTION_READ:
                return "SUBSCRIPTION_READ";
            case AppPermission::SUBSCRIPTION_WRITE:
                return "SUBSCRIPTION_WRITE";
            case AppPermission::FRM_WRITE:
                return "FRM_WRITE";
        }
        return std::string_view{};
    }
};

} // namespace app::auth

#ifdef CONGELADO_TEST
namespace app::auth::tests {
using namespace boost::ut;

suite<"app_auth_permission"> permission_suite = []
{
    "names every permission with its wire value"_test = []
    {
        const AppPermissionName name;
        expect(name(AppPermission::PAYMENT_READ) == "PAYMENT_READ");
        expect(name(AppPermission::PAYMENT_WRITE) == "PAYMENT_WRITE");
        expect(name(AppPermission::REFUND_WRITE) == "REFUND_WRITE");
        expect(name(AppPermission::API_KEY_READ) == "API_KEY_READ");
        expect(name(AppPermission::API_KEY_WRITE) == "API_KEY_WRITE");
        expect(name(AppPermission::USER_READ) == "USER_READ");
        expect(name(AppPermission::USER_WRITE) == "USER_WRITE");
        expect(name(AppPermission::ROUTING_READ) == "ROUTING_READ");
        expect(name(AppPermission::ROUTING_WRITE) == "ROUTING_WRITE");
        expect(name(AppPermission::WEBHOOK_READ) == "WEBHOOK_READ");
        expect(name(AppPermission::WEBHOOK_WRITE) == "WEBHOOK_WRITE");
        expect(name(AppPermission::ADMIN) == "ADMIN");
        expect(name(AppPermission::MANDATE_READ) == "MANDATE_READ");
        expect(name(AppPermission::PAYMENT_METHOD_READ) == "PAYMENT_METHOD_READ");
        expect(name(AppPermission::PAYMENT_METHOD_WRITE) == "PAYMENT_METHOD_WRITE");
        expect(name(AppPermission::CONNECTOR_READ) == "CONNECTOR_READ");
        expect(name(AppPermission::CONNECTOR_WRITE) == "CONNECTOR_WRITE");
        expect(name(AppPermission::MERCHANT_READ) == "MERCHANT_READ");
        expect(name(AppPermission::MERCHANT_WRITE) == "MERCHANT_WRITE");
        expect(name(AppPermission::ORGANIZATION_READ) == "ORGANIZATION_READ");
        expect(name(AppPermission::ORGANIZATION_WRITE) == "ORGANIZATION_WRITE");
        expect(name(AppPermission::PROFILE_READ) == "PROFILE_READ");
        expect(name(AppPermission::PROFILE_WRITE) == "PROFILE_WRITE");
        expect(name(AppPermission::REFUND_READ) == "REFUND_READ");
        expect(name(AppPermission::MANDATE_WRITE) == "MANDATE_WRITE");
        expect(name(AppPermission::FILE_READ) == "FILE_READ");
        expect(name(AppPermission::FILE_WRITE) == "FILE_WRITE");
        expect(name(AppPermission::DISPUTE_READ) == "DISPUTE_READ");
        expect(name(AppPermission::DISPUTE_WRITE) == "DISPUTE_WRITE");
        expect(name(AppPermission::SUBSCRIPTION_READ) == "SUBSCRIPTION_READ");
        expect(name(AppPermission::SUBSCRIPTION_WRITE) == "SUBSCRIPTION_WRITE");
        expect(name(AppPermission::FRM_WRITE) == "FRM_WRITE");
    };
};

} // namespace app::auth::tests
#endif
