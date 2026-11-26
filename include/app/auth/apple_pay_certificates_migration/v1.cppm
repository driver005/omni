export module app_auth:apple_pay_certificates_migration_handlers_v1;

import std;
import interfaces;
import core_router;
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

export namespace app::handlers::apple_pay_certificates_migration {

/** @brief v1 route family. Owns the single upstream POST route and its adapter.
 *
 *  Upstream registers the scope under `#[cfg(all(feature = "olap", feature =
 *  "v1"))]` (routes/app.rs:2459). The gate is compile-time scaffolding: this port
 *  always installs the route, matching a release build with the features on.
 */
class ApplePayCertificatesMigrationHandlersV1
{
public:
    ApplePayCertificatesMigrationHandlersV1(
        ::app::context::AppContext& ctx,
        ApplePayCertificatesMigrationRuntime& runtime,
        ApplePayCertificatesMigrationInputSupport& input_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_apple_pay_certificates_migration(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // server_wrap begins by recording the flow, before auth/body handling
        // (routes/apple_pay_certificates_migration.rs:17-28).
        m_context.metrics().record_flow("ApplePayCertificatesMigration");

        // AdminApiAuth is modeled by the PLATFORM_ADMIN/ADMIN rule registered for
        // this operation (app/auth/registry.cppm:38); upstream
        // routes/apple_pay_certificates_migration.rs:26. LockAction::NotApplicable
        // (routes:27) means no lock is acquired.
        if (const auto auth = m_input_support.authorize(req)) {
            const auto& error = *auth;
            error.send(res, send);
            return;
        }

        // ApiEventMetric on the request type (api_models/apple_pay_certificates_migration.rs:12).
        m_context.metrics().counter("api_event.APPLE_PAY_CERTIFICATES_MIGRATION").increment();

        // web::Json<ApplePayCertificatesMigrationRequest> (routes:13-15). Vec<MerchantId>
        // is a plain JSON string array. No generic-tree / wrapper fallback.
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        using Input =
            ::app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        ApplePayCertificatesMigrationInput input{*parsed};

        auto result = m_runtime.migrate(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    /** @brief Installs the single upstream route.
     *
     *  Upstream registers this scope under `#[cfg(all(feature = "olap", feature =
     *  "v1"))]` (routes/app.rs:2459) and gates the core function with
     *  `#[cfg(feature = "v1")]` (core/apple_pay_certificates_migration.rs:17).
     *  Both are compile-time gates: they emit no bytes and create no runtime
     *  branch. This port always installs the route (`Route{""}` + `.post`), which
     *  is the identical surface of a release build with both features compiled
     *  in; no status, header, body, auth or store outcome depends on the gate
     *  (no wire bytes differ). There is therefore no behavior to port here.
     */
    void install_v1(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "apple_pay_certificates_migration"}.add_route(
            Route{""}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_apple_pay_certificates_migration(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch apple_pay_certificates_migration")
        );
    }

private:
    ::app::context::AppContext& m_context;
    ApplePayCertificatesMigrationRuntime& m_runtime;
    ApplePayCertificatesMigrationInputSupport& m_input_support;
};

} // namespace app::handlers::apple_pay_certificates_migration
