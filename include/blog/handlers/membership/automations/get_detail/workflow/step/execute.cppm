export module blog_handlers:automations_get_detail_step;
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
import :automations_operations;

export namespace blog::handlers::automations::get_detail::workflow {

namespace local_mapping {
    [[nodiscard]] inline std::string
    wrap(std::string_view resource, std::string_view rows, std::string_view meta = {})
    {
        return "{\"" + std::string{resource} + "\":" + std::string{rows} +
               (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
    }

    [[nodiscard]] inline std::string clean(std::string_view json, std::string_view)
    {
        return std::string{json};
    }

    [[nodiscard]] inline std::string pagination(
        std::size_t total,
        std::size_t limit,
        std::size_t page,
        std::size_t pages,
        std::string_view prev,
        std::string_view next
    )
    {
        return std::format(
            R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
            total,
            limit,
            page,
            pages,
            prev.empty() ? std::string{"null"} : std::string{prev},
            next.empty() ? std::string{"null"} : std::string{next}
        );
    }
} // namespace local_mapping

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"automations_get_detail_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        const auto path = std::string{in.get_path()};
        const std::string aid = blog::api::automations::automation_id(path);
        const std::string runId = blog::api::automations::suffix_id(path, "/runs/");
        if (aid.empty() || runId.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body", blog::api::ApiError{}.validation("id and run_id are required", "", "id")}}
            };
        }
        auto& store = s_ctx->store();
        if (!blog::api::automations::automation_exists(*s_ctx, aid)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body",
                  blog::api::automations::ghost_error("NotFoundError", "Automation not found.")}}
            };
        }
        const auto steps =
            store.json_for("automation_run_steps", {{"filter[automation_run_id]", runId}});
        if (steps == "[]") {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body", blog::api::automations::ghost_error("NotFoundError", "Run not found.")}}
            };
        }
        return core::workflow::StepResponse<Output>{
            {{"status", "200"}, {"body", local_mapping::wrap("data", steps)}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::automations::get_detail::workflow
