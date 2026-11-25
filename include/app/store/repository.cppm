module;
export module app_store:repository;

import std;
import core_database;
import core_serde;
import core_error;
import :api_key_expiries;
import :api_keys;
import :connector_webhooks;
import :customers;
import :domain_records;
import :identity_records;
import :mandates;
import :merchant_accounts;
import :merchant_connectors;
import :ops_records;
import :organizations;
import :payment_records;
import :payments;
import :payouts;
import :refunds;
import :risk_records;
import :routing;
import :subscriptions;
import :users;
import :webhook_events;
import :webhook_records;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store {

/** @brief Value-object filters for payment list queries. */
class PaymentQuery
{
public:
    PaymentQuery& merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
        return *this;
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    PaymentQuery& merchant_reference_id(std::string value)
    {
        m_merchant_reference_id = std::move(value);
        return *this;
    }

    [[nodiscard]] const std::string& get_merchant_reference_id() const noexcept
    {
        return m_merchant_reference_id;
    }

    PaymentQuery& status(std::string value)
    {
        m_status = std::move(value);
        return *this;
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    PaymentQuery& limit(std::size_t value)
    {
        m_limit = value;
        return *this;
    }

    [[nodiscard]] std::size_t get_limit() const noexcept
    {
        return m_limit;
    }

    PaymentQuery& offset(std::size_t value)
    {
        m_offset = value;
        return *this;
    }

    [[nodiscard]] std::size_t get_offset() const noexcept
    {
        return m_offset;
    }

private:
    std::string m_merchant_id{};
    std::string m_merchant_reference_id{};
    std::string m_status{};
    std::size_t m_limit{0};
    std::size_t m_offset{0};
};

/** @brief Value-object filters for refund list queries. */
class RefundQuery
{
public:
    RefundQuery& payment_id(std::string value)
    {
        m_payment_id = std::move(value);
        return *this;
    }

    [[nodiscard]] const std::string& get_payment_id() const noexcept
    {
        return m_payment_id;
    }

    RefundQuery& merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
        return *this;
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    RefundQuery& status(std::string value)
    {
        m_status = std::move(value);
        return *this;
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    RefundQuery& limit(std::size_t value)
    {
        m_limit = value;
        return *this;
    }

    [[nodiscard]] std::size_t get_limit() const noexcept
    {
        return m_limit;
    }

    RefundQuery& offset(std::size_t value)
    {
        m_offset = value;
        return *this;
    }

    [[nodiscard]] std::size_t get_offset() const noexcept
    {
        return m_offset;
    }

private:
    std::string m_payment_id{};
    std::string m_merchant_id{};
    std::string m_status{};
    std::size_t m_limit{0};
    std::size_t m_offset{0};
};

/** @brief Value-object filters for webhook event list queries. */
class WebhookEventQuery
{
public:
    WebhookEventQuery& merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
        return *this;
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    WebhookEventQuery& event_id(std::string value)
    {
        m_event_id = std::move(value);
        return *this;
    }

    [[nodiscard]] const std::string& get_event_id() const noexcept
    {
        return m_event_id;
    }

    WebhookEventQuery& event_type(std::string value)
    {
        m_event_type = std::move(value);
        return *this;
    }

    [[nodiscard]] const std::string& get_event_type() const noexcept
    {
        return m_event_type;
    }

    WebhookEventQuery& limit(std::size_t value)
    {
        m_limit = value;
        return *this;
    }

    [[nodiscard]] std::size_t get_limit() const noexcept
    {
        return m_limit;
    }

    WebhookEventQuery& offset(std::size_t value)
    {
        m_offset = value;
        return *this;
    }

    [[nodiscard]] std::size_t get_offset() const noexcept
    {
        return m_offset;
    }

private:
    std::string m_merchant_id{};
    std::string m_event_id{};
    std::string m_event_type{};
    std::size_t m_limit{0};
    std::size_t m_offset{0};
};

/** @brief Value-object filters for mandate list queries. */
class MandateQuery
{
public:
    MandateQuery& customer_id(std::string value)
    {
        m_customer_id = std::move(value);
        return *this;
    }

    [[nodiscard]] const std::string& get_customer_id() const noexcept
    {
        return m_customer_id;
    }

    MandateQuery& merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
        return *this;
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    MandateQuery& status(std::string value)
    {
        m_status = std::move(value);
        return *this;
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    MandateQuery& limit(std::size_t value)
    {
        m_limit = value;
        return *this;
    }

    [[nodiscard]] std::size_t get_limit() const noexcept
    {
        return m_limit;
    }

    MandateQuery& offset(std::size_t value)
    {
        m_offset = value;
        return *this;
    }

    [[nodiscard]] std::size_t get_offset() const noexcept
    {
        return m_offset;
    }

private:
    std::string m_customer_id{};
    std::string m_merchant_id{};
    std::string m_status{};
    std::size_t m_limit{0};
    std::size_t m_offset{0};
};

/** @brief Value-object filters for subscription list queries. */
class SubscriptionQuery
{
public:
    SubscriptionQuery& customer_id(std::string value)
    {
        m_customer_id = std::move(value);
        return *this;
    }

    [[nodiscard]] const std::string& get_customer_id() const noexcept
    {
        return m_customer_id;
    }

    SubscriptionQuery& merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
        return *this;
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    SubscriptionQuery& status(std::string value)
    {
        m_status = std::move(value);
        return *this;
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    SubscriptionQuery& limit(std::size_t value)
    {
        m_limit = value;
        return *this;
    }

    [[nodiscard]] std::size_t get_limit() const noexcept
    {
        return m_limit;
    }

    SubscriptionQuery& offset(std::size_t value)
    {
        m_offset = value;
        return *this;
    }

    [[nodiscard]] std::size_t get_offset() const noexcept
    {
        return m_offset;
    }

private:
    std::string m_customer_id{};
    std::string m_merchant_id{};
    std::string m_status{};
    std::size_t m_limit{0};
    std::size_t m_offset{0};
};

/** @brief Value-object filters for process tracker list queries. */
class ProcessTrackerQuery
{
public:
    ProcessTrackerQuery& name(std::string value)
    {
        m_name = std::move(value);
        return *this;
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    ProcessTrackerQuery& status(std::string value)
    {
        m_status = std::move(value);
        return *this;
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    ProcessTrackerQuery& limit(std::size_t value)
    {
        m_limit = value;
        return *this;
    }

    [[nodiscard]] std::size_t get_limit() const noexcept
    {
        return m_limit;
    }

    ProcessTrackerQuery& offset(std::size_t value)
    {
        m_offset = value;
        return *this;
    }

    [[nodiscard]] std::size_t get_offset() const noexcept
    {
        return m_offset;
    }

private:
    std::string m_name{};
    std::string m_status{};
    std::size_t m_limit{0};
    std::size_t m_offset{0};
};

/** @brief Typed CRUD facade over the persisted app records; holds an injected Database handle. */
class AppRepository
{
public:
    explicit AppRepository(core::db::Database& db) :
        m_db{db}
    {
    }

    // ── Existing contract (signatures preserved) ───────────────────────────────

    payments::PaymentIntentRecord save_payment(payments::PaymentIntentRecord record)
    {
        return m_db.insert<payments::PaymentIntentRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<payments::PaymentIntentRecord> find_payment(std::int64_t id) const
    {
        return m_db.find<payments::PaymentIntentRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::vector<payments::PaymentIntentRecord> list_payments() const
    {
        return m_db.all<payments::PaymentIntentRecord>();
    }

    refunds::RefundRecord save_refund(refunds::RefundRecord record)
    {
        return m_db.insert<refunds::RefundRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<refunds::RefundRecord> find_refund(std::int64_t id) const
    {
        return m_db.find<refunds::RefundRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::vector<refunds::RefundRecord> list_refunds() const
    {
        return m_db.all<refunds::RefundRecord>();
    }

    customers::CustomerRecord save_customer(customers::CustomerRecord record)
    {
        return m_db.insert<customers::CustomerRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<customers::CustomerRecord> find_customer(std::int64_t id) const
    {
        return m_db.find<customers::CustomerRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::vector<customers::CustomerRecord> list_customers() const
    {
        return m_db.all<customers::CustomerRecord>();
    }

    api_keys::ApiKeyRecord save_api_key(api_keys::ApiKeyRecord record)
    {
        return m_db.insert<api_keys::ApiKeyRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<api_keys::ApiKeyRecord> find_api_key(std::int64_t id) const
    {
        return m_db.find<api_keys::ApiKeyRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::vector<api_keys::ApiKeyRecord> list_api_keys() const
    {
        return m_db.all<api_keys::ApiKeyRecord>();
    }

    organizations::OrganizationRecord save_organization(organizations::OrganizationRecord record)
    {
        return m_db.insert<organizations::OrganizationRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<organizations::OrganizationRecord>
    find_organization(std::int64_t id) const
    {
        return m_db.find<organizations::OrganizationRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::vector<organizations::OrganizationRecord> list_organizations() const
    {
        return m_db.all<organizations::OrganizationRecord>();
    }

    [[nodiscard]] std::optional<organizations::OrganizationRecord>
    find_organization_by_string(std::string_view organization_id) const
    {
        return find_by_string<organizations::OrganizationRecord>(
            "organization_id",
            organization_id
        );
    }

    bool update_organization(const organizations::OrganizationRecord& record)
    {
        return m_db.update_where_column<organizations::OrganizationRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_organization(std::int64_t id)
    {
        return m_db.remove<organizations::OrganizationRecord>(core::db::DbValue{id}) > 0;
    }

    merchant_accounts::MerchantAccountRecord
    save_merchant_account(merchant_accounts::MerchantAccountRecord record)
    {
        return m_db.insert<merchant_accounts::MerchantAccountRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<merchant_accounts::MerchantAccountRecord>
    find_merchant_account(std::int64_t id) const
    {
        return m_db.find<merchant_accounts::MerchantAccountRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::vector<merchant_accounts::MerchantAccountRecord>
    list_merchant_accounts() const
    {
        return m_db.all<merchant_accounts::MerchantAccountRecord>();
    }

    [[nodiscard]] std::optional<merchant_accounts::MerchantAccountRecord>
    find_merchant_account_by_string(std::string_view merchant_id) const
    {
        return find_by_string<merchant_accounts::MerchantAccountRecord>("merchant_id", merchant_id);
    }

    bool update_merchant_account(const merchant_accounts::MerchantAccountRecord& record)
    {
        return m_db.update_where_column<merchant_accounts::MerchantAccountRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_merchant_account(std::int64_t id)
    {
        return m_db.remove<merchant_accounts::MerchantAccountRecord>(core::db::DbValue{id}) > 0;
    }

    merchant_connectors::MerchantConnectorRecord
    save_merchant_connector(merchant_connectors::MerchantConnectorRecord record)
    {
        return m_db.insert<merchant_connectors::MerchantConnectorRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<merchant_connectors::MerchantConnectorRecord>
    find_merchant_connector(std::int64_t id) const
    {
        return m_db.find<merchant_connectors::MerchantConnectorRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::vector<merchant_connectors::MerchantConnectorRecord>
    list_merchant_connectors() const
    {
        return m_db.all<merchant_connectors::MerchantConnectorRecord>();
    }

    [[nodiscard]] std::optional<merchant_connectors::MerchantConnectorRecord>
    find_merchant_connector_by_string(std::string_view merchant_connector_id) const
    {
        return find_by_string<merchant_connectors::MerchantConnectorRecord>(
            "merchant_connector_id",
            merchant_connector_id
        );
    }

    bool update_merchant_connector(const merchant_connectors::MerchantConnectorRecord& record)
    {
        return m_db.update_where_column<merchant_connectors::MerchantConnectorRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_merchant_connector(std::int64_t id)
    {
        return m_db.remove<merchant_connectors::MerchantConnectorRecord>(core::db::DbValue{id}) > 0;
    }

    api_key_expiries::ApiKeyExpiryRecord
    save_api_key_expiry(api_key_expiries::ApiKeyExpiryRecord record)
    {
        return m_db.insert<api_key_expiries::ApiKeyExpiryRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<api_key_expiries::ApiKeyExpiryRecord>
    find_api_key_expiry(std::int64_t id) const
    {
        return m_db.find<api_key_expiries::ApiKeyExpiryRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::vector<api_key_expiries::ApiKeyExpiryRecord> list_api_key_expiries() const
    {
        return m_db.all<api_key_expiries::ApiKeyExpiryRecord>();
    }

    [[nodiscard]] std::optional<api_key_expiries::ApiKeyExpiryRecord>
    find_api_key_expiry_by_string(std::string_view key_id) const
    {
        return find_by_string<api_key_expiries::ApiKeyExpiryRecord>("key_id", key_id);
    }

    bool update_api_key_expiry(const api_key_expiries::ApiKeyExpiryRecord& record)
    {
        return m_db.update_where_column<api_key_expiries::ApiKeyExpiryRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_api_key_expiry(std::int64_t id)
    {
        return m_db.remove<api_key_expiries::ApiKeyExpiryRecord>(core::db::DbValue{id}) > 0;
    }

    webhook_events::WebhookEventRecord save_webhook_event(webhook_events::WebhookEventRecord record)
    {
        return m_db.insert<webhook_events::WebhookEventRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<webhook_events::WebhookEventRecord>
    find_webhook_event(std::int64_t id) const
    {
        return m_db.find<webhook_events::WebhookEventRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::vector<webhook_events::WebhookEventRecord> list_webhook_events() const
    {
        return m_db.all<webhook_events::WebhookEventRecord>();
    }

    [[nodiscard]] std::optional<webhook_events::WebhookEventRecord>
    find_webhook_event_by_string(std::string_view event_id) const
    {
        return find_by_string<webhook_events::WebhookEventRecord>("event_id", event_id);
    }

    bool update_webhook_event(const webhook_events::WebhookEventRecord& record)
    {
        return m_db.update_where_column<webhook_events::WebhookEventRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_webhook_event(std::int64_t id)
    {
        return m_db.remove<webhook_events::WebhookEventRecord>(core::db::DbValue{id}) > 0;
    }

    routing::RoutingConfigRecord save_routing_config(routing::RoutingConfigRecord record)
    {
        return m_db.insert<routing::RoutingConfigRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<routing::RoutingConfigRecord>
    find_routing_config(std::int64_t id) const
    {
        return m_db.find<routing::RoutingConfigRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::vector<routing::RoutingConfigRecord> list_routing_configs() const
    {
        return m_db.all<routing::RoutingConfigRecord>();
    }

    [[nodiscard]] std::optional<routing::RoutingConfigRecord>
    find_routing_config_by_string(std::string_view config_id) const
    {
        return find_by_string<routing::RoutingConfigRecord>("config_id", config_id);
    }

    bool update_routing_config(const routing::RoutingConfigRecord& record)
    {
        return m_db.update_where_column<routing::RoutingConfigRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_routing_config(std::int64_t id)
    {
        return m_db.remove<routing::RoutingConfigRecord>(core::db::DbValue{id}) > 0;
    }

    mandates::MandateRecord save_mandate(mandates::MandateRecord record)
    {
        return m_db.insert<mandates::MandateRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<mandates::MandateRecord> find_mandate(std::int64_t id) const
    {
        return m_db.find<mandates::MandateRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::vector<mandates::MandateRecord> list_mandates() const
    {
        return m_db.all<mandates::MandateRecord>();
    }

    [[nodiscard]] std::optional<mandates::MandateRecord>
    find_mandate_by_string(std::string_view mandate_id) const
    {
        return find_by_string<mandates::MandateRecord>("mandate_id", mandate_id);
    }

    bool update_mandate(const mandates::MandateRecord& record)
    {
        return m_db.update_where_column<mandates::MandateRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_mandate(std::int64_t id)
    {
        return m_db.remove<mandates::MandateRecord>(core::db::DbValue{id}) > 0;
    }

    subscriptions::SubscriptionRecord save_subscription(subscriptions::SubscriptionRecord record)
    {
        return m_db.insert<subscriptions::SubscriptionRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<subscriptions::SubscriptionRecord>
    find_subscription(std::int64_t id) const
    {
        return m_db.find<subscriptions::SubscriptionRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::vector<subscriptions::SubscriptionRecord> list_subscriptions() const
    {
        return m_db.all<subscriptions::SubscriptionRecord>();
    }

    [[nodiscard]] std::optional<subscriptions::SubscriptionRecord>
    find_subscription_by_string(std::string_view subscription_id) const
    {
        return find_by_string<subscriptions::SubscriptionRecord>(
            "subscription_id",
            subscription_id
        );
    }

    bool update_subscription(const subscriptions::SubscriptionRecord& record)
    {
        return m_db.update_where_column<subscriptions::SubscriptionRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_subscription(std::int64_t id)
    {
        return m_db.remove<subscriptions::SubscriptionRecord>(core::db::DbValue{id}) > 0;
    }

    payouts::PayoutRecord save_payout(payouts::PayoutRecord record)
    {
        return m_db.insert<payouts::PayoutRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<payouts::PayoutRecord> find_payout(std::int64_t id) const
    {
        return m_db.find<payouts::PayoutRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::vector<payouts::PayoutRecord> list_payouts() const
    {
        return m_db.all<payouts::PayoutRecord>();
    }

    [[nodiscard]] std::optional<payouts::PayoutRecord>
    find_payout_by_string(std::string_view payout_id) const
    {
        return find_by_string<payouts::PayoutRecord>("payout_id", payout_id);
    }

    bool update_payout(const payouts::PayoutRecord& record)
    {
        return m_db.update_where_column<payouts::PayoutRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_payout(std::int64_t id)
    {
        return m_db.remove<payouts::PayoutRecord>(core::db::DbValue{id}) > 0;
    }

    connector_webhooks::ConnectorWebhookRecord
    save_connector_webhook(connector_webhooks::ConnectorWebhookRecord record)
    {
        return m_db.insert<connector_webhooks::ConnectorWebhookRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<connector_webhooks::ConnectorWebhookRecord>
    find_connector_webhook(std::int64_t id) const
    {
        return m_db.find<connector_webhooks::ConnectorWebhookRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::vector<connector_webhooks::ConnectorWebhookRecord>
    list_connector_webhooks() const
    {
        return m_db.all<connector_webhooks::ConnectorWebhookRecord>();
    }

    [[nodiscard]] std::optional<connector_webhooks::ConnectorWebhookRecord>
    find_connector_webhook_by_string(std::string_view connector_webhook_id) const
    {
        return find_by_string<connector_webhooks::ConnectorWebhookRecord>(
            "connector_webhook_id",
            connector_webhook_id
        );
    }

    bool update_connector_webhook(const connector_webhooks::ConnectorWebhookRecord& record)
    {
        return m_db.update_where_column<connector_webhooks::ConnectorWebhookRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_connector_webhook(std::int64_t id)
    {
        return m_db.remove<connector_webhooks::ConnectorWebhookRecord>(core::db::DbValue{id}) > 0;
    }

    // ── Completing CRUD for the original four facade entities ─────────────────

    [[nodiscard]] std::optional<payments::PaymentIntentRecord>
    find_payment_by_string(std::string_view payment_id) const
    {
        return find_by_string<payments::PaymentIntentRecord>("payment_id", payment_id);
    }

    bool update_payment(const payments::PaymentIntentRecord& record)
    {
        return m_db.update_where_column<payments::PaymentIntentRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_payment(std::int64_t id)
    {
        return m_db.remove<payments::PaymentIntentRecord>(core::db::DbValue{id}) > 0;
    }

    [[nodiscard]] std::optional<refunds::RefundRecord>
    find_refund_by_string(std::string_view refund_id) const
    {
        return find_by_string<refunds::RefundRecord>("refund_id", refund_id);
    }

    bool update_refund(const refunds::RefundRecord& record)
    {
        return m_db.update_where_column<refunds::RefundRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_refund(std::int64_t id)
    {
        return m_db.remove<refunds::RefundRecord>(core::db::DbValue{id}) > 0;
    }

    [[nodiscard]] std::optional<customers::CustomerRecord>
    find_customer_by_string(std::string_view customer_id) const
    {
        return find_by_string<customers::CustomerRecord>("customer_id", customer_id);
    }

    bool update_customer(const customers::CustomerRecord& record)
    {
        return m_db.update_where_column<customers::CustomerRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_customer(std::int64_t id)
    {
        return m_db.remove<customers::CustomerRecord>(core::db::DbValue{id}) > 0;
    }

    [[nodiscard]] std::optional<api_keys::ApiKeyRecord>
    find_api_key_by_string(std::string_view key_id) const
    {
        return find_by_string<api_keys::ApiKeyRecord>("key_id", key_id);
    }

    bool update_api_key(const api_keys::ApiKeyRecord& record)
    {
        return m_db.update_where_column<api_keys::ApiKeyRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_api_key(std::int64_t id)
    {
        return m_db.remove<api_keys::ApiKeyRecord>(core::db::DbValue{id}) > 0;
    }

    [[nodiscard]] std::optional<users::UserRecord>
    find_user_by_string(std::string_view user_id) const
    {
        return find_by_string<users::UserRecord>("user_id", user_id);
    }

    bool update_user(const users::UserRecord& record)
    {
        return m_db.update_where_column<users::UserRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_user(std::int64_t id)
    {
        return m_db.remove<users::UserRecord>(core::db::DbValue{id}) > 0;
    }

    // ── CRUD for the new persisted entities ───────────────────────────────────

    payment_attempts::PaymentAttemptRecord
    save_payment_attempt(payment_attempts::PaymentAttemptRecord record)
    {
        return m_db.insert<payment_attempts::PaymentAttemptRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<payment_attempts::PaymentAttemptRecord>
    find_payment_attempt(std::int64_t id) const
    {
        return m_db.find<payment_attempts::PaymentAttemptRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<payment_attempts::PaymentAttemptRecord>
    find_payment_attempt_by_string(std::string_view attempt_id) const
    {
        return find_by_string<payment_attempts::PaymentAttemptRecord>("attempt_id", attempt_id);
    }

    [[nodiscard]] std::vector<payment_attempts::PaymentAttemptRecord> list_payment_attempts() const
    {
        return m_db.all<payment_attempts::PaymentAttemptRecord>();
    }

    bool update_payment_attempt(const payment_attempts::PaymentAttemptRecord& record)
    {
        return m_db.update_where_column<payment_attempts::PaymentAttemptRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_payment_attempt(std::int64_t id)
    {
        return m_db.remove<payment_attempts::PaymentAttemptRecord>(core::db::DbValue{id}) > 0;
    }

    payout_attempts::PayoutAttemptRecord
    save_payout_attempt(payout_attempts::PayoutAttemptRecord record)
    {
        return m_db.insert<payout_attempts::PayoutAttemptRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<payout_attempts::PayoutAttemptRecord>
    find_payout_attempt(std::int64_t id) const
    {
        return m_db.find<payout_attempts::PayoutAttemptRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<payout_attempts::PayoutAttemptRecord>
    find_payout_attempt_by_string(std::string_view attempt_id) const
    {
        return find_by_string<payout_attempts::PayoutAttemptRecord>("attempt_id", attempt_id);
    }

    [[nodiscard]] std::vector<payout_attempts::PayoutAttemptRecord> list_payout_attempts() const
    {
        return m_db.all<payout_attempts::PayoutAttemptRecord>();
    }

    bool update_payout_attempt(const payout_attempts::PayoutAttemptRecord& record)
    {
        return m_db.update_where_column<payout_attempts::PayoutAttemptRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_payout_attempt(std::int64_t id)
    {
        return m_db.remove<payout_attempts::PayoutAttemptRecord>(core::db::DbValue{id}) > 0;
    }

    payment_links::PaymentLinkRecord save_payment_link(payment_links::PaymentLinkRecord record)
    {
        return m_db.insert<payment_links::PaymentLinkRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<payment_links::PaymentLinkRecord>
    find_payment_link(std::int64_t id) const
    {
        return m_db.find<payment_links::PaymentLinkRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<payment_links::PaymentLinkRecord>
    find_payment_link_by_string(std::string_view link_id) const
    {
        return find_by_string<payment_links::PaymentLinkRecord>("link_id", link_id);
    }

    [[nodiscard]] std::vector<payment_links::PaymentLinkRecord> list_payment_links() const
    {
        return m_db.all<payment_links::PaymentLinkRecord>();
    }

    bool update_payment_link(const payment_links::PaymentLinkRecord& record)
    {
        return m_db.update_where_column<payment_links::PaymentLinkRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_payment_link(std::int64_t id)
    {
        return m_db.remove<payment_links::PaymentLinkRecord>(core::db::DbValue{id}) > 0;
    }

    webhooks::WebhookRecord save_webhook(webhooks::WebhookRecord record)
    {
        return m_db.insert<webhooks::WebhookRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<webhooks::WebhookRecord> find_webhook(std::int64_t id) const
    {
        return m_db.find<webhooks::WebhookRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<webhooks::WebhookRecord>
    find_webhook_by_string(std::string_view webhook_id) const
    {
        return find_by_string<webhooks::WebhookRecord>("webhook_id", webhook_id);
    }

    [[nodiscard]] std::vector<webhooks::WebhookRecord> list_webhooks() const
    {
        return m_db.all<webhooks::WebhookRecord>();
    }

    bool update_webhook(const webhooks::WebhookRecord& record)
    {
        return m_db.update_where_column<webhooks::WebhookRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_webhook(std::int64_t id)
    {
        return m_db.remove<webhooks::WebhookRecord>(core::db::DbValue{id}) > 0;
    }

    webhook_delivery_attempts::WebhookDeliveryAttemptRecord
    save_webhook_delivery_attempt(webhook_delivery_attempts::WebhookDeliveryAttemptRecord record)
    {
        return m_db.insert<webhook_delivery_attempts::WebhookDeliveryAttemptRecord>(
            std::move(record)
        );
    }

    [[nodiscard]] std::optional<webhook_delivery_attempts::WebhookDeliveryAttemptRecord>
    find_webhook_delivery_attempt(std::int64_t id) const
    {
        return m_db.find<webhook_delivery_attempts::WebhookDeliveryAttemptRecord>(
            core::db::DbValue{id}
        );
    }

    [[nodiscard]] std::optional<webhook_delivery_attempts::WebhookDeliveryAttemptRecord>
    find_webhook_delivery_attempt_by_string(std::string_view attempt_id) const
    {
        return find_by_string<webhook_delivery_attempts::WebhookDeliveryAttemptRecord>(
            "attempt_id",
            attempt_id
        );
    }

    [[nodiscard]] std::vector<webhook_delivery_attempts::WebhookDeliveryAttemptRecord>
    list_webhook_delivery_attempts() const
    {
        return m_db.all<webhook_delivery_attempts::WebhookDeliveryAttemptRecord>();
    }

    bool update_webhook_delivery_attempt(
        const webhook_delivery_attempts::WebhookDeliveryAttemptRecord& record
    )
    {
        return m_db.update_where_column<webhook_delivery_attempts::WebhookDeliveryAttemptRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_webhook_delivery_attempt(std::int64_t id)
    {
        return m_db.remove<webhook_delivery_attempts::WebhookDeliveryAttemptRecord>(
                   core::db::DbValue{id}
               ) > 0;
    }

    roles::RoleRecord save_role(roles::RoleRecord record)
    {
        return m_db.insert<roles::RoleRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<roles::RoleRecord> find_role(std::int64_t id) const
    {
        return m_db.find<roles::RoleRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<roles::RoleRecord>
    find_role_by_string(std::string_view role_id) const
    {
        return find_by_string<roles::RoleRecord>("role_id", role_id);
    }

    [[nodiscard]] std::vector<roles::RoleRecord> list_roles() const
    {
        return m_db.all<roles::RoleRecord>();
    }

    bool update_role(const roles::RoleRecord& record)
    {
        return m_db.update_where_column<roles::RoleRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_role(std::int64_t id)
    {
        return m_db.remove<roles::RoleRecord>(core::db::DbValue{id}) > 0;
    }

    user_roles::UserRoleRecord save_user_role(user_roles::UserRoleRecord record)
    {
        return m_db.insert<user_roles::UserRoleRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<user_roles::UserRoleRecord> find_user_role(std::int64_t id) const
    {
        return m_db.find<user_roles::UserRoleRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<user_roles::UserRoleRecord>
    find_user_role_by_string(std::string_view user_role_id) const
    {
        return find_by_string<user_roles::UserRoleRecord>("user_role_id", user_role_id);
    }

    [[nodiscard]] std::vector<user_roles::UserRoleRecord> list_user_roles() const
    {
        return m_db.all<user_roles::UserRoleRecord>();
    }

    bool update_user_role(const user_roles::UserRoleRecord& record)
    {
        return m_db.update_where_column<user_roles::UserRoleRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_user_role(std::int64_t id)
    {
        return m_db.remove<user_roles::UserRoleRecord>(core::db::DbValue{id}) > 0;
    }

    authentication_records::AuthenticationRecord
    save_authentication_record(authentication_records::AuthenticationRecord record)
    {
        return m_db.insert<authentication_records::AuthenticationRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<authentication_records::AuthenticationRecord>
    find_authentication_record(std::int64_t id) const
    {
        return m_db.find<authentication_records::AuthenticationRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<authentication_records::AuthenticationRecord>
    find_authentication_record_by_string(std::string_view auth_id) const
    {
        return find_by_string<authentication_records::AuthenticationRecord>("auth_id", auth_id);
    }

    [[nodiscard]] std::vector<authentication_records::AuthenticationRecord>
    list_authentication_records() const
    {
        return m_db.all<authentication_records::AuthenticationRecord>();
    }

    bool update_authentication_record(const authentication_records::AuthenticationRecord& record)
    {
        return m_db.update_where_column<authentication_records::AuthenticationRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_authentication_record(std::int64_t id)
    {
        return m_db.remove<authentication_records::AuthenticationRecord>(core::db::DbValue{id}) > 0;
    }

    gsm_rules::GsmRuleRecord save_gsm_rule(gsm_rules::GsmRuleRecord record)
    {
        return m_db.insert<gsm_rules::GsmRuleRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<gsm_rules::GsmRuleRecord> find_gsm_rule(std::int64_t id) const
    {
        return m_db.find<gsm_rules::GsmRuleRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<gsm_rules::GsmRuleRecord>
    find_gsm_rule_by_string(std::string_view natural_key) const
    {
        return find_by_string<gsm_rules::GsmRuleRecord>("natural_key", natural_key);
    }

    [[nodiscard]] std::vector<gsm_rules::GsmRuleRecord> list_gsm_rules() const
    {
        return m_db.all<gsm_rules::GsmRuleRecord>();
    }

    bool update_gsm_rule(const gsm_rules::GsmRuleRecord& record)
    {
        return m_db.update_where_column<gsm_rules::GsmRuleRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_gsm_rule(std::int64_t id)
    {
        return m_db.remove<gsm_rules::GsmRuleRecord>(core::db::DbValue{id}) > 0;
    }

    blocklist_entries::BlocklistEntryRecord
    save_blocklist_entry(blocklist_entries::BlocklistEntryRecord record)
    {
        return m_db.insert<blocklist_entries::BlocklistEntryRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<blocklist_entries::BlocklistEntryRecord>
    find_blocklist_entry(std::int64_t id) const
    {
        return m_db.find<blocklist_entries::BlocklistEntryRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<blocklist_entries::BlocklistEntryRecord>
    find_blocklist_entry_by_string(std::string_view entry_id) const
    {
        return find_by_string<blocklist_entries::BlocklistEntryRecord>("entry_id", entry_id);
    }

    [[nodiscard]] std::vector<blocklist_entries::BlocklistEntryRecord>
    list_blocklist_entries() const
    {
        return m_db.all<blocklist_entries::BlocklistEntryRecord>();
    }

    bool update_blocklist_entry(const blocklist_entries::BlocklistEntryRecord& record)
    {
        return m_db.update_where_column<blocklist_entries::BlocklistEntryRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_blocklist_entry(std::int64_t id)
    {
        return m_db.remove<blocklist_entries::BlocklistEntryRecord>(core::db::DbValue{id}) > 0;
    }

    fraud_checks::FraudCheckRecord save_fraud_check(fraud_checks::FraudCheckRecord record)
    {
        return m_db.insert<fraud_checks::FraudCheckRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<fraud_checks::FraudCheckRecord>
    find_fraud_check(std::int64_t id) const
    {
        return m_db.find<fraud_checks::FraudCheckRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<fraud_checks::FraudCheckRecord>
    find_fraud_check_by_string(std::string_view fraud_check_id) const
    {
        return find_by_string<fraud_checks::FraudCheckRecord>("fraud_check_id", fraud_check_id);
    }

    [[nodiscard]] std::vector<fraud_checks::FraudCheckRecord> list_fraud_checks() const
    {
        return m_db.all<fraud_checks::FraudCheckRecord>();
    }

    bool update_fraud_check(const fraud_checks::FraudCheckRecord& record)
    {
        return m_db.update_where_column<fraud_checks::FraudCheckRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_fraud_check(std::int64_t id)
    {
        return m_db.remove<fraud_checks::FraudCheckRecord>(core::db::DbValue{id}) > 0;
    }

    decision_configs::DecisionConfigRecord
    save_decision_config(decision_configs::DecisionConfigRecord record)
    {
        return m_db.insert<decision_configs::DecisionConfigRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<decision_configs::DecisionConfigRecord>
    find_decision_config(std::int64_t id) const
    {
        return m_db.find<decision_configs::DecisionConfigRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<decision_configs::DecisionConfigRecord>
    find_decision_config_by_string(std::string_view config_id) const
    {
        return find_by_string<decision_configs::DecisionConfigRecord>("config_id", config_id);
    }

    [[nodiscard]] std::vector<decision_configs::DecisionConfigRecord> list_decision_configs() const
    {
        return m_db.all<decision_configs::DecisionConfigRecord>();
    }

    bool update_decision_config(const decision_configs::DecisionConfigRecord& record)
    {
        return m_db.update_where_column<decision_configs::DecisionConfigRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_decision_config(std::int64_t id)
    {
        return m_db.remove<decision_configs::DecisionConfigRecord>(core::db::DbValue{id}) > 0;
    }

    process_trackers::ProcessTrackerRecord
    save_process_tracker(process_trackers::ProcessTrackerRecord record)
    {
        return m_db.insert<process_trackers::ProcessTrackerRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<process_trackers::ProcessTrackerRecord>
    find_process_tracker(std::int64_t id) const
    {
        return m_db.find<process_trackers::ProcessTrackerRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<process_trackers::ProcessTrackerRecord>
    find_process_tracker_by_string(std::string_view task_id) const
    {
        return find_by_string<process_trackers::ProcessTrackerRecord>("task_id", task_id);
    }

    [[nodiscard]] std::vector<process_trackers::ProcessTrackerRecord> list_process_trackers() const
    {
        return m_db.all<process_trackers::ProcessTrackerRecord>();
    }

    bool update_process_tracker(const process_trackers::ProcessTrackerRecord& record)
    {
        return m_db.update_where_column<process_trackers::ProcessTrackerRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_process_tracker(std::int64_t id)
    {
        return m_db.remove<process_trackers::ProcessTrackerRecord>(core::db::DbValue{id}) > 0;
    }

    files::FileRecord save_file(files::FileRecord record)
    {
        return m_db.insert<files::FileRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<files::FileRecord> find_file(std::int64_t id) const
    {
        return m_db.find<files::FileRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<files::FileRecord>
    find_file_by_string(std::string_view file_id) const
    {
        return find_by_string<files::FileRecord>("file_id", file_id);
    }

    [[nodiscard]] std::vector<files::FileRecord> list_files() const
    {
        return m_db.all<files::FileRecord>();
    }

    bool update_file(const files::FileRecord& record)
    {
        return m_db.update_where_column<files::FileRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_file(std::int64_t id)
    {
        return m_db.remove<files::FileRecord>(core::db::DbValue{id}) > 0;
    }

    cards_info::CardInfoRecord save_card_info(cards_info::CardInfoRecord record)
    {
        return m_db.insert<cards_info::CardInfoRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<cards_info::CardInfoRecord> find_card_info(std::int64_t id) const
    {
        return m_db.find<cards_info::CardInfoRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<cards_info::CardInfoRecord>
    find_card_info_by_string(std::string_view iin) const
    {
        return find_by_string<cards_info::CardInfoRecord>("iin", iin);
    }

    [[nodiscard]] std::vector<cards_info::CardInfoRecord> list_cards_info() const
    {
        return m_db.all<cards_info::CardInfoRecord>();
    }

    bool update_card_info(const cards_info::CardInfoRecord& record)
    {
        return m_db.update_where_column<cards_info::CardInfoRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_card_info(std::int64_t id)
    {
        return m_db.remove<cards_info::CardInfoRecord>(core::db::DbValue{id}) > 0;
    }

    relay_records::RelayRecord save_relay_record(relay_records::RelayRecord record)
    {
        return m_db.insert<relay_records::RelayRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<relay_records::RelayRecord> find_relay_record(std::int64_t id) const
    {
        return m_db.find<relay_records::RelayRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<relay_records::RelayRecord>
    find_relay_record_by_string(std::string_view relay_id) const
    {
        return find_by_string<relay_records::RelayRecord>("relay_id", relay_id);
    }

    [[nodiscard]] std::vector<relay_records::RelayRecord> list_relay_records() const
    {
        return m_db.all<relay_records::RelayRecord>();
    }

    bool update_relay_record(const relay_records::RelayRecord& record)
    {
        return m_db.update_where_column<relay_records::RelayRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_relay_record(std::int64_t id)
    {
        return m_db.remove<relay_records::RelayRecord>(core::db::DbValue{id}) > 0;
    }

    tokenizations::TokenizationRecord save_tokenization(tokenizations::TokenizationRecord record)
    {
        return m_db.insert<tokenizations::TokenizationRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<tokenizations::TokenizationRecord>
    find_tokenization(std::int64_t id) const
    {
        return m_db.find<tokenizations::TokenizationRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<tokenizations::TokenizationRecord>
    find_tokenization_by_string(std::string_view token_id) const
    {
        return find_by_string<tokenizations::TokenizationRecord>("token_id", token_id);
    }

    [[nodiscard]] std::vector<tokenizations::TokenizationRecord> list_tokenizations() const
    {
        return m_db.all<tokenizations::TokenizationRecord>();
    }

    bool update_tokenization(const tokenizations::TokenizationRecord& record)
    {
        return m_db.update_where_column<tokenizations::TokenizationRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_tokenization(std::int64_t id)
    {
        return m_db.remove<tokenizations::TokenizationRecord>(core::db::DbValue{id}) > 0;
    }

    revenue_recovery_tokens::RevenueRecoveryTokenRecord
    save_revenue_recovery_token(revenue_recovery_tokens::RevenueRecoveryTokenRecord record)
    {
        return m_db.insert<revenue_recovery_tokens::RevenueRecoveryTokenRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<revenue_recovery_tokens::RevenueRecoveryTokenRecord>
    find_revenue_recovery_token(std::int64_t id) const
    {
        return m_db.find<revenue_recovery_tokens::RevenueRecoveryTokenRecord>(
            core::db::DbValue{id}
        );
    }

    [[nodiscard]] std::optional<revenue_recovery_tokens::RevenueRecoveryTokenRecord>
    find_revenue_recovery_token_by_string(std::string_view token) const
    {
        return find_by_string<revenue_recovery_tokens::RevenueRecoveryTokenRecord>("token", token);
    }

    [[nodiscard]] std::vector<revenue_recovery_tokens::RevenueRecoveryTokenRecord>
    list_revenue_recovery_tokens() const
    {
        return m_db.all<revenue_recovery_tokens::RevenueRecoveryTokenRecord>();
    }

    bool update_revenue_recovery_token(
        const revenue_recovery_tokens::RevenueRecoveryTokenRecord& record
    )
    {
        return m_db.update_where_column<revenue_recovery_tokens::RevenueRecoveryTokenRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_revenue_recovery_token(std::int64_t id)
    {
        return m_db.remove<revenue_recovery_tokens::RevenueRecoveryTokenRecord>(
                   core::db::DbValue{id}
               ) > 0;
    }

    configs::ConfigRecord save_config(configs::ConfigRecord record)
    {
        return m_db.insert<configs::ConfigRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<configs::ConfigRecord> find_config(std::int64_t id) const
    {
        return m_db.find<configs::ConfigRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<configs::ConfigRecord>
    find_config_by_key(std::string_view config_key) const
    {
        return find_by_string<configs::ConfigRecord>("config_key", config_key);
    }

    [[nodiscard]] std::vector<configs::ConfigRecord> list_configs() const
    {
        return m_db.all<configs::ConfigRecord>();
    }

    bool update_config(const configs::ConfigRecord& record)
    {
        return m_db.update_where_column<configs::ConfigRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_config(std::int64_t id)
    {
        return m_db.remove<configs::ConfigRecord>(core::db::DbValue{id}) > 0;
    }

    disputes::DisputeRecord save_dispute(disputes::DisputeRecord record)
    {
        return m_db.insert<disputes::DisputeRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<disputes::DisputeRecord> find_dispute(std::int64_t id) const
    {
        return m_db.find<disputes::DisputeRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<disputes::DisputeRecord>
    find_dispute_by_string(std::string_view dispute_id) const
    {
        return find_by_string<disputes::DisputeRecord>("dispute_id", dispute_id);
    }

    [[nodiscard]] std::vector<disputes::DisputeRecord> list_disputes() const
    {
        return m_db.all<disputes::DisputeRecord>();
    }

    bool update_dispute(const disputes::DisputeRecord& record)
    {
        return m_db.update_where_column<disputes::DisputeRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_dispute(std::int64_t id)
    {
        return m_db.remove<disputes::DisputeRecord>(core::db::DbValue{id}) > 0;
    }

    payment_methods::PaymentMethodRecord
    save_payment_method(payment_methods::PaymentMethodRecord record)
    {
        return m_db.insert<payment_methods::PaymentMethodRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<payment_methods::PaymentMethodRecord>
    find_payment_method(std::int64_t id) const
    {
        return m_db.find<payment_methods::PaymentMethodRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<payment_methods::PaymentMethodRecord>
    find_payment_method_by_string(std::string_view payment_method_id) const
    {
        return find_by_string<payment_methods::PaymentMethodRecord>(
            "payment_method_id",
            payment_method_id
        );
    }

    [[nodiscard]] std::vector<payment_methods::PaymentMethodRecord> list_payment_methods() const
    {
        return m_db.all<payment_methods::PaymentMethodRecord>();
    }

    bool update_payment_method(const payment_methods::PaymentMethodRecord& record)
    {
        return m_db.update_where_column<payment_methods::PaymentMethodRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_payment_method(std::int64_t id)
    {
        return m_db.remove<payment_methods::PaymentMethodRecord>(core::db::DbValue{id}) > 0;
    }

    payment_method_sessions::PaymentMethodSessionRecord
    save_payment_method_session(payment_method_sessions::PaymentMethodSessionRecord record)
    {
        return m_db.insert<payment_method_sessions::PaymentMethodSessionRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<payment_method_sessions::PaymentMethodSessionRecord>
    find_payment_method_session(std::int64_t id) const
    {
        return m_db.find<payment_method_sessions::PaymentMethodSessionRecord>(
            core::db::DbValue{id}
        );
    }

    [[nodiscard]] std::optional<payment_method_sessions::PaymentMethodSessionRecord>
    find_payment_method_session_by_string(std::string_view session_id) const
    {
        return find_by_string<payment_method_sessions::PaymentMethodSessionRecord>(
            "session_id",
            session_id
        );
    }

    [[nodiscard]] std::vector<payment_method_sessions::PaymentMethodSessionRecord>
    list_payment_method_sessions() const
    {
        return m_db.all<payment_method_sessions::PaymentMethodSessionRecord>();
    }

    bool update_payment_method_session(
        const payment_method_sessions::PaymentMethodSessionRecord& record
    )
    {
        return m_db.update_where_column<payment_method_sessions::PaymentMethodSessionRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_payment_method_session(std::int64_t id)
    {
        return m_db.remove<payment_method_sessions::PaymentMethodSessionRecord>(
                   core::db::DbValue{id}
               ) > 0;
    }

    profiles::ProfileRecord save_profile(profiles::ProfileRecord record)
    {
        return m_db.insert<profiles::ProfileRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<profiles::ProfileRecord> find_profile(std::int64_t id) const
    {
        return m_db.find<profiles::ProfileRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<profiles::ProfileRecord>
    find_profile_by_string(std::string_view profile_id) const
    {
        return find_by_string<profiles::ProfileRecord>("profile_id", profile_id);
    }

    [[nodiscard]] std::vector<profiles::ProfileRecord> list_profiles() const
    {
        return m_db.all<profiles::ProfileRecord>();
    }

    bool update_profile(const profiles::ProfileRecord& record)
    {
        return m_db.update_where_column<profiles::ProfileRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_profile(std::int64_t id)
    {
        return m_db.remove<profiles::ProfileRecord>(core::db::DbValue{id}) > 0;
    }

    card_issuers::CardIssuerRecord save_card_issuer(card_issuers::CardIssuerRecord record)
    {
        return m_db.insert<card_issuers::CardIssuerRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<card_issuers::CardIssuerRecord>
    find_card_issuer(std::int64_t id) const
    {
        return m_db.find<card_issuers::CardIssuerRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<card_issuers::CardIssuerRecord>
    find_card_issuer_by_string(std::string_view issuer_id) const
    {
        return find_by_string<card_issuers::CardIssuerRecord>("issuer_id", issuer_id);
    }

    [[nodiscard]] std::vector<card_issuers::CardIssuerRecord> list_card_issuers() const
    {
        return m_db.all<card_issuers::CardIssuerRecord>();
    }

    bool update_card_issuer(const card_issuers::CardIssuerRecord& record)
    {
        return m_db.update_where_column<card_issuers::CardIssuerRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_card_issuer(std::int64_t id)
    {
        return m_db.remove<card_issuers::CardIssuerRecord>(core::db::DbValue{id}) > 0;
    }

    hierarchical_resources::HierarchicalResourceRecord
    save_hierarchical_resource(hierarchical_resources::HierarchicalResourceRecord record)
    {
        return m_db.insert<hierarchical_resources::HierarchicalResourceRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<hierarchical_resources::HierarchicalResourceRecord>
    find_hierarchical_resource(std::int64_t id) const
    {
        return m_db.find<hierarchical_resources::HierarchicalResourceRecord>(core::db::DbValue{id});
    }

    [[nodiscard]] std::optional<hierarchical_resources::HierarchicalResourceRecord>
    find_hierarchical_resource_by_string(std::string_view resource_id) const
    {
        return find_by_string<hierarchical_resources::HierarchicalResourceRecord>(
            "resource_id",
            resource_id
        );
    }

    [[nodiscard]] std::vector<hierarchical_resources::HierarchicalResourceRecord>
    list_hierarchical_resources() const
    {
        return m_db.all<hierarchical_resources::HierarchicalResourceRecord>();
    }

    bool update_hierarchical_resource(
        const hierarchical_resources::HierarchicalResourceRecord& record
    )
    {
        return m_db.update_where_column<hierarchical_resources::HierarchicalResourceRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    bool remove_hierarchical_resource(std::int64_t id)
    {
        return m_db.remove<hierarchical_resources::HierarchicalResourceRecord>(
                   core::db::DbValue{id}
               ) > 0;
    }

    // ── Report-driven query methods ────────────────────────────────────────────

    [[nodiscard]] std::vector<payments::PaymentIntentRecord> find_payments_by_merchant_id(
        std::string_view merchant_id,
        std::size_t limit,
        std::size_t offset
    ) const
    {
        core::db::QuerySpec spec = m_db.query<payments::PaymentIntentRecord>().spec();
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "merchant_id",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{merchant_id}}
            }
        );
        if (limit > 0) {
            spec.limit_count = limit;
        }
        spec.offset_count = offset;
        return m_db.collect<payments::PaymentIntentRecord>(spec);
    }

    [[nodiscard]] std::optional<payments::PaymentIntentRecord>
    find_payment_by_merchant_reference_id(
        std::string_view merchant_id,
        std::string_view merchant_reference_id
    ) const
    {
        core::db::QuerySpec spec = m_db.query<payments::PaymentIntentRecord>().spec();
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "merchant_id",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{merchant_id}}
            }
        );
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "merchant_reference_id",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{merchant_reference_id}}
            }
        );
        spec.limit_count = 1;
        const auto records = m_db.collect<payments::PaymentIntentRecord>(spec);
        if (records.empty()) {
            return std::nullopt;
        }
        return records.front();
    }

    [[nodiscard]] std::vector<payments::PaymentIntentRecord>
    list_payments(const PaymentQuery& query) const
    {
        core::db::QuerySpec spec = m_db.query<payments::PaymentIntentRecord>().spec();
        if (!query.get_merchant_id().empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "merchant_id",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.get_merchant_id()}
                }
            );
        }
        if (!query.get_merchant_reference_id().empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "merchant_reference_id",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.get_merchant_reference_id()}
                }
            );
        }
        if (!query.get_status().empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "status",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.get_status()}
                }
            );
        }
        if (query.get_limit() > 0) {
            spec.limit_count = query.get_limit();
        }
        spec.offset_count = query.get_offset();
        return m_db.collect<payments::PaymentIntentRecord>(spec);
    }

    [[nodiscard]] std::vector<refunds::RefundRecord>
    find_refunds_by_payment_id(std::string_view payment_id) const
    {
        core::db::QuerySpec spec = m_db.query<refunds::RefundRecord>().spec();
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "payment_id",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{payment_id}}
            }
        );
        return m_db.collect<refunds::RefundRecord>(spec);
    }

    [[nodiscard]] std::vector<refunds::RefundRecord>
    find_refunds_by_constraints(const RefundQuery& query) const
    {
        core::db::QuerySpec spec = m_db.query<refunds::RefundRecord>().spec();
        if (!query.get_payment_id().empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "payment_id",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.get_payment_id()}
                }
            );
        }
        if (!query.get_merchant_id().empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "merchant_id",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.get_merchant_id()}
                }
            );
        }
        if (!query.get_status().empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "status",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.get_status()}
                }
            );
        }
        if (query.get_limit() > 0) {
            spec.limit_count = query.get_limit();
        }
        spec.offset_count = query.get_offset();
        return m_db.collect<refunds::RefundRecord>(spec);
    }

    bool update_refund_status(std::string_view refund_id, std::string status)
    {
        auto record = find_refund_by_string(refund_id);
        if (!record) {
            return false;
        }
        record->set_status(std::move(status));
        return update_refund(*record);
    }

    [[nodiscard]] std::vector<webhook_events::WebhookEventRecord>
    find_webhook_events_by_merchant_id(
        std::string_view merchant_id,
        std::size_t limit,
        std::size_t offset
    ) const
    {
        core::db::QuerySpec spec = m_db.query<webhook_events::WebhookEventRecord>().spec();
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "merchant_id",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{merchant_id}}
            }
        );
        if (limit > 0) {
            spec.limit_count = limit;
        }
        spec.offset_count = offset;
        return m_db.collect<webhook_events::WebhookEventRecord>(spec);
    }

    [[nodiscard]] std::vector<webhook_events::WebhookEventRecord>
    list_webhook_events(const WebhookEventQuery& query) const
    {
        core::db::QuerySpec spec = m_db.query<webhook_events::WebhookEventRecord>().spec();
        if (!query.get_merchant_id().empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "merchant_id",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.get_merchant_id()}
                }
            );
        }
        if (!query.get_event_id().empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "event_id",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.get_event_id()}
                }
            );
        }
        if (!query.get_event_type().empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "event_type",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.get_event_type()}
                }
            );
        }
        if (query.get_limit() > 0) {
            spec.limit_count = query.get_limit();
        }
        spec.offset_count = query.get_offset();
        return m_db.collect<webhook_events::WebhookEventRecord>(spec);
    }

    [[nodiscard]] std::optional<webhook_events::WebhookEventRecord>
    find_webhook_event_by_event_id(std::string_view event_id) const
    {
        return find_webhook_event_by_string(event_id);
    }

    std::expected<webhook_events::WebhookEventRecord, core::error::AppError>
    insert_webhook_event(webhook_events::WebhookEventRecord record)
    {
        if (any_match("webhook_events", "event_id", record.get_event_id())) {
            return std::unexpected{already_exists("webhook event", record.get_event_id())};
        }
        return m_db.insert<webhook_events::WebhookEventRecord>(std::move(record));
    }

    [[nodiscard]] std::optional<routing::RoutingConfigRecord>
    find_routing_config_by_algorithm_id_and_profile_id(
        std::string_view algorithm_id,
        std::string_view profile_id
    ) const
    {
        core::db::QuerySpec spec = m_db.query<routing::RoutingConfigRecord>().spec();
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "algorithm_id",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{algorithm_id}}
            }
        );
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "profile_id",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{profile_id}}
            }
        );
        spec.limit_count = 1;
        const auto records = m_db.collect<routing::RoutingConfigRecord>(spec);
        if (records.empty()) {
            return std::nullopt;
        }
        return records.front();
    }

    [[nodiscard]] std::vector<mandates::MandateRecord>
    find_mandates_by_customer_id(std::string_view customer_id) const
    {
        core::db::QuerySpec spec = m_db.query<mandates::MandateRecord>().spec();
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "customer_id",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{customer_id}}
            }
        );
        return m_db.collect<mandates::MandateRecord>(spec);
    }

    [[nodiscard]] std::vector<mandates::MandateRecord>
    find_mandates_by_customer_id(const MandateQuery& query) const
    {
        core::db::QuerySpec spec = m_db.query<mandates::MandateRecord>().spec();
        if (!query.get_customer_id().empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "customer_id",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.get_customer_id()}
                }
            );
        }
        if (!query.get_merchant_id().empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "merchant_id",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.get_merchant_id()}
                }
            );
        }
        if (!query.get_status().empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "status",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.get_status()}
                }
            );
        }
        if (query.get_limit() > 0) {
            spec.limit_count = query.get_limit();
        }
        spec.offset_count = query.get_offset();
        return m_db.collect<mandates::MandateRecord>(spec);
    }

    bool update_mandate_status(std::string_view mandate_id, std::string status)
    {
        auto record = find_mandate_by_string(mandate_id);
        if (!record) {
            return false;
        }
        record->set_status(std::move(status));
        return update_mandate(*record);
    }

    [[nodiscard]] std::vector<subscriptions::SubscriptionRecord>
    find_subscriptions_by_customer_id(std::string_view customer_id) const
    {
        core::db::QuerySpec spec = m_db.query<subscriptions::SubscriptionRecord>().spec();
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "customer_id",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{customer_id}}
            }
        );
        return m_db.collect<subscriptions::SubscriptionRecord>(spec);
    }

    [[nodiscard]] std::vector<subscriptions::SubscriptionRecord>
    find_subscriptions_by_customer_id(const SubscriptionQuery& query) const
    {
        core::db::QuerySpec spec = m_db.query<subscriptions::SubscriptionRecord>().spec();
        if (!query.get_customer_id().empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "customer_id",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.get_customer_id()}
                }
            );
        }
        if (!query.get_merchant_id().empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "merchant_id",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.get_merchant_id()}
                }
            );
        }
        if (!query.get_status().empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "status",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.get_status()}
                }
            );
        }
        if (query.get_limit() > 0) {
            spec.limit_count = query.get_limit();
        }
        spec.offset_count = query.get_offset();
        return m_db.collect<subscriptions::SubscriptionRecord>(spec);
    }

    bool update_subscription_status(std::string_view subscription_id, std::string status)
    {
        auto record = find_subscription_by_string(subscription_id);
        if (!record) {
            return false;
        }
        record->set_status(std::move(status));
        return update_subscription(*record);
    }

    [[nodiscard]] std::optional<api_keys::ApiKeyRecord> find_api_key_by_merchant_id_and_key_id(
        std::string_view merchant_id,
        std::string_view key_id
    ) const
    {
        core::db::QuerySpec spec = m_db.query<api_keys::ApiKeyRecord>().spec();
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "merchant_id",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{merchant_id}}
            }
        );
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "key_id",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{key_id}}
            }
        );
        spec.limit_count = 1;
        const auto records = m_db.collect<api_keys::ApiKeyRecord>(spec);
        if (records.empty()) {
            return std::nullopt;
        }
        return records.front();
    }

    [[nodiscard]] std::optional<std::string> get_config(std::string_view key) const
    {
        const auto record = find_config_by_key(key);
        if (!record) {
            return std::nullopt;
        }
        return record->get_config_value();
    }

    configs::ConfigRecord set_config(std::string key, std::string value)
    {
        auto record = find_config_by_key(key);
        if (record) {
            record->set_config_value(std::move(value));
            update_config(*record);
            return *record;
        }
        configs::ConfigRecord created{};
        created.set_config_key(std::move(key));
        created.set_config_value(std::move(value));
        return save_config(std::move(created));
    }

    [[nodiscard]] std::optional<gsm_rules::GsmRuleRecord>
    find_gsm_rule(std::string_view natural_key) const
    {
        return find_gsm_rule_by_string(natural_key);
    }

    std::expected<process_trackers::ProcessTrackerRecord, core::error::AppError>
    record_process_tracker(std::string name, std::string payload, std::string run_at)
    {
        process_trackers::ProcessTrackerRecord record{};
        record.set_task_id(std::format("{}_{}", name, payload.size()));
        record.set_name(std::move(name));
        record.set_payload(std::move(payload));
        record.set_run_at(std::move(run_at));
        record.set_status("scheduled");
        return create_process_tracker(std::move(record));
    }

    bool update_process_tracker(std::string_view task_id, std::string status, std::string result)
    {
        auto record = find_process_tracker_by_string(task_id);
        if (!record) {
            return false;
        }
        record->set_status(std::move(status));
        record->set_result(std::move(result));
        return update_process_tracker(*record);
    }

    [[nodiscard]] std::vector<process_trackers::ProcessTrackerRecord>
    list_process_trackers(const ProcessTrackerQuery& query) const
    {
        core::db::QuerySpec spec = m_db.query<process_trackers::ProcessTrackerRecord>().spec();
        if (!query.get_name().empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "name",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.get_name()}
                }
            );
        }
        if (!query.get_status().empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "status",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.get_status()}
                }
            );
        }
        if (query.get_limit() > 0) {
            spec.limit_count = query.get_limit();
        }
        spec.offset_count = query.get_offset();
        return m_db.collect<process_trackers::ProcessTrackerRecord>(spec);
    }

    // ── Unique-constraint-aware inserts ────────────────────────────────────────

    std::expected<api_keys::ApiKeyRecord, core::error::AppError>
    create_api_key(api_keys::ApiKeyRecord record)
    {
        if (any_match_pair(
                "api_keys",
                "merchant_id",
                record.get_merchant_id(),
                "key_id",
                record.get_key_id()
            )) {
            return std::unexpected{already_exists("API key", record.get_key_id())};
        }
        return m_db.insert<api_keys::ApiKeyRecord>(std::move(record));
    }

    std::expected<organizations::OrganizationRecord, core::error::AppError>
    create_organization(organizations::OrganizationRecord record)
    {
        if (any_match("organizations", "organization_id", record.get_organization_id())) {
            return std::unexpected{already_exists("organization", record.get_organization_id())};
        }
        return m_db.insert<organizations::OrganizationRecord>(std::move(record));
    }

    std::expected<merchant_accounts::MerchantAccountRecord, core::error::AppError>
    create_merchant_account(merchant_accounts::MerchantAccountRecord record)
    {
        if (any_match("merchant_accounts", "merchant_id", record.get_merchant_id())) {
            return std::unexpected{already_exists("merchant account", record.get_merchant_id())};
        }
        return m_db.insert<merchant_accounts::MerchantAccountRecord>(std::move(record));
    }

    std::expected<merchant_connectors::MerchantConnectorRecord, core::error::AppError>
    create_merchant_connector(merchant_connectors::MerchantConnectorRecord record)
    {
        if (any_match_pair(
                "merchant_connectors",
                "merchant_id",
                record.get_merchant_id(),
                "connector_name",
                record.get_connector_name()
            )) {
            return std::unexpected{
                already_exists("merchant connector", record.get_connector_name())
            };
        }
        return m_db.insert<merchant_connectors::MerchantConnectorRecord>(std::move(record));
    }

    std::expected<users::UserRecord, core::error::AppError> create_user(users::UserRecord record)
    {
        if (any_match("users", "user_id", record.get_user_id())) {
            return std::unexpected{already_exists("user", record.get_user_id())};
        }
        return m_db.insert<users::UserRecord>(std::move(record));
    }

    std::expected<payments::PaymentIntentRecord, core::error::AppError>
    create_payment(payments::PaymentIntentRecord record)
    {
        if (any_match("payment_intents", "payment_id", record.get_payment_id())) {
            return std::unexpected{already_exists("payment", record.get_payment_id())};
        }
        return m_db.insert<payments::PaymentIntentRecord>(std::move(record));
    }

    std::expected<refunds::RefundRecord, core::error::AppError>
    create_refund(refunds::RefundRecord record)
    {
        if (any_match("refunds", "refund_id", record.get_refund_id())) {
            return std::unexpected{already_exists("refund", record.get_refund_id())};
        }
        return m_db.insert<refunds::RefundRecord>(std::move(record));
    }

    std::expected<customers::CustomerRecord, core::error::AppError>
    create_customer(customers::CustomerRecord record)
    {
        if (any_match("customers", "customer_id", record.get_customer_id())) {
            return std::unexpected{already_exists("customer", record.get_customer_id())};
        }
        return m_db.insert<customers::CustomerRecord>(std::move(record));
    }

    std::expected<routing::RoutingConfigRecord, core::error::AppError>
    create_routing_config(routing::RoutingConfigRecord record)
    {
        if (any_match_pair(
                "routing_configs",
                "config_id",
                record.get_config_id(),
                "profile_id",
                record.get_profile_id()
            )) {
            return std::unexpected{already_exists("routing config", record.get_config_id())};
        }
        return m_db.insert<routing::RoutingConfigRecord>(std::move(record));
    }

    std::expected<mandates::MandateRecord, core::error::AppError>
    create_mandate(mandates::MandateRecord record)
    {
        if (any_match("mandates", "mandate_id", record.get_mandate_id())) {
            return std::unexpected{already_exists("mandate", record.get_mandate_id())};
        }
        return m_db.insert<mandates::MandateRecord>(std::move(record));
    }

    std::expected<subscriptions::SubscriptionRecord, core::error::AppError>
    create_subscription(subscriptions::SubscriptionRecord record)
    {
        if (any_match("subscriptions", "subscription_id", record.get_subscription_id())) {
            return std::unexpected{already_exists("subscription", record.get_subscription_id())};
        }
        return m_db.insert<subscriptions::SubscriptionRecord>(std::move(record));
    }

    std::expected<payouts::PayoutRecord, core::error::AppError>
    create_payout(payouts::PayoutRecord record)
    {
        if (any_match("payouts", "payout_id", record.get_payout_id())) {
            return std::unexpected{already_exists("payout", record.get_payout_id())};
        }
        return m_db.insert<payouts::PayoutRecord>(std::move(record));
    }

    std::expected<connector_webhooks::ConnectorWebhookRecord, core::error::AppError>
    create_connector_webhook(connector_webhooks::ConnectorWebhookRecord record)
    {
        if (any_match(
                "connector_webhooks",
                "connector_webhook_id",
                record.get_connector_webhook_id()
            )) {
            return std::unexpected{
                already_exists("connector webhook", record.get_connector_webhook_id())
            };
        }
        return m_db.insert<connector_webhooks::ConnectorWebhookRecord>(std::move(record));
    }

    std::expected<api_key_expiries::ApiKeyExpiryRecord, core::error::AppError>
    create_api_key_expiry(api_key_expiries::ApiKeyExpiryRecord record)
    {
        if (any_match("api_key_expiries", "key_id", record.get_key_id())) {
            return std::unexpected{already_exists("API key expiry", record.get_key_id())};
        }
        return m_db.insert<api_key_expiries::ApiKeyExpiryRecord>(std::move(record));
    }

    std::expected<payment_attempts::PaymentAttemptRecord, core::error::AppError>
    create_payment_attempt(payment_attempts::PaymentAttemptRecord record)
    {
        if (any_match("payment_attempts", "attempt_id", record.get_attempt_id())) {
            return std::unexpected{already_exists("payment attempt", record.get_attempt_id())};
        }
        return m_db.insert<payment_attempts::PaymentAttemptRecord>(std::move(record));
    }

    std::expected<payout_attempts::PayoutAttemptRecord, core::error::AppError>
    create_payout_attempt(payout_attempts::PayoutAttemptRecord record)
    {
        if (any_match("payout_attempts", "attempt_id", record.get_attempt_id())) {
            return std::unexpected{already_exists("payout attempt", record.get_attempt_id())};
        }
        return m_db.insert<payout_attempts::PayoutAttemptRecord>(std::move(record));
    }

    std::expected<payment_links::PaymentLinkRecord, core::error::AppError>
    create_payment_link(payment_links::PaymentLinkRecord record)
    {
        if (any_match("payment_links", "link_id", record.get_link_id())) {
            return std::unexpected{already_exists("payment link", record.get_link_id())};
        }
        return m_db.insert<payment_links::PaymentLinkRecord>(std::move(record));
    }

    std::expected<webhooks::WebhookRecord, core::error::AppError>
    create_webhook(webhooks::WebhookRecord record)
    {
        if (any_match("webhooks", "webhook_id", record.get_webhook_id())) {
            return std::unexpected{already_exists("webhook", record.get_webhook_id())};
        }
        return m_db.insert<webhooks::WebhookRecord>(std::move(record));
    }

    std::expected<webhook_delivery_attempts::WebhookDeliveryAttemptRecord, core::error::AppError>
    create_webhook_delivery_attempt(webhook_delivery_attempts::WebhookDeliveryAttemptRecord record)
    {
        if (any_match("webhook_delivery_attempts", "attempt_id", record.get_attempt_id())) {
            return std::unexpected{
                already_exists("webhook delivery attempt", record.get_attempt_id())
            };
        }
        return m_db.insert<webhook_delivery_attempts::WebhookDeliveryAttemptRecord>(
            std::move(record)
        );
    }

    std::expected<roles::RoleRecord, core::error::AppError> create_role(roles::RoleRecord record)
    {
        if (any_match("roles", "role_id", record.get_role_id())) {
            return std::unexpected{already_exists("role", record.get_role_id())};
        }
        return m_db.insert<roles::RoleRecord>(std::move(record));
    }

    std::expected<user_roles::UserRoleRecord, core::error::AppError>
    create_user_role(user_roles::UserRoleRecord record)
    {
        if (any_match_pair(
                "user_roles",
                "user_id",
                record.get_user_id(),
                "role_id",
                record.get_role_id()
            )) {
            return std::unexpected{already_exists("user role", record.get_user_role_id())};
        }
        return m_db.insert<user_roles::UserRoleRecord>(std::move(record));
    }

    std::expected<authentication_records::AuthenticationRecord, core::error::AppError>
    create_authentication_record(authentication_records::AuthenticationRecord record)
    {
        if (any_match("authentication_records", "auth_id", record.get_auth_id())) {
            return std::unexpected{already_exists("authentication record", record.get_auth_id())};
        }
        return m_db.insert<authentication_records::AuthenticationRecord>(std::move(record));
    }

    std::expected<gsm_rules::GsmRuleRecord, core::error::AppError>
    create_gsm_rule(gsm_rules::GsmRuleRecord record)
    {
        if (any_match("gsm_rules", "natural_key", record.get_natural_key())) {
            return std::unexpected{already_exists("GSM rule", record.get_natural_key())};
        }
        return m_db.insert<gsm_rules::GsmRuleRecord>(std::move(record));
    }

    std::expected<blocklist_entries::BlocklistEntryRecord, core::error::AppError>
    create_blocklist_entry(blocklist_entries::BlocklistEntryRecord record)
    {
        if (any_match_pair(
                "blocklist_entries",
                "fingerprint",
                record.get_fingerprint(),
                "transaction_type",
                record.get_transaction_type()
            )) {
            return std::unexpected{already_exists("blocklist entry", record.get_fingerprint())};
        }
        return m_db.insert<blocklist_entries::BlocklistEntryRecord>(std::move(record));
    }

    std::expected<fraud_checks::FraudCheckRecord, core::error::AppError>
    create_fraud_check(fraud_checks::FraudCheckRecord record)
    {
        if (any_match("fraud_checks", "fraud_check_id", record.get_fraud_check_id())) {
            return std::unexpected{already_exists("fraud check", record.get_fraud_check_id())};
        }
        return m_db.insert<fraud_checks::FraudCheckRecord>(std::move(record));
    }

    std::expected<decision_configs::DecisionConfigRecord, core::error::AppError>
    create_decision_config(decision_configs::DecisionConfigRecord record)
    {
        if (any_match("decision_configs", "config_id", record.get_config_id())) {
            return std::unexpected{already_exists("decision config", record.get_config_id())};
        }
        return m_db.insert<decision_configs::DecisionConfigRecord>(std::move(record));
    }

    std::expected<process_trackers::ProcessTrackerRecord, core::error::AppError>
    create_process_tracker(process_trackers::ProcessTrackerRecord record)
    {
        if (any_match("process_trackers", "task_id", record.get_task_id())) {
            return std::unexpected{already_exists("process tracker", record.get_task_id())};
        }
        return m_db.insert<process_trackers::ProcessTrackerRecord>(std::move(record));
    }

    std::expected<files::FileRecord, core::error::AppError> create_file(files::FileRecord record)
    {
        if (any_match("files", "file_id", record.get_file_id())) {
            return std::unexpected{already_exists("file", record.get_file_id())};
        }
        return m_db.insert<files::FileRecord>(std::move(record));
    }

    std::expected<cards_info::CardInfoRecord, core::error::AppError>
    create_card_info(cards_info::CardInfoRecord record)
    {
        if (any_match("cards_info", "iin", record.get_iin())) {
            return std::unexpected{already_exists("card info", record.get_iin())};
        }
        return m_db.insert<cards_info::CardInfoRecord>(std::move(record));
    }

    std::expected<relay_records::RelayRecord, core::error::AppError>
    create_relay_record(relay_records::RelayRecord record)
    {
        if (any_match("relay_records", "relay_id", record.get_relay_id())) {
            return std::unexpected{already_exists("relay record", record.get_relay_id())};
        }
        return m_db.insert<relay_records::RelayRecord>(std::move(record));
    }

    std::expected<tokenizations::TokenizationRecord, core::error::AppError>
    create_tokenization(tokenizations::TokenizationRecord record)
    {
        if (any_match("tokenizations", "token_id", record.get_token_id())) {
            return std::unexpected{already_exists("tokenization", record.get_token_id())};
        }
        return m_db.insert<tokenizations::TokenizationRecord>(std::move(record));
    }

    std::expected<revenue_recovery_tokens::RevenueRecoveryTokenRecord, core::error::AppError>
    create_revenue_recovery_token(revenue_recovery_tokens::RevenueRecoveryTokenRecord record)
    {
        if (any_match("revenue_recovery_tokens", "token", record.get_token())) {
            return std::unexpected{already_exists("revenue recovery token", record.get_token())};
        }
        return m_db.insert<revenue_recovery_tokens::RevenueRecoveryTokenRecord>(std::move(record));
    }

    std::expected<configs::ConfigRecord, core::error::AppError>
    create_config(configs::ConfigRecord record)
    {
        if (any_match("configs", "config_key", record.get_config_key())) {
            return std::unexpected{already_exists("config", record.get_config_key())};
        }
        return m_db.insert<configs::ConfigRecord>(std::move(record));
    }

    std::expected<disputes::DisputeRecord, core::error::AppError>
    create_dispute(disputes::DisputeRecord record)
    {
        if (any_match("disputes", "dispute_id", record.get_dispute_id())) {
            return std::unexpected{already_exists("dispute", record.get_dispute_id())};
        }
        return m_db.insert<disputes::DisputeRecord>(std::move(record));
    }

    std::expected<payment_methods::PaymentMethodRecord, core::error::AppError>
    create_payment_method(payment_methods::PaymentMethodRecord record)
    {
        if (any_match("payment_methods", "payment_method_id", record.get_payment_method_id())) {
            return std::unexpected{
                already_exists("payment method", record.get_payment_method_id())
            };
        }
        return m_db.insert<payment_methods::PaymentMethodRecord>(std::move(record));
    }

    std::expected<payment_method_sessions::PaymentMethodSessionRecord, core::error::AppError>
    create_payment_method_session(payment_method_sessions::PaymentMethodSessionRecord record)
    {
        if (any_match("payment_method_sessions", "session_id", record.get_session_id())) {
            return std::unexpected{
                already_exists("payment method session", record.get_session_id())
            };
        }
        return m_db.insert<payment_method_sessions::PaymentMethodSessionRecord>(std::move(record));
    }

    std::expected<profiles::ProfileRecord, core::error::AppError>
    create_profile(profiles::ProfileRecord record)
    {
        if (any_match("profiles", "profile_id", record.get_profile_id())) {
            return std::unexpected{already_exists("profile", record.get_profile_id())};
        }
        return m_db.insert<profiles::ProfileRecord>(std::move(record));
    }

    std::expected<card_issuers::CardIssuerRecord, core::error::AppError>
    create_card_issuer(card_issuers::CardIssuerRecord record)
    {
        if (any_match("card_issuers", "name", record.get_name())) {
            return std::unexpected{already_exists("card issuer", record.get_name())};
        }
        return m_db.insert<card_issuers::CardIssuerRecord>(std::move(record));
    }

    std::expected<hierarchical_resources::HierarchicalResourceRecord, core::error::AppError>
    create_hierarchical_resource(hierarchical_resources::HierarchicalResourceRecord record)
    {
        if (any_match("hierarchical_resources", "resource_id", record.get_resource_id())) {
            return std::unexpected{
                already_exists("hierarchical resource", record.get_resource_id())
            };
        }
        return m_db.insert<hierarchical_resources::HierarchicalResourceRecord>(std::move(record));
    }

private:
    template<typename Record>
    [[nodiscard]] std::optional<Record>
    find_by_string(std::string_view column, std::string_view value) const
    {
        core::db::QuerySpec spec = m_db.query<Record>().spec();
        spec.predicates.push_back(
            core::db::Predicate{
                .column = std::string{column},
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{value}},
            }
        );
        spec.limit_count = 1;
        std::vector<Record> records = m_db.collect<Record>(spec);
        if (records.empty()) {
            return std::nullopt;
        }
        return records.front();
    }

    [[nodiscard]] bool
    any_match(std::string_view table, std::string_view column, std::string_view value) const
    {
        core::db::QuerySpec spec;
        spec.table = std::string{table};
        spec.predicates.push_back(
            core::db::Predicate{
                .column = std::string{column},
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{value}},
            }
        );
        spec.limit_count = 1;
        return !m_db.execute(spec).rows.empty();
    }

    [[nodiscard]] bool any_match_pair(
        std::string_view table,
        std::string_view first_column,
        std::string_view first_value,
        std::string_view second_column,
        std::string_view second_value
    ) const
    {
        core::db::QuerySpec spec;
        spec.table = std::string{table};
        spec.predicates.push_back(
            core::db::Predicate{
                .column = std::string{first_column},
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{first_value}},
            }
        );
        spec.predicates.push_back(
            core::db::Predicate{
                .column = std::string{second_column},
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{second_value}},
            }
        );
        spec.limit_count = 1;
        return !m_db.execute(spec).rows.empty();
    }

    [[nodiscard]] core::error::AppError
    already_exists(std::string_view entity, std::string_view key) const
    {
        return core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::ALREADY_EXISTS,
            std::format("{} already exists: {}", entity, key)
        );
    }

    core::db::Database& m_db;
};

} // namespace app::store

#ifdef CONGELADO_TEST
namespace app::store::tests {

using namespace boost::ut;

suite<"app_store_repository"> app_repository_suite = []
{
    "constructs with an injected database handle"_test = []
    {
        core::db::Database db{};
        AppRepository repository{db};
        (void)repository;
        expect(true);
    };

    "all persisted record types expose their table names"_test = []
    {
        expect(
            serde::Serializable<payments::PaymentIntentRecord>::table_name() == "payment_intents"
        );
        expect(serde::Serializable<refunds::RefundRecord>::table_name() == "refunds");
        expect(serde::Serializable<customers::CustomerRecord>::table_name() == "customers");
        expect(serde::Serializable<api_keys::ApiKeyRecord>::table_name() == "api_keys");
        expect(serde::Serializable<users::UserRecord>::table_name() == "users");
        expect(
            serde::Serializable<routing::RoutingConfigRecord>::table_name() == "routing_configs"
        );
        expect(
            serde::Serializable<webhook_events::WebhookEventRecord>::table_name() ==
            "webhook_events"
        );
        expect(serde::Serializable<payouts::PayoutRecord>::table_name() == "payouts");
    };

    "new persisted record types expose their table names"_test = []
    {
        expect(
            serde::Serializable<organizations::OrganizationRecord>::table_name() == "organizations"
        );
        expect(
            serde::Serializable<merchant_accounts::MerchantAccountRecord>::table_name() ==
            "merchant_accounts"
        );
        expect(
            serde::Serializable<merchant_connectors::MerchantConnectorRecord>::table_name() ==
            "merchant_connectors"
        );
        expect(
            serde::Serializable<api_key_expiries::ApiKeyExpiryRecord>::table_name() ==
            "api_key_expiries"
        );
        expect(serde::Serializable<mandates::MandateRecord>::table_name() == "mandates");
        expect(
            serde::Serializable<subscriptions::SubscriptionRecord>::table_name() == "subscriptions"
        );
        expect(
            serde::Serializable<connector_webhooks::ConnectorWebhookRecord>::table_name() ==
            "connector_webhooks"
        );
    };

    "new entity table names cover the persistence gap"_test = []
    {
        expect(
            serde::Serializable<payment_attempts::PaymentAttemptRecord>::table_name() ==
            "payment_attempts"
        );
        expect(
            serde::Serializable<payout_attempts::PayoutAttemptRecord>::table_name() ==
            "payout_attempts"
        );
        expect(
            serde::Serializable<payment_links::PaymentLinkRecord>::table_name() == "payment_links"
        );
        expect(serde::Serializable<webhooks::WebhookRecord>::table_name() == "webhooks");
        expect(
            serde::Serializable<
                webhook_delivery_attempts::WebhookDeliveryAttemptRecord>::table_name() ==
            "webhook_delivery_attempts"
        );
        expect(serde::Serializable<roles::RoleRecord>::table_name() == "roles");
        expect(serde::Serializable<user_roles::UserRoleRecord>::table_name() == "user_roles");
        expect(
            serde::Serializable<authentication_records::AuthenticationRecord>::table_name() ==
            "authentication_records"
        );
        expect(serde::Serializable<gsm_rules::GsmRuleRecord>::table_name() == "gsm_rules");
        expect(
            serde::Serializable<blocklist_entries::BlocklistEntryRecord>::table_name() ==
            "blocklist_entries"
        );
        expect(serde::Serializable<fraud_checks::FraudCheckRecord>::table_name() == "fraud_checks");
        expect(
            serde::Serializable<decision_configs::DecisionConfigRecord>::table_name() ==
            "decision_configs"
        );
        expect(
            serde::Serializable<process_trackers::ProcessTrackerRecord>::table_name() ==
            "process_trackers"
        );
        expect(serde::Serializable<files::FileRecord>::table_name() == "files");
        expect(serde::Serializable<cards_info::CardInfoRecord>::table_name() == "cards_info");
        expect(serde::Serializable<relay_records::RelayRecord>::table_name() == "relay_records");
        expect(
            serde::Serializable<tokenizations::TokenizationRecord>::table_name() == "tokenizations"
        );
        expect(
            serde::Serializable<
                revenue_recovery_tokens::RevenueRecoveryTokenRecord>::table_name() ==
            "revenue_recovery_tokens"
        );
        expect(serde::Serializable<configs::ConfigRecord>::table_name() == "configs");
        expect(serde::Serializable<disputes::DisputeRecord>::table_name() == "disputes");
        expect(
            serde::Serializable<payment_methods::PaymentMethodRecord>::table_name() ==
            "payment_methods"
        );
        expect(
            serde::Serializable<
                payment_method_sessions::PaymentMethodSessionRecord>::table_name() ==
            "payment_method_sessions"
        );
        expect(serde::Serializable<profiles::ProfileRecord>::table_name() == "profiles");
        expect(serde::Serializable<card_issuers::CardIssuerRecord>::table_name() == "card_issuers");
        expect(
            serde::Serializable<hierarchical_resources::HierarchicalResourceRecord>::table_name() ==
            "hierarchical_resources"
        );
    };

    "query value objects compose as data"_test = []
    {
        const PaymentQuery query = PaymentQuery{}.merchant_id("m_1").limit(20).offset(5);
        expect(query.get_merchant_id() == "m_1");
        expect(query.get_limit() == 20);
        expect(query.get_offset() == 5);
    };
};

} // namespace app::store::tests
#endif
