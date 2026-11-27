export module app_payment:poll_runtime;
import core_behavior;

import std;
import core_error;
import app_context;

import :poll_input;
import :poll_result;
import :poll_runtime_common;
import :poll_runtime_retrieve_poll_status;

export namespace app::handlers::poll {

class PollRuntime
{
public:
    explicit PollRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_retrieve_poll_status{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::poll::PollResponseWire>
    retrieve_poll_status(const core::behavior::CallContext& ctx) const
    {
        return m_retrieve_poll_status.retrieve_poll_status(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    RetrievePollStatusRuntime m_retrieve_poll_status;
};

} // namespace app::handlers::poll
