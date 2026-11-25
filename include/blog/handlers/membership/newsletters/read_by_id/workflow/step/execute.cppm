export module blog_handlers:newsletters_read_by_id_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import interfaces;
import :newsletters_browse_step;

export namespace blog::handlers::newsletters::read_by_id::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"newsletters_read_by_id_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        auto& ctx = *s_ctx;
        const std::string path{in.get_path()};
        std::string id = path.substr(path.rfind('/') + 1);
        if (const auto q = id.find('?'); q != std::string::npos) {
            id = id.substr(0, q);
        }
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body", validation("Validation error", "An id is required", "id")}}
            };
        }

        constexpr std::array<std::string_view, 5>
            kOptions{"include", "fields", "debug", "forUpdate", "transacting"};
        auto q = core::query::parse(in.get_path(), kOptions);
        if (!valid_include(q)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body", validation("Validation error", "Invalid include", "include")}}
            };
        }

        const auto one = ctx.store().find_by_resource("newsletters", id);
        if (!one) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"}, {"body", not_found("Newsletter not found.")}}
            };
        }

        // service.read -> findOne(data, options) -> toJSON(frame.options).
        std::string rows = "[" + *one + "]";
        try {
            rows = core::query::apply(rows, q);
        } catch (const std::runtime_error&) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body", bad_request("Restricted fields cannot be used in order.")}}
            };
        }
        rows = embed_counts(
            rows,
            q.include,
            ctx.store().total("posts"),
            ctx.store().total("members"),
            ctx.store().total("members")
        );
        if (request_context(in) == "CONTENT_API") {
            rows = content_dto(rows);
        }
        return core::workflow::StepResponse<Output>{
            {{"status", "200"}, {"body", wrap("newsletters", rows)}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::newsletters::read_by_id::workflow
