module;
export module app_context_core:store_payouts;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::payouts {

/** @brief Persisted payout mirror of the upstream Payout api model. */
class PayoutRecord
{
public:
    PayoutRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_payout_id(std::string value)
    {
        m_payout_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_payout_id() const noexcept
    {
        return m_payout_id;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_amount(std::int64_t value)
    {
        m_amount = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_amount() const noexcept
    {
        return m_amount;
    }

    void set_currency(std::string value)
    {
        m_currency = std::move(value);
    }

    [[nodiscard]] const std::string& get_currency() const noexcept
    {
        return m_currency;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_connector(std::string value)
    {
        m_connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
    {
        return m_connector;
    }

    void set_payout_type(std::string value)
    {
        m_payout_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_payout_type() const noexcept
    {
        return m_payout_type;
    }

    void set_customer_id(std::string value)
    {
        m_customer_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_customer_id() const noexcept
    {
        return m_customer_id;
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
    std::string m_payout_id{};
    std::string m_merchant_id{};
    std::int64_t m_amount{};
    std::string m_currency{};
    std::string m_status{};
    std::string m_connector{};
    std::string m_payout_type{};
    std::string m_customer_id{};
    std::string m_metadata{};
    std::string m_created_at{};
};

} // namespace app::store::payouts

template<>
struct serde::Serializable<app::store::payouts::PayoutRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::payouts::PayoutRecord::get_id,
                &app::store::payouts::PayoutRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "payout_id",
                &app::store::payouts::PayoutRecord::get_payout_id,
                &app::store::payouts::PayoutRecord::set_payout_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::payouts::PayoutRecord::get_merchant_id,
                &app::store::payouts::PayoutRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::store::payouts::PayoutRecord::get_amount,
                &app::store::payouts::PayoutRecord::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::store::payouts::PayoutRecord::get_currency,
                &app::store::payouts::PayoutRecord::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::payouts::PayoutRecord::get_status,
                &app::store::payouts::PayoutRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::store::payouts::PayoutRecord::get_connector,
                &app::store::payouts::PayoutRecord::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payout_type",
                &app::store::payouts::PayoutRecord::get_payout_type,
                &app::store::payouts::PayoutRecord::set_payout_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::store::payouts::PayoutRecord::get_customer_id,
                &app::store::payouts::PayoutRecord::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::store::payouts::PayoutRecord::get_metadata,
                &app::store::payouts::PayoutRecord::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::payouts::PayoutRecord::get_created_at,
                &app::store::payouts::PayoutRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payouts";
    }
};

#ifdef CONGELADO_TEST
namespace app::store::payouts::tests {

using namespace boost::ut;

suite<"app_store_payout_record"> payout_record_suite = []
{
    "stores payout fields and reports its table"_test = []
    {
        PayoutRecord record{};
        record.set_payout_id("po_test");
        record.set_merchant_id("merchant_test");
        record.set_amount(15'000);

        expect(record.get_payout_id() == "po_test");
        expect(record.get_merchant_id() == "merchant_test");
        expect(record.get_amount() == 15'000);
        expect(serde::Serializable<PayoutRecord>::table_name() == "payouts");
    };
};

} // namespace app::store::payouts::tests
#endif
