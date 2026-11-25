export module core_auth:ghost;

import std;
import :permission;
import :role;
import :rbac;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::auth::ghost {

/** @brief Which Ghost surface a request arrived on. */
enum class ApiContext
{
    ADMIN_API,
    CONTENT_API,
    MEMBERS_APP,
    INTERNAL,
    SYSTEM,
    ANONYMOUS
};

} // namespace core::auth::ghost

// Internal helpers — not part of the exported surface.
namespace core::auth::ghost::detail {

[[nodiscard]] inline bool
in_set(std::string_view value, std::initializer_list<std::string_view> set) noexcept
{
    for (const std::string_view entry: set) {
        if (value == entry) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] inline bool privileged(ApiContext context) noexcept
{
    return context == ApiContext::INTERNAL || context == ApiContext::SYSTEM;
}

[[nodiscard]] inline bool is_admin_resource(std::string_view resource) noexcept
{
    return in_set(
        resource,
        {"users",
         "roles",
         "webhooks",
         "integrations",
         "db",
         "invites",
         "schedules",
         "exports",
         "redirects",
         "authentication",
         "slack",
         "tinybird",
         "featurebase",
         "upgrades",
         "actions"}
    );
}

[[nodiscard]] inline bool is_settings_resource(std::string_view resource) noexcept
{
    return in_set(
        resource,
        {"settings", "config", "site", "themes", "notifications", "images", "media", "files"}
    );
}

[[nodiscard]] inline bool is_member_resource(std::string_view resource) noexcept
{
    return in_set(
        resource,
        {"members",
         "tiers",
         "offers",
         "subscriptions",
         "newsletters",
         "automations",
         "emails",
         "automated_emails",
         "gift_links",
         "gifts"}
    );
}

[[nodiscard]] inline bool is_billing_resource(std::string_view resource) noexcept
{
    return in_set(resource, {"billing", "stripe", "checkout"});
}

[[nodiscard]] inline bool is_public_content(std::string_view resource) noexcept
{
    return in_set(
        resource,
        {"posts",
         "pages",
         "tags",
         "authors",
         "comments",
         "snippets",
         "labels",
         "slugs",
         "recommendations",
         "search_index"}
    );
}

/** @brief Normalizes an HTTP verb or Ghost action name to a Ghost action word. */
[[nodiscard]] inline std::string_view method_action(std::string_view method) noexcept
{
    if (method == "GET" || method == "HEAD" || method == "get" || method == "browse" ||
        method == "read") {
        return "browse";
    }
    if (method == "POST" || method == "post" || method == "add" || method == "create") {
        return "add";
    }
    if (method == "PUT" || method == "PATCH" || method == "put" || method == "patch" ||
        method == "edit" || method == "update") {
        return "edit";
    }
    if (method == "DELETE" || method == "delete" || method == "destroy") {
        return "destroy";
    }
    return method;
}

[[nodiscard]] inline bool is_read_action(std::string_view method) noexcept
{
    return method_action(method) == "browse";
}

/** @brief Coarse core_auth permission bit implied by a Ghost `resource:action` scope. */
[[nodiscard]] inline Permission coarse_permission(std::string_view permission) noexcept
{
    const std::size_t colon = permission.find(':');
    const std::string_view resource =
        colon == std::string_view::npos ? permission : permission.substr(0, colon);
    const std::string_view action =
        colon == std::string_view::npos ? std::string_view{} : permission.substr(colon + 1);

    if (is_admin_resource(resource)) {
        return Permission::ADMIN;
    }
    if (is_settings_resource(resource)) {
        return Permission::SETTINGS;
    }
    if (is_member_resource(resource)) {
        return Permission::MEMBERS;
    }
    if (is_billing_resource(resource)) {
        return Permission::BILLING;
    }

    if (action == "delete" || action == "destroy") {
        return Permission::DELETE;
    }
    if (action.empty() || action == "browse" || action == "read") {
        return Permission::READ;
    }
    return Permission::WRITE;
}

/** @brief Coarse core_auth permission bit needed to service a (docName, method) pair. */
[[nodiscard]] inline Permission
needed_permission(std::string_view docName, std::string_view method) noexcept
{
    if (is_admin_resource(docName)) {
        return Permission::ADMIN;
    }
    if (is_settings_resource(docName)) {
        return Permission::SETTINGS;
    }
    if (is_member_resource(docName)) {
        return Permission::MEMBERS;
    }
    if (is_billing_resource(docName)) {
        return Permission::BILLING;
    }

    const std::string_view action = method_action(method);
    if (action == "browse") {
        return Permission::READ;
    }
    if (action == "destroy") {
        return Permission::DELETE;
    }
    return Permission::WRITE;
}

/** @brief Wildcard-aware check of a Ghost scope set against (docName, action). */
[[nodiscard]] inline bool has_scope(
    const std::set<std::string>& scopes,
    std::string_view docName,
    std::string_view action
) noexcept
{
    if (scopes.contains("*") || scopes.contains("*:*")) {
        return true;
    }
    const std::string exact = std::string{docName} + ":" + std::string{action};
    if (scopes.contains(exact)) {
        return true;
    }
    const std::string wildcard = std::string{docName} + ":*";
    return scopes.contains(wildcard);
}

[[nodiscard]] inline bool is_unsafe_attr(std::string_view attr) noexcept
{
    return in_set(
        attr,
        {"password", "token", "gift_link_token", "secret", "api_key", "role", "staff"}
    );
}

[[nodiscard]] inline std::string quote(std::string_view text)
{
    std::string out{"\""};
    for (const char c: text) {
        switch (c) {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                out += c;
                break;
        }
    }
    out += '"';
    return out;
}

/** @brief Ghost-shaped uuid used by the error envelope `id` field. */
[[nodiscard]] inline std::string make_id()
{
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> digit{0, 15};
    const auto chunk = [&](const int count)
    {
        std::string out;
        out.reserve(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i) {
            out += "0123456789abcdef"[digit(rng)];
        }
        return out;
    };
    return std::format("{}-{}-{}-{}-{}", chunk(8), chunk(4), chunk(4), chunk(4), chunk(12));
}

} // namespace core::auth::ghost::detail

export namespace core::auth::ghost {

/** @brief The identity a permission decision is made for. */
struct Principal
{
    ApiContext context{ApiContext::ANONYMOUS};
    std::string id{};
    std::string role{};
    std::set<std::string> permissions{};
    bool staff{false};
};

/** @brief Outcome of a permission check; `error_json` is a Ghost-shaped envelope on denial. */
struct AuthResult
{
    bool allowed{false};
    int status{200};
    std::string error_json{};
};

/** @brief Ghost human-readable message for a denied (docName, method) pair. */
[[nodiscard]] inline std::string
permission_message(std::string_view docName, std::string_view method)
{
    return std::format(
        "You do not have permission to {} {}",
        detail::method_action(method),
        docName
    );
}

/** @brief Ghost-shaped NoPermissionError envelope. */
[[nodiscard]] inline std::string
no_permission_error_json(std::string_view docName, std::string_view method)
{
    return std::format(
        R"({{"errors":[{{"id":"{}","code":null,"context":null,"details":null,"ghostErrorCode":null,"help":null,"message":{},"property":null,"type":"NoPermissionError"}}]}})",
        detail::make_id(),
        detail::quote(permission_message(docName, method))
    );
}

/** @brief Ghost-shaped UnauthorizedError envelope. */
[[nodiscard]] inline std::string unauthorized_error_json(std::string_view message = "Unauthorized")
{
    return std::format(
        R"({{"errors":[{{"id":"{}","code":null,"context":null,"details":null,"ghostErrorCode":null,"help":null,"message":{},"property":null,"type":"UnauthorizedError"}}]}})",
        detail::make_id(),
        detail::quote(message)
    );
}

/**
 * @brief Ghost permission service built on top of the existing core_auth RBAC primitives.
 * A default-constructed instance is preloaded with Ghost's built-in roles.
 */
class GhostAuthorizer
{
public:
    GhostAuthorizer()
    {
        add_role("Administrator", {"*"}, true);
        add_role("Owner", {"*"}, true);
        add_role(
            "Editor",
            {"posts:*",
             "pages:*",
             "tags:*",
             "authors:*",
             "comments:*",
             "snippets:*",
             "labels:*",
             "notifications:*",
             "images:*",
             "media:*",
             "files:*",
             "slugs:*",
             "settings:browse",
             "site:browse"},
            true
        );
        add_role(
            "Writer",
            {"posts:browse",
             "posts:add",
             "posts:edit",
             "posts:delete",
             "pages:*",
             "tags:browse",
             "tags:add",
             "authors:browse",
             "images:*",
             "media:*",
             "files:*"},
            true
        );
        add_role(
            "Contributor",
            {"posts:browse", "posts:add", "posts:edit", "tags:browse", "authors:browse"},
            true
        );
        add_role("Anonymous", {});
    }

