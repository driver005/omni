export module app_auth:apple_pay_certificates_migration_handler;

import std;
import interfaces;
import core_router;
import core_behavior;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_context_core;
import app_crypto;
import app_metrics;
import app_context_core;
import app_keystore;

import :apple_pay_certificates_migration_input;
import :apple_pay_certificates_migration_result;
import :apple_pay_certificates_migration_runtime;
import :apple_pay_certificates_migration_handlers_v1;
import :apple_pay_certificates_migration_handlers_v2;

export namespace app::handlers::apple_pay_certificates_migration {

/** @brief Facade ("handler of handlers").
 *
 *  Owns the single shared runtime and request-support objects; version
 *  partitions only hold references to them. No business logic lives here.
 */
class ApplePayCertificatesMigrationRouteHandlers
{
public:
    explicit ApplePayCertificatesMigrationRouteHandlers(
        ::app::context::AppContext& context
    ) noexcept :
        m_runtime{context},
        m_policy{context},
        m_request_support{m_policy},
        m_v1{context, m_runtime, m_request_support},
        m_v2{}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v1.install_v1(router);
        m_v2.install_v2(router);
    }

private:
    ApplePayCertificatesMigrationRuntime m_runtime;
    ApplePayCertificatesMigrationPolicy m_policy;
    core::behavior::RequestSupport<ApplePayCertificatesMigrationPolicy> m_request_support;
    ApplePayCertificatesMigrationHandlersV1 m_v1;
    ApplePayCertificatesMigrationHandlersV2 m_v2;
};

} // namespace app::handlers::apple_pay_certificates_migration
