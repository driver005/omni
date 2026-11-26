module;
export module app_dto:verify_connector;

import core_serde;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

/**
 * Typed wire shapes for the upstream `POST /account/connectors/verify` request.
 *
 * Upstream reference (READ-ONLY, SHA 7736d55adaf235f7af74e133ae408573b1826823):
 *  - api_models/src/verify_connector.rs:5-9    VerifyConnectorRequest { connector_name,
 * connector_account_details }
 *  - api_models/src/enums.rs:8                 re-export of common_enums Connector
 *  - common_enums/src/connector_enums.rs:29-32 #[serde(rename_all = "snake_case")] enum Connector
 *  - api_models/src/admin.rs:1582-1615         #[serde(tag = "auth_type")] enum ConnectorAuthType
 *
 * The previous generated wrappers carried each value inside a `{"value": ...}` object, which
 * does not match the upstream wire (`connector_name` is a plain snake_case string and
 * `connector_account_details` is an internally-tagged object). These types are the DTO-layer
 * change that lets the handler decode the real upstream shape with `serde::Ser::deserialize`.
 */
export namespace app::dto::verify_connector {

/** @brief Upstream `api_models::enums::Connector` (snake_case wire names). */
enum class Connector
{
    stripe,
    paypal,
    aci,
    adyen,
    adyenplatform,
    affirm,
    airwallex,
    amazonpay,
    archipel,
    authorizedotnet,
    bambora,
    bamboraapac,
    bankofamerica,
    barclaycard,
    betterpayment,
    billwerk,
    bitpay,
    bluesnap,
    blackhawknetwork,
    calida,
    boku,
    braintree,
    breadpay,
    cardinal,
    cashtocode,
    celero,
    chargebee,
    checkbook,
    checkout,
    citigate,
    coinbase,
    coingate,
    custombilling,
    cryptopay,
    ctp_mastercard,
    ctp_visa,
    cybersource,
    cybersourcedecisionmanager,
    d24,
    datatrans,
    deutschebank,
    digitalvirgo,
    dlocal,
    dwolla,
    ebanx,
    ecommpay,
    envoy,
    elavon,
    etisalat,
    elavon_pg,
    facilitapay,
    finix,
    fiserv,
    fiservemea,
    fiservcommercehub,
    fiuu,
    flexiti,
    forte,
    getnet,
    gigadat,
    givepayments,
    globalpay,
    globalpayments_heartland,
    globalpayments_realex,
    globepay,
    gocardless,
    gotyme_sanlam,
    gpayments,
    hipay,
    helcim,
    hyperpg,
    hyperswitch_vault,
    interpayments,
    inespay,
    iatapay,
    ilixium,
    imerchantsolutions,
    itaubank,
    jpmorgan,
    jpmorgan_orbital,
    juspay,
    juspaythreedsserver,
    klarna,
    loonio,
    merchante,
    mifinity,
    mollie,
    moneris,
    multisafepay,
    netcetera,
    nexinets,
    nexixpay,
    nmi,
    nomupay,
    noon,
    nordea,
    novalnet,
    nuvei,
    opennode,
    paybox,
    paydotcom,
    payhound,
    payload,
    payme,
    paynearme,
    payone,
    paysafe,
    paystack,
    paytm,
    payconex,
    payu,
    peachpayments,
    payjustnow,
    payjustnowinstore,
    phonepe,
    placetopay,
    powertranz,
    prophetpay,
    rapyd,
    razorpay,
    recurly,
    redsys,
    revolv3,
    saferpay,
    santander,
    shift4,
    silverflow,
    square,
    stax,
    stripebilling,
    taxjar,
    threedsecureio,
    tesouro,
    tokenex,
    tokenio,
    trustly,
    truelayer,
    trustpay,
    trustpayments,
    tsys,
    tsys_transit,
    vgs,
    volt,
    wellsfargo,
    wise,
    worldline,
    worldpay,
    worldpayraft,
    worldpayvantiv,
    worldpayxml,
    worldpaymodular,
    signifyd,
    nsure,
    plaid,
    riskified,
    sanlam_payshield,
    xendit,
    zen,
    zift,
    zsl,
};

/** @brief Wire-name table mirroring upstream `#[serde(rename_all = "snake_case")]`. */
struct ConnectorName : public serde::Serializer<ConnectorName>
{
    std::string_view wire{};
    Connector value{Connector::stripe};
};

inline constexpr std::array<ConnectorName, 161> kConnectorNames{{
    {{}, "stripe", Connector::stripe},
    {{}, "paypal", Connector::paypal},
    {{}, "aci", Connector::aci},
    {{}, "adyen", Connector::adyen},
    {{}, "adyenplatform", Connector::adyenplatform},
    {{}, "affirm", Connector::affirm},
    {{}, "airwallex", Connector::airwallex},
    {{}, "amazonpay", Connector::amazonpay},
    {{}, "archipel", Connector::archipel},
    {{}, "authorizedotnet", Connector::authorizedotnet},
    {{}, "bambora", Connector::bambora},
    {{}, "bamboraapac", Connector::bamboraapac},
    {{}, "bankofamerica", Connector::bankofamerica},
    {{}, "barclaycard", Connector::barclaycard},
    {{}, "betterpayment", Connector::betterpayment},
    {{}, "billwerk", Connector::billwerk},
    {{}, "bitpay", Connector::bitpay},
    {{}, "bluesnap", Connector::bluesnap},
    {{}, "blackhawknetwork", Connector::blackhawknetwork},
    {{}, "calida", Connector::calida},
    {{}, "bluecode", Connector::calida},
    {{}, "boku", Connector::boku},
    {{}, "braintree", Connector::braintree},
    {{}, "breadpay", Connector::breadpay},
    {{}, "cardinal", Connector::cardinal},
    {{}, "cashtocode", Connector::cashtocode},
    {{}, "celero", Connector::celero},
    {{}, "chargebee", Connector::chargebee},
    {{}, "checkbook", Connector::checkbook},
    {{}, "checkout", Connector::checkout},
    {{}, "citigate", Connector::citigate},
    {{}, "coinbase", Connector::coinbase},
    {{}, "coingate", Connector::coingate},
    {{}, "custombilling", Connector::custombilling},
    {{}, "cryptopay", Connector::cryptopay},
    {{}, "ctp_mastercard", Connector::ctp_mastercard},
    {{}, "ctp_visa", Connector::ctp_visa},
    {{}, "cybersource", Connector::cybersource},
    {{}, "cybersourcedecisionmanager", Connector::cybersourcedecisionmanager},
    {{}, "d24", Connector::d24},
    {{}, "datatrans", Connector::datatrans},
    {{}, "deutschebank", Connector::deutschebank},
    {{}, "digitalvirgo", Connector::digitalvirgo},
    {{}, "dlocal", Connector::dlocal},
    {{}, "dwolla", Connector::dwolla},
    {{}, "ebanx", Connector::ebanx},
    {{}, "ecommpay", Connector::ecommpay},
    {{}, "envoy", Connector::envoy},
    {{}, "elavon", Connector::elavon},
    {{}, "etisalat", Connector::etisalat},
    {{}, "elavon_pg", Connector::elavon_pg},
    {{}, "facilitapay", Connector::facilitapay},
    {{}, "finix", Connector::finix},
    {{}, "fiserv", Connector::fiserv},
    {{}, "fiservemea", Connector::fiservemea},
    {{}, "fiservcommercehub", Connector::fiservcommercehub},
    {{}, "fiuu", Connector::fiuu},
    {{}, "flexiti", Connector::flexiti},
    {{}, "forte", Connector::forte},
    {{}, "getnet", Connector::getnet},
    {{}, "gigadat", Connector::gigadat},
    {{}, "givepayments", Connector::givepayments},
    {{}, "globalpay", Connector::globalpay},
    {{}, "globalpayments_heartland", Connector::globalpayments_heartland},
    {{}, "globalpayments_realex", Connector::globalpayments_realex},
    {{}, "globepay", Connector::globepay},
    {{}, "gocardless", Connector::gocardless},
    {{}, "gotyme_sanlam", Connector::gotyme_sanlam},
    {{}, "gpayments", Connector::gpayments},
    {{}, "hipay", Connector::hipay},
    {{}, "helcim", Connector::helcim},
    {{}, "hyperpg", Connector::hyperpg},
    {{}, "hyperswitch_vault", Connector::hyperswitch_vault},
    {{}, "interpayments", Connector::interpayments},
    {{}, "inespay", Connector::inespay},
    {{}, "iatapay", Connector::iatapay},
    {{}, "ilixium", Connector::ilixium},
    {{}, "imerchantsolutions", Connector::imerchantsolutions},
    {{}, "itaubank", Connector::itaubank},
    {{}, "jpmorgan", Connector::jpmorgan},
    {{}, "jpmorgan_orbital", Connector::jpmorgan_orbital},
    {{}, "juspay", Connector::juspay},
    {{}, "juspaythreedsserver", Connector::juspaythreedsserver},
    {{}, "klarna", Connector::klarna},
    {{}, "loonio", Connector::loonio},
    {{}, "merchante", Connector::merchante},
    {{}, "mifinity", Connector::mifinity},
    {{}, "mollie", Connector::mollie},
    {{}, "moneris", Connector::moneris},
    {{}, "multisafepay", Connector::multisafepay},
    {{}, "netcetera", Connector::netcetera},
    {{}, "nexinets", Connector::nexinets},
    {{}, "nexixpay", Connector::nexixpay},
    {{}, "nmi", Connector::nmi},
    {{}, "nomupay", Connector::nomupay},
    {{}, "noon", Connector::noon},
    {{}, "nordea", Connector::nordea},
    {{}, "novalnet", Connector::novalnet},
    {{}, "nuvei", Connector::nuvei},
    {{}, "opennode", Connector::opennode},
    {{}, "paybox", Connector::paybox},
    {{}, "paydotcom", Connector::paydotcom},
    {{}, "payhound", Connector::payhound},
    {{}, "payload", Connector::payload},
    {{}, "payme", Connector::payme},
    {{}, "paynearme", Connector::paynearme},
    {{}, "payone", Connector::payone},
    {{}, "paysafe", Connector::paysafe},
    {{}, "paystack", Connector::paystack},
    {{}, "paytm", Connector::paytm},
    {{}, "payconex", Connector::payconex},
    {{}, "payu", Connector::payu},
    {{}, "peachpayments", Connector::peachpayments},
    {{}, "payjustnow", Connector::payjustnow},
    {{}, "payjustnowinstore", Connector::payjustnowinstore},
    {{}, "phonepe", Connector::phonepe},
    {{}, "placetopay", Connector::placetopay},
    {{}, "powertranz", Connector::powertranz},
    {{}, "prophetpay", Connector::prophetpay},
    {{}, "rapyd", Connector::rapyd},
    {{}, "razorpay", Connector::razorpay},
    {{}, "recurly", Connector::recurly},
    {{}, "redsys", Connector::redsys},
    {{}, "revolv3", Connector::revolv3},
    {{}, "saferpay", Connector::saferpay},
    {{}, "santander", Connector::santander},
    {{}, "shift4", Connector::shift4},
    {{}, "silverflow", Connector::silverflow},
    {{}, "square", Connector::square},
    {{}, "stax", Connector::stax},
    {{}, "stripebilling", Connector::stripebilling},
    {{}, "taxjar", Connector::taxjar},
    {{}, "threedsecureio", Connector::threedsecureio},
    {{}, "tesouro", Connector::tesouro},
    {{}, "tokenex", Connector::tokenex},
    {{}, "tokenio", Connector::tokenio},
    {{}, "trustly", Connector::trustly},
    {{}, "truelayer", Connector::truelayer},
    {{}, "trustpay", Connector::trustpay},
    {{}, "trustpayments", Connector::trustpayments},
    {{}, "tsys", Connector::tsys},
    {{}, "tsys_transit", Connector::tsys_transit},
    {{}, "vgs", Connector::vgs},
    {{}, "volt", Connector::volt},
    {{}, "wellsfargo", Connector::wellsfargo},
    {{}, "wise", Connector::wise},
    {{}, "worldline", Connector::worldline},
    {{}, "worldpay", Connector::worldpay},
    {{}, "worldpayraft", Connector::worldpayraft},
    {{}, "worldpayvantiv", Connector::worldpayvantiv},
    {{}, "worldpayxml", Connector::worldpayxml},
    {{}, "worldpaymodular", Connector::worldpaymodular},
    {{}, "signifyd", Connector::signifyd},
    {{}, "nsure", Connector::nsure},
    {{}, "plaid", Connector::plaid},
    {{}, "riskified", Connector::riskified},
    {{}, "sanlam_payshield", Connector::sanlam_payshield},
    {{}, "xendit", Connector::xendit},
    {{}, "zen", Connector::zen},
    {{}, "zift", Connector::zift},
    {{}, "zsl", Connector::zsl},
}};

/** @brief Parse an upstream snake_case connector wire name into the typed enum. */
[[nodiscard]] inline std::optional<Connector> connector_from_wire(std::string_view wire) noexcept
{
    for (const auto& entry: kConnectorNames) {
        if (entry.wire == wire) {
            return entry.value;
        }
    }
    return std::nullopt;
}

/** @brief Render a typed Connector back to its upstream snake_case wire name. */
[[nodiscard]] inline std::string_view connector_to_wire(Connector value) noexcept
{
    // First entry wins so serde aliases (e.g. "bluecode" -> calida) map to the canonical name.
    for (const auto& entry: kConnectorNames) {
        if (entry.value == value) {
            return entry.wire;
        }
    }
    return {};
}

/**
 * @brief Upstream `admin::ConnectorAuthType` represented as the internally-tagged object
 * (admin.rs:1583 `#[serde(tag = "auth_type")]`). All secret fields are plain strings at the
 * DTO boundary; the handler wraps them in app::crypto::SecretString for the domain value.
 */
class ConnectorAuthTypeValue : public serde::Serializer<ConnectorAuthTypeValue>
{
public:
    void set_auth_type(std::string value)
    {
        auth_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_auth_type() const noexcept
    {
        return auth_type;
    }

    void set_api_key(std::string value)
    {
        api_key = std::move(value);
    }

    [[nodiscard]] const std::string& get_api_key() const noexcept
    {
        return api_key;
    }

    void set_key1(std::string value)
    {
        key1 = std::move(value);
    }

    [[nodiscard]] const std::string& get_key1() const noexcept
    {
        return key1;
    }

    void set_api_secret(std::string value)
    {
        api_secret = std::move(value);
    }

    [[nodiscard]] const std::string& get_api_secret() const noexcept
    {
        return api_secret;
    }

    void set_key2(std::string value)
    {
        key2 = std::move(value);
    }

    [[nodiscard]] const std::string& get_key2() const noexcept
    {
        return key2;
    }

    void set_certificate(std::string value)
    {
        certificate = std::move(value);
    }

    [[nodiscard]] const std::string& get_certificate() const noexcept
    {
        return certificate;
    }

    void set_private_key(std::string value)
    {
        private_key = std::move(value);
    }

    [[nodiscard]] const std::string& get_private_key() const noexcept
    {
        return private_key;
    }

    void set_auth_key_map(std::string value)
    {
        auth_key_map = std::move(value);
    }

    [[nodiscard]] const std::string& get_auth_key_map() const noexcept
    {
        return auth_key_map;
    }

    std::string auth_type{};
    std::string api_key{};
    std::string key1{};
    std::string api_secret{};
    std::string key2{};
    std::string certificate{};
    std::string private_key{};
    std::string auth_key_map{};
};

/** @brief Kept for source compatibility with the previous generated wrapper. */
class ConnectorValue : public serde::Serializer<ConnectorValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

/** @brief Upstream `VerifyConnectorRequest` (api_models/src/verify_connector.rs:5-9). */
class VerifyConnectorRequest : public serde::Serializer<VerifyConnectorRequest>
{
public:
    void set_connector_name(std::string value)
    {
        connector_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_name() const noexcept
    {
        return connector_name;
    }

    void set_connector_account_details(ConnectorAuthTypeValue value)
    {
        connector_account_details = std::move(value);
    }

    [[nodiscard]] const ConnectorAuthTypeValue& get_connector_account_details() const noexcept
    {
        return connector_account_details;
    }

    std::string connector_name{};
    ConnectorAuthTypeValue connector_account_details{};
};

} // namespace app::dto::verify_connector

template<>
struct serde::Serializable<app::dto::verify_connector::ConnectorAuthTypeValue>
{
    static constexpr auto fields()
    {
        using Type = app::dto::verify_connector::ConnectorAuthTypeValue;
        return std::tuple{
            serde::FieldDesc<
                "auth_type",
                &Type::get_auth_type,
                &Type::set_auth_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "api_key",
                &Type::get_api_key,
                &Type::set_api_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"key1", &Type::get_key1, &Type::set_key1, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "api_secret",
                &Type::get_api_secret,
                &Type::set_api_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"key2", &Type::get_key2, &Type::set_key2, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "certificate",
                &Type::get_certificate,
                &Type::set_certificate,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "private_key",
                &Type::get_private_key,
                &Type::set_private_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "auth_key_map",
                &Type::get_auth_key_map,
                &Type::set_auth_key_map,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "verify_connector_auth_type_value";
    }
};

template<>
struct serde::Serializable<app::dto::verify_connector::VerifyConnectorRequest>
{
    static constexpr auto fields()
    {
        using Type = app::dto::verify_connector::VerifyConnectorRequest;
        return std::tuple{
            serde::FieldDesc<
                "connector_name",
                &Type::get_connector_name,
                &Type::set_connector_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_account_details",
                &Type::get_connector_account_details,
                &Type::set_connector_account_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "verify_connector_request";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::verify_connector::tests {
using namespace boost::ut;
suite<"app_dto_verify_connector"> app_dto_verify_connector_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<
                         app::dto::verify_connector::VerifyConnectorRequest>::fields())> > 0
        );
    };
    "parses upstream snake_case connector names"_test = []
    {
        expect(connector_from_wire("stripe").has_value());
        expect(connector_from_wire("paypal").has_value());
        expect(connector_from_wire("ctp_mastercard").has_value());
        expect(!connector_from_wire("not_a_connector").has_value());
        expect(connector_to_wire(Connector::stripe) == "stripe");
    };
};
} // namespace app::dto::verify_connector::tests
#endif
