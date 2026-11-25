export module blog_handlers:labels_read_by_id_step;
import :domain_all;
import :labels_browse_step;
import std;
import core_workflow;
import core_query;
import core_serde;
import interfaces;

export namespace blog::handlers::labels::read_by_id::workflow {

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
    static constexpr std::string_view name{"labels_read_by_id_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        auto& ctx = *s_ctx;

        constexpr std::array<std::string_view, 3> kOptions{"include", "filter", "fields"};
        const auto q = core::query::parse(in.get_path(), kOptions);
        for (const auto& include: q.include) {
            if (include != "count.members") {
                return core::workflow::StepResponse<Output>{
                    {{"status", "422"},
                     {"body",
                      validation(
                          "Validation error, cannot read label.",
                          "Invalid include",
                          "include"
                      )}}
                };
            }
        }
        if (restricted(q)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body", bad_request("Restricted fields cannot be used in order.")}}
            };
        }

        const auto id = last_segment(in.get_path());
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  validation("Validation error, cannot read label.", "id is required", "id")}}
            };
        }

        // models.Label.findOne({id}, restricted options) expressed through the
        // shared NQL filter parser; numeric ids resolve to their Ghost wire ids.
        constexpr std::array<std::string_view, 1> kFilter{"filter"};
        auto fq = core::query::parse("?filter[id]=" + numeric_id(id), kFilter);
        fq.fields = q.fields;
        const auto available =
            local_mapping::clean(rows(ctx, q.include.contains("count.members")), "label");
        std::string matched;
        try {
            matched = core::query::apply(available, fq);
        } catch (const std::runtime_error&) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body", bad_request("Restricted fields cannot be used in order.")}}
            };
        }
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(matched);
        const auto* array = parsed ? std::get_if<serde::Value::Array>(&parsed->get()) : nullptr;
        if (array == nullptr || array->empty()) {
            return core::workflow::StepResponse<Output>{{{"status", "404"}, {"body", not_found()}}};
        }
        const std::string body =
            local_mapping::wrap("labels", core_serde::JsonCodec{}.encode((*array)[0]));
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    [[nodiscard]] static std::string last_segment(std::string_view path)
    {
        const auto base = path.substr(0, path.find('?'));
        return std::string{base.substr(base.rfind('/') + 1)};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::labels::read_by_id::workflow
