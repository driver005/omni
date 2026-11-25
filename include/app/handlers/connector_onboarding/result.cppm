module;
#include <rfl/to_generic.hpp>

export module app_handlers_connector_onboarding:result;

import std;
import core_serde;

export namespace app::handlers::connector_onboarding {

/** @brief Builds typed dynamic response documents; no hand-written JSON strings. */
class ResponseDocument
{
public:
    [[nodiscard]] static serde::Value empty() noexcept
    {
        return serde::Value{serde::Value::Object{}};
    }

    template<typename T>
    [[nodiscard]] static serde::Value from(const T& value)
    {
        return rfl::to_generic(value);
    }
};

// Externally-tagged / dynamic response shapes are carried as typed serde::Value
// documents built from reflected values. The handler writes each document with
// serde::Ser::serialize, so no pre-encoded JSON text crosses the boundary.

class GetActionUrlResult
{
public:
    GetActionUrlResult() = default;

    explicit GetActionUrlResult(serde::Value document) noexcept :
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

class SyncOnboardingStatusResult
{
public:
    SyncOnboardingStatusResult() = default;

    explicit SyncOnboardingStatusResult(serde::Value document) noexcept :
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

class ResetTrackingIdResult
{
public:
    ResetTrackingIdResult() = default;

    explicit ResetTrackingIdResult(serde::Value document) noexcept :
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

} // namespace app::handlers::connector_onboarding
