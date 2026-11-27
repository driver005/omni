export module app_webhook:webhooks_result;

import std;
import interfaces;
import core_behavior;
import core_serde;
import core_error;
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

} // namespace app::handlers::webhooks

// The webhook ack has its own wire shape (empty 200 / header-bearing variants),
// so it publishes itself instead of going through the generic DTO serializer.
export template<>
class core::behavior::Outcome<app::handlers::webhooks::WebhookAck> :
    public std::expected<app::handlers::webhooks::WebhookAck, core::error::AppError>
{
public:
    using std::expected<app::handlers::webhooks::WebhookAck, core::error::AppError>::expected;

    void handle(interfaces::io::ResponseSink& sink)
    {
        if (!has_value()) {
            error().send(sink);
            return;
        }
        const auto& ack = value();
        using app::handlers::webhooks::AckKind;
        if (ack.kind() == AckKind::STATUS_OK) {
            sink.set_status(interfaces::io::types::Status::OK);
            sink.set_body(std::vector<std::byte>{});
            sink.send();
            return;
        }
        if (ack.kind() == AckKind::FILE_DATA || ack.kind() == AckKind::JSON_WITH_HEADERS) {
            for (const auto& [name, header_value]: ack.headers()) {
                sink.add_header(name, header_value);
            }
            if (ack.kind() == AckKind::FILE_DATA) {
                sink.add_header("content-type", ack.mime());
            }
        }
        sink.set_status(interfaces::io::types::Status::OK);
        const auto body = app::handlers::webhooks::WebhookAckBody{ack};
        sink.set_body(serde::Ser::serialize("application/json", body.value()));
        sink.send();
    }
};
