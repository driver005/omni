export module blog_handlers:automations_automations_id_performance_stats_get_step;
import :domain_all;
import :automations_dto_automation_email_preview;
import :automations_dto_automation_email_preview_input;
import :automations_dto_automation_email_preview_response;
import :automations_dto_automation_email_test_input;
import :automations_dto_links;
import :automations_dto_stats_empty_response;
import std;
import core_workflow;
import core_serde;
import core_query;
import interfaces;
import :automations_operations;

export namespace blog::handlers::automations::automations_id_performance_stats_get::workflow {

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
    static constexpr std::string_view name{
        "automations_automations_id_performance_stats_get_execute"
    };

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        const std::string id = blog::api::automations::automation_id(in.get_path());
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body", blog::api::automations::ghost_error("BadRequestError", "id is required")}}
            };
        }
        // Ghost options date_from/date_to/timezone flow through core::query's allowlist
        // and Ghost option validation before the IANA/ISO window checks.
        static constexpr std::string_view kOptions[]{"date_from", "date_to", "timezone"};
        const auto q = core::query::parse(in.get_path(), kOptions);
        const auto from = option(q, "date_from");
        const auto to = option(q, "date_to");
        const auto zone = q.options.contains("timezone") && !q.options.at("timezone").empty()
                              ? q.options.at("timezone")
                              : std::string{"UTC"};
        if ((!from.empty() && !valid_date(from)) || (!to.empty() && !valid_date(to)) ||
            (!to.empty() && from.empty()) || (!to.empty() && from > to)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}.validation(
                      "Validation error, cannot read performance stats.",
                      "Invalid date window",
                      "date_from"
                  )}}
            };
        }

        auto& ctx = *s_ctx;
        if (!blog::api::automations::automation_exists(ctx, id)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body",
                  blog::api::automations::ghost_error("NotFoundError", "Automation not found.")}}
            };
        }
        const auto rows = ctx.store().json_for("automation_runs", {{"filter[automation_id]", id}});
        std::size_t total = 0, running = 0, completed = 0, exited = 0;
        std::string entries = "[";
        bool first = true;
        for (const auto& item: as_array_or_empty(rows)) {
            const auto* object = as_object(&item);
            if (object == nullptr) {
                continue;
            }
            ++total;
            const auto status = field_string(object, "status");
            if (status == "in_progress") {
                ++running;
            } else if (status == "completed") {
                ++completed;
            } else if (status == "exited_early") {
                ++exited;
            }
            const auto date = field_string(object, "date");
            if (!date.empty()) {
                entries += std::string{first ? "" : ","} +
                           "{\"date\":" + core_serde::JsonCodec{}.encode(date) + ",\"count\":1}";
                first = false;
            }
        }
        entries += "]";
        const auto item =
            "{\"automation_id\":" + core_serde::JsonCodec{}.encode(id) +
            ",\"total_run_count\":" + std::to_string(total) +
            ",\"in_progress_run_count\":" + std::to_string(running) +
            ",\"completed_run_count\":" + std::to_string(completed) +
            ",\"exited_early_run_count\":" + std::to_string(exited) + ",\"entries\":" + entries +
            ",\"entry_window\":{\"date_from\":" + core_serde::JsonCodec{}.encode(from) +
            ",\"date_to\":" + core_serde::JsonCodec{}.encode(to) +
            ",\"bucket\":\"day\",\"timezone\":" + core_serde::JsonCodec{}.encode(zone) + "}}";
        return core::workflow::StepResponse<Output>{
            {{"status", "200"},
             {"body", local_mapping::wrap("automation_performance_stats", "[" + item + "]")}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;
    using Array = serde::Value::Array;

    static std::string option(const core::query::ParsedQuery& q, std::string_view key)
    {
        const auto it = q.options.find(std::string{key});
        return it == q.options.end() ? std::string{} : it->second;
    }

    static bool valid_date(std::string_view date)
    {
        if (date.size() != 10 || date[4] != '-' || date[7] != '-') {
            return false;
        }
        for (std::size_t i = 0; i < date.size(); ++i) {
            if (i != 4 && i != 7 && !std::isdigit(static_cast<unsigned char>(date[i]))) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] static const Object* as_object(const serde::Value* v)
    {
        return v != nullptr ? std::get_if<Object>(&v->get()) : nullptr;
    }

    [[nodiscard]] static std::vector<serde::Value> as_array_or_empty(std::string_view json)
    {
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(json);
        if (!parsed) {
            return {};
        }
        const auto* arr = std::get_if<Array>(&parsed->get());
        return arr == nullptr ? std::vector<serde::Value>{} : *arr;
    }

    [[nodiscard]] static std::string field_string(const Object* object, std::string_view key)
    {
        if (object == nullptr) {
            return {};
        }
        for (const auto& [name, value]: *object) {
            if (name == key) {
                if (const auto* text = std::get_if<std::string>(&value.get())) {
                    return *text;
                }
            }
        }
        return {};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::automations::automations_id_performance_stats_get::workflow
