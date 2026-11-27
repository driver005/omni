export module app_payment:payouts_handlers_v1_crud;

import std;
import core_behavior;
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
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;
import app_locking;

import :payouts_input;
import :payouts_result;
import :payouts_runtime;

export namespace app::handlers::payouts {

class PayoutsCrudHandlersV1
{
public:
    void bind_request_support(PayoutsRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PayoutsRequestSupport* m_request_support{nullptr};

    explicit PayoutsCrudHandlersV1(
        PayoutsRuntime& runtime,
        PayoutsInputSupport& input_support
    ) noexcept :
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_payouts_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied =
                m_request_support->authorize(req, "payouts_create", spec_for("payouts_create"))) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(sink);
            return;
        }
        auto parsed =
            core::behavior::Serializer<::app::model::payouts::PayoutCreateRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }

        const auto header_merchant_id = m_input_support.header_merchant_id(req);
        const auto auth_merchant_id = m_input_support.auth_merchant_id(req);
        (void)m_input_support.org_id(req);
        const auto profile_id = m_input_support.profile_id(req, parsed->profile_id);
        const auto browser_info = m_input_support.browser_info(req, parsed->browser_info);
        const auto& dto = *parsed;

        auto result = m_runtime.create(dto, m_request_support->context(req));
        result.handle(sink);
    }

    void handle_payouts_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied =
                m_request_support
                    ->authorize(req, "payouts_retrieve", spec_for("payouts_retrieve"))) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        const auto path_id = m_input_support.payout_id_from_path(req);
        if (!path_id.has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payout id in path"
            );
            error.send(sink);
            return;
        }
        // Upstream uses `web::Query<PayoutRetrieveBody>` (`payouts.rs:90-97`); all
        // three fields come from the query string, never from a request body.
        const auto query = m_input_support.query(req);
        auto retrieve = serde::Ser::deserialize<::app::model::payouts::PayoutRetrieveBody>(
            "application/json",
            m_input_support.query_json(query, {"force_sync", "merchant_id", "expand_attempts"})
        );
        if (!retrieve) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "invalid PayoutRetrieveBody query"
            );
            error.send(sink);
            return;
        }
        const bool force_sync = retrieve->get_force_sync().value_or(false);
        const bool expand_attempts = retrieve->get_expand_attempts().value_or(false);
        const auto profile_id = m_input_support.profile_id(req, std::nullopt);

        auto result = m_runtime.retrieve(m_request_support->context(req));
        result.handle(sink);
    }

    void handle_payouts_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied =
                m_request_support->authorize(req, "payouts_update", spec_for("payouts_update"))) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(sink);
            return;
        }
        auto parsed =
            core::behavior::Serializer<::app::model::payouts::PayoutCreateRequest>::pass(req, sink);
        if (!parsed) {
            return;
        }

        const auto path_id = m_input_support.payout_id_from_path(req);
        if (!path_id.has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payout id in path"
            );
            error.send(sink);
            return;
        }

        const auto header_merchant_id = m_input_support.header_merchant_id(req);
        const auto profile_id = m_input_support.profile_id(req, parsed->profile_id);
        const auto browser_info = m_input_support.browser_info(req, parsed->browser_info);
        const auto& dto = *parsed;

        auto result = m_runtime.update(dto, m_request_support->context(req));
        result.handle(sink);
    }

    void handle_payouts_confirm(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied =
                m_request_support->authorize(req, "payouts_confirm", spec_for("payouts_confirm"))) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        using Input = ::app::model::payouts::PayoutCreateRequest;
        std::optional<Input> parsed{};
        const auto body = req.body_as_string();
        if (!body.empty()) {
            auto decoded = serde::Ser::deserialize<Input>("application/json", body);
            if (!decoded) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_JSON,
                    "invalid JSON"
                );
                error.send(sink);
                return;
            }
            parsed.emplace(std::move(*decoded));
        }

        const auto path_id = m_input_support.payout_id_from_path(req);
        if (!path_id.has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payout id in path"
            );
            error.send(sink);
            return;
        }

        auto result = m_runtime.confirm(m_request_support->context(req));
        result.handle(sink);
    }

    void handle_payouts_cancel(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied =
                m_request_support->authorize(req, "payouts_cancel", spec_for("payouts_cancel"))) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        const auto path_id = m_input_support.payout_id_from_path(req);
        if (!path_id.has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payout id in path"
            );
            error.send(sink);
            return;
        }

        auto result = m_runtime.cancel(m_request_support->context(req));
        result.handle(sink);
    }

    void handle_payouts_fulfill(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        if (const auto denied =
                m_request_support->authorize(req, "payouts_fulfill", spec_for("payouts_fulfill"))) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        const auto path_id = m_input_support.payout_id_from_path(req);
        if (!path_id.has_value()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing payout id in path"
            );
            error.send(sink);
            return;
        }

        auto result = m_runtime.fulfill(m_request_support->context(req));
        result.handle(sink);
    }

    using Route = utils::openapi::ApiRoute;
    using Router = utils::openapi::ApiRouter;

    [[nodiscard]] Router install(Router chain, core::router::RouterContext<>& router)
    {
        return std::move(chain)
            .add_route(
                Route{"create"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payouts_create(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payouts_create")
            )
            .add_route(
                Route{":payout_id"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payouts_retrieve(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payouts_retrieve")
            )
            .add_route(
                Route{":payout_id"}
                    .put(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            interfaces::io::ResponseSink sink{res, std::move(send)};

                            handle_payouts_update(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payouts_update")
            )
            .add_router(
                Router{router, ":payout_id"}.add_route(
                    Route{"confirm"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_payouts_confirm(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch payouts_confirm")
                )
            )
            .add_router(
                Router{router, ":payout_id"}.add_route(
                    Route{"cancel"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_payouts_cancel(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch payouts_cancel")
                )
            )
            .add_router(
                Router{router, ":payout_id"}.add_route(
                    Route{"fulfill"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_payouts_fulfill(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch payouts_fulfill")
                )
            );
    }

private:
    PayoutsRuntime& m_runtime;
    PayoutsInputSupport& m_input_support;
};

} // namespace app::handlers::payouts
