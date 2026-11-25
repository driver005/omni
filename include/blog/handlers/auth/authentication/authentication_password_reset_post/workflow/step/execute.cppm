export module blog_handlers:authentication_authentication_password_reset_post_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import core_query;
import core_auth;
import blog_handlers_middleware;
import :outbox_shim;

export namespace blog::handlers::authentication::authentication_password_reset_post::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{
        "authentication_authentication_password_reset_post_execute"
    };

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        // Ghost input pipeline (authentication.js:140-157, input/password_reset.js:30-39):
        // options:['email'], body data.password_reset[0].email. core::query parses the
        // allowlisted option; the resource envelope is unwrapped from the JSON body.
        const auto query =
            core::query::parse(in.get_path(), std::array<std::string_view, 1>{"email"});
        std::string email;
        if (const auto it = query.options.find("email"); it != query.options.end()) {
            email = it->second;
        }
        const auto data = decode(unwrap_data(in.body_as_string(), "password_reset"));
        if (email.empty()) {
            email = plain_field(data, "email");
        }
        if (!valid_email(email)) {
            return failure(
                "400",
                err("The server did not receive a valid email", "BadRequestError")
            );
        }

        // assertSetupCompleted(true) (setup.js:42-63).
        if (!is_setup(s_ctx->store())) {
            return failure(
                "403",
                err("Setup must be completed before making this request.", "NoPermissionError")
            );
        }

        // permissions:true (authentication.js:147). Public route reached under the
        // trusted trusted-api context; an explicit principal is still authorizable.
        auto principal = blog::handlers::middleware::principal_from_request(in);
        if (principal.context == core::auth::ghost::ApiContext::ANONYMOUS) {
            principal.context = core::auth::ghost::ApiContext::INTERNAL;
        }
        const auto decision =
            core::auth::ghost::GhostAuthorizer{}.authorize(principal, "authentication", "add");
        if (!decision.allowed) {
            return failure(std::to_string(decision.status), decision.error_json);
        }

        auto& store = s_ctx->store();
        const auto users =
            blog::api::parse_array(store.json_for("users", {{"filter", "email:" + email}}));
        if (users.empty()) {
            return failure("404", err("User not found.", "NotFoundError"));
        }
        const auto& user = users.front();
        const auto status = field(user, "status");
        if (status == "inactive") {
            return failure("403", err("This account has been suspended.", "NoPermissionError"));
        }
        const auto user_id = field(user, "id");

        // generateToken (passwordreset.ts:74-97): derive a token bound to db_hash /
        // passwordHash with a +1 day expiry. The signed hash cannot be reproduced
        // without @tryghost/security; the context-local resource store records an
        // issued, expiring token as the stand-in (see the parity report residuals).
        const auto db_hash = setting(store, "db_hash");
        const auto expires = now_ms() + std::chrono::milliseconds{86'400'000};
        const auto payload = std::format(
            "{}:{}:{}:{}",
            user_id,
            std::chrono::duration_cast<std::chrono::milliseconds>(expires.time_since_epoch())
                .count(),
            email,
            db_hash
        );
        const auto token = b64encode(payload);
        const auto token_json = std::format(
            R"({{"token":{},"email":{},"user_id":{},"expires":{}}})",
            q(token),
            q(email),
            q(user_id),
            std::chrono::duration_cast<std::chrono::milliseconds>(expires.time_since_epoch())
                .count()
        );
        (void)store.resources().create("password_reset_tokens", token_json);

        // sendResetNotification (passwordreset.ts:211-242): mail transport is not a
        // C++ primitive here; record the outbound send on the shared outbox.
        blog::handlers::outbox::global_outbox().emit(
            blog::handlers::outbox::Event{
                .type = "mail.reset_password",
                .payload = std::format(
                    R"({{"to":{},"token":{},"template":"reset-password"}})",
                    q(email),
                    q(token)
                ),
                .aggregate_id = user_id
            }
        );

        // output/serializers/output/authentication.js#generateResetToken.
        return core::workflow::StepResponse<Output>{
            {{"status", "200"},
             {"body",
              R"({"password_reset":[{"message":"Check your email for further instructions."}]})"}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;

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

    static std::optional<Object> decode(std::string_view text)
    {
        const auto value = core_serde::JsonCodec{}.decode<serde::Value>(text);
        if (!value) {
            return std::nullopt;
        }
        const auto* object = std::get_if<Object>(&value->get());
        return object == nullptr ? std::nullopt : std::optional<Object>{*object};
    }

    static std::string unwrap_data(std::string_view body, std::string_view resource)
    {
        const auto root = decode(body);
        if (!root) {
            return "{}";
        }
        for (const auto& [key, value]: *root) {
            if (key != "data") {
                continue;
            }
            const auto data = core_serde::JsonCodec{}.encode(value);
            const auto d = decode(data);
            if (!d) {
                return "{}";
            }
            for (const auto& [rkey, rvalue]: *d) {
                if (rkey != resource) {
                    continue;
                }
                const auto rows = blog::api::parse_array(core_serde::JsonCodec{}.encode(rvalue));
                if (!rows.empty()) {
                    return blog::api::record_to_json(rows.front());
                }
            }
        }
        return "{}";
    }

    static std::string plain_field(const std::optional<Object>& object, std::string_view key)
    {
        if (!object) {
            return {};
        }
        for (const auto& [name, value]: *object) {
            if (name != key) {
                continue;
            }
            if (const auto* text = std::get_if<std::string>(&value.get())) {
                return *text;
            }
        }
        return {};
    }

    static std::string setting(const blog::api::ApiDataStore& store, std::string_view key)
    {
        for (const auto& record: blog::api::parse_array(store.json_for("settings", {}))) {
            const auto k = record.find("key");
            const auto v = record.find("value");
            if (k != record.end() && field(record, "key") == key && v != record.end()) {
                return unquote(v->second);
            }
        }
        return {};
    }

    static bool is_setup(const blog::api::ApiDataStore& store)
    {
        for (const auto& user: blog::api::parse_array(store.json_for("users", {}))) {
            if (field(user, "status") == "inactive") {
                continue;
            }
            return true; // fixture Owner present -> User.isSetup() true
        }
        return false;
    }

    static bool valid_email(std::string_view value)
    {
        const auto at = value.find('@');
        if (at == std::string_view::npos || at == 0 || at + 1 == value.size()) {
            return false;
        }
        if (value.find(' ', 0) != std::string_view::npos) {
            return false;
        }
        return value.find('.', at + 1) != std::string_view::npos;
    }

    static std::chrono::system_clock::time_point now_ms()
    {
        return std::chrono::system_clock::now();
    }

    static std::string b64encode(std::string_view text)
    {
        static constexpr char alphabet[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        std::size_t i = 0;
        for (; i + 3 <= text.size(); i += 3) {
            const unsigned n = (static_cast<unsigned char>(text[i]) << 16) |
                               (static_cast<unsigned char>(text[i + 1]) << 8) |
                               static_cast<unsigned char>(text[i + 2]);
            out += alphabet[(n >> 18) & 63];
            out += alphabet[(n >> 12) & 63];
            out += alphabet[(n >> 6) & 63];
            out += alphabet[n & 63];
        }
        const auto left = text.size() - i;
        if (left == 1) {
            const unsigned a = static_cast<unsigned char>(text[i]);
            out += alphabet[a >> 2];
            out += alphabet[(a & 3) << 4];
            out += "==";
        } else if (left == 2) {
            const unsigned n = (static_cast<unsigned char>(text[i]) << 8) |
                               static_cast<unsigned char>(text[i + 1]);
            out += alphabet[(n >> 10) & 63];
            out += alphabet[(n >> 4) & 63];
            out += alphabet[(n & 15) << 2];
            out += '=';
        }
        return out;
    }

    static std::string
    err(std::string_view message,
        std::string_view type,
        std::string_view context = {},
        std::string_view help = {})
    {
        static std::uint64_t counter = 0;
        static thread_local std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<int> digit{0, 15};
        const auto chunk = [&](int n)
        {
            std::string s;
            for (int i = 0; i < n; ++i) {
                s += "0123456789abcdef"[digit(rng)];
            }
            return s;
        };
        const auto uuid =
            std::format("{}-{}-{}-{}-{}", chunk(8), chunk(4), chunk(4), chunk(4), chunk(12));
        return std::format(
            R"({{"errors":[{{"id":"{}","code":null,"context":{},"details":null,"ghostErrorCode":null,"help":{},"message":{},"property":null,"type":{}}}]}})",
            uuid,
            context.empty() ? "null" : q(context),
            help.empty() ? "null" : q(help),
            q(message),
            q(type)
        );
    }

    static core::workflow::StepResponse<Output>
    failure(std::string_view status, std::string_view body)
    {
        return core::workflow::StepResponse<Output>{
            {{"status", std::string{status}}, {"body", std::string{body}}}
        };
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::authentication::authentication_password_reset_post::workflow
