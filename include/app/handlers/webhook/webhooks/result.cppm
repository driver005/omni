export module app_webhook:webhooks_result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::webhooks {

/**
 * The acknowledgement stays the typed upstream `WebhookResponse<R>` mirror from
 * app/dto/webhooks.cppm. Its five variants (StatusOk / Json / TextPlain /
 * FileData / JsonWithHeaders) are carried as typed values: JSON bodies become a
 * decoded serde::Value document, plain text a typed string, file data a typed
 * numeric array. The handler serializes that value through Ser; only the ack's
 * headers and status are written directly onto the response.
 */
using WebhookAck = ::app::dto::webhooks::WebhookResponse;
using WebhookAckKind = ::app::dto::webhooks::WebhookResponseKind;
using AckKind = WebhookAckKind;

/**
 * @brief Typed body value for one WebhookAck. No hand-written JSON is kept:
 * raw JSON text is decoded into a serde::Value document before it can reach
 * the response, and non-JSON payloads are wrapped in their own typed value.
 */
class WebhookAckBody
{
public:
    WebhookAckBody() = default;

    explicit WebhookAckBody(const WebhookAck& ack)
    {
        switch (ack.kind()) {
            case AckKind::STATUS_OK:
                m_value = serde::Value{};
                break;
            case AckKind::FILE_DATA:
                {
                    serde::Value::Array bytes{};
                    bytes.reserve(ack.file_data().size());
                    for (const auto byte: ack.file_data()) {
                        bytes.push_back(serde::Value{static_cast<std::int64_t>(byte)});
                    }
                    m_value = serde::Value{std::move(bytes)};
                    break;
                }
            case AckKind::TEXT_PLAIN:
                m_value = serde::Value{ack.body()};
                break;
            case AckKind::JSON:
            case AckKind::JSON_WITH_HEADERS:
            default:
                {
                    auto decoded = serde::Ser::decode_generic("application/json", ack.body());
                    m_value = decoded ? std::move(*decoded) : serde::Value{};
                    break;
                }
        }
    }

    [[nodiscard]] const serde::Value& value() const noexcept
    {
        return m_value;
    }

private:
    serde::Value m_value{};
};

/** One Result per handle_* method. */
class ReceiveIncomingWebhookResult
{
public:
    ReceiveIncomingWebhookResult() = default;

    explicit ReceiveIncomingWebhookResult(WebhookAck ack) noexcept :
        m_ack{std::move(ack)},
        m_body{m_ack}
    {
    }

    [[nodiscard]] const WebhookAck& get_ack() const noexcept
    {
        return m_ack;
    }

    [[nodiscard]] WebhookAck& get_ack() noexcept
    {
        return m_ack;
    }

    void set_ack(WebhookAck value)
    {
        m_ack = std::move(value);
        m_body = WebhookAckBody{m_ack};
    }

    [[nodiscard]] const serde::Value& get_body() const noexcept
    {
        return m_body.value();
    }

private:
    WebhookAck m_ack{};
    WebhookAckBody m_body{};
};

class ReceiveIncomingWebhookV2Result
{
public:
    ReceiveIncomingWebhookV2Result() = default;

    explicit ReceiveIncomingWebhookV2Result(WebhookAck ack) noexcept :
        m_ack{std::move(ack)},
        m_body{m_ack}
    {
    }

    [[nodiscard]] const WebhookAck& get_ack() const noexcept
    {
        return m_ack;
    }

    [[nodiscard]] WebhookAck& get_ack() noexcept
    {
        return m_ack;
    }

    void set_ack(WebhookAck value)
    {
        m_ack = std::move(value);
        m_body = WebhookAckBody{m_ack};
    }

    [[nodiscard]] const serde::Value& get_body() const noexcept
    {
        return m_body.value();
    }

private:
    WebhookAck m_ack{};
    WebhookAckBody m_body{};
};

class ReceiveNetworkTokenResult
{
public:
    ReceiveNetworkTokenResult() = default;

    explicit ReceiveNetworkTokenResult(WebhookAck ack) noexcept :
        m_ack{std::move(ack)},
        m_body{m_ack}
    {
    }

    [[nodiscard]] const WebhookAck& get_ack() const noexcept
    {
        return m_ack;
    }

    [[nodiscard]] WebhookAck& get_ack() noexcept
    {
        return m_ack;
    }

    void set_ack(WebhookAck value)
    {
        m_ack = std::move(value);
        m_body = WebhookAckBody{m_ack};
    }

    [[nodiscard]] const serde::Value& get_body() const noexcept
    {
        return m_body.value();
    }

private:
    WebhookAck m_ack{};
    WebhookAckBody m_body{};
};

} // namespace app::handlers::webhooks
