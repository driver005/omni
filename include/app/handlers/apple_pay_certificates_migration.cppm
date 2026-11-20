module;
export module app_handlers:apple_pay_certificates_migration;

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
import app_auth;
import app_crypto;
import app_metrics;
import app_store;
import app_keystore;

namespace app::handlers::apple_pay_certificates_migration {

/** @brief Local stand-in for upstream `types::storage::MerchantKeyStore`.
 *
 *  Upstream carries `merchant_id` plus the decrypted `key` loaded through
 *  `get_merchant_key_store_by_merchant_id` (core/apple_pay_certificates_migration.rs:33-37).
 *  `key()` is the local secret-safe material passed to the crypto operation in
 *  place of `key_store.key.get_inner().peek()` (core:72).
 */
class MerchantKeyStore
{
public:
    MerchantKeyStore(std::string merchant_id, app::crypto::SecretString key) noexcept :
        m_merchant_id{std::move(merchant_id)},
        m_key{std::move(key)}
    {
    }
    [[nodiscard]] const std::string& merchant_id() const noexcept { return m_merchant_id; }
    [[nodiscard]] const app::crypto::SecretString& key() const noexcept { return m_key; }

private:
    std::string m_merchant_id{};
    app::crypto::SecretString m_key{};
};

/** @brief One `(MCA, ConnectorWalletDetailsUpdate)` pair.
 *
 *  Upstream builds `mca_to_update: Vec<(MerchantConnectorAccount,
 *  MerchantConnectorAccountUpdateInternal)>` (core:49,79-84) and hands the whole
 *  vector to `update_multiple_merchant_connector_accounts` (core:88-91).
 */
struct MerchantConnectorAccountUpdate
{
    ::app::store::merchant_connectors::MerchantConnectorRecord account{};
    std::string encrypted_connector_wallets_details{};
};

} // namespace app::handlers::apple_pay_certificates_migration

export namespace app::handlers::apple_pay_certificates_migration {

class ApplePayCertificatesMigrationRouteHandlers
{
public:
    void handle_apple_pay_certificates_migration(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // server_wrap begins by recording the flow, before auth/body handling
        // (routes/apple_pay_certificates_migration.rs:17-28).
        m_context->metrics().record_flow("ApplePayCertificatesMigration");

        // AdminApiAuth is modeled by the PLATFORM_ADMIN/ADMIN rule registered for
        // this operation (app/auth/registry.cppm:38); upstream
        // routes/apple_pay_certificates_migration.rs:26. LockAction::NotApplicable
        // (routes:27) means no lock is acquired — there is deliberately no
        // locks().acquire(...) call in this handler.
        const auto rule = m_context->auth_rules().rule_for("apple_pay_certificates_migration");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        // ApiEventMetric on the request type (api_models/apple_pay_certificates_migration.rs:12).
        m_context->metrics().counter("api_event.APPLE_PAY_CERTIFICATES_MIGRATION").increment();

        // web::Json<ApplePayCertificatesMigrationRequest> (routes:13-15). The
        // DTO mirrors the upstream serde model exactly — Vec<MerchantId> is a
        // plain JSON string array (api_models:7-10). No generic-tree / wrapper
        // fallback: upstream rejects any other shape.
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto request = serde::Ser::deserialize<app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationRequest>(
            "application/json", body
        );
        if (!request) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        const auto& merchant_id_list = request->get_merchant_ids();

        // Upstream core/apple_pay_certificates_migration.rs:27-30.
        std::vector<std::string> migration_successful_merchant_ids{};
        std::vector<std::string> migration_failed_merchant_ids{};

        for (const auto& merchant_id : merchant_id_list) {
            // `state.store.get_master_key().to_vec().into()` (core:32). The local
            // master key is the configured key-store secret with a deterministic
            // domain-separated fallback (in-process substitute for the master-key
            // store; no external key manager).
            const auto key_store_master_key = get_master_key();

            // `get_merchant_key_store_by_merchant_id(...).change_context(
            // InternalServerError)?` (core:33-37). A failed load is fatal for the
            // whole request, not a per-merchant failure.
            auto key_store = get_merchant_key_store_by_merchant_id(merchant_id, key_store_master_key);
            if (!key_store) {
                send_error(res, send, core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INTERNAL,
                    "Unable to load merchant key store"));
                return;
            }

            // `find_merchant_connector_account_by_merchant_id_and_disabled_list(
            // merchant_id, true, &key_store).to_not_found_response(
            // InternalServerError)?` (core:39-47). get_disabled=true selects every
            // MCA for the merchant (storage_impl/merchant_connector_account.rs:
            // 1560-1596). An empty result is NOT an error upstream.
            auto merchant_connector_accounts =
                find_merchant_connector_account_by_merchant_id_and_disabled_list(merchant_id, *key_store);
            if (!merchant_connector_accounts) {
                send_error(res, send, core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INTERNAL,
                    "Unable to load merchant connector accounts"));
                return;
            }

