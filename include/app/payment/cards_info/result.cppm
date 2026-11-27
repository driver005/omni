export module app_payment:cards_info_result;

import std;
import core_serde;
import app_model;

export namespace app::handlers::cards_info {

/**
 * @brief Wire encoder for the domain's response models.
 *
 * The former Result wrappers are gone: runtimes hand back the underlying model,
 * a std::vector of models, or an explicit serde::Value document. Model encoding
 * goes through Serializer<T>::serialize(fields). An empty span means all declared
 * fields in declaration order — Serializer<T>::metadata::field_names() is the
 * captured wire list, identical to the field tuple the old wrapper forwarded.
 */
template<class T>
    requires requires(const T& value, std::span<const std::string_view> fields) {
        value.serialize(fields);
    }
[[nodiscard]] std::vector<std::byte>
serialize_wire(const T& value, std::span<const std::string_view> fields = {})
{
    auto text = value.serialize(fields);
    if (!text) {
        return serde::Ser::serialize_error("application/json", "serialization failed");
    }
    std::vector<std::byte> bytes(text->size());
    std::ranges::transform(
        *text,
        bytes.begin(),
        [](char character) noexcept
        {
            return std::byte(character);
        }
    );
    return bytes;
}

template<class T>
[[nodiscard]] std::vector<std::byte>
serialize_wire(const std::vector<T>& value, std::span<const std::string_view> fields = {})
{
    auto text = serde::Serializer<std::vector<T>>::serialize_values(value, fields);
    if (!text) {
        return serde::Ser::serialize_error("application/json", "serialization failed");
    }
    std::vector<std::byte> bytes(text->size());
    std::ranges::transform(
        *text,
        bytes.begin(),
        [](char character) noexcept
        {
            return std::byte(character);
        }
    );
    return bytes;
}

[[nodiscard]] inline std::vector<std::byte> serialize_wire(const serde::Value& value)
{
    return serde::Ser::serialize("application/json", value);
}

[[nodiscard]] inline std::vector<std::byte> serialize_wire(const std::string& value)
{
    return serde::Ser::serialize("application/json", value);
}

} // namespace app::handlers::cards_info
