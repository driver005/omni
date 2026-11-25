export module blog_handlers:authentication_authentication_invitation_post_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::authentication::authentication_invitation_post::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"authentication_authentication_invitation_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        // Ghost input pipeline (authentication.js:207-219, input/invitations.js:14-32):
        // body data.invitation[0].{token,name,password}; permissions:false.
        const auto data = decode(unwrap_data(in.body_as_string(), "invitation"));
        const auto token = plain_field(data, "token");
        const auto password = plain_field(data, "password");
        const auto name = plain_field(data, "name");
        if (token.empty()) {
            return failure("422", err("No token provided.", "ValidationError"));
        }
        if (password.empty()) {
            return failure("422", err("No password provided.", "ValidationError"));
        }
        if (name.empty()) {
            return failure("422", err("No name provided.", "ValidationError"));
        }

        if (!is_setup(s_ctx->store())) {
            return failure(
                "403",
                err("Setup must be completed before making this request.", "NoPermissionError")
            );
        }

        // invites.service.accept (accept.ts:49-98): token is base64 encoded on the wire.
        const auto invite_token = b64decode(token);
        auto& store = s_ctx->store();
        const blog::api::RecordDto* invite = nullptr;
        for (const auto& record: blog::api::parse_array(store.resources().list("invites"))) {
            const auto tok = record.find("token");
            const auto status = record.find("status");
            if (tok == record.end() || unquote(tok->second) != invite_token) {
                continue;
            }
            if (status == record.end() || unquote(status->second) != "sent") {
                continue;
            }
            invite = &record;
            break;
        }
        if (invite == nullptr) {
            return failure("404", err("Invite not found.", "NotFoundError"));
        }
        const auto expires = to_number(field(*invite, "expires"));
        if (expires != 0 && std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::system_clock::now().time_since_epoch()
                            )
                                    .count() > static_cast<std::int64_t>(expires)) {
            return failure("404", err("Invite is expired.", "NotFoundError"));
        }
        const auto email = field(*invite, "email");
        const auto role_id = field(*invite, "role_id");

        // User.findOne({email}) duplicate guard (accept.ts:73-82).
        if (!blog::api::parse_array(
                 store.json_for("users", {{"filter", "email:" + email}})
            ).empty()) {
            return failure(
                "422",
                err("Could not create an account, email is already in use.",
                    "ValidationError",
                    "Attempting to create an account with existing email address.",
                    "Use different email address to register your account.")
            );
        }

        // accept() is a transaction: destroy the invite, then User.add with the
        // invite's role. The context-local store makes both mutations observable.
        const auto invite_id = invite->find("id");
        if (invite_id != invite->end()) {
            (void)store.resources().destroy("invites", unquote(invite_id->second));
        }
        const auto user_json = std::format(
            R"({{"email":{},"name":{},"password":{},"status":"active","roles":{}}})",
            q(email),
            q(name),
            q(password),
            roles_json(role_id)
        );
        (void)store.create("users", user_json);

        // output/serializers/output/authentication.js#acceptInvitation.
        return core::workflow::StepResponse<Output>{
            {{"status", "200"}, {"body", R"({"invitation":[{"message":"Invitation accepted."}]})"}}
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
            const auto d = decode(core_serde::JsonCodec{}.encode(value));
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
            if (name == key) {
                if (const auto* text = std::get_if<std::string>(&value.get())) {
                    return *text;
                }
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
            return true;
        }
        return false;
    }

    static std::uint64_t to_number(std::string_view text)
    {
        std::uint64_t value = 0;
        for (const char c: text) {
            if (c < '0' || c > '9') {
                return 0;
            }
            value = value * 10 + static_cast<unsigned>(c - '0');
        }
        return value;
    }

    static std::string roles_json(std::string_view role_id)
    {
        if (role_id.empty()) {
            return R"(["Owner"])";
        }
        return "[" + q(role_id) + "]";
    }

    static std::string b64decode(std::string_view text)
    {
        static constexpr std::string_view alphabet =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        unsigned buffer = 0;
        int bits = 0;
        for (const char c: text) {
            if (c == '=') {
                break;
            }
            const auto pos = alphabet.find(c);
            if (pos == std::string_view::npos) {
                return {};
            }
            buffer = (buffer << 6) | static_cast<unsigned>(pos);
            bits += 6;
            if (bits >= 8) {
                bits -= 8;
                out.push_back(static_cast<char>((buffer >> bits) & 0xFF));
            }
        }
        return out;
    }

    static std::string
    err(std::string_view message,
        std::string_view type,
        std::string_view context = {},
        std::string_view help = {})
    {
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
} // namespace blog::handlers::authentication::authentication_invitation_post::workflow
