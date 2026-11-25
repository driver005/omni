export module blog_handlers:automations_automations_poll_put_step;
import :domain_all;
import :automations_dto_automation_email_preview;
import :automations_dto_automation_email_preview_input;
import :automations_dto_automation_email_preview_response;
import :automations_dto_automation_email_test_input;
import :automations_dto_links;
import :automations_dto_stats_empty_response;
import std;
import core_workflow;
import interfaces;
import :outbox_shim;
import :automations_operations;

export namespace blog::handlers::automations::automations_poll_put::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"automations_automations_poll_put_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input&, core::workflow::WorkflowContext&) const
    {
        // Ghost requestPoll() dispatches StartAutomationsPollEvent; no model write and
        // no query/body input. The outbox is the observable stand-in for the event.
        if (s_ops != nullptr) {
            s_ops->request_poll();
        } else {
            ensure_adapter();
            auto& outbox = blog::handlers::outbox::global_outbox();
            outbox.emit(blog::handlers::outbox::Event{"StartAutomationsPollEvent", "{}", ""});
            [[maybe_unused]] const auto drained = outbox.drain();
        }
        return core::workflow::StepResponse<Output>{""};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
    static blog::api::automations::AutomationsOperations* s_ops;

private:
    using Event = blog::handlers::outbox::Event;

    static std::vector<Event>& log()
    {
        static std::vector<Event> v;
        return v;
    }

    class Adapter final : public blog::handlers::outbox::Adapter
    {
    public:
        void dispatch(const Event& event) override
        {
            if (event.type == "StartAutomationsPollEvent") {
                log().push_back(event);
            }
        }
    };

    static void ensure_adapter()
    {
        static const bool registered = []
        {
            blog::handlers::outbox::register_adapter(std::make_shared<Adapter>());
            return true;
        }();
        static_cast<void>(registered);
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::automations::AutomationsOperations* ExecuteStep::s_ops{nullptr};
} // namespace blog::handlers::automations::automations_poll_put::workflow
