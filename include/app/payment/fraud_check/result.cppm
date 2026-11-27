export module app_payment:fraud_check_result;

import std;
import core_serde;

export namespace app::handlers::fraud_check {

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

/** @brief Typed mirror of upstream FraudCheckResponseData::FulfillmentResponse. */
class FulfillmentResponseWire : public serde::Serializer<FulfillmentResponseWire>
{
public:
    void set_order_id(std::string value)
    {
        order_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_order_id() const noexcept
    {
        return order_id;
    }

    void set_shipment_ids(std::vector<std::string> value)
    {
        shipment_ids = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_shipment_ids() const noexcept
    {
        return shipment_ids;
    }

    std::string order_id{};
    std::vector<std::string> shipment_ids{};
};

// Exact wire key order of FulfillmentResponseWire (formerly also FrmFulfillmentResult).
inline constexpr std::array<std::string_view, 2> kFrmFulfillmentWireFields{
    "order_id",
    "shipment_ids"
};

} // namespace app::handlers::fraud_check

template<>
struct serde::Serializable<::app::handlers::fraud_check::FulfillmentResponseWire>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::fraud_check::FulfillmentResponseWire;
        return std::tuple{
            serde::FieldDesc<"order_id", &T::get_order_id, &T::set_order_id>{},
            serde::FieldDesc<"shipment_ids", &T::get_shipment_ids, &T::set_shipment_ids>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "frm_fulfillment_response_wire";
    }
};
