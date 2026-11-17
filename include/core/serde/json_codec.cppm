module;
#include <rfl/json.hpp>
export module core_serde:json_codec;

import std;

export namespace core_serde {

/** @brief JSON text <-> reflected value, backed by reflect-cpp. */
class JsonCodec
{
public:
    template<typename T>
    [[nodiscard]] std::string encode(const T& value) const
    {
        return rfl::json::write(value);
    }

    template<typename T>
    [[nodiscard]] std::optional<T> decode(std::string_view text) const
    {
        auto result = rfl::json::read<T>(std::string{text});
        if (!result) {
            return std::nullopt;
        }
        return std::move(*result);
    }
};

} // namespace core_serde
