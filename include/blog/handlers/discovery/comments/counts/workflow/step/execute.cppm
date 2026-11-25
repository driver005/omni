export module blog_handlers:comments_counts_step;
import :domain_all;
import :comments_model_comment;
import :comments_model_member;
import :comments_model_wire_fixtures;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::comments::counts::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"comments_counts_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input&, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const auto rows = ctx.store().json_for(
            "comments",
            std::map<std::string, std::string>{{"filter[status]", "published"}}
        );
        std::map<std::string, std::size_t> counts;
        const auto value = core_serde::JsonCodec{}.decode<serde::Value>(rows);
        if (value) {
            if (const auto* array = std::get_if<serde::Value::Array>(&value->get())) {
                for (const auto& item: *array) {
                    if (const auto* object = std::get_if<Object>(&item.get())) {
                        const auto* post = value_for(object, "post_id");
                        if (post == nullptr) {
                            continue;
                        }
                        if (const auto* text = std::get_if<std::string>(&post->get())) {
                            counts[*text] += 1;
                        }
                    }
                }
            }
        }
        std::string body = "{";
        bool first = true;
        for (const auto& [post_id, count]: counts) {
            body += (first ? std::string{} : std::string{","}) + "\"" + post_id +
                    "\":" + std::to_string(count);
            first = false;
        }
        body += "}";
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;

    static const serde::Value* value_for(const Object* object, std::string_view key)
    {
        if (object == nullptr) {
            return nullptr;
        }
        for (const auto& [name, value]: *object) {
            if (name == key) {
                return &value;
            }
        }
        return nullptr;
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::comments::counts::workflow
