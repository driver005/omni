export module blog_handlers:labels_create_step;
import :domain_all;
import :labels_browse_step;
import std;
import core_workflow;
import core_query;
import core_serde;
import :outbox_shim;
import interfaces;

export namespace blog::handlers::labels::create::workflow {

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
    static constexpr std::string_view name{"labels_create_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        auto& ctx = *s_ctx;

        constexpr std::array<std::string_view, 1> kOptions{"include"};
        const auto q = core::query::parse(in.get_path(), kOptions);
        for (const auto& include: q.include) {
            if (include != "count.members") {
                return core::workflow::StepResponse<Output>{
                    {{"status", "422"},
                     {"body",
                      validation(
                          "Validation error, cannot save label.",
                          "Invalid include",
                          "include"
                      )}}
                };
            }
        }

        const auto inner = unwrap(in.body_as_string());
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(inner);
        const auto* object = parsed ? std::get_if<serde::Value::Object>(&parsed->get()) : nullptr;
        const std::string name = object != nullptr ? scalar(*object, "name") : std::string{};
        if (name.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  validation("Validation error, cannot save label.", "name is required", "name")}}
            };
        }
        if (name.size() > 191) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  validation(
                      "Validation error, cannot save label.",
                      "name should NOT be longer than 191 characters",
                      "name"
                  )}}
            };
        }
        const std::string trimmed = trim(name);
        if (trimmed.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  validation("Validation error, cannot save label.", "name is required", "name")}}
            };
        }
        if (name_taken(ctx, trimmed)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"}, {"body", validation("Label already exists", {}, {})}}
            };
        }

        std::string slug = object != nullptr ? scalar(*object, "slug") : std::string{};
        if (slug.empty()) {
            slug = slugify(trimmed);
        }
        slug = unique_slug(ctx, slug);
        const std::string stamp = now();
        const std::string payload = "{\"name\":" + quote(trimmed) + ",\"slug\":" + quote(slug) +
                                    R"(,"count":{"posts":0})" + ",\"created_at\":" + quote(stamp) +
                                    ",\"updated_at\":" + quote(stamp) + "}";

        const std::string numeric = ctx.store().resources().create("labels", payload);
        const std::string id = wire_id(numeric);
        const auto stored = ctx.store().resources().get("labels", numeric);
        const std::string body = local_mapping::wrap("labels", canonical(stored));

        // label.added == Ghost model/label.js onCreated emitChange('added').
        blog::handlers::outbox::global_outbox().emit(
            blog::handlers::outbox::Event{"label.added", body, id}
        );

        return core::workflow::StepResponse<Output>{
            {{"status", "201"}, {"body", body}, {"location", location(in, id)}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    [[nodiscard]] static std::string trim(std::string_view value)
    {
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
            value.remove_prefix(1);
        }
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
            value.remove_suffix(1);
        }
        return std::string{value};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::labels::create::workflow
