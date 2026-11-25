export module app_handlers_three_ds_decision_rule:runtime_common;

import std;
import interfaces;
import core_serde;
import app_context;
import app_dto;
import app_services;

export namespace app::handlers::three_ds_decision_rule {

namespace Dto = app::dto::three_ds_decision_rule;

using Decision = Dto::ThreeDSDecision;
using Request = Dto::ThreeDsDecisionRuleExecuteRequest;
using Value = interfaces::Value;

// ---------------------------------------------------------------------------
// Local mirrors of `euclid::backend::inputs` (crates/euclid/src/backend/inputs.rs:12-72).
// Every field that upstream `ForeignFrom` populates is represented here; fields that
// upstream sets to None are represented as empty optionals with a source comment.
// ---------------------------------------------------------------------------

struct PaymentInput
{
    std::string amount{};
    std::string currency{};
    std::optional<std::string> card_bin{};
    std::optional<std::string> extended_card_bin{};
    std::optional<std::string> business_country{};
};

struct PaymentMethodInput
{
    std::optional<std::string> card_network{};
    std::optional<std::string> card_discovery{};
};

struct CustomerDeviceDataInput
{
    std::optional<std::string> platform{};
    std::optional<std::string> device_type{};
    std::optional<std::string> display_size{};
};

struct IssuerDataInput
{
    std::optional<std::string> name{};
    std::optional<std::string> country{};
};

struct AcquirerDataInput
{
    std::optional<std::string> country{};
    std::optional<double> fraud_rate{};
};

// MandateData all-None: mandate_acceptance_type / mandate_type / payment_type
// (utils.rs:151-155, inputs.rs:12-16).
struct MandateData
{};

struct BackendInput
{
    std::optional<std::unordered_map<std::string, std::string>> metadata{};
    PaymentInput payment{};
    PaymentMethodInput payment_method{};
    std::optional<AcquirerDataInput> acquirer_data{};
    std::optional<CustomerDeviceDataInput> customer_device_data{};
    std::optional<IssuerDataInput> issuer_data{};
    MandateData mandate{};
};

// Program<ThreeDSDecisionRule> evaluation (core/three_ds_decision_rule.rs:55-75).
// `initialized` is the VirInterpreterBackend::with_program step; `decision` is the
// interpreter.execute step, so the two upstream InternalServerError branches stay distinct.
struct EvaluationResult
{
    bool initialized{false};
    std::optional<Decision> decision{};
};

/**
 * @brief Shared input-conversion and DSL-evaluation helpers for the execute route.
 *
 * Pure logic: converts the typed DTO into the upstream BackendInput mirror,
 * evaluates a stored program against it and applies the SCA validations.
 */
class ThreeDsDecisionRuleRuntimeSupport
{
public:
    explicit ThreeDsDecisionRuleRuntimeSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ForeignFrom<PaymentData> for PaymentInput — utils.rs:54-71.
    [[nodiscard]] PaymentInput payment_input_from(const Dto::PaymentData& p) const
    {
        PaymentInput in{};
        in.amount = p.get_amount().get_value();
        in.currency = p.get_currency().get_value();
        // transaction_initiator/authentication_type/capture_method/billing_country/
        // business_label/setup_future_usage/surcharge_amount are None upstream (utils.rs:59-68).
        if (p.get_card_bin()) {
            in.card_bin = *p.get_card_bin();
        }
        if (p.get_extended_card_bin()) {
            in.extended_card_bin = *p.get_extended_card_bin();
        }
        if (p.get_business_country()) {
            in.business_country = p.get_business_country()->get_value();
        }
        return in;
    }

