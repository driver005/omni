module;
#include <rfl/json.hpp>
export module core_serde:json_codec;

import std;
import :core;
import :converter;

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

export namespace serde {

template<>
struct FieldConverter<RawJson>
{
    using rfl_type = rfl::Generic;

    static rfl_type to_rfl(const RawJson& value)
    {
        const auto parsed = rfl::json::read<rfl::Generic>(value.json);
        return parsed ? *parsed : rfl::Generic{};
    }

    static RawJson from_rfl(const rfl_type& value)
    {
        return RawJson{rfl::json::write(value)};
    }
};

} // namespace serde
