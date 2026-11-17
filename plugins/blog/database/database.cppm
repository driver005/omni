module;
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <expected>

export module blog_database:database;

import std;
import blog_api:types;
import connector;

export namespace ghost::blog {

using json = nlohmann::json;

/**
 * Database Configuration
 */
struct DatabaseConfig {
    std::string m_host{};
    int m_port = 5432;
    std::string m_database{};
    std::string m_username{};
    std::string m_password{};
    std::string m_ssl_mode{};
    int m_pool_size = 10;
    int m_max_overflow = 20;
    int m_pool_timeout = 30;
    int m_pool_recycle = 3600;
    bool m_echo_sql = false;

    DatabaseConfig() noexcept = default;

    DatabaseConfig(std::string&& host, int port, std::string&& database,
                   std::string&& username, std::string&& password,
                   std::string&& ssl_mode, int pool_size, int max_overflow,
                   int pool_timeout, int pool_recycle, bool echo_sql) noexcept
        : m_host{std::move(host)}, m_port{port}, m_database{std::move(database)},
          m_username{std::move(username)}, m_password{std::move(password)},
          m_ssl_mode{std::move(ssl_mode)}, m_pool_size{pool_size},
          m_max_overflow{max_overflow}, m_pool_timeout{pool_timeout},
          m_pool_recycle{pool_recycle}, m_echo_sql{echo_sql} {}

    DatabaseConfig& add_host(std::string&& host) noexcept { m_host = std::move(host); return *this; }
    DatabaseConfig& add_port(int port) noexcept { m_port = port; return *this; }
    DatabaseConfig& add_database(std::string&& database) noexcept { m_database = std::move(database); return *this; }
    DatabaseConfig& add_username(std::string&& username) noexcept { m_username = std::move(username); return *this; }
    DatabaseConfig& add_password(std::string&& password) noexcept { m_password = std::move(password); return *this; }
    DatabaseConfig& add_ssl_mode(std::string&& ssl_mode) noexcept { m_ssl_mode = std::move(ssl_mode); return *this; }
    DatabaseConfig& add_pool_size(int size) noexcept { m_pool_size = size; return *this; }
    DatabaseConfig& add_max_overflow(int overflow) noexcept { m_max_overflow = overflow; return *this; }
    DatabaseConfig& add_pool_timeout(int timeout) noexcept { m_pool_timeout = timeout; return *this; }
    DatabaseConfig& add_pool_recycle(int recycle) noexcept { m_pool_recycle = recycle; return *this; }
    DatabaseConfig& add_echo_sql(bool echo) noexcept { m_echo_sql = echo; return *this; }

    void set_host(std::string&& host) noexcept { m_host = std::move(host); }
    void set_port(int port) noexcept { m_port = port; }
    void set_database(std::string&& database) noexcept { m_database = std::move(database); }
    void set_username(std::string&& username) noexcept { m_username = std::move(username); }
    void set_password(std::string&& password) noexcept { m_password = std::move(password); }
    void set_ssl_mode(std::string&& ssl_mode) noexcept { m_ssl_mode = std::move(ssl_mode); }
    void set_pool_size(int size) noexcept { m_pool_size = size; }
    void set_max_overflow(int overflow) noexcept { m_max_overflow = overflow; }
    void set_pool_timeout(int timeout) noexcept { m_pool_timeout = timeout; }
    void set_pool_recycle(int recycle) noexcept { m_pool_recycle = recycle; }
    void set_echo_sql(bool echo) noexcept { m_echo_sql = echo; }

    std::string& get_host() noexcept { return m_host; }
    int& get_port() noexcept { return m_port; }
    std::string& get_database() noexcept { return m_database; }
    std::string& get_username() noexcept { return m_username; }
    std::string& get_password() noexcept { return m_password; }
    std::string& get_ssl_mode() noexcept { return m_ssl_mode; }
    int& get_pool_size() noexcept { return m_pool_size; }
    int& get_max_overflow() noexcept { return m_max_overflow; }
    int& get_pool_timeout() noexcept { return m_pool_timeout; }
    int& get_pool_recycle() noexcept { return m_pool_recycle; }
    bool& get_echo_sql() noexcept { return m_echo_sql; }

