module;
export module app_handlers:revenue_recovery_data_backfill;

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

export namespace app::handlers::revenue_recovery_data_backfill {

class RevenueRecoveryDataBackfillRouteHandlers
{
public:
    explicit RevenueRecoveryDataBackfillRouteHandlers(::app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    void handle_revenue_recovery_data_backfill(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream guards every route with V2AdminApiAuth (platform admin + ADMIN).
        if (const auto auth_error = authorize(req, "revenue_recovery_data_backfill")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = ::app::dto::revenue_recovery_data_backfill::CsvParsingResult;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream reads BackfillQuery.cutoff_time from the query string and parses it as
        // ISO8601 before any record is processed; a bad value returns BadRequest.
        const auto query = ::app::http::RequestQuery{}(req);
        const auto cutoff_time = query.get("cutoff_time");
        if (cutoff_time && !cutoff_time->empty() && !is_iso8601_datetime(*cutoff_time)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Invalid datetime format: " + *cutoff_time + ". Use ISO8601: 2024-01-15T10:30:00"
            ));
            return;
        }

        std::size_t processed_records = 0;
        std::size_t failed_records = 0;

        for (const auto& record : parsed->get_records()) {
            // Upstream build_comprehensive_card_data falls back to a card-only payload on
            // any build error (e.g. a malformed expiration date) and STILL processes the
            // record. Build failure must not increment `failed_records`.
            const auto card_data = build_card_data(record);
            const auto card_json = serde::Ser::encode_json(card_data);

            const auto& token = record.get_token();
            if (token && !token->empty() && *token != "nan") {
                // Revenue recovery tokens are business records: persist them through the
                // repository, with the normalized cutoff applied as the record expiry.
                persist_token(record, card_json, cutoff_time);
            }

            // Upstream sets the connector-customer status lock (owner = payment id) as part
            // of the token update. LockRegistry backs this with a TTL, giving atomic
            // ownership semantics instead of a non-atomic cache compare-then-erase.
            if (const auto& payment_id = record.get_connector_payment_id();
                payment_id && !payment_id->empty()) {
                (void)m_context->locks().acquire(
                    status_lock_key(record.get_customer_id_resp()), lock_ttl_seconds(), *payment_id);
            }

            ++processed_records;
        }

        failed_records += parsed->get_failed_records().size();

        // The cut-off is applied to every persisted token (see persist_token). Keep a
        // short-lived run marker as ephemeral state for traceability/dedupe.
        if (cutoff_time && !cutoff_time->empty()) {
            m_context->cache().put(
                "revenue_recovery.backfill.cutoff_time",
                core::cache::to_cache_value(normalize_cutoff(*cutoff_time)),
                core::cache::Ttl{}.seconds(lock_ttl_seconds())
            );
        }

        m_context->metrics().record_flow("RecoveryDataBackfill");
        m_context->metrics().counter("revenue_recovery_data_backfill.requests").increment();
        m_context->metrics().counter("revenue_recovery_data_backfill.processed_records").increment(processed_records);
        m_context->metrics().counter("revenue_recovery_data_backfill.failed_records").increment(failed_records);

        ::app::dto::revenue_recovery_data_backfill::RevenueRecoveryDataBackfillResponse response;
        response.set_processed_records(processed_records);
        response.set_failed_records(failed_records);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_revenue_recovery_retry_stats_migration(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream guards every route with V2AdminApiAuth (platform admin + ADMIN).
        if (const auto auth_error = authorize(req, "revenue_recovery_retry_stats_migration")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = ::app::dto::revenue_recovery_data_backfill::RetryStatsMigrationCsvResult;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        using Error = ::app::dto::revenue_recovery_data_backfill::CsvParsingError;
        const auto row_error = [](std::size_t row, std::string message) {
            Error error{};
            error.set_row_number(row);
            error.set_error(std::move(message));
            return error;
        };

        // Upstream migrate_retry_stats_from_csv validates the WHOLE batch before any write:
        // parse each stats document, reject duplicate cluster keys, and persist nothing if
        // any row is invalid.
        std::vector<Error> row_errors{};
        std::vector<std::pair<std::string, std::string>> valid_rows{};
        std::unordered_set<std::string> seen_keys{};

        for (const auto& entry : parsed->get_records()) {
            const auto& record = entry.second;
            if (record.get_cluster_key().empty()) {
                row_errors.push_back(row_error(entry.first, "invalid cluster_key"));
                continue;
            }
            if (!serde::Ser::decode_generic("application/json", record.get_stats())) {
                row_errors.push_back(row_error(entry.first, "stats is not a valid stats document"));
                continue;
            }
            if (!seen_keys.insert(record.get_cluster_key()).second) {
                row_errors.push_back(row_error(entry.first, "duplicate cluster_key present"));
                continue;
            }
            valid_rows.emplace_back(record.get_cluster_key(), record.get_stats());
        }

        std::size_t processed_records = 0;
        if (row_errors.empty()) {
            // Phase 2: whole batch is valid. Whole-document replace per cluster key under a
            // per-key lock; contention is reported per row. ConfigStore is the service-level
            // stand-in for the upstream revenue_recovery_retry_stats table.
            for (const auto& [cluster_key, stats] : valid_rows) {
                const auto lock_key = "revenue_recovery.retry_stats:" + cluster_key;
                const auto lock = m_context->locks().acquire(lock_key, lock_ttl_seconds(), "migration");
                if (!lock) {
                    row_errors.push_back(row_error(
                        std::size_t{0},
                        "per-key redis lock stayed contended; row skipped, retry the file"));
                    continue;
                }
                m_context->config_store().put("revenue_recovery_retry_stats", cluster_key, stats);
                (void)m_context->locks().release(lock_key);
            }
            processed_records = valid_rows.size() - row_errors.size();
        }

        m_context->metrics().record_flow("RecoveryRetryStatsMigration");
        m_context->metrics().counter("revenue_recovery_retry_stats_migration.requests").increment();
        m_context->metrics().counter("revenue_recovery_retry_stats_migration.processed_records").increment(processed_records);

        ::app::dto::revenue_recovery_data_backfill::RetryStatsMigrationResponse response;
        response.set_processed_records(processed_records);
        response.set_failed_records(row_errors.size());
        response.set_row_errors(std::move(row_errors));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_update_revenue_recovery_additional_redis_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream guards every route with V2AdminApiAuth (platform admin + ADMIN).
        if (const auto auth_error = authorize(req, "update_revenue_recovery_additional_redis_data")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = ::app::dto::revenue_recovery_data_backfill::UpdateTokenStatusRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream Secret<String> debug output is masked; mirror that so the raw token
        // never appears in the response message or in any cache key material.
        const auto secret = ::app::crypto::SecretString{parsed->get_payment_processor_token()};

        auto repository = m_context->repository();
        auto existing = repository.find_revenue_recovery_token_by_string(parsed->get_payment_processor_token());
        if (!existing || existing->get_merchant_id() != parsed->get_connector_customer_id()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND,
                "Token '" + secret.masked()
                    + "' not found for connector customer id:- "
                    + parsed->get_connector_customer_id()
            ));
            return;
        }

        // Apply the requested field updates to the persisted token payload.
        std::string payload = existing->get_payload().empty() ? std::string{"{}"} : existing->get_payload();
        if (auto doc = serde::Ser::decode_generic("application/json", payload)) {
            if (auto* object = std::get_if<serde::Value::Object>(&doc->get())) {
                if (const auto& scheduled_at = parsed->get_scheduled_at()) {
                    if (*scheduled_at == ::app::dto::revenue_recovery_data_backfill::ScheduledAtUpdate::SetToNull) {
                        (*object)["scheduled_at"] = serde::Value{};
                    } else {
                        // Local DTO enum has no datetime payload (shared file); record a
                        // deterministic "datetime" marker for the set-to-datetime case.
                        (*object)["scheduled_at"] = serde::Value{std::string{"datetime"}};
                    }
                }
                if (const auto& is_hard_decline = parsed->get_is_hard_decline()) {
                    (*object)["is_hard_decline"] = serde::Value{*is_hard_decline};
                }
                if (const auto& error_code = parsed->get_error_code()) {
                    (*object)["error_code"] = serde::Value{*error_code};
                }
                payload = serde::Ser::encode_json(*doc);
            }
        }
        existing->set_payload(payload);
        (void)repository.update_revenue_recovery_token(*existing);

        std::string updated_fields;
        if (const auto& scheduled_at = parsed->get_scheduled_at()) {
            updated_fields += (*scheduled_at == ::app::dto::revenue_recovery_data_backfill::ScheduledAtUpdate::SetToNull)
                ? "scheduled_at: set to null"
                : "scheduled_at: set to datetime";
        }
        if (const auto& is_hard_decline = parsed->get_is_hard_decline()) {
            if (!updated_fields.empty()) {
                updated_fields += ", ";
            }
            updated_fields += std::string{"is_hard_decline: "} + (*is_hard_decline ? "true" : "false");
        }
        if (const auto& error_code = parsed->get_error_code()) {
            if (!updated_fields.empty()) {
                updated_fields += ", ";
            }
            updated_fields += std::string{"error_code: "} + *error_code;
        }
        if (updated_fields.empty()) {
            updated_fields = "no fields were updated";
        }

        m_context->metrics().record_flow("RecoveryDataBackfill");
        m_context->metrics().counter("revenue_recovery_update_additional_redis_data.requests").increment();

        ::app::dto::revenue_recovery_data_backfill::UpdateTokenStatusResponse response;
        response.set_updated(true);
        response.set_message(
            std::string{"Successfully updated token '"} + secret.masked()
            + "' for connector customer '" + parsed->get_connector_customer_id()
            + "'. Updated fields: " + updated_fields
        );
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_revenue_recovery_data_backfill_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream guards every route with V2AdminApiAuth (platform admin + ADMIN).
        if (const auto auth_error = authorize(req, "revenue_recovery_data_backfill_status")) {
            send_error(res, send, *auth_error);
            return;
        }

        // Upstream builds UnlockStatusRequest straight from typed path segments; there is no
        // request body. Route: /status/:connector_cutomer_id/:payment_intent_id
        const auto segments = ::app::http::RequestPath{}(req);
        using Request = ::app::dto::revenue_recovery_data_backfill::UnlockStatusRequest;
        Request parsed;
        if (segments.count() < 2) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing connector customer id or payment intent id in path"
            ));
            return;
        }
        parsed.set_connector_customer_id(std::string{segments.at(segments.count() - 2).value_or("")});
        ::app::dto::revenue_recovery_data_backfill::GlobalPaymentIdValue payment_id;
        payment_id.set_value(std::string{segments.at(segments.count() - 1).value_or("")});
        parsed.set_payment_intent_id(std::move(payment_id));

