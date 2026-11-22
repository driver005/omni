module;
export module app_services:decision;

import std;
import core_serde;
import core_error;
import app_metrics;
import app_store;
import :config;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::services {

/** @brief Result of one decision-engine evaluation. */
class Decision
{
public:
    Decision() = default;

    void set_action(std::string value)
    {
        m_action = std::move(value);
    }
    [[nodiscard]] const std::string& get_action() const noexcept
    {
        return m_action;
    }

    void set_connector_name(std::string value)
    {
        m_connector_name = std::move(value);
    }
    [[nodiscard]] const std::string& get_connector_name() const noexcept
    {
        return m_connector_name;
    }

    void set_reason(std::string value)
    {
        m_reason = std::move(value);
    }
    [[nodiscard]] const std::string& get_reason() const noexcept
    {
        return m_reason;
    }

    void set_matched(bool value) noexcept
    {
        m_matched = value;
    }
    [[nodiscard]] const bool& get_matched() const noexcept
    {
        return m_matched;
    }

    /** @brief Wire name of a typed error, empty when the program was understood. */
    void set_error_kind(std::string value)
    {
        m_error_kind = std::move(value);
    }
    [[nodiscard]] const std::string& get_error_kind() const noexcept
    {
        return m_error_kind;
    }

private:
    std::string m_action{};
    std::string m_connector_name{};
    std::string m_reason{};
    bool m_matched{};
    std::string m_error_kind{};
};

/**
 * Local DSL subset (JSON, decoded by reflect-cpp into the plain structs below):
 *   program  := { "rules": [ rule, ... ], "default_action": "...", "default_connector": "..." }
 *   rule     := { "if_condition": { "field": ..., "op": ..., "value": ... },
 *                 "action": "...", "connector": "..." }
 *   context  := { "amount": "...", "currency": "...", "payment_method": "...",
 *                 "country": "...", "connector": "..." }
 * Supported fields: amount, currency, payment_method, country, connector.
 * Supported ops: eq, neq, contains, in (comma-separated), gt, gte, lt, lte (numeric).
 * Any other field, operator, or malformed JSON yields a typed VALIDATION_FAILED error —
 * the engine never silently falls back to a default.
 */
struct DecisionCondition
{
    std::string field{};
    std::string op{};
    std::string value{};
};

struct DecisionRule
{
    DecisionCondition if_condition{};
    std::string action{};
    std::string connector{};
};

struct DecisionProgram
{
    std::vector<DecisionRule> rules{};
    std::string default_action{};
    std::string default_connector{};
};

struct DecisionContext
{
    std::string amount{};
    std::string currency{};
    std::string payment_method{};
    std::string country{};
    std::string connector{};
};

/** @brief Deterministic, in-process evaluator for the documented DSL subset. */
class DecisionEngine
{
public:
    DecisionEngine() = default;

    /** @brief Typed-error evaluation. Unknown programs never default. */
    [[nodiscard]] std::expected<Decision, core::error::AppError>
    evaluate_checked(std::string_view program_json, std::string_view context_json) const
    {
        const auto program = core_serde::JsonCodec{}.decode<DecisionProgram>(program_json);
        if (!program) {
            return std::unexpected{validation_error("unknown decision program")};
        }
        const auto context = core_serde::JsonCodec{}.decode<DecisionContext>(context_json);
        if (!context) {
            return std::unexpected{validation_error("invalid decision context")};
        }
        return evaluate_program(*program, *context);
    }

    /** @brief Value-returning facade: on failure Decision carries error_kind = VALIDATION_FAILED. */
    [[nodiscard]] Decision evaluate(std::string_view program_json, std::string_view context_json) const
    {
        auto checked = evaluate_checked(program_json, context_json);
        if (checked.has_value()) {
            return std::move(*checked);
        }
        Decision decision{};
        decision.set_action("error");
        decision.set_error_kind("VALIDATION_FAILED");
        decision.set_reason(checked.error().get_message());
        return decision;
    }

    [[nodiscard]] std::expected<Decision, core::error::AppError>
    evaluate_program(const DecisionProgram& program, const DecisionContext& context) const
    {
        if (program.rules.empty() && program.default_action.empty()) {
            return std::unexpected{validation_error("program has no rules and no default action")};
        }
        for (const auto& rule: program.rules) {
            auto matched = matches(
                rule.if_condition.field,
                rule.if_condition.op,
                rule.if_condition.value,
                context
            );
            if (!matched.has_value()) {
                return std::unexpected{std::move(matched.error())};
            }
            if (*matched) {
                Decision decision{};
                decision.set_action(rule.action);
                decision.set_connector_name(rule.connector);
                decision.set_reason("rule matched");
                decision.set_matched(true);
                return decision;
            }
        }
        Decision decision{};
        decision.set_action(program.default_action.empty() ? "allow" : program.default_action);
        decision.set_connector_name(program.default_connector);
        decision.set_reason("no rule matched");
        decision.set_matched(false);
        return decision;
    }

private:
    [[nodiscard]] core::error::AppError validation_error(std::string message) const
    {
        return core::error::AppErrorBuilder{}(core::error::AppErrorKind::VALIDATION_FAILED, std::move(message));
    }

    [[nodiscard]] std::expected<bool, core::error::AppError> matches(
        std::string_view field,
        std::string_view op,
        std::string_view expected,
        const DecisionContext& context
    ) const
    {
        const std::string actual = field_value(field, context);
        if (field_value_failed(field)) {
            return std::unexpected{validation_error("unsupported decision field")};
        }

        if (op == "eq") {
            return actual == expected;
        }
        if (op == "neq") {
            return actual != expected;
        }
        if (op == "contains") {
            return actual.find(expected) != std::string::npos;
        }
        if (op == "in") {
            for (const auto part: std::views::split(expected, ',')) {
                if (std::string{std::ranges::begin(part), std::ranges::end(part)} == actual) {
                    return true;
                }
            }
            return false;
        }
        if (op == "gt" || op == "gte" || op == "lt" || op == "lte") {
            const auto lhs = to_number(actual);
            const auto rhs = to_number(expected);
            if (!lhs.has_value() || !rhs.has_value()) {
                return std::unexpected{validation_error("numeric comparison on non-numeric value")};
            }
            if (op == "gt") {
                return *lhs > *rhs;
            }
            if (op == "gte") {
                return *lhs >= *rhs;
            }
            if (op == "lt") {
                return *lhs < *rhs;
            }
            return *lhs <= *rhs;
        }
        return std::unexpected{validation_error("unsupported decision operator")};
    }

    [[nodiscard]] bool field_value_failed(std::string_view field) const
    {
        return field != "amount" && field != "currency" && field != "payment_method" &&
               field != "country" && field != "connector";
    }

    [[nodiscard]] std::string field_value(std::string_view field, const DecisionContext& context) const
    {
        if (field == "amount") {
            return context.amount;
        }
        if (field == "currency") {
            return context.currency;
        }
        if (field == "payment_method") {
            return context.payment_method;
        }
        if (field == "country") {
            return context.country;
        }
        if (field == "connector") {
            return context.connector;
        }
        return {};
    }

    [[nodiscard]] std::optional<std::int64_t> to_number(std::string_view text) const
    {
        std::int64_t value{};
        const char* begin = text.data();
        const char* end = text.data() + text.size();
        const auto result = std::from_chars(begin, end, value);
        if (result.ec != std::errc{} || result.ptr != end) {
            return std::nullopt;
        }
        return value;
    }
};

/** @brief Superposition config client backed by ConfigStore config records. */
class SuperpositionClient
{
public:
    explicit SuperpositionClient(ConfigStore store) :
        m_store{std::move(store)}
    {
    }

    [[nodiscard]] std::optional<ConfigRecord> get_config(std::string_view config_namespace, std::string_view key) const
    {
        return m_store.get(config_namespace, key);
    }

    [[nodiscard]] std::vector<ContextRecord> list_contexts() const
    {
        return m_store.list_contexts();
    }

    /** @brief Context override wins over the namespace default; deterministic, no network. */
    [[nodiscard]] std::optional<ConfigRecord>
    resolve(std::string_view config_namespace, std::string_view key, std::string_view context) const
    {
        const std::string scoped_key{std::string{key} + "@" + std::string{context}};
        auto scoped = m_store.get(config_namespace, scoped_key);
        if (scoped.has_value()) {
            return scoped;
        }
        return m_store.get(config_namespace, key);
    }

    [[nodiscard]] std::vector<AuditLogRecord> audit_logs() const
    {
        return m_store.audit_logs();
    }

    void put_config(std::string_view config_namespace, std::string_view key, std::string value, std::string context = {})
    {
        m_store.put(config_namespace, key, std::move(value), std::move(context));
    }

    void add_context(ContextRecord context)
    {
        m_store.add_context(std::move(context));
    }

private:
    ConfigStore m_store{};
};

} // namespace app::services

