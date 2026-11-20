module;
export module app_store:refunds;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::refunds {

/** @brief Persisted refund mirror of the upstream Refund api model. */
class RefundRecord
{
public:
    RefundRecord() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_refund_id(std::string value) { m_refund_id = std::move(value); }
    [[nodiscard]] const std::string& get_refund_id() const noexcept { return m_refund_id; }
    void set_payment_id(std::string value) { m_payment_id = std::move(value); }
    [[nodiscard]] const std::string& get_payment_id() const noexcept { return m_payment_id; }
    void set_merchant_id(std::string value) { m_merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return m_merchant_id; }
    void set_amount(std::int64_t value) { m_amount = std::move(value); }
    [[nodiscard]] const std::int64_t& get_amount() const noexcept { return m_amount; }
    void set_currency(std::string value) { m_currency = std::move(value); }
    [[nodiscard]] const std::string& get_currency() const noexcept { return m_currency; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_reason(std::string value) { m_reason = std::move(value); }
    [[nodiscard]] const std::string& get_reason() const noexcept { return m_reason; }
    void set_connector(std::string value) { m_connector = std::move(value); }
    [[nodiscard]] const std::string& get_connector() const noexcept { return m_connector; }
    void set_created_at(std::string value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::string& get_created_at() const noexcept { return m_created_at; }
    void set_updated_at(std::string value) { m_updated_at = std::move(value); }
    [[nodiscard]] const std::string& get_updated_at() const noexcept { return m_updated_at; }

    std::int64_t m_id{};
    std::string m_refund_id{};
    std::string m_payment_id{};
    std::string m_merchant_id{};
    std::int64_t m_amount{};
    std::string m_currency{};
    std::string m_status{};
    std::string m_reason{};
    std::string m_connector{};
    std::string m_created_at{};
    std::string m_updated_at{};
};

} // namespace app::store::refunds

template<>
struct serde::Serializable<app::store::refunds::RefundRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::store::refunds::RefundRecord::get_id, &app::store::refunds::RefundRecord::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"refund_id", &app::store::refunds::RefundRecord::get_refund_id, &app::store::refunds::RefundRecord::set_refund_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_id", &app::store::refunds::RefundRecord::get_payment_id, &app::store::refunds::RefundRecord::set_payment_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::store::refunds::RefundRecord::get_merchant_id, &app::store::refunds::RefundRecord::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"amount", &app::store::refunds::RefundRecord::get_amount, &app::store::refunds::RefundRecord::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::store::refunds::RefundRecord::get_currency, &app::store::refunds::RefundRecord::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::store::refunds::RefundRecord::get_status, &app::store::refunds::RefundRecord::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"reason", &app::store::refunds::RefundRecord::get_reason, &app::store::refunds::RefundRecord::set_reason, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &app::store::refunds::RefundRecord::get_connector, &app::store::refunds::RefundRecord::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::store::refunds::RefundRecord::get_created_at, &app::store::refunds::RefundRecord::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"updated_at", &app::store::refunds::RefundRecord::get_updated_at, &app::store::refunds::RefundRecord::set_updated_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "refunds"; }
};

#ifdef CONGELADO_TEST
namespace app::store::refunds::tests {

using namespace boost::ut;

suite<"app_store_refund_record"> refund_record_suite = []
{
    "stores refund fields and reports its table"_test = []
    {
        RefundRecord record{};
        record.set_refund_id("rf_test");
        record.set_payment_id("pi_test");
        record.set_amount(900);

        expect(record.get_refund_id() == "rf_test");
        expect(record.get_payment_id() == "pi_test");
        expect(record.get_amount() == 900);
        expect(serde::Serializable<RefundRecord>::table_name() == "refunds");
    };
};

} // namespace app::store::refunds::tests
#endif
