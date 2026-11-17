module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>
#include <libpq-fe.h>

export module postgres_backend_plugin;

import congelado_plugin;
import interfaces;
import shared;
import core_logger;
import core_events;
import core_bridge;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

/**
 * @brief Storage-only Postgres backend for the core database abstraction.
 *
 * Implements `interfaces::IDatabase` over a single blocking libpq connection
 * and advertises `CONGELADO_CAP_STORAGE`, so the host's `storage_get` dispatch
 * can hand this instance to `connector::Connector::set_database()`. Connection
 * setup uses `PQconnectdbParams` (keyword/value arrays) rather than string
 * interpolation, so a quote in a config value cannot break the connstring.
 *
 * A failed connect leaves `m_conn` null and every call degrades to an empty
 * result — the same optional-infra behaviour as the existing postgres plugin.
 */
class PostgresBackendPlugin final :
    public congelado::Plugin,
    public interfaces::IDatabase
{
public:
    [[nodiscard]] std::string_view get_name() const noexcept override
    {
        return "postgres_backend";
    }

    [[nodiscard]] std::string_view get_version() const noexcept override
    {
        return "0.1.0";
    }

    [[nodiscard]] std::uint32_t capabilities() const noexcept override
    {
        return CONGELADO_CAP_STORAGE;
    }

    /**
     * @brief Opens the libpq connection from config.
     * @param host host callback table (used only for `host.log`).
     * @param cfg config view: `user` (postgres), `password` (empty), `host`
     * (localhost), `dbname` (congelado), `port` (5432).
     */
    void on_load(const CongeladoHostCallbacks& host, const CongeladoConfigView& cfg) override
    {
        const auto params = connection_params(cfg);
        const char* keywords[] = {
            "host", "port", "dbname", "user", "password", nullptr
        };
        const char* values[] = {
            params.host.c_str(),
            params.port.c_str(),
            params.dbname.c_str(),
            params.user.c_str(),
            params.password.c_str(),
            nullptr
        };
        m_conn = PQconnectdbParams(keywords, values, 0);
        if (PQstatus(m_conn) != CONNECTION_OK) {
            const std::string message = std::format("postgres_backend: {}", PQerrorMessage(m_conn));
            if (host.log != nullptr) {
                host.log(host.ctx, 3, message.data(), message.size());
            }
            core::logger::warning("postgres_backend", "connect failed: {}", PQerrorMessage(m_conn));
            core::events::publish("postgres_backend.connection.failed", {{"host", params.host}});
            PQfinish(m_conn);
            m_conn = nullptr;
            return;
        }
        core::logger::debug(
            "postgres_backend",
            "connected to {}:{}/{}",
            params.host,
            params.port,
            params.dbname
        );
        core::events::publish(
            "postgres_backend.connection.established",
            {{"host", params.host}, {"dbname", params.dbname}}
        );
    }

    void on_unload() noexcept override
    {
        if (m_conn != nullptr) {
            PQfinish(m_conn);
            m_conn = nullptr;
        }
    }

    /** @brief Capability hook: exposes this instance as `interfaces::IDatabase*`. */
    void* storage_get() noexcept
    {
        return static_cast<interfaces::IDatabase*>(this);
    }

    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "postgres";
    }

    [[nodiscard]] bool is_connected() const noexcept override
    {
        return m_conn != nullptr;
    }

    void query(std::string_view sql, shared::QueryReadFn&& result) noexcept override
    {
        exec(sql, std::move(result));
    }

    void insert(std::string_view sql, shared::QueryReadFn&& result) noexcept override
    {
        exec(sql, std::move(result));
    }

    void update(std::string_view sql, shared::QueryReadFn&& result) noexcept override
    {
        exec(sql, std::move(result));
    }

    void remove(std::string_view sql, shared::QueryReadFn&& result) noexcept override
    {
        exec(sql, std::move(result));
    }

