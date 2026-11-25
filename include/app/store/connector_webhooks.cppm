module;
export module app_store:connector_webhooks;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::connector_webhooks {

/** @brief Persisted connector webhook mirror of the upstream ConnectorWebhook api model. */
class ConnectorWebhookRecord
{
public:
    ConnectorWebhookRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_connector_webhook_id(std::string value)
    {
        m_connector_webhook_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_webhook_id() const noexcept
    {
        return m_connector_webhook_id;
    }

    void set_connector(std::string value)
    {
        m_connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
    {
        return m_connector;
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

    void set_scope(std::string value)
    {
        m_scope = std::move(value);
    }

    [[nodiscard]] const std::string& get_scope() const noexcept
    {
        return m_scope;
    }

    void set_secret(std::string value)
    {
        m_secret = std::move(value);
    }

    [[nodiscard]] const std::string& get_secret() const noexcept
    {
        return m_secret;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_error_code(std::string value)
    {
        m_error_code = std::move(value);
    }

    [[nodiscard]] const std::string& get_error_code() const noexcept
    {
        return m_error_code;
    }

    void set_error_message(std::string value)
    {
        m_error_message = std::move(value);
    }

    [[nodiscard]] const std::string& get_error_message() const noexcept
    {
        return m_error_message;
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
    std::string m_connector_webhook_id{};
    std::string m_connector{};
    std::string m_merchant_id{};
    std::string m_event_type{};
    std::string m_scope{};
    std::string m_secret{};
    std::string m_status{};
    std::string m_error_code{};
    std::string m_error_message{};
    std::string m_created_at{};
};

} // namespace app::store::connector_webhooks

template<>
struct serde::Serializable<app::store::connector_webhooks::ConnectorWebhookRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::connector_webhooks::ConnectorWebhookRecord::get_id,
                &app::store::connector_webhooks::ConnectorWebhookRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "connector_webhook_id",
                &app::store::connector_webhooks::ConnectorWebhookRecord::get_connector_webhook_id,
                &app::store::connector_webhooks::ConnectorWebhookRecord::set_connector_webhook_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::store::connector_webhooks::ConnectorWebhookRecord::get_connector,
                &app::store::connector_webhooks::ConnectorWebhookRecord::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::connector_webhooks::ConnectorWebhookRecord::get_merchant_id,
                &app::store::connector_webhooks::ConnectorWebhookRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "event_type",
                &app::store::connector_webhooks::ConnectorWebhookRecord::get_event_type,
                &app::store::connector_webhooks::ConnectorWebhookRecord::set_event_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "scope",
                &app::store::connector_webhooks::ConnectorWebhookRecord::get_scope,
                &app::store::connector_webhooks::ConnectorWebhookRecord::set_scope,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "secret",
                &app::store::connector_webhooks::ConnectorWebhookRecord::get_secret,
                &app::store::connector_webhooks::ConnectorWebhookRecord::set_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::connector_webhooks::ConnectorWebhookRecord::get_status,
                &app::store::connector_webhooks::ConnectorWebhookRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &app::store::connector_webhooks::ConnectorWebhookRecord::get_error_code,
                &app::store::connector_webhooks::ConnectorWebhookRecord::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::store::connector_webhooks::ConnectorWebhookRecord::get_error_message,
                &app::store::connector_webhooks::ConnectorWebhookRecord::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::connector_webhooks::ConnectorWebhookRecord::get_created_at,
                &app::store::connector_webhooks::ConnectorWebhookRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "connector_webhooks";
    }
};

#ifdef CONGELADO_TEST
namespace app::store::connector_webhooks::tests {

using namespace boost::ut;

suite<"app_store_connector_webhook_record"> connector_webhook_record_suite = []
{
    "stores connector webhook fields and reports its table"_test = []
    {
        ConnectorWebhookRecord record{};
        record.set_connector_webhook_id("cwh_test");
        record.set_connector("stripe");
        record.set_event_type("payment_intent.succeeded");

        expect(record.get_connector_webhook_id() == "cwh_test");
        expect(record.get_connector() == "stripe");
        expect(record.get_event_type() == "payment_intent.succeeded");
        expect(serde::Serializable<ConnectorWebhookRecord>::table_name() == "connector_webhooks");
    };
};

} // namespace app::store::connector_webhooks::tests
#endif
