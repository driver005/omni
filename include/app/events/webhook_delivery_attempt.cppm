module;
export module app_events:webhook_delivery_attempt;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::events {

/** @brief One persisted delivery attempt for a webhook event. */
class WebhookDeliveryAttemptRecord
{
public:
    WebhookDeliveryAttemptRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_event_id(std::string value)
    {
        m_event_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_event_id() const noexcept
    {
        return m_event_id;
    }

    void set_endpoint(std::string value)
    {
        m_endpoint = std::move(value);
    }

    [[nodiscard]] const std::string& get_endpoint() const noexcept
    {
        return m_endpoint;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_response(std::string value)
    {
        m_response = std::move(value);
    }

    [[nodiscard]] const std::string& get_response() const noexcept
    {
        return m_response;
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
    std::string m_event_id{};
    std::string m_endpoint{};
    std::string m_status{};
    std::string m_response{};
    std::string m_created_at{};
};

} // namespace app::events

template<>
struct serde::Serializable<app::events::WebhookDeliveryAttemptRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::events::WebhookDeliveryAttemptRecord::get_id,
                &app::events::WebhookDeliveryAttemptRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "event_id",
                &app::events::WebhookDeliveryAttemptRecord::get_event_id,
                &app::events::WebhookDeliveryAttemptRecord::set_event_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "endpoint",
                &app::events::WebhookDeliveryAttemptRecord::get_endpoint,
                &app::events::WebhookDeliveryAttemptRecord::set_endpoint,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::events::WebhookDeliveryAttemptRecord::get_status,
                &app::events::WebhookDeliveryAttemptRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "response",
                &app::events::WebhookDeliveryAttemptRecord::get_response,
                &app::events::WebhookDeliveryAttemptRecord::set_response,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::events::WebhookDeliveryAttemptRecord::get_created_at,
                &app::events::WebhookDeliveryAttemptRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "webhook_delivery_attempts";
    }
};

#ifdef CONGELADO_TEST
namespace app::events::tests {

using namespace boost::ut;

suite<"app_events_webhook_delivery_attempt"> webhook_delivery_attempt_suite = []
{
    "stores an attempt and reports its table"_test = []
    {
        WebhookDeliveryAttemptRecord attempt{};
        attempt.set_event_id("evt_1");
        attempt.set_endpoint("https://merchant.example/hook");
        attempt.set_status("SUCCESS");

        expect(attempt.get_event_id() == "evt_1");
        expect(attempt.get_status() == "SUCCESS");
        expect(
            serde::Serializable<WebhookDeliveryAttemptRecord>::table_name() ==
            "webhook_delivery_attempts"
        );
    };
};

} // namespace app::events::tests
#endif
