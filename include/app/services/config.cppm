module;
export module app_services:config;

import std;
import core_serde;
import app_store;
import app_metrics;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::services {

/** @brief One key/value config record (namespace + key), mirroring upstream config rows. */
class ConfigRecord
{
public:
    ConfigRecord() = default;

    void set_id(std::int64_t value) noexcept
    {
        m_id = value;
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_config_namespace(std::string value)
    {
        m_config_namespace = std::move(value);
    }

    [[nodiscard]] const std::string& get_config_namespace() const noexcept
    {
        return m_config_namespace;
    }

    void set_config_key(std::string value)
    {
        m_config_key = std::move(value);
    }

    [[nodiscard]] const std::string& get_config_key() const noexcept
    {
        return m_config_key;
    }

    void set_value(std::string value)
    {
        m_value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return m_value;
    }

    void set_context(std::string value)
    {
        m_context = std::move(value);
    }

    [[nodiscard]] const std::string& get_context() const noexcept
    {
        return m_context;
    }

    void set_sequence(std::int64_t value) noexcept
    {
        m_sequence = value;
    }

    [[nodiscard]] const std::int64_t& get_sequence() const noexcept
    {
        return m_sequence;
    }

private:
    std::int64_t m_id{};
    std::string m_config_namespace{};
    std::string m_config_key{};
    std::string m_value{};
    std::string m_context{};
    std::int64_t m_sequence{};
};

/** @brief One superposition context (name, resolved value, selection weight). */
class ContextRecord
{
public:
    ContextRecord() = default;

    void set_name(std::string value)
    {
        m_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    void set_value(std::string value)
    {
        m_value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return m_value;
    }

    void set_weight(std::int64_t value) noexcept
    {
        m_weight = value;
    }

    [[nodiscard]] const std::int64_t& get_weight() const noexcept
    {
        return m_weight;
    }

private:
    std::string m_name{};
    std::string m_value{};
    std::int64_t m_weight{};
};

/** @brief Append-only audit entry for a config read or write. */
class AuditLogRecord
{
public:
    AuditLogRecord() = default;

    void set_id(std::int64_t value) noexcept
    {
        m_id = value;
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_config_namespace(std::string value)
    {
        m_config_namespace = std::move(value);
    }

    [[nodiscard]] const std::string& get_config_namespace() const noexcept
    {
        return m_config_namespace;
    }

    void set_config_key(std::string value)
    {
        m_config_key = std::move(value);
    }

    [[nodiscard]] const std::string& get_config_key() const noexcept
    {
        return m_config_key;
    }

    void set_action(std::string value)
    {
        m_action = std::move(value);
    }

    [[nodiscard]] const std::string& get_action() const noexcept
    {
        return m_action;
    }

    void set_detail(std::string value)
    {
        m_detail = std::move(value);
    }

    [[nodiscard]] const std::string& get_detail() const noexcept
    {
        return m_detail;
    }

    void set_sequence(std::int64_t value) noexcept
    {
        m_sequence = value;
    }

    [[nodiscard]] const std::int64_t& get_sequence() const noexcept
    {
        return m_sequence;
    }

private:
    std::int64_t m_id{};
    std::string m_config_namespace{};
    std::string m_config_key{};
    std::string m_action{};
    std::string m_detail{};
    std::int64_t m_sequence{};
};

/**
 * @brief Deterministic config store. Holds an AppRepository for persisted config records and
 * an in-memory index keyed by "namespace/key" (the local substitute for the upstream config
 * table while AppRepository config columns are being added by the persistence lane).
 */
class ConfigStore
{
public:
    ConfigStore() = default;

    explicit ConfigStore(app::store::AppRepository repository) :
        m_repository{std::make_shared<app::store::AppRepository>(std::move(repository))}
    {
    }

    void set_metrics(app::metrics::MetricsRegistry& metrics) noexcept
    {
        m_metrics = &metrics;
    }

    [[nodiscard]] bool has_repository() const noexcept
    {
        return m_repository != nullptr;
    }

    void put(
        std::string_view config_namespace,
        std::string_view key,
        std::string value,
        std::string context = {}
    )
    {
        const std::string composed = compose_key(config_namespace, key);
        ++m_sequence;
        ConfigRecord record{};
        record.set_id(static_cast<std::int64_t>(m_records.size()) + 1);
        record.set_config_namespace(std::string{config_namespace});
        record.set_config_key(std::string{key});
        record.set_value(std::move(value));
        record.set_context(std::move(context));
        record.set_sequence(static_cast<std::int64_t>(m_sequence));
        m_records[composed] = std::move(record);
        append_audit(config_namespace, key, "put", "config written");
        record_flow("config.put");
    }

    [[nodiscard]] std::optional<ConfigRecord>
    get(std::string_view config_namespace, std::string_view key) const
    {
        const auto it = m_records.find(compose_key(config_namespace, key));
        if (it == m_records.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    [[nodiscard]] std::optional<std::string>
    get_value(std::string_view config_namespace, std::string_view key) const
    {
        const auto record = get(config_namespace, key);
        if (!record) {
            return std::nullopt;
        }
        return record->get_value();
    }

    [[nodiscard]] std::vector<ConfigRecord> list(std::string_view config_namespace) const
    {
        std::vector<ConfigRecord> result{};
        const std::string prefix{std::string{config_namespace} + "/"};
        for (const auto& [key, record]: m_records) {
            if (key.starts_with(prefix)) {
                result.push_back(record);
            }
        }
        return result;
    }

    bool remove(std::string_view config_namespace, std::string_view key)
    {
        const std::string composed = compose_key(config_namespace, key);
        const bool existed = m_records.erase(composed) > 0;
        if (existed) {
            append_audit(config_namespace, key, "remove", "config removed");
            record_flow("config.remove");
        }
        return existed;
    }

    void add_context(ContextRecord context)
    {
        m_contexts.push_back(std::move(context));
        record_flow("config.context.add");
    }

    [[nodiscard]] std::vector<ContextRecord> list_contexts() const
    {
        return m_contexts;
    }

    [[nodiscard]] std::vector<AuditLogRecord> audit_logs() const
    {
        return m_audit_logs;
    }

    // Kill-switch / rollout consultation (config records under reserved namespaces).
    void set_kill_switch(std::string_view connector, bool killed)
    {
        put("connector_kill_switch", connector, killed ? "true" : "false");
    }

    [[nodiscard]] bool is_killed(std::string_view connector) const
    {
        const auto value = get_value("connector_kill_switch", connector);
        return value.has_value() && *value == "true";
    }

    void set_rollout(std::string_view connector, int percentage)
    {
        put("connector_rollout", connector, std::to_string(percentage));
    }

    [[nodiscard]] int rollout(std::string_view connector) const
    {
        const auto value = get_value("connector_rollout", connector);
        if (!value) {
            return 100;
        }
        try {
            return std::stoi(*value);
        } catch (...) {
            return 100;
        }
    }

    void set_shadow_kill(std::string_view connector, bool shadow_killed)
    {
        put("connector_shadow_kill", connector, shadow_killed ? "true" : "false");
    }

    [[nodiscard]] bool is_shadow_killed(std::string_view connector) const
    {
        const auto value = get_value("connector_shadow_kill", connector);
        return value.has_value() && *value == "true";
    }

private:
    [[nodiscard]] std::string
    compose_key(std::string_view config_namespace, std::string_view key) const
    {
        return std::string{config_namespace} + "/" + std::string{key};
    }

    void append_audit(
        std::string_view config_namespace,
        std::string_view key,
        std::string_view action,
        std::string_view detail
    )
    {
        ++m_audit_sequence;
        AuditLogRecord entry{};
        entry.set_id(static_cast<std::int64_t>(m_audit_logs.size()) + 1);
        entry.set_config_namespace(std::string{config_namespace});
        entry.set_config_key(std::string{key});
        entry.set_action(std::string{action});
        entry.set_detail(std::string{detail});
        entry.set_sequence(static_cast<std::int64_t>(m_audit_sequence));
        m_audit_logs.push_back(std::move(entry));
    }

    void record_flow(std::string_view flow) const
    {
        if (m_metrics != nullptr) {
            m_metrics->record_flow(flow);
            m_metrics->counter(flow).increment();
        }
    }

    std::shared_ptr<app::store::AppRepository> m_repository{};
    app::metrics::MetricsRegistry* m_metrics{nullptr};
    std::map<std::string, ConfigRecord> m_records{};
    std::vector<ContextRecord> m_contexts{};
    std::vector<AuditLogRecord> m_audit_logs{};
    std::uint64_t m_sequence{0};
    std::uint64_t m_audit_sequence{0};
};

} // namespace app::services

template<>
struct serde::Serializable<app::services::ConfigRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::services::ConfigRecord::get_id,
                &app::services::ConfigRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "config_namespace",
                &app::services::ConfigRecord::get_config_namespace,
                &app::services::ConfigRecord::set_config_namespace,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "config_key",
                &app::services::ConfigRecord::get_config_key,
                &app::services::ConfigRecord::set_config_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "value",
                &app::services::ConfigRecord::get_value,
                &app::services::ConfigRecord::set_value,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "context",
                &app::services::ConfigRecord::get_context,
                &app::services::ConfigRecord::set_context,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sequence",
                &app::services::ConfigRecord::get_sequence,
                &app::services::ConfigRecord::set_sequence,
                serde::FieldOptions{}>{},
        };
    }

    static constexpr std::string_view table_name()
    {
        return "configs";
    }
};

template<>
struct serde::Serializable<app::services::ContextRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &app::services::ContextRecord::get_name,
                &app::services::ContextRecord::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "value",
                &app::services::ContextRecord::get_value,
                &app::services::ContextRecord::set_value,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "weight",
                &app::services::ContextRecord::get_weight,
                &app::services::ContextRecord::set_weight,
                serde::FieldOptions{}>{},
        };
    }

    static constexpr std::string_view table_name()
    {
        return "superposition_contexts";
    }
};

