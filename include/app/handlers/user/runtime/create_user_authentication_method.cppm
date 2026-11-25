export module app_handlers_user:runtime_create_user_authentication_method;

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

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::user {

class CreateUserAuthenticationMethodRuntime
{
public:
    explicit CreateUserAuthenticationMethodRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<CreateUserAuthenticationMethodResult, core::error::AppError>
    create_user_authentication_method(const CreateUserAuthenticationMethodInput& input) const
    {
        std::string set_cookie{};


        const auto auth_id = m_context.ids().key_id();
        app::store::authentication_records::AuthenticationRecord record{};
        record.set_auth_id(auth_id);
        record.set_user_id(input.request.get_owner_id());
        record.set_merchant_id(input.request_context.merchant_id);
        record.set_method("password");
        record.set_status("active");
        record.set_created_at(m_support.now_string());
        (void)m_context.repository().create_authentication_record(record);

        const auto key = std::string{"user.create_user_authentication_method:"} +
                         std::to_string(input.request_context.body.size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{input.request_context.body})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.metrics().record_flow("create_user_authentication_method");
        m_context.metrics().counter("user.create_user_authentication_method").increment();

        using Response = app::model::user::CreateUserAuthenticationMethodResponse;
        Response response{};
        response.set_id(auth_id);
        response.set_auth_id(auth_id);
        response.set_owner_id(input.request.get_owner_id());
        response.set_owner_type(input.request.get_owner_type());
        app::model::user::UserAuthTypeValue auth_type{};
        auth_type.set_value(m_context.repository()
                                .get_config("user.auth_type." + input.request.get_owner_id())
                                .value_or(std::string{"password"}));
        response.set_auth_type(auth_type);
        response.set_email_domain(input.request.get_email_domain());
        response.set_allow_signup(input.request.get_allow_signup());
        auto document = ResponseDocument::from(response);
        return CreateUserAuthenticationMethodResult{document, set_cookie};
    }

private:
    app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
