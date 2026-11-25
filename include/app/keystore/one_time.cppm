module;
export module app_keystore:one_time;

import std;
import core_database;
import app_crypto;
import :record;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::keystore {

/** @brief Repository-backed state for TOTP and recovery-code verification.
 *
 *  Local deterministic substitute for upstream process-tracker / Redis state:
 *  code digests are persisted in the injected core::db::Database and a digest
 *  can be consumed only once. Consumption events are stamped for audit. No
 *  real Redis TTL, distributed lock or external OTP service is involved.
 */
class RepositoryOneTimeStateStore final : public app::crypto::IOneTimeStateStore
{
public:
    explicit RepositoryOneTimeStateStore(core::db::Database& db) :
        m_db{db}
    {
    }

    bool consume_totp(std::string_view code_hash, std::int64_t now_seconds) override
    {
        std::optional<OneTimeCodeRecord> record = find_by_hash_and_kind(code_hash, "totp");
        if (record) {
            if (record->get_used()) {
                return false;
            }
            record->set_used(true);
            record->set_consumed_at(now_seconds);
            return update(*record);
        }
        OneTimeCodeRecord created{};
        created.set_kind("totp");
        created.set_code_hash(std::string{code_hash});
        created.set_used(true);
        created.set_created_at(now_seconds);
        created.set_consumed_at(now_seconds);
        m_db.insert(created);
        return true;
    }

    void record_consumption(
        std::string_view code_hash,
        std::string_view kind,
        std::int64_t at_seconds
    ) override
    {
        std::optional<OneTimeCodeRecord> record = find_by_hash_and_kind(code_hash, kind);
        if (!record) {
            OneTimeCodeRecord created{};
            created.set_kind(std::string{kind});
            created.set_code_hash(std::string{code_hash});
            created.set_used(true);
            created.set_created_at(at_seconds);
            created.set_consumed_at(at_seconds);
            m_db.insert(created);
            return;
        }
        record->set_consumed_at(at_seconds);
        update(*record);
    }

    void save_recovery_codes(
        std::string_view owner,
        const std::vector<std::string>& code_hashes
    ) override
    {
        for (const std::string& hash: code_hashes) {
            if (find_by_owner_hash_kind(owner, hash, "recovery_code")) {
                continue;
            }
            OneTimeCodeRecord record{};
            record.set_owner(std::string{owner});
            record.set_kind("recovery_code");
            record.set_code_hash(hash);
            record.set_used(false);
            record.set_created_at(0);
            record.set_consumed_at(0);
            m_db.insert(record);
        }
    }

    bool consume_recovery_code(std::string_view owner, std::string_view code_hash) override
    {
        std::optional<OneTimeCodeRecord> record =
            find_by_owner_hash_kind(owner, code_hash, "recovery_code");
        if (!record || record->get_used()) {
            return false;
        }
        record->set_used(true);
        return update(*record);
    }

private:
    [[nodiscard]] std::optional<OneTimeCodeRecord>
    find_by_hash_and_kind(std::string_view code_hash, std::string_view kind) const
    {
        core::db::QuerySpec spec = m_db.query<OneTimeCodeRecord>().spec();
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "code_hash",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{code_hash}},
            }
        );
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "kind",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{kind}},
            }
        );
        spec.limit_count = 1;
        return first(spec);
    }

    [[nodiscard]] std::optional<OneTimeCodeRecord> find_by_owner_hash_kind(
        std::string_view owner,
        std::string_view code_hash,
        std::string_view kind
    ) const
    {
        core::db::QuerySpec spec = m_db.query<OneTimeCodeRecord>().spec();
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "owner",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{owner}},
            }
        );
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "code_hash",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{code_hash}},
            }
        );
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "kind",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{kind}},
            }
        );
        spec.limit_count = 1;
        return first(spec);
    }

    [[nodiscard]] std::optional<OneTimeCodeRecord> first(const core::db::QuerySpec& spec) const
    {
        const std::vector<OneTimeCodeRecord> rows = m_db.collect<OneTimeCodeRecord>(spec);
        if (rows.empty()) {
            return std::nullopt;
        }
        return rows.front();
    }

    bool update(const OneTimeCodeRecord& record)
    {
        return m_db.update_where_column<OneTimeCodeRecord>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{record.get_id()}
               ) > 0;
    }

    core::db::Database& m_db;
};

} // namespace app::keystore

#ifdef CONGELADO_TEST
namespace app::keystore::tests {
using namespace boost::ut;

suite<"app_keystore_one_time"> one_time_suite = []
{
    "implements the crypto one-time state interface"_test = []
    {
        static_assert(
            std::derived_from<RepositoryOneTimeStateStore, app::crypto::IOneTimeStateStore>
        );
        core::db::Database db{};
        RepositoryOneTimeStateStore store{db};
        (void)store;
        expect(true);
    };
};

} // namespace app::keystore::tests
#endif