    void add_role(std::string name, std::set<std::string> permissions, bool staff = false)
    {
        std::set<Permission> bits;
        bool all = false;

        for (const std::string& permission: permissions) {
            if (permission == "*" || permission == "*:*" || permission == "all") {
                all = true;
                continue;
            }
            const std::size_t colon = permission.find(':');
            const std::string_view resource = colon == std::string::npos
                                                  ? std::string_view{permission}
                                                  : std::string_view{permission}.substr(0, colon);
            const std::string_view action = colon == std::string::npos
                                                ? std::string_view{}
                                                : std::string_view{permission}.substr(colon + 1);

            if (action == "*" || action.empty()) {
                // Whole-resource grant: expand to every bit that matters for the category.
                if (detail::is_admin_resource(resource)) {
                    bits.insert(Permission::ADMIN);
                    bits.insert(Permission::SETTINGS);
                } else if (detail::is_member_resource(resource)) {
                    bits.insert(Permission::MEMBERS);
                } else if (detail::is_billing_resource(resource)) {
                    bits.insert(Permission::BILLING);
                } else {
                    bits.insert(Permission::READ);
                    bits.insert(Permission::WRITE);
                    bits.insert(Permission::DELETE);
                }
            } else {
                bits.insert(detail::coarse_permission(permission));
            }
        }

        Role role{name};
        if (all) {
            for (const Permission bit: kAllBits) {
                role.grant(bit);
            }
        }
        for (const Permission bit: bits) {
            if (bit != Permission::NONE) {
                role.grant(bit);
            }
        }
        m_rbac.add(std::move(role));
        m_staff[std::string{name}] = staff;
        m_roles[std::string{name}] = std::move(permissions);
    }

