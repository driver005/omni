export module blog_handlers:tiers_read_by_id_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_serde;
import interfaces;

export namespace blog::handlers::tiers::read_by_id::workflow {

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
    static constexpr std::string_view name{"tiers_read_by_id_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        const auto q = core::query::parse(in.get_path(), kOptions);
        for (const auto& inc: q.include) {
            if (inc != "requirements") {
                return invalid("include", "Invalid include value");
            }
        }

        const std::string path_text{in.get_path()};
        const std::string_view path{path_text};
        const auto bare = path.substr(0, path.find('?'));
        auto tail = bare;
        while (tail.size() > 1 && tail.back() == '/') {
            tail.remove_suffix(1);
        }
        const auto slash = tail.find_last_of('/');
        const std::string id{slash == std::string_view::npos ? tail : tail.substr(slash + 1)};
        if (id.empty()) {
            return invalid("id", "A tier id is required");
        }
        // TiersAPI.read converts through ObjectID.createFromHexString (tiers-api.js:83).
        if (!is_object_id(id)) {
            return failure("400", "Invalid ObjectID");
        }

        const auto one = ctx.store().find_by_resource("tiers", id);
        if (!one) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body", blog::api::ApiError{}.json("Resource not found", "Not Found")}}
            };
        }
        const auto body = local_mapping::wrap("tiers", clean_one("{\"tiers\":[" + *one + "]}"));
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;
    static constexpr std::string_view kOptions[]{"include"};

    [[nodiscard]] static bool is_object_id(std::string_view id) noexcept
    {
        if (id.size() != 24) {
            return false;
        }
        for (char c: id) {
            const bool digit = c >= '0' && c <= '9';
            const bool hex = (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
            if (!digit && !hex) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] static std::string clean_one(std::string_view envelope)
    {
        auto value = core_serde::JsonCodec{}.decode<serde::Value>(envelope);
        if (!value) {
            return "[]";
        }
        const auto* root = std::get_if<Object>(&value->get());
        if (root == nullptr) {
            return "[]";
        }
        for (const auto& [key, val]: *root) {
            if (key == "tiers") {
                return local_mapping::clean(core_serde::JsonCodec{}.encode(val), "tier");
            }
        }
        return "[]";
    }

    static core::workflow::StepResponse<Output>
    invalid(std::string_view property, std::string_view context)
    {
        return core::workflow::StepResponse<Output>{
            {{"status", "422"},
             {"body", blog::api::ApiError{}.validation("Validation error", context, property)}}
        };
    }

    static core::workflow::StepResponse<Output>
    failure(std::string_view code, std::string_view message)
    {
        return core::workflow::StepResponse<Output>{
            {{"status", std::string{code}},
             {"body", blog::api::ApiError{}.json(message, "Bad Request")}}
        };
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::tiers::read_by_id::workflow
