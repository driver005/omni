module;
export module app_store:risk_records;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::gsm_rules {

/** @brief Persisted GSM inference rule keyed by its natural key. */
class GsmRuleRecord
{
public:
    GsmRuleRecord() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_natural_key(std::string value) { m_natural_key = std::move(value); }
    [[nodiscard]] const std::string& get_natural_key() const noexcept { return m_natural_key; }
    void set_merchant_id(std::string value) { m_merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return m_merchant_id; }
    void set_rule_name(std::string value) { m_rule_name = std::move(value); }
    [[nodiscard]] const std::string& get_rule_name() const noexcept { return m_rule_name; }
    void set_decision(std::string value) { m_decision = std::move(value); }
    [[nodiscard]] const std::string& get_decision() const noexcept { return m_decision; }
    void set_program_json(std::string value) { m_program_json = std::move(value); }
    [[nodiscard]] const std::string& get_program_json() const noexcept { return m_program_json; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_created_at(std::string value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::string& get_created_at() const noexcept { return m_created_at; }

    std::int64_t m_id{};
    std::string m_natural_key{};
    std::string m_merchant_id{};
    std::string m_rule_name{};
    std::string m_decision{};
    std::string m_program_json{};
    std::string m_status{};
    std::string m_created_at{};
};

} // namespace app::store::gsm_rules

template<>
struct serde::Serializable<app::store::gsm_rules::GsmRuleRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::store::gsm_rules::GsmRuleRecord::get_id, &app::store::gsm_rules::GsmRuleRecord::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"natural_key", &app::store::gsm_rules::GsmRuleRecord::get_natural_key, &app::store::gsm_rules::GsmRuleRecord::set_natural_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::store::gsm_rules::GsmRuleRecord::get_merchant_id, &app::store::gsm_rules::GsmRuleRecord::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"rule_name", &app::store::gsm_rules::GsmRuleRecord::get_rule_name, &app::store::gsm_rules::GsmRuleRecord::set_rule_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"decision", &app::store::gsm_rules::GsmRuleRecord::get_decision, &app::store::gsm_rules::GsmRuleRecord::set_decision, serde::FieldOptions{}>{},
            serde::FieldDesc<"program_json", &app::store::gsm_rules::GsmRuleRecord::get_program_json, &app::store::gsm_rules::GsmRuleRecord::set_program_json, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::store::gsm_rules::GsmRuleRecord::get_status, &app::store::gsm_rules::GsmRuleRecord::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::store::gsm_rules::GsmRuleRecord::get_created_at, &app::store::gsm_rules::GsmRuleRecord::set_created_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "gsm_rules"; }
};

