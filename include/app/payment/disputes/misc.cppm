export module app_payment:disputes_handlers_misc;

import std;
import interfaces;
import core_router;
import core_serde;
import core_error;
import utils_openapi;
import app_model;
import app_context;
import app_auth;
import app_http;

import :disputes_input;
import :disputes_result;
import :disputes_runtime;

export namespace app::handlers::disputes {

class DisputesHandlersMisc
{
public:
    void bind_request_support(DisputesRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    DisputesRequestSupport* m_request_support{nullptr};

    void bind_policy(DisputesPolicy& p) noexcept
    {
        m_policy = &p;
    }

    DisputesPolicy* m_policy{nullptr};

    DisputesHandlersMisc(
        ::app::context::AppContext& context,
        DisputesRuntime& runtime,
        DisputesInputSupport& input_support
    ) noexcept :
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "disputes"}
            .add_route(
                Route{"filter"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_disputes_filters(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_disputes_filters")
            )
            .add_router(
                Router{router, "profile"}.add_route(
                    Route{"filter"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_get_disputes_filters_profile(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch get_disputes_filters_profile")
                )
            );
    }

    void install_platform(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "disputes"}.add_router(
            Router{router, "platform"}.add_route(
                Route{"filter"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_platform_disputes_filters(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_platform_disputes_filters")
            )
        );
    }

    void install_aggregate(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "disputes"}
            .add_route(
                Route{"aggregate"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_disputes_aggregate(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_disputes_aggregate")
            )
            .add_router(
                Router{router, "profile"}.add_route(
                    Route{"aggregate"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_get_disputes_aggregate_profile(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch get_disputes_aggregate_profile")
                )
            );
    }

    // ── routes/disputes.rs:350-375  get_disputes_filters ──────────────────────
    void handle_get_disputes_filters(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto denied =
                m_request_support
                    ->authorize(req, "MerchantDisputeRead", kDisputeMerchantReadSpec)) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto scope = m_request_support->scope(req);
        if (const auto invalid = m_request_support->scope_error(scope)) {
            const auto& error = *invalid;
            error.send(res, send);
            return;
        }

        GetDisputesFiltersInput input{scope};
        auto result = m_runtime.get_disputes_filters(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    // ── routes/disputes.rs:390-422  get_disputes_filters_profile ──────────────
    void handle_get_disputes_filters_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto denied =
                m_request_support->authorize(req, "ProfileDisputeRead", kDisputeProfileReadSpec)) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto scope = m_request_support->scope(req);
        if (const auto invalid = m_request_support->scope_error(scope)) {
            const auto& error = *invalid;
            error.send(res, send);
            return;
        }

        const std::vector<std::string> profiles = scope.profile_id.empty()
                                                      ? std::vector<std::string>{}
                                                      : std::vector<std::string>{scope.profile_id};
        GetDisputesFiltersProfileInput input{scope, profiles};
        auto result = m_runtime.get_disputes_filters_profile(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    // ── routes/disputes.rs:244-272  get_platform_disputes_filters ─────────────
    void handle_get_platform_disputes_filters(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto denied =
                m_request_support
                    ->authorize(req, "MerchantDisputeRead", kDisputePlatformMerchantReadSpec)) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto scope = m_request_support->scope(req);
        if (const auto invalid = m_policy->platform_scope_error(scope)) {
            const auto& error = *invalid;
            error.send(res, send);
            return;
        }

        GetPlatformDisputesFiltersInput input{scope};
        auto result = m_runtime.get_platform_disputes_filters(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    // ── routes/disputes.rs:702-738  get_disputes_aggregate ────────────────────
    void handle_get_disputes_aggregate(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto denied =
                m_request_support
                    ->authorize(req, "MerchantDisputeRead", kDisputeMerchantReadSpec)) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto scope = m_request_support->scope(req);
        if (const auto invalid = m_request_support->scope_error(scope)) {
            const auto& error = *invalid;
            error.send(res, send);
            return;
        }

        GetDisputesAggregateInput input{scope, m_input_support.time_range_from(query)};
        auto result = m_runtime.get_disputes_aggregate(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    // ── routes/disputes.rs:742-778  get_disputes_aggregate_profile ────────────
    void handle_get_disputes_aggregate_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto denied =
                m_request_support->authorize(req, "ProfileDisputeRead", kDisputeProfileReadSpec)) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto scope = m_request_support->scope(req);
        if (const auto invalid = m_request_support->scope_error(scope)) {
            const auto& error = *invalid;
            error.send(res, send);
            return;
        }

        const std::vector<std::string> profiles = scope.profile_id.empty()
                                                      ? std::vector<std::string>{}
                                                      : std::vector<std::string>{scope.profile_id};
        GetDisputesAggregateProfileInput input{
            scope,
            m_input_support.time_range_from(query),
            profiles
        };
        auto result = m_runtime.get_disputes_aggregate_profile(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

private:
    DisputesRuntime& m_runtime;            // SINGLE runtime, owned by the facade.
    DisputesInputSupport& m_input_support; // SINGLE stateless input helper, shared.
};

} // namespace app::handlers::disputes
