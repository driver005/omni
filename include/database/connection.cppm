export module database.connection;

import std;
import database.types;
import reflection.entity_traits;

export namespace database {

class ConnectionPool {
public:
    explicit ConnectionPool(const DatabaseConfig& config);
    ~ConnectionPool();
    
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;
    ConnectionPool(ConnectionPool&&) = default;
    ConnectionPool& operator=(ConnectionPool&&) = default;
    
    std::shared_ptr<class Connection> acquire();
    void release(std::shared_ptr<Connection> conn);
    size_t size() const;
    size_t available() const;
    void resize(size_t new_size);
    void shutdown();
    
private:
    struct PoolImpl;
    std::unique_ptr<PoolImpl> impl_;
};

export class Database {
public:
    static std::unique_ptr<Database> create(const DatabaseConfig& config);
    static std::unique_ptr<Database> create(const std::string& connection_string);
    
    ~Database();
    
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    Database(Database&&) noexcept;
    Database& operator=(Database&&) noexcept;
    
    bool connect();
    void disconnect();
    bool is_connected() const;
    const DatabaseConfig& config() const;
    
    QueryResult execute(const std::string& query);
    QueryResult execute(const std::string& query, const std::vector<std::variant<
        int64_t, double, bool, std::string, 
        std::vector<uint8_t>, std::chrono::system_clock::time_point
    >>& params);
    
    std::unique_ptr<PreparedStatement> prepare(const std::string& query);
    std::unique_ptr<Transaction> begin_transaction();
    
    template<typename Entity>
    bool create_table() {
        static_assert(std::is_base_of_v<reflection::EntityBase, Entity>, "Entity must inherit from EntityBase");
        return execute(reflection::generate_create_sql<Entity>()).rows_affected >= 0;
    }
    
    template<typename Entity>
    bool drop_table() {
        static_assert(std::is_base_of_v<reflection::EntityBase, Entity>, "Entity must inherit from EntityBase");
        return execute(reflection::generate_drop_sql<Entity>()).rows_affected >= 0;
    }
    
    template<typename Entity>
    bool table_exists() {
        static_assert(std::is_base_of_v<reflection::EntityBase, Entity>, "Entity must inherit from EntityBase");
        auto result = execute(
            "SELECT 1 FROM information_schema.tables WHERE table_schema = $1 AND table_name = $2",
            {reflection::EntityTraits<Entity>::table_type::schema, reflection::EntityTraits<Entity>::table_type::name}
        );
        return !result.rows.empty();
    }
    
    std::shared_ptr<IEntityManager> entity_manager();
    bool run_migrations(const std::string& migrations_path);
    std::vector<std::string> pending_migrations(const std::string& migrations_path);
    bool ping();
    std::string last_error() const;
    
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    
    explicit Database(const DatabaseConfig& config);
};

export class ScopedTransaction {
public:
    explicit ScopedTransaction(Database& db);
    ~ScopedTransaction();
    
    ScopedTransaction(const ScopedTransaction&) = delete;
    ScopedTransaction& operator=(const ScopedTransaction&) = delete;
    ScopedTransaction(ScopedTransaction&&) noexcept;
    ScopedTransaction& operator=(ScopedTransaction&&) noexcept;
    
    QueryResult execute(const std::string& query);
    QueryResult execute(const std::string& query, const std::vector<std::variant<
        int64_t, double, bool, std::string, 
        std::vector<uint8_t>, std::chrono::system_clock::time_point
    >>& params);
    
    void commit();
    void rollback();
    bool is_active() const;
    
    Transaction& transaction();
    
private:
    std::unique_ptr<Transaction> tx_;
    bool committed_ = false;
    bool rolled_back_ = false;
};

export class QueryBuilder {
public:
    QueryBuilder(Database& db, const std::string& table_name);
    
    QueryBuilder& select(const std::vector<std::string>& columns = {"*"});
    QueryBuilder& where(const std::string& condition, const std::vector<std::variant<
        int64_t, double, bool, std::string, 
        std::vector<uint8_t>, std::chrono::system_clock::time_point
    >>& params = {});
    QueryBuilder& where_in(const std::string& column, const std::vector<std::variant<
        int64_t, double, bool, std::string
    >>& values);
    QueryBuilder& order_by(const std::string& column, bool ascending = true);
    QueryBuilder& limit(size_t count);
    QueryBuilder& offset(size_t count);
    QueryBuilder& join(const std::string& table, const std::string& on_condition, JoinType type = JoinType::INNER);
    
    QueryResult get();
    std::optional<QueryResult> first();
    uint64_t count();
    bool exists();
    
