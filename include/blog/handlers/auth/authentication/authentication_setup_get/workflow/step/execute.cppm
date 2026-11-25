export module blog_handlers:authentication_authentication_setup_get_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::authentication::authentication_setup_get::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"authentication_authentication_setup_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input&, core::workflow::WorkflowContext&) const
    {
        auto& store = s_ctx->store();
        const auto setup = is_setup(store);
        std::string data;
        if (setup) {
            data = R"({"status":true})";
        } else {
            // Ghost reads config.title / config.user_name / config.user_email
            // (authentication.js:131-136). The context-local `config` resource is the
            // stand-in; settings rows are the fallback used by the other handlers.
            const auto config = blog::api::parse_array(store.resources().list("config"));
            const auto settings = blog::api::parse_array(store.resources().list("settings"));
            const auto legacy = blog::api::parse_array(store.json_for("settings", {}));
            const auto title = lookup(
                config,
                "title",
                lookup(settings, "title", lookup(legacy, "title", "Ghost"))
            );
            const auto name = lookup(
                config,
                "user_name",
                lookup(settings, "user_name", lookup(legacy, "name", ""))
            );
            const auto email = lookup(
                config,
                "user_email",
                lookup(settings, "user_email", lookup(legacy, "email", ""))
            );
            data = std::format(
                R"({{"status":false,"title":{},"name":{},"email":{}}})",
                q(title),
                q(name),
                q(email)
            );
        }
        // output/serializers/output/authentication.js#isSetup.
        return core::workflow::StepResponse<Output>{
            {{"status", "200"}, {"body", "{\"setup\":[" + data + "]}"}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static std::string q(std::string_view value)
    {
        return core_serde::JsonCodec{}.encode(std::string{value});
    }

    static std::string unquote(std::string_view value)
    {
        if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
            return std::string{value.substr(1, value.size() - 2)};
        }
        return std::string{value};
    }

    static std::string field(const blog::api::RecordDto& r, std::string_view key)
    {
        const auto it = r.find(std::string{key});
        return it == r.end() ? std::string{} : unquote(it->second);
    }

    static bool is_setup(const blog::api::ApiDataStore& store)
    {
        for (const auto& user: blog::api::parse_array(store.json_for("users", {}))) {
            if (field(user, "status") == "inactive") {
                continue;
            }
            return true;
        }
        return false;
    }

    static std::string lookup(
        const std::vector<blog::api::RecordDto>& records,
        std::string_view key,
        std::string_view fallback
    )
    {
        for (auto it = records.rbegin(); it != records.rend(); ++it) {
            const auto k = it->find("key");
            if (k == it->end() || field(*it, "key") != key) {
                continue;
            }
            const auto v = it->find("value");
            if (v == it->end()) {
                continue;
            }
            const auto value = unquote(v->second);
            return value.empty() ? std::string{fallback} : value;
        }
        return std::string{fallback};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::authentication::authentication_setup_get::workflow
