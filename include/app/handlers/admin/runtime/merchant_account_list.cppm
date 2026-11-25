export module app_handlers_admin:runtime_merchant_account_list;

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

class MerchantAccountListRuntime
{
public:
    explicit MerchantAccountListRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<MerchantAccountListResult, core::error::AppError>
    list(const MerchantAccountListInput& input) const
    {
        std::vector<app::dto::admin::MerchantAccountResponse> response;
        const auto accounts = m_context.repository().list_merchant_accounts();
        std::size_t emitted{0};
        for (const auto& account: accounts) {
            if (!input.organization_id.empty() &&
                account.get_organization_id() != input.organization_id) {
                continue;
            }
            if (!input.merchant_filter.empty() &&
                account.get_merchant_id() != input.merchant_filter) {
                continue;
            }
            if (emitted < input.offset) {
                ++emitted;
                continue;
            }
            if (input.limit != 0 && response.size() >= input.limit) {
                break;
            }
            app::dto::admin::MerchantAccountResponse item;
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

        m_context.metrics().record_flow("merchant_account_list");
        m_context.metrics().counter("admin.merchant_account_list").increment();
        return MerchantAccountListResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
