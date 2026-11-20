module;
export module app_handlers:mandates;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import core_cache;
import core_error;
import app_http;
import app_metrics;
import app_store;
import app_connector;
import app_keystore;

export namespace app::handlers::mandates {

/**
 * @brief v1 mandate route handlers, ported from the upstream router core.
 *
 *  Upstream references (SHA 7736d55adaf235f7af74e133ae408573b1826823):
 *   - routes/mandates.rs:14-40,42-68,69-101  (extractors, server_wrap, auth, LockAction)
 *   - core/mandate.rs:33-58 (get_mandate), 60-165 (revoke_mandate), 403-426 (list)
 *   - core/mandate/helpers.rs:13-44 (get_profile_id_for_mandate), 46-85 (get_mandate_type)
 *   - core/mandate/utils.rs:13-104 (construct_mandate_revoke_router_data)
 *   - types/api/mandates.rs:36-135 (from_db_mandate wire mapping)
 *   - api_models/mandates.rs:10-13,15-33,35-86,139-179 (DTOs)
 *
 *  Every upstream core call is mirrored here as a local helper. External infrastructure
 *  (processor key store, locker, MCA/connector-data chain, DB storage_scheme) is replaced
 *  by the in-process AppContext facilities with the same call order and observable shapes.
 */
class MandatesRouteHandlers
{
public:
    /** @brief Upstream get_mandate (core/mandate.rs:33-58). */
    void handle_get_mandate(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto denied = authorize(auth, "get_mandate")) {
            send_error(res, send, *denied);
            return;
        }

        const auto mandate_id = mandate_id_from_path(req, 1);
        if (!mandate_id.has_value() || mandate_id->empty()) {
            send_error(res, send, error(
                ::core::error::AppErrorKind::INVALID_REQUEST, "missing mandate id in path"));
            return;
        }

        // platform.get_processor().get_account().get_id() is localised to the
        // authenticated merchant id (core/mandate.rs:43).
        const auto merchant = merchant_scope(req, auth);
        record_flow("MandatesRetrieve"); // LockAction::NotApplicable (routes/mandates.rs:37)

        const auto record = find_mandate_for_merchant(merchant, *mandate_id);
        if (!record) {
            send_error(res, send, error(
                ::core::error::AppErrorKind::NOT_FOUND,
                "Mandate does not exist in our records")); // MandateNotFound
            return;
        }

        // MandateResponse::from_db_mandate (types/api/mandates.rs:39-135).
        send_json(res, send, from_db_mandate(*record), true);
    }

    /** @brief Upstream revoke_mandate (core/mandate.rs:60-165). */
    void handle_revoke_mandate(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto denied = authorize(auth, "revoke_mandate")) {
            send_error(res, send, *denied);
            return;
        }

        const auto mandate_id = mandate_id_from_path(req, 2);
        if (!mandate_id.has_value() || mandate_id->empty()) {
            send_error(res, send, error(
                ::core::error::AppErrorKind::INVALID_REQUEST, "missing mandate id in path"));
            return;
        }

        const auto merchant = merchant_scope(req, auth);
        record_flow("MandatesRevoke"); // LockAction::NotApplicable (routes/mandates.rs:65)

        const auto found = find_mandate_for_merchant(merchant, *mandate_id);
        if (!found) {
            send_error(res, send, error(
                ::core::error::AppErrorKind::NOT_FOUND,
                "Mandate does not exist in our records")); // MandateNotFound
            return;
        }
        auto record = *found;

        // common_enums::MandateStatus has exactly four variants (enums.rs:3633-3639).
        // Active | Inactive | Pending are revocable, Revoked is a validation failure
        // (core/mandate.rs:76-79,158-163).
        const auto revocable = classify_revoke_status(record.get_status());
        if (revocable != RevokeStatus::Revocable) {
            send_error(res, send, error(
                ::core::error::AppErrorKind::INVALID_REQUEST, // MandateValidationFailed -> 400
                revocable == RevokeStatus::AlreadyRevoked
                    ? "Mandate Validation Failed: Mandate has already been revoked"
                    : "Mandate Validation Failed"));
            return;
        }

