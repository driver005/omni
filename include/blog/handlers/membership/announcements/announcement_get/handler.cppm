export module blog_handlers:announcements_announcement_get_handler;
import :domain_all;
import std;
import interfaces;
import core_workflow;
import :announcements_announcement_get_flow;
import :announcements_announcement_get_step;

export namespace blog::handlers::announcements::announcement_get {

namespace detail {

// Minimal in-process workflow driver. Mirrors memory_backends::LocalWorkflowDriver
// so this module needs only core_workflow (no extra plugin dependency).
class AnnouncementDriver final : public core::workflow::WorkflowDriver {
public:
    [[nodiscard]] std::string_view name() const noexcept override
    {
        return "announcements_local";
    }

    core::workflow::WorkflowExecution run(
        const core::workflow::WorkflowDefinition& definition,
        core::workflow::WorkflowContext& context
    ) override {
        core::workflow::WorkflowExecution execution;
        execution.id = std::format("{}-1", definition.name);
        execution.transaction_id = execution.id;
        execution.run_id = "1";
        execution.workflow_name = definition.name;
        execution.state = core::workflow::WorkflowState::INVOKING;

        context.set_workflow_name(definition.name);
        context.set_transaction_id(execution.transaction_id);
        context.set_run_id(execution.run_id);

        for (const auto& step : definition.steps) {
            context.set_step_name(step.name);
            context.set_attempt(1);
            auto outcome = step.invoke(context);
            execution.step_results.insert_or_assign(step.name, outcome);
            if (outcome.signal == core::workflow::StepSignal::PERMANENT_FAILURE) {
                execution.errors.push_back(
                    outcome.message.empty() ? "step failed: " + std::string{step.name} : outcome.message
                );
                execution.state = core::workflow::WorkflowState::FAILED;
                return execution;
            }
            if (outcome.signal == core::workflow::StepSignal::SKIP) {
                continue;
            }
            if (outcome.typed_output.has_value()) {
                context.set_step_output(step.name, outcome.typed_output);
            }
        }

        execution.state = core::workflow::WorkflowState::DONE;
        return execution;
    }

    bool set_step_success(
        core::workflow::WorkflowExecution& execution,
        std::string_view step_name,
        core::workflow::WorkflowValue output
    ) override {
        auto* outcome = const_cast<core::workflow::StepOutcome*>(execution.result_of(step_name));
        if (outcome == nullptr) {
            return false;
        }
        outcome->output = std::move(output);
        outcome->signal = core::workflow::StepSignal::NONE;
        return true;
    }

    bool set_step_failure(
        core::workflow::WorkflowExecution& execution,
        std::string_view step_name,
        std::string message
    ) override {
        if (execution.result_of(step_name) == nullptr) {
            return false;
        }
        execution.errors.push_back(std::move(message));
        execution.state = core::workflow::WorkflowState::FAILED;
        return true;
    }
};

} // namespace detail

class AnnouncementGetHandler {
public:
    AnnouncementGetHandler(blog::BlogContext& ctx) noexcept {
        workflow::ExecuteStep::s_ctx = &ctx;
        m_workflow.use_backend(std::make_unique<detail::AnnouncementDriver>());
        m_workflow.register_workflow(workflow::MainFlow{}.build());
    }

    void handle(interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
        const auto ex = m_workflow.run("announcements_announcement_get", req);
        const auto* r = ex.result_of("announcements_announcement_get_execute");
        const auto payload = r && r->typed_output.has_value()
            ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
            : std::map<std::string, std::string>{{"status", "200"}, {"body", "{}"}};
        const int status = payload.contains("status") ? std::stoi(payload.at("status")) : 200;
        const auto body = payload.contains("body") ? payload.at("body") : std::string{"{}"};

        std::vector<std::byte> bytes;
        for (char c : body) {
            bytes.push_back(static_cast<std::byte>(c));
        }

        // Ghost headers.cacheInvalidate:false (announcements.js:42-44) is a
        // no-header instruction: headers.get omits X-Cache-Invalidate entirely
        // (headers.ts:135-144). The members app installs cacheControl('private')
        // (members/app.js:27, announcement/routes.js:13), so write the full
        // private/no-store directive string (cache-control.ts:16-19).
        res.set_header(
            interfaces::io::types::Token::CACHE_CONTROL,
            "no-cache, private, no-store, must-revalidate, max-stale=0, post-check=0, pre-check=0"
        );
        res.set_status(static_cast<interfaces::io::types::Status>(status));
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};

} // namespace blog::handlers::announcements::announcement_get
