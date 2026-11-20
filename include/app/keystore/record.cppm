module;
export module app_keystore:record;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

/** @brief Persisted key and one-time-code records.
 *
 *  These records are the local, deterministic stand-in for upstream key-store
 *  and Redis-backed one-time state. The secret value is stored here so it can
 *  be resolved at runtime; never serialize these records into a response, log
 *  line or cache key — use SecretString::masked() and cache_key() instead.
 */
export namespace app::keystore {

/** @brief One configured signing/encryption key, keyed by (purpose, key_id). */
class KeyStoreRecord
{
public:
    KeyStoreRecord() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_purpose(std::string value) { m_purpose = std::move(value); }
    [[nodiscard]] const std::string& get_purpose() const noexcept { return m_purpose; }
    void set_key_id(std::string value) { m_key_id = std::move(value); }
    [[nodiscard]] const std::string& get_key_id() const noexcept { return m_key_id; }
    void set_algorithm(std::string value) { m_algorithm = std::move(value); }
    [[nodiscard]] const std::string& get_algorithm() const noexcept { return m_algorithm; }
    void set_value(std::string value) { m_value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return m_value; }
    void set_created_at(std::int64_t value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::int64_t& get_created_at() const noexcept { return m_created_at; }
    void set_rotated_at(std::int64_t value) { m_rotated_at = std::move(value); }
    [[nodiscard]] const std::int64_t& get_rotated_at() const noexcept { return m_rotated_at; }
    void set_active(bool value) { m_active = std::move(value); }
    [[nodiscard]] const bool& get_active() const noexcept { return m_active; }

    std::int64_t m_id{};
    std::string m_purpose{};
    std::string m_key_id{};
    std::string m_algorithm{};
    std::string m_value{};
    std::int64_t m_created_at{};
    std::int64_t m_rotated_at{};
    bool m_active{};
};

/** @brief A single one-time-code digest and its consumption state. */
class OneTimeCodeRecord
{
public:
    OneTimeCodeRecord() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_owner(std::string value) { m_owner = std::move(value); }
    [[nodiscard]] const std::string& get_owner() const noexcept { return m_owner; }
    void set_kind(std::string value) { m_kind = std::move(value); }
    [[nodiscard]] const std::string& get_kind() const noexcept { return m_kind; }
    void set_code_hash(std::string value) { m_code_hash = std::move(value); }
    [[nodiscard]] const std::string& get_code_hash() const noexcept { return m_code_hash; }
    void set_used(bool value) { m_used = std::move(value); }
    [[nodiscard]] const bool& get_used() const noexcept { return m_used; }
    void set_created_at(std::int64_t value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::int64_t& get_created_at() const noexcept { return m_created_at; }
    void set_consumed_at(std::int64_t value) { m_consumed_at = std::move(value); }
    [[nodiscard]] const std::int64_t& get_consumed_at() const noexcept { return m_consumed_at; }

    std::int64_t m_id{};
    std::string m_owner{};
    std::string m_kind{};
    std::string m_code_hash{};
    bool m_used{};
    std::int64_t m_created_at{};
    std::int64_t m_consumed_at{};
};

} // namespace app::keystore

template<>
struct serde::Serializable<app::keystore::KeyStoreRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::keystore::KeyStoreRecord::get_id, &app::keystore::KeyStoreRecord::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"purpose", &app::keystore::KeyStoreRecord::get_purpose, &app::keystore::KeyStoreRecord::set_purpose, serde::FieldOptions{}>{},
            serde::FieldDesc<"key_id", &app::keystore::KeyStoreRecord::get_key_id, &app::keystore::KeyStoreRecord::set_key_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"algorithm", &app::keystore::KeyStoreRecord::get_algorithm, &app::keystore::KeyStoreRecord::set_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"value", &app::keystore::KeyStoreRecord::get_value, &app::keystore::KeyStoreRecord::set_value, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::keystore::KeyStoreRecord::get_created_at, &app::keystore::KeyStoreRecord::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"rotated_at", &app::keystore::KeyStoreRecord::get_rotated_at, &app::keystore::KeyStoreRecord::set_rotated_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"active", &app::keystore::KeyStoreRecord::get_active, &app::keystore::KeyStoreRecord::set_active, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "key_store"; }
};

template<>
struct serde::Serializable<app::keystore::OneTimeCodeRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::keystore::OneTimeCodeRecord::get_id, &app::keystore::OneTimeCodeRecord::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"owner", &app::keystore::OneTimeCodeRecord::get_owner, &app::keystore::OneTimeCodeRecord::set_owner, serde::FieldOptions{}>{},
            serde::FieldDesc<"kind", &app::keystore::OneTimeCodeRecord::get_kind, &app::keystore::OneTimeCodeRecord::set_kind, serde::FieldOptions{}>{},
            serde::FieldDesc<"code_hash", &app::keystore::OneTimeCodeRecord::get_code_hash, &app::keystore::OneTimeCodeRecord::set_code_hash, serde::FieldOptions{}>{},
            serde::FieldDesc<"used", &app::keystore::OneTimeCodeRecord::get_used, &app::keystore::OneTimeCodeRecord::set_used, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::keystore::OneTimeCodeRecord::get_created_at, &app::keystore::OneTimeCodeRecord::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"consumed_at", &app::keystore::OneTimeCodeRecord::get_consumed_at, &app::keystore::OneTimeCodeRecord::set_consumed_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "one_time_codes"; }
};

#ifdef CONGELADO_TEST
namespace app::keystore::tests {
using namespace boost::ut;

suite<"app_keystore_record"> record_suite = []
{
    "key store record carries the configured key metadata"_test = []
    {
        KeyStoreRecord record{};
        record.set_purpose("jwt");
        record.set_key_id("key_test");
        record.set_algorithm("HS256");
        record.set_value("secret-value");
        record.set_rotated_at(42);
        record.set_active(true);
        expect(record.get_purpose() == "jwt");
        expect(record.get_key_id() == "key_test");
        expect(record.get_algorithm() == "HS256");
        expect(record.get_value() == "secret-value");
        expect(record.get_rotated_at() == 42);
        expect(record.get_active());
        expect(serde::Serializable<KeyStoreRecord>::table_name() == "key_store");
    };

    "one time code record carries consumption state"_test = []
    {
        OneTimeCodeRecord record{};
        record.set_owner("merchant_1");
        record.set_kind("totp");
        record.set_code_hash("abcdef");
        record.set_used(true);
        expect(record.get_owner() == "merchant_1");
        expect(record.get_kind() == "totp");
        expect(record.get_used());
        expect(serde::Serializable<OneTimeCodeRecord>::table_name() == "one_time_codes");
    };
};

} // namespace app::keystore::tests
#endif
