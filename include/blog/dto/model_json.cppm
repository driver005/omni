export module blog_dto:model_json;

import std;
import core_serde;

export namespace blog::dto {

/** @brief JSON codec for DTOs, backed by core_serde. */
template<typename T>
class ModelJson
{
public:
    [[nodiscard]] std::string to_json(const T& value) const
    {
        return core_serde::JsonCodec{}.encode(value);
    }

    [[nodiscard]] std::optional<T> from_json(std::string_view text) const
    {
        return core_serde::JsonCodec{}.decode<T>(text);
    }
};

} // namespace blog::dto
