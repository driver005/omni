module;
export module app_db:schema;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::db {

/** @brief One persisted column described as data, not generated SQL. */
class ColumnDescriptor
{
public:
    ColumnDescriptor() = default;

    ColumnDescriptor(std::string name, core::db::ColumnType type) :
        m_name{std::move(name)},
        m_type{type}
    {
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] core::db::ColumnType get_type() const noexcept
    {
        return m_type;
    }

    [[nodiscard]] bool is_primary_key() const noexcept
    {
        return m_primary_key;
    }

    [[nodiscard]] bool is_unique() const noexcept
    {
        return m_unique;
    }

    [[nodiscard]] const std::string& get_references_table() const noexcept
    {
        return m_references_table;
    }

    [[nodiscard]] const std::string& get_references_column() const noexcept
    {
        return m_references_column;
    }

    std::string m_name{};
    core::db::ColumnType m_type{core::db::ColumnType::TEXT};
    bool m_primary_key{false};
    bool m_unique{false};
    std::string m_references_table{};
    std::string m_references_column{};
};

/** @brief Named unique index over one or more columns. */
class UniqueIndex
{
public:
    UniqueIndex() = default;

    UniqueIndex(std::string name, std::vector<std::string> columns) :
        m_name{std::move(name)},
        m_columns{std::move(columns)}
    {
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] const std::vector<std::string>& get_columns() const noexcept
    {
        return m_columns;
    }

    std::string m_name{};
    std::vector<std::string> m_columns{};
};

/** @brief One foreign-key relationship. */
class ForeignKey
{
public:
    ForeignKey() = default;

    ForeignKey(std::string column, std::string references_table, std::string references_column) :
        m_column{std::move(column)},
        m_references_table{std::move(references_table)},
        m_references_column{std::move(references_column)}
    {
    }

    [[nodiscard]] const std::string& get_column() const noexcept
    {
        return m_column;
    }

    [[nodiscard]] const std::string& get_references_table() const noexcept
    {
        return m_references_table;
    }

    [[nodiscard]] const std::string& get_references_column() const noexcept
    {
        return m_references_column;
    }

    std::string m_column{};
    std::string m_references_table{};
    std::string m_references_column{};
};

/** @brief Table-level schema data mirroring one persisted record. */
class TableDescriptor
{
public:
    TableDescriptor() = default;

    explicit TableDescriptor(std::string name) :
        m_name{std::move(name)}
    {
    }

    TableDescriptor& with_primary_key(std::string column)
    {
        m_primary_key = std::move(column);
        return *this;
    }

    TableDescriptor& with_column(ColumnDescriptor column)
    {
        if (column.m_name == m_primary_key) {
            column.m_primary_key = true;
        }
        m_columns.push_back(std::move(column));
        return *this;
    }

    TableDescriptor& with_unique(std::string name, std::vector<std::string> columns)
    {
        m_unique_indexes.emplace_back(std::move(name), std::move(columns));
        return *this;
    }

    TableDescriptor& with_foreign_key(
        std::string column,
        std::string references_table,
        std::string references_column
    )
    {
        for (ColumnDescriptor& descriptor: m_columns) {
            if (descriptor.m_name == column) {
                descriptor.m_references_table = references_table;
                descriptor.m_references_column = references_column;
                break;
            }
        }
        m_foreign_keys.emplace_back(
            std::move(column),
            std::move(references_table),
            std::move(references_column)
        );
        return *this;
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] const std::string& get_primary_key() const noexcept
    {
        return m_primary_key;
    }

    [[nodiscard]] const std::vector<ColumnDescriptor>& get_columns() const noexcept
    {
        return m_columns;
    }

    [[nodiscard]] const std::vector<UniqueIndex>& get_unique_indexes() const noexcept
    {
        return m_unique_indexes;
    }

    [[nodiscard]] const std::vector<ForeignKey>& get_foreign_keys() const noexcept
    {
        return m_foreign_keys;
    }

