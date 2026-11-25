export module blog_handlers:gifts_gifts_flush_reminders_put_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import :outbox_shim;

export namespace blog::handlers::gifts::gifts_flush_reminders_put::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"gifts_gifts_flush_reminders_put_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke([[maybe_unused]] const Input& in, core::workflow::WorkflowContext&) const
    {
        // Ghost query(): domainEvents.dispatch(StartGiftReminderFlushEvent.create())
        // (gifts.js:20-22), consumed by the gifts service subscriber that runs
        // giftService.processReminders() (services/gifts/index.ts:165-180). Emit the
        // same event through the registered blog_handlers:outbox_shim primitive
        // (CONTRACTS.md §4) instead of a store marker: no request input, no store
        // write, no direct SQL, no new global state. The real reminder-email pipeline
        // is a subscriber outside this controller and has no allowed C++ primitive here.
        blog::handlers::outbox::global_outbox().emit(
            blog::handlers::outbox::Event{
                .type = "StartGiftReminderFlushEvent",
                .payload = std::format(R"({{"data":null,"timestamp":"{}"}})", now_iso()),
                .aggregate_id = ""
            }
        );
        // Ghost query() returns undefined; 204 carries no body.
        return core::workflow::StepResponse<Output>{std::string{}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    [[nodiscard]] static std::string now_iso()
    {
        return std::format(
            "{:%Y-%m-%dT%H:%M:%SZ}",
            std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now())
        );
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::gifts::gifts_flush_reminders_put::workflow
