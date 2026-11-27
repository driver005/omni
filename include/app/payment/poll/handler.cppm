export module app_payment:poll_handler;

import std;
import core_router;
import app_context;

import :poll_input;
import :poll_result;
import :poll_runtime;
import :poll_handlers_v1;

export namespace app::handlers::poll {

/**
 * Facade ("handler of handlers") for the poll domain.
 *
 * Owns the single PollRuntime instance shared by every version partition.
 * all.cppm keeps using this exact class name, ctor and install() signature.
 */
class PollRouteHandlers
{
public:
    explicit PollRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_v1{context, m_runtime},
        m_input_support{context},
        m_policy{context, m_input_support},
        m_request_support{m_policy}
    {
        m_v1.bind_request_support(m_request_support);
        m_v1.bind_policy(m_policy);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v1.install(router);
    }

private:
    PollRuntime m_runtime;
    PollHandlersV1 m_v1;
    PollInputSupport m_input_support;
    PollPolicy m_policy;
    PollRequestSupport m_request_support;
};

} // namespace app::handlers::poll
