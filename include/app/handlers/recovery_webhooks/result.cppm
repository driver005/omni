export module app_handlers_recovery_webhooks:result;

import std;
import core_serde;

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

/** One Result per handle_* method. */
struct RecoveryReceiveIncomingWebhookResult
{
    WebhookResponse response{};
};

} // namespace app::handlers::recovery_webhooks
