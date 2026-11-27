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

} // namespace app::model::refunds
