export module app_handlers_health:result;

import std;
import core_serde;

export namespace app::handlers::health {

/**
 * HttpResponse::Ok().body("health is good") — the plain-text health body is
 * held as a typed string value and encoded through Ser, with text/plain kept
 * as the response content type by the handler.
 */
class HealthResult
{
public:
    HealthResult() = default;

    explicit HealthResult(std::string text) noexcept :
        m_value{serde::Value{std::move(text)}}
    {
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
    serde::Value m_value{};
};

/**
 * RouterHealthCheckResponse document. Optional `vault` /
 * `unified_connector_service` fields are omitted when unset and
 * `grpc_health_check` holds dynamic service keys, so the typed carrier is a
 * serde::Value document built from typed field assignments.
 */
class DeepHealthCheckResult
{
public:
    DeepHealthCheckResult() = default;

    explicit DeepHealthCheckResult(serde::Value document) noexcept :
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
    serde::Value m_document{};
};

} // namespace app::handlers::health
