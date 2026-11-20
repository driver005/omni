module;
export module app_handlers:poll;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import app_http;
import app_metrics;
import core_cache;
import core_error;

export namespace app::handlers::poll {

/**
 * Local substitutes for the upstream Cargo features that gate the poll route.
 * Upstream app.rs:2472 registers the scope only under
 * `#[cfg(all(feature = "oltp", feature = "v1"))]`. This port has no Cargo
 * `#[cfg]`, so these constexpr gates are the exact local equivalent. Both
 * features are compiled in here, matching an upstream release build.
 */
inline constexpr bool kOltp = true;
inline constexpr bool kV1 = true;

/**
 * Strict upstream-identical port of routes/poll.rs + core/poll.rs with local
 * in-process substitutes for external infrastructure (Redis, processor domain
 * object, ApiEventMetric). Wire shape, auth, errors, key derivation and control
 * flow mirror upstream; see docs/hyperswitch-parity/poll.md for file:line
 * evidence per helper.
 */
class PollRouteHandlers
{
public:
    void handle_retrieve_poll_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // routes/poll.rs:27,33,51-52 — Flow::RetrievePollStatus from the
        // #[instrument] field and the api::server_wrap call. LockAction is
        // NotApplicable (routes/poll.rs:60), so no lock is acquired locally.
        m_context->metrics().record_flow("RetrievePollStatus");

        // routes/poll.rs:37-50 — dynamic auth extractor selection: an
        // Authorization header selects SdkAuthorizationAuth, otherwise
        // HeaderAuth(PublishableKeyAuth). Local equivalents are AuthScheme::SDK
        // (parses the key=value Authorization header into SDK claims) and
        // AuthScheme::PUBLISHABLE_KEY (reads the publishable-key header). Both
        // allow connected-scope operations and deny platform-self operations.
        const auto auth_ctx = app::auth::AuthContextBuilder{}(req);
        const auto base_rule = m_context->auth_rules().rule_for("retrieve_poll_status");
        if (base_rule.scheme() == app::auth::AuthScheme::FORBIDDEN) {
            send_error(
                res,
                send,
                core::error::AppErrorBuilder{}.operator()(
                    core::error::AppErrorKind::FORBIDDEN,
                    "operation is not registered in the auth rule registry"
                )
            );
            return;
        }
        const bool has_authorization = !req.find_header("authorization").empty();
        const auto rule = app::auth::RouteAuthRuleBuilder{}
                              .scheme(has_authorization ? app::auth::AuthScheme::SDK
                                                        : app::auth::AuthScheme::PUBLISHABLE_KEY)
                              .permission(base_rule.permission())
                              .allow_connected(true)
                              .allow_platform(false)
                              .build();
        if (const auto denied = m_context->authorizer().authorize_route(auth_ctx, rule)) {
            send_error(res, send, *denied);
            return;
        }

        // routes/poll.rs:31,34-36 — web::Path<String> wrapped into PollId.
        // types/api/poll.rs:1-4 defines that single-field input DTO.
        const auto poll_id = decode_poll_id(req);
        if (!poll_id) {
            send_error(
                res,
                send,
                core::error::AppErrorBuilder{}.operator()(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "missing poll id"
                )
            );
            return;
        }
        const std::string& request_poll_id = poll_id->get_poll_id();

        // routes/poll.rs:56-57 — auth.platform.get_processor().clone() supplies
        // the processor passed to core::poll::retrieve_poll_status. Its account
        // id is resolved by resolve_processor_account_id below.
        const auto processor_merchant_id = resolve_processor_account_id(req, auth_ctx);

        // core/poll.rs:11-48 — the complete core read path.
        const auto outcome = retrieve_poll_status(request_poll_id, processor_merchant_id);
        if (!outcome.status) {
            send_upstream_error(res, send, outcome.error);
            return;
        }

