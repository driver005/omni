export module app_user:profiles_handlers_v1_toggles;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;

import :profiles_input;
import :profiles_result;
import :profiles_runtime;

export namespace app::handlers::profiles {

/**
 * v1 toggle adapters: connector-agnostic MIT and extended card info.
 */
class ProfilesHandlersV1Toggles
{
public:
    ProfilesHandlersV1Toggles(
        ::app::context::AppContext& context,
        ProfilesRuntime& runtime,
        ProfilesInputSupport& input_support,
        ProfilesRequestSupport& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_support{request_support}
    {
    }

    void handle_toggle_connector_agnostic_mit(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "toggle_connector_agnostic_mit", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const auto body = req.body_as_string();
        auto parsed =
            core::behavior::Serializer<::app::dto::admin::ConnectorAgnosticMitChoice>::pass(
                req,
                sink
            );
        if (!parsed) {
            return;
        }
        const std::string profile_id = m_input_support.profile_id_from_request(req, 1);
        if (profile_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing profile id"
            );
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign(
            "merchant_id",
            std::string{m_input_support.merchant_from_request(req)}
        );
        ctx.claims.insert_or_assign("profile_id", std::string{profile_id});
        m_runtime.toggle_connector_agnostic_mit(*parsed, ctx).handle(sink);
    }

    void handle_toggle_extended_card_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "toggle_extended_card_info", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto parsed =
            core::behavior::Serializer<::app::dto::admin::ExtendedCardInfoChoice>::pass(req, sink);
        if (!parsed) {
            return;
        }
        const std::string profile_id = m_input_support.profile_id_from_request(req, 1);
        if (profile_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing profile id"
            );
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign(
            "merchant_id",
            std::string{m_input_support.merchant_from_request(req)}
        );
        ctx.claims.insert_or_assign("profile_id", std::string{profile_id});
        m_runtime.toggle_extended_card_info(*parsed, ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    ProfilesRuntime& m_runtime;
    ProfilesInputSupport& m_input_support;
    ProfilesRequestSupport& m_support;
};

} // namespace app::handlers::profiles
