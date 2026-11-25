export module blog_handlers:posts_upload_step;
import :domain_all;
import :posts_support;

import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::posts::upload::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"posts_upload_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& input, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::posts::support;
        const auto raw = std::string{input.body_as_string()};
        auto json = raw.empty() ? std::string{"{}"} : raw;
        std::size_t total = 0;

        const auto parsed = decode(raw);
        if (parsed && std::holds_alternative<Object>(parsed->get())) {
            const auto& object = std::get<Object>(parsed->get());
            for (const auto& [name, value]: object) {
                if (name == "rows" && std::holds_alternative<serde::Value::Array>(value.get())) {
                    total = std::get<serde::Value::Array>(value.get()).size();
                }
                if (name == "csv" && std::holds_alternative<std::string>(value.get())) {
                    const auto& text = std::get<std::string>(value.get());
                    const auto lines = static_cast<std::size_t>(std::ranges::count(text, '\n'));
                    total = lines == 0 ? (text.empty() ? 0 : 1) : lines;
                }
            }
            json = decorate(json);
        }

        const auto id = s_ctx->store().resources().create("content_imports", json);
        emit(
            "posts.import_csv",
            std::format(
                R"({{"import_id":{},"total":{},"docName":"content_imports","method":"importContent"}})",
                quote(id),
                total
            ),
            id
        );
        return core::workflow::StepResponse<Output>{
            {{"status", "202"},
             {"body", std::format(R"({{"meta":{{"import_id":"{}","total":{}}}}})", id, total)}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;

    [[nodiscard]] static std::string quote(std::string_view text)
    {
        return core_serde::JsonCodec{}.encode(std::string{text});
    }

    /** @brief Mirrors Ghost's `docName`/`method` metadata for the import record. */
    [[nodiscard]] static std::string decorate(std::string json)
    {
        while (!json.empty() && std::isspace(static_cast<unsigned char>(json.back()))) {
            json.pop_back();
        }
        if (!json.empty() && json.back() == '}') {
            json.pop_back();
        }
        if (!json.empty() && json.front() == '{') {
            json.erase(0, 1);
        }
        while (!json.empty() && (std::isspace(static_cast<unsigned char>(json.front())) ||
                                 std::isspace(static_cast<unsigned char>(json.back())))) {
            if (std::isspace(static_cast<unsigned char>(json.front()))) {
                json.erase(0, 1);
            } else {
                json.pop_back();
            }
        }
        if (!json.empty()) {
            json += ',';
        }
        return "{" + json + R"("docName":"content_imports","method":"importContent"})";
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::posts::upload::workflow
