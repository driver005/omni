export module app_auth:apple_pay_certificates_migration_runtime_migrate;

import std;
import core_error;
import app_context;
import app_dto;
import app_metrics;
import app_context_core;

import :apple_pay_certificates_migration_input;
import :apple_pay_certificates_migration_result;
import :apple_pay_certificates_migration_runtime_common;

export namespace app::handlers::apple_pay_certificates_migration {

class ApplePayCertificatesMigrationMigrateRuntime
{
public:
    explicit ApplePayCertificatesMigrationMigrateRuntime(
        ::app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: migration ─────────────────────────
    // core/apple_pay_certificates_migration.rs:27-113.
    [[nodiscard]] std::expected<ApplePayCertificatesMigrationResult, core::error::AppError>
    migrate(const ApplePayCertificatesMigrationInput& input) const
    {
        const auto& merchant_id_list = input.request.get_merchant_ids();

        // Upstream core:27-30.
        std::vector<std::string> migration_successful_merchant_ids{};
        std::vector<std::string> migration_failed_merchant_ids{};

        for (const auto& merchant_id: merchant_id_list) {
            // `state.store.get_master_key().to_vec().into()` (core:32).
            const auto key_store_master_key = m_support.get_master_key();

            // `get_merchant_key_store_by_merchant_id(...).change_context(
            // InternalServerError)?` (core:33-37). A failed load is fatal for the
            // whole request, not a per-merchant failure.
            auto key_store =
                m_support.get_merchant_key_store_by_merchant_id(merchant_id, key_store_master_key);
            if (!key_store) {
                return std::unexpected{core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INTERNAL,
                    "Unable to load merchant key store"
                )};
            }

            // `find_merchant_connector_account_by_merchant_id_and_disabled_list(
            // merchant_id, true, &key_store).to_not_found_response(
            // InternalServerError)?` (core:39-47). Empty result is NOT an error.
            auto merchant_connector_accounts =
                m_support.find_merchant_connector_account_by_merchant_id_and_disabled_list(
                    merchant_id,
                    *key_store
                );
            if (!merchant_connector_accounts) {
                return std::unexpected{core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INTERNAL,
                    "Unable to load merchant connector accounts"
                )};
            }

            // core:49.
            std::vector<MerchantConnectorAccountUpdate> mca_to_update{};

            for (const auto& connector_account: *merchant_connector_accounts) {
                // `helpers::get_applepay_metadata(connector_account.metadata)
                // .map_err(|e| logger::error!("... {:?} ...", connector_name, e)).ok()`
                // (core:51-61). A parse failure logs the connector name and skips this MCA.
                auto apple_pay_metadata =
                    m_support.get_applepay_metadata(connector_account.get_metadata());
                if (!apple_pay_metadata) {
                    m_context.metrics()
                        .counter(
                            "apple_pay_certificates_migration.metadata_parse_failed." +
                            connector_account.get_connector_name()
                        )
                        .increment();
                    continue;
                }

                // `domain_types::crypto_operation(... Encrypt ...)` (core:63-77).
                auto encrypted =
                    m_support.crypto_operation_encrypt(*apple_pay_metadata, *key_store);
                if (!encrypted) {
                    return std::unexpected{core::error::AppErrorBuilder{}(
                        core::error::AppErrorKind::INTERNAL,
                        "Unable to encrypt connector apple pay metadata"
                    )};
                }

                // `MerchantConnectorAccountUpdate::ConnectorWalletDetailsUpdate`
                // (core:79-82). Only connector_wallets_details is updated.
                MerchantConnectorAccountUpdate update{};
                update.account = connector_account;
                update.encrypted_connector_wallets_details = std::move(*encrypted);
                mca_to_update.push_back(std::move(update));
            }

            // `update_multiple_merchant_connector_accounts(mca_to_update).await`
            // runs all updates in one DB transaction (core:88-91): empty list → Ok;
            // any failed write → Err and the whole merchant is marked failed.
            const bool updated =
                m_support.update_multiple_merchant_connector_accounts(mca_to_update);
            if (updated) {
                migration_successful_merchant_ids.push_back(merchant_id);
                m_context.metrics().counter("APPLE_PAY_CERTIFICATES_MIGRATED").increment();
            } else {
                migration_failed_merchant_ids.push_back(merchant_id);
            }
        }

        // `ApplicationResponse::Json(ApplePayCertificatesMigrationResponse {
        // migration_successful, migration_failed })` (core:108-113).
        ::app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationResponse
            response{};
        response.set_migration_successful(std::move(migration_successful_merchant_ids));
        response.set_migration_failed(std::move(migration_failed_merchant_ids));
        return ApplePayCertificatesMigrationResult{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    ApplePayCertificatesMigrationRuntimeSupport m_support;
};

} // namespace app::handlers::apple_pay_certificates_migration
