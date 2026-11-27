module;
export module app_connector:model_shared_refunds;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::refunds {

class CurrencyValue : public serde::Serializer<CurrencyValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MerchantConnectorAccountIdValue : public serde::Serializer<MerchantConnectorAccountIdValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MerchantConnectorAuthDetailsValue :
    public serde::Serializer<MerchantConnectorAuthDetailsValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MerchantConnectorDetailsWrapValue :
    public serde::Serializer<MerchantConnectorDetailsWrapValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MerchantConnectorInfoValue : public serde::Serializer<MerchantConnectorInfoValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentMethodDataValue : public serde::Serializer<PaymentMethodDataValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class RefundFeatureMetadataValue : public serde::Serializer<RefundFeatureMetadataValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class RefundStatusValue : public serde::Serializer<RefundStatusValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class SecretSerdeValueValue : public serde::Serializer<SecretSerdeValueValue>
{
public:
    void set_value(std::string new_value)
    {
        value = std::move(new_value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class RefundListMetaData : public serde::Serializer<RefundListMetaData>
{
public:
    void set_connector(std::vector<std::string> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_connector() const noexcept
    {
        return connector;
    }

    void set_currency(std::vector<CurrencyValue> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::vector<CurrencyValue>& get_currency() const noexcept
    {
        return currency;
    }

    void set_refund_status(std::vector<RefundStatusValue> value)
    {
        refund_status = std::move(value);
    }

    [[nodiscard]] const std::vector<RefundStatusValue>& get_refund_status() const noexcept
    {
        return refund_status;
    }

    std::vector<std::string> connector{};
    std::vector<CurrencyValue> currency{};
    std::vector<RefundStatusValue> refund_status{};
};

class RefundMetadataUpdateRequest : public serde::Serializer<RefundMetadataUpdateRequest>
{
public:
    void set_reason(std::optional<std::string> value)
    {
        reason = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_reason() const noexcept
    {
        return reason;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    std::optional<std::string> reason{};
    std::optional<SecretSerdeValueValue> metadata{};
};

} // namespace app::model::refunds

template<>
struct serde::Serializable<app::model::refunds::RefundMetadataUpdateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "reason",
                &app::model::refunds::RefundMetadataUpdateRequest::get_reason,
                &app::model::refunds::RefundMetadataUpdateRequest::set_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::refunds::RefundMetadataUpdateRequest::get_metadata,
                &app::model::refunds::RefundMetadataUpdateRequest::set_metadata,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refund_metadata_update_request";
    }
};

template<>
struct serde::Serializable<app::model::refunds::RefundListMetaData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector",
                &app::model::refunds::RefundListMetaData::get_connector,
                &app::model::refunds::RefundListMetaData::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::refunds::RefundListMetaData::get_currency,
                &app::model::refunds::RefundListMetaData::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund_status",
                &app::model::refunds::RefundListMetaData::get_refund_status,
                &app::model::refunds::RefundListMetaData::set_refund_status,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "refund_list_meta_data";
    }
};
