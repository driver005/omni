module;
export module app_dto:enums;

import core_serde;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::enums {

enum class BillingConnectors
{
    Chargebee,
    Recurly,
    Stripebilling,
    Custombilling,
    DummyBillingConnector,
};

enum class FieldType
{
    UserCardNumber,
    UserGiftCardNumber,
    UserCardExpiryMonth,
    UserCardExpiryYear,
    UserCardCvc,
    UserGiftCardPin,
    UserCardNetwork,
    UserFullName,
    UserEmailAddress,
    UserPhoneNumber,
    UserPhoneNumberCountryCode,
    UserCountry,
    UserCurrency,
    UserCryptoCurrencyNetwork,
    UserBillingName,
    UserAddressLine1,
    UserAddressLine2,
    UserAddressCity,
    UserAddressPincode,
    UserAddressState,
    UserAddressCountry,
    UserShippingName,
    UserShippingAddressLine1,
    UserShippingAddressLine2,
    UserShippingAddressCity,
    UserShippingAddressPincode,
    UserShippingAddressState,
    UserShippingAddressCountry,
    UserDocumentType,
    UserSocialSecurityNumber,
    UserBlikCode,
    UserBank,
    UserBankOptions,
    UserBankAccountNumber,
    UserSourceBankAccountId,
    UserDestinationBankAccountId,
    Text,
    DropDown,
    UserDateOfBirth,
    UserVpaId,
    LanguagePreference,
    UserPixKey,
    UserCpf,
    UserCnpj,
    UserIban,
    UserBsbNumber,
    UserBankSortCode,
    UserBankRoutingNumber,
    UserBankType,
    UserBankAccountHolderName,
    UserMsisdn,
    UserClientIdentifier,
    OrderDetailsProductName,
    UserBranchCode,
    UserBankIdentifier,
    UserPixAccountNumber,
};

enum class FrmAction
{
    CancelTxn,
    AutoRefund,
    ManualReview,
};

enum class FrmConnectors
{
    Signifyd,
    Riskified,
    Cybersourcedecisionmanager,
    SanlamPayshield,
    Nsure,
};

enum class FrmPreferredFlowTypes
{
    Pre,
    Post,
};

enum class ManualUpdateIntentStatus
{
    Succeeded,
    Failed,
    PartiallyCaptured,
    RequiresCapture,
    PartiallyAuthorizedAndRequiresCapture,
    PartiallyCapturedAndCapturable,
};

enum class PayoutConnectors
{
    Adyen,
    Adyenplatform,
    Checkout,
    Cybersource,
    Deutschebank,
    Ebanx,
    Gigadat,
    GotymeSanlam,
    Loonio,
    Mifinity,
    Nomupay,
    Nuvei,
    Payone,
    Paypal,
    Stripe,
    Truelayer,
    Trustly,
    Wise,
    Worldpay,
    Worldpayxml,
    Envoy,
    Itaubank,
    Santander,
};

enum class PmAuthConnectors
{
    Plaid,
};

enum class ReconPermissionScope
{
    Read,
    Write,
};

enum class RetryAction
{
    ManualRetry,
    Requeue,
};

enum class RoutingAlgorithm
{
    RoundRobin,
    MaxConversion,
    MinCost,
    Custom,
};

enum class SetOrUnset
{
    Set,
    Unset,
};

enum class SurchargeConnectors
{
    Interpayments,
};

enum class TaxConnectors
{
    Taxjar,
};

enum class TokenStatus
{
    Active,
    Inactive,
    Suspended,
    Expired,
    Deleted,
};

class UnresolvedResponseReason : public serde::Serializer<UnresolvedResponseReason>
{
public:
    void set_code(std::string value)
    {
        code = std::move(value);
    }

    [[nodiscard]] const std::string& get_code() const noexcept
    {
        return code;
    }

    void set_message(std::string value)
    {
        message = std::move(value);
    }

    [[nodiscard]] const std::string& get_message() const noexcept
    {
        return message;
    }

    std::string code{};
    std::string message{};
};

enum class UpdateActiveAttempt
{
    Set,
    Unset,
    NoAction,
};

enum class VaultConnectors
{
    Vgs,
    HyperswitchVault,
    Tokenex,
};
} // namespace app::dto::enums

template<>
struct serde::Serializable<app::dto::enums::UnresolvedResponseReason>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "code",
                &app::dto::enums::UnresolvedResponseReason::get_code,
                &app::dto::enums::UnresolvedResponseReason::set_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "message",
                &app::dto::enums::UnresolvedResponseReason::get_message,
                &app::dto::enums::UnresolvedResponseReason::set_message,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "unresolved_response_reason";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::enums::tests {
using namespace boost::ut;
suite<"app_dto_enums"> app_dto_enums_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<decltype(serde::Serializable<
                                       app::dto::enums::UnresolvedResponseReason>::fields())> > 0
        );
    };
};
} // namespace app::dto::enums::tests
#endif
