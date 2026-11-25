export module app_handlers_payment_methods:runtime_common;

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
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

export namespace app::handlers::payment_methods {

/** @brief Shared business helpers for the payment-method sub-runtimes. */
class PaymentMethodsRuntimeSupport
{
public:
    explicit PaymentMethodsRuntimeSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] core::error::AppError
    error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    /** @brief Local error kind/status substitute for upstream variants with no local kind. */
    [[nodiscard]] core::error::AppError
    local_error(std::string_view wire_name, std::string_view message) const
    {
        core::error::AppError result{};
        result.set_kind(core::error::AppErrorKind::INVALID_REQUEST);
        result.set_kind_name(std::string{wire_name});
        result.set_message(std::string{message});
        return result;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    check_merchant_connector(std::string_view merchant_id) const
    {
        const auto connectors = m_context.repository().list_merchant_connectors();
        if (connectors.empty()) {
            return std::nullopt;
        }
        for (const auto& mca: connectors) {
            if (std::string_view{mca.get_merchant_id()} == merchant_id) {
                return std::nullopt;
            }
        }
        return error(core::error::AppErrorKind::NOT_FOUND, "merchant connector account not found");
    }

    [[nodiscard]] std::string client_secret_hash(std::string_view secret) const
    {
        return app::crypto::KeyedHasher{"payment_methods.client_secret"}.hash(secret);
    }

    void remember_client_secret(std::string_view key, std::string_view secret) const
    {
        m_context.cache().put(
            std::string{"payment_methods.client_secret_hash:"} + std::string{key},
            core::cache::to_cache_value(client_secret_hash(secret))
        );
        m_context.cache().put(
            "payment_methods.client_secret:" + std::string{key},
            core::cache::to_cache_value(std::string{secret})
        );
    }

    [[nodiscard]] std::string encryption_material() const
    {
        const auto key = m_context.key_store().get("payment_methods", "data");
        return key ? std::string{key->value()} : std::string{"payment-methods-local-data-key"};
    }

    [[nodiscard]] std::string encrypt_data(std::string_view plaintext) const
    {
        return app::crypto::AesWrap{}.encrypt(plaintext, encryption_material());
    }

    [[nodiscard]] std::string decrypt_data(std::string_view ciphertext) const
    {
        return app::crypto::AesWrap{}.decrypt(ciphertext, encryption_material());
    }

    [[nodiscard]] std::string merchant_fingerprint(
        std::string_view merchant_id,
        std::string_view last4,
        std::string_view network
    ) const
    {
        return app::crypto::KeyedHasher{"payment_methods.fingerprint"}.hash(
            std::string{merchant_id} + "|" + std::string{last4} + "|" + std::string{network}
        );
    }

    [[nodiscard]] std::string global_id(std::string_view prefix, std::string_view subject) const
    {
        const auto digest = app::crypto::KeyedHasher{"payment_methods.cell"}.hash(
            std::string{subject} + "_" + m_context.ids().uuid_v4()
        );
        return std::string{prefix} + digest.substr(0, 16);
    }

    [[nodiscard]] app::connector::ConnectorResponse call_connector(
        std::string_view merchant_id,
        std::string_view operation,
        std::string_view resource_id,
        std::string_view payload
    ) const
    {
        auto request = app::connector::ConnectorRequestBuilder{}
                           .operation(std::string{operation})
                           .merchant_id(std::string{merchant_id})
                           .resource_id(std::string{resource_id})
                           .raw_payload(std::string{payload})();
        const auto connector = m_context.connector_router().select_for(request);
        if (!connector) {
            // Local in-process substitute for the external vault / PSP / network-token service.
            return local_connector_response(operation, merchant_id, resource_id);
        }
        app::connector::ConnectorResponse response{};
        if (operation == "tokenize" || operation == "vault") {
            response = connector->tokenize(request);
        } else if (operation == "delete_token") {
            response = connector->delete_token(request);
        } else if (operation == "sync" || operation == "network_token_status") {
            response = connector->sync(request);
        } else if (operation == "check_feature_matrix" || operation == "eligibility") {
            response = connector->check_feature_matrix(request);
        } else if (operation == "create_link") {
            response = connector->create_link(request);
        } else if (operation == "exchange_token") {
            response = connector->exchange_token(request);
        } else if (operation == "verify" || operation == "zero_auth") {
            response = connector->verify(request);
        } else {
            response = connector->authorize(request);
        }
        if (response.get_reference_id().empty() &&
            (operation == "tokenize" || operation == "vault" || operation == "zero_auth")) {
            response.set_reference_id(local_vault_token(merchant_id, resource_id));
        }
        if (response.get_status().empty()) {
            response.set_status("success");
        }
        return response;
    }

    /** @brief Deterministic local vault token (in-process locker substitute). */
    [[nodiscard]] std::string
    local_vault_token(std::string_view merchant_id, std::string_view resource_id) const
    {
        const std::string digest = app::crypto::KeyedHasher{"payment_methods.vault"}.hash(
            std::string{merchant_id} + "|" + std::string{resource_id}
        );
        return "pm_" + digest.substr(0, 24);
    }

    /** @brief Upstream-shaped response when no external connector is configured. */
    [[nodiscard]] app::connector::ConnectorResponse local_connector_response(
        std::string_view operation,
        std::string_view merchant_id,
        std::string_view resource_id
    ) const
    {
        app::connector::ConnectorResponse response{};
        response.set_status("success");
        response.set_attempt(1);
        if (operation == "tokenize" || operation == "vault" || operation == "zero_auth") {
            const std::string token = local_vault_token(merchant_id, resource_id);
            response.set_reference_id(token);
            response.set_raw_json(
                "{\"token\":" + serde::Ser::encode_json(token) + ",\"tokenize_support\":true}"
            );
        } else if (operation == "eligibility" || operation == "check_feature_matrix") {
            response.set_raw_json("{\"tokenize_support\":true}");
        } else if (operation == "create_link") {
            response.set_raw_json(
                "/payment_methods/collect/" + std::string{merchant_id} + "/" +
                std::string{resource_id}
            );
        } else if (operation == "exchange_token") {
            response.set_reference_id(issued_token(resource_id));
            response.set_raw_json(
                "{\"access_token\":" + serde::Ser::encode_json(issued_token(resource_id)) + "}"
            );
        }
        return response;
    }

    [[nodiscard]] app::model::payment_methods::PaymentMethodResponse
    response_from_record(const app::store::payment_methods::PaymentMethodRecord& record) const
    {
        using app::model::payment_methods::CardDetailFromLocker;
        using app::model::payment_methods::CustomerIdValue;
        using app::model::payment_methods::MerchantIdValue;
        using app::model::payment_methods::PaymentMethodResponse;
        using app::model::payment_methods::PaymentMethodValue;
        using app::model::payment_methods::PrimitiveDateTimeValue;
        PaymentMethodResponse response{};
        MerchantIdValue merchant{};
        merchant.set_value(record.get_merchant_id());
        response.set_merchant_id(std::move(merchant));
        if (!record.get_customer_id().empty()) {
            CustomerIdValue customer{};
            customer.set_value(record.get_customer_id());
            response.set_customer_id(std::move(customer));
        }
        response.set_payment_method_id(record.get_payment_method_id());
        if (!record.get_type().empty()) {
            PaymentMethodValue method{};
            method.set_value(record.get_type());
            response.set_payment_method(std::move(method));
        }
        CardDetailFromLocker card{};
        if (!record.get_network().empty()) {
            card.set_scheme(record.get_network());
        }
        if (!record.get_last4().empty()) {
            card.set_last4_digits(record.get_last4());
        }
        if (!record.get_vault_id().empty()) {
            card.set_card_token(record.get_vault_id());
        }
        response.set_card(std::move(card));
        PrimitiveDateTimeValue created{};
        created.set_value(record.get_created_at());
        response.set_created(std::move(created));
        const auto cached = m_context.cache().get(
            "payment_methods.client_secret:" + record.get_payment_method_id()
        );
        if (cached.hit) {
            response.set_client_secret(core::cache::value_as<std::string>(cached.value));
        }
        return response;
    }

    void schedule_status_task(
        std::string_view payment_method_id,
        std::string_view merchant_id
    ) const
    {
        const auto payload =
            std::string{"{\"payment_method_id\":"} +
            serde::Ser::encode_json(std::string{payment_method_id}) +
            ",\"merchant_id\":" + serde::Ser::encode_json(std::string{merchant_id}) + "}";
        m_context.process_trackers().schedule("payment_method_status_update", payload, now_value());
    }

    void publish_pm_event(std::string_view topic, std::string_view payment_method_id) const
    {
        const auto payload = std::string{"{\"payment_method_id\":"} +
                             serde::Ser::encode_json(std::string{payment_method_id}) + "}";
        m_context.events().publish(std::string{topic}, payload);
    }

    [[nodiscard]] bool customer_exists(std::string_view customer_id) const
    {
        if (customer_id.empty()) {
            return false;
        }
        for (const auto& record: m_context.repository().list_customers()) {
            if (record.get_customer_id() == customer_id) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] std::string issued_token(std::string_view subject) const
    {
        const app::crypto::SecretString secret{m_context.ids().uuid_v4()};
        return app::crypto::JwtSigner{std::string{secret.value()}}.sign(subject);
    }

    [[nodiscard]] std::string now_value() const
    {
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
                                 std::chrono::system_clock::now().time_since_epoch()
        )
                                 .count();
        return std::to_string(seconds);
    }

private:
    app::context::AppContext& m_context;
};

} // namespace app::handlers::payment_methods
