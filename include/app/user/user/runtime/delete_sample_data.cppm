export module app_user:user_runtime_delete_sample_data;

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

class DeleteSampleDataRuntime
{
public:
    explicit DeleteSampleDataRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<std::pair<serde::Value, std::string>, core::error::AppError>
    delete_sample_data(const DeleteSampleDataInput& input) const
    {
        std::string set_cookie{};


        // Local in-process substitute for core/user/sample_data.rs deletion:
        // tracked job + event; upstream returns StatusOk (empty 200).
        const auto tracker_id = m_context.process_trackers().schedule(
            "sample_data_delete",
            input.request_context.body,
            m_support.now_string()
        );
        m_context.process_trackers().mark_terminal(tracker_id, "queued");
        m_context.events().publish(
            "sample_data.delete",
            std::format(R"({{"tracker_id":{}}})", tracker_id)
        );
        m_context.repository().set_config(
            "user.sample_data.last_delete." + input.request_context.merchant_id,
            std::to_string(input.request.get_record().value_or(0U))
        );

        m_context.metrics().record_flow("delete_sample_data");
        m_context.metrics().counter("user.delete_sample_data").increment();

        return std::pair{ResponseDocument::empty(), set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
