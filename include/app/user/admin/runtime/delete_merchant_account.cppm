export module app_user:admin_runtime_delete_merchant_account;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_behavior;
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

class DeleteMerchantAccountRuntime
{
public:
    explicit DeleteMerchantAccountRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::MerchantAccountDeleteResponse>
        remove(const ::core::behavior::CallContext& ctx) const
    {
        const auto found =
            m_context.repository().find_merchant_account_by_string(ctx_string(ctx,"merchant_id"));
        if (!found.has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "Merchant account does not exist in our records"
            )};
        }
        // core/admin.rs:1453-1457: platform merchant accounts are not deletable.
        if (found->get_merchant_account_type() == "platform") {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Platform merchant account cannot be deleted"
            )};
        }
        // core/admin.rs:1459-1471: `deleted` is account-delete AND key-store-delete.
        const bool account_deleted =
            m_context.repository().remove_merchant_account(found->get_id());
        bool key_store_deleted = false;
        if (account_deleted) {
            key_store_deleted =
                m_context.key_store().find_record("merchant_aes", ctx_string(ctx,"merchant_id")).has_value();
        }
        ::app::dto::admin::MerchantAccountDeleteResponse response;
        response.set_merchant_id(m_support.merchant_id_value(found->get_merchant_id()));
        response.set_deleted(account_deleted && key_store_deleted);

        m_context.metrics().record_flow("delete_merchant_account");
        m_context.metrics().counter("admin.delete_merchant_account").increment();
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
