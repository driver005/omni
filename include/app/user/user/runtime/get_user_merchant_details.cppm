export module app_user:user_runtime_get_user_merchant_details;

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
import app_connector;
import app_events;
import app_workflow;
import app_keystore;
import app_locking;
import app_services;

import :user_input;
import :user_result;
import :user_runtime_common;

export namespace app::handlers::user {

class GetUserMerchantDetailsRuntime
{
public:
    explicit GetUserMerchantDetailsRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    get_user_merchant_details(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        std::string set_cookie{};


        const auto merchant_id = ctx.merchant_id;
        const auto key = std::string{"user.get_user_merchant_details:"} + merchant_id;
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{ctx_string(ctx, "body")})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.logger().record_flow("get_user_merchant_details");
        m_context.logger().counter("user.get_user_merchant_details").increment();

        using Response = ::app::model::user::UserMerchantDetailsResponse;
        Response response{};
        if (!merchant_id.empty()) {
            const auto account =
                m_context.repository().find_merchant_account_by_string(merchant_id);
            if (!account) {
                return std::unexpected{m_support.wire_error(
                    "UR_18",
                    "Invalid Merchant ID",
                    core::error::AppErrorKind::INVALID_REQUEST
                )};
            }
            ::app::model::user::MerchantAccountTypeValue account_type{};
            account_type.set_value(account->get_merchant_account_type());
            response.set_merchant_account_type(account_type);
        }
        auto document = ResponseDocument::from(response);
        return std::pair{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
