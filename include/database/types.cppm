export module database.types;

import std;

export namespace database {

struct DatabaseConfig
{
    std::string host = "localhost";
    uint16_t port = 5'432;
    std::string database;
    std::string username;
    std::string password;
    std::string schema = "public";
    uint32_t pool_min = 2;
    uint32_t pool_max = 10;
    bool ssl = false;
    std::chrono::seconds connect_timeout{30};

    std::string connection_string() const
    {
        return "postgresql://" + username + ":" + password + "@" + host + ":" +
               std::to_string(port) + "/" + database + "?schema=" + schema;
    }
};

export enum class ColumnType
{
    BIGINT,
    BIGSERIAL,
    BOOLEAN,
    BYTEA,
    CHAR,
    CIDR,
    DATE,
    DOUBLE_PRECISION,
    INET,
    INTEGER,
    INTERVAL,
    JSON,
    JSONB,
    MACADDR,
    MONEY,
    NUMERIC,
    OID,
    REAL,
    SMALLINT,
    SERIAL,
    TEXT,
    TIME,
    TIMESTAMP,
    TIMESTAMPTZ,
    TIMETZ,
    UUID,
    VARCHAR,
    XML,
    ARRAY,
    CUSTOM
};

export struct ColumnDef
{
    std::string name;
    ColumnType type;
    bool primary_key = false;
    bool nullable = true;
    bool unique = false;
    std::optional<std::string> default_value;
    std::optional<std::string> check_constraint;
    std::optional<std::pair<std::string, std::string>> foreign_key;
    std::string custom_type;

    std::string to_sql() const;
};

export struct IndexDef
{
    std::string name;
    std::vector<std::string> columns;
    bool unique = false;
    std::string method = "btree";
    std::optional<std::string> where_clause;

    std::string to_sql(const std::string& table_name) const;
};

export struct CheckConstraint
{
    std::string name;
    std::string expression;

    std::string to_sql() const
    {
        return "CONSTRAINT " + name + " CHECK (" + expression + ")";
    }
};

export struct TableDef
{
    std::string name;
    std::string schema = "public";
    std::vector<ColumnDef> columns;
    std::vector<IndexDef> indexes;
    std::vector<CheckConstraint> checks;
    std::vector<std::string> primary_keys;

    std::string create_sql() const;
    std::string drop_sql() const;
};

export struct QueryResult
{
    uint64_t rows_affected = 0;
    std::vector<std::map<
        std::string,
        std::variant<
            int64_t,
            double,
            bool,
            std::string,
            std::vector<uint8_t>,
            std::chrono::system_clock::time_point>>>
        rows;

    template<typename T>
    std::optional<T> get(uint64_t row, const std::string& column) const
    {
        if (row >= rows.size()) {
            return std::nullopt;
        }
        auto it = rows[row].find(column);
        if (it == rows[row].end()) {
            return std::nullopt;
        }
        return std::get<T>(it->second);
    }
};

export class PreparedStatement
{
public:
    virtual ~PreparedStatement() = default;
    virtual QueryResult execute(
        const std::vector<std::variant<
            int64_t,
            double,
            bool,
            std::string,
            std::vector<uint8_t>,
            std::chrono::system_clock::time_point>>& params
    ) = 0;
    virtual std::string name() const = 0;
};

export class IDatabaseAdapter
{
public:
    virtual ~IDatabaseAdapter() = default;
    virtual bool connect(const DatabaseConfig& config) = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() const = 0;
    virtual QueryResult execute(const std::string& query) = 0;
    virtual QueryResult execute(
        const std::string& query,
        const std::vector<std::variant<
            int64_t,
            double,
            bool,
            std::string,
            std::vector<uint8_t>,
            std::chrono::system_clock::time_point>>& params
    ) = 0;
    virtual std::unique_ptr<PreparedStatement> prepare(const std::string& query) = 0;
    virtual std::unique_ptr<class Transaction> begin_transaction() = 0;
    virtual std::string last_error() const = 0;
};

export class Transaction
{
public:
    virtual ~Transaction() = default;
    virtual QueryResult execute(const std::string& query) = 0;
    virtual QueryResult execute(
        const std::string& query,
        const std::vector<std::variant<
            int64_t,
            double,
            bool,
            std::string,
            std::vector<uint8_t>,
            std::chrono::system_clock::time_point>>& params
    ) = 0;
    virtual void commit() = 0;
    virtual void rollback() = 0;
    virtual bool is_active() const = 0;
};

export class IEntityManager
{
public:
    virtual ~IEntityManager() = default;
    virtual void persist(void* entity) = 0;
    virtual void remove(void* entity) = 0;
    virtual void flush() = 0;
    virtual void clear() = 0;
    virtual void*
    find(const std::type_info& type, const std::variant<int64_t, std::string>& id) = 0;
    virtual std::vector<void*> find_all(const std::type_info& type) = 0;
    virtual std::vector<void*> find_by(
        const std::type_info& type,
        const std::map<std::string, std::variant<int64_t, double, bool, std::string>>& criteria
    ) = 0;
    virtual class QueryBuilder create_query_builder(const std::type_info& type) = 0;
};

} // namespace database
