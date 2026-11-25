export module app_handlers_blocklist:handlers_export;

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

class ExportHandlers
{
public:
    ExportHandlers(app::context::AppContext& ctx, BlocklistRuntime& runtime) noexcept :
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
            Route{"export"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_create_blocklist_export(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch create_blocklist_export")
        );
    }

    // routes/blocklist.rs:549-584 -> core/blocklist.rs:140-149 ->
    // export::initiate_blocklist_export (export.rs:73-162).
    void handle_create_blocklist_export(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied =
                m_input_support.authorize_operation(req, "create_blocklist_export")) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        std::string transaction_type;
        const auto query = app::http::RequestQuery{}(req);
        if (const auto raw = query.get("transaction_type")) {
            const auto parsed = m_input_support.parse_transaction_type(*raw);
            if (!parsed) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "unknown variant for transaction_type"
                );
                res.set_status(error.get_status());
                res.set_body(serde::Ser::serialize("application/json", error));
                send();
                return;
            }
            transaction_type = *parsed;
        }

        const auto merchant = m_input_support.merchant_id_of(req);
        if (const auto bad = m_input_support.require_merchant(merchant)) {
            const auto& error = *bad;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto profile = m_input_support.resolve_required_profile(req, merchant);
        if (profile.failure) {
            const auto& error = *profile.failure;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        CreateBlocklistExportInput input{merchant, profile.id, transaction_type};
        auto result = m_runtime.create_blocklist_export(input);
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
