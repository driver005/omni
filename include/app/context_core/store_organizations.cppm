module;
export module app_context_core:store_organizations;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::organizations {

/** @brief Persisted organization mirror of the upstream Organization api model. */
class OrganizationRecord
{
public:
    OrganizationRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_organization_id(std::string value)
    {
        m_organization_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_organization_id() const noexcept
    {
        return m_organization_id;
    }

    void set_organization_name(std::string value)
    {
        m_organization_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_organization_name() const noexcept
    {
        return m_organization_name;
    }

    void set_organization_type(std::string value)
    {
        m_organization_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_organization_type() const noexcept
    {
        return m_organization_type;
    }

    void set_organization_details(std::string value)
    {
        m_organization_details = std::move(value);
    }

    [[nodiscard]] const std::string& get_organization_details() const noexcept
    {
        return m_organization_details;
    }

    void set_metadata(std::string value)
    {
        m_metadata = std::move(value);
    }

    [[nodiscard]] const std::string& get_metadata() const noexcept
    {
        return m_metadata;
    }

    void set_platform_merchant_id(std::string value)
    {
        m_platform_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_platform_merchant_id() const noexcept
    {
        return m_platform_merchant_id;
    }

    void set_created_at(std::string value)
    {
        m_created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return m_created_at;
    }

    void set_modified_at(std::string value)
    {
        m_modified_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_modified_at() const noexcept
    {
        return m_modified_at;
    }

    std::int64_t m_id{};
    std::string m_organization_id{};
    std::string m_organization_name{};
    std::string m_organization_type{};
    std::string m_organization_details{};
    std::string m_metadata{};
    std::string m_platform_merchant_id{};
    std::string m_created_at{};
    std::string m_modified_at{};
};

} // namespace app::store::organizations

template<>
struct serde::Serializable<app::store::organizations::OrganizationRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::organizations::OrganizationRecord::get_id,
                &app::store::organizations::OrganizationRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "organization_id",
                &app::store::organizations::OrganizationRecord::get_organization_id,
                &app::store::organizations::OrganizationRecord::set_organization_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "organization_name",
                &app::store::organizations::OrganizationRecord::get_organization_name,
                &app::store::organizations::OrganizationRecord::set_organization_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "organization_type",
                &app::store::organizations::OrganizationRecord::get_organization_type,
                &app::store::organizations::OrganizationRecord::set_organization_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "organization_details",
                &app::store::organizations::OrganizationRecord::get_organization_details,
                &app::store::organizations::OrganizationRecord::set_organization_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::store::organizations::OrganizationRecord::get_metadata,
                &app::store::organizations::OrganizationRecord::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "platform_merchant_id",
                &app::store::organizations::OrganizationRecord::get_platform_merchant_id,
                &app::store::organizations::OrganizationRecord::set_platform_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::organizations::OrganizationRecord::get_created_at,
                &app::store::organizations::OrganizationRecord::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "modified_at",
                &app::store::organizations::OrganizationRecord::get_modified_at,
                &app::store::organizations::OrganizationRecord::set_modified_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "organizations";
    }
};

#ifdef CONGELADO_TEST
namespace app::store::organizations::tests {

using namespace boost::ut;

suite<"app_store_organization_record"> organization_record_suite = []
{
    "stores organization fields and reports its table"_test = []
    {
        OrganizationRecord record{};
        record.set_organization_id("org_test");
        record.set_organization_name("Acme");
        record.set_organization_type("merchant");

        expect(record.get_organization_id() == "org_test");
        expect(record.get_organization_name() == "Acme");
        expect(record.get_organization_type() == "merchant");
        expect(serde::Serializable<OrganizationRecord>::table_name() == "organizations");
    };
};

} // namespace app::store::organizations::tests
#endif
