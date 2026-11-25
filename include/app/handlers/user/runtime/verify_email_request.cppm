export module app_handlers_user:runtime_verify_email_request;

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

class VerifyEmailRequestRuntime
{
public:
    explicit VerifyEmailRequestRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<VerifyEmailRequestResult, core::error::AppError>
    verify_email_request(const VerifyEmailRequestInput& input) const
    {
        std::string set_cookie{};


        const auto email = input.request.get_email().get_value();
        const auto verify_payload = std::string{R"({"email":")"} + email + R"("})";
        const auto verify_token = m_support.user_signer().sign(verify_payload);
        m_context.cache().put(
            "user.verify_token:" + email,
            core::cache::to_cache_value(verify_token)
        );

        const auto key = std::string{"user.verify_email_request:"} +
                         std::to_string(input.request_context.body.size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{input.request_context.body})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.metrics().record_flow("verify_email_request");
        m_context.metrics().counter("user.verify_email_request").increment();

        return VerifyEmailRequestResult{ResponseDocument::empty(), set_cookie};
    }

private:
    app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
