export module app_store:configs_result;

import std;

export namespace app::handlers::configs {

// All four config routes emit the same flat {key, value} document. The order is
// captured from the retired *Result DTOs and is preserved when the runtime
// builds the serde::Value::Object.
[[nodiscard]] inline constexpr auto config_key_wire_fields()
{
    return std::array<std::string_view, 2>{"key", "value"};
}

} // namespace app::handlers::configs
