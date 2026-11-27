export module app_routing:superposition_sdk_config_router;
import core_behavior;

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
import app_store;
import app_http;
import app_crypto;
import app_metrics;
import app_services;

import :superposition_sdk_config_input;
import :superposition_sdk_config_result;
import :superposition_sdk_config_runtime;

export namespace app::handlers::superposition_sdk_config {


/** @brief v1 route family: the two upstream GETs registered under
 *  `v1/sdk/configs/...` (app.rs:3584-3595). Both adapters are verbatim from the
 *  original combined handler; the facade owns the single runtime and input
 *  support and shares them by reference. No v2 scope exists for this domain.
 */
class SuperpositionSdkConfigHandlersV1
{
public:
    SuperpositionSdkConfigHandlersV1(
        ::app::context::AppContext& ctx,
        SuperpositionSdkConfigRuntime& runtime,
        SuperpositionSdkConfigInputSupport& input_support,
        SuperpositionSdkConfigRequestSupport& request_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_input_support{input_support},
        m_request_support{request_support}
    {
    }

    // Upstream routes::get_sdk_config (routes:52-95).
    void handle_get_sdk_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // Upstream step 1 (routes:63-66): get_payment_id_from_headers_or_payload, before auth.
        const std::string auth_header = ::app::http::HeaderReader{}.get(req, "authorization");
        const auto sdk_auth = m_input_support.decode_sdk_authorization(auth_header);
        const auto query = ::app::http::RequestQuery{}(req);
        const auto resolved =
            m_input_support.get_payment_id_from_headers_or_payload(auth_header, sdk_auth, query);
        if (resolved.error) {
            const auto& error = *resolved.error;
            const auto status_override = m_errors.status_override_for(error.get_kind_name());
            sink.set_status(status_override.value_or(error.get_status()));
            sink.set_body(serde::Ser::serialize("application/json", error));
            sink.send();
            return;
        }
        const std::string payment_id = resolved.value->payment_id;
        const std::string client_secret = resolved.value->client_secret;

        // Upstream step 2 (routes:68-77): check_sdk_auth_and_get_auth with
        // ApiKeyAuth{allow_connected_scope_operation, allow_platform_self_operation}. The local
        // registry row for "get_sdk_config" is the canonical rule (registry.cppm:174).
        if (const auto denied = m_request_support.authorize(req, "get_sdk_config", {})) {
            const auto& error = *denied;
            const auto status_override = m_errors.status_override_for(error.get_kind_name());
            sink.set_status(status_override.value_or(error.get_status()));
            sink.set_body(serde::Ser::serialize("application/json", error));
            sink.send();
            return;
        }

        // The policy ran the same auth check; carry its identity into the runtime context.
        auto ctx = m_request_support.context(req);
        ctx.query["payment_id"] = payment_id;
        ctx.client_secret = client_secret;
        if (sdk_auth && sdk_auth->decoded && !sdk_auth->profile_id.empty()) {
            ctx.profile_id = sdk_auth->profile_id;
        }

        auto result = m_runtime.get_sdk_config(ctx);
        if (!result) {
            const auto& error = result.error();
            const auto status_override = m_errors.status_override_for(error.get_kind_name());
            sink.set_status(status_override.value_or(error.get_status()));
            sink.set_body(serde::Ser::serialize("application/json", error));
            sink.send();
            return;
        }

        result.handle(sink);
    }

    // Upstream routes::get_profile_sdk_config (routes:98-126).
    void handle_get_profile_sdk_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // Upstream HeaderAuth(PublishableKeyAuth{...}) (routes:119-122). Registry row:
        // registry.cppm:165.
        if (const auto denied = m_request_support.authorize(req, "get_profile_sdk_config", {})) {
            const auto& error = *denied;
            const auto status_override = m_errors.status_override_for(error.get_kind_name());
            sink.set_status(status_override.value_or(error.get_status()));
            sink.set_body(serde::Ser::serialize("application/json", error));
            sink.send();
            return;
        }
        // Upstream path.into_inner() (routes:105); the payload is ignored.
        const std::string profile_id = m_input_support.profile_id_from_path(req);
        if (profile_id.empty()) {
            const auto error = m_errors.missing_required_field("profile_id");
            const auto status_override = m_errors.status_override_for(error.get_kind_name());
            sink.set_status(status_override.value_or(error.get_status()));
            sink.set_body(serde::Ser::serialize("application/json", error));
            sink.send();
            return;
        }

