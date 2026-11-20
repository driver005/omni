module;
export module app_store:payment_records;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::payment_attempts {

/** @brief Persisted connector attempt mirror of the upstream PaymentAttempt model. */
class PaymentAttemptRecord
{
public:
    PaymentAttemptRecord() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_attempt_id(std::string value) { m_attempt_id = std::move(value); }
    [[nodiscard]] const std::string& get_attempt_id() const noexcept { return m_attempt_id; }
    void set_payment_id(std::string value) { m_payment_id = std::move(value); }
    [[nodiscard]] const std::string& get_payment_id() const noexcept { return m_payment_id; }
    void set_merchant_id(std::string value) { m_merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return m_merchant_id; }
    void set_connector(std::string value) { m_connector = std::move(value); }
    [[nodiscard]] const std::string& get_connector() const noexcept { return m_connector; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_amount(std::int64_t value) { m_amount = std::move(value); }
    [[nodiscard]] const std::int64_t& get_amount() const noexcept { return m_amount; }
    void set_currency(std::string value) { m_currency = std::move(value); }
    [[nodiscard]] const std::string& get_currency() const noexcept { return m_currency; }
    void set_error_code(std::string value) { m_error_code = std::move(value); }
    [[nodiscard]] const std::string& get_error_code() const noexcept { return m_error_code; }
    void set_error_message(std::string value) { m_error_message = std::move(value); }
    [[nodiscard]] const std::string& get_error_message() const noexcept { return m_error_message; }
    void set_created_at(std::string value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::string& get_created_at() const noexcept { return m_created_at; }

    std::int64_t m_id{};
    std::string m_attempt_id{};
    std::string m_payment_id{};
    std::string m_merchant_id{};
    std::string m_connector{};
    std::string m_status{};
    std::int64_t m_amount{};
    std::string m_currency{};
    std::string m_error_code{};
    std::string m_error_message{};
    std::string m_created_at{};
};

} // namespace app::store::payment_attempts

template<>
struct serde::Serializable<app::store::payment_attempts::PaymentAttemptRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::store::payment_attempts::PaymentAttemptRecord::get_id, &app::store::payment_attempts::PaymentAttemptRecord::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"attempt_id", &app::store::payment_attempts::PaymentAttemptRecord::get_attempt_id, &app::store::payment_attempts::PaymentAttemptRecord::set_attempt_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_id", &app::store::payment_attempts::PaymentAttemptRecord::get_payment_id, &app::store::payment_attempts::PaymentAttemptRecord::set_payment_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::store::payment_attempts::PaymentAttemptRecord::get_merchant_id, &app::store::payment_attempts::PaymentAttemptRecord::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &app::store::payment_attempts::PaymentAttemptRecord::get_connector, &app::store::payment_attempts::PaymentAttemptRecord::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::store::payment_attempts::PaymentAttemptRecord::get_status, &app::store::payment_attempts::PaymentAttemptRecord::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"amount", &app::store::payment_attempts::PaymentAttemptRecord::get_amount, &app::store::payment_attempts::PaymentAttemptRecord::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::store::payment_attempts::PaymentAttemptRecord::get_currency, &app::store::payment_attempts::PaymentAttemptRecord::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_code", &app::store::payment_attempts::PaymentAttemptRecord::get_error_code, &app::store::payment_attempts::PaymentAttemptRecord::set_error_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_message", &app::store::payment_attempts::PaymentAttemptRecord::get_error_message, &app::store::payment_attempts::PaymentAttemptRecord::set_error_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::store::payment_attempts::PaymentAttemptRecord::get_created_at, &app::store::payment_attempts::PaymentAttemptRecord::set_created_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payment_attempts"; }
};