        // api_models/poll.rs:5-28 — PollResponse wire shape (snake_case status)
        // plus the ApiEventMetric::Poll { poll_id } event dimension.
        const auto wire = encode_poll_response(request_poll_id, *outcome.status);
        m_context->metrics().counter("poll_retrieve_total").increment();
        m_context->metrics().counter(std::format("poll_response.{}", request_poll_id)).increment();
        const auto text = serde::Ser::encode_json(wire);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // app.rs:2472-2480 — web::scope("/poll") +
        // web::resource("/status/{poll_id}").route(web::get()), gated by
        // all(feature = "oltp", feature = "v1"). No C++-only routes exist.
        if constexpr (kOltp && kV1) {
            Router{router, "/poll"}
                .add_route(
                    Route{"/status/:poll_id"}
                        .get([this](
                                 interfaces::io::IRequest& req,
                                 interfaces::io::IResponse& res,
                                 std::function<void()> send
                             ) {
                            handle_retrieve_poll_status(req, res, std::move(send));
                        })
                        .summary("Hyperswitch retrieve_poll_status")
                        .description(
                            "Retrieve poll status. Auth: SDK authorization when an "
                            "Authorization header is present, otherwise publishable key "
                            "(connected scope allowed, platform self denied). "
                            "404 PollNotFound when the processor-scoped poll key is absent."
                        )
                );
        }
    }

    explicit PollRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    /** @brief Upstream RetrieveError variants surfaced by the core read path. */
    enum class RetrieveError
    {
        NONE,
        POLL_NOT_FOUND,
        INTERNAL_SERVER_ERROR,
    };

    struct RetrieveOutcome
    {
        std::optional<app::dto::poll::PollStatus> status{};
        RetrieveError error{RetrieveError::NONE};
    };

    /** @brief web::Path<String> -> PollId (routes/poll.rs:31,34-36). */
    [[nodiscard]] std::optional<app::dto::poll::PollId>
    decode_poll_id(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        if (segments.count() == 0) {
            return std::nullopt;
        }
        const auto segment = segments.at(segments.count() - 1);
        if (!segment || segment->empty()) {
            return std::nullopt;
        }
        app::dto::poll::PollId id;
        id.set_poll_id(std::string{*segment});
        return id;
    }

    /**
     * @brief auth.platform.get_processor() -> processor.get_account().get_id()
     * (routes/poll.rs:57, core/poll.rs:25).
     *
     * No processor domain object exists in this port, so the authenticated
     * merchant id (falling back to the request header) is canonicalized through
     * the merchant-account repository — the same local substitute used for
     * `provider.get_account().get_id()` in tokenization.cppm:250-258.
     */
    [[nodiscard]] std::string resolve_processor_account_id(
        interfaces::io::IRequest& req,
        const app::auth::AuthContext& auth_ctx
    ) const
    {
        std::string merchant_id{auth_ctx.merchant_id()};
        if (merchant_id.empty()) {
            merchant_id = std::string{app::http::merchant_id_from_header{}(req)};
        }
        if (const auto account = m_context->repository().find_merchant_account_by_string(merchant_id)) {
            merchant_id = account->get_merchant_id();
        }
        return merchant_id;
    }

    /**
     * @brief Port of super::utils::get_poll_id (core/poll.rs:24-25,
     * utils.rs:2452-2457): prefix "poll_{processor_merchant_id}_" to the
     * free-form request poll id.
     */
    [[nodiscard]] std::string get_poll_id(
        std::string_view processor_merchant_id,
        std::string_view unique_id
    ) const
    {
        return std::string{"poll_"} + std::string{processor_merchant_id} + "_" + std::string{unique_id};
    }

