export module app_handlers_admin:runtime_merchant_account_toggle_all_kv;

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

class MerchantAccountToggleAllKvRuntime
{
public:
    explicit MerchantAccountToggleAllKvRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<MerchantAccountToggleAllKvResult, core::error::AppError>
    toggle_all(const MerchantAccountToggleAllKvInput& input) const
    {
        if (const auto blocked = m_support.kv_soft_kill_guard()) {
            return std::unexpected{*blocked};
        }
        if (const auto duplicate =
                m_support.acquire_operation_lock("merchant_account_toggle_all_kv", "all")) {
            return std::unexpected{*duplicate};
        }
        const std::string target_scheme{input.kv_enabled ? "kv" : "database"};
        std::size_t total_updated{0};
        for (const auto& record: m_context.repository().list_merchant_accounts()) {
            const auto current =
                m_context.config_store().get_value("merchant_kv_scheme", record.get_merchant_id());
            if (current.has_value() && *current == target_scheme) {
                continue;
            }
            m_context.config_store()
                .put("merchant_kv_scheme", record.get_merchant_id(), target_scheme);
            ++total_updated;
        }
        m_support.release_operation_lock("merchant_account_toggle_all_kv", "all");

        app::dto::admin::ToggleAllKVResponse response;
        response.set_total_updated(total_updated);
        response.set_kv_enabled(input.kv_enabled);

        m_context.metrics().record_flow("merchant_account_toggle_all_kv");
        m_context.metrics().counter("admin.merchant_account_toggle_all_kv").increment();
        m_context.events().publish(
            "merchant_account.kv_toggled_all",
            serde::Ser::encode_json(response)
        );
        return MerchantAccountToggleAllKvResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