    [[nodiscard]] AuthResult authorize(
        const Principal& principal,
        std::string_view docName,
        std::string_view method,
        std::string_view /* id */ = {}
    ) const
    {
        if (detail::privileged(principal.context)) {
            return permitted();
        }
        if (explicitly_allowed(principal, docName, method)) {
            return permitted();
        }

        // The Ghost Content API serves published content to anonymous readers.
        if (principal.context == ApiContext::CONTENT_API && detail::is_read_action(method) &&
            detail::is_public_content(docName)) {
            return permitted();
        }

        // No identity at all -> UnauthorizedError; a known but under-privileged identity
        // -> NoPermissionError.
        if (principal.context == ApiContext::ANONYMOUS && principal.id.empty() &&
            principal.role.empty() && principal.permissions.empty()) {
            return denied(unauthorized_error_json(), 401);
        }
        return denied(no_permission_error_json(docName, method), 403);
    }

    /** @brief `permissions: true` / `false` / `{docName, method}` all route through authorize. */
    [[nodiscard]] AuthResult authorize(
        const Principal& principal,
        bool enabled,
        std::string_view docName,
        std::string_view method,
        std::string_view id = {}
    ) const
    {
        return enabled ? authorize(principal, docName, method, id) : permitted();
    }

    /** @brief Whether a named role is a staff role. */
    [[nodiscard]] bool is_staff(std::string_view role) const
    {
        const auto it = m_staff.find(std::string{role});
        return it != m_staff.end() && it->second;
    }