            // core:49.
            std::vector<MerchantConnectorAccountUpdate> mca_to_update{};

            for (const auto& connector_account : *merchant_connector_accounts) {
                // `helpers::get_applepay_metadata(connector_account.metadata)
                // .map_err(|e| logger::error!("... {:?} ...", connector_name, e)).ok()`
                // (core:51-61; helper at core/payments/helpers.rs:7065-7095).
                // A parse failure logs the connector name and skips this MCA.
                auto apple_pay_metadata = get_applepay_metadata(connector_account.get_metadata());
                if (!apple_pay_metadata) {
                    m_context->metrics().counter(
                        "apple_pay_certificates_migration.metadata_parse_failed."
                        + connector_account.get_connector_name()
                    ).increment();
                    continue;
                }

                // `domain_types::crypto_operation(&(&state).into(),
                // type_name!(storage::MerchantConnectorAccount),
                // CryptoOperation::Encrypt(Secret::new(serde_json::to_value(
                // apple_pay_metadata)?)), Identifier::Merchant(merchant_id.clone()),
                // key_store.key.get_inner().peek()).await .and_then(try_into_operation)
                // .change_context(InternalServerError).attach_printable(
                // "Unable to encrypt connector apple pay metadata")?` (core:63-77).
                // AesWrap is the local in-process crypto primitive substitute; the
                // Identifier::Merchant binding is applied when deriving the merchant
                // key below.
                auto encrypted = crypto_operation_encrypt(*apple_pay_metadata, *key_store);
                if (!encrypted) {
                    send_error(res, send, core::error::AppErrorBuilder{}(
                        core::error::AppErrorKind::INTERNAL,
                        "Unable to encrypt connector apple pay metadata"));
                    return;
                }

                // `storage::MerchantConnectorAccountUpdate::
                // ConnectorWalletDetailsUpdate { connector_wallets_details: encrypted }`
                // (core:79-82; hyperswitch_domain_models/merchant_connector_account.rs:
                // 540-542). Only connector_wallets_details is updated — metadata is
                // untouched.
                MerchantConnectorAccountUpdate update{};
                update.account = connector_account;
                update.encrypted_connector_wallets_details = std::move(*encrypted);
                mca_to_update.push_back(std::move(update));
            }

            // `update_multiple_merchant_connector_accounts(mca_to_update).await`
            // runs all updates in one DB transaction (storage_impl/
            // merchant_connector_account.rs:823-850): empty list → Ok; any failed
            // write → Err and the whole merchant is marked failed.
            const bool updated = update_multiple_merchant_connector_accounts(mca_to_update);
            if (updated) {
                migration_successful_merchant_ids.push_back(merchant_id);
                m_context->metrics().counter("APPLE_PAY_CERTIFICATES_MIGRATED").increment();
            } else {
                migration_failed_merchant_ids.push_back(merchant_id);
            }
        }

