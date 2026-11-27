export module app_routing:feature_matrix_handlers_v1;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_connector;
import app_metrics;
import app_store;
import app_services;

import :feature_matrix_input;
import :feature_matrix_result;
import :feature_matrix_runtime;
import :feature_matrix_runtime_common;

export namespace app::handlers::feature_matrix {

/** @brief v1 route family: the single upstream `fetch_feature_matrix` GET.
 *
 *  Upstream registers exactly one route (`fetch_feature_matrix`,
 *  `app.rs:3433-3435`) gated `all(feature = "olap", feature = "v1")`
 *  (`app.rs:3430`) under `&auth::NoAuth` and `LockAction::NotApplicable`
 *  (`routes/feature_matrix.rs:30-39`). There is no v2 scope for this domain,
 *  so only a v1 partition exists. Generation lives in `FetchFeatureMatrixRuntime`
 *  and `FeatureMatrixRuntimeSupport` (`routes/feature_matrix.rs:42-161`); no
 *  C++-only `/generate_feature_matrix` route exists — that identifier is an
 *  internal upstream helper only.
 */
class FeatureMatrixHandlersV1
{
public:
    FeatureMatrixHandlersV1(
        ::app::context::AppContext& ctx,
        FeatureMatrixRuntime& runtime,
        FeatureMatrixRequestSupport& request_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_request_support{request_support}
    {
    }

    void handle_fetch_feature_matrix(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // `#[instrument(skip_all)]` + Flow::FeatureMatrix (routes/feature_matrix.rs:19,25) and
        // api::server_wrap(..., &auth::NoAuth, LockAction::NotApplicable) (:30-38). The NO_AUTH
        // rule is a pass-through; no lock is acquired because the request is a read-only GET.
        const auto auth = m_request_support.authorize(req, "fetch_feature_matrix", {});
        if (auth) {
            const auto& error = *auth;
            error.send(res, send);
            return;
        }

        FetchFeatureMatrixInput input{req.body_as_string()};
        auto result = m_runtime.fetch_feature_matrix(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        auto wire =
            result->serialize(wire_fields<::app::dto::feature_matrix::FeatureMatrixListResponse>());
        if (!wire) {
            wire.error().send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::as_bytes(std::span{*wire}));
        send();
    }

    /** @brief Installs the single upstream v1 route.
     *
     *  Compile-time `all(feature = "olap", feature = "v1")` gate (app.rs:3430)
     *  expressed as a local constexpr, plus the existing runtime config gate
     *  facility used by blocklist. Paths, method, summary and order are unchanged.
     */
    void install_v1(core::router::RouterContext<>& router)
    {
        if constexpr (!catalog_support::kOlapV1Enabled) {
            return;
        }
        if (const auto enabled =
                m_context.config_store().get_value("route_features", "feature_matrix");
            enabled.has_value() && *enabled == "false") {
            return;
        }

        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "feature_matrix"}.add_route(
            Route{""}
                .get(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_fetch_feature_matrix(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch fetch_feature_matrix")
        );
    }

private:
    ::app::context::AppContext& m_context;
    FeatureMatrixRuntime& m_runtime;
    FeatureMatrixRequestSupport& m_request_support;
};

} // namespace app::handlers::feature_matrix
