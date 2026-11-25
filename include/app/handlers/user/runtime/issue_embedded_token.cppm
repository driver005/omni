export module app_handlers_user:runtime_issue_embedded_token;

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

class IssueEmbeddedTokenRuntime
{
public:
    explicit IssueEmbeddedTokenRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<IssueEmbeddedTokenResult, core::error::AppError>
    issue_embedded_token(const IssueEmbeddedTokenInput& input) const
    {
        std::string set_cookie{};


        const auto auth_ctx = input.request_context.auth;
        const auto processor = m_context.repository()
                                   .get_config("embedded.processor." + auth_ctx.merchant_id())
                                   .value_or(std::string{"local"});
        const auto embedded_payload = std::format(
            R"({{"mid":"{}","oid":"{}","pid":"{}","processor":"{}"}})",
            auth_ctx.merchant_id(),
            auth_ctx.org_id(),
            auth_ctx.profile_id(),
            processor
        );
        const auto embedded_token = m_support.user_signer().sign(embedded_payload);

        const auto key = std::string{"user.issue_embedded_token:"} +
                         std::to_string(input.request_context.body.size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{input.request_context.body})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.metrics().record_flow("issue_embedded_token");
        m_context.metrics().counter("user.issue_embedded_token").increment();

        using Response = app::model::user::IssueEmbeddedTokenResponse;
        Response response{};
        response.set_token(embedded_token);
        auto document = ResponseDocument::from(response);
        return IssueEmbeddedTokenResult{document, set_cookie};
    }

private:
    app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
