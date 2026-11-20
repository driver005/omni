export module blog_handlers:slack_slack_test_post_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import :outbox_shim;

export namespace blog::handlers::slack::slack_test_post::workflow {

class ExecuteStep {
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"slack_slack_test_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input&, core::workflow::WorkflowContext&) const
    {
        // Ghost sendTest is `events.emit('slack.test')` (slack.js:12-14): no request
        // input, no model write. Emit the same event through the registered
        // blog_handlers:outbox_shim primitive (CONTRACTS.md §4) instead of a
        // ctx.store().resources().create("outbox", ...) stand-in. The payload
        // carries the test-ping text; a real Slack webhook adapter (settings,
        // icon, HTTP POST) belongs in core_adapters and is not wired here.
        blog::handlers::outbox::global_outbox().emit(
            blog::handlers::outbox::Event{
                "slack.test",
                R"({"text":"Heya! This is a test notification from your Ghost blog :smile:. Seems to work fine!","unfurl_links":true})",
                ""});

        // Ghost output serializer sets frame.response = undefined, then
        // res.json(result || {}) -> `{}`. No envelope, no body from query().
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", "{}"}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    // Kept for the handler ctor wiring / future adapters; the outbox event is
    // emitted through the registered process-wide outbox, not the context store.
    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::slack::slack_test_post::workflow
