export module app_user:user_runtime_embedded_token_info;

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

class EmbeddedTokenInfoRuntime
{
public:
    explicit EmbeddedTokenInfoRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<std::pair<serde::Value, std::string>, core::error::AppError>
    embedded_token_info(const EmbeddedTokenInfoInput& input) const
    {
        std::string set_cookie{};


        const auto embedded_info_token = input.request_context.token;
        if (!embedded_info_token.value().empty() &&
            !m_support.user_signer().verify(embedded_info_token.value())) {
            return std::unexpected{m_support.wire_error(
                "IR_17",
                "Access forbidden, invalid JWT token was used",
                core::error::AppErrorKind::UNAUTHORIZED
            )};
        }

        const auto auth_ctx = input.request_context.auth;
        m_context.metrics().record_flow("embedded_token_info");
        m_context.metrics().counter("user.embedded_token_info").increment();

        using Response = ::app::model::user::EmbeddedTokenInfoResponse;
        Response response{};
        ::app::model::user::OrganizationIdValue oid{};
        oid.set_value(auth_ctx.org_id());
        ::app::model::user::MerchantIdValue mid{};
        mid.set_value(auth_ctx.merchant_id());
        ::app::model::user::ProfileIdValue pid{};
        pid.set_value(auth_ctx.profile_id());
        ::app::model::user::ApiVersionValue version{};
        version.set_value("V2");
        response.set_org_id(oid);
        response.set_merchant_id(mid);
        response.set_profile_id(pid);
        response.set_merchant_account_version(version);
        auto document = ResponseDocument::from(response);
        return std::pair{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
