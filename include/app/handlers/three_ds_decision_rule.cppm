module;
export module app_handlers:three_ds_decision_rule;

import std;
import interfaces;
import core_router;
import core_serde;
import core_error;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import app_http;
import app_metrics;
import app_services;

export namespace app::handlers::three_ds_decision_rule {

namespace Dto = app::dto::three_ds_decision_rule;

namespace detail {

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
{
};

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

// ForeignFrom<PaymentData> for PaymentInput — utils.rs:54-71.
[[nodiscard]] PaymentInput payment_input_from(const Dto::PaymentData& p)
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
payment_method_input_from(const std::optional<Dto::PaymentMethodMetaData>& pm)
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
[[nodiscard]] CustomerDeviceDataInput customer_device_input_from(const Dto::CustomerDeviceData& d)
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
[[nodiscard]] IssuerDataInput issuer_input_from(const Dto::IssuerData& i)
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
[[nodiscard]] AcquirerDataInput acquirer_input_from(const Dto::AcquirerData& a)
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
parse_string_map(const Value& value)
{
    const auto* object = std::get_if<Value::Object>(&value.get());
    if (object == nullptr) {
        return std::nullopt;
    }
    std::unordered_map<std::string, std::string> map{};
    for (const auto& [key, entry] : *object) {
        const auto* text = std::get_if<std::string>(&entry.get());
        if (text == nullptr) {
            return std::nullopt;
        }
        map.insert_or_assign(key, *text);
    }
    return map;
}

// ForeignFrom<ThreeDsDecisionRuleExecuteRequest> for BackendInput — utils.rs:118-160.
[[nodiscard]] BackendInput backend_input_from(const Request& request)
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

// --- typed ThreeDSDecision helpers -----------------------------------------

[[nodiscard]] std::optional<Decision> decision_from_string(std::string_view value)
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
[[nodiscard]] bool in_sca_country(const std::optional<std::string>& country)
{
    if (!country) {
        return false;
    }
    std::string code = *country;
    for (char& c : code) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    constexpr std::string_view sca_countries[] = {
        "AT", "BE", "BG", "HR", "CY", "CZ", "DK", "EE", "FI", "FR", "DE", "GR", "HU",
        "IE", "IT", "LV", "LT", "LU", "MT", "NL", "PL", "PT", "RO", "SK", "SI", "ES",
        "SE", "IS", "LI", "NO", "AX", "GF", "GP", "MQ", "YT", "RE", "MF", "GB", "GI"
    };
    for (auto sca : sca_countries) {
        if (code == sca) {
            return true;
        }
    }
    return false;
}

// utils.rs:30-52 — apply_sca_validations_to_decision.
[[nodiscard]] Decision
apply_sca_validations_to_decision(Decision decision, bool sca_mandate_applies)
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
[[nodiscard]] Decision apply_sca_validations_during_execute(Decision decision, const Request& request)
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

[[nodiscard]] std::optional<std::string> as_string(const Value& value)
{
    const auto* text = std::get_if<std::string>(&value.get());
    return text != nullptr ? std::optional<std::string>{*text} : std::nullopt;
}

[[nodiscard]] std::optional<std::string>
field_of(const Value::Object& object, const char* key)
{
    const auto field = object.get(key);
    if (!field) {
        return std::nullopt;
    }
    return as_string(*field);
}

[[nodiscard]] std::optional<double> as_number(const Value& value)
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
[[nodiscard]] std::optional<std::string> field_value(std::string_view lhs, const BackendInput& in)
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
apply_string_op(std::string_view op, std::string_view actual, std::string_view expected)
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

[[nodiscard]] std::optional<bool> apply_number_op(std::string_view op, double actual, double expected)
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
[[nodiscard]] std::optional<bool> compare(const Value::Object& comparison, const BackendInput& in)
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
        for (const auto& element : *array) {
            if (const auto number = as_number(element); number &&
                                                        apply_number_op(*op, actual_number, *number).value_or(false)) {
                return true;
            }
            if (const auto* comparison = std::get_if<Value::Object>(&element.get())) {
                const auto comparison_type = field_of(*comparison, "comparisonType");
                const auto number_field = comparison->get("number");
                if (comparison_type && number_field) {
                    if (const auto number = as_number(*number_field);
                        number && apply_number_op(*comparison_type, actual_number, *number).value_or(false)) {
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
        for (const auto& element : *array) {
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
[[nodiscard]] bool matches_statement(const Value& statement_node, const BackendInput& in)
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
    for (const auto& comparison : *conditions) {
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
            for (const auto& child : *nested) {
                if (!matches_statement(child, in)) {
                    return false;
                }
            }
        }
    }
    return true;
}

// Program<ThreeDSDecisionRule> evaluation (core/three_ds_decision_rule.rs:55-75).
// `initialized` is the VirInterpreterBackend::with_program step; `decision` is the
// interpreter.execute step, so the two upstream InternalServerError branches stay distinct.
struct EvaluationResult
{
    bool initialized{false};
    std::optional<Decision> decision{};
};

[[nodiscard]] EvaluationResult
evaluate_upstream_program(const Value& program, const BackendInput& in)
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
    for (const auto& rule_node : *rules) {
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
        for (const auto& statement : *statements) {
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
[[nodiscard]] EvaluationResult decide(const Value& program, const BackendInput& in)
{
    const auto* root = std::get_if<Value::Object>(&program.get());
    if (root != nullptr && root->get("defaultSelection") && root->get("rules")) {
        return evaluate_upstream_program(program, in);
    }

    const auto program_json = core_serde::JsonCodec{}.encode(program);
    const auto local_program = core_serde::JsonCodec{}.decode<app::services::DecisionProgram>(program_json);
    if (!local_program) {
        return {};
    }
    app::services::DecisionContext context{};
    context.amount = in.payment.amount;
    context.currency = in.payment.currency;
    context.payment_method = in.payment_method.card_network.value_or(
        in.payment_method.card_discovery.value_or("")
    );
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

} // namespace detail

class ThreeDsDecisionRuleRouteHandlers
{
public:
    void handle_execute_decision_rule(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream route uses HeaderAuth(ApiKeyAuth { allow_connected_scope_operation:
        // false, allow_platform_self_operation: false }). The ApiKey scheme is resolved
        // from the auditable RouteAuthRegistry. For API_KEY the two scope flags do not
        // affect authorization (AppAuthorizer consults them only for
        // HEADER_MERCHANT/HEADER_ORG); the registry row is recorded as a shared-file
        // follow-up so its evidence matches upstream exactly.
        const auto rule = m_context->auth_rules().rule_for("execute_decision_rule");
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto auth = m_context->authorizer().authorize_route(auth_context, rule);
        if (auth) {
            send_error(res, send, *auth);
            return;
        }

        // Flow::ThreeDsDecisionRuleExecute (routes/three_ds_decision_rule.rs:10,17).
        m_context->metrics().record_flow("ThreeDsDecisionRuleExecute");
        m_context->metrics().counter("THREE_DS_DECISION_RULE_EXECUTED").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = detail::Request;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream passes `auth.platform.get_processor().get_account().get_id()` as the
        // processor merchant id (core:29-34). Locally the authenticated AuthContext is
        // the single identity source used by authorization; the routing row is looked
        // up by algorithm id and checked against that merchant id.
        const auto& processor_merchant_id = auth_context.merchant_id();
        auto response = execute_three_ds_decision_rule(*parsed, processor_merchant_id);
        if (!response) {
            send_error(res, send, response.error());
            return;
        }

        auto text = serde::Ser::encode_json(*response);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/three_ds_decision"}
            .add_route(
                Route{"/execute"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_execute_decision_rule(req, res, std::move(send));
                    })
                    .summary("Hyperswitch execute_decision_rule")
            );
    }

    explicit ThreeDsDecisionRuleRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    // Port of core::three_ds_decision_rule::get_three_ds_decision_rule_output
    // (core/three_ds_decision_rule.rs:41-85).
    [[nodiscard]] std::expected<detail::Decision, core::error::AppError>
    get_three_ds_decision_rule_output(
        const detail::Request& request,
        std::string_view processor_merchant_id
    ) const
    {
        // find_routing_algorithm_by_algorithm_id_processor_merchant_id(routing_id,
        // processor_merchant_id) -> ResourceIdNotFound (core.rs:48-54).
        const auto routing_id = request.get_routing_id().get_value();
        const auto config = m_context->repository().find_routing_config_by_string(routing_id);
        if (!config || (!processor_merchant_id.empty() && !config->get_merchant_id().empty() &&
                        config->get_merchant_id() != processor_merchant_id)) {
            return std::unexpected{make_error(core::error::AppErrorKind::NOT_FOUND, "routing algorithm not found")};
        }

        // algorithm_data.parse_value("Algorithm") -> InternalServerError
        // "Error parsing program from three_ds_decision rule algorithm" (core.rs:55-64).
        auto envelope = serde::Ser::decode_generic("application/json", config->get_config_json());
        if (!envelope) {
            return std::unexpected{make_error(core::error::AppErrorKind::INTERNAL_ERROR, "Error parsing program from three_ds_decision rule algorithm")};
        }
        std::optional<detail::Value> program_storage{};
        const detail::Value* program_node = &*envelope;
        if (const auto* object = std::get_if<detail::Value::Object>(&envelope->get())) {
            if (const auto data = object->get("data")) {
                program_storage = *data; // upstream Algorithm { data: Value }
                program_node = &*program_storage;
            }
        }

        // dsl_inputs::BackendInput::foreign_from(request) — utils.rs:118-160.
        const auto backend_input = detail::backend_input_from(request);

        // VirInterpreterBackend::with_program(program) -> InternalServerError
        // "Error initializing DSL interpreter backend" (core.rs:67-70).
        const auto evaluated = detail::decide(*program_node, backend_input);
        if (!evaluated.initialized) {
            return std::unexpected{make_error(core::error::AppErrorKind::INTERNAL_ERROR, "Error initializing DSL interpreter backend")};
        }

        // interpreter.execute(backend_input) -> InternalServerError
        // "Error executing 3DS decision rule" (core.rs:71-75).
        if (!evaluated.decision) {
            return std::unexpected{make_error(core::error::AppErrorKind::INTERNAL_ERROR, "Error executing 3DS decision rule")};
        }

        // apply_sca_validations_during_execute (core.rs:81-84, utils.rs:8-28).
        return detail::apply_sca_validations_during_execute(*evaluated.decision, request);
    }

    // Port of core::three_ds_decision_rule::execute_three_ds_decision_rule
    // (core/three_ds_decision_rule.rs:23-39).
    [[nodiscard]] std::expected<Dto::ThreeDsDecisionRuleExecuteResponse, core::error::AppError>
    execute_three_ds_decision_rule(
        const detail::Request& request,
        std::string_view processor_merchant_id
    ) const
    {
        auto decision = get_three_ds_decision_rule_output(request, processor_merchant_id);
        if (!decision) {
            return std::unexpected{decision.error()};
        }
        Dto::ThreeDsDecisionRuleExecuteResponse response{};
        response.set_decision(*decision);
        return response;
    }

    [[nodiscard]] static core::error::AppError make_error(
        core::error::AppErrorKind kind,
        std::string message
    )
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::three_ds_decision_rule
