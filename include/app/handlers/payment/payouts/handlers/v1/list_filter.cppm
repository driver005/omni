export module app_payment:payouts_handlers_v1_list_filter;

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

class PayoutsListFilterHandlersV1
{
public:
    explicit PayoutsListFilterHandlersV1(
        PayoutsRuntime& runtime,
        PayoutsInputSupport& input_support
    ) noexcept :
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_payouts_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize_operation(req, "payouts_list")) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto query = m_input_support.query(req);
        auto constraints = serde::Ser::deserialize<::app::model::payouts::PayoutListConstraints>(
            "application/json",
            m_input_support.query_json(
                query,
                {"customer_id", "starting_after", "ending_before", "limit", "created"}
            )
        );
        if (!constraints) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "invalid PayoutListConstraints query"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        PayoutsListInput input{*constraints, m_input_support.time_range_from_query(query)};
        auto result = m_runtime.list(input);
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

    void handle_payouts_list_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize_operation(req, "payouts_list_profile")) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto query = m_input_support.query(req);
        auto constraints = serde::Ser::deserialize<::app::model::payouts::PayoutListConstraints>(
            "application/json",
            m_input_support.query_json(
                query,
                {"customer_id", "starting_after", "ending_before", "limit", "created"}
            )
        );
        if (!constraints) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "invalid PayoutListConstraints query"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        PayoutsListProfileInput input{
            *constraints,
            m_input_support.time_range_from_query(query),
            m_input_support.profile_id(req, std::nullopt)
        };
        auto result = m_runtime.list_profile(input);
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

    void handle_payouts_list_by_filter(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied =
                m_input_support.authorize_operation(req, "payouts_list_by_filter")) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        using Input = ::app::model::payouts::PayoutListFilterConstraints;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto query = m_input_support.query(req);
        PayoutListQuery query_values{};
        query_values.payout_id = query.get("payout_id").value_or("");
        query_values.customer_id = query.get("customer_id").value_or("");
        query_values.offset = query.get("offset");
        query_values.limit = query.get("limit");
        PayoutsListByFilterInput input{*parsed, std::string{body}, query_values};

        auto result = m_runtime.list_by_filter(input);
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

    void handle_payouts_list_by_filter_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied =
                m_input_support.authorize_operation(req, "payouts_list_by_filter_profile")) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        using Input = ::app::model::payouts::PayoutListFilterConstraints;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto query = m_input_support.query(req);
        PayoutListQuery query_values{};
        query_values.payout_id = query.get("payout_id").value_or("");
        query_values.customer_id = query.get("customer_id").value_or("");
        query_values.offset = query.get("offset");
        query_values.limit = query.get("limit");
        PayoutsListByFilterProfileInput input{
            *parsed,
            std::string{body},
            query_values,
            m_input_support.profile_id(req, parsed->profile_id)
        };

        auto result = m_runtime.list_by_filter_profile(input);
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

    void handle_payouts_list_available_filters_for_merchant(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize_operation(
                req,
                "payouts_list_available_filters_for_merchant"
            )) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        PayoutsListAvailableFiltersForMerchantInput input{std::string{body}};
        auto result = m_runtime.list_available_filters_for_merchant(input);
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

    void handle_payouts_list_available_filters_for_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize_operation(
                req,
                "payouts_list_available_filters_for_profile"
            )) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        PayoutsListAvailableFiltersForProfileInput input{
            std::string{body},
            m_input_support.profile_id(req, std::nullopt)
        };
        auto result = m_runtime.list_available_filters_for_profile(input);
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

    using Route = utils::openapi::ApiRoute;
    using Router = utils::openapi::ApiRouter;

    [[nodiscard]] Router install(Router chain, core::router::RouterContext<>& router)
    {
        return std::move(chain)
            .add_route(
                Route{"list"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_payouts_list(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payouts_list")
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
                                handle_payouts_list_profile(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch payouts_list_profile")
                )
            )
            .add_route(
                Route{"list"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_payouts_list_by_filter(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch payouts_list_by_filter")
            )
            .add_router(
                Router{router, "profile"}.add_route(
                    Route{"list"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_payouts_list_by_filter_profile(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch payouts_list_by_filter_profile")
                )
            )
            .add_route(
                Route{"filter"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_payouts_list_available_filters_for_merchant(
                                req,
                                res,
                                std::move(send)
                            );
                        }
                    )
                    .summary("Hyperswitch payouts_list_available_filters_for_merchant")
            )
            .add_router(
                Router{router, "profile"}.add_route(
                    Route{"filter"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_payouts_list_available_filters_for_profile(
                                    req,
                                    res,
                                    std::move(send)
                                );
                            }
                        )
                        .summary("Hyperswitch payouts_list_available_filters_for_profile")
                )
            );
    }

private:
    PayoutsRuntime& m_runtime;
    PayoutsInputSupport& m_input_support;
};

} // namespace app::handlers::payouts