    // ForeignFrom<Option<PaymentMethodMetaData>> for PaymentMethodInput — utils.rs:73-88.
    [[nodiscard]] PaymentMethodInput
    payment_method_input_from(const std::optional<Dto::PaymentMethodMetaData>& pm) const
    {
        PaymentMethodInput in{};
        // payment_method / payment_method_type are None upstream (utils.rs:80-81).
        if (pm) {
            if (pm->get_card_network()) {
                in.card_network = pm->get_card_network()->get_value();
            }
            if (pm->get_card_discovery()) {
                in.card_discovery = pm->get_card_discovery()->get_value();
            }
        }
        return in;
    }

    // ForeignFrom<CustomerDeviceData> for CustomerDeviceDataInput — utils.rs:90-98.
    [[nodiscard]] CustomerDeviceDataInput
    customer_device_input_from(const Dto::CustomerDeviceData& d) const
    {
        CustomerDeviceDataInput in{};
        if (d.get_platform()) {
            in.platform = d.get_platform()->get_value();
        }
        if (d.get_device_type()) {
            in.device_type = d.get_device_type()->get_value();
        }
        if (d.get_display_size()) {
            in.display_size = d.get_display_size()->get_value();
        }
        return in;
    }

    // ForeignFrom<IssuerData> for IssuerDataInput — utils.rs:100-107.
    [[nodiscard]] IssuerDataInput issuer_input_from(const Dto::IssuerData& i) const
    {
        IssuerDataInput in{};
        if (i.get_name()) {
            in.name = *i.get_name();
        }
        if (i.get_country()) {
            in.country = i.get_country()->get_value();
        }
        return in;
    }

    // ForeignFrom<AcquirerData> for AcquirerDataInput — utils.rs:109-116.
    [[nodiscard]] AcquirerDataInput acquirer_input_from(const Dto::AcquirerData& a) const
    {
        AcquirerDataInput in{};
        if (a.get_country()) {
            in.country = a.get_country()->get_value();
        }
        if (a.get_fraud_rate()) {
            in.fraud_rate = *a.get_fraud_rate();
        }
        return in;
    }

    // The upstream flat "routing_parameters" namespace is parsed as
    // FxHashMap<String, String>; any non-string value makes the whole map parse fail
    // (`.ok()` drops it) — utils.rs:121-148.
    [[nodiscard]] std::optional<std::unordered_map<std::string, std::string>>
    parse_string_map(const Value& value) const
    {
        const auto* object = std::get_if<Value::Object>(&value.get());
        if (object == nullptr) {
            return std::nullopt;
        }
        std::unordered_map<std::string, std::string> map{};
        for (const auto& [key, entry]: *object) {
            const auto* text = std::get_if<std::string>(&entry.get());
            if (text == nullptr) {
                return std::nullopt;
            }
            map.insert_or_assign(key, *text);
        }
        return map;
    }

    // ForeignFrom<ThreeDsDecisionRuleExecuteRequest> for BackendInput — utils.rs:118-160.
    [[nodiscard]] BackendInput backend_input_from(const Request& request) const
    {
        BackendInput in{};
        if (request.get_metadata()) {
            const Value& metadata = *request.get_metadata();
            // routing_parameters namespace wins; fall back to the flat root object.
            std::optional<Value> nested_storage{};
            const Value* root = &metadata;
            if (const auto* object = std::get_if<Value::Object>(&metadata.get())) {
                if (const auto nested = object->get("routing_parameters")) {
                    nested_storage = *nested;
                    root = &*nested_storage;
                }
            }
            in.metadata = parse_string_map(*root);
        }
        in.payment = payment_input_from(request.get_payment());
        in.payment_method = payment_method_input_from(request.get_payment_method());
        if (request.get_acquirer()) {
            in.acquirer_data = acquirer_input_from(*request.get_acquirer());
        }
        if (request.get_customer_device()) {
            in.customer_device_data = customer_device_input_from(*request.get_customer_device());
        }
        if (request.get_issuer()) {
            in.issuer_data = issuer_input_from(*request.get_issuer());
        }
        return in;
    }

    // --- typed ThreeDSDecision helpers ---------------------------------------

