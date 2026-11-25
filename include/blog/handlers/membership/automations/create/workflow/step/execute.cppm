export module blog_handlers:automations_create_step;
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
import :automations_operations;
import :outbox_shim;

export namespace blog::handlers::automations::create::workflow {

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

namespace created_events {
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
            if (event.type == "automation.created") {
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
} // namespace created_events

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"automations_create_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const std::string body =
            in.body_as_string().empty() ? "{}" : std::string{in.body_as_string()};
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(body);
        const auto* root = parsed ? std::get_if<Object>(&parsed->get()) : nullptr;
        const Object* automation = input_automation(root);
        const std::string name = field_string(automation, "name");
        if (name.empty() || name.size() > 191) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}.validation(
                      "Validation error, cannot save automation.",
                      "name is required",
                      "name"
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

        // Ghost repository.add(): insert the automation, trigger rules and graph in one
        // transaction. The outbox stages the domain events for commit-time dispatch.
        created_events::ensure();
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        const auto result = ctx.store().create(
            "automations",
            core_serde::JsonCodec{}.encode(serde::Value{*automation})
        );
        const std::string id = first_id(result.body());
        outbox.emit(
            blog::handlers::outbox::Event{
                "automation.created",
                "{\"id\":\"" + id + "\",\"name\":" + core_serde::JsonCodec{}.encode(name) + "}",
                id
            }
        );
        outbox.commit();
        [[maybe_unused]] const auto drained = outbox.drain();

        const auto stored = ctx.store().resources().get("automations", id);
        return core::workflow::StepResponse<Output>{
            {{"status", "201"},
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

    /** @brief Ghost input serializer `add` reads `data.automations[0]`; a bare object is
     *  accepted for legacy callers so existing admin clients keep working. */
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

    static std::string first_id(std::string_view json)
    {
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(json);
        if (!parsed) {
            return {};
        }
        const auto* root = std::get_if<Object>(&parsed->get());
        if (root == nullptr) {
            return {};
        }
        for (const auto& [name, value]: *root) {
            const auto* array = std::get_if<Array>(&value.get());
            if (array == nullptr || array->empty()) {
                continue;
            }
            const auto* object = std::get_if<Object>(&array->front().get());
            if (object != nullptr) {
                return field_string(object, "id");
            }
        }
        return {};
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
} // namespace blog::handlers::automations::create::workflow