        // helpers::get_profile_id_for_mandate (core/mandate/helpers.rs:13-44).
        const auto profile = get_profile_id_for_mandate(record);
        if (!profile) {
            send_error(res, send, profile.error());
            return;
        }

        // payment_helper::get_merchant_connector_account (core/payments/helpers.rs:5222).
        const auto mca = get_merchant_connector_account(*profile, record.get_connector(), merchant);
        if (!mca) {
            send_error(res, send, error(
                ::core::error::AppErrorKind::INTERNAL,
                "merchant connector account not found for mandate revoke"));
            return;
        }

        // ConnectorData::get_connector_by_name + get_connector_integration +
        // utils::construct_mandate_revoke_router_data (core/mandate/utils.rs:19-104).
        auto router_data = construct_mandate_revoke_router_data(
            record, merchant, *profile, mca->get_connector_account_details());

        auto connector = m_context->connector_router().select_for(router_data);
        if (!connector) {
            connector = m_context->connector_router().select(merchant);
        }
        if (!connector) {
            m_context->metrics().record_flow("mandate.revoke.connector_unavailable");
            send_error(res, send, m_context->connector_router().last_error().value_or(
                error(::core::error::AppErrorKind::CONNECTOR_ERROR,
                      "no connector available for mandate revoke")));
            return;
        }

        // execute_connector_processing_step(CallConnectorAction::Trigger)
        // (core/mandate.rs:113-122).
        const auto connector_response = connector->mandate_revoke(router_data);
        if (connector_response.get_status().empty() || connector_response.get_retryable()) {
            // Err(err) -> ExternalConnectorError (core/mandate.rs:148-155). The local error
            // transport carries code/message/connector/status_code/reason in its message.
            const std::string code{"CE"};
            const std::string message = connector_response.get_message().empty()
                ? "connector rejected mandate revoke"
                : connector_response.get_message();
            send_error(res, send, error(
                ::core::error::AppErrorKind::CONNECTOR_ERROR,
                std::format("{}: {} (connector={}, status_code={}, reason={})",
                            code, message, record.get_connector(), 502,
                            connector_response.get_raw_json())));
            return;
        }
        record_flow("mandate.revoke.connector_succeeded");

        // Ok(_) -> MandateUpdate::StatusUpdate { Revoked } (core/mandate.rs:125-145).
        record.set_status("revoked");
        m_context->repository().update_mandate(record);

