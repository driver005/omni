export module app_handlers_admin:runtime_merchant_account_create;

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

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::admin {

class MerchantAccountCreateRuntime
{
public:
    explicit MerchantAccountCreateRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<MerchantAccountCreateResult, core::error::AppError>
    create(const MerchantAccountCreateInput& input) const
    {
        std::string merchant_id{input.request.get_merchant_id().get_value()};
        if (merchant_id.empty()) {
            merchant_id = "merchant_" + m_support.uuid_simple();
        }
        if (m_context.repository().find_merchant_account_by_string(merchant_id).has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::ALREADY_EXISTS,
                "The merchant account with the specified details already exists in our records"
            )};
        }
        if (const auto duplicate =
                m_support.acquire_operation_lock("merchant_account_create", merchant_id)) {
            return std::unexpected{*duplicate};
        }
        std::string organization_id{input.organization_id};
        if (organization_id.empty() && input.request.get_organization_id().has_value()) {
            organization_id = input.request.get_organization_id()->get_value();
        }

        const std::string simple_uuid{m_support.uuid_simple()};
        const std::string env_prefix{
            m_context.config_store().get_value("env", "prefix").value_or("test")
        };
        const app::crypto::SecretString publishable_key{
            std::format("pk_{}_{}", env_prefix, simple_uuid)
        };
        const std::string locker_id{simple_uuid};
        const std::string response_hash{
            app::crypto::KeyedHasher{"admin.publishable_key"}.hash(publishable_key.value())
        };
        const std::string aes_key{m_context.ids().key_id()};
        m_context.key_store()
            .put("merchant_aes", merchant_id, "AES-256", app::crypto::SecretString{aes_key});
        m_context.key_store().put(
            "merchant_locker",
            merchant_id,
            "AES-256-wrap",
            app::crypto::SecretString{app::crypto::AesWrap{}.encrypt(locker_id, aes_key)}
        );

        app::store::merchant_accounts::MerchantAccountRecord record;
        record.set_merchant_id(merchant_id);
        record.set_merchant_name(input.request.get_merchant_name().value_or(""));
        record.set_organization_id(organization_id);
        if (input.request.get_parent_merchant_id().has_value()) {
            record.set_parent_merchant_id(input.request.get_parent_merchant_id()->get_value());
        }
        if (input.request.get_return_url().has_value()) {
            record.set_return_url(input.request.get_return_url()->get_value());
        }
        record.set_publishable_key(publishable_key.value());
        record.set_locker_id(locker_id);
        record.set_payment_response_hash_key(response_hash);
        record.set_sub_merchants_enabled(input.request.get_sub_merchants_enabled().value_or(false));
        record.set_enable_payment_response_hash(
            input.request.get_enable_payment_response_hash().value_or(false)
        );
        record.set_redirect_to_merchant_with_http_post(
            input.request.get_redirect_to_merchant_with_http_post().value_or(false)
        );
        if (input.request.get_merchant_account_type().has_value()) {
            record.set_merchant_account_type(
                input.request.get_merchant_account_type()->get_value()
            );
        }
        const auto saved = m_context.repository().save_merchant_account(std::move(record));
        m_support.release_operation_lock("merchant_account_create", merchant_id);

        app::dto::admin::MerchantAccountResponse response;
        response.set_merchant_id(m_support.merchant_id_value(saved.get_merchant_id()));
        response.set_merchant_name(m_support.encryptable_name(saved.get_merchant_name()));
        response.set_organization_id(m_support.organization_id_value(saved.get_organization_id()));
        if (!saved.get_return_url().empty()) {
            response.set_return_url(saved.get_return_url());
        }
        if (!saved.get_publishable_key().empty()) {
            response.set_publishable_key(saved.get_publishable_key());
        }
        if (!saved.get_locker_id().empty()) {
            response.set_locker_id(saved.get_locker_id());
        }
        response.set_sub_merchants_enabled(saved.get_sub_merchants_enabled());
        response.set_enable_payment_response_hash(saved.get_enable_payment_response_hash());
        response.set_redirect_to_merchant_with_http_post(
            saved.get_redirect_to_merchant_with_http_post()
        );
        if (!saved.get_payment_response_hash_key().empty()) {
            response.set_payment_response_hash_key(saved.get_payment_response_hash_key());
        }

        m_context.metrics().record_flow("merchant_account_create");
        m_context.metrics().counter("admin.merchant_account_create").increment();
        m_context.events().publish("merchant_account.created", serde::Ser::encode_json(response));
        return MerchantAccountCreateResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
