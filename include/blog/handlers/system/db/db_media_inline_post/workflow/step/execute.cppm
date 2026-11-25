export module blog_handlers:db_db_media_inline_post_step;
import :domain_all;
import std;
import core_serde;
import core_workflow;
import interfaces;

export namespace blog::handlers::db::db_media_inline_post::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"db_db_media_inline_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const std::string raw =
            in.body_as_string().empty() ? std::string{"{}"} : in.body_as_string();
        // Ghost startInlining uses DEFAULT_DOMAINS when domains is absent/empty
        // (external-media-inliner.js:11,676-681) — no 422 for an empty list.
        const auto domains = decode_data_domains(raw);
        const std::string selected =
            domains ? *domains : R"(["https://s3.amazonaws.com/revue","https://substackcdn.com"])";

        // Record the ExternalMediaInlinerJob in the context-local db namespace; the
        // real network fetch/rewrite has no allowed primitive here.
        (void)ctx.store().create(
            "db",
            "{\"kind\":\"ExternalMediaInlinerJob\",\"domains\":" + selected + "}"
        );
        (void)ctx.store().json_for("posts", {{"filter", "html:*"}});

        // db.js:129-133 returns {status:'success'}; output/default.js + handle.ts wrap
        // it as {db:[{status:'success'}]}. cacheInvalidate:false -> no header.
        return core::workflow::StepResponse<Output>{
            {{"status", "200"}, {"body", R"({"db":[{"status":"success"}]})"}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    [[nodiscard]] static std::optional<std::string> decode_data_domains(std::string_view raw)
    {
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(raw);
        if (!parsed) {
            return std::nullopt;
        }
        const auto* root = std::get_if<serde::Value::Object>(&parsed->get());
        if (root == nullptr) {
            return std::nullopt;
        }
        const serde::Value* data = nullptr;
        for (const auto& [key, value]: *root) {
            if (key == "data") {
                data = &value;
                break;
            }
        }
        if (data == nullptr) {
            return std::nullopt;
        }
        const auto* data_object = std::get_if<serde::Value::Object>(&data->get());
        if (data_object == nullptr) {
            return std::nullopt;
        }
        const serde::Value* list = nullptr;
        for (const auto& [key, value]: *data_object) {
            if (key == "domains") {
                list = &value;
                break;
            }
        }
        if (list == nullptr) {
            return std::nullopt;
        }
        const auto* array = std::get_if<serde::Value::Array>(&list->get());
        if (array == nullptr || array->empty()) {
            return std::nullopt;
        }
        std::string out = "[";
        bool first = true;
        for (const auto& item: *array) {
            const auto* domain = std::get_if<std::string>(&item.get());
            if (domain == nullptr || domain->empty()) {
                return std::nullopt;
            }
            if (!first) {
                out += ",";
            }
            out += core_serde::JsonCodec{}.encode(*domain);
            first = false;
        }
        out += "]";
        return out;
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::db::db_media_inline_post::workflow