    [[nodiscard]] std::optional<Decision> decision_from_string(std::string_view value) const
    {
        if (value == "no_three_ds") {
            return Decision::no_three_ds;
        }
        if (value == "challenge_requested") {
            return Decision::challenge_requested;
        }
        if (value == "challenge_preferred") {
            return Decision::challenge_preferred;
        }
        if (value == "no_preference") {
            return Decision::no_preference;
        }
        if (value == "three_ds_exemption_requested_tra") {
            return Decision::three_ds_exemption_requested_tra;
        }
        if (value == "three_ds_exemption_requested_low_value") {
            return Decision::three_ds_exemption_requested_low_value;
        }
        if (value == "issuer_three_ds_exemption_requested") {
            return Decision::issuer_three_ds_exemption_requested;
        }
        return std::nullopt;
    }

    // SCA_MANDATED_COUNTRIES (crates/router/src/consts.rs:343-386).
    [[nodiscard]] bool in_sca_country(const std::optional<std::string>& country) const
    {
        if (!country) {
            return false;
        }
        std::string code = *country;
        for (char& c: code) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
        constexpr std::string_view sca_countries[] = {
            "AT", "BE", "BG", "HR", "CY", "CZ", "DK", "EE", "FI", "FR", "DE", "GR", "HU",
            "IE", "IT", "LV", "LT", "LU", "MT", "NL", "PL", "PT", "RO", "SK", "SI", "ES",
            "SE", "IS", "LI", "NO", "AX", "GF", "GP", "MQ", "YT", "RE", "MF", "GB", "GI"
        };
        for (auto sca: sca_countries) {
            if (code == sca) {
                return true;
            }
        }
        return false;
    }

    // utils.rs:30-52 — apply_sca_validations_to_decision.
    [[nodiscard]] Decision
    apply_sca_validations_to_decision(Decision decision, bool sca_mandate_applies) const
    {
        if (sca_mandate_applies) {
            return decision == Decision::no_three_ds ? Decision::no_preference : decision;
        }
        if (decision == Decision::no_three_ds || decision == Decision::no_preference) {
            return decision;
        }
        return Decision::challenge_requested;
    }

    // utils.rs:8-28 — apply_sca_validations_during_execute.
    [[nodiscard]] Decision
    apply_sca_validations_during_execute(Decision decision, const Request& request) const
    {
        std::optional<std::string> issuer_country{};
        if (request.get_issuer() && request.get_issuer()->get_country()) {
            issuer_country = request.get_issuer()->get_country()->get_value();
        }
        std::optional<std::string> acquirer_country{};
        if (request.get_acquirer() && request.get_acquirer()->get_country()) {
            acquirer_country = request.get_acquirer()->get_country()->get_value();
        }
        const bool issuer_sca = in_sca_country(issuer_country);
        const bool acquirer_sca = in_sca_country(acquirer_country);
        return apply_sca_validations_to_decision(decision, issuer_sca && acquirer_sca);
    }

    // --- local, deterministic substitute for euclid::backend::VirInterpreterBackend ---
    // Reads the upstream `ast::Program<ThreeDSDecisionRule>` wire shape (camelCase:
    // defaultSelection, rules[].connectorSelection/routingOutput, rules[].statements[].
    // condition/nested, comparison.lhs/comparison/value) from the stored algorithm data
    // and evaluates its comparisons against the fully-constructed BackendInput. The Rust
    // Euclid VM is external infrastructure; this in-process evaluator is the documented
    // substitute (see Shared contract requests in the parity report).

    [[nodiscard]] std::optional<std::string> as_string(const Value& value) const
    {
        const auto* text = std::get_if<std::string>(&value.get());
        return text != nullptr ? std::optional<std::string>{*text} : std::nullopt;
    }

    [[nodiscard]] std::optional<std::string>
    field_of(const Value::Object& object, const char* key) const
    {
        const auto field = object.get(key);
        if (!field) {
            return std::nullopt;
        }
        return as_string(*field);
    }

