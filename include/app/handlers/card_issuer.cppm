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

class CardIssuerRouteHandlers
{
public:
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

        using Input = app::dto::card_issuer::CardIssuerRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const std::string issuer_name = typed->get_issuer_name().get_value();
        if (const auto invalid = validate_issuer_name(issuer_name)) {
            send_error(res, send, *invalid);
            return;
        }

        const std::string id = new_id();
        if (id.empty()) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "card issuer id generation failed"
            ));
            return;
        }

        // Upstream core/card_issuer.rs:21-38 builds NewCardIssuer with a generated id,
        // issuer_name, created_at and last_modified_at, then inserts through the store.
        // The local record carries created_at; last_modified_at is seeded identically.
        auto record = to_store_record(id, issuer_name);
        const auto stamp = now_stamp();
        record.set_created_at(stamp);
        record.set_metadata("{\"last_modified_at\":\"" + stamp + "\"}");

        auto repository = m_context->repository();
        auto created = repository.create_card_issuer(std::move(record));
        if (!created) {
            const auto& failure = created.error();
            if (failure.get_kind() == core::error::AppErrorKind::ALREADY_EXISTS) {
                // Upstream to_duplicate_response(ApiErrorResponse::GenericDuplicateError)
                // at core/card_issuer.rs:34-36.
                send_error(res, send, make_error(
                    core::error::AppErrorKind::ALREADY_EXISTS,
                    "A card issuer with this name already exists"
                ));
            } else {
                send_error(res, send, failure);
            }
            return;
        }

        auto response = response_from(*created);

        m_context->metrics().record_flow("AddCardIssuer");
        m_context->metrics().counter("CARD_ISSUER_CREATED").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

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

        const std::string id = id_from_request(req);
        if (id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND,
                "Card issuer not found"
            ));
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = app::dto::card_issuer::CardIssuerUpdateRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const std::string issuer_name = typed->get_issuer_name().get_value();
        if (const auto invalid = validate_issuer_name(issuer_name)) {
            send_error(res, send, *invalid);
            return;
        }

        auto repository = m_context->repository();
        // Upstream store.update_card_issuer maps a missing row through
        // to_not_found_response at core/card_issuer.rs:56-58.
        auto stored = repository.find_card_issuer_by_string(id);
        if (!stored) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "Card issuer with id " + id + " not found"
            ));
            return;
        }

        auto record = *stored;
        record.set_name(issuer_name);
        // Upstream UpdateCardIssuer sets last_modified_at (core/card_issuer.rs:47-50).
        // The local record has no dedicated column, so the stamp is persisted in metadata.
        record.set_metadata("{\"last_modified_at\":\"" + now_stamp() + "\"}");
        if (!repository.update_card_issuer(record)) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "Card issuer with id " + id + " not found"
            ));
            return;
        }

        auto response = response_from(record);

        m_context->metrics().record_flow("UpdateCardIssuer");
        m_context->metrics().counter("CARD_ISSUER_UPDATED").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

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

        std::string id = id_from_request(req);
        if (id.empty()) {
            const auto body = req.body_as_string();
            if (!body.empty()) {
                using Input = app::dto::card_issuer::CardIssuerDeleteRequest;
                if (auto typed = serde::Ser::deserialize<Input>("application/json", body)) {
                    id = typed->get_id().get_value();
                }
            }
        }
        if (id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND,
                "Card issuer not found"
            ));
            return;
        }

        auto repository = m_context->repository();
        auto stored = repository.find_card_issuer_by_string(id);
        if (!stored) {
            // Upstream to_not_found_response at core/card_issuer.rs:72-74.
            send_error(res, send, make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "Card issuer with id " + id + " not found"
            ));
            return;
        }

        // Upstream delete_card_issuer returns the store's is_deleted flag
        // (core/card_issuer.rs:68-81); the wire `deleted` now mirrors that result.
        const bool is_deleted = repository.remove_card_issuer(stored->get_id());
        if (!is_deleted) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "failed to delete card issuer"
            ));
            return;
        }

        m_context->metrics().record_flow("DeleteCardIssuer");
        m_context->metrics().counter("CARD_ISSUER_DELETED").increment();

        app::dto::card_issuer::CardIssuerIdValue response_id;
        response_id.set_value(id);
        app::dto::card_issuer::CardIssuerDeleteResponse response;
        response.set_id(std::move(response_id));
        response.set_deleted(is_deleted);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_list_card_issuers(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "list_card_issuers", true)) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto query = app::http::RequestQuery{}(req);
        const std::size_t max_limit = configured_list_limit();
        std::size_t limit = max_limit;
        if (const auto raw = query.get("limit")) {
            if (const auto parsed = parse_limit(*raw)) {
                limit = std::min(*parsed, max_limit);
            }
        }

        auto repository = m_context->repository();
        auto records = repository.list_card_issuers();
        if (records.size() >= max_limit) {
            // Upstream logs a warning when the configured limit is hit
            // (core/card_issuer.rs:103-106). No tracing sink is injected locally;
            // a named counter is the deterministic substitute.
            m_context->metrics().counter("CARD_ISSUER_LIST_LIMIT_REACHED").increment();
        }

        std::vector<app::dto::card_issuer::CardIssuerResponse> issuers;
        issuers.reserve(std::min(limit, records.size()));
        for (const auto& record : records) {
            if (issuers.size() >= limit) {
                break;
            }
            // Upstream ForeignTryInto failure maps to InvalidDataValue on
            // issuer_name (core/card_issuer.rs:108-117).
            if (const auto invalid = validate_issuer_name(record.get_name())) {
                send_error(res, send, *invalid);
                return;
            }
            issuers.push_back(response_from(record));
        }

        app::dto::card_issuer::CardIssuerListResponse response;
        response.set_issuers(std::move(issuers));

        m_context->metrics().record_flow("ListCardIssuers");
        m_context->metrics().counter("CARD_ISSUERS_LISTED").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/card_issuers"}
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
                Route{"/:id"}
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
                Route{"/:id"}
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
        // "card_issuer.list_max_limit", DB config key "card_issuer_list_max_limit",
        // default 18000 (core/configs/dimension_config.rs:1420-1430).
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