export namespace app::store::payout_attempts {

/** @brief Persisted payout attempt mirror of the upstream PayoutAttempt model. */
class PayoutAttemptRecord
{
public:
    PayoutAttemptRecord() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_attempt_id(std::string value) { m_attempt_id = std::move(value); }
    [[nodiscard]] const std::string& get_attempt_id() const noexcept { return m_attempt_id; }
    void set_payout_id(std::string value) { m_payout_id = std::move(value); }
    [[nodiscard]] const std::string& get_payout_id() const noexcept { return m_payout_id; }
    void set_merchant_id(std::string value) { m_merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return m_merchant_id; }
    void set_connector(std::string value) { m_connector = std::move(value); }
    [[nodiscard]] const std::string& get_connector() const noexcept { return m_connector; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_amount(std::int64_t value) { m_amount = std::move(value); }
    [[nodiscard]] const std::int64_t& get_amount() const noexcept { return m_amount; }
    void set_currency(std::string value) { m_currency = std::move(value); }
    [[nodiscard]] const std::string& get_currency() const noexcept { return m_currency; }
    void set_error_code(std::string value) { m_error_code = std::move(value); }
    [[nodiscard]] const std::string& get_error_code() const noexcept { return m_error_code; }
    void set_error_message(std::string value) { m_error_message = std::move(value); }
    [[nodiscard]] const std::string& get_error_message() const noexcept { return m_error_message; }
    void set_created_at(std::string value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::string& get_created_at() const noexcept { return m_created_at; }

    std::int64_t m_id{};
    std::string m_attempt_id{};
    std::string m_payout_id{};
    std::string m_merchant_id{};
    std::string m_connector{};
    std::string m_status{};
    std::int64_t m_amount{};
    std::string m_currency{};
    std::string m_error_code{};
    std::string m_error_message{};
    std::string m_created_at{};
};

} // namespace app::store::payout_attempts

template<>
struct serde::Serializable<app::store::payout_attempts::PayoutAttemptRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::store::payout_attempts::PayoutAttemptRecord::get_id, &app::store::payout_attempts::PayoutAttemptRecord::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"attempt_id", &app::store::payout_attempts::PayoutAttemptRecord::get_attempt_id, &app::store::payout_attempts::PayoutAttemptRecord::set_attempt_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payout_id", &app::store::payout_attempts::PayoutAttemptRecord::get_payout_id, &app::store::payout_attempts::PayoutAttemptRecord::set_payout_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::store::payout_attempts::PayoutAttemptRecord::get_merchant_id, &app::store::payout_attempts::PayoutAttemptRecord::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &app::store::payout_attempts::PayoutAttemptRecord::get_connector, &app::store::payout_attempts::PayoutAttemptRecord::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::store::payout_attempts::PayoutAttemptRecord::get_status, &app::store::payout_attempts::PayoutAttemptRecord::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"amount", &app::store::payout_attempts::PayoutAttemptRecord::get_amount, &app::store::payout_attempts::PayoutAttemptRecord::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::store::payout_attempts::PayoutAttemptRecord::get_currency, &app::store::payout_attempts::PayoutAttemptRecord::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_code", &app::store::payout_attempts::PayoutAttemptRecord::get_error_code, &app::store::payout_attempts::PayoutAttemptRecord::set_error_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_message", &app::store::payout_attempts::PayoutAttemptRecord::get_error_message, &app::store::payout_attempts::PayoutAttemptRecord::set_error_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::store::payout_attempts::PayoutAttemptRecord::get_created_at, &app::store::payout_attempts::PayoutAttemptRecord::set_created_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payout_attempts"; }
};

