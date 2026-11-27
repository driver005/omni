export module app_payment:three_ds_decision_rule_runtime_execute_decision_rule;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_services;

import :three_ds_decision_rule_input;
import :three_ds_decision_rule_result;
import :three_ds_decision_rule_runtime_common;

export namespace app::handlers::three_ds_decision_rule {

class ExecuteDecisionRuleRuntime
{
public:
    explicit ExecuteDecisionRuleRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Port of core::three_ds_decision_rule::execute_three_ds_decision_rule
    // (core/three_ds_decision_rule.rs:23-39).
    [[nodiscard]] std::expected<
        ::app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteResponse,
        core::error::AppError>
    execute_decision_rule(const ExecuteDecisionRuleInput& input) const
    {
        auto decision =
            get_three_ds_decision_rule_output(input.request, input.processor_merchant_id);
        if (!decision) {
            return std::unexpected{decision.error()};
        }
        Dto::ThreeDsDecisionRuleExecuteResponse response{};
        response.set_decision(*decision);
        return ::app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteResponse{response};
    }

private:
    // Port of core::three_ds_decision_rule::get_three_ds_decision_rule_output
    // (core/three_ds_decision_rule.rs:41-85).
    [[nodiscard]] std::expected<Decision, core::error::AppError> get_three_ds_decision_rule_output(
        const Request& request,
        std::string_view processor_merchant_id
    ) const
    {
        // find_routing_algorithm_by_algorithm_id_processor_merchant_id(routing_id,
        // processor_merchant_id) -> ResourceIdNotFound (core.rs:48-54).
        const auto routing_id = request.get_routing_id().get_value();
        const auto config = m_context.repository().find_routing_config_by_string(routing_id);
        if (!config || (!processor_merchant_id.empty() && !config->get_merchant_id().empty() &&
                        config->get_merchant_id() != processor_merchant_id)) {
            return std::unexpected{
                make_error(core::error::AppErrorKind::NOT_FOUND, "routing algorithm not found")
            };
        }

        // algorithm_data.parse_value("Algorithm") -> InternalServerError
        // "Error parsing program from three_ds_decision rule algorithm" (core.rs:55-64).
        auto envelope = serde::Ser::decode_generic("application/json", config->get_config_json());
        if (!envelope) {
            return std::unexpected{make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "Error parsing program from three_ds_decision rule algorithm"
            )};
        }
        std::optional<Value> program_storage{};
        const Value* program_node = &*envelope;
        if (const auto* object = std::get_if<Value::Object>(&envelope->get())) {
            if (const auto data = object->get("data")) {
                program_storage = *data; // upstream Algorithm { data: Value }
                program_node = &*program_storage;
            }
        }

        // dsl_inputs::BackendInput::foreign_from(request) — utils.rs:118-160.
        const auto backend_input = m_support.backend_input_from(request);

        // VirInterpreterBackend::with_program(program) -> InternalServerError
        // "Error initializing DSL interpreter backend" (core.rs:67-70).
        const auto evaluated = m_support.decide(*program_node, backend_input);
        if (!evaluated.initialized) {
            return std::unexpected{make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "Error initializing DSL interpreter backend"
            )};
        }

        // interpreter.execute(backend_input) -> InternalServerError
        // "Error executing 3DS decision rule" (core.rs:71-75).
        if (!evaluated.decision) {
            return std::unexpected{make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "Error executing 3DS decision rule"
            )};
        }

        // apply_sca_validations_during_execute (core.rs:81-84, utils.rs:8-28).
        return m_support.apply_sca_validations_during_execute(*evaluated.decision, request);
    }

    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    ::app::context::AppContext& m_context;
    ThreeDsDecisionRuleRuntimeSupport m_support;
};

} // namespace app::handlers::three_ds_decision_rule