        auto ctx = m_request_support.context(req);
        ctx.profile_id = profile_id;

        auto result = m_runtime.get_profile_sdk_config(ctx);
        if (!result) {
            const auto& error = result.error();
            const auto status_override = m_errors.status_override_for(error.get_kind_name());
            sink.set_status(status_override.value_or(error.get_status()));
            sink.set_body(serde::Ser::serialize("application/json", error));
            sink.send();
            return;
        }

        result.handle(sink);
    }

    // Upstream app.rs SdkConfig::server (app.rs:3584-3595): exactly the two GET routes below.
    void install_v1(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "superposition_sdk_config"}
            .add_router(
                Router{router, "v1"}.add_router(
                    Router{router, "sdk"}.add_router(
                        Router{router, "configs"}.add_router(
                            Router{router, ":platform"}.add_route(
                                Route{"sdk_config.json"}
                                    .get(
                                        [this](
                                            interfaces::io::IRequest& req,
                                            interfaces::io::IResponse& res,
                                            std::function<void()> send
                                        )
                                        {
                                            handle_get_sdk_config(req, res, std::move(send));
                                        }
                                    )
                                    .summary("Hyperswitch get_sdk_config")
                            )
                        )
                    )
                )
            )
            .add_router(
                Router{router, "v1"}.add_router(
                    Router{router, "sdk"}.add_router(
                        Router{router, "configs"}.add_router(
                            Router{router, ":platform"}.add_router(
                                Router{router, ":profile_id"}.add_route(
                                    Route{"sdk_config.json"}
                                        .get(
                                            [this](
                                                interfaces::io::IRequest& req,
                                                interfaces::io::IResponse& res,
                                                std::function<void()> send
                                            )
                                            {
                                                handle_get_profile_sdk_config(
                                                    req,
                                                    res,
                                                    std::move(send)
                                                );
                                            }
                                        )
                                        .summary("Hyperswitch get_profile_sdk_config")
                                )
                            )
                        )
                    )
                )
            );
    }

private:
    ::app::context::AppContext& m_context;
    SuperpositionSdkConfigRuntime& m_runtime;
    SuperpositionSdkConfigInputSupport& m_input_support;
    SuperpositionSdkConfigRequestSupport& m_request_support;
    SdkConfigErrorSupport m_errors{};
};

/**
 * @brief Facade ("handler of handlers").
 *
 * Owns the ONE shared `SuperpositionSdkConfigRuntime` and the single input
 * support object; the v1 partition holds references to them, never its own
 * instances. Both upstream routes are v1 (`app.rs:3584-3595`), so there is no
 * v2 partition. The public class name, ctor `(AppContext&)` and `install(router)`
 * are unchanged, so all.cppm/routes.cppm need no edits. Business logic,
 * input.cppm, result.cppm and runtime/** stay untouched.
 */
class SuperpositionSdkConfigRouteHandlers
{
public:
    explicit SuperpositionSdkConfigRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_policy{m_input_support},
        m_request_support{m_policy},
        m_v1{context, m_runtime, m_input_support, m_request_support}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v1.install_v1(router);
    }

private:
    SuperpositionSdkConfigRuntime m_runtime;            // SINGLE runtime - no duplicated state.
    SuperpositionSdkConfigInputSupport m_input_support; // SINGLE parse/auth support.
    SuperpositionSdkConfigPolicy m_policy;
    SuperpositionSdkConfigRequestSupport m_request_support;
    SuperpositionSdkConfigHandlersV1 m_v1; // Holds AppContext& + refs above.
};

} // namespace app::handlers::superposition_sdk_config