        // Upstream unlock maps a missing lock to GenericNotFound and a wrong-owner DEL to
        // InvalidPaymentIdProvided. LockRegistry owns the key with TTL; read its scoped
        // owner, verify it, then release through the lock service (not a raw cache erase).
        const auto key = status_lock_key(parsed.get_connector_customer_id());
        if (!m_context->locks().is_held(key)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND,
                "Lock not found for connector customer id: " + parsed.get_connector_customer_id()
            ));
            return;
        }

        const auto owner = m_context->cache().get(lock_scoped_key(key))
                               .value_or<std::string>(std::string{});
        if (owner != parsed.get_payment_intent_id().get_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::FORBIDDEN,
                "The Status token for connector customer id:- " + parsed.get_connector_customer_id()
                    + " is locked by different PaymentIntent ID"
            ));
            return;
        }

        (void)m_context->locks().release(key);

        m_context->metrics().record_flow("RecoveryDataBackfillStatus");
        m_context->metrics().counter("revenue_recovery_data_backfill_status.requests").increment();

        ::app::dto::revenue_recovery_data_backfill::UnlockStatusResponse response;
        response.set_unlocked(true);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/v2/recovery/data-backfill"}
            .add_route(
                Route{""}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_revenue_recovery_data_backfill(req, res, std::move(send));
                    })
                    .summary("Hyperswitch revenue_recovery_data_backfill")
            )
            .add_route(
                Route{"/status/:connector_cutomer_id/:payment_intent_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_revenue_recovery_data_backfill_status(req, res, std::move(send));
                    })
                    .summary("Hyperswitch revenue_recovery_data_backfill_status")
            )
            .add_route(
                Route{"/update-token"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_update_revenue_recovery_additional_redis_data(req, res, std::move(send));
                    })
                    .summary("Hyperswitch update_revenue_recovery_additional_redis_data")
            )
            .add_route(
                Route{"/retry-stats"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_revenue_recovery_retry_stats_migration(req, res, std::move(send));
                    })
                    .summary("Hyperswitch revenue_recovery_retry_stats_migration")
            );
    }

