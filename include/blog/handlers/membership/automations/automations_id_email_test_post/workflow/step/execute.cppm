export module blog_handlers:automations_automations_id_email_test_post_step;
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

export namespace blog::handlers::automations::automations_id_email_test_post::workflow {
namespace test_mail_events {
    using Event = blog::handlers::outbox::Event;

    inline std::vector<Event>& log()
    {
        static std::vector<Event> v;
        return v;
    }

    inline blog::BlogContext*& ctx_ptr()
    {
        static blog::BlogContext* p{nullptr};
        return p;
    }

    class Adapter final : public blog::handlers::outbox::Adapter
    {
    public:
        void dispatch(const Event& event) override
        {
            if (event.type != "email.send_test") {
                return;
            }
            log().push_back(event);
            if (ctx_ptr() != nullptr) {
                ctx_ptr()->store().resources().create("emails", event.payload);
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
} // namespace test_mail_events

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"automations_automations_id_email_test_post_execute"};

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
        if (!blog::api::automations::automation_exists(*s_ctx, id)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body",
                  blog::api::automations::ghost_error("NotFoundError", "Automation not found.")}}
            };
        }
        const auto r = s_ops->automation_email_test(id, in.body_as_string());
        if (r.first != 0) {
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(r.first)}, {"body", r.second}}
            };
        }
        test_mail_events::ctx_ptr() = s_ctx;
        test_mail_events::ensure();
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.emit(
            blog::handlers::outbox::Event{
                "email.send_test",
                "{\"to\":" + core_serde::JsonCodec{}.encode(email_of(in.body_as_string())) +
                    ",\"status\":\"submitted\",\"direction\":\"outbound\"}",
                id
            }
        );
        [[maybe_unused]] const auto drained = outbox.drain();
        return core::workflow::StepResponse<Output>{{{"status", "204"}, {"body", ""}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
    static blog::api::automations::AutomationsOperations* s_ops;

private:
    using Object = serde::Value::Object;

    [[nodiscard]] static std::string email_of(std::string_view body)
    {
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(body);
        if (!parsed) {
            return {};
        }
        const auto* object = std::get_if<Object>(&parsed->get());
        if (object == nullptr) {
            return {};
        }
        for (const auto& [name, value]: *object) {
            if (name == "email") {
                if (const auto* text = std::get_if<std::string>(&value.get())) {
                    return *text;
                }
            }
        }
        return {};
    }
};

blog::api::automations::AutomationsOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::automations::automations_id_email_test_post::workflow
