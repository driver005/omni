export module blog_handlers:automations_read_by_id_step;
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

export namespace blog::handlers::automations::read_by_id::workflow {

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
    static constexpr std::string_view name{"automations_read_by_id_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const std::string id = blog::api::automations::automation_id(in.get_path());
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body", blog::api::automations::ghost_error("BadRequestError", "id is required")}}
            };
        }
        const auto record = find_record(ctx, id);
        if (!record) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body",
                  blog::api::automations::ghost_error("NotFoundError", "Automation not found.")}}
            };
        }
        return core::workflow::StepResponse<Output>{
            {{"status", "200"},
             {"body",
              local_mapping::wrap(
                  "automations",
                  blog::api::automations::build_automation(ctx, *record)
              )}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    [[nodiscard]] static std::optional<std::string>
    find_record(blog::BlogContext& ctx, std::string_view id)
    {
        const auto stored = ctx.store().resources().get("automations", id);
        if (!stored.empty()) {
            return stored;
        }
        for (const auto& record: blog::api::parse_array(
                 ctx.store().json_for("automations", {{"filter[id]", std::string{id}}})
             )) {
            const auto it = record.find("id");
            if (it != record.end() && it->second == id) {
                return blog::api::record_to_json(record);
            }
        }
        return std::nullopt;
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::automations::read_by_id::workflow
