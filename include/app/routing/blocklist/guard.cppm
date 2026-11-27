export module app_routing:blocklist_handlers_guard;

import std;
import interfaces;
import core_router;
import core_serde;
import app_model;
import utils_openapi;
import app_context;
import core_error;
import app_http;

import :blocklist_input;
import :blocklist_result;
import :blocklist_runtime;

export namespace app::handlers::blocklist {

class GuardHandlers
{
public:
    GuardHandlers(
        ::app::context::AppContext& ctx,
        BlocklistRuntime& runtime,
        BlocklistInputSupport& input_support,
        BlocklistRequestSupport& request_support
    ) noexcept :
        m_context{ctx},
        m_input_support{input_support},
        m_request_support{request_support},
        m_runtime{runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "blocklist"}.add_route(
            Route{"toggle"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_toggle_blocklist_guard(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch toggle_blocklist_guard")
        );
    }

    // routes/blocklist.rs:314-343 -> core/blocklist.rs:78-86 ->
    // utils::toggle_blocklist_guard_for_merchant (utils.rs:76-118). Merchant-wide, no profile.
    void handle_toggle_blocklist_guard(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied = m_request_support.authorize(req, "toggle_blocklist_guard", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        const auto query = ::app::http::RequestQuery{}(req);
        auto raw = query.get("status");
        if (!raw) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing required param: status"
            );
            error.send(sink);
            return;
        }
        bool status{false};
        if (*raw == "true") {
            status = true;
        } else if (*raw != "false") {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "invalid bool value for status"
            );
            error.send(sink);
            return;
        }

        const auto merchant = m_input_support.merchant_id_of(req);
        if (const auto bad = m_input_support.require_merchant(merchant)) {
            const auto& error = *bad;
            error.send(sink);
            return;
        }

        auto ctx = m_request_support.context(req);
        ctx.merchant_id = merchant;
        ctx.query["status"] = status ? "true" : "false";
        m_runtime.toggle_blocklist_guard(ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistInputSupport& m_input_support;

    BlocklistRequestSupport& m_request_support;
    BlocklistRuntime& m_runtime;
};

} // namespace app::handlers::blocklist