    [[nodiscard]] std::optional<double> as_number(const Value& value) const
    {
        if (const auto* integer = std::get_if<std::int64_t>(&value.get())) {
            return static_cast<double>(*integer);
        }
        if (const auto* number = std::get_if<double>(&value.get())) {
            return *number;
        }
        return std::nullopt;
    }

    // Upstream DirKeyKind serde names (crates/euclid/src/frontend/dir.rs:88-352).
    [[nodiscard]] std::optional<std::string>
    field_value(std::string_view lhs, const BackendInput& in) const
    {
        if (lhs == "amount") {
            return in.payment.amount;
        }
        if (lhs == "currency") {
            return in.payment.currency;
        }
        if (lhs == "card_bin") {
            return in.payment.card_bin;
        }
        if (lhs == "extended_card_bin") {
            return in.payment.extended_card_bin;
        }
        if (lhs == "business_country" || lhs == "country") {
            return in.payment.business_country;
        }
        if (lhs == "card_network") {
            return in.payment_method.card_network;
        }
        if (lhs == "card_discovery") {
            return in.payment_method.card_discovery;
        }
        if (lhs == "issuer_name") {
            return in.issuer_data ? in.issuer_data->name : std::nullopt;
        }
        if (lhs == "issuer_country") {
            return in.issuer_data ? in.issuer_data->country : std::nullopt;
        }
        if (lhs == "customer_device_platform") {
            return in.customer_device_data ? in.customer_device_data->platform : std::nullopt;
        }
        if (lhs == "customer_device_type") {
            return in.customer_device_data ? in.customer_device_data->device_type : std::nullopt;
        }
        if (lhs == "customer_device_display_size") {
            return in.customer_device_data ? in.customer_device_data->display_size : std::nullopt;
        }
        if (lhs == "acquirer_country") {
            return in.acquirer_data ? in.acquirer_data->country : std::nullopt;
        }
        if (lhs == "acquirer_fraud_rate") {
            return in.acquirer_data && in.acquirer_data->fraud_rate
                       ? std::optional<std::string>{std::to_string(*in.acquirer_data->fraud_rate)}
                       : std::nullopt;
        }
        // "metadata" is resolved as a MetadataVariant payload in compare() below.
        return std::nullopt;
    }

