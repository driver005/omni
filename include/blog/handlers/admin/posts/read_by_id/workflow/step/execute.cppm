export module blog_handlers:posts_read_by_id_step;
import :domain_all;
import :posts_support;

import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::posts::read_by_id::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"posts_read_by_id_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& input, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::posts::support;
        const bool content = is_content(const_cast<Input&>(input));
        const std::span<const std::string_view> allowed =
            content ? std::span<const std::string_view>{kPublicReadOptions}
                    : std::span<const std::string_view>{kAdminReadOptions};
        const auto parsed = parse(input.get_path(), allowed, false, content);
        if (!parsed.error.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", parsed.status}, {"body", parsed.error}}
            };
        }
        if (const auto error = validate_options(parsed.query, content)) {
            return core::workflow::StepResponse<Output>{{{"status", "422"}, {"body", *error}}};
        }

        const std::string identifier = path_tail(input.get_path());
        if (identifier.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body",
                  blog::api::ApiError{}
                      .json("A post id, slug or uuid is required.", "Bad Request")}}
            };
        }

        auto entity = identifier_like_id(identifier)
                          ? s_ctx->store().find_by_resource("posts", identifier)
                          : std::optional<std::string>{};
        if (!entity) {
            entity = find_by_identifier(*s_ctx, identifier);
        }
        if (!entity) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body", blog::api::ApiError{}.json("Post not found.", "Not Found")}}
            };
        }

        std::string row = *entity;
        if (content) {
            row = gift_access(row, const_cast<Input&>(input), *s_ctx);
        }
        return core::workflow::StepResponse<Output>{
            {{"status", "200"}, {"body", read_body(row, parsed.query, content)}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    [[nodiscard]] static bool identifier_like_id(std::string_view text)
    {
        if (text.empty()) {
            return false;
        }
        if (std::ranges::all_of(
                text,
                [](unsigned char c)
                {
                    return std::isdigit(c) != 0;
                }
            )) {
            return true;
        }
        return text.size() == 24 && std::ranges::all_of(
                                        text,
                                        [](unsigned char c)
                                        {
                                            return std::isxdigit(c) != 0;
                                        }
                                    );
    }

    [[nodiscard]] static std::optional<std::string>
    find_by_identifier(blog::BlogContext& ctx, std::string_view identifier)
    {
        using namespace blog::handlers::posts::support;
        Query q;
        q.raw_filter =
            std::string{"slug:"} + std::string{identifier} + ",uuid:" + std::string{identifier};
        q.limit = std::numeric_limits<std::size_t>::max();
        q.page = 1;
        const auto rows = ctx.store().json_for("posts", {});
        const auto applied = query_apply(rows, q);
        const auto parsed = decode(applied);
        if (!parsed || !std::holds_alternative<serde::Value::Array>(parsed->get())) {
            return std::nullopt;
        }
        const auto& array = std::get<serde::Value::Array>(parsed->get());
        if (array.empty() || !std::holds_alternative<serde::Value::Object>(array.front().get())) {
            return std::nullopt;
        }
        return core_serde::JsonCodec{}.encode(array.front());
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::posts::read_by_id::workflow
