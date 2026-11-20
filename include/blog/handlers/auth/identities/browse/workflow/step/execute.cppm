export module blog_handlers:identities_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import blog_handlers_middleware;

export namespace blog::handlers::identities::browse::workflow {

// GET /ghost/api/admin/identities — Ghost identities.js:11-28.
// IdentityTokenService.instance.getTokenForUser(email, role) resolves the
// authenticated user's email plus its first DB-loaded role and returns a signed
// RS256 JWT. This folder has no User model, role relation, signing keys or JWT
// signer, so the claims are reconstructed from the authenticated principal and
// serde-encoded instead of cryptographically signed (documented residual gap).
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"identities_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        // Ghost reads frame.user.get('email') and the first relation role name
        // (identities.js:16-17,23). No User store exists here: the identity comes
        // from the authenticated principal (x-blog-role, mapped from the admin
        // token by IdentitiesRoutes) with the x-blog-identity-* overrides.
        const std::string email = header_or(in, "x-blog-identity-email",
            std::string{blog::handlers::middleware::token(in)});
        const std::string role = header_or(in, "x-blog-identity-role",
            std::string{in.find_header(blog::handlers::middleware::kGhostRoleHeader)});

        // Ghost claims: {sub: email, role?} — role omitted when it is not a
        // string (identity-token-service.ts:16-22).
        serde::Value::Object claims;
        claims["sub"] = serde::Value{email};
        if (!role.empty()) {
            claims["role"] = serde::Value{role};
        }
        // No RS256 signing primitive in this folder: this is serde-encoded, NOT
        // a signed JWT (no issuer / 5m expiry / kid / signature).
        const std::string token = core_serde::JsonCodec{}.encode(serde::Value{std::move(claims)});

        // Ghost default.all serializer: { identities: [ mapResponse(...) ] },
        // no meta (output/default.js:32-36). No identities mapper exists, so the
        // { token } response object is passed through unchanged.
        serde::Value::Object identity;
        identity["token"] = serde::Value{token};
        serde::Value::Array identities;
        identities.push_back(serde::Value{std::move(identity)});
        serde::Value::Object root;
        root["identities"] = serde::Value{std::move(identities)};
        return core::workflow::StepResponse<Output>{
            core_serde::JsonCodec{}.encode(serde::Value{std::move(root)})};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    // Assigned by BrowseHandler; no ResourceStore access in this route, but the
    // pointer is retained so the step keeps the standard handler/step contract.
    static blog::BlogContext* s_ctx;

private:
    static std::string header_or(interfaces::io::IRequest& in, std::string_view name,
                                 const std::string& fallback)
    {
        const auto value = in.find_header(name);
        return value.empty() ? fallback : std::string{value};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::identities::browse::workflow
