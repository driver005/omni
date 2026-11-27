export module app_user:admin_runtime_retrieve_merchant_account;

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

class RetrieveMerchantAccountRuntime
{
public:
    explicit RetrieveMerchantAccountRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<::app::dto::admin::MerchantAccountResponse, core::error::AppError>
    retrieve(const RetrieveMerchantAccountInput& input) const
    {
        const auto found =
            m_context.repository().find_merchant_account_by_string(input.merchant_id);
        if (!found.has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "Merchant account does not exist in our records"
            )};
        }
        ::app::dto::admin::MerchantAccountResponse response;
        response.set_merchant_id(m_support.merchant_id_value(found->get_merchant_id()));
        response.set_merchant_name(m_support.encryptable_name(found->get_merchant_name()));
        response.set_organization_id(m_support.organization_id_value(found->get_organization_id()));
        if (!found->get_return_url().empty()) {
            response.set_return_url(found->get_return_url());
        }
        if (!found->get_publishable_key().empty()) {
            response.set_publishable_key(found->get_publishable_key());
        }
        if (!found->get_locker_id().empty()) {
            response.set_locker_id(found->get_locker_id());
        }
        response.set_sub_merchants_enabled(found->get_sub_merchants_enabled());

        m_context.metrics().record_flow("retrieve_merchant_account");
        m_context.metrics().counter("admin.retrieve_merchant_account").increment();
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
