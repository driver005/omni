export module app_user:admin_runtime_merchant_account_kv_status;

import std;
import core_behavior;
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

class MerchantAccountKvStatusRuntime
{
public:
    explicit MerchantAccountKvStatusRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::ToggleKVResponse>
    status(const ::core::behavior::CallContext& ctx) const
    {
        const auto found =
            m_context.repository().find_merchant_account_by_string(ctx_string(ctx,"merchant_id"));
        if (!found.has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "Merchant account does not exist in our records"
            )};
        }
        const bool kv_enabled = m_context.config_store()
                                    .get_value("merchant_kv_scheme", found->get_merchant_id())
                                    .value_or("database") == "kv";
        ::app::dto::admin::ToggleKVResponse response;
        response.set_merchant_id(m_support.merchant_id_value(found->get_merchant_id()));
        response.set_kv_enabled(kv_enabled);

        m_context.metrics().record_flow("merchant_account_kv_status");
        m_context.metrics().counter("admin.merchant_account_kv_status").increment();
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
