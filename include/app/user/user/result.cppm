module;
#include <rfl/to_generic.hpp>

export module app_user:user_result;

import std;
import core_serde;

export namespace app::handlers::user {

/** @brief Builds typed dynamic response documents; no hand-written JSON strings. */
class ResponseDocument
{
public:
    [[nodiscard]] static serde::Value empty()
    {
        return serde::Value{serde::Value::Object{}};
    }

    template<typename T>
    [[nodiscard]] static serde::Value from(const T& value)
    {
        return rfl::to_generic(value);
    }
};

} // namespace app::handlers::user
