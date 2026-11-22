module;
export module app_handlers:card_issuer;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_services;

export namespace app::handlers::card_issuer {

/**
 * Card issuer route handlers.
 *
 * The four public `handle_*` methods are local ports of the upstream route
 * wrappers in `docs/hyperswitch/crates/router/src/routes/card_issuer.rs`
 * (`add_card_issuer`, `update_card_issuer`, `delete_card_issuer`,
 * `list_card_issuers`). Each one performs the route-layer concerns that upstream
 * delegates to `api::server_wrap`: auth, JSON body extraction, error dispatch
 * and response emission.
 *
 * The private `core_*` methods are direct ports of the upstream core functions
 * in `docs/hyperswitch/crates/router/src/core/card_issuer.rs` (same order, same
 * control flow, same store calls, same error mapping and wire shapes):
 *   - core_add_card_issuer      <- core/card_issuer.rs:16-39
 *   - core_update_card_issuer   <- core/card_issuer.rs:41-61
 *   - core_delete_card_issuer   <- core/card_issuer.rs:63-82
 *   - core_list_card_issuers    <- core/card_issuer.rs:84-119
 *
 * Upstream external infrastructure (Diesel/Postgres, tracing, superposition)
 * is substituted with in-process equivalents supplied by `AppContext`:
 * repository (store), metrics (instrumentation), superposition/config_store
 * (dimension config) and ids (id generation). No behaviour is simulated beyond
 * these infrastructure boundaries.
 */
class CardIssuerRouteHandlers
{
public:
    /** @brief Port of routes/card_issuer.rs:12-30 `add_card_issuer` (AdminApiAuth). */
    void handle_add_card_issuer(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "add_card_issuer")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        // Upstream `web::Json<api_types::CardIssuerRequest>` extraction.
        using Input = app::dto::card_issuer::CardIssuerRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto result = core_add_card_issuer(std::move(*typed));
        if (!result) {
            send_error(res, send, std::move(result.error()));
            return;
        }

        // `#[instrument(skip_all, fields(flow = Flow::AddCardIssuer))]` substitute.
        m_context->metrics().record_flow("AddCardIssuer");
        m_context->metrics().counter("CARD_ISSUER_CREATED").increment();

        auto text = serde::Ser::encode_json(*result);
        send_json(res, send, text, true);
    }

