export module app_auth:apple_pay_certificates_migration_runtime;

import std;
import core_behavior;
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

    [[nodiscard]] core::behavior::Outcome<
        ::app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationResponse>
    migrate(
        const ::app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationRequest&
            request,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_migrate.migrate(request, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    ApplePayCertificatesMigrationMigrateRuntime m_migrate;
};

} // namespace app::handlers::apple_pay_certificates_migration
