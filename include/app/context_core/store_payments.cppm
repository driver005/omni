module;
export module app_context_core:store_payments;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::payments {

/** @brief Persisted payment intent mirror of the upstream PaymentIntent api model. */
class PaymentIntentRecord
{
public:
    PaymentIntentRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_payment_id(std::string value)
    {
        m_payment_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_id() const noexcept
    {
        return m_payment_id;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_merchant_reference_id(std::string value)
    {
        m_merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_reference_id() const noexcept
    {
        return m_merchant_reference_id;
    }

    void set_customer_id(std::string value)
    {
        m_customer_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_customer_id() const noexcept
    {
        return m_customer_id;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
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

    void set_updated_at(std::string value)
    {
        m_updated_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_updated_at() const noexcept
    {
        return m_updated_at;
    }

    std::int64_t m_id{};
    std::string m_payment_id{};
    std::string m_merchant_id{};
    std::string m_merchant_reference_id{};
    std::string m_customer_id{};
    std::string m_status{};
    std::int64_t m_amount{};
    std::string m_currency{};
    std::string m_description{};
    std::string m_metadata{};
    std::string m_created_at{};
    std::string m_updated_at{};
};

} // namespace app::store::payments

template<>
struct serde::Serializable<app::store::payments::PaymentIntentRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::payments::PaymentIntentRecord::get_id,
                &app::store::payments::PaymentIntentRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "payment_id",
                &app::store::payments::PaymentIntentRecord::get_payment_id,
                &app::store::payments::PaymentIntentRecord::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::payments::PaymentIntentRecord::get_merchant_id,
                &app::store::payments::PaymentIntentRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::store::payments::PaymentIntentRecord::get_merchant_reference_id,
                &app::store::payments::PaymentIntentRecord::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::store::payments::PaymentIntentRecord::get_customer_id,
                &app::store::payments::PaymentIntentRecord::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::payments::PaymentIntentRecord::get_status,
                &app::store::payments::PaymentIntentRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::store::payments::PaymentIntentRecord::get_amount,
                &app::store::payments::PaymentIntentRecord::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::store::payments::PaymentIntentRecord::get_currency,
                &app::store::payments::PaymentIntentRecord::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::store::payments::PaymentIntentRecord::get_description,
                &app::store::payments::PaymentIntentRecord::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::store::payments::PaymentIntentRecord::get_metadata,
                &app::store::payments::PaymentIntentRecord::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::payments::PaymentIntentRecord::get_created_at,
                &app::store::payments::PaymentIntentRecord::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "updated_at",
                &app::store::payments::PaymentIntentRecord::get_updated_at,
                &app::store::payments::PaymentIntentRecord::set_updated_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_intents";
    }
};

#ifdef CONGELADO_TEST
namespace app::store::payments::tests {

using namespace boost::ut;

suite<"app_store_payment_intent_record"> payment_intent_record_suite = []
{
    "stores payment intent fields and reports its table"_test = []
    {
        PaymentIntentRecord record{};
        record.set_payment_id("pi_test");
        record.set_amount(4'200);
        record.set_status("requires_capture");

        expect(record.get_payment_id() == "pi_test");
        expect(record.get_amount() == 4'200);
        expect(record.get_status() == "requires_capture");
        expect(serde::Serializable<PaymentIntentRecord>::table_name() == "payment_intents");
    };
};

} // namespace app::store::payments::tests
#endif
