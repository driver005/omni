export module blog_handlers:exports_create_step;
import :domain_all;
import :outbox_shim;
import std;
import core_workflow;
import core_serde;
import interfaces;
import blog_handlers_middleware;

export namespace blog::handlers::exports::create::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"exports_create_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        // Ghost `add` validates a strict body schema before enqueuing work (exports.js:155-184).
        const auto raw = in.body_as_string();
        const auto root = core_serde::JsonCodec{}.decode<serde::Value>(raw.empty() ? "{}" : raw);
        if (!root) {
            return invalid("components must be an object");
        }
        const auto* object = std::get_if<serde::Value::Object>(&root->get());
        if (object == nullptr) {
            return invalid("components must be an object");
        }

        const auto* components = value_for(object, "components");
        if (components == nullptr) {
            if (const auto* data = value_for(object, "data")) {
                if (const auto* data_object = std::get_if<serde::Value::Object>(&data->get())) {
                    components = value_for(data_object, "components");
                }
            }
        }
        const auto* component_object =
            components == nullptr ? nullptr : std::get_if<serde::Value::Object>(&components->get());
        if (component_object == nullptr) {
            return invalid("components must be an object");
        }

        static constexpr std::array<std::string_view, 6>
            async_components{"members", "analytics", "content", "themes", "routes", "media"};
        std::string normalized = "{";
        bool first = true;
        bool any_true = false;
        for (const auto& [name, value]: *component_object) {
            if (std::ranges::find(async_components, std::string_view{name}) ==
                async_components.end()) {
                return invalid("Unknown export components: " + std::string{name});
            }
            const auto* flag = std::get_if<bool>(&value.get());
            if (flag == nullptr) {
                return invalid("Export component values must be booleans");
            }
            normalized += (first ? "" : ",") + q(name) + ":" + (*flag ? "true" : "false");
            first = false;
            any_true = any_true || *flag;
        }
        for (auto key: async_components) {
            if (value_for(component_object, key) == nullptr) {
                normalized += (first ? "" : ",") + q(key) + ":false";
                first = false;
            }
        }
        normalized += "}";
        if (!any_true) {
            return invalid("At least one export component must be selected");
        }

        // `frame.options.context.user` is carried by the harness as `x-blog-user-id`
        // (same convention as the other Admin API steps); the bearer token is the
        // fallback request-scoped identity primitive.
        std::string user{in.find_header("x-blog-user-id")};
        if (user.empty()) {
            user = std::string{blog::handlers::middleware::token(in)};
        }

        // Ghost writes no model record: `exportRequests.service.requestExport` builds a
        // signed webhook and hands it to the host. With `core_adapters` not a registered
        // dependency, the context-local outbox carries the webhook-shaped request so the
        // side effect is observable and transactional (CONTRACTS.md §4).
        const auto payload = std::format(
            R"({{"type":"export","siteId":{},"components":{},"requestedByUserId":{}}})",
            q(std::string{in.find_header("x-blog-site-id")}),
            normalized,
            q(user)
        );
        blog::handlers::outbox::global_outbox().emit(
            blog::handlers::outbox::Event{
                .type = "export.requested",
                .payload = payload,
                .aggregate_id = "exports"
            }
        );

        // Ghost `add.statusCode = 202` with JSON body `{}` (http.ts:147).
        return core::workflow::StepResponse<Output>{{{"status", "202"}, {"body", "{}"}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;

    static std::string q(std::string_view text)
    {
        return core_serde::JsonCodec{}.encode(std::string{text});
    }

    static const serde::Value* value_for(const Object* object, std::string_view key)
    {
        for (const auto& [name, value]: *object) {
            if (name == key) {
                return &value;
            }
        }
        return nullptr;
    }

    static core::workflow::StepResponse<Output> invalid(std::string_view reason)
    {
        return core::workflow::StepResponse<Output>{
            {{"status", "400"},
             {"body", blog::api::ApiError{}.validation(reason, "", "", "BadRequestError")}}
        };
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::exports::create::workflow
