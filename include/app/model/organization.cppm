module;
export module app_model:organization;

import core_serde;
import core_behavior;

import std;
import core_serde;
import core_behavior;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::organization {

class ConvertOrganizationToPlatformRequest :
    public core::behavior::Serializer<ConvertOrganizationToPlatformRequest>
{
public:
    void set_platform_merchant_name(std::optional<std::string> value)
    {
        platform_merchant_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_platform_merchant_name() const noexcept
    {
        return platform_merchant_name;
    }

    std::optional<std::string> platform_merchant_name{};
};

class MerchantIdValue : public core::behavior::Serializer<MerchantIdValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class OrganizationIdValue : public core::behavior::Serializer<OrganizationIdValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class OrganizationTypeValue : public core::behavior::Serializer<OrganizationTypeValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PrimitiveDateTimeValue : public core::behavior::Serializer<PrimitiveDateTimeValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class SecretSerdeValueValue : public core::behavior::Serializer<SecretSerdeValueValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ConvertOrganizationToPlatformResponse :
    public core::behavior::Serializer<ConvertOrganizationToPlatformResponse>
{
public:
    void set_organization_id(OrganizationIdValue value)
    {
        organization_id = std::move(value);
    }

    [[nodiscard]] const OrganizationIdValue& get_organization_id() const noexcept
    {
        return organization_id;
    }

    void set_organization_type(OrganizationTypeValue value)
    {
        organization_type = std::move(value);
    }

    [[nodiscard]] const OrganizationTypeValue& get_organization_type() const noexcept
    {
        return organization_type;
    }

    void set_platform_merchant_id(MerchantIdValue value)
    {
        platform_merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_platform_merchant_id() const noexcept
    {
        return platform_merchant_id;
    }

    OrganizationIdValue organization_id{};
    OrganizationTypeValue organization_type{};
    MerchantIdValue platform_merchant_id{};
};

class OrganizationCreateRequest : public core::behavior::Serializer<OrganizationCreateRequest>
{
public:
    void set_organization_name(std::string value)
    {
        organization_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_organization_name() const noexcept
    {
        return organization_name;
    }

    void set_organization_details(std::optional<SecretSerdeValueValue> value)
    {
        organization_details = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>&
    get_organization_details() const noexcept
    {
        return organization_details;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    std::string organization_name{};
    std::optional<SecretSerdeValueValue> organization_details{};
    std::optional<SecretSerdeValueValue> metadata{};
};

class OrganizationId : public core::behavior::Serializer<OrganizationId>
{
public:
    void set_organization_id(OrganizationIdValue value)
    {
        organization_id = std::move(value);
    }

    [[nodiscard]] const OrganizationIdValue& get_organization_id() const noexcept
    {
        return organization_id;
    }

    OrganizationIdValue organization_id{};
};

class OrganizationNew : public core::behavior::Serializer<OrganizationNew>
{
public:
    void set_org_id(OrganizationIdValue value)
    {
        org_id = std::move(value);
    }

    [[nodiscard]] const OrganizationIdValue& get_org_id() const noexcept
    {
        return org_id;
    }

    void set_org_type(OrganizationTypeValue value)
    {
        org_type = std::move(value);
    }

    [[nodiscard]] const OrganizationTypeValue& get_org_type() const noexcept
    {
        return org_type;
    }

    void set_org_name(std::optional<std::string> value)
    {
        org_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_org_name() const noexcept
    {
        return org_name;
    }

    OrganizationIdValue org_id{};
    OrganizationTypeValue org_type{};
    std::optional<std::string> org_name{};
};

class OrganizationResponse : public core::behavior::Serializer<OrganizationResponse>
{
public:
    void set_organization_id(OrganizationIdValue value)
    {
        organization_id = std::move(value);
    }

    [[nodiscard]] const OrganizationIdValue& get_organization_id() const noexcept
    {
        return organization_id;
    }

    void set_organization_name(std::optional<std::string> value)
    {
        organization_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_organization_name() const noexcept
    {
        return organization_name;
    }

    void set_organization_details(std::optional<SecretSerdeValueValue> value)
    {
        organization_details = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>&
    get_organization_details() const noexcept
    {
        return organization_details;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_modified_at(PrimitiveDateTimeValue value)
    {
        modified_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_modified_at() const noexcept
    {
        return modified_at;
    }

    void set_created_at(PrimitiveDateTimeValue value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_organization_type(std::optional<OrganizationTypeValue> value)
    {
        organization_type = std::move(value);
    }

    [[nodiscard]] const std::optional<OrganizationTypeValue>& get_organization_type() const noexcept
    {
        return organization_type;
    }

    OrganizationIdValue organization_id{};
    std::optional<std::string> organization_name{};
    std::optional<SecretSerdeValueValue> organization_details{};
    std::optional<SecretSerdeValueValue> metadata{};
    PrimitiveDateTimeValue modified_at{};
    PrimitiveDateTimeValue created_at{};
    std::optional<OrganizationTypeValue> organization_type{};
};

class OrganizationUpdateRequest : public core::behavior::Serializer<OrganizationUpdateRequest>
{
public:
    void set_organization_name(std::optional<std::string> value)
    {
        organization_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_organization_name() const noexcept
    {
        return organization_name;
    }

    void set_organization_details(std::optional<SecretSerdeValueValue> value)
    {
        organization_details = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>&
    get_organization_details() const noexcept
    {
        return organization_details;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_platform_merchant_id(std::optional<MerchantIdValue> value)
    {
        platform_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantIdValue>& get_platform_merchant_id() const noexcept
    {
        return platform_merchant_id;
    }

    std::optional<std::string> organization_name{};
    std::optional<SecretSerdeValueValue> organization_details{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<MerchantIdValue> platform_merchant_id{};
};
} // namespace app::model::organization

template<>
struct serde::Serializable<app::model::organization::OrganizationNew>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "org_id",
                &app::model::organization::OrganizationNew::get_org_id,
                &app::model::organization::OrganizationNew::set_org_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "org_type",
                &app::model::organization::OrganizationNew::get_org_type,
                &app::model::organization::OrganizationNew::set_org_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "org_name",
                &app::model::organization::OrganizationNew::get_org_name,
                &app::model::organization::OrganizationNew::set_org_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "organization_new";
    }
};

template<>
struct serde::Serializable<app::model::organization::OrganizationId>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "organization_id",
            &app::model::organization::OrganizationId::get_organization_id,
            &app::model::organization::OrganizationId::set_organization_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "organization_id";
    }
};

template<>
struct serde::Serializable<app::model::organization::OrganizationCreateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "organization_name",
                &app::model::organization::OrganizationCreateRequest::get_organization_name,
                &app::model::organization::OrganizationCreateRequest::set_organization_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "organization_details",
                &app::model::organization::OrganizationCreateRequest::get_organization_details,
                &app::model::organization::OrganizationCreateRequest::set_organization_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::organization::OrganizationCreateRequest::get_metadata,
                &app::model::organization::OrganizationCreateRequest::set_metadata,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "organization_create_request";
    }
};

template<>
struct serde::Serializable<app::model::organization::OrganizationUpdateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "organization_name",
                &app::model::organization::OrganizationUpdateRequest::get_organization_name,
                &app::model::organization::OrganizationUpdateRequest::set_organization_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "organization_details",
                &app::model::organization::OrganizationUpdateRequest::get_organization_details,
                &app::model::organization::OrganizationUpdateRequest::set_organization_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::organization::OrganizationUpdateRequest::get_metadata,
                &app::model::organization::OrganizationUpdateRequest::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "platform_merchant_id",
                &app::model::organization::OrganizationUpdateRequest::get_platform_merchant_id,
                &app::model::organization::OrganizationUpdateRequest::set_platform_merchant_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "organization_update_request";
    }
};

template<>
struct serde::Serializable<app::model::organization::OrganizationResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "organization_id",
                &app::model::organization::OrganizationResponse::get_organization_id,
                &app::model::organization::OrganizationResponse::set_organization_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "organization_name",
                &app::model::organization::OrganizationResponse::get_organization_name,
                &app::model::organization::OrganizationResponse::set_organization_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "organization_details",
                &app::model::organization::OrganizationResponse::get_organization_details,
                &app::model::organization::OrganizationResponse::set_organization_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::organization::OrganizationResponse::get_metadata,
                &app::model::organization::OrganizationResponse::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "modified_at",
                &app::model::organization::OrganizationResponse::get_modified_at,
                &app::model::organization::OrganizationResponse::set_modified_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::model::organization::OrganizationResponse::get_created_at,
                &app::model::organization::OrganizationResponse::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "organization_type",
                &app::model::organization::OrganizationResponse::get_organization_type,
                &app::model::organization::OrganizationResponse::set_organization_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "organization_response";
    }
};

template<>
struct serde::Serializable<app::model::organization::ConvertOrganizationToPlatformRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "platform_merchant_name",
            &app::model::organization::ConvertOrganizationToPlatformRequest::
                get_platform_merchant_name,
            &app::model::organization::ConvertOrganizationToPlatformRequest::
                set_platform_merchant_name,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "convert_organization_to_platform_request";
    }
};

template<>
struct serde::Serializable<app::model::organization::ConvertOrganizationToPlatformResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "organization_id",
                &app::model::organization::ConvertOrganizationToPlatformResponse::
                    get_organization_id,
                &app::model::organization::ConvertOrganizationToPlatformResponse::
                    set_organization_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "organization_type",
                &app::model::organization::ConvertOrganizationToPlatformResponse::
                    get_organization_type,
                &app::model::organization::ConvertOrganizationToPlatformResponse::
                    set_organization_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "platform_merchant_id",
                &app::model::organization::ConvertOrganizationToPlatformResponse::
                    get_platform_merchant_id,
                &app::model::organization::ConvertOrganizationToPlatformResponse::
                    set_platform_merchant_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "convert_organization_to_platform_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::model::organization::tests {
using namespace boost::ut;
suite<"app_model_organization"> app_model_organization_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<decltype(serde::Serializable<
                                       app::model::organization::OrganizationNew>::fields())> > 0
        );
    };
};
} // namespace app::model::organization::tests
#endif