    /** @brief Port of routes/card_issuer.rs:32-52 `update_card_issuer` (AdminApiAuth). */
    void handle_update_card_issuer(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "update_card_issuer")) {
            send_error(res, send, *auth_error);
            return;
        }

        // Upstream `web::Path<id_type::CardIssuerId>` extraction (path id only).
        const std::string id = id_from_request(req);

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        // Upstream `web::Json<api_types::CardIssuerUpdateRequest>` extraction.
        using Input = app::dto::card_issuer::CardIssuerUpdateRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto result = core_update_card_issuer(id, std::move(*typed));
        if (!result) {
            send_error(res, send, std::move(result.error()));
            return;
        }

        // `#[instrument(skip_all, fields(flow = Flow::UpdateCardIssuer))]` substitute.
        m_context->metrics().record_flow("UpdateCardIssuer");
        m_context->metrics().counter("CARD_ISSUER_UPDATED").increment();

        auto text = serde::Ser::encode_json(*result);
        send_json(res, send, text, true);
    }

    /** @brief Port of routes/card_issuer.rs:54-74 `delete_card_issuer` (AdminApiAuth). */
    void handle_delete_card_issuer(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "delete_card_issuer")) {
            send_error(res, send, *auth_error);
            return;
        }

        // Upstream builds `CardIssuerDeleteRequest { id: path.into_inner() }` —
        // the id comes from the path only (no body fallback upstream).
        const std::string id = id_from_request(req);

        auto result = core_delete_card_issuer(id);
        if (!result) {
            send_error(res, send, std::move(result.error()));
            return;
        }

        // `#[instrument(skip_all, fields(flow = Flow::DeleteCardIssuer))]` substitute.
        m_context->metrics().record_flow("DeleteCardIssuer");
        m_context->metrics().counter("CARD_ISSUER_DELETED").increment();

        auto text = serde::Ser::encode_json(*result);
        send_json(res, send, text, true);
    }

    /**
     * @brief Port of routes/card_issuer.rs:76-100 `list_card_issuers`
     * (`auth_type(HeaderAuth(ApiKeyAuth), JWTAuth(MerchantAccountRead), headers)`).
     */
    void handle_list_card_issuers(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // The `list_jwt_fallback` flag models upstream `auth_type`: the
        // ApiKeyAuth branch (default) OR the JWTAuth(MerchantAccountRead) branch,
        // both with allow_connected:false and allow_platform:false.
        if (const auto auth_error = authorize(req, "list_card_issuers", true)) {
            send_error(res, send, *auth_error);
            return;
        }

        auto result = core_list_card_issuers();
        if (!result) {
            send_error(res, send, std::move(result.error()));
            return;
        }

        // `#[instrument(skip_all, fields(flow = Flow::ListCardIssuers))]` substitute.
        m_context->metrics().record_flow("ListCardIssuers");
        m_context->metrics().counter("CARD_ISSUERS_LISTED").increment();

        auto text = serde::Ser::encode_json(*result);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Upstream app.rs:2093-2105: scope "/card_issuers" with resource "" (POST/GET)
        // and resource "/{id}" (PUT/DELETE). `delt()` is the router's DELETE binding
        // (core/router/builder.cppm:540-548), the local equivalent of `web::delete()`.
        Router{router, "card_issuers"}
            .add_route(
                Route{""}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_add_card_issuer(req, res, std::move(send));
                    })
                    .summary("Hyperswitch add_card_issuer")
            )
            .add_route(
                Route{""}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_card_issuers(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_card_issuers")
            )
            .add_route(
                Route{":id"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_update_card_issuer(req, res, std::move(send));
                    })
                    .summary("Hyperswitch update_card_issuer")
            )
            .add_route(
                Route{":id"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_delete_card_issuer(req, res, std::move(send));
                    })
                    .summary("Hyperswitch delete_card_issuer")
            );
    }

    explicit CardIssuerRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    // ── Direct ports of upstream core/card_issuer.rs ───────────────────────────

    /**
     * @brief core/card_issuer.rs:16-39 `add_card_issuer`.
     *
     * let now = date_time::now();
     * let new = NewCardIssuer { id: CardIssuerId::generate()?, issuer_name,
     *                           created_at: now, last_modified_at: now };
     * let issuer = store.insert_card_issuer(new)
     *     .to_duplicate_response(GenericDuplicateError { message })?;
     * Ok(Json(issuer.foreign_try_into()?))
     */
    [[nodiscard]] std::expected<app::dto::card_issuer::CardIssuerResponse, core::error::AppError>
    core_add_card_issuer(app::dto::card_issuer::CardIssuerRequest body) const
    {
        const std::string issuer_name = body.get_issuer_name().get_value();
        // CardIssuerName nutype validation (common_utils/new_type.rs:21-24):
        // serde rejects an empty or >255-char name with a 422 on the request path.
        if (const auto invalid = validate_issuer_name(issuer_name)) {
            return std::unexpected{*invalid};
        }

        // `CardIssuerId::generate()` (common_utils/id_type/card_issuer.rs:37-41)
        // -> 5-char id (consts.rs:283); failure maps to InternalServerError.
        const std::string id = new_id();
        if (id.empty()) {
            return std::unexpected{make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "card issuer id generation failed"
            )};
        }

        // created_at == last_modified_at == now. The in-process record stores
        // created_at in its column and last_modified_at in metadata (no dedicated
        // column); neither is part of the wire response.
        auto record = to_store_record(id, issuer_name);
        const auto stamp = now_stamp();
        record.set_created_at(stamp);
        record.set_metadata("{\"last_modified_at\":\"" + stamp + "\"}");

        auto repository = m_context->repository();
        auto created = repository.create_card_issuer(std::move(record));
        if (!created) {
            // core/card_issuer.rs:34-36 to_duplicate_response(GenericDuplicateError)
            if (created.error().get_kind() == core::error::AppErrorKind::ALREADY_EXISTS) {
                return std::unexpected{make_error(
                    core::error::AppErrorKind::ALREADY_EXISTS,
                    "A card issuer with this name already exists"
                )};
            }
            return std::unexpected{created.error()};
        }

        // ForeignTryInto<CardIssuer> (types/transformers.rs:3034-3047); name was
        // already validated on input, so conversion cannot fail here.
        return response_from(*created);
    }

    /**
     * @brief core/card_issuer.rs:41-61 `update_card_issuer`.
     *
     * let update = UpdateCardIssuer { issuer_name, last_modified_at: now() };
     * let issuer = store.update_card_issuer(id.clone(), update)
     *     .to_not_found_response(GenericNotFoundError { message })?;
     * Ok(Json(issuer.foreign_try_into()?))
     */
    [[nodiscard]] std::expected<app::dto::card_issuer::CardIssuerResponse, core::error::AppError>
    core_update_card_issuer(std::string_view id, app::dto::card_issuer::CardIssuerUpdateRequest body) const
    {
        const std::string issuer_name = body.get_issuer_name().get_value();
        if (const auto invalid = validate_issuer_name(issuer_name)) {
            return std::unexpected{*invalid};
        }

        // core/card_issuer.rs:56-58 to_not_found_response message.
        const auto not_found = [this, &id]()
        {
            return make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "Card issuer with id " + std::string{id} + " not found"
            );
        };
        if (id.empty()) {
            return std::unexpected{not_found()};
        }

        auto repository = m_context->repository();
        auto stored = repository.find_card_issuer_by_string(id);
        if (!stored) {
            return std::unexpected{not_found()};
        }

        auto record = *stored;
        record.set_name(issuer_name);
        // UpdateCardIssuer.last_modified_at (core/card_issuer.rs:47-50) is kept
        // in metadata in the in-process record (no dedicated column locally).
        record.set_metadata("{\"last_modified_at\":\"" + now_stamp() + "\"}");
        if (!repository.update_card_issuer(record)) {
            return std::unexpected{not_found()};
        }

        return response_from(record);
    }

    /**
     * @brief core/card_issuer.rs:63-82 `delete_card_issuer`.
     *
     * let is_deleted = store.delete_card_issuer(id.clone())
     *     .to_not_found_response(GenericNotFoundError { message })?;
     * Ok(Json(CardIssuerDeleteResponse { id, deleted: is_deleted }))
     */
    [[nodiscard]] std::expected<app::dto::card_issuer::CardIssuerDeleteResponse, core::error::AppError>
    core_delete_card_issuer(std::string_view id) const
    {
        // core/card_issuer.rs:72-74 to_not_found_response message.
        const auto not_found = [this, &id]()
        {
            return make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "Card issuer with id " + std::string{id} + " not found"
            );
        };
        if (id.empty()) {
            return std::unexpected{not_found()};
        }

        auto repository = m_context->repository();
        auto stored = repository.find_card_issuer_by_string(id);
        if (!stored) {
            return std::unexpected{not_found()};
        }

        // The store's `is_deleted` result drives the wire `deleted` flag; a
        // failed delete surfaces as InternalServerError (core/card_issuer.rs:68-81).
        const bool is_deleted = repository.remove_card_issuer(stored->get_id());
        if (!is_deleted) {
            return std::unexpected{make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "failed to delete card issuer"
            )};
        }

        app::dto::card_issuer::CardIssuerIdValue response_id;
        response_id.set_value(std::string{id});
        app::dto::card_issuer::CardIssuerDeleteResponse response;
        response.set_id(std::move(response_id));
        response.set_deleted(is_deleted);
        return response;
    }

    /**
     * @brief core/card_issuer.rs:84-119 `list_card_issuers`.
     *
     * let dimensions = DimensionsGlobal::new();
     * let limit = dimensions.get_card_issuer_list_max_limit(store, superposition, None).await;
     * let issuers = store.list_card_issuers(limit).await.map_err(internal_server_error)?;
     * warn when issuers.len() >= limit;
     * issuers.into_iter().map(ForeignTryInto::foreign_try_into)
     *        .collect::<Result<Vec<_>,_>>()
     *        .map_err(|_| InvalidDataValue { field_name: "issuer_name" })?
     */
    [[nodiscard]] std::expected<app::dto::card_issuer::CardIssuerListResponse, core::error::AppError>
    core_list_card_issuers() const
    {
        const std::size_t limit = configured_list_limit();

        auto repository = m_context->repository();
        auto records = repository.list_card_issuers();

        // core/card_issuer.rs:103-106 warn!("Card issuer list hit the configured limit").
        // No tracing sink is injected locally; a named counter is the deterministic
        // in-process substitute.
        if (records.size() >= limit) {
            m_context->metrics().counter("CARD_ISSUER_LIST_LIMIT_REACHED").increment();
        }

        std::vector<app::dto::card_issuer::CardIssuerResponse> issuers;
        issuers.reserve(std::min(limit, records.size()));
        for (const auto& record : records) {
            // Upstream asks the store for at most `limit` rows; the local store
            // returns all rows, so the same bound is applied here.
            if (issuers.size() >= limit) {
                break;
            }
            // ForeignTryInto<CardIssuerListItem> (types/transformers.rs:3018-3032):
            // a bad stored issuer_name maps to InvalidDataValue { "issuer_name" },
            // which upstream renders as 400 "Invalid value provided: issuer_name".
            if (validate_issuer_name(record.get_name())) {
                return std::unexpected{make_error(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "Invalid value provided: issuer_name"
                )};
            }
            issuers.push_back(response_from(record));
        }

        app::dto::card_issuer::CardIssuerListResponse response;
        response.set_issuers(std::move(issuers));
        return response;
    }

    // ── Shared helpers ─────────────────────────────────────────────────────────

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        bool list_jwt_fallback = false
    ) const
    {
        const auto context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for(operation);
        auto auth = m_context->authorizer().authorize_route(context, rule);
        if (auth && list_jwt_fallback) {
            // Upstream routes/card_issuer.rs:85-96: list accepts
            // HeaderAuth(ApiKeyAuth{allow_connected_scope_operation:false,
            // allow_platform_self_operation:false}) OR
            // JWTAuth(MerchantAccountRead, allow_connected:false, allow_platform:false).
            const auto jwt_rule = app::auth::RouteAuthRuleBuilder{}
                                      .scheme(app::auth::AuthScheme::JWT)
                                      .permission(app::auth::AppPermission::MERCHANT_READ)
                                      .allow_connected(false)
                                      .allow_platform(false)
                                      .build();
            auth = m_context->authorizer().authorize_route(context, jwt_rule);
        }
        return auth;
    }

    [[nodiscard]] core::error::AppError make_error(
        core::error::AppErrorKind kind,
        std::string message
    ) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    [[nodiscard]] std::optional<core::error::AppError> validate_issuer_name(
        std::string_view name
    ) const
    {
        // CardIssuerName is a nutype validated to 1..=255 chars
        // (common_utils/new_type.rs:21-24, consts.rs:157).
        if (name.empty() || name.size() > 255) {
            return make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "InvalidDataValue: issuer_name"
            );
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string id_from_request(interfaces::io::IRequest& req) const
    {
        return std::string{app::http::RequestPath{}(req).at(1).value_or("")};
    }

    [[nodiscard]] std::string new_id() const
    {
        // Upstream id_type::CardIssuerId::generate() emits a 5-character nanoid
        // (common_utils/id_type/card_issuer.rs:31-35, consts.rs:283). The local id
        // keeps that length and alphanumeric shape using the injected id generator.
        const std::string raw = m_context->ids().uuid_v4();
        std::string out;
        for (const char c : raw) {
            if (c != '-') {
                out.push_back(c);
                if (out.size() == 5) {
                    break;
                }
            }
        }
        return out;
    }

    [[nodiscard]] std::string now_stamp() const
    {
        return std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
    }

    [[nodiscard]] std::optional<std::size_t> parse_limit(std::string_view value) const
    {
        std::int64_t parsed{};
        const auto converted = std::from_chars(value.data(), value.data() + value.size(), parsed);
        if (converted.ec == std::errc{} && converted.ptr == value.data() + value.size() && parsed > 0) {
            return static_cast<std::size_t>(parsed);
        }
        return std::nullopt;
    }

    [[nodiscard]] std::size_t configured_list_limit() const
    {
        // Upstream dimension_state::CardIssuerListMaxLimit: superposition key
        // "CARD_ISSUER_LIST_MAX_LIMIT", DB config key "card_issuer_list_max_limit",
        // default 18000 (core/configs/dimension_config.rs:1419-1429).
        constexpr std::size_t default_limit = 18'000;
        if (const auto configured = m_context->superposition().get_config("card_issuer", "list_max_limit")) {
            return parse_limit(configured->get_value()).value_or(default_limit);
        }
        if (const auto configured = m_context->config_store().get_value("configs", "card_issuer.list_max_limit")) {
            return parse_limit(*configured).value_or(default_limit);
        }
        if (const auto configured = m_context->config_store().get_value("configs", "card_issuer_list_max_limit")) {
            return parse_limit(*configured).value_or(default_limit);
        }
        return default_limit;
    }

    [[nodiscard]] app::store::card_issuers::CardIssuerRecord to_store_record(
        std::string_view id,
        std::string_view name
    ) const
    {
        app::store::card_issuers::CardIssuerRecord record;
        record.set_issuer_id(std::string{id});
        record.set_name(std::string{name});
        record.set_issuer_type("");
        record.set_status("");
        record.set_metadata("{}");
        record.set_created_at(now_stamp());
        return record;
    }

    [[nodiscard]] app::dto::card_issuer::CardIssuerResponse response_from(
        const app::store::card_issuers::CardIssuerRecord& record
    ) const
    {
        app::dto::card_issuer::CardIssuerIdValue id;
        id.set_value(record.get_issuer_id());
        app::dto::card_issuer::CardIssuerNameValue name;
        name.set_value(record.get_name());
        app::dto::card_issuer::CardIssuerResponse response;
        response.set_id(std::move(id));
        response.set_issuer_name(std::move(name));
        return response;
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

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::card_issuer
