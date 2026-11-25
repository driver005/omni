export module blog_handlers:authentication_authentication_setup_post_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import :outbox_shim;

export namespace blog::handlers::authentication::authentication_setup_post::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"authentication_authentication_setup_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        // Ghost input pipeline (authentication.js:31-86): data.setup[0].{name,
        // email,password,blogTitle,theme,accentColor,description}, status:'active'.
        const auto data = decode(unwrap_data(in.body_as_string(), "setup"));
        const auto name = plain_field(data, "name");
        const auto email = plain_field(data, "email");
        const auto password = plain_field(data, "password");
        if (name.empty()) {
            return failure("422", err("A name is required", "ValidationError", "", "name"));
        }
        if (email.empty()) {
            return failure("422", err("An email is required", "ValidationError", "", "email"));
        }
        if (password.empty()) {
            return failure("422", err("A password is required", "ValidationError", "", "password"));
        }
        const auto blog_title = plain_field(data, "blogTitle");
        const auto description = plain_field(data, "description");
        const auto accent = plain_field(data, "accentColor");
        const auto theme = plain_field(data, "theme");

        // assertSetupCompleted(false): setup must NOT already be complete.
        if (is_setup(s_ctx->store())) {
            return failure("403", err("Setup has already been completed.", "NoPermissionError"));
        }

        auto& store = s_ctx->store();
        // setupUser (setup.js:65-82): find the Owner fixture, else create it in the
        // context-local store. Ghost updates the pre-seeded inactive Owner row.
        const auto owner = find_owner(store);
        const auto user_json = std::format(
            R"({{"name":{},"email":{},"password":{},"status":"active","roles":[{{"name":"Owner"}}]}})",
            q(name),
            q(email),
            q(password)
        );
        std::string owner_id;
        if (owner) {
            owner_id = field(*owner, "id");
            (void)store.edit("users", owner_id, user_json);
        } else {
            const auto created = store.create("users", user_json);
            owner_id = created.ok() ? extract_id(created.body()) : std::string{};
        }

        // doFixtures (setup.js:162-179): update the coming-soon / about posts.
        for (const auto& record: blog::api::parse_array(store.json_for("posts", {}))) {
            const auto slug = field(record, "slug");
            if (slug != "coming-soon" && slug != "about") {
                continue;
            }
            const auto id = field(record, "id");
            if (id.empty()) {
                continue;
            }
            auto mobiledoc = std::format("This is {}, a brand new site by {}.", blog_title, name);
            (void)store.edit("posts", id, std::format(R"({{"mobiledoc":{}}})", q(mobiledoc)));
        }

        // doProductAndNewsletter (setup.js:112-160): rename the default tier and
        // newsletter to the site title.
        if (!blog_title.empty()) {
            for (const auto& tier: blog::api::parse_array(store.json_for("tiers", {}))) {
                if (field(tier, "slug") == "default-product" && !field(tier, "id").empty()) {
                    (void)store.edit(
                        "tiers",
                        field(tier, "id"),
                        std::format(R"({{"name":{}}})", q(blog_title))
                    );
                }
            }
            for (const auto& nl: blog::api::parse_array(store.json_for("newsletters", {}))) {
                if (field(nl, "slug") == "default-newsletter" && !field(nl, "id").empty()) {
                    (void)store.edit(
                        "newsletters",
                        field(nl, "id"),
                        std::format(R"({{"name":{}}})", q(blog_title))
                    );
                }
            }
        }

        // installTheme (setup.js:213-244): theme install/activate is an external
        // adapter operation; record the intent on the outbox.
        if (!theme.empty() && to_lower(theme) != "tryghost/source") {
            blog::handlers::outbox::global_outbox().emit(
                blog::handlers::outbox::Event{
                    .type = "theme.install",
                    .payload = std::format(R"({{"theme":{},"activate":true}})", q(theme)),
                    .aggregate_id = owner_id
                }
            );
        }

        // doSettings (setup.js:84-109): only title, description, accent_color.
        // Settings/config writes go through the context-local resource store.
        if (!blog_title.empty()) {
            put_setting(store, "title", trim(blog_title));
        }
        put_setting(
            store,
            "description",
            description.empty() ? std::string{"Thoughts, stories and ideas."} : trim(description)
        );
        if (!accent.empty()) {
            put_setting(store, "accent_color", accent);
        }

        // sendWelcomeEmail (setup.js:181-211) is fire-and-forget mail; record it.
        blog::handlers::outbox::global_outbox().emit(
            blog::handlers::outbox::Event{
                .type = "mail.welcome",
                .payload = std::format(R"({{"to":{},"template":"welcome"}})", q(email)),
                .aggregate_id = owner_id
            }
        );

        const auto user = store.find_by_resource("users", owner_id);
        const auto model = user ? clean_user(*user) : std::string{"{}"};
        // output/serializers/output/authentication.js#setup.
        return core::workflow::StepResponse<Output>{
            {{"status", "201"}, {"body", "{\"users\":[" + model + "]}"}}
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

    static const blog::api::RecordDto* find_owner(const blog::api::ApiDataStore& store)
    {
        static thread_local std::vector<blog::api::RecordDto> cache;
        cache = blog::api::parse_array(store.json_for("users", {{"filter", "roles:Owner"}}));
        if (cache.empty()) {
            cache = blog::api::parse_array(store.json_for("users", {}));
        }
        return cache.empty() ? nullptr : &cache.front();
    }

    static std::string extract_id(std::string_view body)
    {
        const auto rows = blog::api::parse_array(body);
        if (rows.empty()) {
            return {};
        }
        return field(rows.front(), "id");
    }

    static void
    put_setting(blog::api::ApiDataStore& store, std::string_view key, std::string_view value)
    {
        const auto record = std::format(R"({{"key":{},"value":{}}})", q(key), q(value));
        (void)store.resources().create("settings", record);
        (void)store.resources().create("config", record);
    }

    static std::string trim(std::string_view value)
    {
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
            value.remove_prefix(1);
        }
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
            value.remove_suffix(1);
        }
        return std::string{value};
    }

    static std::string to_lower(std::string_view value)
    {
        std::string out;
        for (const char c: value) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
        return out;
    }

    static std::string clean_user(std::string_view json)
    {
        static constexpr std::string_view keep[]{
            "id",
            "name",
            "slug",
            "email",
            "profile_image",
            "cover_image",
            "bio",
            "website",
            "location",
            "status",
            "roles",
            "created_at",
            "updated_at",
            "last_seen",
            "url"
        };
        const auto parsed = blog::api::JsonBody{}.parse_raw(json);
        std::string out = "{";
        bool first = true;
        for (const auto key: keep) {
            const auto it = parsed.find(std::string{key});
            if (it == parsed.end()) {
                continue;
            }
            if (!first) {
                out += ",";
            }
            first = false;
            out += q(key) + ":" + it->second;
        }
        if (!parsed.contains("roles")) {
            if (!first) {
                out += ",";
            }
            out += R"("roles":[{"name":"Owner"}])";
        }
        out += "}";
        return out;
    }

    static std::string
    err(std::string_view message,
        std::string_view type,
        std::string_view context = {},
        std::string_view property = {})
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
            R"({{"errors":[{{"id":"{}","code":null,"context":{},"details":null,"ghostErrorCode":null,"help":null,"message":{},"property":{},"type":{}}}]}})",
            uuid,
            context.empty() ? "null" : q(context),
            q(message),
            property.empty() ? "null" : q(property),
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
} // namespace blog::handlers::authentication::authentication_setup_post::workflow
