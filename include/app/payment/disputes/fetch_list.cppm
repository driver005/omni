export module app_payment:disputes_handlers_fetch_list;

import std;
import core_behavior;
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

class DisputesHandlersFetchList
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

    DisputesHandlersFetchList(
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
                Route{"list"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_retrieve_disputes_list(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch retrieve_disputes_list")
            )
            .add_router(
                Router{router, "profile"}.add_route(
                    Route{"list"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_retrieve_disputes_list_profile(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch retrieve_disputes_list_profile")
                )
            );
    }

    void install_platform(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "disputes"}.add_router(
            Router{router, "platform"}.add_route(
                Route{"list"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_retrieve_disputes_list_for_platform(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch retrieve_disputes_list_for_platform")
            )
        );
    }

    void install_fetch(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "disputes"}.add_router(
            Router{router, ":connector_id"}.add_route(
                Route{"fetch"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
        interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_fetch_disputes(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch fetch_disputes")
            )
        );
    }

    // ── routes/disputes.rs:73-113  fetch_disputes ─────────────────────────────
    void handle_fetch_disputes(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto denied =
                m_request_support->authorize(req, "ProfileDisputeRead", kDisputeProfileReadSpec)) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        const auto scope = m_request_support->scope(req);
        if (const auto invalid = m_request_support->scope_error(scope)) {
            const auto& error = *invalid;
            error.send(sink);
            return;
        }

        const std::string connector_id = m_input_support.path_segment(req, 1);
        if (connector_id.empty()) {
            const auto error =
                core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing connector id");
            error.send(sink);
            return;
        }

        ::app::model::disputes::DisputeFetchQueryData payload{};
        payload.set_fetch_from(query.get("fetch_from").value_or(std::string{}));
        payload.set_fetch_till(query.get("fetch_till").value_or(std::string{}));
        if (const auto invalid = m_input_support.parse_fetch_window(payload)) {
            const auto& error = *invalid;
            error.send(sink);
            return;
        }

        const auto& dto = scope;
        auto result = m_runtime.fetch_disputes(dto, m_request_support->context(req));
        result.handle(sink);
    }

    // ── routes/disputes.rs:140-175  retrieve_disputes_list ────────────────────
    void handle_retrieve_disputes_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto denied =
                m_request_support
                    ->authorize(req, "MerchantDisputeRead", kDisputeMerchantReadSpec)) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        const auto scope = m_request_support->scope(req);
        if (const auto invalid = m_request_support->scope_error(scope)) {
            const auto& error = *invalid;
            error.send(sink);
            return;
        }

        const auto constraints = m_input_support.list_constraints_from(query);
        const auto& dto = scope;
        auto result = m_runtime.retrieve_disputes_list(dto, m_request_support->context(req));
        result.handle(sink);
    }

    // ── routes/disputes.rs:300-335  retrieve_disputes_list_profile ────────────
    void handle_retrieve_disputes_list_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto denied =
                m_request_support->authorize(req, "ProfileDisputeRead", kDisputeProfileReadSpec)) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        const auto scope = m_request_support->scope(req);
        if (const auto invalid = m_request_support->scope_error(scope)) {
            const auto& error = *invalid;
            error.send(sink);
            return;
        }

        const auto constraints = m_input_support.list_constraints_from(query);
        const std::vector<std::string> profiles = scope.profile_id.empty()
                                                      ? std::vector<std::string>{}
                                                      : std::vector<std::string>{scope.profile_id};
        const auto& dto = scope;
        auto result = m_runtime.retrieve_disputes_list_profile(dto, m_request_support->context(req));
        result.handle(sink);
    }

    // ── routes/disputes.rs:199-229  retrieve_disputes_list_for_platform ───────
    void handle_retrieve_disputes_list_for_platform(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto denied =
                m_request_support
                    ->authorize(req, "MerchantDisputeRead", kDisputePlatformMerchantReadSpec)) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        auto scope = m_request_support->scope(req);
        if (const auto invalid = m_policy->platform_scope_error(scope)) {
            const auto& error = *invalid;
            error.send(sink);
            return;
        }

        auto constraints = m_input_support.platform_constraints_from(query);
        if (const auto processor = query.get("processor_merchant_id")) {
            ::app::model::disputes::MerchantIdValue merchant{};
            merchant.set_value(*processor);
            constraints.set_processor_merchant_id(merchant);
        }
        const auto& dto = scope;
        auto result = m_runtime.retrieve_disputes_list_for_platform(dto, m_request_support->context(req));
        result.handle(sink);
    }

private:
    DisputesRuntime& m_runtime;            // SINGLE runtime, owned by the facade.
    DisputesInputSupport& m_input_support; // SINGLE stateless input helper, shared.
};

} // namespace app::handlers::disputes
