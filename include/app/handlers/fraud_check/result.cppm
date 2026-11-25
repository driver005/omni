export module app_handlers_fraud_check:result;

import std;
import core_serde;

export namespace app::handlers::fraud_check {

/**
 * @brief Typed mirror of upstream `FraudCheckResponseData::FulfillmentResponse`
 * (hyperswitch_domain_models/src/router_response_types/fraud_check.rs:15-18).
 */
class FulfillmentResponseWire
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

// The old core encoded this exact wire type with Ser::encode_json. Deriving keeps the same
// FieldDesc tuple (field names, order and wrapper types), so the serialized bytes match.
class FrmFulfillmentResult : public FulfillmentResponseWire
{
public:
    FrmFulfillmentResult() = default;

    explicit FrmFulfillmentResult(FulfillmentResponseWire value) noexcept :
        FulfillmentResponseWire{std::move(value)}
    {
    }
};

} // namespace app::handlers::fraud_check

template<>
struct serde::Serializable<::app::handlers::fraud_check::FulfillmentResponseWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "order_id",
                &::app::handlers::fraud_check::FulfillmentResponseWire::get_order_id,
                &::app::handlers::fraud_check::FulfillmentResponseWire::set_order_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipment_ids",
                &::app::handlers::fraud_check::FulfillmentResponseWire::get_shipment_ids,
                &::app::handlers::fraud_check::FulfillmentResponseWire::set_shipment_ids,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "frm_fulfillment_response_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::fraud_check::FrmFulfillmentResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::handlers::fraud_check::FulfillmentResponseWire>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "frm_fulfillment_result";
    }
};
