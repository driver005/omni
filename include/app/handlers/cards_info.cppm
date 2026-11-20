module;
export module app_handlers:cards_info;

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

export namespace app::handlers::cards_info {

class CardsInfoRouteHandlers
{
public:
    void handle_card_iin_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream routes/cards_info.rs:42-47 runs check_sdk_auth_and_get_auth
        // (SDK authorization header or api_key/publishable-key) before server_wrap.
        // The registry rule is the primary check; the two SDK schemes are the
        // documented fallback for the publishable-key/client-secret SDK path.
        const auto auth_context = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize_operation(auth_context, "card_iin_info", true)) {
            send_error(res, send, *auth_error);
            return;
        }

        // Upstream GET /cards/{bin}: IIN is a path segment, client_secret a query param
        // (routes/cards_info.rs:31-40).
        const auto segments = ::app::http::RequestPath{}(req);
        const auto bin = segments.at(1); // /cards/:bin -> "cards", "<bin>"
        if (!bin) {
            server_wrap(
                "CardsInfo", res, send,
                std::expected<CardInfoResponse, AppError>{
                    std::unexpected{invalid_card_iin()}
                },
                "CARDS_INFO_RETRIEVED"
            );
            return;
        }
        ::app::model::cards_info::CardsInfoRequest payload;
        payload.set_card_iin(std::string{*bin});
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto client_secret = query.get("client_secret")) {
            payload.set_client_secret(*client_secret);
        }
        // Upstream routes/cards_info.rs:55-57 overwrites req.client_secret from the
        // authenticated credential. The AuthContext carries that authenticated claim.
        if (const auto authenticated_secret = auth_context.claim(::app::auth::kClaimClientSecret)) {
            payload.set_client_secret(std::string{*authenticated_secret});
        }

        // api::server_wrap(Flow::CardsInfo, ..., LockAction::NotApplicable) then calls
        // core/cards_info.rs:32-53 retrieve_card_info. No lock is acquired.
        server_wrap(
            "CardsInfo", res, send,
            retrieve_card_info(payload),
            "CARDS_INFO_RETRIEVED"
        );
    }

    void handle_create_cards_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize_operation(auth_context, "create_cards_info")) {
            send_error(res, send, *auth_error);
            return;
        }

        // web::Json<CardInfoCreateRequest> extraction (routes/cards_info.rs:71) is the
        // deserialization step that server_wrap's typed payload represents.
        const auto body = req.body_as_string();
        if (body.empty()) {
            server_wrap(
                "CardsInfoCreate", res, send,
                std::expected<CardInfoResponse, AppError>{
                    std::unexpected{invalid_request_data("Json payload deserialization failed: empty body")}
                },
                "CARDS_INFO_CREATED"
            );
            return;
        }
        using Input = CardInfoCreateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            server_wrap(
                "CardsInfoCreate", res, send,
                std::expected<CardInfoResponse, AppError>{
                    std::unexpected{invalid_request_data("Json payload deserialization failed")}
                },
                "CARDS_INFO_CREATED"
            );
            return;
        }

        // Upstream create_card_info has no IIN-length check (core/cards_info.rs:56-67).
        server_wrap(
            "CardsInfoCreate", res, send,
            create_card_info(*parsed),
            "CARDS_INFO_CREATED"
        );
    }

    void handle_update_cards_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize_operation(auth_context, "update_cards_info")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            server_wrap(
                "CardsInfoUpdate", res, send,
                std::expected<CardInfoResponse, AppError>{
                    std::unexpected{invalid_request_data("Json payload deserialization failed: empty body")}
                },
                "CARDS_INFO_UPDATED"
            );
            return;
        }
        using Input = CardInfoUpdateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            server_wrap(
                "CardsInfoUpdate", res, send,
                std::expected<CardInfoResponse, AppError>{
                    std::unexpected{invalid_request_data("Json payload deserialization failed")}
                },
                "CARDS_INFO_UPDATED"
            );
            return;
        }

        // Upstream update_card_info: update by IIN, last_updated = now(), missing record
        // is GenericNotFoundError (core/cards_info.rs:70-125). No IIN-length check.
        server_wrap(
            "CardsInfoUpdate", res, send,
            update_card_info(*parsed),
            "CARDS_INFO_UPDATED"
        );
    }

    void handle_migrate_cards_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize_operation(auth_context, "migrate_cards_info")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            server_wrap(
                "CardsInfoMigrate", res, send,
                std::expected<std::vector<CardInfoMigrationResponse>, AppError>{
                    std::unexpected{invalid_request_data("Multipart form deserialization failed: empty body")}
                }
            );
            return;
        }

        std::vector<CardInfoUpdateRequest> records;
        if (const auto csv_bytes = multipart_file_bytes(req, body)) {
            // Upstream CardsInfoUpdateForm caps the multipart file at 1MB and a CSV
            // parse failure maps to PreconditionFailed (core/cards_info.rs:127-157).
            if (csv_bytes->size() > (1024u * 1024u)) {
                server_wrap(
                    "CardsInfoMigrate", res, send,
                    std::expected<std::vector<CardInfoMigrationResponse>, AppError>{
                        std::unexpected{precondition_failed("cards bin CSV exceeds 1MB limit")}
                    }
                );
                return;
            }
            auto parsed_csv = parse_cards_csv(*csv_bytes);
            if (!parsed_csv) {
                server_wrap(
                    "CardsInfoMigrate", res, send,
                    std::expected<std::vector<CardInfoMigrationResponse>, AppError>{
                        std::unexpected{precondition_failed("failed to parse cards bin CSV")}
                    }
                );
                return;
            }
            records = std::move(*parsed_csv);
        } else {
            using Input = std::vector<CardInfoUpdateRequest>;
            auto parsed = serde::Ser::deserialize<Input>("application/json", body);
            if (!parsed) {
                server_wrap(
                    "CardsInfoMigrate", res, send,
                    std::expected<std::vector<CardInfoMigrationResponse>, AppError>{
                        std::unexpected{invalid_request_data("Json payload deserialization failed")}
                    }
                );
                return;
            }
            records = std::move(*parsed);
            // Upstream assigns a 1-based line_number from a row counter for every
            // record (core/cards_info.rs:138-143); caller input is never trusted.
            for (std::size_t i{0}; i < records.size(); ++i) {
                records[i].set_line_number(static_cast<std::int64_t>(i + 1));
            }
        }

        server_wrap(
            "CardsInfoMigrate", res, send,
            migrate_cards_info(std::move(records))
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/cards"}
            .add_route(
                Route{"/:bin"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_card_iin_info(req, res, std::move(send));
                    })
                    .summary("Hyperswitch card_iin_info")
            )
            .add_route(
                Route{"/create"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_cards_info(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_cards_info")
            )
            .add_route(
                Route{"/update"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_update_cards_info(req, res, std::move(send));
                    })
                    .summary("Hyperswitch update_cards_info")
            )
            .add_route(
                Route{"/update-batch"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_migrate_cards_info(req, res, std::move(send));
                    })
                    .summary("Hyperswitch migrate_cards_info")
            );
    }

    explicit CardsInfoRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    using AppError = ::core::error::AppError;
    using AppErrorKind = ::core::error::AppErrorKind;
    using CardInfoRecord = ::app::store::cards_info::CardInfoRecord;
    using CardInfoResponse = ::app::model::cards_info::CardInfoResponse;
    using CardInfoCreateRequest = ::app::model::cards_info::CardInfoCreateRequest;
    using CardInfoUpdateRequest = ::app::model::cards_info::CardInfoUpdateRequest;
    using CardInfoMigrateResponseRecord = ::app::model::cards_info::CardInfoMigrateResponseRecord;
    using CardInfoMigrationResponse = ::app::model::cards_info::CardInfoMigrationResponse;

    // ── Upstream ApiErrorResponse wire mapping ──────────────────────────────────
    // hyperswitch_domain_models/src/errors/api_error_response.rs:586-688, 459-461, 599-605.

    [[nodiscard]] AppError make_error(
        AppErrorKind kind,
        std::string message,
        std::string wire_name = {}
    ) const
    {
        auto error = ::core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
        if (!wire_name.empty()) {
            error.set_kind_name(std::move(wire_name));
        }
        return error;
    }

    [[nodiscard]] AppError invalid_request_data(std::string_view message) const
    {
        // InvalidRequestData -> IR_6, Unprocessable (api_error_response.rs:586-588).
        return make_error(AppErrorKind::VALIDATION_FAILED, std::string{message}, "InvalidRequestData");
    }

    [[nodiscard]] AppError invalid_card_iin_length() const
    {
        // InvalidCardIinLength -> IR_32, BadRequest (api_error_response.rs:670-671).
        return make_error(
            AppErrorKind::INVALID_REQUEST,
            "The provided card IIN length is invalid, please provide an IIN with 6 digits",
            "InvalidCardIinLength"
        );
    }

    [[nodiscard]] AppError invalid_card_iin() const
    {
        // InvalidCardIin -> IR_31, BadRequest (api_error_response.rs:670).
        return make_error(
            AppErrorKind::INVALID_REQUEST,
            "The provided card IIN does not exist",
            "InvalidCardIin"
        );
    }

    [[nodiscard]] AppError generic_duplicate_error(std::string_view message) const
    {
        // GenericDuplicateError -> IR_38, BadRequest (api_error_response.rs:687-689).
        return make_error(AppErrorKind::INVALID_REQUEST, std::string{message}, "GenericDuplicateError");
    }

    [[nodiscard]] AppError generic_not_found_error(std::string_view message) const
    {
        // GenericNotFoundError -> IR_37, NotFound (api_error_response.rs:684-686).
        return make_error(AppErrorKind::NOT_FOUND, std::string{message}, "GenericNotFoundError");
    }

    [[nodiscard]] AppError precondition_failed(std::string_view message) const
    {
        // PreconditionFailed -> IR_16, BadRequest (api_error_response.rs:626-628).
        return make_error(AppErrorKind::INVALID_REQUEST, std::string{message}, "PreconditionFailed");
    }

    [[nodiscard]] AppError invalid_data_value(std::string_view field_name) const
    {
        // InvalidDataValue -> IR_7, BadRequest (api_error_response.rs:589-593).
        return make_error(
            AppErrorKind::INVALID_REQUEST,
            "Invalid value provided: " + std::string{field_name},
            "InvalidDataValue"
        );
    }

    [[nodiscard]] AppError payment_not_found() const
    {
        // PaymentNotFound -> HE_2, NotFound (api_error_response.rs:459-461).
        return make_error(
            AppErrorKind::NOT_FOUND,
            "Payment does not exist in our records",
            "PaymentNotFound"
        );
    }

    [[nodiscard]] AppError client_secret_invalid() const
    {
        // ClientSecretInvalid -> IR_9, BadRequest (api_error_response.rs:604-606).
        return make_error(
            AppErrorKind::INVALID_REQUEST,
            "The client_secret provided does not match the client_secret associated with the Payment",
            "ClientSecretInvalid"
        );
    }

    [[nodiscard]] AppError client_secret_expired() const
    {
        // ClientSecretExpired -> IR_8, BadRequest (api_error_response.rs:599-603).
        return make_error(
            AppErrorKind::INVALID_REQUEST,
            "The provided client_secret has expired",
            "ClientSecretExpired"
        );
    }

    [[nodiscard]] AppError internal_server_error(std::string_view message) const
    {
        return make_error(AppErrorKind::INTERNAL_ERROR, std::string{message}, "InternalServerError");
    }

    // ── server_wrap equivalent ──────────────────────────────────────────────────

    /**
     * Local api::server_wrap (routes/cards_info.rs:49-64,75-84,95-104,119-128):
     * Flow instrumentation, auth (checked by authorize_operation before the core
     * call) and LockAction::NotApplicable (no lock is acquired). The typed payload
     * outcome is mapped to the upstream JSON response, or to a typed error body.
     */
    template <typename T>
    void server_wrap(
        std::string_view flow,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::expected<T, AppError> outcome,
        std::string_view counter = {},
        std::uint64_t delta = 1
    ) const
    {
        m_context->metrics().record_flow(flow);
        if (!outcome) {
            send_error(res, send, std::move(outcome.error()));
            return;
        }
        if (!counter.empty()) {
            m_context->metrics().counter(counter).increment(delta);
        }
        const auto text = serde::Ser::encode_json(*outcome);
        send_json(res, send, text, true);
    }

    [[nodiscard]] std::string now_stamp() const
    {
        return std::to_string(now_seconds());
    }

    [[nodiscard]] std::int64_t now_seconds() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    // ── core/cards_info.rs:24-29 verify_iin_length ──────────────────────────────

    [[nodiscard]] std::optional<AppError> verify_iin_length(std::string_view card_iin) const
    {
        const bool in_range = card_iin.size() == 6 || card_iin.size() == 8;
        if (in_range) {
            return std::nullopt;
        }
        return invalid_card_iin_length();
    }

    // ── core/payments/helpers.rs:5172-5177 get_payment_id_from_client_secret ────

    [[nodiscard]] std::optional<AppError> get_payment_id_from_client_secret(
        std::string_view secret,
        std::string& payment_id
    ) const
    {
        const auto pos = secret.rfind("_secret_");
        if (pos == std::string_view::npos) {
            return client_secret_invalid();
        }
        payment_id = std::string{secret.substr(0, pos)};
        if (payment_id.empty()) {
            return invalid_data_value("payment_id");
        }
        return std::nullopt;
    }

    // ── local store accessors for the payment intent's client secret/expiry ──────
    // The in-process PaymentIntentRecord has no dedicated client_secret /
    // session_expiry columns, so the upstream fields are read from its metadata
    // JSON (local in-process substitute for the upstream DB columns).

    [[nodiscard]] std::optional<serde::Value> metadata_field(
        const ::app::store::payments::PaymentIntentRecord& intent,
        std::string_view key
    ) const
    {
        const auto meta = serde::Ser::decode_generic("application/json", intent.get_metadata());
        if (!meta) {
            return std::nullopt;
        }
        const auto* object = std::get_if<serde::Value::Object>(&meta->get());
        if (object == nullptr) {
            return std::nullopt;
        }
        const auto field = object->get(std::string{key});
        if (!field) {
            return std::nullopt;
        }
        return std::optional<serde::Value>{field.value()};
    }

    [[nodiscard]] std::optional<std::string> intent_client_secret(
        const ::app::store::payments::PaymentIntentRecord& intent
    ) const
    {
        const auto field = metadata_field(intent, "client_secret");
        if (!field) {
            return std::nullopt;
        }
        if (const auto* text = std::get_if<std::string>(&field->get())) {
            return std::optional<std::string>{*text};
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::int64_t> json_integer(const serde::Value& value) const
    {
        if (const auto* number = std::get_if<std::int64_t>(&value.get())) {
            return *number;
        }
        if (const auto* number = std::get_if<double>(&value.get())) {
            return static_cast<std::int64_t>(*number);
        }
        if (const auto* text = std::get_if<std::string>(&value.get())) {
            std::int64_t parsed{};
            const auto result = std::from_chars(text->data(), text->data() + text->size(), parsed);
            if (result.ec == std::errc{} && result.ptr == text->data() + text->size()) {
                return parsed;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::int64_t> intent_session_expiry(
        const ::app::store::payments::PaymentIntentRecord& intent
    ) const
    {
        static constexpr std::int64_t DEFAULT_SESSION_EXPIRY = 15 * 60; // common_utils/consts.rs:85
        if (const auto field = metadata_field(intent, "session_expiry")) {
            if (const auto value = json_integer(*field)) {
                return value;
            }
        }
        // Fall back to created_at + DEFAULT_SESSION_EXPIRY
        // (core/payments/helpers.rs:4990-4994).
        const auto& created = intent.get_created_at();
        if (created.empty()) {
            return std::nullopt;
        }
        std::int64_t created_seconds{};
        const auto result = std::from_chars(created.data(), created.data() + created.size(), created_seconds);
        if (result.ec != std::errc{} || result.ptr != created.data() + created.size()) {
            return std::nullopt;
        }
        return created_seconds + DEFAULT_SESSION_EXPIRY;
    }

    // ── core/payments/helpers.rs:4979-5005 authenticate_client_secret ──────────

    [[nodiscard]] std::optional<AppError> authenticate_client_secret(
        const std::optional<std::string>& presented,
        const ::app::store::payments::PaymentIntentRecord& intent
    ) const
    {
        if (!presented) {
            return std::nullopt; // (_ , _) -> Ok
        }
        const auto expected = intent_client_secret(intent);
        if (!expected) {
            return client_secret_expired(); // (Some, None) -> ClientSecretExpired
        }
        if (*presented != *expected) {
            return client_secret_invalid(); // req_cs != pi_cs -> ClientSecretInvalid
        }
        const auto expiry = intent_session_expiry(intent);
        if (expiry && now_seconds() > *expiry) {
            return client_secret_expired();
        }
        return std::nullopt;
    }

    // ── core/payments/helpers.rs:5092-5135 verify_payment_intent_time_and_client_secret ──

    [[nodiscard]] std::optional<AppError> verify_payment_intent_time_and_client_secret(
        const std::optional<std::string>& client_secret
    ) const
    {
        if (!client_secret) {
            return std::nullopt;
        }
        std::string payment_id{};
        if (const auto error = get_payment_id_from_client_secret(*client_secret, payment_id)) {
            return error;
        }
        const auto intent = m_context->repository().find_payment_by_string(payment_id);
        if (!intent) {
            return payment_not_found();
        }
        return authenticate_client_secret(client_secret, *intent);
    }

    [[nodiscard]] std::optional<AppError> authorize_operation(
        const ::app::auth::AuthContext& context,
        std::string_view operation,
        bool sdk_fallback = false
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        auto auth = m_context->authorizer().authorize_route(context, rule);
        if (auth && sdk_fallback) {
            // routes/cards_info.rs:25,44-47: api_key (x-api-key) or publishable_key
            // with client_secret is accepted; both are real upstream auth paths.
            const auto api_key_rule = ::app::auth::RouteAuthRuleBuilder{}
                                          .scheme(::app::auth::AuthScheme::V2_API_KEY)
                                          .permission(rule.permission())
                                          .build();
            auth = m_context->authorizer().authorize_route(context, api_key_rule);
            if (auth) {
                const auto sdk_rule = ::app::auth::RouteAuthRuleBuilder{}
                                          .scheme(::app::auth::AuthScheme::SDK)
                                          .permission(rule.permission())
                                          .build();
                auth = m_context->authorizer().authorize_route(context, sdk_rule);
            }
            // auth is nullopt when either SDK path authorizes; otherwise it carries
            // the SDK path's error, matching check_sdk_auth_and_get_auth's rejection.
        }
        return auth;
    }

    // ── local substitutes for the upstream full-column store ────────────────────
    // CardInfoRecord has only iin/scheme/type/bank/country; the upstream DB row
    // carries every CardInfoResponse column. The full serialized response is kept
    // in-process under the IIN so retrieve and migration emit the same wire shape.

    [[nodiscard]] std::string response_cache_key(std::string_view iin) const
    {
        return "cards_info.response." + std::string{iin};
    }

    void cache_response(std::string_view iin, const CardInfoResponse& response) const
    {
        m_context->cache().put(
            response_cache_key(iin),
            core::cache::to_cache_value(serde::Ser::encode_json(response))
        );
    }

    [[nodiscard]] std::optional<CardInfoResponse> cached_response(std::string_view iin) const
    {
        const auto result = m_context->cache().get(response_cache_key(iin));
        if (!result.hit) {
            return std::nullopt;
        }
        return serde::Ser::deserialize<CardInfoResponse>(
            "application/json", result.value_or<std::string>("")
        );
    }

    [[nodiscard]] CardInfoResponse response_from_store_enriched(
        const CardInfoRecord& record,
        CardInfoResponse fallback
    ) const
    {
        // CardInfoResponse::foreign_from(card_info) (core/cards_info.rs:50-52).
        fallback.set_card_iin(record.get_iin());
        if (!record.get_scheme().empty()) {
            fallback.set_card_network(std::optional<std::string>{record.get_scheme()});
        }
        if (!record.get_card_type().empty()) {
            fallback.set_card_type(record.get_card_type());
        }
        if (!record.get_bank_name().empty()) {
            fallback.set_card_issuer(record.get_bank_name());
        }
        if (!record.get_country().empty()) {
            fallback.set_country_code(record.get_country());
        }
        return fallback;
    }

    // ── core/cards_info.rs:32-53 retrieve_card_info ─────────────────────────────

    [[nodiscard]] std::expected<CardInfoResponse, AppError> retrieve_card_info(
        const ::app::model::cards_info::CardsInfoRequest& payload
    ) const
    {
        if (const auto invalid = verify_iin_length(payload.get_card_iin())) {
            return std::unexpected{*invalid};
        }
        if (const auto invalid = verify_payment_intent_time_and_client_secret(payload.get_client_secret())) {
            return std::unexpected{*invalid};
        }
        const auto stored = m_context->repository().find_card_info_by_string(payload.get_card_iin());
        if (!stored) {
            return std::unexpected{invalid_card_iin()};
        }
        auto fallback = cached_response(payload.get_card_iin()).value_or(CardInfoResponse{});
        return response_from_store_enriched(*stored, std::move(fallback));
    }

    // ── core/cards_info.rs:56-67 create_card_info ───────────────────────────────

    [[nodiscard]] std::expected<CardInfoResponse, AppError> create_card_info(
        const CardInfoCreateRequest& payload
    ) const
    {
        auto repository = m_context->repository();
        auto created = repository.create_card_info(to_store_record(payload));
        if (!created) {
            return std::unexpected{generic_duplicate_error(
                "CardInfo with given key already exists in our records"
            )};
        }
        auto response = build_response(payload);
        cache_response(created->get_iin(), response);
        return response_from_store_enriched(*created, std::move(response));
    }

    // ── core/cards_info.rs:70-125 update_card_info ──────────────────────────────

    [[nodiscard]] std::expected<CardInfoResponse, AppError> update_card_info(
        const CardInfoUpdateRequest& payload
    ) const
    {
        auto repository = m_context->repository();
        auto stored = repository.find_card_info_by_string(payload.get_card_iin());
        if (!stored) {
            return std::unexpected{generic_not_found_error(
                "Card info with given key does not exist in our records"
            )};
        }
        apply_update(*stored, payload); // last_updated = now()
        if (!repository.update_card_info(*stored)) {
            return std::unexpected{internal_server_error("Failed while updating card info")};
        }
        auto response = build_response(payload);
        cache_response(stored->get_iin(), response);
        return response_from_store_enriched(*stored, std::move(response));
    }

    // ── core/cards_info.rs:179-196 typestate markers ────────────────────────────
    // State/TransitionTo are represented as a local enum + builder; the legal
    // transitions are Fetch->Add/Update and Add/Update->Response.

    enum class MigrateState
    {
        Fetch,
        Add,
        Update,
        Response,
    };

    class CardInfoBuilder
    {
    public:
        [[nodiscard]] static CardInfoBuilder new_builder()
        {
            return CardInfoBuilder{};
        }
        [[nodiscard]] CardInfoBuilder set_card_info(CardInfoMigrateResponseRecord record) const
        {
            return CardInfoBuilder{MigrateState::Update, std::move(record)};
        }
        [[nodiscard]] CardInfoBuilder transition() const
        {
            return CardInfoBuilder{MigrateState::Add, std::nullopt};
        }
        [[nodiscard]] CardInfoBuilder set_updated_card_info(CardInfoMigrateResponseRecord record) const
        {
            return CardInfoBuilder{MigrateState::Response, std::move(record)};
        }
        [[nodiscard]] CardInfoBuilder set_added_card_info(CardInfoMigrateResponseRecord record) const
        {
            return CardInfoBuilder{MigrateState::Response, std::move(record)};
        }
        [[nodiscard]] CardInfoMigrateResponseRecord build() const
        {
            return card_info.value_or(CardInfoMigrateResponseRecord{});
        }
        [[nodiscard]] MigrateState state() const noexcept { return this->state_value; }

    private:
        explicit CardInfoBuilder(
            MigrateState value = MigrateState::Fetch,
            std::optional<CardInfoMigrateResponseRecord> record = std::nullopt
        )
            : state_value{value}, card_info{std::move(record)}
        {
        }
        MigrateState state_value{MigrateState::Fetch};
        std::optional<CardInfoMigrateResponseRecord> card_info{};
    };

    // ── core/cards_info.rs:199-287 CardInfoMigrateExecutor ──────────────────────

    class CardInfoMigrateExecutor
    {
    public:
        CardInfoMigrateExecutor(
            const CardsInfoRouteHandlers& handler,
            const CardInfoUpdateRequest& record
        )
            : m_handler{handler}, m_record{record}
        {
        }

        [[nodiscard]] std::expected<std::optional<CardInfoRecord>, AppError> fetch_card_info() const
        {
            // core/cards_info.rs:212-219; storage error maps to InvalidCardIin.
            return m_handler.m_context->repository().find_card_info_by_string(
                m_record.get_card_iin()
            );
        }

        [[nodiscard]] std::expected<CardInfoRecord, AppError> add_card_info() const
        {
            // core/cards_info.rs:221-232; duplicate -> GenericDuplicateError.
            auto added = m_handler.m_context->repository().create_card_info(
                m_handler.to_store_record(m_record)
            );
            if (!added) {
                return std::unexpected{m_handler.generic_duplicate_error(
                    "CardInfo with given key already exists in our records"
                )};
            }
            m_handler.cache_response(m_record.get_card_iin(), m_handler.build_response(m_record));
            return *added;
        }

        [[nodiscard]] std::expected<CardInfoRecord, AppError> update_card_info() const
        {
            // core/cards_info.rs:234-286; last_updated=now(), miss -> GenericNotFoundError.
            auto repository = m_handler.m_context->repository();
            auto stored = repository.find_card_info_by_string(m_record.get_card_iin());
            if (!stored) {
                return std::unexpected{m_handler.generic_not_found_error(
                    "Card info with given key does not exist in our records"
                )};
            }
            m_handler.apply_update(*stored, m_record);
            if (!repository.update_card_info(*stored)) {
                return std::unexpected{m_handler.internal_server_error(
                    "Failed while updating card info"
                )};
            }
            m_handler.cache_response(m_record.get_card_iin(), m_handler.build_response(m_record));
            return *stored;
        }

    private:
        const CardsInfoRouteHandlers& m_handler;
        const CardInfoUpdateRequest& m_record;
    };

    // ── core/cards_info.rs:289-345 CardInfoBuilder::build ───────────────────────

    [[nodiscard]] CardInfoMigrateResponseRecord to_migrate_record(
        const CardInfoResponse& response
    ) const
    {
        using namespace ::app::model::cards_info;
        CardInfoMigrateResponseRecord out;
        if (!response.get_card_iin().empty()) {
            out.set_card_iin(response.get_card_iin());
        }
        out.set_card_issuer(response.get_card_issuer());
        out.set_card_network(response.get_card_network());
        out.set_card_type(response.get_card_type());
        out.set_card_sub_type(response.get_card_sub_type());
        out.set_card_issuing_country(response.get_card_issuing_country());
        out.set_country_code(response.get_country_code());
        out.set_funding_source(response.get_funding_source());
        out.set_card_iin_type(response.get_card_iin_type());
        out.set_virtual_card(response.get_virtual_card());
        out.set_gambling_blocked(response.get_gambling_blocked());
        out.set_co_badged_card_networks(response.get_co_badged_card_networks());
        out.set_card_segment_type(response.get_card_segment_type());
        out.set_numeric_country_code(response.get_numeric_country_code());
        out.set_prepaid(response.get_prepaid());
        out.set_regulated(response.get_regulated());
        out.set_issuer_phone(response.get_issuer_phone());
        out.set_issuer_url(response.get_issuer_url());
        out.set_regulated_name(response.get_regulated_name());
        out.set_reloadable_prepaid(response.get_reloadable_prepaid());
        out.set_account_updater(response.get_account_updater());
        out.set_account_level_management(response.get_account_level_management());
        out.set_domestic_only(response.get_domestic_only());
        out.set_level_two_supported(response.get_level_two_supported());
        out.set_level_three_supported(response.get_level_three_supported());
        out.set_issuer_currency(response.get_issuer_currency());
        out.set_combo_card(response.get_combo_card());
        out.set_authentication(response.get_authentication());
        out.set_cost(response.get_cost());
        out.set_issuer_supports_tokenization(response.get_issuer_supports_tokenization());
        out.set_billpay_enabled(response.get_billpay_enabled());
        out.set_ecom_enabled(response.get_ecom_enabled());
        out.set_flexible_credential_supported(response.get_flexible_credential_supported());
        out.set_card_subtype_code(response.get_card_subtype_code());
        out.set_multi_account_access_indicator(response.get_multi_account_access_indicator());
        return out;
    }

    [[nodiscard]] CardInfoMigrateResponseRecord migrate_record_from_store(
        const CardInfoRecord& record
    ) const
    {
        auto fallback = cached_response(record.get_iin()).value_or(CardInfoResponse{});
        return to_migrate_record(response_from_store_enriched(record, std::move(fallback)));
    }

    // ── core/cards_info.rs:428-450 card_info_flow ───────────────────────────────

    [[nodiscard]] std::expected<CardInfoMigrateResponseRecord, std::string> card_info_flow(
        const CardInfoUpdateRequest& record
    ) const
    {
        const auto builder = CardInfoBuilder::new_builder();
        const CardInfoMigrateExecutor executor{*this, record};
        auto fetched = executor.fetch_card_info();
        if (!fetched) {
            return std::unexpected{fetched.error().get_message()};
        }
        if (*fetched) {
            // Fetch -> Update -> Response.
            const auto updated_builder = builder.set_card_info(
                migrate_record_from_store(**fetched)
            );
            auto updated = executor.update_card_info();
            if (!updated) {
                return std::unexpected{updated.error().get_message()};
            }
            return updated_builder.set_updated_card_info(
                migrate_record_from_store(*updated)
            ).build();
        }
        // Fetch -> Add -> Response.
        const auto added_builder = builder.transition();
        auto added = executor.add_card_info();
        if (!added) {
            return std::unexpected{added.error().get_message()};
        }
        return added_builder.set_added_card_info(
            migrate_record_from_store(*added)
        ).build();
    }

    // ── api_models/cards_info.rs:349-406 CardInfoMigrationResponse::from ────────

    [[nodiscard]] CardInfoMigrationResponse migration_response_from(
        std::expected<CardInfoMigrateResponseRecord, std::string> outcome,
        const CardInfoUpdateRequest& record
    ) const
    {
        using namespace ::app::model::cards_info;
        CardInfoMigrationResponse out;
        out.set_card_iin(record.get_card_iin());
        out.set_line_number(record.get_line_number());
        if (outcome) {
            const auto& res = *outcome;
            out.set_card_issuer(res.get_card_issuer());
            out.set_card_network(res.get_card_network());
            out.set_card_type(res.get_card_type());
            out.set_card_sub_type(res.get_card_sub_type());
            out.set_card_issuing_country(res.get_card_issuing_country());
            out.set_country_code(res.get_country_code());
            out.set_funding_source(res.get_funding_source());
            out.set_card_iin_type(res.get_card_iin_type());
            out.set_virtual_card(res.get_virtual_card());
            out.set_gambling_blocked(res.get_gambling_blocked());
            out.set_co_badged_card_networks(res.get_co_badged_card_networks());
            out.set_card_segment_type(res.get_card_segment_type());
            out.set_numeric_country_code(res.get_numeric_country_code());
            out.set_prepaid(res.get_prepaid());
            out.set_regulated(res.get_regulated());
            out.set_issuer_phone(res.get_issuer_phone());
            out.set_issuer_url(res.get_issuer_url());
            out.set_regulated_name(res.get_regulated_name());
            out.set_reloadable_prepaid(res.get_reloadable_prepaid());
            out.set_account_updater(res.get_account_updater());
            out.set_account_level_management(res.get_account_level_management());
            out.set_domestic_only(res.get_domestic_only());
            out.set_level_two_supported(res.get_level_two_supported());
            out.set_level_three_supported(res.get_level_three_supported());
            out.set_issuer_currency(res.get_issuer_currency());
            out.set_combo_card(res.get_combo_card());
            out.set_authentication(res.get_authentication());
            out.set_cost(res.get_cost());
            out.set_issuer_supports_tokenization(res.get_issuer_supports_tokenization());
            out.set_billpay_enabled(res.get_billpay_enabled());
            out.set_ecom_enabled(res.get_ecom_enabled());
            out.set_flexible_credential_supported(res.get_flexible_credential_supported());
            out.set_card_subtype_code(res.get_card_subtype_code());
            out.set_multi_account_access_indicator(res.get_multi_account_access_indicator());
            out.set_migration_status(CardInfoMigrationStatus::Success);
        } else {
            // Err(e) -> default fields, migration_error = e, status Failed.
            out.set_migration_status(CardInfoMigrationStatus::Failed);
            out.set_migration_error(std::string{outcome.error()});
        }
        return out;
    }

    // ── core/cards_info.rs:159-177 migrate_cards_info ───────────────────────────

    [[nodiscard]] std::expected<std::vector<CardInfoMigrationResponse>, AppError> migrate_cards_info(
        std::vector<CardInfoUpdateRequest> records
    ) const
    {
        std::vector<CardInfoMigrationResponse> result{};
        result.reserve(records.size());
        std::size_t migrated{0};
        for (const auto& record : records) {
            auto flow = card_info_flow(record);
            if (flow) {
                ++migrated;
            }
            result.push_back(migration_response_from(std::move(flow), record));
        }
        m_context->metrics().counter("CARDS_INFO_MIGRATED").increment(
            static_cast<std::uint64_t>(migrated)
        );
        return result;
    }

    [[nodiscard]] CardInfoRecord to_store_record(const CardInfoCreateRequest& in) const
    {
        CardInfoRecord record;
        record.set_iin(in.get_card_iin());
        apply_update(record, in);
        record.set_status("active");
        record.set_created_at(now_stamp());
        return record;
    }

    [[nodiscard]] CardInfoRecord to_store_record(const CardInfoUpdateRequest& in) const
    {
        CardInfoRecord record;
        record.set_iin(in.get_card_iin());
        apply_update(record, in);
        record.set_status("active");
        record.set_created_at(now_stamp());
        return record;
    }

    void apply_update(CardInfoRecord& record, const CardInfoCreateRequest& in) const
    {
        if (in.get_card_network()) {
            record.set_scheme(in.get_card_network()->get_value());
        }
        if (in.get_card_type()) {
            record.set_card_type(*in.get_card_type());
        }
        if (in.get_card_issuer()) {
            record.set_bank_name(*in.get_card_issuer());
        }
        if (in.get_country_code()) {
            record.set_country(*in.get_country_code());
        } else if (in.get_card_issuing_country()) {
            record.set_country(*in.get_card_issuing_country());
        }
        // Upstream UpdateCardInfo.last_updated = Some(now()) (core/cards_info.rs:87,248).
        record.set_updated_at(now_stamp());
    }

    void apply_update(CardInfoRecord& record, const CardInfoUpdateRequest& in) const
    {
        if (in.get_card_network()) {
            record.set_scheme(in.get_card_network()->get_value());
        }
        if (in.get_card_type()) {
            record.set_card_type(*in.get_card_type());
        }
        if (in.get_card_issuer()) {
            record.set_bank_name(*in.get_card_issuer());
        }
        if (in.get_country_code()) {
            record.set_country(*in.get_country_code());
        } else if (in.get_card_issuing_country()) {
            record.set_country(*in.get_card_issuing_country());
        }
        record.set_updated_at(now_stamp());
    }

    [[nodiscard]] std::optional<std::string> multipart_file_bytes(
        interfaces::io::IRequest& req,
        std::string_view body
    ) const
    {
        const std::string content_type{req.find_header("content-type")};
        if (content_type.find("multipart/form-data") == std::string::npos) {
            return std::nullopt;
        }
        const auto marker = content_type.find("boundary=");
        if (marker == std::string::npos) {
            return std::nullopt;
        }
        std::string boundary = content_type.substr(marker + 9);
        const auto semicolon = boundary.find(';');
        if (semicolon != std::string::npos) {
            boundary = boundary.substr(0, semicolon);
        }
        const auto trim = [](std::string& text)
        {
            while (!text.empty() && (text.front() == '"' || text.front() == ' ' || text.front() == '\t')) {
                text.erase(text.begin());
            }
            while (!text.empty() && (text.back() == '"' || text.back() == ' ' || text.back() == '\t')) {
                text.pop_back();
            }
        };
        trim(boundary);
        if (boundary.empty()) {
            return std::nullopt;
        }

        const std::string delimiter = "--" + boundary;
        std::size_t cursor{0};
        while (cursor < body.size()) {
            const auto start = body.find(delimiter, cursor);
            if (start == std::string_view::npos) {
                break;
            }
            std::size_t header_start = start + delimiter.size();
            if (body.substr(header_start, 2) == "--") {
                break;
            }
            while (header_start < body.size() &&
                   (body[header_start] == '\r' || body[header_start] == '\n')) {
                ++header_start;
            }
            std::size_t header_end = body.find("\r\n\r\n", header_start);
            std::size_t payload_start{};
            if (header_end != std::string_view::npos) {
                payload_start = header_end + 4;
            } else {
                header_end = body.find("\n\n", header_start);
                if (header_end == std::string_view::npos) {
                    break;
                }
                payload_start = header_end + 2;
            }
            const std::string_view part_headers = body.substr(header_start, header_end - header_start);
            const auto next = body.find(delimiter, payload_start);
            if (next == std::string_view::npos) {
                break;
            }
            std::size_t payload_end = next;
            while (payload_end > payload_start &&
                   (body[payload_end - 1] == '\r' || body[payload_end - 1] == '\n')) {
                --payload_end;
            }
            if (part_headers.find("name=\"file\"") != std::string_view::npos ||
                part_headers.find("name=file") != std::string_view::npos) {
                return std::string{body.substr(payload_start, payload_end - payload_start)};
            }
            cursor = next;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::vector<std::string> split_csv_line(std::string_view line) const
    {
        std::vector<std::string> fields;
        std::string current;
        bool quoted{false};
        for (std::size_t i{0}; i < line.size(); ++i) {
            const char ch = line[i];
            if (quoted) {
                if (ch == '"') {
                    if (i + 1 < line.size() && line[i + 1] == '"') {
                        current.push_back('"');
                        ++i;
                    } else {
                        quoted = false;
                    }
                } else {
                    current.push_back(ch);
                }
            } else if (ch == '"') {
                quoted = true;
            } else if (ch == ',') {
                fields.push_back(std::move(current));
                current.clear();
            } else {
                current.push_back(ch);
            }
        }
        fields.push_back(std::move(current));
        return fields;
    }

    [[nodiscard]] std::optional<bool> parse_bool(std::string_view value) const
    {
        std::string lowered{value};
        std::transform(
            lowered.begin(), lowered.end(), lowered.begin(),
            [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); }
        );
        if (lowered == "true" || lowered == "1" || lowered == "yes") {
            return true;
        }
        if (lowered == "false" || lowered == "0" || lowered == "no") {
            return false;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::int64_t> parse_int(std::string_view value) const
    {
        std::int64_t parsed{};
        const auto result = std::from_chars(value.data(), value.data() + value.size(), parsed);
        if (result.ec != std::errc{} || result.ptr != value.data() + value.size()) {
            return std::nullopt;
        }
        return parsed;
    }

    void apply_csv_field(CardInfoUpdateRequest& out, std::string_view header, std::string_view value) const
    {
        using ::app::model::cards_info::CardAuthenticationValue;
        using ::app::model::cards_info::CardCostValue;
        using ::app::model::cards_info::CardNetworkValue;
        using ::app::model::cards_info::CoBadgedCardNetworkMetadataValue;
        using ::app::model::cards_info::FundingSourceValue;
        using ::app::model::cards_info::PanOrTokenValue;

        const auto text = [value]() { return std::string{value}; };
        if (header == "card_iin") {
            out.set_card_iin(text());
        } else if (header == "card_issuer") {
            out.set_card_issuer(text());
        } else if (header == "card_network") {
            CardNetworkValue wrapped;
            wrapped.set_value(text());
            out.set_card_network(std::move(wrapped));
        } else if (header == "card_type") {
            out.set_card_type(text());
        } else if (header == "card_subtype") {
            out.set_card_subtype(text());
        } else if (header == "card_issuing_country") {
            out.set_card_issuing_country(text());
        } else if (header == "bank_code_id") {
            out.set_bank_code_id(text());
        } else if (header == "bank_code") {
            out.set_bank_code(text());
        } else if (header == "country_code") {
            out.set_country_code(text());
        } else if (header == "last_updated_provider") {
            out.set_last_updated_provider(text());
        } else if (header == "funding_source") {
            FundingSourceValue wrapped;
            wrapped.set_value(text());
            out.set_funding_source(std::move(wrapped));
        } else if (header == "card_iin_type") {
            PanOrTokenValue wrapped;
            wrapped.set_value(text());
            out.set_card_iin_type(std::move(wrapped));
        } else if (header == "virtual_card") {
            if (const auto flag = parse_bool(value)) { out.set_virtual_card(*flag); }
        } else if (header == "gambling_blocked") {
            if (const auto flag = parse_bool(value)) { out.set_gambling_blocked(*flag); }
        } else if (header == "co_badged_card_networks") {
            CoBadgedCardNetworkMetadataValue wrapped;
            wrapped.set_value(text());
            out.set_co_badged_card_networks(std::move(wrapped));
        } else if (header == "card_segment_type") {
            out.set_card_segment_type(text());
        } else if (header == "numeric_country_code") {
            out.set_numeric_country_code(text());
        } else if (header == "prepaid") {
            if (const auto flag = parse_bool(value)) { out.set_prepaid(*flag); }
        } else if (header == "regulated") {
            if (const auto flag = parse_bool(value)) { out.set_regulated(*flag); }
        } else if (header == "issuer_phone") {
            out.set_issuer_phone(text());
        } else if (header == "issuer_url") {
            out.set_issuer_url(text());
        } else if (header == "regulated_name") {
            out.set_regulated_name(text());
        } else if (header == "reloadable_prepaid") {
            if (const auto flag = parse_bool(value)) { out.set_reloadable_prepaid(*flag); }
        } else if (header == "account_updater") {
            if (const auto flag = parse_bool(value)) { out.set_account_updater(*flag); }
        } else if (header == "account_level_management") {
            if (const auto flag = parse_bool(value)) { out.set_account_level_management(*flag); }
        } else if (header == "domestic_only") {
            if (const auto flag = parse_bool(value)) { out.set_domestic_only(*flag); }
        } else if (header == "level_two_supported") {
            if (const auto flag = parse_bool(value)) { out.set_level_two_supported(*flag); }
        } else if (header == "level_three_supported") {
            if (const auto flag = parse_bool(value)) { out.set_level_three_supported(*flag); }
        } else if (header == "issuer_currency") {
            out.set_issuer_currency(text());
        } else if (header == "combo_card") {
            out.set_combo_card(text());
        } else if (header == "authentication") {
            CardAuthenticationValue wrapped;
            wrapped.set_value(text());
            out.set_authentication(std::move(wrapped));
        } else if (header == "cost") {
            CardCostValue wrapped;
            wrapped.set_value(text());
            out.set_cost(std::move(wrapped));
        } else if (header == "issuer_supports_tokenization") {
            if (const auto flag = parse_bool(value)) { out.set_issuer_supports_tokenization(*flag); }
        } else if (header == "billpay_enabled") {
            if (const auto flag = parse_bool(value)) { out.set_billpay_enabled(*flag); }
        } else if (header == "ecom_enabled") {
            if (const auto flag = parse_bool(value)) { out.set_ecom_enabled(*flag); }
        } else if (header == "flexible_credential_supported") {
            if (const auto flag = parse_bool(value)) { out.set_flexible_credential_supported(*flag); }
        } else if (header == "card_subtype_code") {
            out.set_card_subtype_code(text());
        } else if (header == "multi_account_access_indicator") {
            out.set_multi_account_access_indicator(text());
        } else if (header == "line_number") {
            if (const auto number = parse_int(value)) { out.set_line_number(*number); }
        }
    }

    // ── core/cards_info.rs:133-157 parse_cards_bin_csv / get_cards_bin_records ───

    [[nodiscard]] std::optional<std::vector<CardInfoUpdateRequest>> parse_cards_csv(
        std::string_view csv
    ) const
    {
        std::vector<std::string_view> lines;
        std::size_t start{0};
        while (start <= csv.size()) {
            const auto newline = csv.find('\n', start);
            const auto end = newline == std::string_view::npos ? csv.size() : newline;
            std::string_view line = csv.substr(start, end - start);
            if (!line.empty() && line.back() == '\r') {
                line.remove_suffix(1);
            }
            if (!line.empty()) {
                lines.push_back(line);
            }
            if (newline == std::string_view::npos) {
                break;
            }
            start = newline + 1;
        }
        if (lines.empty()) {
            return std::vector<CardInfoUpdateRequest>{};
        }
        const auto headers = split_csv_line(lines.front());
        if (headers.empty() || headers.front().empty()) {
            return std::nullopt;
        }

        std::vector<CardInfoUpdateRequest> records;
        std::int64_t line_number{0};
        for (std::size_t i{1}; i < lines.size(); ++i) {
            const auto values = split_csv_line(lines[i]);
            if (values.size() == 1 && values.front().empty()) {
                continue;
            }
            CardInfoUpdateRequest record;
            for (std::size_t column{0}; column < headers.size() && column < values.size(); ++column) {
                if (!values[column].empty()) {
                    apply_csv_field(record, headers[column], values[column]);
                }
            }
            // core/cards_info.rs:138-143 generates line numbers from a row counter.
            record.set_line_number(++line_number);
            records.push_back(std::move(record));
        }
        return records;
    }

    [[nodiscard]] CardInfoResponse build_response(const CardInfoCreateRequest& in) const
    {
        using namespace app::model::cards_info;
        CardInfoResponse out;
        out.set_card_iin(in.get_card_iin());
        if (in.get_card_issuer()) { out.set_card_issuer(in.get_card_issuer()); }
        if (in.get_card_network()) { out.set_card_network(in.get_card_network()->get_value()); }
        if (in.get_card_type()) { out.set_card_type(in.get_card_type()); }
        if (in.get_card_subtype()) { out.set_card_sub_type(in.get_card_subtype()); }
        if (in.get_card_issuing_country()) { out.set_card_issuing_country(in.get_card_issuing_country()); }
        if (in.get_country_code()) { out.set_country_code(in.get_country_code()); }
        if (in.get_funding_source()) { out.set_funding_source(in.get_funding_source()->get_value()); }
        if (in.get_card_iin_type()) { out.set_card_iin_type(in.get_card_iin_type()->get_value()); }
        if (in.get_virtual_card()) { out.set_virtual_card(in.get_virtual_card()); }
        if (in.get_gambling_blocked()) { out.set_gambling_blocked(in.get_gambling_blocked()); }
        if (in.get_co_badged_card_networks()) { out.set_co_badged_card_networks(in.get_co_badged_card_networks()); }
        if (in.get_card_segment_type()) { out.set_card_segment_type(in.get_card_segment_type()); }
        if (in.get_numeric_country_code()) { out.set_numeric_country_code(in.get_numeric_country_code()); }
        if (in.get_prepaid()) { out.set_prepaid(in.get_prepaid()); }
        if (in.get_regulated()) { out.set_regulated(in.get_regulated()); }
        if (in.get_issuer_phone()) { out.set_issuer_phone(in.get_issuer_phone()); }
        if (in.get_issuer_url()) { out.set_issuer_url(in.get_issuer_url()); }
        if (in.get_regulated_name()) { out.set_regulated_name(in.get_regulated_name()); }
        if (in.get_reloadable_prepaid()) { out.set_reloadable_prepaid(in.get_reloadable_prepaid()); }
        if (in.get_account_updater()) { out.set_account_updater(in.get_account_updater()); }
        if (in.get_account_level_management()) { out.set_account_level_management(in.get_account_level_management()); }
        if (in.get_domestic_only()) { out.set_domestic_only(in.get_domestic_only()); }
        if (in.get_level_two_supported()) { out.set_level_two_supported(in.get_level_two_supported()); }
        if (in.get_level_three_supported()) { out.set_level_three_supported(in.get_level_three_supported()); }
        if (in.get_issuer_currency()) { out.set_issuer_currency(in.get_issuer_currency()); }
        if (in.get_combo_card()) { out.set_combo_card(in.get_combo_card()); }
        if (in.get_authentication()) { out.set_authentication(in.get_authentication()); }
        if (in.get_cost()) { out.set_cost(in.get_cost()); }
        if (in.get_issuer_supports_tokenization()) { out.set_issuer_supports_tokenization(in.get_issuer_supports_tokenization()); }
        if (in.get_billpay_enabled()) { out.set_billpay_enabled(in.get_billpay_enabled()); }
        if (in.get_ecom_enabled()) { out.set_ecom_enabled(in.get_ecom_enabled()); }
        if (in.get_flexible_credential_supported()) { out.set_flexible_credential_supported(in.get_flexible_credential_supported()); }
        if (in.get_card_subtype_code()) { out.set_card_subtype_code(in.get_card_subtype_code()); }
        if (in.get_multi_account_access_indicator()) { out.set_multi_account_access_indicator(in.get_multi_account_access_indicator()); }
        return out;
    }

    [[nodiscard]] CardInfoResponse build_response(const CardInfoUpdateRequest& in) const
    {
        using namespace app::model::cards_info;
        CardInfoResponse out;
        out.set_card_iin(in.get_card_iin());
        if (in.get_card_issuer()) { out.set_card_issuer(in.get_card_issuer()); }
        if (in.get_card_network()) { out.set_card_network(in.get_card_network()->get_value()); }
        if (in.get_card_type()) { out.set_card_type(in.get_card_type()); }
        if (in.get_card_subtype()) { out.set_card_sub_type(in.get_card_subtype()); }
        if (in.get_card_issuing_country()) { out.set_card_issuing_country(in.get_card_issuing_country()); }
        if (in.get_country_code()) { out.set_country_code(in.get_country_code()); }
        if (in.get_funding_source()) { out.set_funding_source(in.get_funding_source()->get_value()); }
        if (in.get_card_iin_type()) { out.set_card_iin_type(in.get_card_iin_type()->get_value()); }
        if (in.get_virtual_card()) { out.set_virtual_card(in.get_virtual_card()); }
        if (in.get_gambling_blocked()) { out.set_gambling_blocked(in.get_gambling_blocked()); }
        if (in.get_co_badged_card_networks()) { out.set_co_badged_card_networks(in.get_co_badged_card_networks()); }
        if (in.get_card_segment_type()) { out.set_card_segment_type(in.get_card_segment_type()); }
        if (in.get_numeric_country_code()) { out.set_numeric_country_code(in.get_numeric_country_code()); }
        if (in.get_prepaid()) { out.set_prepaid(in.get_prepaid()); }
        if (in.get_regulated()) { out.set_regulated(in.get_regulated()); }
        if (in.get_issuer_phone()) { out.set_issuer_phone(in.get_issuer_phone()); }
        if (in.get_issuer_url()) { out.set_issuer_url(in.get_issuer_url()); }
        if (in.get_regulated_name()) { out.set_regulated_name(in.get_regulated_name()); }
        if (in.get_reloadable_prepaid()) { out.set_reloadable_prepaid(in.get_reloadable_prepaid()); }
        if (in.get_account_updater()) { out.set_account_updater(in.get_account_updater()); }
        if (in.get_account_level_management()) { out.set_account_level_management(in.get_account_level_management()); }
        if (in.get_domestic_only()) { out.set_domestic_only(in.get_domestic_only()); }
        if (in.get_level_two_supported()) { out.set_level_two_supported(in.get_level_two_supported()); }
        if (in.get_level_three_supported()) { out.set_level_three_supported(in.get_level_three_supported()); }
        if (in.get_issuer_currency()) { out.set_issuer_currency(in.get_issuer_currency()); }
        if (in.get_combo_card()) { out.set_combo_card(in.get_combo_card()); }
        if (in.get_authentication()) { out.set_authentication(in.get_authentication()); }
        if (in.get_cost()) { out.set_cost(in.get_cost()); }
        if (in.get_issuer_supports_tokenization()) { out.set_issuer_supports_tokenization(in.get_issuer_supports_tokenization()); }
        if (in.get_billpay_enabled()) { out.set_billpay_enabled(in.get_billpay_enabled()); }
        if (in.get_ecom_enabled()) { out.set_ecom_enabled(in.get_ecom_enabled()); }
        if (in.get_flexible_credential_supported()) { out.set_flexible_credential_supported(in.get_flexible_credential_supported()); }
        if (in.get_card_subtype_code()) { out.set_card_subtype_code(in.get_card_subtype_code()); }
        if (in.get_multi_account_access_indicator()) { out.set_multi_account_access_indicator(in.get_multi_account_access_indicator()); }
        return out;
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        AppError error
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

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::cards_info