    QueryBuilder& insert(const std::map<std::string, std::variant<
        int64_t, double, bool, std::string, 
        std::vector<uint8_t>, std::chrono::system_clock::time_point
    >>& data);
    QueryResult execute_insert();
    
    QueryBuilder& update(const std::map<std::string, std::variant<
        int64_t, double, bool, std::string, 
        std::vector<uint8_t>, std::chrono::system_clock::time_point
    >>& data);
    uint64_t execute_update();
    
    uint64_t execute_delete();
    std::string to_sql() const;
    
private:
    enum class JoinType { INNER, LEFT, RIGHT, FULL };
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

template<typename Entity>
class Repository {
    static_assert(std::is_base_of_v<reflection::EntityBase, Entity>, "Entity must inherit from EntityBase");
    
public:
    explicit Repository(Database& db) : db_(db), table_name_(Entity::table_name()) {}
    
    std::optional<Entity> find(const std::variant<int64_t, std::string>& id) {
        auto result = db_.execute(
            "SELECT * FROM " + table_name_ + " WHERE id = $1",
            {id}
        );
        if (result.rows.empty()) return std::nullopt;
        return row_to_entity(result.rows[0]);
    }
    
    std::vector<Entity> find_all() {
        auto result = db_.execute("SELECT * FROM " + table_name_);
        std::vector<Entity> entities;
        entities.reserve(result.rows.size());
        for (const auto& row : result.rows) {
            entities.push_back(row_to_entity(row));
        }
        return entities;
    }
    
    std::vector<Entity> find_by(const std::map<std::string, std::variant<
        int64_t, double, bool, std::string
    >>& criteria) {
        QueryBuilder qb(db_, table_name_);
        for (const auto& [key, value] : criteria) {
            qb.where(key + " = $" + std::to_string(qb.param_count() + 1), {value});
        }
        auto result = qb.get();
        std::vector<Entity> entities;
        entities.reserve(result.rows.size());
        for (const auto& row : result.rows) {
            entities.push_back(row_to_entity(row));
        }
        return entities;
    }
    
    std::optional<Entity> find_one_by(const std::map<std::string, std::variant<
        int64_t, double, bool, std::string
    >>& criteria) {
        auto results = find_by(criteria);
        if (results.empty()) return std::nullopt;
        return results[0];
    }
    
    Entity save(Entity entity) {
        if (has_id(entity)) {
            return update(entity);
        } else {
            return insert(entity);
        }
    }
    
    Entity insert(Entity entity) {
        auto columns = get_columns(entity);
        columns.erase("id");
        
        QueryBuilder qb(db_, table_name_);
        qb.insert(columns);
        auto result = qb.execute_insert();
        
        if (!result.rows.empty() && result.rows[0].contains("id")) {
            set_id(entity, result.rows[0]["id"]);
        }
        return entity;
    }
    
    Entity update(Entity entity) {
        auto id = get_id(entity);
        auto columns = get_columns(entity);
        columns.erase("id");
        
        QueryBuilder qb(db_, table_name_);
        qb.where("id = $" + std::to_string(qb.param_count() + 1), {id});
        qb.update(columns);
        qb.execute_update();
        
        return entity;
    }
    
    void remove(const Entity& entity) {
        auto id = get_id(entity);
        db_.execute("DELETE FROM " + table_name_ + " WHERE id = $1", {id});
    }
    
    void remove(const std::variant<int64_t, std::string>& id) {
        db_.execute("DELETE FROM " + table_name_ + " WHERE id = $1", {id});
    }
    
    uint64_t count() {
        auto result = db_.execute("SELECT COUNT(*) as cnt FROM " + table_name_);
        return std::get<int64_t>(result.rows[0]["cnt"]);
    }
    
    QueryBuilder query() {
        return QueryBuilder(db_, table_name_);
    }
    
private:
    Database& db_;
    std::string table_name_;
    
    bool has_id(const Entity&) { return false; }
    std::variant<int64_t, std::string> get_id(const Entity&) { return int64_t{0}; }
    void set_id(Entity&, const std::variant<
        int64_t, double, bool, std::string, 
        std::vector<uint8_t>, std::chrono::system_clock::time_point
    >&) {}
    std::map<std::string, std::variant<
        int64_t, double, bool, std::string, 
        std::vector<uint8_t>, std::chrono::system_clock::time_point
    >> get_columns(const Entity&) { return {}; }
    Entity row_to_entity(const std::map<std::string, std::variant<
        int64_t, double, bool, std::string, 
        std::vector<uint8_t>, std::chrono::system_clock::time_point
    >>&) { return Entity{}; }
};

} // namespace database