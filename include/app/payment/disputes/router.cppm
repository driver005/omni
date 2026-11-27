export module app_payment:disputes_handler;

import std;
import interfaces;
import core_router;
import app_context;

import :disputes_input;
import :context;
import :disputes_result;
import :disputes_runtime;
import :disputes_handlers_misc;
import core_behavior;
import core_serde;
import core_error;
import utils_openapi;
import app_model;
import app_auth;
import app_http;

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
        auto result =
            m_runtime.retrieve_disputes_list_profile(dto, m_request_support->context(req));
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
        auto result =
            m_runtime.retrieve_disputes_list_for_platform(dto, m_request_support->context(req));
        result.handle(sink);
    }

private:
    DisputesRuntime& m_runtime;            // SINGLE runtime, owned by the facade.
    DisputesInputSupport& m_input_support; // SINGLE stateless input helper, shared.
};

class DisputesHandlersRetrieve
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

    DisputesHandlersRetrieve(
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
        Router{router, "disputes"}.add_route(
            Route{":dispute_id"}
                .get(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        interfaces::io::ResponseSink sink{res, std::move(send)};

                        handle_retrieve_dispute(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch retrieve_dispute")
        );
    }

    // ── routes/disputes.rs:33-69  retrieve_dispute ────────────────────────────
    void handle_retrieve_dispute(
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

        ::app::model::disputes::DisputeRetrieveRequest request{};
        request.set_dispute_id(m_input_support.path_segment(req));
        if (request.get_dispute_id().empty()) {
            const auto error =
                core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing dispute id");
            error.send(sink);
            return;
        }
        if (const auto force = query.get("force_sync")) {
            request.set_force_sync(*force == "true" || *force == "1");
        }

        const auto& dto = scope;
        auto result = m_runtime.retrieve_dispute(dto, m_request_support->context(req));
        result.handle(sink);
    }

private:
    DisputesRuntime& m_runtime;            // SINGLE runtime, owned by the facade.
    DisputesInputSupport& m_input_support; // SINGLE stateless input helper, shared.
};

class DisputesHandlersSubmitAccept
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

    DisputesHandlersSubmitAccept(
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
        Router{router, "disputes"}.add_router(
            Router{router, "accept"}.add_route(
                Route{":dispute_id"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_accept_dispute(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch accept_dispute")
            )
        );
    }

    void install_evidence(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "disputes"}.add_route(
            Route{"evidence"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        interfaces::io::ResponseSink sink{res, std::move(send)};

                        handle_submit_dispute_evidence(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch submit_dispute_evidence")
        );
    }

    // ── routes/disputes.rs:441-479  accept_dispute ────────────────────────────
    void handle_accept_dispute(
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
                    ->authorize(req, "ProfileDisputeWrite", kDisputeProfileWriteSpec)) {
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

        ::app::model::disputes::DisputeId dispute_id{};
        dispute_id.set_dispute_id(m_input_support.path_segment(req));
        if (dispute_id.get_dispute_id().empty()) {
            const auto error =
                core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing dispute id");
            error.send(sink);
            return;
        }
        const auto& dto = scope;
        auto result = m_runtime.accept_dispute(dto, m_request_support->context(req));
        result.handle(sink);
    }

    // ── routes/disputes.rs:496-532  submit_dispute_evidence ───────────────────
    void handle_submit_dispute_evidence(
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
                    ->authorize(req, "ProfileDisputeWrite", kDisputeProfileWriteSpec)) {
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

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request");
            error.send(sink);
            return;
        }
        auto parsed = serde::Ser::deserialize<::app::model::disputes::SubmitEvidenceRequest>(
            "application/json",
            body
        );
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON");
            error.send(sink);
            return;
        }
        const auto& dto = scope;
        auto result = m_runtime.submit_dispute_evidence(dto, m_request_support->context(req));
        result.handle(sink);
    }

private:
    DisputesRuntime& m_runtime;            // SINGLE runtime, owned by the facade.
    DisputesInputSupport& m_input_support; // SINGLE stateless input helper, shared.
};

class DisputesHandlersEvidence
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

    DisputesHandlersEvidence(
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
                Route{"evidence"}
                    .put(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_attach_dispute_evidence(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch attach_dispute_evidence")
            )
            .add_route(
                Route{"evidence"}
                    .delt(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_delete_dispute_evidence(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch delete_dispute_evidence")
            )
            .add_router(
                Router{router, "evidence"}.add_route(
                    Route{":dispute_id"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_retrieve_dispute_evidence(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch retrieve_dispute_evidence")
                )
            );
    }

    // ── routes/disputes.rs:550-586  attach_dispute_evidence ───────────────────
    void handle_attach_dispute_evidence(
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
                    ->authorize(req, "ProfileDisputeWrite", kDisputeProfileWriteSpec)) {
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

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request");
            error.send(sink);
            return;
        }
        auto parsed = serde::Ser::deserialize<EvidenceUploadWire>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON");
            error.send(sink);
            return;
        }
        const auto& dto = scope;
        auto result = m_runtime.attach_dispute_evidence(dto, m_request_support->context(req));
        result.handle(sink);
    }

    // ── routes/disputes.rs:661-699  delete_dispute_evidence ───────────────────
    void handle_delete_dispute_evidence(
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
                    ->authorize(req, "ProfileDisputeWrite", kDisputeProfileWriteSpec)) {
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

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request");
            error.send(sink);
            return;
        }
        auto parsed = serde::Ser::deserialize<::app::model::disputes::DeleteEvidenceRequest>(
            "application/json",
            body
        );
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON");
            error.send(sink);
            return;
        }
        const auto& dto = scope;
        auto result = m_runtime.delete_dispute_evidence(dto, m_request_support->context(req));
        result.handle(sink);
    }

    // ── routes/disputes.rs:605-643  retrieve_dispute_evidence ─────────────────
    void handle_retrieve_dispute_evidence(
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

        ::app::model::disputes::DisputeId dispute_id{};
        dispute_id.set_dispute_id(m_input_support.path_segment(req));
        if (dispute_id.get_dispute_id().empty()) {
            const auto error =
                core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing dispute id");
            error.send(sink);
            return;
        }
        const auto& dto = scope;
        auto result = m_runtime.retrieve_dispute_evidence(dto, m_request_support->context(req));
        result.handle(sink);
    }

private:
    DisputesRuntime& m_runtime;            // SINGLE runtime, owned by the facade.
    DisputesInputSupport& m_input_support; // SINGLE stateless input helper, shared.
};

/** @brief Port of upstream `routes/disputes.rs` + `core/disputes.rs` (SHA 7736d55). */
class DisputesRouteHandlers
{
public:
    explicit DisputesRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_fetch_list{context, m_runtime, m_input_support},
        m_retrieve{context, m_runtime, m_input_support},
        m_submit_accept{context, m_runtime, m_input_support},
        m_misc{context, m_runtime, m_input_support},
        m_evidence{context, m_runtime, m_input_support},
        m_policy{context, m_input_support},
        m_request_support{m_policy}
    {
        m_fetch_list.bind_request_support(m_request_support);
        m_fetch_list.bind_policy(m_policy);
        m_retrieve.bind_request_support(m_request_support);
        m_retrieve.bind_policy(m_policy);
        m_submit_accept.bind_request_support(m_request_support);
        m_submit_accept.bind_policy(m_policy);
        m_misc.bind_request_support(m_request_support);
        m_misc.bind_policy(m_policy);
        m_evidence.bind_request_support(m_request_support);
        m_evidence.bind_policy(m_policy);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        // Original registration order preserved across family fragments.
        m_fetch_list.install(router);
        m_misc.install(router);
        m_fetch_list.install_platform(router);
        m_misc.install_platform(router);
        m_submit_accept.install(router);
        m_misc.install_aggregate(router);
        m_submit_accept.install_evidence(router);
        m_evidence.install(router);
        m_retrieve.install(router);
        m_fetch_list.install_fetch(router);
    }

private:
    DisputesRuntime m_runtime;            // SINGLE runtime instance, shared by every family.
    DisputesInputSupport m_input_support; // SINGLE stateless input helper, shared by every family.
    DisputesHandlersFetchList m_fetch_list;
    DisputesHandlersRetrieve m_retrieve;
    DisputesHandlersSubmitAccept m_submit_accept;
    DisputesHandlersMisc m_misc;
    DisputesHandlersEvidence m_evidence;
    DisputesPolicy m_policy;
    DisputesRequestSupport m_request_support;
};

} // namespace app::handlers::disputes
