export module memory_backends:workflow;

import std;
import core_workflow;

export namespace core::workflow {

/**
 * @brief In-process driver: depth-ordered invoke, reverse compensation,
 * per-step retries and permanent-failure signals — the core Medusa behavior
 * without a queue or checkpoint store.
 */
class LocalWorkflowDriver final : public WorkflowDriver
{
public:
    [[nodiscard]] std::string_view name() const noexcept override
    {
        return "local";
    }

    WorkflowExecution run(const WorkflowDefinition& definition, WorkflowContext& context) override
    {
        WorkflowExecution execution;
        execution.id = std::format("{}-{}", definition.name, m_next_id++);
        execution.transaction_id = execution.id;
        execution.run_id = "1";
        execution.workflow_name = definition.name;
        execution.state = WorkflowState::INVOKING;

        context.set_workflow_name(definition.name);
        context.set_transaction_id(execution.transaction_id);
        context.set_run_id(execution.run_id);

        std::vector<std::string> completed{};
        for (const StepDescriptor& step: definition.steps) {
            StepOutcome outcome = invoke_with_retries(step, context);
            execution.step_results.insert_or_assign(step.name, outcome);

            if (outcome.signal == StepSignal::PERMANENT_FAILURE) {
                execution.errors.push_back(
                    outcome.message.empty() ? "step failed: " + step.name : outcome.message);
                execution.state = WorkflowState::FAILED;
                compensate(completed, definition, context, execution);
                execution.state = WorkflowState::REVERTED;
                return execution;
            }

            if (outcome.signal == StepSignal::SKIP) {
                continue;
            }

            if (outcome.typed_output.has_value()) {
                context.set_step_output(step.name, outcome.typed_output);
            }
            completed.push_back(step.name);
        }

        fire_hooks(definition, context);
        execution.state = WorkflowState::DONE;
        return execution;
    }

    bool set_step_success(
        WorkflowExecution& execution,
        std::string_view step_name,
        WorkflowValue output
    ) override
    {
        auto* outcome = const_cast<StepOutcome*>(execution.result_of(step_name));
        if (outcome == nullptr) {
            return false;
        }
        outcome->output = std::move(output);
        outcome->signal = StepSignal::NONE;
        return true;
    }

    bool set_step_failure(
        WorkflowExecution& execution,
        std::string_view step_name,
        std::string message
    ) override
    {
        if (execution.result_of(step_name) == nullptr) {
            return false;
        }
        execution.errors.push_back(std::move(message));
        execution.state = WorkflowState::FAILED;
        return true;
    }

private:
    [[nodiscard]] StepOutcome invoke_with_retries(const StepDescriptor& step, WorkflowContext& context)
    {
        const int max_attempts = step.options.max_retries() + 1;
        StepOutcome outcome;
        for (int attempt = 1; attempt <= max_attempts; ++attempt) {
            context.set_step_name(step.name);
            context.set_attempt(attempt);
            try {
                outcome = step.invoke(context);
            } catch (const std::exception& error) {
                outcome = StepOutcome{};
                outcome.signal = StepSignal::PERMANENT_FAILURE;
                outcome.message = error.what();
            }
            if (outcome.signal != StepSignal::PERMANENT_FAILURE) {
                return outcome;
            }
            if (!step.options.continue_on_permanent_failure()) {
                return outcome;
            }
        }
        return outcome;
    }

    void compensate(
        const std::vector<std::string>& completed,
        const WorkflowDefinition& definition,
        WorkflowContext& context,
        WorkflowExecution& execution
    )
    {
        for (auto it = completed.rbegin(); it != completed.rend(); ++it) {
            const StepDescriptor* descriptor = definition.find_step(*it);
            const StepOutcome* outcome = execution.result_of(*it);
            if (descriptor == nullptr || outcome == nullptr || descriptor->compensate == nullptr ||
                descriptor->options.no_compensation()) {
                continue;
            }
            context.set_step_name(*it);
            try {
                descriptor->compensate(outcome->compensate_input, context);
            } catch (const std::exception& error) {
                execution.errors.push_back(std::format("compensation failed for {}: {}", *it, error.what()));
                execution.state = WorkflowState::FAILED;
                return;
            }
        }
    }

    void fire_hooks(const WorkflowDefinition& definition, WorkflowContext& context)
    {
        for (const HookDescriptor& hook: definition.hooks) {
            if (hook.handler) {
                hook.handler(hook.data, context);
            }
        }
    }

    std::uint64_t m_next_id{1};
};

} // namespace core::workflow
