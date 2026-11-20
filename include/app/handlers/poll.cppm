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
 * @brief Local normalization of app::dto::poll::PollResponse for the wire.
 *
 * Upstream api_models::poll::PollStatus serializes snake_case
 * ("pending"/"completed"/"not_found"). rfl reflects the C++ enum by enumerator
 * name ("Pending"/...), so the typed DTO alone cannot produce the upstream
 * wire value. This shape keeps the field names/types typed and fixes the
 * status spelling without touching the shared DTO partition.
 */
class PollWireResponse
{
public:
    void set_poll_id(std::string value) { poll_id = std::move(value); }
    [[nodiscard]] const std::string& get_poll_id() const noexcept { return poll_id; }
    void set_status(std::string value) { status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return status; }

    std::string poll_id{};
    std::string status{};
};

class PollRouteHandlers
{
public:
    void handle_retrieve_poll_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream routes/poll.rs:37-50 selects SdkAuthorizationAuth when an
        // Authorization header is present, otherwise HeaderAuth(PublishableKeyAuth).
        // Both allow connected-scope operations and deny platform-self operations.
        // The registry rule supplies the operation's permission; the scheme is
        // resolved per request because the static entry is SDK-only.
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
        const bool sdk_auth = !req.find_header("authorization").empty();
        const auto rule = app::auth::RouteAuthRuleBuilder{}
                              .scheme(sdk_auth ? app::auth::AuthScheme::JWT
                                               : app::auth::AuthScheme::PUBLISHABLE_KEY)
                              .permission(base_rule.permission())
                              .allow_connected(true)
                              .allow_platform(false)
                              .build();
        const auto auth = m_context->authorizer().authorize_route(auth_ctx, rule);
        if (auth) {
            send_error(res, send, *auth);
            return;
        }

        // server_wrap(Flow::RetrievePollStatus, LockAction::NotApplicable):
        // the upstream flow records the flow name and takes no lock. app_metrics
        // has no Flow/LockAction object, so both facts are expressed here.
        m_context->metrics().record_flow("RetrievePollStatus");

        // poll_id arrives as the trailing :poll_id path segment of
        // /poll/status/{poll_id} (upstream web::Path<String>), not in the body.
        // Upstream's typed PollId DTO is just that one string; this port keeps
        // the same value without a separate wrapper type.
        const auto segments = app::http::RequestPath{}(req);
        const auto poll_segment = segments.count() == 0
                                      ? std::optional<std::string_view>{}
                                      : segments.at(segments.count() - 1);
        if (!poll_segment || poll_segment->empty()) {
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
        const std::string request_poll_id{*poll_segment};

        // Upstream core/poll.rs:23-25 scopes the redis key with the processor
        // merchant id: poll_{processor_merchant_id}_{poll_id}. No processor
        // domain object exists in this port, so the calling merchant identity
        // (explicit header, else auth context) is the deterministic substitute.
        std::string processor_merchant_id{app::http::merchant_id_from_header{}(req)};
        if (processor_merchant_id.empty()) {
            processor_merchant_id = auth_ctx.merchant_id();
        }
        const auto key = std::string{"poll_"} + processor_merchant_id + "_" + request_poll_id;

        // Local Cache has only hit/miss; it carries no Redis connection-error
        // channel. A miss maps to the upstream PollNotFound response below.
        const auto cached = m_context->cache().get(key);
        if (!cached.hit) {
            auto error = core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND,
                std::format("Poll not found: {}", request_poll_id)
            );
            error.set_kind_name("PollNotFound");
            send_error(res, send, std::move(error));
            return;
        }

        // Upstream core/poll.rs:39-42 parse_enum("PollStatus"): an unrecognized
        // stored value is an InternalServerError, never a silent default.
        const auto status_raw = cached.value_or<std::string>(std::string{});
        const auto status = PollStatusParser{}(status_raw);
        if (!status) {
            auto error = core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INTERNAL,
                std::format("invalid PollStatus value: {}", status_raw)
            );
            error.set_kind_name("InternalServerError");
            send_error(res, send, std::move(error));
            return;
        }

        // typed wire DTO: poll_id + snake_case status, same shape as upstream.
        PollWireResponse response;
        response.set_poll_id(request_poll_id);
        response.set_status(PollStatusWireName{}(*status));

        m_context->metrics().counter("poll_retrieve_total").increment();
        // ApiEventsType::Poll { poll_id } substitute: app_metrics has no typed
        // event, so a poll-id-scoped counter carries the same dimension.
        m_context->metrics().counter(std::format("poll_response.{}", request_poll_id)).increment();
        const auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
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

    explicit PollRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    /** @brief parse_enum("PollStatus") substitute: strict, no silent default. */
    class PollStatusParser
    {
    public:
        [[nodiscard]] std::optional<app::dto::poll::PollStatus>
        operator()(std::string_view raw) const noexcept
        {
            // snake_case is the upstream wire form; the PascalCase spellings
            // are accepted for compatibility with any local cache writer.
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
    };

    /** @brief Maps the typed status to its upstream snake_case wire spelling. */
    class PollStatusWireName
    {
    public:
        [[nodiscard]] std::string operator()(app::dto::poll::PollStatus status) const
        {
            switch (status) {
            case app::dto::poll::PollStatus::Pending:
                return "pending";
            case app::dto::poll::PollStatus::Completed:
                return "completed";
            case app::dto::poll::PollStatus::NotFound:
                return "not_found";
            }
            return "pending";
        }
    };

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

template<>
struct serde::Serializable<app::handlers::poll::PollWireResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"poll_id", &app::handlers::poll::PollWireResponse::get_poll_id, &app::handlers::poll::PollWireResponse::set_poll_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::handlers::poll::PollWireResponse::get_status, &app::handlers::poll::PollWireResponse::set_status, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "poll_response_wire"; }
};
