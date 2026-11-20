module;
export module app_store:mandates;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::mandates {

/** @brief Persisted mandate mirror of the upstream Mandate api model. */
class MandateRecord
{
public:
    MandateRecord() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_mandate_id(std::string value) { m_mandate_id = std::move(value); }
    [[nodiscard]] const std::string& get_mandate_id() const noexcept { return m_mandate_id; }
    void set_payment_id(std::string value) { m_payment_id = std::move(value); }
    [[nodiscard]] const std::string& get_payment_id() const noexcept { return m_payment_id; }
    void set_merchant_id(std::string value) { m_merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return m_merchant_id; }
    void set_customer_id(std::string value) { m_customer_id = std::move(value); }
    [[nodiscard]] const std::string& get_customer_id() const noexcept { return m_customer_id; }
    void set_profile_id(std::string value) { m_profile_id = std::move(value); }
    [[nodiscard]] const std::string& get_profile_id() const noexcept { return m_profile_id; }
    void set_amount(std::int64_t value) { m_amount = std::move(value); }
    [[nodiscard]] const std::int64_t& get_amount() const noexcept { return m_amount; }
    void set_currency(std::string value) { m_currency = std::move(value); }
    [[nodiscard]] const std::string& get_currency() const noexcept { return m_currency; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_payment_method_id(std::string value) { m_payment_method_id = std::move(value); }
    [[nodiscard]] const std::string& get_payment_method_id() const noexcept { return m_payment_method_id; }
    void set_payment_method(std::string value) { m_payment_method = std::move(value); }
    [[nodiscard]] const std::string& get_payment_method() const noexcept { return m_payment_method; }
    void set_payment_method_type(std::string value) { m_payment_method_type = std::move(value); }
    [[nodiscard]] const std::string& get_payment_method_type() const noexcept { return m_payment_method_type; }
    void set_connector(std::string value) { m_connector = std::move(value); }
    [[nodiscard]] const std::string& get_connector() const noexcept { return m_connector; }
    void set_mandate_metadata(std::string value) { m_mandate_metadata = std::move(value); }
    [[nodiscard]] const std::string& get_mandate_metadata() const noexcept { return m_mandate_metadata; }
    void set_created_at(std::string value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::string& get_created_at() const noexcept { return m_created_at; }

    std::int64_t m_id{};
    std::string m_mandate_id{};
    std::string m_payment_id{};
    std::string m_merchant_id{};
    std::string m_customer_id{};
    std::string m_profile_id{};
    std::int64_t m_amount{};
    std::string m_currency{};
    std::string m_status{};
    std::string m_payment_method_id{};
    std::string m_payment_method{};
    std::string m_payment_method_type{};
    std::string m_connector{};
    std::string m_mandate_metadata{};
    std::string m_created_at{};
};

} // namespace app::store::mandates

template<>
struct serde::Serializable<app::store::mandates::MandateRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::store::mandates::MandateRecord::get_id, &app::store::mandates::MandateRecord::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"mandate_id", &app::store::mandates::MandateRecord::get_mandate_id, &app::store::mandates::MandateRecord::set_mandate_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_id", &app::store::mandates::MandateRecord::get_payment_id, &app::store::mandates::MandateRecord::set_payment_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::store::mandates::MandateRecord::get_merchant_id, &app::store::mandates::MandateRecord::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_id", &app::store::mandates::MandateRecord::get_customer_id, &app::store::mandates::MandateRecord::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::store::mandates::MandateRecord::get_profile_id, &app::store::mandates::MandateRecord::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"amount", &app::store::mandates::MandateRecord::get_amount, &app::store::mandates::MandateRecord::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::store::mandates::MandateRecord::get_currency, &app::store::mandates::MandateRecord::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::store::mandates::MandateRecord::get_status, &app::store::mandates::MandateRecord::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method_id", &app::store::mandates::MandateRecord::get_payment_method_id, &app::store::mandates::MandateRecord::set_payment_method_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method", &app::store::mandates::MandateRecord::get_payment_method, &app::store::mandates::MandateRecord::set_payment_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method_type", &app::store::mandates::MandateRecord::get_payment_method_type, &app::store::mandates::MandateRecord::set_payment_method_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &app::store::mandates::MandateRecord::get_connector, &app::store::mandates::MandateRecord::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"mandate_metadata", &app::store::mandates::MandateRecord::get_mandate_metadata, &app::store::mandates::MandateRecord::set_mandate_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::store::mandates::MandateRecord::get_created_at, &app::store::mandates::MandateRecord::set_created_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "mandates"; }
};

#ifdef CONGELADO_TEST
namespace app::store::mandates::tests {

using namespace boost::ut;

suite<"app_store_mandate_record"> mandate_record_suite = []
{
    "stores mandate fields and reports its table"_test = []
    {
        MandateRecord record{};
        record.set_mandate_id("mnd_test");
        record.set_payment_id("pi_test");
        record.set_status("active");

        expect(record.get_mandate_id() == "mnd_test");
        expect(record.get_payment_id() == "pi_test");
        expect(record.get_status() == "active");
        expect(serde::Serializable<MandateRecord>::table_name() == "mandates");
    };
};

} // namespace app::store::mandates::tests
#endif
