export module app_handlers_user:runtime_update_user_account_details;

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

class UpdateUserAccountDetailsRuntime
{
public:
    explicit UpdateUserAccountDetailsRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<UpdateUserAccountDetailsResult, core::error::AppError>
    update_user_account_details(const UpdateUserAccountDetailsInput& input) const
    {
        std::string set_cookie{};


        const auto owner = input.request_context.token.value();
        const auto email_entry = m_context.cache().get("user.token_email:" + owner);
        const auto email =
            email_entry.hit ? core::cache::value_as<std::string>(email_entry.value, std::string{})
                            : std::string{};
        const auto index_entry = m_context.cache().get("user.email_index:" + email);
        const auto user_id =
            index_entry.hit ? core::cache::value_as<std::string>(index_entry.value, email) : email;
        if (auto user = m_context.repository().find_user_by_string(user_id)) {
            if (input.request.get_name()) {
                user->set_name(*input.request.get_name());
            }
            m_context.repository().update_user(*user);
            m_support.publish_identity_event("user.details_updated", user_id);
        }

        m_context.metrics().record_flow("update_user_account_details");
        m_context.metrics().counter("user.update_user_account_details").increment();

        return UpdateUserAccountDetailsResult{ResponseDocument::empty(), set_cookie};
    }

private:
    app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
