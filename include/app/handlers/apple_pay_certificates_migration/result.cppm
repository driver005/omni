export module app_handlers_apple_pay_certificates_migration:result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::apple_pay_certificates_migration {

// The response wire fields come from the exact DTO the old core built and
// encoded, so field names and order stay byte-identical. The Serializable
// specialization reuses the DTO's FieldDesc tuple for the derived object.
class ApplePayCertificatesMigrationResult :
    public app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationResponse
{
public:
    ApplePayCertificatesMigrationResult() = default;

    explicit ApplePayCertificatesMigrationResult(
        app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationResponse value
    ) noexcept :
        ApplePayCertificatesMigrationResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::apple_pay_certificates_migration

template<>
struct serde::Serializable<
    app::handlers::apple_pay_certificates_migration::ApplePayCertificatesMigrationResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::apple_pay_certificates_migration::
                                       ApplePayCertificatesMigrationResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "apple_pay_certificates_migration_result";
    }
};
