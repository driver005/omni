module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <memory>

export module blog_database:migration_manager;

import std;
import blog_database:database;

export namespace ghost::blog {

using json = nlohmann::json;

/**
 * Migration definition
 */
struct Migration {
    int version;
    std::string name;
    std::string description;
    std::string up_sql;
    std::string down_sql;
    std::function<void(IDatabase&)> up_fn;
    std::function<void(IDatabase&)> down_fn;
    
    Migration(int v, const std::string& n, const std::string& desc)
        : version(v), name(n), description(desc) {}
    
    Migration& up(const std::string& sql) { up_sql = sql; return *this; }
    Migration& down(const std::string& sql) { down_sql = sql; return *this; }
    Migration& up_fn(std::function<void(IDatabase&)> fn) { up_fn = fn; return *this; }
    Migration& down_fn(std::function<void(IDatabase&)> fn) { down_fn = fn; return *this; }
};

/**
 * Migration Result
 */
struct MigrationResult {
    bool success = false;
    int version = 0;
    std::string name;
    std::string error;
    std::chrono::milliseconds duration{0};
};

/**
 * Migration Manager - Handles database schema migrations
 */
class MigrationManager {
public:
    explicit MigrationManager(std::shared_ptr<IDatabase> database);
    
    // Register migrations
    void add_migration(const Migration& migration);
    void add_migration(Migration&& migration);
    
    // Get registered migrations
    const std::vector<Migration>& get_migrations() const;
    std::vector<Migration> get_pending_migrations() const;
    
    // Get current schema version
    int get_current_version() const;
    
    // Run migrations
    MigrationResult migrate_up(int target_version = -1);
    MigrationResult migrate_down(int target_version);
    MigrationResult migrate_to(int target_version);
    
    // Migration status
    struct MigrationStatus {
        int current_version = 0;
        int latest_version = 0;
        std::vector<MigrationInfo> migrations;
    };
    
    struct MigrationInfo {
        int version;
        std::string name;
        std::string description;
        bool applied = false;
        std::string applied_at;
        std::string checksum;
    };
    
    MigrationStatus get_status() const;
    
    // Validation
    bool validate_checksums() const;
    std::vector<std::string> get_checksum_mismatches() const;
    
    // Repair
    bool repair_checksums();
    bool mark_as_applied(int version);
    bool mark_as_unapplied(int version);
    
    // Callbacks
    void set_on_migration_start(std::function<void(const Migration&)> callback);
    void set_on_migration_complete(std::function<void(const MigrationResult&)> callback);
    void set_on_migration_error(std::function<void(const Migration&, const std::string&)> callback);

private:
    std::shared_ptr<IDatabase> database_;
    std::vector<Migration> migrations_;
    
    std::function<void(const Migration&)> on_migration_start_;
    std::function<void(const MigrationResult&)> on_migration_complete_;
    std::function<void(const Migration&, const std::string&)> on_migration_error_;
    
    // Helper methods
    bool ensure_migration_table();
    std::vector<MigrationInfo> get_applied_migrations() const;
    bool record_migration(const Migration& migration, bool success);
    bool remove_migration_record(int version);
    std::string calculate_checksum(const Migration& migration) const;
    MigrationResult execute_migration(const Migration& migration, bool up);
    
    // Built-in migrations
    static std::vector<Migration> create_initial_migrations();
};

} // namespace ghost::blog