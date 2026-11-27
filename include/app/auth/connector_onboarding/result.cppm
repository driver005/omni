module;
#include <rfl/to_generic.hpp>

export module app_auth:connector_onboarding_result;

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

// Externally-tagged / dynamic response shapes travel as typed serde::Value documents built in
// the runtime; the handler writes each document with serde::Ser::serialize.

} // namespace app::handlers::connector_onboarding
