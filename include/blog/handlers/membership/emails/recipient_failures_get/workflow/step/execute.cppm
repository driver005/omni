export module blog_handlers:emails_recipient_failures_get_step;
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

export namespace blog::handlers::emails::recipient_failures_get::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"emails_recipient_failures_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const std::string path{in.get_path()};
        const std::string id = blog::api::emails::id_from_path(path);

        static constexpr std::string_view
            kOptions[]{"limit", "fields", "filter", "order", "page", "include"};
        static constexpr std::string_view kIncludes[]{"member", "email_recipient"};
        std::string error;
        auto q = blog::api::emails::parse_query(path, kOptions, error);
        if (!q) {
            return core::workflow::StepResponse<Output>{{{"status", "400"}, {"body", error}}};
        }

        // input/emails.ts maps the wire include email_recipient -> emailRecipient;
        // the allowlist itself stays snake_case as Ghost declares it.
        for (const auto& include: q->include) {
            const bool allowed =
                std::ranges::find(kIncludes, include) != std::ranges::end(kIncludes) ||
                include == "emailRecipient";
            if (!allowed) {
                return core::workflow::StepResponse<Output>{
                    {{"status", "422"},
                     {"body",
                      blog::api::emails::validation_error("include", "Invalid include value")}}
                };
            }
        }

        const std::string user_filter = q->raw_filter;
        const std::string extra = user_filter.empty() ? std::string{} : ("+(" + user_filter + ")");
        q->raw_filter = "email_id:'" + id + "'" + extra;

        try {
            const auto raw = ctx.store().json_for("email_recipient_failures", {});
            const auto mapped = blog::api::emails::map_failures(raw);
            const auto ran = core::query::run(mapped, *q, 15);
            return core::workflow::StepResponse<Output>{
                {{"status", "200"}, {"body", blog::api::emails::collection("failures", ran)}}
            };
        } catch (const std::exception&) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body",
                  blog::api::emails::bad_request("Restricted fields cannot be used in order.")}}
            };
        }
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::emails::recipient_failures_get::workflow
