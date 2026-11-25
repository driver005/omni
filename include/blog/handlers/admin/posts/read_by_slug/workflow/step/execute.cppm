export module blog_handlers:posts_read_by_slug_step;
import :domain_all;
import :posts_support;

import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::posts::read_by_slug::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"posts_read_by_slug_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& input, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::posts::support;
        const bool content = is_content(const_cast<Input&>(input));
        const auto parsed = parse(input.get_path(), kPublicReadOptions, false, content);
        if (!parsed.error.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", parsed.status}, {"body", parsed.error}}
            };
        }
        if (const auto error = validate_options(parsed.query, content)) {
            return core::workflow::StepResponse<Output>{{{"status", "422"}, {"body", *error}}};
        }

        const std::string slug = path_tail(input.get_path());
        if (slug.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body",
                  blog::api::ApiError{}
                      .json("A post id, slug or uuid is required.", "Bad Request")}}
            };
        }

        Query lookup;
        lookup.raw_filter = "slug:" + slug;
        lookup.limit = std::numeric_limits<std::size_t>::max();
        lookup.page = 1;
        const auto rows = s_ctx->store().json_for("posts", {});
        const auto applied = query_apply(rows, lookup);
        const auto decoded = decode(applied);
        std::string entity;
        if (decoded && std::holds_alternative<serde::Value::Array>(decoded->get())) {
            const auto& array = std::get<serde::Value::Array>(decoded->get());
            if (!array.empty() &&
                std::holds_alternative<serde::Value::Object>(array.front().get())) {
                entity = core_serde::JsonCodec{}.encode(array.front());
            }
        }
        if (entity.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body", blog::api::ApiError{}.json("Post not found.", "Not Found")}}
            };
        }
        if (content) {
            entity = gift_access(entity, const_cast<Input&>(input), *s_ctx);
        }
        return core::workflow::StepResponse<Output>{
            {{"status", "200"}, {"body", read_body(entity, parsed.query, content)}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::posts::read_by_slug::workflow
