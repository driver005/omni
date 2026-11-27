export module app_webhook:recovery_webhooks_result;

import std;
import interfaces;
import core_behavior;
import core_serde;
import core_error;

export namespace app::handlers::recovery_webhooks {

/**
 * Upstream `WebhookResponse<R>` (api.rs:33-39): wire kind + typed body value.
 * The body is a `serde::Value` built from typed fields in the runtime, never a
 * hand-written JSON/text string; the handler encodes it with `Ser::serialize`.
 */
class WebhookResponse
{
public:
    enum class Kind
    {
        StatusOk,
        Json,
        TextPlain,
    };

    WebhookResponse() = default;

    WebhookResponse(Kind kind, serde::Value value) noexcept :
        m_kind{kind},
        m_value{std::move(value)}
    {
    }

    [[nodiscard]] Kind get_kind() const noexcept
    {
        return m_kind;
    }

    void set_kind(Kind kind) noexcept
    {
        m_kind = kind;
    }

    [[nodiscard]] const serde::Value& get_value() const noexcept
    {
        return m_value;
    }

    void set_value(serde::Value value)
    {
        m_value = std::move(value);
    }

private:
    Kind m_kind{Kind::StatusOk};
    serde::Value m_value{};
};

} // namespace app::handlers::recovery_webhooks

export template<>
class core::behavior::Outcome<app::handlers::recovery_webhooks::WebhookResponse> :
    public std::expected<app::handlers::recovery_webhooks::WebhookResponse, core::error::AppError>
{
public:
    using std::expected<
        app::handlers::recovery_webhooks::WebhookResponse,
        core::error::AppError>::expected;

    void handle(interfaces::io::ResponseSink& sink)
    {
        if (!has_value()) {
            error().send(sink);
            return;
        }
        using Response = app::handlers::recovery_webhooks::WebhookResponse;
        sink.set_status(interfaces::io::types::Status::OK);
        if (value().get_kind() == Response::Kind::StatusOk) {
            sink.set_body(std::vector<std::byte>{});
        } else {
            sink.set_body(serde::Ser::serialize("application/json", value().get_value()));
        }
        sink.send();
    }
};
