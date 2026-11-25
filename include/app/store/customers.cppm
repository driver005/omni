module;
export module app_store:customers;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::customers {

/** @brief Persisted customer mirror of the upstream Customer api model. */
class CustomerRecord
{
public:
    CustomerRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_customer_id(std::string value)
    {
        m_customer_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_customer_id() const noexcept
    {
        return m_customer_id;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_name(std::string value)
    {
        m_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    void set_email(std::string value)
    {
        m_email = std::move(value);
    }

    [[nodiscard]] const std::string& get_email() const noexcept
    {
        return m_email;
    }

    void set_phone(std::string value)
    {
        m_phone = std::move(value);
    }

    [[nodiscard]] const std::string& get_phone() const noexcept
    {
        return m_phone;
    }

    void set_description(std::string value)
    {
        m_description = std::move(value);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return m_description;
    }

    void set_metadata(std::string value)
    {
        m_metadata = std::move(value);
    }

    [[nodiscard]] const std::string& get_metadata() const noexcept
    {
        return m_metadata;
    }

    void set_created_at(std::string value)
    {
        m_created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return m_created_at;
    }

    std::int64_t m_id{};
    std::string m_customer_id{};
    std::string m_merchant_id{};
    std::string m_name{};
    std::string m_email{};
    std::string m_phone{};
    std::string m_description{};
    std::string m_metadata{};
    std::string m_created_at{};
};

} // namespace app::store::customers

template<>
struct serde::Serializable<app::store::customers::CustomerRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::customers::CustomerRecord::get_id,
                &app::store::customers::CustomerRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "customer_id",
                &app::store::customers::CustomerRecord::get_customer_id,
                &app::store::customers::CustomerRecord::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::customers::CustomerRecord::get_merchant_id,
                &app::store::customers::CustomerRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::store::customers::CustomerRecord::get_name,
                &app::store::customers::CustomerRecord::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::store::customers::CustomerRecord::get_email,
                &app::store::customers::CustomerRecord::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::store::customers::CustomerRecord::get_phone,
                &app::store::customers::CustomerRecord::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::store::customers::CustomerRecord::get_description,
                &app::store::customers::CustomerRecord::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::store::customers::CustomerRecord::get_metadata,
                &app::store::customers::CustomerRecord::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::customers::CustomerRecord::get_created_at,
                &app::store::customers::CustomerRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customers";
    }
};

#ifdef CONGELADO_TEST
namespace app::store::customers::tests {

using namespace boost::ut;

suite<"app_store_customer_record"> customer_record_suite = []
{
    "stores customer fields and reports its table"_test = []
    {
        CustomerRecord record{};
        record.set_customer_id("cus_test");
        record.set_merchant_id("merchant_test");
        record.set_email("buyer@example.com");

        expect(record.get_customer_id() == "cus_test");
        expect(record.get_merchant_id() == "merchant_test");
        expect(record.get_email() == "buyer@example.com");
        expect(serde::Serializable<CustomerRecord>::table_name() == "customers");
    };
};

} // namespace app::store::customers::tests
#endif