    /**
     * @brief Full port of core::poll::retrieve_poll_status (core/poll.rs:11-48).
     *
     *  - get_redis_conn failure -> InternalServerError with
     *    "Failed to get redis connection" (core/poll.rs:17-21): the local
     *    in-process Cache must be open; a closed cache is the connection error.
     *  - get_key fetch failure -> InternalServerError (core/poll.rs:26-35):
     *    the local cache has no transport failure channel, so this path is
     *    unreachable; a miss maps to PollNotFound (core/poll.rs:36-38).
     *  - parse_enum("PollStatus") failure -> InternalServerError
     *    (core/poll.rs:39-42), handled by parse_poll_status.
     */
    [[nodiscard]] RetrieveOutcome
    retrieve_poll_status(std::string_view request_poll_id, std::string_view processor_merchant_id) const
    {
        if (!m_context->cache().is_open()) {
            return {std::nullopt, RetrieveError::INTERNAL_SERVER_ERROR};
        }

        const auto key = get_poll_id(processor_merchant_id, request_poll_id);
        const auto cached = m_context->cache().get(key);
        if (!cached.hit) {
            return {std::nullopt, RetrieveError::POLL_NOT_FOUND};
        }

        const auto raw = cached.value_or<std::string>(std::string{});
        const auto status = parse_poll_status(raw);
        if (!status) {
            return {std::nullopt, RetrieveError::INTERNAL_SERVER_ERROR};
        }
        return {status, RetrieveError::NONE};
    }

    /**
     * @brief StringExt::parse_enum("PollStatus") (core/poll.rs:39-42).
     * snake_case is the upstream wire form; PascalCase is accepted only to
     * tolerate local cache writers. Unknown values never default silently.
     */
    [[nodiscard]] std::optional<app::dto::poll::PollStatus>
    parse_poll_status(std::string_view raw) const noexcept
    {
        if (raw == "pending" || raw == "Pending") {
            return app::dto::poll::PollStatus::Pending;
        }
        if (raw == "completed" || raw == "Completed") {
            return app::dto::poll::PollStatus::Completed;
        }
        if (raw == "not_found" || raw == "NotFound" || raw == "NOT_FOUND") {
            return app::dto::poll::PollStatus::NotFound;
        }
        return std::nullopt;
    }

    /** @brief PollResponse { poll_id, status } with snake_case status. */
    [[nodiscard]] app::dto::poll::PollResponseWire
    encode_poll_response(std::string_view poll_id, app::dto::poll::PollStatus status) const
    {
        app::dto::poll::PollResponseWire wire;
        wire.set_poll_id(std::string{poll_id});
        wire.set_status(std::string{app::dto::poll::PollStatusWireName{}(status)});
        return wire;
    }

    /**
     * Exact ApiErrorResponse wire envelopes from
     * hyperswitch_domain_models/errors/api_error_response.rs:118,480 and
     * api_models/errors/types.rs:40-70,159-176:
     *  - PollNotFound -> 404 {"type":"invalid_request","message":"Poll does not
     *    exist in our records","code":"HE_02"} (error_type "invalid_request",
     *    code HE_02, id discarded by the AER conversion).
     *  - InternalServerError -> 500 {"type":"api","message":"Something went
     *    wrong","code":"HE_00"} (error_type "api", code HE_00).
     */
    void send_upstream_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        RetrieveError error
    ) const
    {
        std::string_view body{};
        interfaces::io::types::Status status{interfaces::io::types::Status::INTERNAL_SERVER_ERROR};
        switch (error) {
        case RetrieveError::POLL_NOT_FOUND:
            body = R"({"type":"invalid_request","message":"Poll does not exist in our records","code":"HE_02"})";
            status = interfaces::io::types::Status::NOT_FOUND;
            break;
        case RetrieveError::INTERNAL_SERVER_ERROR:
        case RetrieveError::NONE:
            body = R"({"type":"api","message":"Something went wrong","code":"HE_00"})";
            break;
        }
        auto bytes = serde::Ser::serialize_raw("application/json", body);
        res.set_status(status);
        res.set_body(std::move(bytes));
        send();
    }

    /** @brief Framework-level error envelope for auth/malformed-path denials. */
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

} // namespace app::handlers::poll
