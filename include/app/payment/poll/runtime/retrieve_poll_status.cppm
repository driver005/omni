export module app_payment:poll_runtime_retrieve_poll_status;
import core_behavior;

import std;
import core_error;
import app_context;
import core_cache;
import app_metrics;

import :poll_input;
import :poll_result;
import :poll_runtime_common;

export namespace app::handlers::poll {

class RetrievePollStatusRuntime
{
public:
    explicit RetrievePollStatusRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Full port of core::poll::retrieve_poll_status (core/poll.rs:11-48).
    //  - cache closed -> InternalServerError ("Failed to get redis connection").
    //  - key miss -> PollNotFound.
    //  - parse_enum("PollStatus") failure -> InternalServerError.
    [[nodiscard]] core::behavior::Outcome<::app::dto::poll::PollResponseWire>
    retrieve_poll_status(const core::behavior::CallContext& ctx) const
    {
        const auto input = RetrievePollStatusData{ctx};
        // routes/poll.rs:27,33,51-52 — Flow::RetrievePollStatus.
        m_context.logger().record_flow("RetrievePollStatus");

        if (!m_context.cache().is_open()) {
            return std::unexpected{m_support.internal_server_error()};
        }

        const auto key = m_support.get_poll_id(input.processor_merchant_id, input.request_poll_id);
        const auto cached = m_context.cache().get(key);
        if (!cached.hit) {
            return std::unexpected{m_support.poll_not_found_error()};
        }

        const auto raw = cached.value_or<std::string>(std::string{});
        const auto status = m_support.parse_poll_status(raw);
        if (!status) {
            return std::unexpected{m_support.internal_server_error()};
        }

        m_support.record_retrieve_metrics(input.request_poll_id);
        return ::app::dto::poll::PollResponseWire{
            m_support.encode_poll_response(input.request_poll_id, *status)
        };
    }

private:
    ::app::context::AppContext& m_context;
    PollRuntimeSupport m_support;
};

} // namespace app::handlers::poll
