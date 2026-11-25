export module app_handlers_poll:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_retrieve_poll_status;

export namespace app::handlers::poll {

class PollRuntime
{
public:
    explicit PollRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_retrieve_poll_status{context}
    {
    }

    [[nodiscard]] std::expected<RetrievePollStatusResult, core::error::AppError>
    retrieve_poll_status(const RetrievePollStatusInput& input) const
    {
        return m_retrieve_poll_status.retrieve_poll_status(input);
    }

private:
    app::context::AppContext& m_context;
    RetrievePollStatusRuntime m_retrieve_poll_status;
};

} // namespace app::handlers::poll
