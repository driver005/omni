export module blog_handlers:tags_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import interfaces;

export namespace blog::handlers::tags::browse::workflow {

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
    static constexpr std::string_view name{"tags_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        constexpr std::array<std::string_view, 7>
            kOptions{"include", "filter", "fields", "limit", "order", "page", "debug"};
        auto q = core::query::parse(in.get_path(), kOptions);
        for (const auto& inc: q.include) {
            if (inc != "count.posts") {
                return core::workflow::StepResponse<Output>{
                    {{"status", "422"},
                     {"body",
                      blog::api::ApiError{}
                          .validation("Validation error", "include invalid", "include")}}
                };
            }
        }
        const std::string path{in.get_path()};
        if (path.find("/content/") != std::string::npos && q.order.empty()) {
            q.order = "name asc"; // tags-public input serializer default
        }

        const auto rows = ctx.store().json_for("tags", {});
        const auto cleaned = local_mapping::clean(rows, "tag");
        std::string payload;
        try {
            // core::query enforces the restricted-field guard and applies NQL filter,
            // order, fields projection and real pagination in one Ghost-equivalent pass.
            payload = core::query::run(cleaned, q);
        } catch (const std::runtime_error&) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body",
                  blog::api::ApiError{}
                      .json("Restricted field used in Ghost query", "Bad Request")}}
            };
        }
        const auto colon = payload.find(':'); // "data" -> Ghost collection key "tags"
        const std::string body = "{\"tags\":" + payload.substr(colon + 1);
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::tags::browse::workflow
