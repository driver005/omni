export module app_user:admin_runtime_merchant_account_toggle_kv;

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

class MerchantAccountToggleKvRuntime
{
public:
    explicit MerchantAccountToggleKvRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<::app::dto::admin::ToggleKVResponse, core::error::AppError>
    toggle(const MerchantAccountToggleKvInput& input) const
    {
        const auto found =
            m_context.repository().find_merchant_account_by_string(input.merchant_id);
        if (!found.has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "Merchant account does not exist in our records"
            )};
        }
        if (const auto blocked = m_support.kv_soft_kill_guard()) {
            return std::unexpected{*blocked};
        }
        if (const auto duplicate =
                m_support.acquire_operation_lock("merchant_account_toggle_kv", input.merchant_id)) {
            return std::unexpected{*duplicate};
        }
        const std::string target_scheme{input.kv_enabled ? "kv" : "database"};
        const bool was_enabled = m_context.config_store()
                                     .get_value("merchant_kv_scheme", found->get_merchant_id())
                                     .value_or("database") == "kv";
        if (was_enabled != input.kv_enabled) {
            m_context.config_store()
                .put("merchant_kv_scheme", found->get_merchant_id(), target_scheme);
        }
        m_support.release_operation_lock("merchant_account_toggle_kv", input.merchant_id);

        ::app::dto::admin::ToggleKVResponse response;
        response.set_merchant_id(m_support.merchant_id_value(found->get_merchant_id()));
        response.set_kv_enabled(input.kv_enabled);

        m_context.metrics().record_flow("merchant_account_toggle_kv");
        m_context.metrics().counter("admin.merchant_account_toggle_kv").increment();
        m_context.events().publish(
            "merchant_account.kv_toggled",
            serde::Ser::encode_json(response)
        );
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
