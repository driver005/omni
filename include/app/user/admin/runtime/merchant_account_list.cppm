export module app_user:admin_runtime_merchant_account_list;

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

class MerchantAccountListRuntime
{
public:
    explicit MerchantAccountListRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list(const ::core::behavior::CallContext& ctx) const
    {
        std::vector<::app::dto::admin::MerchantAccountResponse> response;
        const auto accounts = m_context.repository().list_merchant_accounts();
        std::size_t emitted{0};
        for (const auto& account: accounts) {
            if (!ctx_string(ctx, "organization_id").empty() &&
                account.get_organization_id() != ctx_string(ctx, "organization_id")) {
                continue;
            }
            if (!ctx_string(ctx, "merchant_filter").empty() &&
                account.get_merchant_id() != ctx_string(ctx, "merchant_filter")) {
                continue;
            }
            if (emitted < ctx_size(ctx, "offset")) {
                ++emitted;
                continue;
            }
            if (ctx_size(ctx, "limit") != 0 && response.size() >= ctx_size(ctx, "limit")) {
                break;
            }
            ::app::dto::admin::MerchantAccountResponse item;
            item.set_merchant_id(m_support.merchant_id_value(account.get_merchant_id()));
            item.set_merchant_name(m_support.encryptable_name(account.get_merchant_name()));
            item.set_organization_id(
                m_support.organization_id_value(account.get_organization_id())
            );
            if (!account.get_publishable_key().empty()) {
                item.set_publishable_key(account.get_publishable_key());
            }
            response.push_back(std::move(item));
        }

        m_context.logger().record_flow("merchant_account_list");
        m_context.logger().counter("admin.merchant_account_list").increment();
        serde::Value::Array array{};
        for (const auto& item: response) {
            array.push_back(
                serde::Serializer<::app::dto::admin::MerchantAccountResponse>::build_value(
                    item,
                    kMerchantAccountResponseWireFields
                )
            );
        }
        return serde::Value{std::move(array)};
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
