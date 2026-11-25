export module app_handlers_poll:handler;

import std;
import core_router;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_v1;

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
    explicit PollRouteHandlers(app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_v1{context, m_runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_v1.install(router);
    }

private:
    PollRuntime m_runtime;
    PollHandlersV1 m_v1;
};

} // namespace app::handlers::poll
