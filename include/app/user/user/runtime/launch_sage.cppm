export module app_user:user_runtime_launch_sage;

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

class LaunchSageRuntime
{
public:
    explicit LaunchSageRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<LaunchSageResult, core::error::AppError>
    launch_sage(const LaunchSageInput& input) const
    {
        std::string set_cookie{};


        // Sage requires an external AI service. Locally we record a tracked session
        // and publish a deterministic handoff instead of returning an empty response.
        const auto session_id = m_context.ids().uuid_v4();
        const auto tracker_id = m_context.process_trackers()
                                    .schedule("sage_session", session_id, m_support.now_string());
        m_context.events().publish(
            "sage.session_launched",
            std::format(R"({{"session_id":"{}","tracker_id":{}}})", session_id, tracker_id)
        );

        m_context.metrics().record_flow("launch_sage");
        m_context.metrics().counter("user.launch_sage").increment();

        using Response = ::app::dto::launch_sage::LaunchSageResponse;
        Response response{};
        response.set_handoff_url("/sage/session/" + session_id);
        auto document = ResponseDocument::from(response);
        return LaunchSageResult{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
