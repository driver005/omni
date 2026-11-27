module;
export module app_connector:model_shared_routing;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::routing {

enum class ConnectorSelection
{
    Priority,
    VolumeSplit,
};

class MerchantConnectorAccountIdValue : public serde::Serializer<MerchantConnectorAccountIdValue>
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MetadataValue : public serde::Serializer<MetadataValue>
{
public:
    void set_key(std::string value)
    {
        key = std::move(value);
    }

    [[nodiscard]] const std::string& get_key() const noexcept
    {
        return key;
    }

    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string key{};
    std::string value{};
};

enum class RoutableChoiceKind
{
    OnlyConnector,
    FullStruct,
};

class RoutableConnectorsValue : public serde::Serializer<RoutableConnectorsValue>
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class DeRoutableConnectorChoice : public serde::Serializer<DeRoutableConnectorChoice>
{
public:
    void set_gateway_name(RoutableConnectorsValue value)
    {
        gateway_name = std::move(value);
    }

    [[nodiscard]] const RoutableConnectorsValue& get_gateway_name() const noexcept
    {
        return gateway_name;
    }

    void set_gateway_id(std::optional<MerchantConnectorAccountIdValue> value)
    {
        gateway_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>&
    get_gateway_id() const noexcept
    {
        return gateway_id;
    }

    RoutableConnectorsValue gateway_name{};
    std::optional<MerchantConnectorAccountIdValue> gateway_id{};
};

class RoutableConnectorChoice : public serde::Serializer<RoutableConnectorChoice>
{
public:
    void set_choice_kind(RoutableChoiceKind value)
    {
        choice_kind = std::move(value);
    }

    [[nodiscard]] const RoutableChoiceKind& get_choice_kind() const noexcept
    {
        return choice_kind;
    }

    void set_connector(RoutableConnectorsValue value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const RoutableConnectorsValue& get_connector() const noexcept
    {
        return connector;
    }

    void set_merchant_connector_id(std::optional<MerchantConnectorAccountIdValue> value)
    {
        merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>&
    get_merchant_connector_id() const noexcept
    {
        return merchant_connector_id;
    }

    RoutableChoiceKind choice_kind{};
    RoutableConnectorsValue connector{};
    std::optional<MerchantConnectorAccountIdValue> merchant_connector_id{};
};

class ConnectorVolumeSplit : public serde::Serializer<ConnectorVolumeSplit>
{
public:
    void set_connector(RoutableConnectorChoice value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const RoutableConnectorChoice& get_connector() const noexcept
    {
        return connector;
    }

    void set_split(std::uint8_t value)
    {
        split = std::move(value);
    }

    [[nodiscard]] const std::uint8_t& get_split() const noexcept
    {
        return split;
    }

    RoutableConnectorChoice connector{};
    std::uint8_t split{};
};

class RoutableConnectorChoiceWithBucketName :
    public serde::Serializer<RoutableConnectorChoiceWithBucketName>
{
public:
    void set_routable_connector_choice(RoutableConnectorChoice value)
    {
        routable_connector_choice = std::move(value);
    }

    [[nodiscard]] const RoutableConnectorChoice& get_routable_connector_choice() const noexcept
    {
        return routable_connector_choice;
    }

    void set_bucket_name(std::string value)
    {
        bucket_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_bucket_name() const noexcept
    {
        return bucket_name;
    }

    RoutableConnectorChoice routable_connector_choice{};
    std::string bucket_name{};
};

class RoutableConnectorChoiceWithStatus :
    public serde::Serializer<RoutableConnectorChoiceWithStatus>
{
public:
    void set_routable_connector_choice(RoutableConnectorChoice value)
    {
        routable_connector_choice = std::move(value);
    }

    [[nodiscard]] const RoutableConnectorChoice& get_routable_connector_choice() const noexcept
    {
        return routable_connector_choice;
    }

    void set_status(bool value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const bool& get_status() const noexcept
    {
        return status;
    }

    RoutableConnectorChoice routable_connector_choice{};
    bool status{};
};

} // namespace app::model::routing

template<>
struct serde::Serializable<app::model::routing::ConnectorVolumeSplit>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector",
                &app::model::routing::ConnectorVolumeSplit::get_connector,
                &app::model::routing::ConnectorVolumeSplit::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "split",
                &app::model::routing::ConnectorVolumeSplit::get_split,
                &app::model::routing::ConnectorVolumeSplit::set_split,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "connector_volume_split";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutableConnectorChoice>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "choice_kind",
                &app::model::routing::RoutableConnectorChoice::get_choice_kind,
                &app::model::routing::RoutableConnectorChoice::set_choice_kind,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::routing::RoutableConnectorChoice::get_connector,
                &app::model::routing::RoutableConnectorChoice::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::routing::RoutableConnectorChoice::get_merchant_connector_id,
                &app::model::routing::RoutableConnectorChoice::set_merchant_connector_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routable_connector_choice";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutableConnectorChoiceWithStatus>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "routable_connector_choice",
                &app::model::routing::RoutableConnectorChoiceWithStatus::
                    get_routable_connector_choice,
                &app::model::routing::RoutableConnectorChoiceWithStatus::
                    set_routable_connector_choice,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::routing::RoutableConnectorChoiceWithStatus::get_status,
                &app::model::routing::RoutableConnectorChoiceWithStatus::set_status,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routable_connector_choice_with_status";
    }
};

template<>
struct serde::Serializable<app::model::routing::RoutableConnectorChoiceWithBucketName>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "routable_connector_choice",
                &app::model::routing::RoutableConnectorChoiceWithBucketName::
                    get_routable_connector_choice,
                &app::model::routing::RoutableConnectorChoiceWithBucketName::
                    set_routable_connector_choice,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bucket_name",
                &app::model::routing::RoutableConnectorChoiceWithBucketName::get_bucket_name,
                &app::model::routing::RoutableConnectorChoiceWithBucketName::set_bucket_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "routable_connector_choice_with_bucket_name";
    }
};

template<>
struct serde::Serializable<app::model::routing::DeRoutableConnectorChoice>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "gateway_name",
                &app::model::routing::DeRoutableConnectorChoice::get_gateway_name,
                &app::model::routing::DeRoutableConnectorChoice::set_gateway_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gateway_id",
                &app::model::routing::DeRoutableConnectorChoice::get_gateway_id,
                &app::model::routing::DeRoutableConnectorChoice::set_gateway_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "de_routable_connector_choice";
    }
};

template<>
struct serde::Serializable<app::model::routing::MetadataValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "key",
                &app::model::routing::MetadataValue::get_key,
                &app::model::routing::MetadataValue::set_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "value",
                &app::model::routing::MetadataValue::get_value,
                &app::model::routing::MetadataValue::set_value,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "metadata_value";
    }
};