export namespace app::store::payment_links {

/** @brief Persisted payment link mirror of the upstream PaymentLink model. */
class PaymentLinkRecord
{
public:
    PaymentLinkRecord() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_link_id(std::string value) { m_link_id = std::move(value); }
    [[nodiscard]] const std::string& get_link_id() const noexcept { return m_link_id; }
    void set_merchant_id(std::string value) { m_merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return m_merchant_id; }
    void set_profile_id(std::string value) { m_profile_id = std::move(value); }
    [[nodiscard]] const std::string& get_profile_id() const noexcept { return m_profile_id; }
    void set_customer_id(std::string value) { m_customer_id = std::move(value); }
    [[nodiscard]] const std::string& get_customer_id() const noexcept { return m_customer_id; }
    void set_amount(std::int64_t value) { m_amount = std::move(value); }
    [[nodiscard]] const std::int64_t& get_amount() const noexcept { return m_amount; }
    void set_currency(std::string value) { m_currency = std::move(value); }
    [[nodiscard]] const std::string& get_currency() const noexcept { return m_currency; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_url(std::string value) { m_url = std::move(value); }
    [[nodiscard]] const std::string& get_url() const noexcept { return m_url; }
    void set_expires_at(std::string value) { m_expires_at = std::move(value); }
    [[nodiscard]] const std::string& get_expires_at() const noexcept { return m_expires_at; }
    void set_created_at(std::string value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::string& get_created_at() const noexcept { return m_created_at; }

    std::int64_t m_id{};
    std::string m_link_id{};
    std::string m_merchant_id{};
    std::string m_profile_id{};
    std::string m_customer_id{};
    std::int64_t m_amount{};
    std::string m_currency{};
    std::string m_status{};
    std::string m_url{};
    std::string m_expires_at{};
    std::string m_created_at{};
};

} // namespace app::store::payment_links

template<>
struct serde::Serializable<app::store::payment_links::PaymentLinkRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::store::payment_links::PaymentLinkRecord::get_id, &app::store::payment_links::PaymentLinkRecord::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"link_id", &app::store::payment_links::PaymentLinkRecord::get_link_id, &app::store::payment_links::PaymentLinkRecord::set_link_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::store::payment_links::PaymentLinkRecord::get_merchant_id, &app::store::payment_links::PaymentLinkRecord::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::store::payment_links::PaymentLinkRecord::get_profile_id, &app::store::payment_links::PaymentLinkRecord::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_id", &app::store::payment_links::PaymentLinkRecord::get_customer_id, &app::store::payment_links::PaymentLinkRecord::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"amount", &app::store::payment_links::PaymentLinkRecord::get_amount, &app::store::payment_links::PaymentLinkRecord::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::store::payment_links::PaymentLinkRecord::get_currency, &app::store::payment_links::PaymentLinkRecord::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::store::payment_links::PaymentLinkRecord::get_status, &app::store::payment_links::PaymentLinkRecord::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"url", &app::store::payment_links::PaymentLinkRecord::get_url, &app::store::payment_links::PaymentLinkRecord::set_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"expires_at", &app::store::payment_links::PaymentLinkRecord::get_expires_at, &app::store::payment_links::PaymentLinkRecord::set_expires_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::store::payment_links::PaymentLinkRecord::get_created_at, &app::store::payment_links::PaymentLinkRecord::set_created_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "payment_links"; }
};

#ifdef CONGELADO_TEST
namespace app::store::payment_records::tests {

using namespace boost::ut;

suite<"app_store_payment_records"> payment_records_suite = []
{
    "payment attempt fields and table"_test = []
    {
        payment_attempts::PaymentAttemptRecord record{};
        record.set_attempt_id("att_1");
        record.set_payment_id("pi_1");
        expect(record.get_attempt_id() == "att_1");
        expect(record.get_payment_id() == "pi_1");
        expect(serde::Serializable<payment_attempts::PaymentAttemptRecord>::table_name() == "payment_attempts");
    };

    "payout attempt fields and table"_test = []
    {
        payout_attempts::PayoutAttemptRecord record{};
        record.set_attempt_id("patt_1");
        record.set_payout_id("po_1");
        expect(record.get_attempt_id() == "patt_1");
        expect(serde::Serializable<payout_attempts::PayoutAttemptRecord>::table_name() == "payout_attempts");
    };

    "payment link fields and table"_test = []
    {
        payment_links::PaymentLinkRecord record{};
        record.set_link_id("plink_1");
        record.set_amount(5000);
        expect(record.get_link_id() == "plink_1");
        expect(record.get_amount() == 5000);
        expect(serde::Serializable<payment_links::PaymentLinkRecord>::table_name() == "payment_links");
    };
};

} // namespace app::store::payment_records::tests
#endif
