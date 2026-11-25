export module app_handlers_user:runtime_get_user_details_internal;

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

class GetUserDetailsInternalRuntime
{
public:
    explicit GetUserDetailsInternalRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<GetUserDetailsInternalResult, core::error::AppError>
    get_user_details_internal(const GetUserDetailsInternalInput& input) const
    {
        std::string set_cookie{};


        const auto segments = app::http::PathSegments{input.request_context.path};
        if (segments.count() == 0) {
            return std::unexpected{m_support.wire_error(
                "api:HE_00",
                "Something went wrong",
                core::error::AppErrorKind::INTERNAL_ERROR
            )};
        }
        const auto user_id = std::string{segments.at(segments.count() - 1).value_or("")};
        if (user_id.empty()) {
            return std::unexpected{m_support.wire_error(
                "api:HE_00",
                "Something went wrong",
                core::error::AppErrorKind::INTERNAL_ERROR
            )};
        }

        const auto stored = m_context.repository().find_user_by_string(user_id);
        m_context.metrics().record_flow("get_user_details_internal");
        m_context.metrics().counter("user.get_user_details_internal").increment();

        using Response = app::model::user::GetUserInternalDetailsResponse;
        Response response{};
        response.set_user_id(user_id);
        response.set_is_active(stored && stored->get_status() == "active");
        auto document = ResponseDocument::from(response);
        return GetUserDetailsInternalResult{document, set_cookie};
    }

private:
    app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
