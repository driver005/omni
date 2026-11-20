export module blog_handlers:posts_create_step;
import :domain_all;
import :posts_support;

import std;
import core_workflow;
import core_serde;
import :outbox_shim;
import interfaces;

export namespace blog::handlers::posts::create::workflow {

namespace local_mapping {
[[nodiscard]] inline std::string wrap(std::string_view resource, std::string_view rows,
                                      std::string_view meta = {})
{
    return "{\"" + std::string{resource} + "\":" + std::string{rows} +
           (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
}
[[nodiscard]] inline std::string clean(std::string_view json, std::string_view) { return std::string{json}; }
[[nodiscard]] inline std::string pagination(std::size_t total, std::size_t limit, std::size_t page,
                                            std::size_t pages, std::string_view prev,
                                            std::string_view next)
{
    return std::format(R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
                       total, limit, page, pages,
                       prev.empty() ? std::string{"null"} : std::string{prev},
                       next.empty() ? std::string{"null"} : std::string{next});
}
} // namespace local_mapping

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"posts_create_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::posts::support;
        const auto parsed = parse(input.get_path(), kAddOptions);
        if (!parsed.error.empty()) {
            return core::workflow::StepResponse<Output>{{{"status", parsed.status},
                {"body", parsed.error}}};
        }
        if (const auto error = validate_options(parsed.query)) {
            return core::workflow::StepResponse<Output>{{{"status", "422"}, {"body", *error}}};
        }

        const auto raw = std::string{input.body_as_string()};
        std::string post_json = raw.empty() ? std::string{"{}"} : raw;
        const auto decoded = decode(raw);
        if (decoded && std::holds_alternative<Object>(decoded->get())) {
            const auto& root = std::get<Object>(decoded->get());
            if (const auto data = value_for(root, "data")) {
                if (std::holds_alternative<Object>(data->get())) {
                    if (const auto posts = value_for(std::get<Object>(data->get()), "posts")) {
                        if (std::holds_alternative<serde::Value::Array>(posts->get())) {
                            const auto& array = std::get<serde::Value::Array>(posts->get());
                            if (!array.empty() && std::holds_alternative<Object>(array.front().get())) {
                                post_json = core_serde::JsonCodec{}.encode(array.front());
                            }
                        }
                    }
                }
            }
            if (const auto error = unsafe_attrs_error(const_cast<Input&>(input), post_json)) {
                return core::workflow::StepResponse<Output>{{{"status", "403"}, {"body", *error}}};
            }
        }

        const std::string status = field_of(post_json, "status");
        auto& ctx = *s_ctx;
        blog::handlers::outbox::global_outbox().begin_transaction();
        const auto result = ctx.store().create("posts", post_json);
        const std::string id = first_post_id(result.body());
        emit("posts.added", result.body(), id);
        blog::handlers::outbox::global_outbox().commit();

        return core::workflow::StepResponse<Output>{{{"status", std::to_string(result.status())},
            {"body", mapped(result.body())}, {"published", status == "published" ? "true" : "false"},
            {"location", "/posts/" + id + "/"}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;

    [[nodiscard]] static const serde::Value* value_for(const Object& object, std::string_view key)
    {
        for (const auto& [name, value] : object) {
            if (name == key) return &value;
        }
        return nullptr;
    }

    [[nodiscard]] static std::string mapped(std::string_view body)
    {
        using namespace blog::handlers::posts::support;
        const auto parsed = decode(body);
        if (parsed && std::holds_alternative<Object>(parsed->get())) {
            const auto* posts = value_for(std::get<Object>(parsed->get()), "posts");
            if (posts != nullptr && std::holds_alternative<serde::Value::Array>(posts->get())) {
                std::string rows = "[";
                bool first = true;
                for (const auto& row : std::get<serde::Value::Array>(posts->get())) {
                    if (!first) rows += ',';
                    rows += local_mapping::clean(core_serde::JsonCodec{}.encode(row), "post");
                    first = false;
                }
                rows += ']';
                return local_mapping::wrap("posts", rows);
            }
        }
        return std::string{body};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace
