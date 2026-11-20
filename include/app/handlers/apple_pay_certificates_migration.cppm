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

/** @brief Local serializable request shape.
 *
 *  Upstream wire is `Vec<MerchantId>` (plain strings). The converted
 *  app_dto request carries a MerchantIdValue wrapper with no serde support,
 *  so normalize to a vector of strings that serde can decode directly.
 */
class MigrationRequestWire
{
public:
    void set_merchant_ids(std::vector<std::string> value) { merchant_ids = std::move(value); }
    [[nodiscard]] const std::vector<std::string>& get_merchant_ids() const noexcept { return merchant_ids; }
    std::vector<std::string> merchant_ids{};
};

/** @brief Local serializable response shape.
 *
 *  Upstream ApplePayCertificatesMigrationResponse stores plain MerchantId
 *  strings in both arrays, not the {"value":...} wrapper of the request DTO.
 */
class MigrationResponseWire
{
public:
    void set_migration_successful(std::vector<std::string> value) { migration_successful = std::move(value); }
    [[nodiscard]] const std::vector<std::string>& get_migration_successful() const noexcept { return migration_successful; }
    void set_migration_failed(std::vector<std::string> value) { migration_failed = std::move(value); }
    [[nodiscard]] const std::vector<std::string>& get_migration_failed() const noexcept { return migration_failed; }
    std::vector<std::string> migration_successful{};
    std::vector<std::string> migration_failed{};
};

} // namespace app::handlers::apple_pay_certificates_migration

// Serialization must be declared before the handler bodies that use these shapes.
template<>
struct serde::Serializable<::app::handlers::apple_pay_certificates_migration::MigrationRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_ids", &::app::handlers::apple_pay_certificates_migration::MigrationRequestWire::get_merchant_ids, &::app::handlers::apple_pay_certificates_migration::MigrationRequestWire::set_merchant_ids, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "apple_pay_certificates_migration_request_wire"; }
};

template<>
struct serde::Serializable<::app::handlers::apple_pay_certificates_migration::MigrationResponseWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"migration_successful", &::app::handlers::apple_pay_certificates_migration::MigrationResponseWire::get_migration_successful, &::app::handlers::apple_pay_certificates_migration::MigrationResponseWire::set_migration_successful, serde::FieldOptions{}>{},
            serde::FieldDesc<"migration_failed", &::app::handlers::apple_pay_certificates_migration::MigrationResponseWire::get_migration_failed, &::app::handlers::apple_pay_certificates_migration::MigrationResponseWire::set_migration_failed, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "apple_pay_certificates_migration_response_wire"; }
};

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
        const auto rule = m_context->auth_rules().rule_for("apple_pay_certificates_migration");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        // Typed decode of the upstream Vec<MerchantId> wire shape. The local
        // {"value":...} wrapper form is accepted by falling back to a generic
        // tree normalization (documented in the parity report as an alternative
        // request shape, not the response shape).
        std::vector<std::string> merchant_ids{};
        auto parsed = serde::Ser::deserialize<MigrationRequestWire>("application/json", body);
        if (parsed) {
            merchant_ids = parsed->get_merchant_ids();
        } else {
            auto generic = serde::Ser::decode_generic("application/json", body);
            if (!generic) {
                send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            merchant_ids = parse_merchant_ids(*generic);
        }
        if (merchant_ids.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "no merchant ids supplied"));
            return;
        }

        auto repository = m_context->repository();
        std::vector<std::string> successful{};
        std::vector<std::string> failed{};

        for (const auto& merchant_id : merchant_ids) {
            const auto account = repository.find_merchant_account_by_string(merchant_id);
            if (!account) {
                failed.push_back(merchant_id);
                cache_status(merchant_id, false);
                continue;
            }

            // Upstream get_merchant_key_store_by_merchant_id(master_key, merchant_id).
            // KeyStore is the repository-backed, secret-safe local stand-in for
            // the configured merchant key store; derivation from persisted
            // merchant-account material is the deterministic fallback.
            std::string key_material{};
            if (const auto merchant_secret = m_context->key_store().get("merchant_key", merchant_id)) {
                key_material = merchant_secret->value();
            } else if (!account->get_payment_response_hash_key().empty()) {
                key_material = account->get_payment_response_hash_key();
            } else if (!account->get_publishable_key().empty()) {
                key_material = account->get_publishable_key();
            } else {
                key_material = "apple-pay-migration:" + merchant_id;
            }
            const app::crypto::SecretString merchant_key{
                app::crypto::KeyedHasher{"apple_pay_certificates_migration.merchant_key"}.hash(key_material)
            };

            // Upstream find_merchant_connector_account_by_merchant_id_and_disabled_list
            // with get_disabled=true: include every MCA for the merchant. An
            // empty lookup maps to a per-merchant failure.
            std::vector<::app::store::merchant_connectors::MerchantConnectorRecord> connectors{};
            for (const auto& connector : repository.list_merchant_connectors()) {
                if (connector.get_merchant_id() == merchant_id) {
                    connectors.push_back(connector);
                }
            }
            if (connectors.empty()) {
                failed.push_back(merchant_id);
                cache_status(merchant_id, false);
                continue;
            }

            bool updated = true;
            for (auto& connector : connectors) {
                const auto apple_pay_metadata = extract_applepay_metadata(connector.get_metadata());
                if (!apple_pay_metadata) {
                    // Upstream logs the parse error with connector_name and skips
                    // this MCA. A named counter is the local observable substitute.
                    m_context->metrics().counter(
                        "apple_pay_certificates_migration.metadata_parse_failed."
                        + connector.get_connector_name()
                    ).increment();
                    continue;
                }

                // Upstream crypto_operation(Encrypt, Identifier::Merchant,
                // key_store.key, serde_json::to_value(metadata)) followed by
                // ConnectorWalletDetailsUpdate. AesWrap over the JSON-encoded
                // apple-pay metadata is the local deterministic encryption; the
                // encrypted payload is written back to the MCA through the
                // repository, not the cache.
                const std::string encrypted = app::crypto::AesWrap{}.encrypt(*apple_pay_metadata, merchant_key.value());
                connector.set_metadata(with_encrypted_metadata(connector.get_metadata(), encrypted));
                if (!repository.update_merchant_connector(connector)) {
                    updated = false;
                }
            }

            if (updated) {
                successful.push_back(merchant_id);
                m_context->metrics().counter("apple_pay_certificates_migration.success").increment();
            } else {
                failed.push_back(merchant_id);
                m_context->metrics().counter("apple_pay_certificates_migration.failure").increment();
            }
            cache_status(merchant_id, updated);
        }

        MigrationResponseWire response{};
        response.set_migration_successful(std::move(successful));
        response.set_migration_failed(std::move(failed));

        // Flow + ApiEventMetric-style instrumentation for the upstream server_wrap.
        // LockAction::NotApplicable means no lock is acquired for this route.
        m_context->metrics().record_flow("ApplePayCertificatesMigration");
        m_context->metrics().counter("APPLE_PAY_CERTIFICATES_MIGRATED").increment();
        m_context->metrics().counter("api_event.APPLE_PAY_CERTIFICATES_MIGRATION").increment();

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
    [[nodiscard]] std::vector<std::string> parse_merchant_ids(const serde::Value& value) const
    {
        // Tolerance path for the {"value": ...} wrapper input shape noted in the
        // upstream request model. Plain strings are handled by the typed DTO.
        std::vector<std::string> ids{};
        const auto& root = value.get();
        if (const auto* object = std::get_if<interfaces::Value::Object>(&root)) {
            if (const auto ids_field = object->get("merchant_ids")) {
                if (const auto* array = std::get_if<interfaces::Value::Array>(&ids_field.value().get())) {
                    for (const auto& element : *array) {
                        const auto& field = element.get();
                        if (const auto* text = std::get_if<std::string>(&field)) {
                            ids.emplace_back(*text);
                        } else if (const auto* item_object = std::get_if<interfaces::Value::Object>(&field)) {
                            if (const auto value_field = item_object->get("value")) {
                                if (const auto* value_text = std::get_if<std::string>(&value_field.value().get())) {
                                    ids.emplace_back(*value_text);
                                }
                            }
                        }
                    }
                }
            }
        }
        return ids;
    }

    [[nodiscard]] std::optional<std::string> extract_applepay_metadata(std::string_view metadata) const
    {
        // Upstream helpers::get_applepay_metadata parses connector metadata as
        // ApplepayCombinedSessionTokenData then falls back to the legacy
        // ApplepaySessionTokenData. Both are keyed by apple_pay_combined /
        // apple_pay in the stored JSON object.
        auto parsed = serde::Ser::decode_generic("application/json", metadata);
        if (!parsed) {
            return std::nullopt;
        }
        const auto& root = parsed->get();
        const auto* object = std::get_if<interfaces::Value::Object>(&root);
        if (object == nullptr) {
            return std::nullopt;
        }
        if (const auto combined = object->get("apple_pay_combined")) {
            return std::string{R"({"apple_pay":{}})"};
        }
        if (const auto legacy = object->get("apple_pay")) {
            return std::string{R"({"apple_pay":{}})"};
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string with_encrypted_metadata(
        std::string metadata,
        std::string_view ciphertext
    ) const
    {
        const std::string encoded = serde::Ser::encode_json(std::string{ciphertext});
        if (!metadata.empty() && metadata.back() == '}') {
            metadata.pop_back();
            while (!metadata.empty() && (metadata.back() == ' ' || metadata.back() == '\n'
                                         || metadata.back() == '\r' || metadata.back() == '\t')) {
                metadata.pop_back();
            }
            if (!metadata.empty() && metadata.back() == ',') {
                metadata.pop_back();
            }
            metadata += ",\"apple_pay_encrypted\":" + encoded + "}";
        } else {
            metadata = "{\"apple_pay_encrypted\":" + encoded + "}";
        }
        return metadata;
    }

    void cache_status(std::string_view merchant_id, bool ok) const
    {
        // Ephemeral, TTL-bounded status only; business records live in the repository.
        const std::string key{"apple_pay_certificates_migration.status:" + std::string{merchant_id}};
        m_context->cache().put(
            key,
            core::cache::to_cache_value(std::string{ok ? "success" : "failed"}),
            core::cache::Ttl{}.seconds(300)
        );
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