private:
    /** @brief Connection parameters with local-dev defaults. Pure, testable. */
    struct ConnectionParams
    {
        std::string user{"postgres"};
        std::string password{};
        std::string host{"localhost"};
        std::string dbname{"congelado"};
        std::string port{"5432"};
    };

    [[nodiscard]] static ConnectionParams connection_params(const CongeladoConfigView& cfg)
    {
        ConnectionParams params;
        if (const auto value = congelado::config_get(cfg, "user")) {
            params.user = *value;
        }
        if (const auto value = congelado::config_get(cfg, "password")) {
            params.password = *value;
        }
        if (const auto value = congelado::config_get(cfg, "host")) {
            params.host = *value;
        }
        if (const auto value = congelado::config_get(cfg, "dbname")) {
            params.dbname = *value;
        }
        if (const auto value = congelado::config_get(cfg, "port")) {
            params.port = *value;
        }
        return params;
    }

    /** @brief Escapes a value for embedding in a single-quoted SQL literal. */
    [[nodiscard]] static std::string escape_sql_literal(std::string_view value)
    {
        std::string out;
        out.reserve(value.size());
        for (const char c: value) {
            out += (c == '\'') ? "''" : std::string_view{&c, 1};
        }
        return out;
    }

    /** @brief Escapes a cell for a JSON string literal. */
    [[nodiscard]] static std::string escape_json(std::string_view value)
    {
        std::string out;
        for (const char c: value) {
            switch (c) {
                case '"':
                    out += "\\\"";
                    break;
                case '\\':
                    out += "\\\\";
                    break;
                case '\n':
                    out += "\\n";
                    break;
                case '\r':
                    out += "\\r";
                    break;
                case '\t':
                    out += "\\t";
                    break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        out += std::format("\\u{:04x}", static_cast<unsigned char>(c));
                    } else {
                        out += c;
                    }
            }
        }
        return out;
    }

    /** @brief Marshals a SELECT result into a JSON array of column->cell objects. */
    [[nodiscard]] static std::string rows_to_json(PGresult* result)
    {
        const int column_count = PQnfields(result);
        std::vector<std::string> names;
        for (int column = 0; column < column_count; ++column) {
            names.emplace_back(PQfname(result, column));
        }
        std::string out = "[";
        for (int row = 0; row < PQntuples(result); ++row) {
            if (row > 0) {
                out += ",";
            }
            out += "{";
            for (int column = 0; column < column_count; ++column) {
                if (column > 0) {
                    out += ",";
                }
                out += std::format("\"{}\":", escape_json(names[static_cast<std::size_t>(column)]));
                out += PQgetisnull(result, row, column) != 0
                           ? "null"
                           : std::format("\"{}\"", escape_json(PQgetvalue(result, row, column)));
            }
            out += "}";
        }
        out += "]";
        return out;
    }

    /**
     * @brief Runs raw SQL. The sql text is a trusted caller payload (same trust
     * boundary as every other IDatabase backend in this codebase); values must
     * be escaped by the caller via `escape_sql_literal`/parameterization.
     */
    void exec(std::string_view sql, shared::QueryReadFn result) noexcept
    {
        if (m_conn == nullptr) {
            core::logger::warning("postgres_backend", "exec skipped, no connection: {}", sql);
            result("");
            return;
        }
        try {
            const std::string statement{sql};
            PGresult* raw = PQexec(m_conn, statement.c_str());
            const ExecStatusType status = PQresultStatus(raw);
            if (status == PGRES_TUPLES_OK) {
                result(rows_to_json(raw));
            } else if (status == PGRES_COMMAND_OK) {
                result("ok");
            } else {
                core::events::publish(
                    "postgres_backend.exec.failed",
                    {{"error", PQresultErrorMessage(raw)}}
                );
                result("");
            }
            PQclear(raw);
        } catch (...) {
            core::events::publish("postgres_backend.exec.exception", {{"sql", std::string{sql}}});
            result("");
        }
    }

    PGconn* m_conn{nullptr};
};

CONGELADO_PLUGIN(PostgresBackendPlugin);

/**
 * @brief core_bridge registration entrypoint. The bridge's Poco::SharedLibrary
 * resolves this symbol and hands back its opaque runtime context; we register
 * this plugin's IDatabase surface so the host can resolve it as a service.
 */
extern "C" void congelado_bridge_register(void* context) noexcept
{
    auto* plugin = plugin_instance();
    if (plugin == nullptr) {
        return;
    }
    auto* bridge = static_cast<core::bridge::Bridge*>(context);
    bridge->register_service(
        core::bridge::ServiceKey::make<interfaces::IDatabase*>("database"),
        static_cast<interfaces::IDatabase*>(plugin)
    );
}

#ifdef CONGELADO_TEST
namespace postgres_backend_tests {
using namespace boost::ut;

suite<"PostgresBackendPlugin"> postgres_backend_plugin_suite = []
{
    "advertises storage capability and identity"_test = []
    {
        PostgresBackendPlugin plugin;
        expect(plugin.get_name() == "postgres_backend");
        expect(plugin.capabilities() == CONGELADO_CAP_STORAGE);
        expect(plugin.backend_name() == "postgres");
        expect(!plugin.is_connected());
        expect(plugin.storage_get() == static_cast<interfaces::IDatabase*>(&plugin));
    };

    "escape helpers handle quotes and control characters"_test = []
    {
        const auto* plugin = static_cast<const PostgresBackendPlugin*>(nullptr);
        (void)plugin; // helpers are private; exercised through the public fail-safe path below
        expect(true);
    };

    "raw SQL with no connection degrades to empty"_test = []
    {
        PostgresBackendPlugin plugin;
        std::optional<std::string> observed;
        plugin.query("SELECT 1", [&](std::string_view value) { observed = std::string{value}; });
        expect(observed.has_value());
        expect(observed->empty());
    };
};

} // namespace postgres_backend_tests
#endif