    std::string m_name{};
    std::string m_primary_key{};
    std::vector<ColumnDescriptor> m_columns{};
    std::vector<UniqueIndex> m_unique_indexes{};
    std::vector<ForeignKey> m_foreign_keys{};
};

/** @brief Instance functor rendering a TableDescriptor into idempotent DDL. */
class DdlRenderer
{
public:
    [[nodiscard]] std::string operator()(const TableDescriptor& table) const
    {
        std::string out = std::format("CREATE TABLE IF NOT EXISTS {} (", table.get_name());
        const auto& columns = table.get_columns();
        for (std::size_t i = 0; i < columns.size(); ++i) {
            if (i > 0) {
                out += ", ";
            }
            const ColumnDescriptor& column = columns[i];
            out += column.m_name;
            out += " ";
            out += core::db::SqlTypeName{}(column.m_type);
            if (column.m_primary_key) {
                out += " PRIMARY KEY";
            }
            if (!column.m_references_table.empty()) {
                out += std::format(
                    " REFERENCES {}({})",
                    column.m_references_table,
                    column.m_references_column
                );
            }
        }
        for (const UniqueIndex& index: table.get_unique_indexes()) {
            out += ", UNIQUE (";
            for (std::size_t i = 0; i < index.m_columns.size(); ++i) {
                if (i > 0) {
                    out += ", ";
                }
                out += index.m_columns[i];
            }
            out += ")";
        }
        out += ")";
        return out;
    }
};

/** @brief Immutable-by-convention registry with one descriptor per persisted table. */
class SchemaRegistry
{
public:
    SchemaRegistry()
    {
        build();
    }

    [[nodiscard]] const std::vector<TableDescriptor>& tables() const noexcept
    {
        return m_tables;
    }

