export module app_user:admin_runtime_merchant_account_transfer_keys;

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
import app_locking;
import app_events;
import app_keystore;
import app_services;

import :admin_input;
import :admin_result;
import :admin_runtime_common;

export namespace app::handlers::admin {

class MerchantAccountTransferKeysRuntime
{
public:
    explicit MerchantAccountTransferKeysRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<::app::dto::admin::TransferKeyResponse, core::error::AppError>
    transfer(const MerchantAccountTransferKeysInput& input) const
    {
        if (const auto duplicate =
                m_support.acquire_operation_lock("merchant_account_transfer_keys", "all")) {
            return std::unexpected{*duplicate};
        }
        std::size_t total_transferred{0};
        const auto accounts = m_context.repository().list_merchant_accounts();
        for (std::size_t index{input.offset};
             index < accounts.size() && total_transferred < input.limit;
             ++index) {
            auto record = accounts[index];
            const std::string new_key{m_context.ids().key_id()};
            const ::app::crypto::SecretString wrapped{::app::crypto::AesWrap{}.encrypt(
                new_key,
                m_support.connector_key(record.get_merchant_id(), new_key)
            )};
            record.set_payment_response_hash_key(
                ::app::crypto::KeyedHasher{new_key}.hash(wrapped.value())
            );
            if (!m_context.key_store().rotate(
                    "merchant_aes",
                    record.get_merchant_id(),
                    ::app::crypto::SecretString{new_key}
                )) {
                m_context.key_store().put(
                    "merchant_aes",
                    record.get_merchant_id(),
                    "AES-256",
                    ::app::crypto::SecretString{new_key}
                );
            }
            if (m_context.repository().update_merchant_account(record)) {
                ++total_transferred;
            }
        }
        m_support.release_operation_lock("merchant_account_transfer_keys", "all");
        ::app::dto::admin::TransferKeyResponse response;
        response.set_total_transferred(total_transferred);

        m_context.metrics().record_flow("merchant_account_transfer_keys");
        m_context.metrics().counter("admin.merchant_account_transfer_keys").increment();
        m_context.events().publish(
            "merchant_account.keys_transferred",
            serde::Ser::encode_json(response)
        );
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
