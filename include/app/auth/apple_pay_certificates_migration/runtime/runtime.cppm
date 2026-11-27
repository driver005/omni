export module app_auth:apple_pay_certificates_migration_runtime;

import std;
import core_error;
import app_context;
import app_dto;

import :apple_pay_certificates_migration_input;
import :apple_pay_certificates_migration_result;
import :apple_pay_certificates_migration_runtime_common;
import :apple_pay_certificates_migration_runtime_migrate;

export namespace app::handlers::apple_pay_certificates_migration {

class ApplePayCertificatesMigrationRuntime
{
public:
    explicit ApplePayCertificatesMigrationRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_migrate{context}
    {
    }

    [[nodiscard]] std::expected<
        ::app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationResponse,
        core::error::AppError>
    migrate(const ApplePayCertificatesMigrationInput& input) const
    {
        return m_migrate.migrate(input);
    }

private:
    ::app::context::AppContext& m_context;
    ApplePayCertificatesMigrationMigrateRuntime m_migrate;
};

} // namespace app::handlers::apple_pay_certificates_migration
