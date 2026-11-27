export module app_payment:revenue_recovery_data_backfill_handlers_v2;

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
import app_metrics;
import app_store;
import app_crypto;
import app_locking;
import app_services;

import :revenue_recovery_data_backfill_input;
import :revenue_recovery_data_backfill_result;
import :revenue_recovery_data_backfill_runtime;

export namespace app::handlers::revenue_recovery_data_backfill {

class RevenueRecoveryDataBackfillHandlersV2
{
public:
    void bind_request_support(RevenueRecoveryDataBackfillRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    RevenueRecoveryDataBackfillRequestSupport* m_request_support{nullptr};

    /**
     * Upstream: routes/revenue_recovery_data_backfill.rs:19-73 ->
     * core/revenue_recovery_data_backfill.rs:23-64.
     */
    void handle_revenue_recovery_data_backfill(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream parses cutoff_time and the multipart form BEFORE server_wrap
        // applies V2AdminApiAuth (routes:29-71); keep that order so malformed input
        // is rejected by the route regardless of the caller's permissions.
        const auto cutoff = m_input_support.cutoff_datetime_from_query(req);
        if (!cutoff) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                cutoff.error()
            );
            error.send(res, send);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }

        using Input = ::app::dto::revenue_recovery_data_backfill::CsvParsingResult;
        std::optional<Input> parsed{};
        std::string parse_error{};
        if (m_input_support.is_json_body(body)) {
            auto decoded = serde::Ser::deserialize<Input>("application/json", body);
            if (!decoded) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_JSON,
                    "invalid JSON"
                );
                error.send(res, send);
                return;
            }
            parsed = std::move(*decoded);
        } else {
            // Local substitute for MultipartForm<RevenueRecoveryDataBackfillForm>:
            // validate_and_get_records_with_errors (api_models:298-331), run in-process.
            parsed = m_input_support.parse_backfill_csv(
                m_input_support.unwrap_multipart_file(body),
                parse_error
            );
            if (!parsed) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "CSV parsing error: " +
                        (parse_error.empty() ? std::string{"invalid CSV file"} : parse_error)
                );
                error.send(res, send);
                return;
            }
        }

        // server_wrap / V2AdminApiAuth is applied after route-level parsing (routes:57-71)
        // but before the core function mutates any token.
        if (const auto auth_error = m_request_support->authorize(
                req,
                "revenue_recovery_data_backfill",
                spec_for("revenue_recovery_data_backfill")
            )) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }

        RevenueRecoveryDataBackfillInput input{std::move(*parsed), *cutoff};
        auto result = m_runtime.backfill(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    /**
     * Upstream: routes/revenue_recovery_data_backfill.rs:75-104 ->
     * core/revenue_recovery/retry_stats/migration.rs:21-130.
     */
    void handle_revenue_recovery_retry_stats_migration(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream validates the multipart CSV before server_wrap applies auth
        // (routes:83-95); keep that order for malformed uploads.
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }

        using Input = ::app::dto::revenue_recovery_data_backfill::RetryStatsMigrationCsvResult;
        std::optional<Input> parsed{};
        std::string parse_error{};
        if (m_input_support.is_json_body(body)) {
            auto decoded = serde::Ser::deserialize<Input>("application/json", body);
            if (!decoded) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_JSON,
                    "invalid JSON"
                );
                error.send(res, send);
                return;
            }
            parsed = std::move(*decoded);
        } else {
            // validate_and_get_records (api_models:340-379) aborts the whole upload on any
            // row parse failure and reports at most ten row messages.
            parsed = m_input_support.parse_retry_stats_csv(
                m_input_support.unwrap_multipart_file(body),
                parse_error
            );
            if (!parsed) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "failed to parse the retry stats migration CSV file: " + parse_error
                );
                error.send(res, send);
                return;
            }
        }

        if (const auto auth_error = m_request_support->authorize(
                req,
                "revenue_recovery_retry_stats_migration",
                spec_for("revenue_recovery_retry_stats_migration")
            )) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }

        RetryStatsMigrationInput input{std::move(*parsed)};
        auto result = m_runtime.retry_stats_migration(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    /**
     * Upstream: routes/revenue_recovery_data_backfill.rs:106-128 ->
     * core/revenue_recovery_data_backfill.rs:157-261.
     */
    void handle_update_revenue_recovery_additional_redis_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_request_support->authorize(
                req,
                "update_revenue_recovery_additional_redis_data",
                spec_for("update_revenue_recovery_additional_redis_data")
            )) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }

        // Upstream ScheduledAtUpdate has a custom Deserializer (api_models:228-253):
        // a JSON string "null" -> SetToNull, any other string -> iso8601 datetime.
        // The local DTO enum cannot carry a datetime, so handle that field from the
        // generic tree first and strip it before the typed decode.
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!generic) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        std::optional<ScheduledUpdate> scheduled_update{};
        if (auto* root = std::get_if<serde::Value::Object>(&generic->get())) {
            auto parsed_schedule = m_input_support.parse_scheduled_update(*root);
            if (!parsed_schedule) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    parsed_schedule.error()
                );
                error.send(res, send);
                return;
            }
            scheduled_update = std::move(*parsed_schedule);
        }

        using Input = ::app::dto::revenue_recovery_data_backfill::UpdateTokenStatusRequest;
        auto parsed = serde::Ser::from_value<Input>(*generic);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }

        UpdateRedisDataInput input{*parsed, std::move(scheduled_update)};
        auto result = m_runtime.update_redis_data(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    /**
     * Upstream: routes/revenue_recovery_data_backfill.rs:130-160 ->
     * core/revenue_recovery_data_backfill.rs:66-117 ->
     * RedisTokenManager::unlock_connector_customer_status
     * (types/storage/revenue_recovery_redis_operation.rs:232-286).
     */
    void handle_revenue_recovery_data_backfill_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_request_support->authorize(
                req,
                "revenue_recovery_data_backfill_status",
                spec_for("revenue_recovery_data_backfill_status")
            )) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }

        const auto request = m_input_support.status_request_from_path(req);
        if (!request) {
            const auto& error = request.error();
            error.send(res, send);
            return;
        }

        UnlockStatusInput input{*request};
        auto result = m_runtime.unlock_status(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serialize_wire(*result));
        send();
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/recovery/data-backfill"}
            .add_route(
                Route{""}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_revenue_recovery_data_backfill(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch revenue_recovery_data_backfill")
            )
            .add_router(
                Router{router, "status"}.add_router(
                    Router{router, ":connector_cutomer_id"}.add_route(
                        Route{":payment_intent_id"}
                            .post(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_revenue_recovery_data_backfill_status(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch revenue_recovery_data_backfill_status")
                    )
                )
            )
            .add_route(
                Route{"update-token"}
                    .put(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_update_revenue_recovery_additional_redis_data(
                                req,
                                res,
                                std::move(send)
                            );
                        }
                    )
                    .summary("Hyperswitch update_revenue_recovery_additional_redis_data")
            )
            .add_route(
                Route{"retry-stats"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_revenue_recovery_retry_stats_migration(
                                req,
                                res,
                                std::move(send)
                            );
                        }
                    )
                    .summary("Hyperswitch revenue_recovery_retry_stats_migration")
            );
    }

    RevenueRecoveryDataBackfillHandlersV2(
        ::app::context::AppContext& ctx,
        RevenueRecoveryDataBackfillRuntime& runtime,
        RevenueRecoveryDataBackfillInputSupport& input_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

private:
    ::app::context::AppContext& m_context;
    RevenueRecoveryDataBackfillRuntime& m_runtime;
    RevenueRecoveryDataBackfillInputSupport& m_input_support;
};

} // namespace app::handlers::revenue_recovery_data_backfill