    const std::string& get_host() const noexcept { return m_host; }
    int get_port() const noexcept { return m_port; }
    const std::string& get_database() const noexcept { return m_database; }
    const std::string& get_username() const noexcept { return m_username; }
    const std::string& get_password() const noexcept { return m_password; }
    const std::string& get_ssl_mode() const noexcept { return m_ssl_mode; }
    int get_pool_size() const noexcept { return m_pool_size; }
    int get_max_overflow() const noexcept { return m_max_overflow; }
    int get_pool_timeout() const noexcept { return m_pool_timeout; }
    int get_pool_recycle() const noexcept { return m_pool_recycle; }
    bool get_echo_sql() const noexcept { return m_echo_sql; }

    std::string connection_string() const noexcept {
        return "postgresql://" + m_username + ":" + m_password + "@" + m_host + ":" +
               std::to_string(m_port) + "/" + m_database + "?sslmode=" + m_ssl_mode;
    }

    static std::expected<DatabaseConfig, std::string> from_env() noexcept {
        try {
            DatabaseConfig config;
            config.set_host(std::getenv("DB_HOST") ? std::getenv("DB_HOST") : "localhost");
            config.set_port(std::getenv("DB_PORT") ? std::stoi(std::getenv("DB_PORT")) : 5432);
            config.set_database(std::getenv("DB_NAME") ? std::getenv("DB_NAME") : "ghost_blog");
            config.set_username(std::getenv("DB_USER") ? std::getenv("DB_USER") : "postgres");
            config.set_password(std::getenv("DB_PASS") ? std::getenv("DB_PASS") : "");
            config.set_ssl_mode(std::getenv("DB_SSL_MODE") ? std::getenv("DB_SSL_MODE") : "prefer");
            return config;
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    static std::expected<DatabaseConfig, std::string> from_json(const json& config) noexcept {
        try {
            DatabaseConfig cfg;
            cfg.set_host(config.value("host", std::string{"localhost"}));
            cfg.set_port(config.value("port", 5432));
            cfg.set_database(config.value("database", std::string{"ghost_blog"}));
            cfg.set_username(config.value("username", std::string{"postgres"}));
            cfg.set_password(config.value("password", std::string{}));
            cfg.set_ssl_mode(config.value("ssl_mode", std::string{"prefer"}));
            cfg.set_pool_size(config.value("pool_size", 10));
            cfg.set_max_overflow(config.value("max_overflow", 20));
            cfg.set_pool_timeout(config.value("pool_timeout", 30));
            cfg.set_pool_recycle(config.value("pool_recycle", 3600));
            cfg.set_echo_sql(config.value("echo_sql", false));
            return cfg;
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }
};

/**
 * Database Factory
 */
class DatabaseFactory {
public:
    static std::expected<std::unique_ptr<IDatabase>, std::string> create(const DatabaseConfig& config) noexcept {
        return create_postgresql(config);
    }

    static std::expected<std::unique_ptr<IDatabase>, std::string> create_postgresql(const DatabaseConfig& config) noexcept {
        try {
            return std::make_unique<PostgresDatabase>(config);
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    static std::expected<std::unique_ptr<IDatabase>, std::string> create_sqlite(const std::string& path) noexcept {
        try {
            return std::make_unique<SqliteDatabase>(path);
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }
};

/**
 * Database abstraction interface
 */
class IDatabase {
public:
    virtual ~IDatabase() = default;

    virtual std::expected<void, std::string> connect(const std::string& connection_string) noexcept = 0;
    virtual void disconnect() noexcept = 0;
    virtual bool is_connected() const noexcept = 0;
    virtual std::string get_connection_info() const noexcept = 0;

    virtual std::expected<void, std::string> begin_transaction() noexcept = 0;
    virtual std::expected<void, std::string> commit_transaction() noexcept = 0;
    virtual std::expected<void, std::string> rollback_transaction() noexcept = 0;

    virtual std::expected<void, std::string> create_schema() noexcept = 0;
    virtual std::expected<void, std::string> drop_schema() noexcept = 0;
    virtual std::expected<void, std::string> migrate_schema(int target_version = -1) noexcept = 0;
    virtual std::expected<int, std::string> get_schema_version() const noexcept = 0;

    virtual std::expected<json, std::string> execute_query(const std::string& sql, const json& params = {}) noexcept = 0;
    virtual std::expected<int, std::string> execute_update(const std::string& sql, const json& params = {}) noexcept = 0;

    virtual std::expected<bool, std::string> health_check() noexcept = 0;

    virtual connector::Sql* get_connector() noexcept = 0;
};

/**
 * PostgreSQL Database Implementation
 */
class PostgresDatabase : public IDatabase {
public:
    explicit PostgresDatabase(const DatabaseConfig& config) noexcept
        : m_config{config}, m_connected{false}, m_connection_string{} {}

    ~PostgresDatabase() override {
        if (m_connected) {
            disconnect();
        }
    }

    std::expected<void, std::string> connect(const std::string& connection_string) noexcept override {
        try {
            m_connection_string = connection_string.empty() ? m_config.connection_string() : connection_string;
            m_connector = std::make_unique<connector::Sql>(m_connection_string);
            m_connected = true;
            return {};
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    void disconnect() noexcept override {
        m_connector.reset();
        m_connected = false;
    }

    bool is_connected() const noexcept override {
        return m_connected;
    }

    std::string get_connection_info() const noexcept override {
        return m_connection_string;
    }

    std::expected<void, std::string> begin_transaction() noexcept override {
        if (!m_connected || !m_connector) {
            return std::unexpected("Not connected");
        }
        try {
            m_connector->execute("BEGIN");
            return {};
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<void, std::string> commit_transaction() noexcept override {
        if (!m_connected || !m_connector) {
            return std::unexpected("Not connected");
        }
        try {
            m_connector->execute("COMMIT");
            return {};
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<void, std::string> rollback_transaction() noexcept override {
        if (!m_connected || !m_connector) {
            return std::unexpected("Not connected");
        }
        try {
            m_connector->execute("ROLLBACK");
            return {};
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<void, std::string> create_schema() noexcept override {
        if (!m_connected || !m_connector) {
            return std::unexpected("Not connected");
        }
        try {
            // Create posts table
            m_connector->execute(R"(
                CREATE TABLE IF NOT EXISTS posts (
                    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
                    uuid UUID UNIQUE NOT NULL DEFAULT gen_random_uuid(),
                    title VARCHAR(500) NOT NULL,
                    slug VARCHAR(500) UNIQUE NOT NULL,
                    content TEXT,
                    plaintext TEXT,
                    author_id UUID NOT NULL,
                    status VARCHAR(50) DEFAULT 'draft',
                    visibility VARCHAR(50) DEFAULT 'public',
                    created_at TIMESTAMPTZ DEFAULT NOW(),
                    updated_at TIMESTAMPTZ DEFAULT NOW(),
                    published_at TIMESTAMPTZ,
                    scheduled_at TIMESTAMPTZ,
                    feature_image TEXT,
                    feature_image_alt TEXT,
                    feature_image_caption TEXT,
                    excerpt TEXT,
                    codeinjection_head TEXT,
                    codeinjection_foot TEXT,
                    custom_excerpt TEXT,
                    canonical_url TEXT,
                    meta_title TEXT,
                    meta_description TEXT,
                    og_image TEXT,
                    og_title TEXT,
                    og_description TEXT,
                    twitter_image TEXT,
                    twitter_title TEXT,
                    twitter_description TEXT,
                    custom_template TEXT,
                    email_only TEXT,
                    comment_id TEXT,
                    meta JSONB DEFAULT '{}'
                )
            )");

            // Create tags table
            m_connector->execute(R"(
                CREATE TABLE IF NOT EXISTS tags (
                    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
                    name VARCHAR(200) UNIQUE NOT NULL,
                    slug VARCHAR(200) UNIQUE NOT NULL,
                    description TEXT,
                    feature_image TEXT,
                    visibility VARCHAR(50) DEFAULT 'public',
                    meta_title TEXT,
                    meta_description TEXT,
                    og_image TEXT,
                    og_title TEXT,
                    og_description TEXT,
                    twitter_image TEXT,
                    twitter_title TEXT,
                    twitter_description TEXT,
                    created_at TIMESTAMPTZ DEFAULT NOW(),
                    updated_at TIMESTAMPTZ DEFAULT NOW()
                )
            )");

            // Create authors table
            m_connector->execute(R"(
                CREATE TABLE IF NOT EXISTS authors (
                    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
                    name VARCHAR(200) NOT NULL,
                    slug VARCHAR(200) UNIQUE NOT NULL,
                    email VARCHAR(300) UNIQUE NOT NULL,
                    profile_image TEXT,
                    cover_image TEXT,
                    bio TEXT,
                    website TEXT,
                    location TEXT,
                    accessibility TEXT,
                    status VARCHAR(50) DEFAULT 'active',
                    last_login TIMESTAMPTZ,
                    created_at TIMESTAMPTZ DEFAULT NOW(),
                    updated_at TIMESTAMPTZ DEFAULT NOW(),
                    meta JSONB DEFAULT '{}'
                )
            )");

            // Create post_tags junction table
            m_connector->execute(R"(
                CREATE TABLE IF NOT EXISTS post_tags (
                    post_id UUID REFERENCES posts(id) ON DELETE CASCADE,
                    tag_id UUID REFERENCES tags(id) ON DELETE CASCADE,
                    PRIMARY KEY (post_id, tag_id)
                )
            )");

            // Create post_revisions table
            m_connector->execute(R"(
                CREATE TABLE IF NOT EXISTS post_revisions (
                    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
                    post_id UUID REFERENCES posts(id) ON DELETE CASCADE,
                    content TEXT NOT NULL,
                    plaintext TEXT,
                    meta JSONB DEFAULT '{}',
                    created_at TIMESTAMPTZ DEFAULT NOW()
                )
            )");

            // Create schema_migrations table
            m_connector->execute(R"(
                CREATE TABLE IF NOT EXISTS schema_migrations (
                    version INT PRIMARY KEY,
                    name VARCHAR(200) NOT NULL,
                    description TEXT,
                    checksum VARCHAR(64) NOT NULL,
                    applied_at TIMESTAMPTZ DEFAULT NOW()
                )
            )");

            // Create indexes
            m_connector->execute("CREATE INDEX IF NOT EXISTS idx_posts_status ON posts(status)");
            m_connector->execute("CREATE INDEX IF NOT EXISTS idx_posts_author ON posts(author_id)");
            m_connector->execute("CREATE INDEX IF NOT EXISTS idx_posts_published ON posts(published_at)");
            m_connector->execute("CREATE INDEX IF NOT EXISTS idx_posts_slug ON posts(slug)");
            m_connector->execute("CREATE INDEX IF NOT EXISTS idx_tags_slug ON tags(slug)");
            m_connector->execute("CREATE INDEX IF NOT EXISTS idx_authors_slug ON authors(slug)");
            m_connector->execute("CREATE INDEX IF NOT EXISTS idx_authors_email ON authors(email)");

            return {};
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<void, std::string> drop_schema() noexcept override {
        if (!m_connected || !m_connector) {
            return std::unexpected("Not connected");
        }
        try {
            m_connector->execute("DROP TABLE IF EXISTS post_revisions CASCADE");
            m_connector->execute("DROP TABLE IF EXISTS post_tags CASCADE");
            m_connector->execute("DROP TABLE IF EXISTS authors CASCADE");
            m_connector->execute("DROP TABLE IF EXISTS tags CASCADE");
            m_connector->execute("DROP TABLE IF EXISTS posts CASCADE");
            m_connector->execute("DROP TABLE IF EXISTS schema_migrations CASCADE");
            return {};
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<void, std::string> migrate_schema(int target_version = -1) noexcept override {
        // Migration logic would go here
        return {};
    }

    std::expected<int, std::string> get_schema_version() const noexcept override {
        if (!m_connected || !m_connector) {
            return std::unexpected("Not connected");
        }
        try {
            auto result = m_connector->query<int>("SELECT COALESCE(MAX(version), 0) FROM schema_migrations");
            return result.empty() ? 0 : result[0];
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<json, std::string> execute_query(const std::string& sql, const json& params = {}) noexcept override {
        if (!m_connected || !m_connector) {
            return std::unexpected("Not connected");
        }
        try {
            // This is a simplified implementation
            // Real implementation would use parameterized queries
            auto result = m_connector->query<json>(sql);
            return result;
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<int, std::string> execute_update(const std::string& sql, const json& params = {}) noexcept override {
        if (!m_connected || !m_connector) {
            return std::unexpected("Not connected");
        }
        try {
            return m_connector->execute(sql);
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<bool, std::string> health_check() noexcept override {
        if (!m_connected || !m_connector) {
            return false;
        }
        try {
            m_connector->execute("SELECT 1");
            return true;
        } catch (...) {
            return false;
        }
    }

    connector::Sql* get_connector() noexcept override {
        return m_connector.get();
    }

private:
    DatabaseConfig m_config;
    std::unique_ptr<connector::Sql> m_connector;
    bool m_connected;
    std::string m_connection_string;
};

/**
 * SQLite Database Implementation (for testing/development)
 */
class SqliteDatabase : public IDatabase {
public:
    explicit SqliteDatabase(const std::string& path) noexcept
        : m_path{path}, m_db{nullptr}, m_connected{false} {}

    ~SqliteDatabase() override {
        if (m_connected) {
            disconnect();
        }
    }

    std::expected<void, std::string> connect(const std::string& connection_string) noexcept override {
        // SQLite connection logic would go here
        m_connected = true;
        return {};
    }

    void disconnect() noexcept override {
        m_db = nullptr;
        m_connected = false;
    }

    bool is_connected() const noexcept override {
        return m_connected;
    }

    std::string get_connection_info() const noexcept override {
        return m_path;
    }

    std::expected<void, std::string> begin_transaction() noexcept override { return {}; }
    std::expected<void, std::string> commit_transaction() noexcept override { return {}; }
    std::expected<void, std::string> rollback_transaction() noexcept override { return {}; }
    std::expected<void, std::string> create_schema() noexcept override { return {}; }
    std::expected<void, std::string> drop_schema() noexcept override { return {}; }
    std::expected<void, std::string> migrate_schema(int target_version = -1) noexcept override { return {}; }
    std::expected<int, std::string> get_schema_version() const noexcept override { return 0; }
    std::expected<json, std::string> execute_query(const std::string& sql, const json& params = {}) noexcept override { return json{}; }
    std::expected<int, std::string> execute_update(const std::string& sql, const json& params = {}) noexcept override { return 0; }
    std::expected<bool, std::string> health_check() noexcept override { return m_connected; }
    connector::Sql* get_connector() noexcept override { return nullptr; }

private:
    std::string m_path;
    void* m_db;
    bool m_connected;
};

} // namespace ghost::blog