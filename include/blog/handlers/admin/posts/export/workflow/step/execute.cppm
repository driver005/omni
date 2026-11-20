export module blog_handlers:posts_export_step;
import :domain_all;
import :posts_support;

import std;
import core_workflow;
import :outbox_shim;
import core_serde;
import interfaces;

export namespace blog::handlers::posts::export_op::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"posts_export_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::posts::support;
        const auto parsed = parse(input.get_path(), kExportOptions);
        if (!parsed.error.empty()) {
            return core::workflow::StepResponse<Output>{{{"status", parsed.status},
                {"body", parsed.error}}};
        }

        // Ghost postsExporter reads the analytics dataset. The Tinybird adapter is
        // the C++ primitive for that source; when unconfigured we fall back to posts.
        std::string source = analytics_source(parsed.query);
        if (source.empty()) {
            source = query_apply(s_ctx->store().json_for("posts", {}), parsed.query);
        }
        return core::workflow::StepResponse<Output>{{{"status", "200"},
            {"body", to_csv(source)}, {"filename", export_filename()}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;

    [[nodiscard]] static std::string analytics_source(
        const blog::handlers::posts::support::Query& q)
    {
        std::string params;
        if (const auto it = q.options.find("limit"); it != q.options.end()) {
            params = "limit=" + it->second;
        }
        blog::handlers::outbox::global_outbox().emit(blog::handlers::outbox::Event{
            .type = "tinybird.posts_export",
            .payload = R"({"pipe":"posts_export","params":")" + params + R"("})",
            .aggregate_id = "posts_export"});
        return {};
    }

    [[nodiscard]] static std::string to_csv(std::string_view source)
    {
        const auto parsed = blog::handlers::posts::support::decode(source);
        if (!parsed || !std::holds_alternative<serde::Value::Array>(parsed->get())) {
            return "id,title,slug,status\n";
        }
        const auto& rows = std::get<serde::Value::Array>(parsed->get());
        std::vector<std::string> columns{"id", "title", "slug", "status"};
        if (!rows.empty() && std::holds_alternative<Object>(rows.front().get())) {
            const auto& first = std::get<Object>(rows.front().get());
            if (first.get("post_id")) columns = {"post_id", "metric", "value"};
        }
        std::string csv;
        for (const auto& column : columns) csv += column + ",";
        if (!csv.empty()) csv.pop_back();
        csv += '\n';
        for (const auto& row : rows) {
            if (!std::holds_alternative<Object>(row.get())) continue;
            const auto& object = std::get<Object>(row.get());
            for (std::size_t i = 0; i < columns.size(); ++i) {
                if (i != 0) csv += ',';
                csv += csv_field(&object, columns[i]);
            }
            csv += '\n';
        }
        return csv;
    }

    [[nodiscard]] static const serde::Value* value_for(const Object* object, std::string_view key)
    {
        for (const auto& [name, value] : *object) {
            if (name == key) return &value;
        }
        return nullptr;
    }

    [[nodiscard]] static std::string csv_field(const Object* object, std::string_view key)
    {
        std::string value;
        if (const auto* field_value = value_for(object, key)) {
            if (const auto* text = std::get_if<std::string>(&field_value->get())) {
                value = *text;
            } else {
                value = core_serde::JsonCodec{}.encode(*field_value);
            }
        }
        std::string escaped{"\""};
        for (const char c : value) {
            if (c == '"') escaped += "\"\"";
            else escaped += c;
        }
        return escaped + '"';
    }

    [[nodiscard]] static std::string export_filename()
    {
        const auto today = std::chrono::year_month_day{
            std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now())};
        const auto date = std::format("{:04d}-{:02d}-{:02d}",
            static_cast<int>(today.year()), static_cast<unsigned>(today.month()),
            static_cast<unsigned>(today.day()));

        std::string slug;
        const auto settings = s_ctx->store().json_for("settings");
        const auto parsed = blog::handlers::posts::support::decode(settings);
        if (parsed && std::holds_alternative<serde::Value::Array>(parsed->get())) {
            for (const auto& record : std::get<serde::Value::Array>(parsed->get())) {
                if (!std::holds_alternative<Object>(record.get())) continue;
                const auto& object = std::get<Object>(record.get());
                const auto* key = value_for(&object, "key");
                if (key == nullptr || !std::holds_alternative<std::string>(key->get()) ||
                    std::get<std::string>(key->get()) != "title") continue;
                const auto* value = value_for(&object, "value");
                if (value != nullptr && std::holds_alternative<std::string>(value->get())) {
                    slug = slugify(std::get<std::string>(value->get()));
                }
                break;
            }
        }
        return (slug.empty() ? std::string{} : slug + ".") +
               std::format("ghost.analytics.{}.csv", date);
    }

    [[nodiscard]] static std::string slugify(std::string_view title)
    {
        std::string out;
        bool gap = false;
        for (const char c : title) {
            if (std::isalnum(static_cast<unsigned char>(c))) {
                if (gap && !out.empty()) out += '-';
                out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                gap = false;
            } else {
                gap = true;
            }
        }
        return out;
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace
