export module app_handlers_blocklist:handlers_guard;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_context;
import core_error;
import app_http;

import :input;
import :result;
import :runtime;

export namespace app::handlers::blocklist {

class GuardHandlers
{
public:
    GuardHandlers(app::context::AppContext& ctx, BlocklistRuntime& runtime) noexcept :
        m_context{ctx},
        m_input_support{ctx},
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
        if (const auto denied =
                m_input_support.authorize_operation(req, "toggle_blocklist_guard")) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto query = app::http::RequestQuery{}(req);
        auto raw = query.get("status");
        if (!raw) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing required param: status"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
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
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto merchant = m_input_support.merchant_id_of(req);
        if (const auto bad = m_input_support.require_merchant(merchant)) {
            const auto& error = *bad;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        ToggleBlocklistGuardInput input{merchant, status};
        auto result = m_runtime.toggle_blocklist_guard(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

private:
    app::context::AppContext& m_context;
    BlocklistInputSupport m_input_support;
    BlocklistRuntime& m_runtime;
};

} // namespace app::handlers::blocklist
