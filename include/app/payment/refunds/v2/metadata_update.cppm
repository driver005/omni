export module app_payment:refunds_handlers_v2_metadata_update;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_context;
import core_error;
import app_auth;
import app_http;
import app_locking;

import :refunds_input;
import :refunds_result;
import :refunds_runtime;

export namespace app::handlers::refunds {

class RefundsV2MetadataUpdateHandlers
{
public:
    void bind_request_support(RefundsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    RefundsRequestSupport* m_request_support{nullptr};

    RefundsV2MetadataUpdateHandlers(
        ::app::context::AppContext& ctx,
        RefundsRuntime& runtime,
        RefundsInputSupport& input_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    /** @brief refunds_metadata_update — routes/refunds.rs:460-495 (v2). */
    void handle_refunds_metadata_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied =
                m_request_support->authorize(req, "V2Metadata", spec_for("V2Metadata"))) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request");
            error.send(res, send);
            return;
        }
        using Input = ::app::model::refunds::RefundMetadataUpdateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON");
            error.send(res, send);
            return;
        }
        const auto path_id = m_input_support.refund_id_from_path(req);
        if (!path_id.has_value()) {
            const auto error =
                core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing refund id in path");
            error.send(res, send);
            return;
        }
        RefundsMetadataUpdateInput input{*path_id, *parsed};

        auto result = m_runtime.metadata_update(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    // original install() route #19
    void install_route_19(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/refunds"}.add_router(
            Router{router, ":id"}.add_route(
                Route{"update-metadata"}
                    .put(
                        [this](auto& rq, auto& rs, auto sd)
                        {
                            handle_refunds_metadata_update(rq, rs, std::move(sd));
                        }
                    )
                    .summary("Hyperswitch refunds_metadata_update")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_19(router);
    }

private:
    ::app::context::AppContext& m_context;
    RefundsRuntime& m_runtime;
    RefundsInputSupport& m_input_support;
};

} // namespace app::handlers::refunds
