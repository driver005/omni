export module app_handlers_user:runtime_get_sso_auth_url;

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

class GetSsoAuthUrlRuntime
{
public:
    explicit GetSsoAuthUrlRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<GetSsoAuthUrlResult, core::error::AppError>
    get_sso_auth_url(const GetSsoAuthUrlInput& input) const
    {
        std::string set_cookie{};


        // Typed GetSsoAuthUrlRequest (routes/user.rs:852); upstream resolves the
        // OIDC auth method by id and returns a JsonForRedirection response.
        // Upstream uses web::Query<GetSsoAuthUrlRequest>: the `id` comes from the
        // query string, not the request body (routes/user.rs:852-865).
        app::model::user::GetSsoAuthUrlRequest request{};
        const auto id_param = input.request_context.query.get("id");
        if (!id_param) {
            return std::unexpected{m_support.wire_error(
                "UR_07",
                "Invalid Email",
                core::error::AppErrorKind::INVALID_REQUEST
            )};
        }
        request.set_id(*id_param);
        if (!m_context.repository().find_authentication_record_by_string(request.get_id())) {
            return std::unexpected{m_support.wire_error(
                "UR_44",
                "Invalid user auth method operation",
                core::error::AppErrorKind::NOT_FOUND
            )};
        }

        // Local OIDC substitute: configured IdP base URL + persisted state,
        // emitting the exact upstream RedirectionResponse wire shape as a real
        // typed DTO (app/model/user.cppm:1921-1940, serde :2007-2020).
        const auto sso_state = m_context.ids().uuid_v4();
        const auto base_url = m_context.repository()
                                  .get_config("sso.base_url")
                                  .value_or(std::string{"/sso/authorize"});
        m_context.repository().set_config("user.sso_state." + sso_state, request.get_id());
        const auto authorization_url =
            std::format("{}?state={}&client_id=local", base_url, sso_state);

        m_context.metrics().record_flow("get_sso_auth_url");
        m_context.metrics().counter("user.get_sso_auth_url").increment();

        using Response = app::model::user::RedirectionResponse;
        Response response{};
        response.set_return_url("");
        response.set_return_url_with_query_params(authorization_url);
        response.set_http_method("");
        auto document = ResponseDocument::from(response);
        return GetSsoAuthUrlResult{document, set_cookie};
    }

private:
    app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
