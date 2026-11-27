export module app_payment:refunds_result;

import std;
import interfaces;
import core_behavior;
import core_serde;
import app_model;
import app_context_core;

export namespace app::handlers::refunds {

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

// Former RefundsManualUpdateResult was an empty serde::Value document.
[[nodiscard]] inline serde::Value empty_refunds_wire() noexcept
{
    return serde::Value{serde::Value::Object{}};
}

} // namespace app::handlers::refunds

/** @brief Publish the single refund model using its captured response wire fields. */
export template<>
class core::behavior::Outcome<::app::model::Refund> :
    public std::expected<::app::model::Refund, core::error::AppError>
{
public:
    using std::expected<::app::model::Refund, core::error::AppError>::expected;

    Outcome(std::expected<::app::model::Refund, core::error::AppError>&& value) :
        std::expected<::app::model::Refund, core::error::AppError>{std::move(value)}
    {
    }

    void handle(interfaces::io::ResponseSink& sink)
    {
        if (!has_value()) {
            error().send(sink);
            return;
        }
        constexpr auto fields = serde::wire_fields<::app::model::Refund, serde::WireView::get>();
        const auto bytes = ::app::handlers::refunds::serialize_wire(value(), fields);
        sink.set_status(interfaces::io::types::Status::OK);
        sink.set_body(bytes);
        sink.send();
    }
};
