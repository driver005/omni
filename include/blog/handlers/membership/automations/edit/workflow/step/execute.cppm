export module blog_handlers:automations_edit_step;
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
import interfaces;
import :outbox_shim;
import :automations_operations;

export namespace blog::handlers::automations::edit::workflow {

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

namespace updated_events {
    using Event = blog::handlers::outbox::Event;

    inline std::vector<Event>& log()
    {
        static std::vector<Event> v;
        return v;
    }

    class Adapter final : public blog::handlers::outbox::Adapter
    {
    public:
        void dispatch(const Event& event) override
        {
            if (event.type == "automation.updated") {
                log().push_back(event);
            }
        }
    };

    inline void ensure()
    {
        static const bool registered = []
        {
            blog::handlers::outbox::register_adapter(std::make_shared<Adapter>());
            return true;
        }();
        static_cast<void>(registered);
    }
} // namespace updated_events

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"automations_edit_execute"};

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
        if (!blog::api::automations::automation_exists(ctx, id)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body",
                  blog::api::automations::ghost_error("NotFoundError", "Automation not found.")}}
            };
        }
        const std::string body =
            in.body_as_string().empty() ? "{}" : std::string{in.body_as_string()};
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(body);
        const auto* root = parsed ? std::get_if<Object>(&parsed->get()) : nullptr;
        const Object* automation = input_automation(root);
        if (automation == nullptr) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}.validation(
                      "Validation error, cannot save automation.",
                      "Invalid payload",
                      "body"
                  )}}
            };
        }
        const std::string status = field_string(automation, "status");
        if (!status.empty() && status != "active" && status != "inactive" && status != "archived") {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}.validation(
                      "Validation error, cannot save automation.",
                      "Invalid status",
                      "status"
                  )}}
            };
        }

        updated_events::ensure();
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        const auto result = ctx.store().edit(
            "automations",
            id,
            core_serde::JsonCodec{}.encode(serde::Value{*automation})
        );
        outbox.emit(
            blog::handlers::outbox::Event{
                "automation.updated",
                "{\"id\":\"" + id + "\",\"status\":" + core_serde::JsonCodec{}.encode(status) + "}",
                id
            }
        );
        outbox.commit();
        [[maybe_unused]] const auto drained = outbox.drain();
        const auto stored = ctx.store().resources().get("automations", id);
        return core::workflow::StepResponse<Output>{
            {{"status", "200"},
             {"body",
              local_mapping::wrap(
                  "automations",
                  stored.empty() ? result.body()
                                 : blog::api::automations::build_automation(ctx, stored)
              )}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;
    using Array = serde::Value::Array;

    static const Object* input_automation(const Object* root)
    {
        if (root == nullptr) {
            return nullptr;
        }
        for (const auto& [name, value]: *root) {
            if (name != "data") {
                continue;
            }
            const auto* data = std::get_if<Object>(&value.get());
            if (data == nullptr) {
                break;
            }
            for (const auto& [key, list]: *data) {
                if (key != "automations") {
                    continue;
                }
                const auto* array = std::get_if<Array>(&list.get());
                if (array != nullptr && !array->empty()) {
                    const auto* first = std::get_if<Object>(&array->front().get());
                    if (first != nullptr) {
                        return first;
                    }
                }
            }
        }
        return root;
    }

    static std::string field_string(const Object* object, std::string_view key)
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
} // namespace blog::handlers::automations::edit::workflow
