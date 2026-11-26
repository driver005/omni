module;
export module app_context_core:store_webhook_events;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::webhook_events {

/** @brief Persisted webhook event mirror of the upstream Event api model. */
class WebhookEventRecord
{
public:
    WebhookEventRecord() = default;

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

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_event_type(std::string value)
    {
        m_event_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_event_type() const noexcept
    {
        return m_event_type;
    }

    void set_event_class(std::string value)
    {
        m_event_class = std::move(value);
    }

    [[nodiscard]] const std::string& get_event_class() const noexcept
    {
        return m_event_class;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_payload(std::string value)
    {
        m_payload = std::move(value);
    }

    [[nodiscard]] const std::string& get_payload() const noexcept
    {
        return m_payload;
    }

    void set_delivery_attempts(std::int64_t value)
    {
        m_delivery_attempts = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_delivery_attempts() const noexcept
    {
        return m_delivery_attempts;
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
    std::string m_merchant_id{};
    std::string m_event_type{};
    std::string m_event_class{};
    std::string m_status{};
    std::string m_payload{};
    std::int64_t m_delivery_attempts{};
    std::string m_created_at{};
};

} // namespace app::store::webhook_events

template<>
struct serde::Serializable<app::store::webhook_events::WebhookEventRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::webhook_events::WebhookEventRecord::get_id,
                &app::store::webhook_events::WebhookEventRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "event_id",
                &app::store::webhook_events::WebhookEventRecord::get_event_id,
                &app::store::webhook_events::WebhookEventRecord::set_event_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::webhook_events::WebhookEventRecord::get_merchant_id,
                &app::store::webhook_events::WebhookEventRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "event_type",
                &app::store::webhook_events::WebhookEventRecord::get_event_type,
                &app::store::webhook_events::WebhookEventRecord::set_event_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "event_class",
                &app::store::webhook_events::WebhookEventRecord::get_event_class,
                &app::store::webhook_events::WebhookEventRecord::set_event_class,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::webhook_events::WebhookEventRecord::get_status,
                &app::store::webhook_events::WebhookEventRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payload",
                &app::store::webhook_events::WebhookEventRecord::get_payload,
                &app::store::webhook_events::WebhookEventRecord::set_payload,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "delivery_attempts",
                &app::store::webhook_events::WebhookEventRecord::get_delivery_attempts,
                &app::store::webhook_events::WebhookEventRecord::set_delivery_attempts,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::webhook_events::WebhookEventRecord::get_created_at,
                &app::store::webhook_events::WebhookEventRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "webhook_events";
    }
};

#ifdef CONGELADO_TEST
namespace app::store::webhook_events::tests {

using namespace boost::ut;

suite<"app_store_webhook_event_record"> webhook_event_record_suite = []
{
    "stores webhook event fields and reports its table"_test = []
    {
        WebhookEventRecord record{};
        record.set_event_id("evt_test");
        record.set_event_type("payment_intent.succeeded");
        record.set_delivery_attempts(2);

        expect(record.get_event_id() == "evt_test");
        expect(record.get_event_type() == "payment_intent.succeeded");
        expect(record.get_delivery_attempts() == 2);
        expect(serde::Serializable<WebhookEventRecord>::table_name() == "webhook_events");
    };
};

} // namespace app::store::webhook_events::tests
#endif