        // `ApplicationResponse::Json(ApplePayCertificatesMigrationResponse {
        // migration_successful, migration_failed })` (core:108-113).
        app::dto::apple_pay_certificates_migration::ApplePayCertificatesMigrationResponse response{};
        response.set_migration_successful(std::move(migration_successful_merchant_ids));
        response.set_migration_failed(std::move(migration_failed_merchant_ids));

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/apple_pay_certificates_migration"}
            .add_route(
                Route{""}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_apple_pay_certificates_migration(req, res, std::move(send));
                    })
                    .summary("Hyperswitch apple_pay_certificates_migration")
            );
    }

    explicit ApplePayCertificatesMigrationRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    /** @brief `state.store.get_master_key().to_vec().into()` — core:32. */
    [[nodiscard]] app::crypto::SecretString get_master_key() const
    {
        if (const auto configured = m_context->key_store().get(
                "apple_pay_certificates_migration", "master_key")) {
            return *configured;
        }
        return app::crypto::SecretString{"apple-pay-certificates-migration.master_key"};
    }

    /** @brief `get_merchant_key_store_by_merchant_id` — core:33-37.
     *
     *  The merchant's stored key is the KeyStore `merchant_key` when present;
     *  otherwise it is deterministically derived from persisted merchant-account
     *  material. The derivation binds master key + merchant_id (the local
     *  `Identifier::Merchant`) so each merchant gets a distinct key, matching the
     *  per-merchant key selection of upstream `Identifier::Merchant(merchant_id)`.
     *  A missing merchant account models a failed key-store load (fatal upstream).
     */
    [[nodiscard]] std::optional<MerchantKeyStore> get_merchant_key_store_by_merchant_id(
        std::string_view merchant_id,
        const app::crypto::SecretString& master_key
    ) const
    {
        const auto account = m_context->repository().find_merchant_account_by_string(merchant_id);
        if (!account) {
            return std::nullopt;
        }

        std::string key_material{};
        if (const auto merchant_secret = m_context->key_store().get("merchant_key", merchant_id)) {
            key_material = merchant_secret->value();
        } else if (!account->get_payment_response_hash_key().empty()) {
            key_material = account->get_payment_response_hash_key();
        } else if (!account->get_publishable_key().empty()) {
            key_material = account->get_publishable_key();
        } else {
            key_material = "apple-pay-migration:" + std::string{merchant_id};
        }

        // Identifier::Merchant(merchant_id) — core:71. Domain-separated,
        // master-key-bound derivation (local key-manager substitute).
        std::string identifier_input{master_key.value()};
        identifier_input.push_back('|');
        identifier_input.append(merchant_id);
        identifier_input.push_back('|');
        identifier_input.append(key_material);
        const app::crypto::SecretString merchant_key{
            app::crypto::KeyedHasher{"apple_pay_certificates_migration.merchant_key"}
                .hash(identifier_input)
        };
        return MerchantKeyStore{std::string{merchant_id}, merchant_key};
    }

    /** @brief `find_merchant_connector_account_by_merchant_id_and_disabled_list(
     *  merchant_id, true, &key_store)` — core:39-47.
     *
     *  get_disabled=true includes every MCA belonging to the merchant
     *  (storage_impl/merchant_connector_account.rs:1571-1579). Empty is a valid
     *  `Ok(vec![])` — upstream `to_not_found_response` maps a NotFound *error*,
     *  not an empty list, so an empty list later yields a successful bulk update.
     */
    [[nodiscard]] std::optional<std::vector<::app::store::merchant_connectors::MerchantConnectorRecord>>
    find_merchant_connector_account_by_merchant_id_and_disabled_list(
        std::string_view merchant_id,
        const MerchantKeyStore& /*key_store*/
    ) const
    {
        std::vector<::app::store::merchant_connectors::MerchantConnectorRecord> accounts{};
        for (const auto& connector : m_context->repository().list_merchant_connectors()) {
            if (connector.get_merchant_id() == merchant_id) {
                accounts.push_back(connector);
            }
        }
        return accounts;
    }

    /** @brief Port of `helpers::get_applepay_metadata` —
     *  core/payments/helpers.rs:7065-7095.
     *
     *  Tries `ApplepayCombinedSessionTokenData` (`apple_pay_combined`) first and
     *  falls back to `ApplepaySessionTokenData` (`apple_pay`). On success the
     *  selected enum variant is returned JSON-serialized with snake_case naming
     *  (api_models/payments.rs:11243-11248), which is exactly the value upstream
     *  feeds to `serde_json::to_value` before encryption (core:67).
     */
    [[nodiscard]] std::optional<std::string> get_applepay_metadata(std::string_view metadata) const
    {
        auto parsed = serde::Ser::decode_generic("application/json", metadata);
        if (!parsed) {
            return std::nullopt;
        }
        const auto& root = parsed->get();
        const auto* object = std::get_if<interfaces::Value::Object>(&root);
        if (object == nullptr) {
            return std::nullopt;
        }

        // ApplePayCombined(ApplePayCombinedWrapper) — the flattened optional
        // metadata accepts any object under `apple_pay_combined`.
        if (const auto combined = object->get("apple_pay_combined")) {
            const auto& value = combined.value();
            if (!std::holds_alternative<interfaces::Value::Object>(value.get())) {
                return std::nullopt;
            }
            return "{\"apple_pay_combined\":" + serde::Ser::encode_json(value) + "}";
        }

        // ApplePay(ApplePayMetadata) — requires payment_request_data and
        // session_token_data (api_models/payments.rs:11251-11254).
        if (const auto legacy = object->get("apple_pay")) {
            const auto& value = legacy.value();
            const auto* legacy_object = std::get_if<interfaces::Value::Object>(&value.get());
            if (legacy_object == nullptr
                || !legacy_object->get("payment_request_data")
                || !legacy_object->get("session_token_data")) {
                return std::nullopt;
            }
            return "{\"apple_pay\":" + serde::Ser::encode_json(value) + "}";
        }

        return std::nullopt;
    }

    /** @brief Port of `domain_types::crypto_operation(Encrypt, Identifier::Merchant,
     *  key_store.key, serde_json::to_value(apple_pay_metadata))` — core:63-77.
     *
     *  `apple_pay_metadata_json` is already the serialized enum variant, i.e.
     *  the upstream `serde_json::to_value` result. AesWrap is the local in-process
     *  cipher substitute; the merchant-scoped key was derived with the
     *  Identifier::Merchant binding in get_merchant_key_store_by_merchant_id.
     */
    [[nodiscard]] std::optional<std::string> crypto_operation_encrypt(
        std::string_view apple_pay_metadata_json,
        const MerchantKeyStore& key_store
    ) const
    {
        return app::crypto::AesWrap{}.encrypt(apple_pay_metadata_json, key_store.key().value());
    }

    /** @brief Port of `update_multiple_merchant_connector_accounts` —
     *  core:88-91; storage_impl/merchant_connector_account.rs:823-850.
     *
     *  Upstream updates only `connector_wallets_details` for each MCA inside one
     *  transaction. The local durable store (repository `configs` table) holds
     *  the encrypted wallet details under a deterministic per-MCA key until a
     *  first-class `connector_wallets_details` column is available on
     *  MerchantConnectorRecord (see Shared contract requests). Failure of any
     *  write fails the whole merchant; an empty list succeeds, matching the
     *  empty-transaction behavior upstream.
     */
    [[nodiscard]] bool update_multiple_merchant_connector_accounts(
        const std::vector<MerchantConnectorAccountUpdate>& updates
    ) const
    {
        if (updates.empty()) {
            return true;
        }
        auto repository = m_context->repository();
        for (const auto& update : updates) {
            const std::string key{
                "apple_pay_certificates_migration.connector_wallets_details:"
                + update.account.get_merchant_connector_id()
            };
            repository.set_config(key, update.encrypted_connector_wallets_details);
            const auto stored = repository.get_config(key);
            if (!stored || stored.value() != update.encrypted_connector_wallets_details) {
                return false;
            }
        }
        return true;
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::apple_pay_certificates_migration
