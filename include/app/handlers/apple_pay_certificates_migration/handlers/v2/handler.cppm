export module app_handlers_apple_pay_certificates_migration:handlers_v2;

import std;
import interfaces;
import core_router;

/** @brief v2 partition for the apple_pay_certificates_migration domain.
 *
 *  Upstream registers exactly one route for this domain: POST on scope
 *  `/apple_pay_certificates_migration` under `#[cfg(all(feature = "olap",
 *  feature = "v1"))]` (routes/app.rs:2459-2467). There is no `/v2/...` scope and
 *  no v2 handler registration. This partition therefore installs nothing; it
 *  exists only to keep the facade's versioned composition uniform. Adding a
 *  route here would change the API surface, so install_v2() is a deliberate
 *  no-op.
 */
export namespace app::handlers::apple_pay_certificates_migration {

class ApplePayCertificatesMigrationHandlersV2
{
public:
    ApplePayCertificatesMigrationHandlersV2() noexcept = default;

    void install_v2(core::router::RouterContext<>& /*router*/) noexcept {}
};

} // namespace app::handlers::apple_pay_certificates_migration
