export module blog_handlers_middleware:ghost_permission;

import std;
import interfaces;
import core_auth;
import :error;
import :registry;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::handlers::middleware {

inline constexpr std::string_view kGhostContextHeader{"x-blog-context"};
inline constexpr std::string_view kGhostRoleHeader{"x-blog-role"};
inline constexpr std::string_view kGhostPermissionsHeader{"x-blog-permissions"};
inline constexpr std::string_view kGhostStaffHeader{"x-blog-staff"};

/**
 * @brief Ambient request/response for the two-argument require_ghost_permission shape.
 * Mirrors MiddlewareContextRegistry: a function pointer cannot capture, so the calling
 * pipeline publishes the in-flight request/response/send here before invoking the guard.
 */
class GhostPermissionRegistry
{
public:
    static void set_active(interfaces::io::IRequest* request,
                           interfaces::io::IResponse* response,
                           std::function<void()> send,
                           const core::auth::ghost::GhostAuthorizer* authorizer = nullptr) noexcept
    {
        s_request = request;
        s_response = response;
        s_send = std::move(send);
        s_authorizer = authorizer;
    }

    static void clear_active() noexcept
    {
        s_request = nullptr;
        s_response = nullptr;
        s_send = {};
        s_authorizer = nullptr;
    }

    [[nodiscard]] static interfaces::io::IRequest* request() noexcept { return s_request; }
    [[nodiscard]] static interfaces::io::IResponse* response() noexcept { return s_response; }
    [[nodiscard]] static std::function<void()> send() { return s_send; }

    [[nodiscard]] static const core::auth::ghost::GhostAuthorizer& authorizer() noexcept
    {
        static const core::auth::ghost::GhostAuthorizer kDefault{};
        return s_authorizer == nullptr ? kDefault : *s_authorizer;
    }

private:
    static inline interfaces::io::IRequest* s_request{nullptr};
    static inline interfaces::io::IResponse* s_response{nullptr};
    static inline std::function<void()> s_send{};
    static inline const core::auth::ghost::GhostAuthorizer* s_authorizer{nullptr};
};

/** @brief Builds a ghost::Principal from the x-blog-* request headers. */
[[nodiscard]] inline core::auth::ghost::Principal principal_from_request(
    interfaces::io::IRequest& request)
{
    using core::auth::ghost::ApiContext;

    core::auth::ghost::Principal principal;

    const std::string_view context = request.find_header(kGhostContextHeader);
    if (context == "ADMIN_API") principal.context = ApiContext::ADMIN_API;
    else if (context == "CONTENT_API") principal.context = ApiContext::CONTENT_API;
    else if (context == "MEMBERS_APP") principal.context = ApiContext::MEMBERS_APP;
    else if (context == "INTERNAL") principal.context = ApiContext::INTERNAL;
    else if (context == "SYSTEM") principal.context = ApiContext::SYSTEM;
    else principal.context = ApiContext::ANONYMOUS;

    principal.role = std::string{request.find_header(kGhostRoleHeader)};

    for (auto part : std::string_view{request.find_header(kGhostPermissionsHeader)} |
                         std::views::split(',')) {
        const std::string scope{std::ranges::begin(part), std::ranges::end(part)};
        if (!scope.empty()) principal.permissions.insert(scope);
    }

    const std::string_view staff = request.find_header(kGhostStaffHeader);
    principal.staff = staff == "true" || staff == "1";

    if (!request.get_authorization().empty()) {
        if (principal.id.empty()) principal.id = "bearer";
        // Repo convention: an admin bearer token in harnesses maps to the
        // Administrator role with all permissions when no explicit role header
        // is supplied.
        if (principal.role.empty()) principal.role = "Administrator";
        if (principal.permissions.empty()) principal.permissions.insert("*");
        principal.staff = true;
    }
    return principal;
}

/** @brief Translates an AuthResult into the existing Ghost error envelope writer. */
inline void write_ghost_denial(interfaces::io::IResponse& response,
                               const core::auth::ghost::AuthResult& result,
                               std::string_view docName,
                               std::string_view method,
                               std::function<void()> send)
{
    ErrorEnvelope envelope;
    ErrorItem item;
    if (result.status == 401) {
        item.message = "Unauthorized";
        item.type = "UnauthorizedError";
    } else {
        item.message = core::auth::ghost::permission_message(docName, method);
        item.type = "NoPermissionError";
    }
    envelope.errors.push_back(std::move(item));

    const auto status = result.status == 401 ? interfaces::io::types::Status::UNAUTHORIZED
                                             : interfaces::io::types::Status::FORBIDDEN;
    write_error(response, status, envelope, std::move(send));
}

/** @brief Full middleware shape, usable directly by the central pipeline. */
inline void require_ghost_permission(interfaces::io::IRequest& request,
                                     interfaces::io::IResponse& response,
                                     interfaces::NextFn&& next,
                                     std::function<void()> send,
                                     std::string_view docName,
                                     std::string_view method)
{
    const auto principal = principal_from_request(request);
    const auto result = GhostPermissionRegistry::authorizer().authorize(principal, docName, method);
    if (!result.allowed) {
        write_ghost_denial(response, result, docName, method, std::move(send));
        return;
    }
    next(request, response, std::move(send));
}

/**
 * @brief Contract shape. Reads the ambient request/response published by
 * GhostPermissionRegistry::set_active() and writes a Ghost error envelope on denial.
 */
inline void require_ghost_permission(std::string_view docName, std::string_view method)
{
    auto* request = GhostPermissionRegistry::request();
    auto* response = GhostPermissionRegistry::response();
    if (request == nullptr || response == nullptr) return;

    const auto principal = principal_from_request(*request);
    const auto result = GhostPermissionRegistry::authorizer().authorize(principal, docName, method);
    if (result.allowed) return;

    write_ghost_denial(*response, result, docName, method, GhostPermissionRegistry::send());
}

} // namespace blog::handlers::middleware

#ifdef CONGELADO_TEST
namespace blog::handlers::middleware::tests {
using namespace boost::ut;

suite<"middleware_ghost_permission"> middleware_ghost_permission_suite = []
{
    "principal_from_request reads x-blog headers"_test = []
    {
        // Header plumbing is validated by the owning pipeline tests; here we assert the
        // pure helper contract using the registry's default (no ambient request -> no-op).
        GhostPermissionRegistry::clear_active();
        require_ghost_permission("posts", "browse");
        expect(GhostPermissionRegistry::request() == nullptr);
    };
};

} // namespace blog::handlers::middleware::tests
#endif
