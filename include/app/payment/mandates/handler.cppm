export module app_payment:mandates_handler;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import core_cache;
import core_error;
import app_http;
import app_metrics;
import app_store;
import app_connector;
import app_keystore;

import :mandates_input;
import :mandates_result;
import :mandates_runtime;

export namespace app::handlers::mandates {

/**
 * @brief v1 mandate route handlers, ported from the upstream router core.
 *
 *  Upstream references (SHA 7736d55adaf235f7af74e133ae408573b1826823):
 *   - routes/mandates.rs:14-40,42-68,69-101  (extractors, server_wrap, auth, LockAction)
 *   - core/mandate.rs:33-58 (get_mandate), 60-165 (revoke_mandate), 403-426 (list)
 *   - core/mandate/helpers.rs:13-44 (get_profile_id_for_mandate), 46-85 (get_mandate_type)
 *   - core/mandate/utils.rs:13-104 (construct_mandate_revoke_router_data)
 *   - types/api/mandates.rs:36-135 (from_db_mandate wire mapping)
 *   - api_models/mandates.rs:10-13,15-33,35-86,139-179 (DTOs)
 *
 *  Every upstream core call is mirrored in the runtime partitions. External infrastructure
 *  (processor key store, locker, MCA/connector-data chain, DB storage_scheme) is replaced
 *  by the in-process AppContext facilities with the same call order and observable shapes.
 */
class MandatesRouteHandlers
{
public:
    /** @brief Upstream get_mandate (core/mandate.rs:33-58). */
    void handle_get_mandate(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto denied = m_input_support.authorize(auth, "get_mandate")) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }

        const auto mandate_id = m_input_support.mandate_id_from_path(req, 1);
        if (!mandate_id.has_value() || mandate_id->empty()) {
            const auto error = m_input_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing mandate id in path"
            );
            error.send(res, send);
            return;
        }

        const auto merchant = m_input_support.merchant_scope(req, auth);
        GetMandateInput input{merchant, *mandate_id};
        auto result = m_runtime.get_mandate(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    /** @brief Upstream revoke_mandate (core/mandate.rs:60-165). */
    void handle_revoke_mandate(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto denied = m_input_support.authorize(auth, "revoke_mandate")) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }

        const auto mandate_id = m_input_support.mandate_id_from_path(req, 2);
        if (!mandate_id.has_value() || mandate_id->empty()) {
            const auto error = m_input_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing mandate id in path"
            );
            error.send(res, send);
            return;
        }

        const auto merchant = m_input_support.merchant_scope(req, auth);
        RevokeMandateInput input{merchant, *mandate_id};
        auto result = m_runtime.revoke_mandate(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    /** @brief Upstream retrieve_mandates_list (core/mandate.rs:403-426). */
    void handle_retrieve_mandates_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto denied = m_input_support.authorize(auth, "retrieve_mandates_list")) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }

        // web::Query<MandateListConstraints> (routes/mandates.rs:74;
        // api_models/mandates.rs:139-179).
        const auto query = ::app::http::RequestQuery{}(req);
        const auto constraints = m_input_support.constraints_from_query(query);

        auto merchant_filter = query.get("merchant_id");
        const auto merchant = m_input_support.merchant_scope(req, auth);
        if (!merchant_filter.has_value() && !merchant.empty()) {
            merchant_filter = merchant;
        }

        RetrieveMandatesListInput input{constraints, merchant_filter};
        auto result = m_runtime.retrieve_mandates_list(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        // ApplicationResponse::Json(mandates_list) -> bare array; the typed vector
        // is serialized directly (same as the old send_json(..., true)).
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_items()));
        send();
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "mandates"}
            .add_route(
                Route{"list"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_retrieve_mandates_list(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch retrieve_mandates_list")
            )
            .add_route(
                Route{":id"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_mandate(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_mandate")
            )
            .add_router(
                Router{router, "revoke"}.add_route(
                    Route{":id"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_revoke_mandate(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch revoke_mandate")
                )
            );
    }

    explicit MandatesRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context}
    {
    }

private:
    MandatesRuntime m_runtime;
    MandatesInputSupport m_input_support;
};

} // namespace app::handlers::mandates
