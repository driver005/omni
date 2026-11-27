module;
#include <rfl/to_generic.hpp>

export module app_routing:routing_result;

import std;
import core_serde;

export namespace app::handlers::routing {

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

// Routing response wrappers are gone: every route runtime now returns the
// explicit serde::Value document directly, and the handler encodes that
// document with serde::Ser::serialize. Envelopes/unions stay as typed documents.

} // namespace app::handlers::routing
