export module app_handlers_admin:runtime_update_merchant_account;

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

class UpdateMerchantAccountRuntime
{
public:
    explicit UpdateMerchantAccountRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<UpdateMerchantAccountResult, core::error::AppError>
    update(const UpdateMerchantAccountInput& input) const
    {
        auto found = m_context.repository().find_merchant_account_by_string(input.merchant_id);
        if (!found.has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "Merchant account does not exist in our records"
            )};
        }
        auto record = std::move(*found);
        if (input.request.get_merchant_name().has_value()) {
            record.set_merchant_name(*input.request.get_merchant_name());
        }
        if (input.request.get_return_url().has_value()) {
            record.set_return_url(input.request.get_return_url()->get_value());
        }
        if (input.request.get_publishable_key().has_value()) {
            record.set_publishable_key(*input.request.get_publishable_key());
        }
        if (input.request.get_locker_id().has_value()) {
            record.set_locker_id(*input.request.get_locker_id());
        }
        if (input.request.get_sub_merchants_enabled().has_value()) {
            record.set_sub_merchants_enabled(*input.request.get_sub_merchants_enabled());
        }
        if (input.request.get_enable_payment_response_hash().has_value()) {
            record.set_enable_payment_response_hash(
                *input.request.get_enable_payment_response_hash()
            );
        }
        if (input.request.get_redirect_to_merchant_with_http_post().has_value()) {
            record.set_redirect_to_merchant_with_http_post(
                *input.request.get_redirect_to_merchant_with_http_post()
            );
        }
        if (input.request.get_payment_response_hash_key().has_value()) {
            record.set_payment_response_hash_key(*input.request.get_payment_response_hash_key());
        }
        if (const auto duplicate =
                m_support.acquire_operation_lock("update_merchant_account", input.merchant_id)) {
            return std::unexpected{*duplicate};
        }
        m_context.repository().update_merchant_account(record);
        m_support.release_operation_lock("update_merchant_account", input.merchant_id);

        app::dto::admin::MerchantAccountResponse response;
        response.set_merchant_id(m_support.merchant_id_value(record.get_merchant_id()));
        response.set_merchant_name(m_support.encryptable_name(record.get_merchant_name()));
        response.set_organization_id(m_support.organization_id_value(record.get_organization_id()));
        if (!record.get_return_url().empty()) {
            response.set_return_url(record.get_return_url());
        }
        if (!record.get_publishable_key().empty()) {
            response.set_publishable_key(record.get_publishable_key());
        }
        if (!record.get_locker_id().empty()) {
            response.set_locker_id(record.get_locker_id());
        }
        response.set_sub_merchants_enabled(record.get_sub_merchants_enabled());

        m_context.metrics().record_flow("update_merchant_account");
        m_context.metrics().counter("admin.update_merchant_account").increment();
        m_context.events().publish("merchant_account.updated", serde::Ser::encode_json(response));
        return UpdateMerchantAccountResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
