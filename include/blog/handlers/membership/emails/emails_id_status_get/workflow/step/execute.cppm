export module blog_handlers:emails_emails_id_status_get_step;
import :domain_all;
import :emails_dto_analytics_job;
import :emails_dto_analytics_status;
import :emails_dto_analytics_status_input;
import :emails_dto_scheduled_analytics_job;
import :emails_model_member;
import :emails_model_wire_fixtures;
import :emails_operations;
import std;
import core_query;
import core_serde;
import core_workflow;
import interfaces;

export namespace blog::handlers::emails::emails_id_status_get::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"emails_emails_id_status_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const std::string id = blog::api::emails::id_from_path(in.get_path());

        // Ghost emailService.controller.getSendingStatus -> statusFor(id): a missing
        // email row yields NotFoundError('Email not found.').
        const auto email = blog::api::emails::find_email(ctx.store(), id);
        if (email.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"}, {"body", blog::api::emails::not_found("Email not found.")}}
            };
        }

        // SendingStatusService#batchesFor / recipient count, expressed with the
        // context-local ResourceStore. No fixture row is fabricated when empty.
        const auto batches = ctx.store().json_for("email_batches", {{"filter[email_id]", id}});
        const auto recipients =
            ctx.store().json_for("email_recipients", {{"filter[email_id]", id}});

        // buildSendingStatus + toEmailStatusesResponse wire shape.
        const auto body = blog::api::emails::sending_status_body(id, email, batches, recipients);
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::emails::emails_id_status_get::workflow