export namespace app::store::blocklist_entries {

/** @brief Persisted blocklist entry with a hashed fingerprint. */
class BlocklistEntryRecord
{
public:
    BlocklistEntryRecord() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_entry_id(std::string value) { m_entry_id = std::move(value); }
    [[nodiscard]] const std::string& get_entry_id() const noexcept { return m_entry_id; }
    void set_merchant_id(std::string value) { m_merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return m_merchant_id; }
    void set_profile_id(std::string value) { m_profile_id = std::move(value); }
    [[nodiscard]] const std::string& get_profile_id() const noexcept { return m_profile_id; }
    void set_data_kind(std::string value) { m_data_kind = std::move(value); }
    [[nodiscard]] const std::string& get_data_kind() const noexcept { return m_data_kind; }
    void set_transaction_type(std::string value) { m_transaction_type = std::move(value); }
    [[nodiscard]] const std::string& get_transaction_type() const noexcept { return m_transaction_type; }
    void set_fingerprint(std::string value) { m_fingerprint = std::move(value); }
    [[nodiscard]] const std::string& get_fingerprint() const noexcept { return m_fingerprint; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_created_at(std::string value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::string& get_created_at() const noexcept { return m_created_at; }

    std::int64_t m_id{};
    std::string m_entry_id{};
    std::string m_merchant_id{};
    std::string m_profile_id{};
    std::string m_data_kind{};
    std::string m_transaction_type{};
    std::string m_fingerprint{};
    std::string m_status{};
    std::string m_created_at{};
};

} // namespace app::store::blocklist_entries

template<>
struct serde::Serializable<app::store::blocklist_entries::BlocklistEntryRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::store::blocklist_entries::BlocklistEntryRecord::get_id, &app::store::blocklist_entries::BlocklistEntryRecord::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"entry_id", &app::store::blocklist_entries::BlocklistEntryRecord::get_entry_id, &app::store::blocklist_entries::BlocklistEntryRecord::set_entry_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::store::blocklist_entries::BlocklistEntryRecord::get_merchant_id, &app::store::blocklist_entries::BlocklistEntryRecord::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::store::blocklist_entries::BlocklistEntryRecord::get_profile_id, &app::store::blocklist_entries::BlocklistEntryRecord::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"data_kind", &app::store::blocklist_entries::BlocklistEntryRecord::get_data_kind, &app::store::blocklist_entries::BlocklistEntryRecord::set_data_kind, serde::FieldOptions{}>{},
            serde::FieldDesc<"transaction_type", &app::store::blocklist_entries::BlocklistEntryRecord::get_transaction_type, &app::store::blocklist_entries::BlocklistEntryRecord::set_transaction_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"fingerprint", &app::store::blocklist_entries::BlocklistEntryRecord::get_fingerprint, &app::store::blocklist_entries::BlocklistEntryRecord::set_fingerprint, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::store::blocklist_entries::BlocklistEntryRecord::get_status, &app::store::blocklist_entries::BlocklistEntryRecord::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::store::blocklist_entries::BlocklistEntryRecord::get_created_at, &app::store::blocklist_entries::BlocklistEntryRecord::set_created_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "blocklist_entries"; }
};

export namespace app::store::fraud_checks {

/** @brief Persisted fraud-management check result. */
class FraudCheckRecord
{
public:
    FraudCheckRecord() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_fraud_check_id(std::string value) { m_fraud_check_id = std::move(value); }
    [[nodiscard]] const std::string& get_fraud_check_id() const noexcept { return m_fraud_check_id; }
    void set_payment_id(std::string value) { m_payment_id = std::move(value); }
    [[nodiscard]] const std::string& get_payment_id() const noexcept { return m_payment_id; }
    void set_merchant_id(std::string value) { m_merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return m_merchant_id; }
    void set_connector(std::string value) { m_connector = std::move(value); }
    [[nodiscard]] const std::string& get_connector() const noexcept { return m_connector; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_score(std::int64_t value) { m_score = std::move(value); }
    [[nodiscard]] const std::int64_t& get_score() const noexcept { return m_score; }
    void set_decision(std::string value) { m_decision = std::move(value); }
    [[nodiscard]] const std::string& get_decision() const noexcept { return m_decision; }
    void set_created_at(std::string value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::string& get_created_at() const noexcept { return m_created_at; }

    std::int64_t m_id{};
    std::string m_fraud_check_id{};
    std::string m_payment_id{};
    std::string m_merchant_id{};
    std::string m_connector{};
    std::string m_status{};
    std::int64_t m_score{};
    std::string m_decision{};
    std::string m_created_at{};
};

} // namespace app::store::fraud_checks

template<>
struct serde::Serializable<app::store::fraud_checks::FraudCheckRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::store::fraud_checks::FraudCheckRecord::get_id, &app::store::fraud_checks::FraudCheckRecord::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"fraud_check_id", &app::store::fraud_checks::FraudCheckRecord::get_fraud_check_id, &app::store::fraud_checks::FraudCheckRecord::set_fraud_check_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_id", &app::store::fraud_checks::FraudCheckRecord::get_payment_id, &app::store::fraud_checks::FraudCheckRecord::set_payment_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::store::fraud_checks::FraudCheckRecord::get_merchant_id, &app::store::fraud_checks::FraudCheckRecord::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &app::store::fraud_checks::FraudCheckRecord::get_connector, &app::store::fraud_checks::FraudCheckRecord::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::store::fraud_checks::FraudCheckRecord::get_status, &app::store::fraud_checks::FraudCheckRecord::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"score", &app::store::fraud_checks::FraudCheckRecord::get_score, &app::store::fraud_checks::FraudCheckRecord::set_score, serde::FieldOptions{}>{},
            serde::FieldDesc<"decision", &app::store::fraud_checks::FraudCheckRecord::get_decision, &app::store::fraud_checks::FraudCheckRecord::set_decision, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::store::fraud_checks::FraudCheckRecord::get_created_at, &app::store::fraud_checks::FraudCheckRecord::set_created_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "fraud_checks"; }
};

