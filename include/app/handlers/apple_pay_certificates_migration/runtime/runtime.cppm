export module app_handlers_apple_pay_certificates_migration:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_migrate;

export namespace app::handlers::apple_pay_certificates_migration {

class ApplePayCertificatesMigrationRuntime
{
public:
    explicit ApplePayCertificatesMigrationRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_migrate{context}
    {
    }

    [[nodiscard]] std::expected<ApplePayCertificatesMigrationResult, core::error::AppError>
    migrate(const ApplePayCertificatesMigrationInput& input) const
    {
        return m_migrate.migrate(input);
    }

private:
    app::context::AppContext& m_context;
    ApplePayCertificatesMigrationMigrateRuntime m_migrate;
};

} // namespace app::handlers::apple_pay_certificates_migration