    [[nodiscard]] std::optional<bool>
    apply_string_op(std::string_view op, std::string_view actual, std::string_view expected) const
    {
        if (op == "equal" || op == "eq") {
            return actual == expected;
        }
        if (op == "not_equal" || op == "neq" || op == "notEqual") {
            return actual != expected;
        }
        if (op == "less_than" || op == "lt") {
            return actual < expected;
        }
        if (op == "less_than_equal" || op == "lte") {
            return actual <= expected;
        }
        if (op == "greater_than" || op == "gt") {
            return actual > expected;
        }
        if (op == "greater_than_equal" || op == "gte") {
            return actual >= expected;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<bool>
    apply_number_op(std::string_view op, double actual, double expected) const
    {
        if (op == "equal" || op == "eq") {
            return actual == expected;
        }
        if (op == "not_equal" || op == "neq" || op == "notEqual") {
            return actual != expected;
        }
        if (op == "less_than" || op == "lt") {
            return actual < expected;
        }
        if (op == "less_than_equal" || op == "lte") {
            return actual <= expected;
        }
        if (op == "greater_than" || op == "gt") {
            return actual > expected;
        }
        if (op == "greater_than_equal" || op == "gte") {
            return actual >= expected;
        }
        return std::nullopt;
    }

    // One upstream Comparison (frontend/ast.rs:84-99) with its ValueType payload.
    [[nodiscard]] std::optional<bool>
    compare(const Value::Object& comparison, const BackendInput& in) const
    {
        const auto lhs = field_of(comparison, "lhs");
        const auto op = field_of(comparison, "comparison");
        if (!lhs || !op) {
            return std::nullopt;
        }
        const auto value_field = comparison.get("value");
        if (!value_field) {
            return std::nullopt;
        }
        const Value* payload = &*value_field;
        std::string type{"enum_variant"};
        std::optional<Value> inner_storage{};
        if (const auto* tagged = std::get_if<Value::Object>(&payload->get())) {
            const auto declared_type = field_of(*tagged, "type");
            const auto inner = tagged->get("value");
            if (declared_type && inner) {
                type = *declared_type;
                inner_storage = *inner;
                payload = &*inner_storage;
            }
        }

        // MetadataVariant { key, value } (ast.rs:18-23).
        if (type == "metadata_variant") {
            const auto* meta = std::get_if<Value::Object>(&payload->get());
            if (meta == nullptr || !in.metadata) {
                return false;
            }
            const auto key = field_of(*meta, "key");
            const auto expected = field_of(*meta, "value");
            if (!key || !expected) {
                return false;
            }
            const auto it = in.metadata->find(*key);
            return it != in.metadata->end() ? apply_string_op(*op, it->second, *expected) : false;
        }

        const auto actual = field_value(*lhs, in);

        // Number / NumberArray / NumberComparisonArray (ast.rs:26-47).
        if (type == "number" || type == "number_array" || type == "number_comparison_array") {
            if (!actual) {
                return false;
            }
            double actual_number = 0.0;
            try {
                actual_number = std::stod(*actual);
            } catch (...) {
                return false;
            }
            if (type == "number") {
                const auto expected = as_number(*payload);
                return expected ? apply_number_op(*op, actual_number, *expected) : std::nullopt;
            }
            const auto* array = std::get_if<Value::Array>(&payload->get());
            if (array == nullptr) {
                return std::nullopt;
            }
            for (const auto& element: *array) {
                if (const auto number = as_number(element);
                    number && apply_number_op(*op, actual_number, *number).value_or(false)) {
                    return true;
                }
                if (const auto* element_object = std::get_if<Value::Object>(&element.get())) {
                    const auto comparison_type = field_of(*element_object, "comparisonType");
                    const auto number_field = element_object->get("number");
                    if (comparison_type && number_field) {
                        if (const auto number = as_number(*number_field);
                            number && apply_number_op(*comparison_type, actual_number, *number)
                                          .value_or(false)) {
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        // EnumVariant / StrValue and their array forms.
        if (!actual) {
            return false;
        }
        if (type == "enum_variant_array" || type == "str_value_array") {
            const auto* array = std::get_if<Value::Array>(&payload->get());
            if (array == nullptr) {
                return std::nullopt;
            }
            for (const auto& element: *array) {
                if (const auto expected = as_string(element);
                    expected && apply_string_op(*op, *actual, *expected).value_or(false)) {
                    return true;
                }
            }
            return false;
        }
        const auto expected = as_string(*payload);
        return expected ? apply_string_op(*op, *actual, *expected) : std::nullopt;
    }

    // IfStatement: all conditions AND, plus all nested statements AND (ast.rs:118-124).
    [[nodiscard]] bool matches_statement(const Value& statement_node, const BackendInput& in) const
    {
        const auto* statement = std::get_if<Value::Object>(&statement_node.get());
        if (statement == nullptr) {
            return false;
        }
        const auto condition_field = statement->get("condition");
        if (!condition_field) {
            return false;
        }
        const auto* conditions = std::get_if<Value::Array>(&condition_field->get());
        if (conditions == nullptr) {
            return false;
        }
        for (const auto& comparison: *conditions) {
            const auto* object = std::get_if<Value::Object>(&comparison.get());
            if (object == nullptr) {
                return false;
            }
            const auto result = compare(*object, in);
            if (!result || !*result) {
                return false;
            }
        }
        const auto nested_field = statement->get("nested");
        if (nested_field) {
            if (const auto* nested = std::get_if<Value::Array>(&nested_field->get())) {
                for (const auto& child: *nested) {
                    if (!matches_statement(child, in)) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    [[nodiscard]] EvaluationResult
    evaluate_upstream_program(const Value& program, const BackendInput& in) const
    {
        const auto* root = std::get_if<Value::Object>(&program.get());
        if (root == nullptr) {
            return {};
        }
        std::string default_decision{};
        const auto default_field = root->get("defaultSelection");
        if (default_field) {
            if (const auto* selection = std::get_if<Value::Object>(&default_field->get())) {
                default_decision = field_of(*selection, "decision").value_or("");
            }
        }
        const auto rules_field = root->get("rules");
        if (!rules_field || !default_field) {
            return {};
        }
        const auto* rules = std::get_if<Value::Array>(&rules_field->get());
        if (rules == nullptr) {
            return {};
        }
        for (const auto& rule_node: *rules) {
            const auto* rule = std::get_if<Value::Object>(&rule_node.get());
            if (rule == nullptr) {
                continue;
            }
            auto selection_field = rule->get("connectorSelection");
            if (!selection_field) {
                selection_field = rule->get("routingOutput"); // upstream serde alias
            }
            std::string selected{};
            if (selection_field) {
                if (const auto* selection = std::get_if<Value::Object>(&selection_field->get())) {
                    selected = field_of(*selection, "decision").value_or("");
                }
            }
            const auto statements_field = rule->get("statements");
            if (!statements_field || selected.empty()) {
                continue;
            }
            const auto* statements = std::get_if<Value::Array>(&statements_field->get());
            if (statements == nullptr) {
                continue;
            }
            bool matched = true;
            for (const auto& statement: *statements) {
                if (!matches_statement(statement, in)) {
                    matched = false;
                    break;
                }
            }
            if (matched) {
                return {true, decision_from_string(selected)};
            }
        }
        return {true, decision_from_string(default_decision)};
    }

    // Compatibility subset: locally-seeded routing configs use the in-process
    // app_services::DecisionProgram shape (rules with field/op/value conditions). Such a
    // program is mapped onto the same full BackendInput context and evaluated by the local
    // DecisionEngine — the documented substitute for the external Euclid VM. Upstream-shaped
    // programs (defaultSelection/rules/statements) take the faithful path above.
    [[nodiscard]] EvaluationResult decide(const Value& program, const BackendInput& in) const
    {
        const auto* root = std::get_if<Value::Object>(&program.get());
        if (root != nullptr && root->get("defaultSelection") && root->get("rules")) {
            return evaluate_upstream_program(program, in);
        }

        const auto program_json = core_serde::JsonCodec{}.encode(program);
        const auto local_program =
            core_serde::JsonCodec{}.decode<app::services::DecisionProgram>(program_json);
        if (!local_program) {
            return {};
        }
        app::services::DecisionContext context{};
        context.amount = in.payment.amount;
        context.currency = in.payment.currency;
        context.payment_method =
            in.payment_method.card_network.value_or(in.payment_method.card_discovery.value_or(""));
        if (in.payment.business_country) {
            context.country = *in.payment.business_country;
        } else if (in.issuer_data && in.issuer_data->country) {
            context.country = *in.issuer_data->country;
        } else if (in.acquirer_data && in.acquirer_data->country) {
            context.country = *in.acquirer_data->country;
        }
        if (in.issuer_data && in.issuer_data->name) {
            context.connector = *in.issuer_data->name;
        }
        auto evaluated = app::services::DecisionEngine{}.evaluate_program(*local_program, context);
        if (!evaluated) {
            return {true, std::nullopt};
        }
        return {true, decision_from_string(evaluated->get_action())};
    }

private:
    app::context::AppContext& m_context;
};

} // namespace app::handlers::three_ds_decision_rule
