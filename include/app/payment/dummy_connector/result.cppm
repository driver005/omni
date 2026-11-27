export module app_payment:dummy_connector_result;

import std;
import core_serde;

export namespace app::handlers::dummy_connector {

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

/** @brief Flat refund response model. Replaces the former refund Result wrappers. */
class DummyConnectorRefundWire : public serde::Serializer<DummyConnectorRefundWire>
{
public:
    DummyConnectorRefundWire() = default;

    DummyConnectorRefundWire(
        std::string status,
        std::string id,
        std::string currency,
        std::string created,
        std::int64_t payment_amount,
        std::int64_t refund_amount
    ) noexcept :
        m_status{std::move(status)},
        m_id{std::move(id)},
        m_currency{std::move(currency)},
        m_created{std::move(created)},
        m_payment_amount{payment_amount},
        m_refund_amount{refund_amount}
    {
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return m_id;
    }

    void set_id(std::string value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_currency() const noexcept
    {
        return m_currency;
    }

    void set_currency(std::string value)
    {
        m_currency = std::move(value);
    }

    [[nodiscard]] const std::string& get_created() const noexcept
    {
        return m_created;
    }

    void set_created(std::string value)
    {
        m_created = std::move(value);
    }

    [[nodiscard]] std::int64_t get_payment_amount() const noexcept
    {
        return m_payment_amount;
    }

    void set_payment_amount(std::int64_t value)
    {
        m_payment_amount = value;
    }

    [[nodiscard]] std::int64_t get_refund_amount() const noexcept
    {
        return m_refund_amount;
    }

    void set_refund_amount(std::int64_t value)
    {
        m_refund_amount = value;
    }

private:
    std::string m_status{};
    std::string m_id{};
    std::string m_currency{};
    std::string m_created{};
    std::int64_t m_payment_amount{0};
    std::int64_t m_refund_amount{0};
};

// Exact wire key order of the former DummyConnectorRefundResult / ...DataResult.
inline constexpr std::array<std::string_view, 6> kDummyConnectorRefundWireFields{
    "status",
    "id",
    "currency",
    "created",
    "payment_amount",
    "refund_amount"
};

} // namespace app::handlers::dummy_connector

template<>
struct serde::Serializable<::app::handlers::dummy_connector::DummyConnectorRefundWire>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::dummy_connector::DummyConnectorRefundWire;
        return std::tuple{
            serde::FieldDesc<"status", &T::get_status, &T::set_status>{},
            serde::FieldDesc<"id", &T::get_id, &T::set_id>{},
            serde::FieldDesc<"currency", &T::get_currency, &T::set_currency>{},
            serde::FieldDesc<"created", &T::get_created, &T::set_created>{},
            serde::FieldDesc<"payment_amount", &T::get_payment_amount, &T::set_payment_amount>{},
            serde::FieldDesc<"refund_amount", &T::get_refund_amount, &T::set_refund_amount>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "dummy_connector_refund_wire";
    }
};