        send_json(res, send, revoked_response_json(record.get_mandate_id(), record.get_status()), true);
    }

    /** @brief Upstream retrieve_mandates_list (core/mandate.rs:403-426). */
    void handle_retrieve_mandates_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto denied = authorize(auth, "retrieve_mandates_list")) {
            send_error(res, send, *denied);
            return;
        }

        // web::Query<MandateListConstraints> (routes/mandates.rs:74; api_models/mandates.rs:139-179).
        const auto query = ::app::http::RequestQuery{}(req);
        using Constraints = ::app::model::mandates::MandateListConstraints;
        Constraints constraints{};
        if (const auto limit = parse_positive(query.get("limit"))) {
            constraints.set_limit(static_cast<std::int64_t>(*limit));
        }
        if (const auto offset = parse_positive(query.get("offset"))) {
            constraints.set_offset(static_cast<std::int64_t>(*offset));
        }
        if (const auto status = query.get("mandate_status")) {
            ::app::model::mandates::MandateStatusValue value{};
            value.set_value(*status);
            constraints.set_mandate_status(std::move(value));
        }
        constraints.set_connector(query.get("connector"));
        if (const auto v = query.get("created_time")) {
            ::app::model::mandates::PrimitiveDateTimeValue t{}; t.set_value(*v);
            constraints.set_created_time(std::move(t));
        }
        if (const auto v = query.get("created_time.lt")) {
            ::app::model::mandates::PrimitiveDateTimeValue t{}; t.set_value(*v);
            constraints.set_created_time_lt(std::move(t));
        }
        if (const auto v = query.get("created_time.gt")) {
            ::app::model::mandates::PrimitiveDateTimeValue t{}; t.set_value(*v);
            constraints.set_created_time_gt(std::move(t));
        }
        if (const auto v = query.get("created_time.lte")) {
            ::app::model::mandates::PrimitiveDateTimeValue t{}; t.set_value(*v);
            constraints.set_created_time_lte(std::move(t));
        }
        if (const auto v = query.get("created_time.gte")) {
            ::app::model::mandates::PrimitiveDateTimeValue t{}; t.set_value(*v);
            constraints.set_created_time_gte(std::move(t));
        }

        auto merchant_filter = query.get("merchant_id");
        const auto merchant = merchant_scope(req, auth);
        if (!merchant_filter.has_value() && !merchant.empty()) {
            merchant_filter = merchant;
        }

        record_flow("MandatesList"); // LockAction::NotApplicable (routes/mandates.rs:98)

        // find_mandates_by_merchant_id(merchant_id, constraints) (core/mandate.rs:409-415).
        // MandateQuery applies merchant/status/limit/offset at the database boundary;
        // connector + created_time are applied below (the local query object has no column
        // for them).
        auto db_query = ::app::store::MandateQuery{};
        if (merchant_filter.has_value() && !merchant_filter->empty()) {
            db_query.merchant_id(*merchant_filter);
        }
        if (constraints.get_mandate_status()) {
            db_query.status(constraints.get_mandate_status()->get_value());
        }
        if (const auto limit = constraints.get_limit(); limit && *limit > 0) {
            db_query.limit(static_cast<std::size_t>(*limit));
        }
        if (const auto offset = constraints.get_offset(); offset && *offset > 0) {
            db_query.offset(static_cast<std::size_t>(*offset));
        }
        const auto records = m_context->repository().find_mandates_by_customer_id(db_query);

        // ApplicationResponse::Json(mandates_list) -> bare array (core/mandate.rs:425).
        // from_db_mandate is applied to every row (core/mandate.rs:416-424).
        std::string data{"["};
        bool first{true};
        const auto& connector_filter = constraints.get_connector();
        for (const auto& record : records) {
            if (connector_filter && !connector_filter->empty()
                && record.get_connector() != *connector_filter) {
                continue;
            }
            if (!matches_created_time(record.get_created_at(), constraints)) {
                continue;
            }
            if (!first) {
                data.push_back(',');
            }
            data += from_db_mandate(record);
            first = false;
        }
        data.push_back(']');
        send_json(res, send, data, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/mandates"}
            .add_route(
                Route{"/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_mandates_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_mandates_list")
            )
            .add_route(
                Route{"/:id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_mandate(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_mandate")
            )
            .add_route(
                Route{"/revoke/:id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_revoke_mandate(req, res, std::move(send));
                    })
                    .summary("Hyperswitch revoke_mandate")
            );
    }

    explicit MandatesRouteHandlers(::app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    using Record = ::app::store::mandates::MandateRecord;

    enum class RevokeStatus { Revocable, AlreadyRevoked, NotRevocable };
    enum class MandateAuthMode { ApiKey, ApiKeyOrJwtMandateRead };

    // ── routes/mandates.rs auth extractors ───────────────────────────────────────

    [[nodiscard]] MandateAuthMode auth_mode_for(std::string_view operation) const noexcept
    {
        // HeaderAuth(ApiKeyAuth{allow_connected_scope_operation:false,
        // allow_platform_self_operation:false}) for get/revoke (mandates.rs:33-36,61-64).
        // auth_type(HeaderAuth(...), JWTAuth{MerchantMandateRead, allow_connected:false,
        // allow_platform:false}) for list (mandates.rs:86-97).
        return operation == "retrieve_mandates_list"
            ? MandateAuthMode::ApiKeyOrJwtMandateRead
            : MandateAuthMode::ApiKey;
    }

    [[nodiscard]] std::optional<::core::error::AppError> authorize(
        const ::app::auth::AuthContext& auth,
        std::string_view operation
    ) const
    {
        const auto api_key = auth.claim(::app::auth::kClaimApiKey);
        const bool has_api_key = api_key.has_value() && !api_key->empty();
        const bool has_token = !auth.token().is_empty();
        const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId);
        const bool has_connected = connected.has_value() && !connected->empty();
        const bool has_platform = !auth.org_id().empty();

        if (auth_mode_for(operation) == MandateAuthMode::ApiKey) {
            // HeaderAuth(ApiKeyAuth): the local API-key claim or bearer token is the
            // credential. Both scope-allowing flags are false upstream.
            if (has_connected) {
                return error(::core::error::AppErrorKind::UNAUTHORIZED,
                             "connected scope operation is not allowed for mandates");
            }
            if (has_platform) {
                return error(::core::error::AppErrorKind::UNAUTHORIZED,
                             "platform self operation is not allowed for mandates");
            }
            if (!has_api_key && !has_token) {
                return error(::core::error::AppErrorKind::UNAUTHORIZED, "missing API key");
            }
            return std::nullopt;
        }

        // List: API key path first (authenticated, no connected/platform scope).
        if (has_api_key && !has_connected && !has_platform) {
            return std::nullopt;
        }
        if (has_connected || has_platform) {
            return error(::core::error::AppErrorKind::UNAUTHORIZED,
                         "connected/platform scope is not allowed for mandate list");
        }
        if (!has_token) {
            return error(::core::error::AppErrorKind::UNAUTHORIZED, "missing bearer token");
        }
        return check_mandate_permission(auth, "MerchantMandateRead");
    }

    /** @brief Mirrors services/authorization.rs check_permission for the JWT branch. */
    [[nodiscard]] std::optional<::core::error::AppError> check_mandate_permission(
        const ::app::auth::AuthContext& auth,
        std::string_view required
    ) const
    {
        const auto single = auth.claim("permission");
        const auto list = auth.claim("permissions");
        if (!single.has_value() && !list.has_value()) {
            return std::nullopt;
        }
        const auto grants = [required](std::string_view granted)
        {
            std::string_view rest{granted};
            while (!rest.empty()) {
                const std::size_t comma{rest.find(',')};
                const std::string_view token{rest.substr(0, comma)};
                if (token == required || token == "ADMIN") {
                    return true;
                }
                if (comma == std::string_view::npos) {
                    break;
                }
                rest = rest.substr(comma + 1);
            }
            return false;
        };
        if ((single.has_value() && grants(*single)) || (list.has_value() && grants(*list))) {
            return std::nullopt;
        }
        return error(::core::error::AppErrorKind::FORBIDDEN,
                     std::format("Access forbidden: {}", required));
    }

    [[nodiscard]] std::string merchant_scope(
        interfaces::io::IRequest& req,
        const ::app::auth::AuthContext& auth
    ) const
    {
        const auto header_id = ::app::http::merchant_id_from_header{}(req);
        return header_id.empty() ? auth.merchant_id() : header_id;
    }

    // ── core/mandate.rs lookups ──────────────────────────────────────────────────

    /**
     * @brief find_mandate_by_merchant_id_mandate_id (db/mandate.rs:12-17).
     * The merchant id is applied as a DB predicate so a foreign mandate is invisible
     * (upstream returns MandateNotFound instead of a 403 ownership error).
     */
    [[nodiscard]] std::optional<Record> find_mandate_for_merchant(
        std::string_view merchant_id,
        std::string_view mandate_id
    ) const
    {
        auto query = ::app::store::MandateQuery{};
        if (!merchant_id.empty()) {
            query.merchant_id(std::string{merchant_id});
        }
        for (const auto& row : m_context->repository().find_mandates_by_customer_id(query)) {
            if (row.get_mandate_id() == mandate_id) {
                return row;
            }
        }
        return std::nullopt;
    }

    // ── core/mandate/helpers.rs ──────────────────────────────────────────────────

    /**
     * @brief get_profile_id_for_mandate (core/mandate/helpers.rs:13-44).
     * Upstream loads the original payment intent and reads its profile id. The local
     * payment record has no profile column, so the profile persisted on the mandate is
     * used after verifying the original payment exists.
     */
    [[nodiscard]] std::expected<std::string, ::core::error::AppError>
    get_profile_id_for_mandate(const Record& record) const
    {
        if (record.get_payment_id().empty()) {
            return std::unexpected(error(
                ::core::error::AppErrorKind::NOT_FOUND, "Payment not found"));
        }
        if (!m_context->repository().find_payment_by_string(record.get_payment_id())) {
            return std::unexpected(error(
                ::core::error::AppErrorKind::NOT_FOUND, "Payment not found"));
        }
        if (record.get_profile_id().empty()) {
            return std::unexpected(error(
                ::core::error::AppErrorKind::NOT_FOUND, "Profile id is Null"));
        }
        return record.get_profile_id();
    }

    /** @brief get_mandate_type (core/mandate/helpers.rs:46-85), retained as a local validator. */
    [[maybe_unused]] [[nodiscard]] std::expected<
        std::optional<std::string_view>, ::core::error::AppError>
    get_mandate_type(
        bool has_mandate_data,
        bool has_off_session,
        bool has_off_session_future_usage,
        bool has_customer_acceptance,
        bool has_token,
        bool has_wallet_payment_method
    ) const
    {
        if (has_mandate_data && has_off_session && has_off_session_future_usage &&
            has_customer_acceptance && has_token) {
            return std::unexpected(error(
                ::core::error::AppErrorKind::VALIDATION_FAILED,
                "Expected one out of recurring_details and mandate_data but got both"));
        }
        if ((has_off_session_future_usage && has_customer_acceptance && has_token) ||
            (has_off_session_future_usage && has_customer_acceptance) ||
            (has_mandate_data && has_off_session_future_usage)) {
            return std::optional<std::string_view>{"NewMandateTransaction"};
        }
        if ((has_off_session_future_usage && has_token) || has_off_session ||
            (has_off_session_future_usage && has_wallet_payment_method)) {
            return std::optional<std::string_view>{"RecurringMandateTransaction"};
        }
        return std::optional<std::string_view>{};
    }

    // ── core/mandate/utils.rs + payments/helpers.rs MCA resolution ───────────────

    [[nodiscard]] std::optional<::app::store::merchant_connectors::MerchantConnectorRecord>
    get_merchant_connector_account(
        std::string_view profile_id,
        std::string_view connector,
        std::string_view merchant_id
    ) const
    {
        for (const auto& mca : m_context->repository().list_merchant_connectors()) {
            if (!merchant_id.empty() && mca.get_merchant_id() != merchant_id) {
                continue;
            }
            if (!profile_id.empty() && mca.get_profile_id() != profile_id) {
                continue;
            }
            if (!connector.empty() && mca.get_connector_name() != connector) {
                continue;
            }
            return mca;
        }
        return std::nullopt;
    }

    /** @brief construct_mandate_revoke_router_data (core/mandate/utils.rs:19-104). */
    [[nodiscard]] ::app::connector::ConnectorRequest construct_mandate_revoke_router_data(
        const Record& record,
        std::string_view merchant_id,
        std::string_view profile_id,
        std::string_view connector_auth_details
    ) const
    {
        constexpr std::string_view kAttemptId{"irrelevant_attempt_id_in_mandate_revoke_flow"};
        constexpr std::string_view kReferenceId{
            "irrelevant_connector_request_reference_id_in_mandate_revoke_flow"};
        const std::string payment_id = record.get_payment_id().empty()
            ? "mandate_revoke"
            : record.get_payment_id();
        const std::string auth_json = connector_auth_details.empty()
            ? "null" : std::string{connector_auth_details};

        return ::app::connector::ConnectorRequestBuilder{}
            .operation("mandate_revoke")
            .merchant_id(std::string{merchant_id})
            .resource_id(record.get_mandate_id())
            .raw_payload(std::format(
                "{{\"mandate_id\":{},\"connector_mandate_id\":null,\"connector\":{},"
                "\"merchant_id\":{},\"customer_id\":{},\"payment_id\":{},"
                "\"attempt_id\":{},\"connector_request_reference_id\":{},"
                "\"connector_auth_type\":{},\"profile_id\":{}}}",
                json_quote(record.get_mandate_id()),
                json_quote(record.get_connector()),
                json_quote(merchant_id),
                json_quote(record.get_customer_id()),
                json_quote(payment_id),
                json_quote(kAttemptId),
                json_quote(kReferenceId),
                auth_json,
                json_quote(profile_id)))();
    }

    // ── core/mandate.rs response mapping ────────────────────────────────────────

    /**
     * @brief MandateResponse::from_db_mandate (types/api/mandates.rs:36-135).
     *
     *  v1 mapping sets amount/currency/profile_id/mandate_metadata to None (skipped),
     *  always emits customer_acceptance, and emits card only for the Card payment method.
     *  Card/acceptance details are read from the in-process KeyStore stand-in for the
     *  processor key store / locker using (purpose, mandate_id) as the natural key.
     */
    [[nodiscard]] std::string from_db_mandate(const Record& record) const
    {
        std::string out{"{"};
        out += "\"mandate_id\":" + json_quote(record.get_mandate_id());
        // original_payment_id, Option -> skipped when absent.
        if (!record.get_payment_id().empty()) {
            out += ",\"payment_id\":" + json_quote(record.get_payment_id());
        }
        // amount / currency / profile_id / mandate_metadata: always None in v1.
        out += ",\"status\":" + json_quote(record.get_status());
        out += ",\"payment_method_id\":" + json_quote(record.get_payment_method_id());
        out += ",\"payment_method\":" + json_quote(record.get_payment_method());
        if (!record.get_payment_method_type().empty()) {
            out += ",\"payment_method_type\":" + json_quote(record.get_payment_method_type());
        }

        out += ",\"card\":";
        if (is_card(record.get_payment_method())) {
            if (const auto secret = m_context->key_store().get("mandate_card", record.get_mandate_id());
                secret.has_value() && !secret->value().empty() && secret->value().front() == '{') {
                out += secret->value();
            } else {
                out += "null";
            }
        } else {
            out += "null";
        }

        out += ",\"customer_acceptance\":";
        if (const auto secret = m_context->key_store().get("customer_acceptance", record.get_mandate_id());
            secret.has_value() && !secret->value().empty() && secret->value().front() == '{') {
            out += secret->value();
        } else {
            // types/api/mandates.rs:117-128: acceptance_type from ip address (unavailable
            // locally -> offline), accepted_at from mandate (unavailable -> null), online
            // object always present with the extended user agent.
            out += "{\"acceptance_type\":\"offline\",\"accepted_at\":null,"
                   "\"online\":{\"ip_address\":null,\"user_agent\":\"\"}}";
        }
        out.push_back('}');
        return out;
    }

    [[nodiscard]] std::string revoked_response_json(
        std::string_view mandate_id,
        std::string_view status
    ) const
    {
        return std::format(
            "{{\"mandate_id\":{},\"status\":{},\"error_code\":null,\"error_message\":null}}",
            json_quote(mandate_id), json_quote(status));
    }

    // ── small primitives ────────────────────────────────────────────────────────

    [[nodiscard]] RevokeStatus classify_revoke_status(std::string_view status) const noexcept
    {
        if (status == "active" || status == "inactive" || status == "pending") {
            return RevokeStatus::Revocable;
        }
        if (status == "revoked") {
            return RevokeStatus::AlreadyRevoked;
        }
        return RevokeStatus::NotRevocable;
    }

    [[nodiscard]] bool is_card(std::string_view payment_method) const noexcept
    {
        return payment_method.size() == 4 &&
            std::equal(payment_method.begin(), payment_method.end(), "card",
                       [](char a, char b) {
                           return std::tolower(static_cast<unsigned char>(a)) == b;
                       });
    }

    [[nodiscard]] static std::string json_quote(std::string_view value)
    {
        std::string out{"\""};
        for (const char c : value) {
            switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    out += std::format("\\u{:04x}", static_cast<unsigned>(static_cast<unsigned char>(c)));
                } else {
                    out.push_back(c);
                }
            }
        }
        out.push_back('"');
        return out;
    }

    [[nodiscard]] std::optional<std::string> mandate_id_from_path(
        interfaces::io::IRequest& req,
        std::size_t expected_index
    ) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        if (const auto segment = segments.at(expected_index)) {
            if (!segment->empty()) {
                return std::string{*segment};
            }
        }
        for (std::size_t i{0}; i < segments.count(); ++i) {
            if (segments.at(i).value_or(std::string_view{}) != "mandates") {
                continue;
            }
            for (std::size_t j{i + 1}; j < segments.count(); ++j) {
                const auto candidate = segments.at(j).value_or(std::string_view{});
                if (!candidate.empty() && candidate != "revoke" && candidate != "list") {
                    return std::string{candidate};
                }
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] bool matches_created_time(
        std::string_view actual,
        const ::app::model::mandates::MandateListConstraints& constraints
    ) const
    {
        const auto& exact = constraints.get_created_time();
        const auto& lt = constraints.get_created_time_lt();
        const auto& gt = constraints.get_created_time_gt();
        const auto& lte = constraints.get_created_time_lte();
        const auto& gte = constraints.get_created_time_gte();
        if (!exact && !lt && !gt && !lte && !gte) {
            return true;
        }
        if (actual.empty()) {
            return false;
        }
        if (exact && actual != exact->get_value()) { return false; }
        if (lt && actual >= lt->get_value()) { return false; }
        if (gt && actual <= gt->get_value()) { return false; }
        if (lte && actual > lte->get_value()) { return false; }
        if (gte && actual < gte->get_value()) { return false; }
        return true;
    }

    [[nodiscard]] std::optional<std::size_t> parse_positive(const std::optional<std::string>& raw) const
    {
        if (!raw.has_value() || raw->empty()) {
            return std::nullopt;
        }
        std::size_t value{};
        const auto [end, ec] = std::from_chars(raw->data(), raw->data() + raw->size(), value);
        if (ec != std::errc{} || end != raw->data() + raw->size() || value == 0) {
            return std::nullopt;
        }
        return value;
    }

    void record_flow(std::string_view flow) const
    {
        // router_env::instrument(Flow::Mandates*) + metrics.
        m_context->metrics().record_flow(flow);
        m_context->metrics().counter(flow).increment();
    }

    [[nodiscard]] ::core::error::AppError error(
        ::core::error::AppErrorKind kind,
        std::string message
    ) const
    {
        return ::core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        ::core::error::AppError error_value
    ) const
    {
        auto text = serde::Ser::encode_json(error_value);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(::core::error::AppErrorStatus{}(error_value.get_kind()));
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

} // namespace app::handlers::mandates
