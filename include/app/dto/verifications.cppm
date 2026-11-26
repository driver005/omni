module;
export module app_dto:verifications;

import core_serde;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::verifications {

class ApplepayMerchantResponse : public serde::Serializer<ApplepayMerchantResponse>
{
public:
    void set_status_message(std::string value)
    {
        status_message = std::move(value);
    }

    [[nodiscard]] const std::string& get_status_message() const noexcept
    {
        return status_message;
    }

    std::string status_message{};
};

class ApplepayMerchantVerificationConfigs :
    public serde::Serializer<ApplepayMerchantVerificationConfigs>
{
public:
    void set_domain_names(std::vector<std::string> value)
    {
        domain_names = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_domain_names() const noexcept
    {
        return domain_names;
    }

    void set_encrypt_to(std::string value)
    {
        encrypt_to = std::move(value);
    }

    [[nodiscard]] const std::string& get_encrypt_to() const noexcept
    {
        return encrypt_to;
    }

    void set_partner_internal_merchant_identifier(std::string value)
    {
        partner_internal_merchant_identifier = std::move(value);
    }

    [[nodiscard]] const std::string& get_partner_internal_merchant_identifier() const noexcept
    {
        return partner_internal_merchant_identifier;
    }

    void set_partner_merchant_name(std::string value)
    {
        partner_merchant_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_partner_merchant_name() const noexcept
    {
        return partner_merchant_name;
    }

    std::vector<std::string> domain_names{};
    std::string encrypt_to{};
    std::string partner_internal_merchant_identifier{};
    std::string partner_merchant_name{};
};

class ApplepayVerifiedDomainsResponse : public serde::Serializer<ApplepayVerifiedDomainsResponse>
{
public:
    void set_verified_domains(std::vector<std::string> value)
    {
        verified_domains = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_verified_domains() const noexcept
    {
        return verified_domains;
    }

    std::vector<std::string> verified_domains{};
};

class MerchantConnectorAccountIdValue : public serde::Serializer<MerchantConnectorAccountIdValue>
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

class MerchantIdValue : public serde::Serializer<MerchantIdValue>
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

class ApplepayGetVerifiedDomainsParam : public serde::Serializer<ApplepayGetVerifiedDomainsParam>
{
public:
    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_merchant_connector_account_id(MerchantConnectorAccountIdValue value)
    {
        merchant_connector_account_id = std::move(value);
    }

    [[nodiscard]] const MerchantConnectorAccountIdValue&
    get_merchant_connector_account_id() const noexcept
    {
        return merchant_connector_account_id;
    }

    MerchantIdValue merchant_id{};
    MerchantConnectorAccountIdValue merchant_connector_account_id{};
};

class ApplepayMerchantVerificationRequest :
    public serde::Serializer<ApplepayMerchantVerificationRequest>
{
public:
    void set_domain_names(std::vector<std::string> value)
    {
        domain_names = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_domain_names() const noexcept
    {
        return domain_names;
    }

    void set_merchant_connector_account_id(MerchantConnectorAccountIdValue value)
    {
        merchant_connector_account_id = std::move(value);
    }

    [[nodiscard]] const MerchantConnectorAccountIdValue&
    get_merchant_connector_account_id() const noexcept
    {
        return merchant_connector_account_id;
    }

    std::vector<std::string> domain_names{};
    MerchantConnectorAccountIdValue merchant_connector_account_id{};
};
} // namespace app::dto::verifications

template<>
struct serde::Serializable<app::dto::verifications::ApplepayMerchantVerificationConfigs>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "domainNames",
                &app::dto::verifications::ApplepayMerchantVerificationConfigs::get_domain_names,
                &app::dto::verifications::ApplepayMerchantVerificationConfigs::set_domain_names,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "encryptTo",
                &app::dto::verifications::ApplepayMerchantVerificationConfigs::get_encrypt_to,
                &app::dto::verifications::ApplepayMerchantVerificationConfigs::set_encrypt_to,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "partnerInternalMerchantIdentifier",
                &app::dto::verifications::ApplepayMerchantVerificationConfigs::
                    get_partner_internal_merchant_identifier,
                &app::dto::verifications::ApplepayMerchantVerificationConfigs::
                    set_partner_internal_merchant_identifier,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "partnerMerchantName",
                &app::dto::verifications::ApplepayMerchantVerificationConfigs::
                    get_partner_merchant_name,
                &app::dto::verifications::ApplepayMerchantVerificationConfigs::
                    set_partner_merchant_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "applepay_merchant_verification_configs";
    }
};

template<>
struct serde::Serializable<app::dto::verifications::ApplepayMerchantVerificationRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "domain_names",
                &app::dto::verifications::ApplepayMerchantVerificationRequest::get_domain_names,
                &app::dto::verifications::ApplepayMerchantVerificationRequest::set_domain_names,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_account_id",
                &app::dto::verifications::ApplepayMerchantVerificationRequest::
                    get_merchant_connector_account_id,
                &app::dto::verifications::ApplepayMerchantVerificationRequest::
                    set_merchant_connector_account_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "applepay_merchant_verification_request";
    }
};

template<>
struct serde::Serializable<app::dto::verifications::ApplepayMerchantResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "status_message",
            &app::dto::verifications::ApplepayMerchantResponse::get_status_message,
            &app::dto::verifications::ApplepayMerchantResponse::set_status_message,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "applepay_merchant_response";
    }
};

template<>
struct serde::Serializable<app::dto::verifications::ApplepayGetVerifiedDomainsParam>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::dto::verifications::ApplepayGetVerifiedDomainsParam::get_merchant_id,
                &app::dto::verifications::ApplepayGetVerifiedDomainsParam::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_account_id",
                &app::dto::verifications::ApplepayGetVerifiedDomainsParam::
                    get_merchant_connector_account_id,
                &app::dto::verifications::ApplepayGetVerifiedDomainsParam::
                    set_merchant_connector_account_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "applepay_get_verified_domains_param";
    }
};

template<>
struct serde::Serializable<app::dto::verifications::ApplepayVerifiedDomainsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "verified_domains",
            &app::dto::verifications::ApplepayVerifiedDomainsResponse::get_verified_domains,
            &app::dto::verifications::ApplepayVerifiedDomainsResponse::set_verified_domains,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "applepay_verified_domains_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::verifications::tests {
using namespace boost::ut;
suite<"app_dto_verifications"> app_dto_verifications_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<
                         app::dto::verifications::ApplepayMerchantVerificationConfigs>::fields())> >
            0
        );
    };
};
} // namespace app::dto::verifications::tests
#endif