template<>
struct serde::Serializable<app::services::Decision>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"action", &app::services::Decision::get_action, &app::services::Decision::set_action, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_name", &app::services::Decision::get_connector_name, &app::services::Decision::set_connector_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"reason", &app::services::Decision::get_reason, &app::services::Decision::set_reason, serde::FieldOptions{}>{},
            serde::FieldDesc<"matched", &app::services::Decision::get_matched, &app::services::Decision::set_matched, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_kind", &app::services::Decision::get_error_kind, &app::services::Decision::set_error_kind, serde::FieldOptions{}>{},
        };
    }

    static constexpr std::string_view table_name()
    {
        return "decision";
    }
};

#ifdef CONGELADO_TEST
namespace app::services::tests {
using namespace boost::ut;

suite<"app_services_decision"> decision_suite = []
{
    "comparison on amount routes to a connector"_test = []
    {
        constexpr std::string_view program =
            R"({"rules":[{"if_condition":{"field":"amount","op":"gte","value":"5000"},"action":"route","connector":"stripe"}],"default_action":"allow","default_connector":"local"})";
        constexpr std::string_view high = R"({"amount":"6000","currency":"USD","payment_method":"card","country":"US","connector":""})";
        constexpr std::string_view low = R"({"amount":"100","currency":"USD","payment_method":"card","country":"US","connector":""})";

        const auto routed = DecisionEngine{}.evaluate(program, high);
        expect(routed.get_action() == "route");
        expect(routed.get_connector_name() == "stripe");
        expect(routed.get_matched());
        expect(routed.get_error_kind().empty());

        const auto fallback = DecisionEngine{}.evaluate(program, low);
        expect(fallback.get_action() == "allow");
        expect(fallback.get_connector_name() == "local");
        expect(!fallback.get_matched());
    };

    "string comparisons on currency and country are supported"_test = []
    {
        constexpr std::string_view program =
            R"({"rules":[{"if_condition":{"field":"currency","op":"in","value":"USD,EUR"},"action":"allow","connector":""}],"default_action":"deny"})";
        constexpr std::string_view context = R"({"amount":"1","currency":"EUR","payment_method":"card","country":"DE","connector":""})";
        const auto decision = DecisionEngine{}.evaluate(program, context);
        expect(decision.get_action() == "allow");
        expect(decision.get_error_kind().empty());
    };

    "unknown programs return a typed VALIDATION_FAILED error"_test = []
    {
        constexpr std::string_view bad_field =
            R"({"rules":[{"if_condition":{"field":"colour","op":"eq","value":"red"},"action":"allow","connector":""}],"default_action":"deny"})";
        constexpr std::string_view context = R"({"amount":"1","currency":"USD","payment_method":"card","country":"US","connector":""})";

        const auto checked = DecisionEngine{}.evaluate_checked(bad_field, context);
        expect(!checked.has_value());
        expect(checked.error().get_kind() == core::error::AppErrorKind::VALIDATION_FAILED);

        const auto decision = DecisionEngine{}.evaluate(bad_field, context);
        expect(decision.get_action() == "error");
        expect(decision.get_error_kind() == "VALIDATION_FAILED");
    };
};

suite<"app_services_superposition"> superposition_suite = []
{
    "resolve prefers a context override over the base value"_test = []
    {
        ConfigStore store{};
        store.put("feature", "routing", "base");
        store.put("feature", "routing@US", "us_override");
        SuperpositionClient client{std::move(store)};

        const auto base = client.get_config("feature", "routing");
        expect(base.has_value());
        expect(base->get_value() == "base");
        const auto resolved = client.resolve("feature", "routing", "US");
        expect(resolved.has_value());
        expect(resolved->get_value() == "us_override");
        const auto other = client.resolve("feature", "routing", "DE");
        expect(other.has_value());
        expect(other->get_value() == "base");
    };

    "contexts and audit logs are exposed through the client"_test = []
    {
        ConfigStore store{};
        ContextRecord context{};
        context.set_name("US");
        context.set_value("us");
        context.set_weight(1);
        store.add_context(context);
        store.put("feature", "flag", "on");
        SuperpositionClient client{std::move(store)};

        expect(client.list_contexts().size() == 1);
        expect(client.audit_logs().size() == 1);
        expect(client.audit_logs()[0].get_action() == "put");
    };
};

} // namespace app::services::tests
#endif
