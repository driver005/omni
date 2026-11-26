export module app_auth:unified_connector_service_result;

import std;
import core_serde;

export namespace app::handlers::unified_connector_service {

/** @brief Typed upstream KillSwitchStatusResponse wire shape. */
class KillSwitchStatusResult
{
public:
    void set_rollout_scope(std::string value)
    {
        m_rollout_scope = std::move(value);
    }

    [[nodiscard]] const std::string& get_rollout_scope() const noexcept
    {
        return m_rollout_scope;
    }

    void set_counter(std::uint64_t value) noexcept
    {
        m_counter = value;
    }

    [[nodiscard]] std::uint64_t get_counter() const noexcept
    {
        return m_counter;
    }

    void set_threshold(std::optional<std::uint64_t> value) noexcept
    {
        m_threshold = value;
    }

    [[nodiscard]] const std::optional<std::uint64_t>& get_threshold() const noexcept
    {
        return m_threshold;
    }

    void set_decline_counter(std::uint64_t value) noexcept
    {
        m_decline_counter = value;
    }

    [[nodiscard]] std::uint64_t get_decline_counter() const noexcept
    {
        return m_decline_counter;
    }

    void set_connector_decline_threshold(std::optional<std::uint64_t> value) noexcept
    {
        m_connector_decline_threshold = value;
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_connector_decline_threshold() const noexcept
    {
        return m_connector_decline_threshold;
    }

    void set_tripped(bool value) noexcept
    {
        m_tripped = value;
    }

    [[nodiscard]] bool get_tripped() const noexcept
    {
        return m_tripped;
    }

private:
    std::string m_rollout_scope{};
    std::uint64_t m_counter{0};
    std::optional<std::uint64_t> m_threshold{};
    std::uint64_t m_decline_counter{0};
    std::optional<std::uint64_t> m_connector_decline_threshold{};
    bool m_tripped{false};
};

/** @brief ApplicationResponse::StatusOk: typed empty-object document. */
class ResetKillSwitchResult
{
public:
    ResetKillSwitchResult() :
        m_document{serde::Value{serde::Value::Object{}}}
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

} // namespace app::handlers::unified_connector_service

// Reflect-cpp is wired to serde::Serializable<>::fields(), so the typed wire
// shape needs an explicit FieldDesc reflection to round-trip through
// Ser::serialize. Field names and order match the original
// KillSwitchStatusResponse exactly, including JSON null for absent thresholds.
template<>
struct serde::Serializable<::app::handlers::unified_connector_service::KillSwitchStatusResult>
{
    using Response = ::app::handlers::unified_connector_service::KillSwitchStatusResult;

    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "rollout_scope",
                &Response::get_rollout_scope,
                &Response::set_rollout_scope,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "counter",
                &Response::get_counter,
                &Response::set_counter,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "threshold",
                &Response::get_threshold,
                &Response::set_threshold,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "decline_counter",
                &Response::get_decline_counter,
                &Response::set_decline_counter,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_decline_threshold",
                &Response::get_connector_decline_threshold,
                &Response::set_connector_decline_threshold,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tripped",
                &Response::get_tripped,
                &Response::set_tripped,
                serde::FieldOptions{}>{}
        };
    }
};
