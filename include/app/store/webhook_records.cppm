module;
export module app_store:webhook_records;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::webhooks {

/** @brief Persisted merchant webhook endpoint configuration. */
class WebhookRecord
{
public:
    WebhookRecord() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_webhook_id(std::string value) { m_webhook_id = std::move(value); }
    [[nodiscard]] const std::string& get_webhook_id() const noexcept { return m_webhook_id; }
    void set_merchant_id(std::string value) { m_merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return m_merchant_id; }
    void set_profile_id(std::string value) { m_profile_id = std::move(value); }
    [[nodiscard]] const std::string& get_profile_id() const noexcept { return m_profile_id; }
    void set_url(std::string value) { m_url = std::move(value); }
    [[nodiscard]] const std::string& get_url() const noexcept { return m_url; }
    void set_event_types(std::string value) { m_event_types = std::move(value); }
    [[nodiscard]] const std::string& get_event_types() const noexcept { return m_event_types; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_secret(std::string value) { m_secret = std::move(value); }
    [[nodiscard]] const std::string& get_secret() const noexcept { return m_secret; }
    void set_created_at(std::string value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::string& get_created_at() const noexcept { return m_created_at; }

    std::int64_t m_id{};
    std::string m_webhook_id{};
    std::string m_merchant_id{};
    std::string m_profile_id{};
    std::string m_url{};
    std::string m_event_types{};
    std::string m_status{};
    std::string m_secret{};
    std::string m_created_at{};
};

} // namespace app::store::webhooks

template<>
struct serde::Serializable<app::store::webhooks::WebhookRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::store::webhooks::WebhookRecord::get_id, &app::store::webhooks::WebhookRecord::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"webhook_id", &app::store::webhooks::WebhookRecord::get_webhook_id, &app::store::webhooks::WebhookRecord::set_webhook_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::store::webhooks::WebhookRecord::get_merchant_id, &app::store::webhooks::WebhookRecord::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::store::webhooks::WebhookRecord::get_profile_id, &app::store::webhooks::WebhookRecord::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"url", &app::store::webhooks::WebhookRecord::get_url, &app::store::webhooks::WebhookRecord::set_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"event_types", &app::store::webhooks::WebhookRecord::get_event_types, &app::store::webhooks::WebhookRecord::set_event_types, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::store::webhooks::WebhookRecord::get_status, &app::store::webhooks::WebhookRecord::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"secret", &app::store::webhooks::WebhookRecord::get_secret, &app::store::webhooks::WebhookRecord::set_secret, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::store::webhooks::WebhookRecord::get_created_at, &app::store::webhooks::WebhookRecord::set_created_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "webhooks"; }
};

export namespace app::store::webhook_delivery_attempts {

/** @brief Persisted delivery attempt for a webhook event. */
class WebhookDeliveryAttemptRecord
{
public:
    WebhookDeliveryAttemptRecord() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_attempt_id(std::string value) { m_attempt_id = std::move(value); }
    [[nodiscard]] const std::string& get_attempt_id() const noexcept { return m_attempt_id; }
    void set_event_id(std::string value) { m_event_id = std::move(value); }
    [[nodiscard]] const std::string& get_event_id() const noexcept { return m_event_id; }
    void set_merchant_id(std::string value) { m_merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return m_merchant_id; }
    void set_endpoint(std::string value) { m_endpoint = std::move(value); }
    [[nodiscard]] const std::string& get_endpoint() const noexcept { return m_endpoint; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_response_code(std::string value) { m_response_code = std::move(value); }
    [[nodiscard]] const std::string& get_response_code() const noexcept { return m_response_code; }
    void set_response_body(std::string value) { m_response_body = std::move(value); }
    [[nodiscard]] const std::string& get_response_body() const noexcept { return m_response_body; }
    void set_attempted_at(std::string value) { m_attempted_at = std::move(value); }
    [[nodiscard]] const std::string& get_attempted_at() const noexcept { return m_attempted_at; }

    std::int64_t m_id{};
    std::string m_attempt_id{};
    std::string m_event_id{};
    std::string m_merchant_id{};
    std::string m_endpoint{};
    std::string m_status{};
    std::string m_response_code{};
    std::string m_response_body{};
    std::string m_attempted_at{};
};

} // namespace app::store::webhook_delivery_attempts

template<>
struct serde::Serializable<app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord::get_id, &app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"attempt_id", &app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord::get_attempt_id, &app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord::set_attempt_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"event_id", &app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord::get_event_id, &app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord::set_event_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord::get_merchant_id, &app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"endpoint", &app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord::get_endpoint, &app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord::set_endpoint, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord::get_status, &app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"response_code", &app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord::get_response_code, &app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord::set_response_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"response_body", &app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord::get_response_body, &app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord::set_response_body, serde::FieldOptions{}>{},
            serde::FieldDesc<"attempted_at", &app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord::get_attempted_at, &app::store::webhook_delivery_attempts::WebhookDeliveryAttemptRecord::set_attempted_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "webhook_delivery_attempts"; }
};

#ifdef CONGELADO_TEST
namespace app::store::webhook_records::tests {

using namespace boost::ut;

suite<"app_store_webhook_records"> webhook_records_suite = []
{
    "webhook endpoint fields and table"_test = []
    {
        webhooks::WebhookRecord record{};
        record.set_webhook_id("wh_1");
        record.set_url("https://example.test/hook");
        expect(record.get_webhook_id() == "wh_1");
        expect(serde::Serializable<webhooks::WebhookRecord>::table_name() == "webhooks");
    };

    "delivery attempt fields and table"_test = []
    {
        webhook_delivery_attempts::WebhookDeliveryAttemptRecord record{};
        record.set_attempt_id("wda_1");
        record.set_event_id("evt_1");
        expect(record.get_attempt_id() == "wda_1");
        expect(serde::Serializable<webhook_delivery_attempts::WebhookDeliveryAttemptRecord>::table_name() == "webhook_delivery_attempts");
    };
};

} // namespace app::store::webhook_records::tests
#endif
