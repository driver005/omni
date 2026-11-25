module;
export module app_keystore:store;

import std;
import core_database;
import app_crypto;
import :record;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::keystore {

/** @brief Repository-backed key store with purpose/key-id natural keys.
 *
 *  This is a deterministic local stand-in for upstream configured-key storage:
 *  it persists through the injected core::db::Database (the same backend
 *  app_store::AppRepository wraps) and never emits a secret in a log line or a
 *  cache key. No real key-management service or hardware protector is used.
 */
class KeyStore
{
public:
    explicit KeyStore(core::db::Database& db) :
        m_db{db}
    {
    }

    /** @brief Returns the active secret for (purpose, key_id), or nothing. */
    [[nodiscard]] std::optional<app::crypto::SecretString>
    get(std::string_view purpose, std::string_view key_id) const
    {
        const std::optional<KeyStoreRecord> record = find_record(purpose, key_id);
        if (!record || !record->get_active()) {
            return std::nullopt;
        }
        return app::crypto::SecretString{record->get_value()};
    }

    /** @brief Inserts a key. Returns false when the natural key already exists. */
    bool put(
        std::string_view purpose,
        std::string_view key_id,
        std::string_view algorithm,
        app::crypto::SecretString value,
        std::int64_t created_at = 0
    )
    {
        if (find_record(purpose, key_id)) {
            return false;
        }
        KeyStoreRecord record{};
        record.set_purpose(std::string{purpose});
        record.set_key_id(std::string{key_id});
        record.set_algorithm(std::string{algorithm});
        record.set_value(value.value());
        record.set_created_at(created_at);
        record.set_rotated_at(0);
        record.set_active(true);
        m_db.insert(record);
        return true;
    }

    /** @brief Replaces the value of an existing key and stamps rotation time. */
    bool rotate(
        std::string_view purpose,
        std::string_view key_id,
        app::crypto::SecretString new_value,
        std::int64_t rotated_at = 0
    )
    {
        std::optional<KeyStoreRecord> record = find_record(purpose, key_id);
        if (!record) {
            return false;
        }
        record->set_value(new_value.value());
        record->set_rotated_at(rotated_at);
        record->set_active(true);
        return m_db.update_where_column<KeyStoreRecord>(
                   *record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record->get_id()}
               ) > 0;
    }

    /** @brief Internal lookup; the returned record contains the raw value.
     *
     *  Callers must not pass this record to a logger, response DTO or cache key.
     *  Only get() and KeyStoreSecretResolver are presentation-safe entry points.
     */
    [[nodiscard]] std::optional<KeyStoreRecord>
    find_record(std::string_view purpose, std::string_view key_id) const
    {
        core::db::QuerySpec spec = m_db.query<KeyStoreRecord>().spec();
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "purpose",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{purpose}},
            }
        );
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "key_id",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{key_id}},
            }
        );
        spec.limit_count = 1;
        const std::vector<KeyStoreRecord> rows = m_db.collect<KeyStoreRecord>(spec);
        if (rows.empty()) {
            return std::nullopt;
        }
        return rows.front();
    }

private:
    core::db::Database& m_db;
};

/** @brief Adapts KeyStore to the JwtSigner secret-resolution interface. */
class KeyStoreSecretResolver final : public app::crypto::ISecretResolver
{
public:
    explicit KeyStoreSecretResolver(KeyStore& store) :
        m_store{store}
    {
    }

    [[nodiscard]] std::optional<std::string>
    resolve(std::string_view purpose, std::string_view key_id) override
    {
        const std::optional<app::crypto::SecretString> secret = m_store.get(purpose, key_id);
        if (!secret) {
            return std::nullopt;
        }
        return secret->value();
    }

private:
    KeyStore& m_store;
};

} // namespace app::keystore

#ifdef CONGELADO_TEST
namespace app::keystore::tests {
using namespace boost::ut;

suite<"app_keystore_store"> store_suite = []
{
    "constructs with an injected database handle"_test = []
    {
        core::db::Database db{};
        KeyStore store{db};
        KeyStoreSecretResolver resolver{store};
        static_assert(std::derived_from<KeyStoreSecretResolver, app::crypto::ISecretResolver>);
        (void)store;
        (void)resolver;
        expect(true);
    };
};

} // namespace app::keystore::tests
#endif