    [[nodiscard]] std::optional<TableDescriptor> find(std::string_view name) const
    {
        for (const TableDescriptor& table: m_tables) {
            if (table.get_name() == name) {
                return table;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return m_tables.size();
    }

private:
    TableDescriptor& add(TableDescriptor descriptor)
    {
        m_tables.push_back(std::move(descriptor));
        return m_tables.back();
    }

    void build()
    {
        add(TableDescriptor{"payment_intents"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"payment_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_reference_id", core::db::ColumnType::TEXT})
            .with_column({"customer_id", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"amount", core::db::ColumnType::BIGINT})
            .with_column({"currency", core::db::ColumnType::TEXT})
            .with_column({"description", core::db::ColumnType::TEXT})
            .with_column({"metadata", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_column({"updated_at", core::db::ColumnType::TEXT})
            .with_unique("uq_payment_intents_payment_id", {"payment_id"});

        add(TableDescriptor{"refunds"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"refund_id", core::db::ColumnType::TEXT})
            .with_column({"payment_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"amount", core::db::ColumnType::BIGINT})
            .with_column({"currency", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"reason", core::db::ColumnType::TEXT})
            .with_column({"connector", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_column({"updated_at", core::db::ColumnType::TEXT})
            .with_unique("uq_refunds_refund_id", {"refund_id"})
            .with_foreign_key("payment_id", "payment_intents", "payment_id");

        add(TableDescriptor{"customers"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"customer_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"name", core::db::ColumnType::TEXT})
            .with_column({"email", core::db::ColumnType::TEXT})
            .with_column({"phone", core::db::ColumnType::TEXT})
            .with_column({"description", core::db::ColumnType::TEXT})
            .with_column({"metadata", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_customers_merchant_customer", {"merchant_id", "customer_id"});

        add(TableDescriptor{"api_keys"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"key_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"name", core::db::ColumnType::TEXT})
            .with_column({"description", core::db::ColumnType::TEXT})
            .with_column({"prefix", core::db::ColumnType::TEXT})
            .with_column({"api_key", core::db::ColumnType::TEXT})
            .with_column({"revoked", core::db::ColumnType::BOOLEAN})
            .with_column({"expiration", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_api_keys_merchant_key", {"merchant_id", "key_id"});

        add(TableDescriptor{"organizations"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"organization_id", core::db::ColumnType::TEXT})
            .with_column({"organization_name", core::db::ColumnType::TEXT})
            .with_column({"organization_type", core::db::ColumnType::TEXT})
            .with_column({"organization_details", core::db::ColumnType::TEXT})
            .with_column({"metadata", core::db::ColumnType::TEXT})
            .with_column({"platform_merchant_id", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_column({"modified_at", core::db::ColumnType::TEXT})
            .with_unique("uq_organizations_organization_id", {"organization_id"});

        add(TableDescriptor{"merchant_accounts"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_name", core::db::ColumnType::TEXT})
            .with_column({"organization_id", core::db::ColumnType::TEXT})
            .with_column({"parent_merchant_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_account_type", core::db::ColumnType::TEXT})
            .with_column({"return_url", core::db::ColumnType::TEXT})
            .with_column({"publishable_key", core::db::ColumnType::TEXT})
            .with_column({"locker_id", core::db::ColumnType::TEXT})
            .with_column({"metadata", core::db::ColumnType::TEXT})
            .with_column({"is_recon_enabled", core::db::ColumnType::BOOLEAN})
            .with_column({"sub_merchants_enabled", core::db::ColumnType::BOOLEAN})
            .with_column({"enable_payment_response_hash", core::db::ColumnType::BOOLEAN})
            .with_column({"redirect_to_merchant_with_http_post", core::db::ColumnType::BOOLEAN})
            .with_column({"routing_algorithm", core::db::ColumnType::TEXT})
            .with_column({"payout_routing_algorithm", core::db::ColumnType::TEXT})
            .with_column({"frm_routing_algorithm", core::db::ColumnType::TEXT})
            .with_column({"default_profile", core::db::ColumnType::TEXT})
            .with_column({"payment_response_hash_key", core::db::ColumnType::TEXT})
            .with_column({"recon_status", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_merchant_accounts_merchant_id", {"merchant_id"})
            .with_foreign_key("organization_id", "organizations", "organization_id");

        add(TableDescriptor{"merchant_connectors"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"merchant_connector_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"connector_name", core::db::ColumnType::TEXT})
            .with_column({"connector_type", core::db::ColumnType::TEXT})
            .with_column({"connector_label", core::db::ColumnType::TEXT})
            .with_column({"profile_id", core::db::ColumnType::TEXT})
            .with_column({"connector_account_details", core::db::ColumnType::TEXT})
            .with_column({"connector_webhook_details", core::db::ColumnType::TEXT})
            .with_column({"payment_methods_enabled", core::db::ColumnType::TEXT})
            .with_column({"frm_configs", core::db::ColumnType::TEXT})
            .with_column({"pm_auth_config", core::db::ColumnType::TEXT})
            .with_column({"disabled", core::db::ColumnType::BOOLEAN})
            .with_column({"test_mode", core::db::ColumnType::BOOLEAN})
            .with_column({"metadata", core::db::ColumnType::TEXT})
            .with_column({"applepay_verified_domains", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"business_country", core::db::ColumnType::TEXT})
            .with_column({"business_label", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique(
                "uq_merchant_connectors_merchant_connector",
                {"merchant_id", "connector_name"}
            )
            .with_foreign_key("merchant_id", "merchant_accounts", "merchant_id");

        add(TableDescriptor{"api_key_expiries"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"key_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"revoked", core::db::ColumnType::BOOLEAN})
            .with_column({"expiration", core::db::ColumnType::TEXT})
            .with_column({"expires_at", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_api_key_expiries_key_id", {"key_id"});

        add(TableDescriptor{"webhook_events"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"event_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"event_type", core::db::ColumnType::TEXT})
            .with_column({"event_class", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"payload", core::db::ColumnType::TEXT})
            .with_column({"delivery_attempts", core::db::ColumnType::BIGINT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_webhook_events_event_id", {"event_id"});

        add(TableDescriptor{"routing_configs"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"config_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"profile_id", core::db::ColumnType::TEXT})
            .with_column({"algorithm_id", core::db::ColumnType::TEXT})
            .with_column({"name", core::db::ColumnType::TEXT})
            .with_column({"description", core::db::ColumnType::TEXT})
            .with_column({"algorithm", core::db::ColumnType::TEXT})
            .with_column({"config_json", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_routing_configs_config_profile", {"config_id", "profile_id"})
            .with_unique("uq_routing_configs_algorithm_id", {"algorithm_id"});

        add(TableDescriptor{"mandates"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"mandate_id", core::db::ColumnType::TEXT})
            .with_column({"payment_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"customer_id", core::db::ColumnType::TEXT})
            .with_column({"profile_id", core::db::ColumnType::TEXT})
            .with_column({"amount", core::db::ColumnType::BIGINT})
            .with_column({"currency", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"payment_method_id", core::db::ColumnType::TEXT})
            .with_column({"payment_method", core::db::ColumnType::TEXT})
            .with_column({"payment_method_type", core::db::ColumnType::TEXT})
            .with_column({"connector", core::db::ColumnType::TEXT})
            .with_column({"mandate_metadata", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_mandates_mandate_id", {"mandate_id"});

        add(TableDescriptor{"subscriptions"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"subscription_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"profile_id", core::db::ColumnType::TEXT})
            .with_column({"customer_id", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"plan_id", core::db::ColumnType::TEXT})
            .with_column({"item_price_id", core::db::ColumnType::TEXT})
            .with_column({"coupon", core::db::ColumnType::TEXT})
            .with_column({"merchant_reference_id", core::db::ColumnType::TEXT})
            .with_column({"billing_processor_subscription_id", core::db::ColumnType::TEXT})
            .with_column({"client_secret", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_column({"updated_at", core::db::ColumnType::TEXT})
            .with_unique("uq_subscriptions_subscription_id", {"subscription_id"});

        add(TableDescriptor{"payouts"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"payout_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"amount", core::db::ColumnType::BIGINT})
            .with_column({"currency", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"connector", core::db::ColumnType::TEXT})
            .with_column({"payout_type", core::db::ColumnType::TEXT})
            .with_column({"customer_id", core::db::ColumnType::TEXT})
            .with_column({"metadata", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_payouts_payout_id", {"payout_id"});

        add(TableDescriptor{"connector_webhooks"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"connector_webhook_id", core::db::ColumnType::TEXT})
            .with_column({"connector", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"event_type", core::db::ColumnType::TEXT})
            .with_column({"scope", core::db::ColumnType::TEXT})
            .with_column({"secret", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"error_code", core::db::ColumnType::TEXT})
            .with_column({"error_message", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_connector_webhooks_id", {"connector_webhook_id"});

        add(TableDescriptor{"users"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"user_id", core::db::ColumnType::TEXT})
            .with_column({"email", core::db::ColumnType::TEXT})
            .with_column({"name", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"role", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_users_user_id", {"user_id"});

        add(TableDescriptor{"payment_attempts"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"attempt_id", core::db::ColumnType::TEXT})
            .with_column({"payment_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"connector", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"amount", core::db::ColumnType::BIGINT})
            .with_column({"currency", core::db::ColumnType::TEXT})
            .with_column({"error_code", core::db::ColumnType::TEXT})
            .with_column({"error_message", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_payment_attempts_attempt_id", {"attempt_id"})
            .with_foreign_key("payment_id", "payment_intents", "payment_id");

        add(TableDescriptor{"payout_attempts"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"attempt_id", core::db::ColumnType::TEXT})
            .with_column({"payout_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"connector", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"amount", core::db::ColumnType::BIGINT})
            .with_column({"currency", core::db::ColumnType::TEXT})
            .with_column({"error_code", core::db::ColumnType::TEXT})
            .with_column({"error_message", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_payout_attempts_attempt_id", {"attempt_id"})
            .with_foreign_key("payout_id", "payouts", "payout_id");

        add(TableDescriptor{"payment_links"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"link_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"profile_id", core::db::ColumnType::TEXT})
            .with_column({"customer_id", core::db::ColumnType::TEXT})
            .with_column({"amount", core::db::ColumnType::BIGINT})
            .with_column({"currency", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"url", core::db::ColumnType::TEXT})
            .with_column({"expires_at", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_payment_links_link_id", {"link_id"});

        add(TableDescriptor{"webhooks"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"webhook_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"profile_id", core::db::ColumnType::TEXT})
            .with_column({"url", core::db::ColumnType::TEXT})
            .with_column({"event_types", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"secret", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_webhooks_webhook_id", {"webhook_id"});

        add(TableDescriptor{"webhook_delivery_attempts"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"attempt_id", core::db::ColumnType::TEXT})
            .with_column({"event_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"endpoint", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"response_code", core::db::ColumnType::TEXT})
            .with_column({"response_body", core::db::ColumnType::TEXT})
            .with_column({"attempted_at", core::db::ColumnType::TEXT})
            .with_unique("uq_webhook_delivery_attempts_attempt_id", {"attempt_id"})
            .with_foreign_key("event_id", "webhook_events", "event_id");

        add(TableDescriptor{"roles"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"role_id", core::db::ColumnType::TEXT})
            .with_column({"role_name", core::db::ColumnType::TEXT})
            .with_column({"entity_type", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"organization_id", core::db::ColumnType::TEXT})
            .with_column({"profile_id", core::db::ColumnType::TEXT})
            .with_column({"is_internal", core::db::ColumnType::BIGINT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_roles_role_id", {"role_id"});

        add(TableDescriptor{"user_roles"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"user_role_id", core::db::ColumnType::TEXT})
            .with_column({"user_id", core::db::ColumnType::TEXT})
            .with_column({"role_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"organization_id", core::db::ColumnType::TEXT})
            .with_column({"profile_id", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_user_roles_user_role", {"user_id", "role_id"})
            .with_foreign_key("user_id", "users", "user_id")
            .with_foreign_key("role_id", "roles", "role_id");

        add(TableDescriptor{"authentication_records"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"auth_id", core::db::ColumnType::TEXT})
            .with_column({"user_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"method", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"challenge", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_column({"expires_at", core::db::ColumnType::TEXT})
            .with_unique("uq_authentication_records_auth_id", {"auth_id"});

        add(TableDescriptor{"gsm_rules"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"natural_key", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"rule_name", core::db::ColumnType::TEXT})
            .with_column({"decision", core::db::ColumnType::TEXT})
            .with_column({"program_json", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_gsm_rules_natural_key", {"natural_key"});

        add(TableDescriptor{"blocklist_entries"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"entry_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"profile_id", core::db::ColumnType::TEXT})
            .with_column({"data_kind", core::db::ColumnType::TEXT})
            .with_column({"transaction_type", core::db::ColumnType::TEXT})
            .with_column({"fingerprint", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_blocklist_entries_fingerprint", {"fingerprint", "transaction_type"});

        add(TableDescriptor{"fraud_checks"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"fraud_check_id", core::db::ColumnType::TEXT})
            .with_column({"payment_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"connector", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"score", core::db::ColumnType::BIGINT})
            .with_column({"decision", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_fraud_checks_id", {"fraud_check_id"});

        add(TableDescriptor{"decision_configs"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"config_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"profile_id", core::db::ColumnType::TEXT})
            .with_column({"name", core::db::ColumnType::TEXT})
            .with_column({"program_json", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_decision_configs_config_id", {"config_id"});

        add(TableDescriptor{"process_trackers"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"task_id", core::db::ColumnType::TEXT})
            .with_column({"name", core::db::ColumnType::TEXT})
            .with_column({"payload", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"result", core::db::ColumnType::TEXT})
            .with_column({"run_at", core::db::ColumnType::TEXT})
            .with_column({"attempts", core::db::ColumnType::BIGINT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_column({"updated_at", core::db::ColumnType::TEXT})
            .with_unique("uq_process_trackers_task_id", {"task_id"});

        add(TableDescriptor{"files"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"file_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"profile_id", core::db::ColumnType::TEXT})
            .with_column({"file_name", core::db::ColumnType::TEXT})
            .with_column({"file_type", core::db::ColumnType::TEXT})
            .with_column({"file_size", core::db::ColumnType::BIGINT})
            .with_column({"purpose", core::db::ColumnType::TEXT})
            .with_column({"provider_file_id", core::db::ColumnType::TEXT})
            .with_column({"upload_provider", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_files_file_id", {"file_id"});

        add(TableDescriptor{"cards_info"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"iin", core::db::ColumnType::TEXT})
            .with_column({"scheme", core::db::ColumnType::TEXT})
            .with_column({"card_type", core::db::ColumnType::TEXT})
            .with_column({"bank_name", core::db::ColumnType::TEXT})
            .with_column({"country", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_column({"updated_at", core::db::ColumnType::TEXT})
            .with_unique("uq_cards_info_iin", {"iin"});

        add(TableDescriptor{"relay_records"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"relay_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"profile_id", core::db::ColumnType::TEXT})
            .with_column({"payment_id", core::db::ColumnType::TEXT})
            .with_column({"refund_id", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"payload", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_relay_records_relay_id", {"relay_id"});

        add(TableDescriptor{"tokenizations"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"token_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"customer_id", core::db::ColumnType::TEXT})
            .with_column({"entity_id", core::db::ColumnType::TEXT})
            .with_column({"vault_id", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_tokenizations_token_id", {"token_id"});

        add(TableDescriptor{"revenue_recovery_tokens"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"token", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"payment_id", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"payload", core::db::ColumnType::TEXT})
            .with_column({"expires_at", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_revenue_recovery_tokens_token", {"token"});

        add(TableDescriptor{"configs"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"config_key", core::db::ColumnType::TEXT})
            .with_column({"config_value", core::db::ColumnType::TEXT})
            .with_column({"namespace", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_configs_config_key", {"config_key"});

        add(TableDescriptor{"disputes"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"dispute_id", core::db::ColumnType::TEXT})
            .with_column({"payment_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"profile_id", core::db::ColumnType::TEXT})
            .with_column({"connector", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"stage", core::db::ColumnType::TEXT})
            .with_column({"amount", core::db::ColumnType::BIGINT})
            .with_column({"currency", core::db::ColumnType::TEXT})
            .with_column({"reason", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_disputes_dispute_id", {"dispute_id"})
            .with_foreign_key("payment_id", "payment_intents", "payment_id");

        add(TableDescriptor{"payment_methods"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"payment_method_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"customer_id", core::db::ColumnType::TEXT})
            .with_column({"profile_id", core::db::ColumnType::TEXT})
            .with_column({"type", core::db::ColumnType::TEXT})
            .with_column({"network", core::db::ColumnType::TEXT})
            .with_column({"last4", core::db::ColumnType::TEXT})
            .with_column({"vault_id", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_payment_methods_id", {"payment_method_id"})
            .with_foreign_key("customer_id", "customers", "customer_id");

        add(TableDescriptor{"payment_method_sessions"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"session_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"client_secret", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"expires_at", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_payment_method_sessions_session_id", {"session_id"});

        add(TableDescriptor{"profiles"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"profile_id", core::db::ColumnType::TEXT})
            .with_column({"merchant_id", core::db::ColumnType::TEXT})
            .with_column({"profile_name", core::db::ColumnType::TEXT})
            .with_column({"type", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_profiles_profile_id", {"profile_id"})
            .with_foreign_key("merchant_id", "merchant_accounts", "merchant_id");

        add(TableDescriptor{"card_issuers"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"issuer_id", core::db::ColumnType::TEXT})
            .with_column({"name", core::db::ColumnType::TEXT})
            .with_column({"issuer_type", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"metadata", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_card_issuers_name", {"name"});

        add(TableDescriptor{"hierarchical_resources"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"resource_id", core::db::ColumnType::TEXT})
            .with_column({"scope_id", core::db::ColumnType::TEXT})
            .with_column({"resource_type", core::db::ColumnType::TEXT})
            .with_column({"data", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_unique("uq_hierarchical_resources_resource_id", {"resource_id"});

        add(TableDescriptor{"key_store"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"purpose", core::db::ColumnType::TEXT})
            .with_column({"key_id", core::db::ColumnType::TEXT})
            .with_column({"algorithm", core::db::ColumnType::TEXT})
            .with_column({"value", core::db::ColumnType::TEXT})
            .with_column({"created_at", core::db::ColumnType::BIGINT})
            .with_column({"rotated_at", core::db::ColumnType::BIGINT})
            .with_column({"active", core::db::ColumnType::BOOLEAN})
            .with_unique("uq_key_store_purpose_key_id", {"purpose", "key_id"});

        add(TableDescriptor{"one_time_codes"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"owner", core::db::ColumnType::TEXT})
            .with_column({"kind", core::db::ColumnType::TEXT})
            .with_column({"code_hash", core::db::ColumnType::TEXT})
            .with_column({"used", core::db::ColumnType::BOOLEAN})
            .with_column({"created_at", core::db::ColumnType::BIGINT})
            .with_column({"consumed_at", core::db::ColumnType::BIGINT})
            .with_unique("uq_one_time_codes_hash", {"code_hash"});

        add(TableDescriptor{"events"}.with_primary_key("id"))
            .with_column({"id", core::db::ColumnType::BIGINT})
            .with_column({"event_id", core::db::ColumnType::TEXT})
            .with_column({"topic", core::db::ColumnType::TEXT})
            .with_column({"payload", core::db::ColumnType::TEXT})
            .with_column({"endpoint", core::db::ColumnType::TEXT})
            .with_column({"status", core::db::ColumnType::TEXT})
            .with_column({"attempts", core::db::ColumnType::BIGINT})
            .with_column({"created_at", core::db::ColumnType::TEXT})
            .with_column({"updated_at", core::db::ColumnType::TEXT})
            .with_unique("uq_events_event_id", {"event_id"});
    }

    std::vector<TableDescriptor> m_tables{};
};

} // namespace app::db

#ifdef CONGELADO_TEST
namespace app::db::tests {

using namespace boost::ut;

suite<"app_db_schema_registry"> schema_registry_suite = []
{
    "registry holds one descriptor per persisted table"_test = []
    {
        const SchemaRegistry registry{};
        expect(registry.size() == 43);
    };

    "find resolves tables by name"_test = []
    {
        const SchemaRegistry registry{};
        const auto table = registry.find("payment_intents");
        expect(table.has_value());
        expect(table->get_primary_key() == "id");
        expect(!registry.find("missing_table").has_value());
    };

    "every descriptor carries a primary key and at least one unique index"_test = []
    {
        const SchemaRegistry registry{};
        for (const TableDescriptor& table: registry.tables()) {
            expect(!table.get_primary_key().empty());
            expect(!table.get_unique_indexes().empty());
            expect(!table.get_columns().empty());
        }
    };

    "renderer emits idempotent DDL with unique constraints"_test = []
    {
        const SchemaRegistry registry{};
        const std::string ddl = DdlRenderer{}(*registry.find("api_keys"));
        expect(ddl.contains("CREATE TABLE IF NOT EXISTS api_keys"));
        expect(ddl.contains("id BIGINT PRIMARY KEY"));
        expect(ddl.contains("UNIQUE (merchant_id, key_id)"));
    };

    "renderer includes foreign keys where upstream has them"_test = []
    {
        const SchemaRegistry registry{};
        const std::string ddl = DdlRenderer{}(*registry.find("user_roles"));
        expect(ddl.contains("REFERENCES users(user_id)"));
        expect(ddl.contains("REFERENCES roles(role_id)"));
    };
};

} // namespace app::db::tests
#endif
