export module app_auth:verify_connector_result;

import std;
import core_serde;

export namespace app::handlers::verify_connector {

/**
 * @brief Upstream `RouterResponse<()>` / StatusOk (core/verify_connector.rs:23,60).
 *
 * Upstream returns HTTP 200 with no payload. The old handler wrote an empty raw
 * body with `send_json(res, send, {}, true)`. That empty body is now carried as a
 * typed empty `serde::Value` document, serialized by the handler through
 * `Ser::serialize` — no raw JSON strings and no raw passthrough.
 */
class PaymentConnectorVerifyResult
{
public:
    PaymentConnectorVerifyResult() = default;

    explicit PaymentConnectorVerifyResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{serde::Value::Object{}};
};

} // namespace app::handlers::verify_connector