    /** @brief Guards attributes that are unsafe for non-admin roles to set. */
    [[nodiscard]] AuthResult unsafe_attrs(
        const Principal& principal,
        std::string_view attr,
        std::string_view /* value */
    ) const
    {
        if (detail::privileged(principal.context) || !detail::is_unsafe_attr(attr)) {
            return permitted();
        }
        if (is_admin(principal)) {
            return permitted();
        }
        return denied(no_permission_error_json(attr, "edit"), 403);
    }

private:
    static constexpr std::array<Permission, 7> kAllBits{
        Permission::READ,
        Permission::WRITE,
        Permission::DELETE,
        Permission::ADMIN,
        Permission::MEMBERS,
        Permission::SETTINGS,
        Permission::BILLING
    };

    [[nodiscard]] static AuthResult permitted()
    {
        return AuthResult{.allowed = true, .status = 200, .error_json = {}};
    }

    [[nodiscard]] static AuthResult denied(std::string json, int status)
    {
        return AuthResult{.allowed = false, .status = status, .error_json = std::move(json)};
    }

    [[nodiscard]] bool is_admin(const Principal& principal) const
    {
        return principal.role == "Administrator" || principal.role == "Owner" ||
               m_rbac.allowed(principal.role, Permission::ADMIN);
    }

    [[nodiscard]] bool explicitly_allowed(
        const Principal& principal,
        std::string_view docName,
        std::string_view method
    ) const
    {
        const std::string_view action = detail::method_action(method);
        if (detail::has_scope(principal.permissions, docName, action)) {
            return true;
        }
        if (principal.role.empty()) {
            return false;
        }
        if (m_rbac.allowed(principal.role, detail::needed_permission(docName, method))) {
            return true;
        }

        const auto it = m_roles.find(principal.role);
        return it != m_roles.end() && detail::has_scope(it->second, docName, action);
    }

    core::auth::Rbac m_rbac{};
    std::map<std::string, std::set<std::string>> m_roles{};
    std::map<std::string, bool> m_staff{};
};

} // namespace core::auth::ghost

#ifdef CONGELADO_TEST
namespace core::auth::ghost::tests {
using namespace boost::ut;

suite<"core_auth_ghost"> core_auth_ghost_suite = []
{
    "default roles authorize and deny"_test = []
    {
        GhostAuthorizer auth;
        const Principal admin{.context = ApiContext::ADMIN_API, .role = "Administrator"};
        expect(auth.authorize(admin, "posts", "browse").allowed);
        expect(auth.authorize(admin, "settings", "edit").allowed);

        const Principal anon{};
        const auto denied = auth.authorize(anon, "posts", "browse");
        expect(!denied.allowed);
        expect(denied.status == 401);
        expect(denied.error_json.find("UnauthorizedError") != std::string::npos);

        const Principal contributor{.context = ApiContext::ADMIN_API, .role = "Contributor"};
        const auto forbidden = auth.authorize(contributor, "settings", "edit");
        expect(!forbidden.allowed);
        expect(forbidden.status == 403);
        expect(forbidden.error_json.find("NoPermissionError") != std::string::npos);
        expect(
            forbidden.error_json.find("You do not have permission to edit settings") !=
            std::string::npos
        );
    };

    "content API public reads and permissions toggle"_test = []
    {
        GhostAuthorizer auth;
        const Principal reader{.context = ApiContext::CONTENT_API};
        expect(auth.authorize(reader, "posts", "GET").allowed);
        expect(!auth.authorize(reader, "settings", "GET").allowed);

        const Principal admin{.context = ApiContext::ADMIN_API, .role = "Administrator"};
        expect(!auth.authorize(admin, false, "settings", "edit").allowed);
        expect(auth.authorize(admin, true, "settings", "edit").allowed);
    };
};

} // namespace core::auth::ghost::tests
#endif