private:
    [[nodiscard]] std::int64_t lock_ttl_seconds() const noexcept
    {
        return 3600;
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        return m_context->authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    [[nodiscard]] std::string status_lock_key(std::string_view customer_id) const
    {
        return "revenue_recovery.status:" + std::string{customer_id};
    }

    [[nodiscard]] std::string lock_scoped_key(std::string_view key) const
    {
        return "lock:" + std::string{key};
    }

    [[nodiscard]] std::string normalize_cutoff(std::string_view value) const
    {
        std::string out{value.substr(0, std::size_t{19})};
        if (out.size() == std::size_t{19} && (out[10] == 'T' || out[10] == ' ')) {
            out[10] = ' ';
        }
        return out;
    }

    [[nodiscard]] std::string epoch_now() const
    {
        const auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        return std::to_string(now);
    }

    // Mirrors time::format_description::well_known::Iso8601::DEFAULT used by the backfill
    // route for the cutoff_time query parameter: YYYY-MM-DDTHH:MM:SS.
    [[nodiscard]] bool is_iso8601_datetime(std::string_view value) const noexcept
    {
        if (value.size() < std::size_t{19}) {
            return false;
        }
        const auto digit = [](char ch) noexcept { return ch >= '0' && ch <= '9'; };
        return digit(value[0]) && digit(value[1]) && digit(value[2]) && digit(value[3])
            && value[4] == '-' && digit(value[5]) && digit(value[6])
            && value[7] == '-' && digit(value[8]) && digit(value[9])
            && (value[10] == 'T' || value[10] == ' ')
            && digit(value[11]) && digit(value[12]) && value[13] == ':'
            && digit(value[14]) && digit(value[15]) && value[16] == ':'
            && digit(value[17]) && digit(value[18]);
    }

    // Mirrors upstream parse_expiration_date (core:466-523): empty is a valid (None, None);
    // otherwise MM/YY with a trimmed month in 1..12 and a 2- or 4-digit year.
    [[nodiscard]] bool parse_expiration_date(
        const std::optional<std::string>& raw,
        std::string& month,
        std::string& year
    ) const
    {
        month.clear();
        year.clear();
        if (!raw || raw->empty()) {
            return true;
        }
        const auto slash = raw->find('/');
        if (slash == std::string::npos) {
            return false;
        }
        const auto trim = [](std::string_view part) {
            const auto first = part.find_first_not_of(" \t");
            if (first == std::string_view::npos) {
                return std::string{};
            }
            const auto last = part.find_last_not_of(" \t");
            return std::string{part.substr(first, last - first + 1)};
        };
        const std::string parsed_month = trim(std::string_view{*raw}.substr(0, slash));
        const std::string parsed_year = trim(std::string_view{*raw}.substr(slash + 1));
        if (parsed_month.empty() || parsed_year.empty()) {
            return false;
        }
        int month_number = 0;
        try {
            month_number = std::stoi(parsed_month);
        } catch (...) {
            return false;
        }
        if (month_number < 1 || month_number > 12) {
            return false;
        }
        if (parsed_year.size() == 4) {
            year = parsed_year.substr(2);
        } else if (parsed_year.size() == 2) {
            year = parsed_year;
        } else {
            return false;
        }
        month = parsed_month;
        return true;
    }

    // Upstream parse_daily_retry_history decodes the CSV JSON object of timestamp -> count
    // and parses each key as a PrimitiveDateTime (full datetime, then date-only fallback).
    // The local DTO keeps string keys, so keys are canonicalized to a fixed-width datetime
    // form; keys that do not parse are dropped, matching the upstream behaviour.
    [[nodiscard]] std::optional<std::string> normalize_retry_key(std::string_view key) const
    {
        const auto digits = [](std::string_view value, std::size_t at, std::size_t count) {
            if (at + count > value.size()) {
                return false;
            }
            for (std::size_t i = 0; i < count; ++i) {
                if (value[at + i] < '0' || value[at + i] > '9') {
                    return false;
                }
            }
            return true;
        };
        if (!digits(key, 0, 4) || key.size() < std::size_t{10} || key[4] != '-' || key[7] != '-'
            || !digits(key, 5, 2) || !digits(key, 8, 2)) {
            return std::nullopt;
        }
        if (key.size() == std::size_t{10}) {
            return std::string{key} + " 00:00:00.000000";
        }
        if (key.size() < std::size_t{19} || (key[10] != ' ' && key[10] != 'T')
            || key[13] != ':' || key[16] != ':'
            || !digits(key, 11, 2) || !digits(key, 14, 2) || !digits(key, 17, 2)) {
            return std::nullopt;
        }
        std::string out{key.substr(0, std::size_t{19})};
        out[10] = ' ';
        out.push_back('.');
        std::string fraction;
        if (key.size() > std::size_t{19} && key[19] == '.') {
            for (std::size_t i = 20; i < key.size() && fraction.size() < std::size_t{6}; ++i) {
                if (key[i] >= '0' && key[i] <= '9') {
                    fraction.push_back(key[i]);
                } else {
                    break;
                }
            }
        }
        fraction.resize(std::size_t{6}, '0');
        out += fraction;
        return out;
    }

    [[nodiscard]] std::optional<std::unordered_map<std::string, std::int32_t>>
    parse_daily_retry_history(const std::optional<std::string>& raw) const
    {
        if (!raw || raw->empty()) {
            return std::nullopt;
        }
        auto doc = serde::Ser::decode_generic("application/json", *raw);
        if (!doc) {
            return std::nullopt;
        }
        const auto* object = std::get_if<serde::Value::Object>(&doc->get());
        if (object == nullptr) {
            return std::nullopt;
        }
        std::unordered_map<std::string, std::int32_t> history{};
        for (const auto& [key, value] : *object) {
            auto normalized = normalize_retry_key(key);
            if (!normalized) {
                continue;
            }
            std::int32_t count = 0;
            if (const auto* number = std::get_if<std::int64_t>(&value.get())) {
                count = static_cast<std::int32_t>(*number);
            } else if (const auto* number = std::get_if<double>(&value.get())) {
                count = static_cast<std::int32_t>(*number);
            } else {
                continue;
            }
            history.insert_or_assign(std::move(*normalized), count);
        }
        return history;
    }

    using BackfillRequest = ::app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest;
    using CardData = ::app::dto::revenue_recovery_data_backfill::ComprehensiveCardData;

    [[nodiscard]] CardData build_card_data(const BackfillRequest& record) const
    {
        CardData card_data;

        // determine_card_type: present subtype maps to credit/debit/card; absent subtype
        // stays unset (upstream returns None). Only a build error supplies the "card" fallback.
        if (const auto& payment_method = record.get_payment_method_sub_type()) {
            const auto& value = payment_method->get_value();
            if (value.find("credit") != std::string::npos || value.find("Credit") != std::string::npos) {
                card_data.set_card_type(std::string{"credit"});
            } else if (value.find("debit") != std::string::npos || value.find("Debit") != std::string::npos) {
                card_data.set_card_type(std::string{"debit"});
            } else {
                card_data.set_card_type(std::string{"card"});
            }
        }

        std::string month;
        std::string year;
        if (!parse_expiration_date(record.get_exp_date(), month, year)) {
            CardData fallback;
            fallback.set_card_type(std::string{"card"});
            return fallback;
        }
        if (!month.empty() && !year.empty()) {
            card_data.set_card_exp_month(month);
            card_data.set_card_exp_year(year);
        }

        card_data.set_card_network(record.get_card_network());
        if (const auto& issuer = record.get_clean_bank_name(); issuer && !issuer->empty()) {
            card_data.set_card_issuer(*issuer);
        }
        if (const auto& country = record.get_country_name(); country && !country->empty()) {
            card_data.set_card_issuing_country(*country);
        }
        card_data.set_is_active(record.get_is_active());
        card_data.set_account_update_history(record.get_account_update_history());
        card_data.set_daily_retry_history(parse_daily_retry_history(record.get_daily_retry_history()));
        return card_data;
    }

    void persist_token(
        const BackfillRequest& record,
        const std::string& card_json,
        const std::optional<std::string>& cutoff_time
    ) const
    {
        using TokenRecord = ::app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord;
        auto repository = m_context->repository();

        TokenRecord token_record{};
        const auto& token = record.get_token();
        if (token && !token->empty()) {
            token_record = repository.find_revenue_recovery_token_by_string(*token).value_or(TokenRecord{});
        }
        token_record.set_token(token.value_or(""));
        // The connector-customer id is the Redis key scope upstream; store it as the
        // natural scoping field so update-token can verify ownership.
        token_record.set_merchant_id(record.get_customer_id_resp());
        token_record.set_payment_id(record.get_connector_payment_id().value_or(""));
        token_record.set_status("active");
        token_record.set_payload(card_json);
        token_record.set_expires_at(cutoff_time ? normalize_cutoff(*cutoff_time) : std::string{});
        if (token_record.get_created_at().empty()) {
            token_record.set_created_at(epoch_now());
        }
        if (token_record.get_id() == 0) {
            (void)repository.save_revenue_recovery_token(token_record);
        } else {
            (void)repository.update_revenue_recovery_token(token_record);
        }
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    ::app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::revenue_recovery_data_backfill
