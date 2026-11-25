export module blog_handlers:users_users_id_token_get_step;
import :domain_all;
import :users_dto_subresources;
import std;
import core_workflow;
import interfaces;
import core_auth;
import :outbox_shim;
import blog_handlers_middleware;

export namespace blog::handlers::users::users_id_token_get::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"users_users_id_token_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const auto id = target_id(in.get_path());
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}.validation("Validation error", "An id is required", "id")}}
            };
        }

        // Ghost `permissionOnlySelf`: `me` resolves to the authenticated subject,
        // then the target must equal that subject. The authorizer supplies Ghost's
        // Unauthorized/NoPermission envelope; bearer subjects map to the seeded
        // admin user in this mock (no claims decoder exists in the store layer).
        const auto principal = make_principal(in);
        const auto subject = principal.id;
        const std::string target = (id == "me") ? subject : id;
        const auto authorizer = core::auth::ghost::GhostAuthorizer{};
        const auto decision = authorizer.authorize(principal, "users", "browse", target);
        if (!decision.allowed) {
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(decision.status)}, {"body", decision.error_json}}
            };
        }
        if (subject.empty() || target != subject) {
            return core::workflow::StepResponse<Output>{
                {{"status", "403"},
                 {"body",
                  blog::api::ApiError{}.validation(
                      "You do not have permission to perform this action",
                      "Forbidden",
                      "",
                      "NoPermissionError"
                  )}}
            };
        }

        auto& rs = ctx.store().resources();
        const std::string filter = "user_id:" + target + "+type:admin";
        auto items = blog::api::parse_array(ctx.store().json_for("api_keys", {{"filter", filter}}));
        if (items.empty()) {
            rs.create(
                "api_keys",
                std::format(
                    R"({{"user_id":{},"type":"admin","secret":{}}})",
                    blog::api::JsonBody{}.quote(target),
                    blog::api::JsonBody{}.quote(new_secret())
                )
            );
            items = blog::api::parse_array(ctx.store().json_for("api_keys", {{"filter", filter}}));
            blog::handlers::outbox::global_outbox().emit(
                blog::handlers::outbox::Event{
                    .type = "api_key.created",
                    .payload = std::format(
                        R"({{"user_id":{},"type":"admin"}})",
                        blog::api::JsonBody{}.quote(target)
                    ),
                    .aggregate_id = target
                }
            );
        }
        if (items.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "500"},
                 {"body",
                  blog::api::ApiError{}
                      .json("Could not fetch a staff token.", "Internal Server Error")}}
            };
        }
        return core::workflow::StepResponse<Output>{
            {{"status", "200"}, {"body", api_key_envelope(items.front())}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static core::auth::ghost::Principal make_principal(interfaces::io::IRequest& in)
    {
        auto principal = blog::handlers::middleware::principal_from_request(in);
        auto subject = std::string{in.find_header("x-blog-user-id")};
        if (subject.empty() && !in.get_authorization().empty()) {
            subject = "1";
        }
        principal.id = std::move(subject);
        return principal;
    }

    static std::string new_secret()
    {
        std::random_device rd;
        std::mt19937_64 rng{
            rd() ^
            static_cast<std::uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count())
        };
        std::ostringstream out;
        out << std::hex << std::setw(32) << std::setfill('0') << rng();
        return "sk_" + out.str();
    }

    static std::string api_key_envelope(const blog::api::RecordDto& key)
    {
        auto out = key;
        const auto defaults = std::array<std::pair<std::string_view, std::string_view>, 6>{
            {{"integration_id", "null"},
             {"role_id", "null"},
             {"last_seen_at", "null"},
             {"last_seen_version", "null"},
             {"created_at", R"("2026-01-01T00:00:00.000Z")"},
             {"updated_at", R"("2026-01-01T00:00:00.000Z")"}}
        };
        for (const auto& [name, value]: defaults) {
            if (!out.contains(std::string{name})) {
                out[std::string{name}] = std::string{value};
            }
        }
        return "{\"apiKey\":" + blog::api::record_to_json(out) + "}";
    }

    static std::string target_id(std::string_view path)
    {
        std::map<std::string, std::string> params;
        const auto q = path.find('?');
        if (q != std::string_view::npos) {
            for (auto pair: std::views::split(path.substr(q + 1), '&')) {
                const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
                const auto eq = text.find('=');
                if (eq != std::string::npos) {
                    params[text.substr(0, eq)] = text.substr(eq + 1);
                }
            }
            path = path.substr(0, q);
        }
        if (const auto it = params.find("id"); it != params.end() && !it->second.empty()) {
            return it->second;
        }
        while (path.size() > 1 && path.back() == '/') {
            path.remove_suffix(1);
        }
        const auto last = path.find_last_of('/');
        if (last == std::string_view::npos) {
            return {};
        }
        const auto before = path.substr(0, last);
        const auto prev = before.find_last_of('/');
        return std::string{prev == std::string_view::npos ? before : before.substr(prev + 1)};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::users::users_id_token_get::workflow
