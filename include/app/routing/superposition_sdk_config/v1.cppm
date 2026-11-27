export module app_routing:superposition_sdk_config_handlers_v1;

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
        // Upstream step 1 (routes:63-66): get_payment_id_from_headers_or_payload, before auth.
        const std::string auth_header = ::app::http::HeaderReader{}.get(req, "authorization");
        const auto sdk_auth = m_input_support.decode_sdk_authorization(auth_header);
        const auto query = ::app::http::RequestQuery{}(req);
        const auto resolved =
            m_input_support.get_payment_id_from_headers_or_payload(auth_header, sdk_auth, query);
        if (resolved.error) {
            const auto& error = *resolved.error;
            const auto status_override = m_errors.status_override_for(error.get_kind_name());
            res.set_status(status_override.value_or(error.get_status()));
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
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
            res.set_status(status_override.value_or(error.get_status()));
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // The policy ran the same auth check; rebuild the authenticated context for the
        // runtime input without invoking the authorizer a second time.
        const auto auth_ctx = m_input_support.build_context(req, client_secret, sdk_auth);

        std::string sdk_profile_id{};
        if (sdk_auth && sdk_auth->decoded) {
            sdk_profile_id = sdk_auth->profile_id;
        }
        GetSdkConfigInput
            input{payment_id, sdk_profile_id, m_input_support.provider_merchant_id(req), auth_ctx};

        auto result = m_runtime.get_sdk_config(input);
        if (!result) {
            const auto& error = result.error();
            const auto status_override = m_errors.status_override_for(error.get_kind_name());
            res.set_status(status_override.value_or(error.get_status()));
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        auto wire = result->serialize(
            wire_fields<::app::dto::superposition_sdk_config::SuperPositionConfigResponse>()
        );
        if (!wire) {
            wire.error().send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::as_bytes(std::span{*wire}));
        send();
    }

    // Upstream routes::get_profile_sdk_config (routes:98-126).
    void handle_get_profile_sdk_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream HeaderAuth(PublishableKeyAuth{...}) (routes:119-122). Registry row:
        // registry.cppm:165.
        if (const auto denied = m_request_support.authorize(req, "get_profile_sdk_config", {})) {
            const auto& error = *denied;
            const auto status_override = m_errors.status_override_for(error.get_kind_name());
            res.set_status(status_override.value_or(error.get_status()));
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto auth_ctx = m_input_support.build_context(req, {}, std::nullopt);

        // Upstream path.into_inner() (routes:105); the payload is ignored.
        const std::string profile_id = m_input_support.profile_id_from_path(req);
        if (profile_id.empty()) {
            const auto error = m_errors.missing_required_field("profile_id");
            const auto status_override = m_errors.status_override_for(error.get_kind_name());
            res.set_status(status_override.value_or(error.get_status()));
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        GetProfileSdkConfigInput input{
            profile_id,
            m_input_support.provider_merchant_id(req),
            auth_ctx
        };

        auto result = m_runtime.get_profile_sdk_config(input);
        if (!result) {
            const auto& error = result.error();
            const auto status_override = m_errors.status_override_for(error.get_kind_name());
            res.set_status(status_override.value_or(error.get_status()));
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        auto wire = result->serialize(
            wire_fields<::app::dto::superposition_sdk_config::SuperPositionConfigResponse>()
        );
        if (!wire) {
            wire.error().send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::as_bytes(std::span{*wire}));
        send();
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

} // namespace app::handlers::superposition_sdk_config
