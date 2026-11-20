module;
export module app_dto:apple_pay_certificates_migration;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

// Upstream api_models/apple_pay_certificates_migration.rs:1-10 uses
// `Vec<common_utils::id_type::MerchantId>` for every id field. MerchantId is a
// transparent newtype, so the wire shape is a plain JSON string array — not the
// {"value": ...} wrapper used by many other converted DTOs. Mirror that exactly.
export namespace app::dto::apple_pay_certificates_migration {

class ApplePayCertificatesMigrationRequest
{
public:
    void set_merchant_ids(std::vector<std::string> value) { merchant_ids = std::move(value); }
    [[nodiscard]] const std::vector<std::string>& get_merchant_ids() const noexcept { return merchant_ids; }
    std::vector<std::string> merchant_ids{};
};

class ApplePayCertificatesMigrationResponse
{
public:
    void set_migration_successful(std::vector<std::string> value) { migration_successful = std::move(value); }
    [[nodiscard]] const std::vector<std::string>& get_migration_successful() const noexcept { return migration_successful; }
    void set_migration_failed(std::vector<std::string> value) { migration_failed = std::move(value); }
    [[nodiscard]] const std::vector<std::string>& get_migration_failed() const noexcept { return migration_failed; }
    std::vector<std::string> migration_successful{};
    std::vector<std::string> migration_failed{};
};
} // namespace app::dto::apple_pay_certificates_migration

template<>
struct serde::Serializable<app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"migration_successful", &app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationResponse::get_migration_successful, &app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationResponse::set_migration_successful, serde::FieldOptions{}>{},
            serde::FieldDesc<"migration_failed", &app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationResponse::get_migration_failed, &app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationResponse::set_migration_failed, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "apple_pay_certificates_migration_response"; }
};

template<>
struct serde::Serializable<app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_ids", &app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationRequest::get_merchant_ids, &app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationRequest::set_merchant_ids, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "apple_pay_certificates_migration_request"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::apple_pay_certificates_migration::tests {
using namespace boost::ut;
suite<"app_dto_apple_pay_certificates_migration"> app_dto_apple_pay_certificates_migration_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationResponse>::fields())> > 0);
    };
};
}
#endif