export namespace app::store::decision_configs {

/** @brief Persisted decision-engine program configuration. */
class DecisionConfigRecord
{
public:
    DecisionConfigRecord() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_config_id(std::string value) { m_config_id = std::move(value); }
    [[nodiscard]] const std::string& get_config_id() const noexcept { return m_config_id; }
    void set_merchant_id(std::string value) { m_merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return m_merchant_id; }
    void set_profile_id(std::string value) { m_profile_id = std::move(value); }
    [[nodiscard]] const std::string& get_profile_id() const noexcept { return m_profile_id; }
    void set_name(std::string value) { m_name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return m_name; }
    void set_program_json(std::string value) { m_program_json = std::move(value); }
    [[nodiscard]] const std::string& get_program_json() const noexcept { return m_program_json; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_created_at(std::string value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::string& get_created_at() const noexcept { return m_created_at; }

    std::int64_t m_id{};
    std::string m_config_id{};
    std::string m_merchant_id{};
    std::string m_profile_id{};
    std::string m_name{};
    std::string m_program_json{};
    std::string m_status{};
    std::string m_created_at{};
};

} // namespace app::store::decision_configs

template<>
struct serde::Serializable<app::store::decision_configs::DecisionConfigRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::store::decision_configs::DecisionConfigRecord::get_id, &app::store::decision_configs::DecisionConfigRecord::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"config_id", &app::store::decision_configs::DecisionConfigRecord::get_config_id, &app::store::decision_configs::DecisionConfigRecord::set_config_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::store::decision_configs::DecisionConfigRecord::get_merchant_id, &app::store::decision_configs::DecisionConfigRecord::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::store::decision_configs::DecisionConfigRecord::get_profile_id, &app::store::decision_configs::DecisionConfigRecord::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &app::store::decision_configs::DecisionConfigRecord::get_name, &app::store::decision_configs::DecisionConfigRecord::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"program_json", &app::store::decision_configs::DecisionConfigRecord::get_program_json, &app::store::decision_configs::DecisionConfigRecord::set_program_json, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::store::decision_configs::DecisionConfigRecord::get_status, &app::store::decision_configs::DecisionConfigRecord::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::store::decision_configs::DecisionConfigRecord::get_created_at, &app::store::decision_configs::DecisionConfigRecord::set_created_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "decision_configs"; }
};

#ifdef CONGELADO_TEST
namespace app::store::risk_records::tests {

using namespace boost::ut;

suite<"app_store_risk_records"> risk_records_suite = []
{
    "gsm rule fields and table"_test = []
    {
        gsm_rules::GsmRuleRecord record{};
        record.set_natural_key("payment_method:card");
        expect(record.get_natural_key() == "payment_method:card");
        expect(serde::Serializable<gsm_rules::GsmRuleRecord>::table_name() == "gsm_rules");
    };

    "blocklist entry fields and table"_test = []
    {
        blocklist_entries::BlocklistEntryRecord record{};
        record.set_fingerprint("fp_1");
        record.set_transaction_type("payment");
        expect(record.get_fingerprint() == "fp_1");
        expect(serde::Serializable<blocklist_entries::BlocklistEntryRecord>::table_name() == "blocklist_entries");
    };

    "fraud check fields and table"_test = []
    {
        fraud_checks::FraudCheckRecord record{};
        record.set_fraud_check_id("fc_1");
        record.set_score(42);
        expect(record.get_score() == 42);
        expect(serde::Serializable<fraud_checks::FraudCheckRecord>::table_name() == "fraud_checks");
    };

    "decision config fields and table"_test = []
    {
        decision_configs::DecisionConfigRecord record{};
        record.set_config_id("dc_1");
        expect(record.get_config_id() == "dc_1");
        expect(serde::Serializable<decision_configs::DecisionConfigRecord>::table_name() == "decision_configs");
    };
};

} // namespace app::store::risk_records::tests
#endif
