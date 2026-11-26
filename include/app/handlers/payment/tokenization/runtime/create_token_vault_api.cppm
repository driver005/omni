export module app_payment:tokenization_runtime_create_token_vault_api;

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

import :tokenization_input;
import :tokenization_result;
import :tokenization_runtime_common;

export namespace app::handlers::tokenization {

class CreateTokenVaultRuntime
{
public:
    explicit CreateTokenVaultRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** Port of core::tokenization::create_vault_token_core (core/tokenization.rs:34-96). */
    [[nodiscard]] std::expected<CreateTokenVaultResult, core::error::AppError>
    create_token_vault(const CreateTokenVaultInput& input) const
    {
        // core:40 — VaultId::generate(generate_uuid_v7().to_string()).
        const std::string vault_id = m_support.uuid_v7();

        // core:44-49 — AddVaultRequest { entity_id, vault_id, data, ttl }.
        AddVaultRequestWire add_request{};
        add_request.entity_id = input.request.get_customer_id();
        add_request.vault_id = vault_id;
        add_request.data = input.request.get_token_request();
        add_request.ttl = m_support.vault_ttl_seconds();

        // core:54-61 — WriteMode::Insert + pm_vault::call_to_vault::<AddVault>.
        auto stored = m_support.call_to_vault_add(add_request);
        if (!stored) {
            return std::unexpected{stored.error()};
        }

        // core:70-79 — Tokenization { id, merchant_id, customer_id, locker_id,
        // created_at, updated_at, flag: Enabled, version: V2 }.
        ::app::store::tokenizations::TokenizationRecord record{};
        record.set_token_id(m_support.global_token_id(m_support.cell_id(input.merchant_id)));
        record.set_merchant_id(std::string{input.merchant_id});
        record.set_customer_id(input.request.get_customer_id());
        record.set_entity_id(input.request.get_customer_id());
        record.set_vault_id(stored->vault_id);       // core:74 locker_id = stored_resp.vault_id
        record.set_status(m_support.enabled_flag()); // core:77 TokenizationFlag::Enabled
        record.set_created_at(m_support.primitive_date_time_now()); // core:75-76

        // core:82-86 — insert_tokenization(tokenization_new, provider.get_key_store()).
        auto inserted = m_context.repository().create_tokenization(std::move(record));
        if (!inserted) {
            return std::unexpected{inserted.error()};
        }

        // core:89-94 — GenericTokenizationResponse { id, created_at, flag }.
        // Use the real typed DTO; its plain-string fields emit the exact upstream wire.
        ::app::dto::tokenization::GenericTokenizationResponse response{};
        response.set_id(inserted->get_token_id());
        response.set_created_at(inserted->get_created_at());
        response.set_flag(inserted->get_status());
        return CreateTokenVaultResult{response};
    }

private:
    ::app::context::AppContext& m_context;
    TokenizationRuntimeSupport m_support;
};

} // namespace app::handlers::tokenization