template<>
struct serde::Serializable<app::services::AuditLogRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::services::AuditLogRecord::get_id,
                &app::services::AuditLogRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "config_namespace",
                &app::services::AuditLogRecord::get_config_namespace,
                &app::services::AuditLogRecord::set_config_namespace,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "config_key",
                &app::services::AuditLogRecord::get_config_key,
                &app::services::AuditLogRecord::set_config_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "action",
                &app::services::AuditLogRecord::get_action,
                &app::services::AuditLogRecord::set_action,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "detail",
                &app::services::AuditLogRecord::get_detail,
                &app::services::AuditLogRecord::set_detail,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sequence",
                &app::services::AuditLogRecord::get_sequence,
                &app::services::AuditLogRecord::set_sequence,
                serde::FieldOptions{}>{},
        };
    }

    static constexpr std::string_view table_name()
    {
        return "config_audit_logs";
    }
};

#ifdef CONGELADO_TEST
namespace app::services::tests {
using namespace boost::ut;

suite<"app_services_config"> config_suite = []
{
    "config store round trips a value by namespace and key"_test = []
    {
        ConfigStore store{};
        store.put("routing", "default", "stripe");
        const auto value = store.get_value("routing", "default");
        expect(value.has_value());
        expect(*value == "stripe");
        expect(!store.get_value("routing", "missing").has_value());
    };

    "config records carry a namespace/key composite and sequence"_test = []
    {
        ConfigStore store{};
        store.put("connector_kill_switch", "stripe", "true");
        const auto record = store.get("connector_kill_switch", "stripe");
        expect(record.has_value());
        expect(record->get_config_namespace() == "connector_kill_switch");
        expect(record->get_config_key() == "stripe");
        expect(record->get_value() == "true");
        expect(record->get_sequence() == 1);
    };

    "kill switch and rollout are config-backed data"_test = []
    {
        ConfigStore store{};
        expect(!store.is_killed("stripe"));
        store.set_kill_switch("stripe", true);
        expect(store.is_killed("stripe"));
        store.set_kill_switch("stripe", false);
        expect(!store.is_killed("stripe"));
        expect(store.rollout("checkout") == 100);
        store.set_rollout("checkout", 25);
        expect(store.rollout("checkout") == 25);
        expect(!store.is_shadow_killed("stripe"));
        store.set_shadow_kill("stripe", true);
        expect(store.is_shadow_killed("stripe"));
    };

    "audit log records every mutation in order"_test = []
    {
        ConfigStore store{};
        store.put("ns", "a", "1");
        store.remove("ns", "a");
        const auto logs = store.audit_logs();
        expect(logs.size() == 2);
        expect(logs[0].get_action() == "put");
        expect(logs[1].get_action() == "remove");
        expect(logs[1].get_sequence() == 2);
    };

    "config records expose their table names"_test = []
    {
        expect(serde::Serializable<ConfigRecord>::table_name() == "configs");
        expect(serde::Serializable<ContextRecord>::table_name() == "superposition_contexts");
        expect(serde::Serializable<AuditLogRecord>::table_name() == "config_audit_logs");
    };
};

} // namespace app::services::tests
#endif
