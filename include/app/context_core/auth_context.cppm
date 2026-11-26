module;
export module app_context_core:auth_context;

import std;
import interfaces;
import :auth_token;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::auth {

// Canonical claim keys shared by the builder, the authorizer and the route registry.
inline constexpr std::string_view kClaimApiKey{"x-api-key"};
inline constexpr std::string_view kClaimInternalApiKey{"x-internal-api-key"};
inline constexpr std::string_view kClaimSdkKey{"x-sdk-key"};
inline constexpr std::string_view kClaimPublishableKey{"x-publishable-key"};
inline constexpr std::string_view kClaimClientSecret{"x-client-secret"};
inline constexpr std::string_view kClaimConnectedMerchantId{"x-connected-merchant-id"};

/** @brief Authentication state extracted from a request's headers. */
class AuthContext
{
public:
    AuthContext() = default;

    AuthContext(
        BearerToken token,
        std::string merchant_id,
        std::string org_id,
        std::string profile_id,
        std::map<std::string, std::string> claims
    ) :
        m_token{std::move(token)},
        m_merchant_id{std::move(merchant_id)},
        m_org_id{std::move(org_id)},
        m_profile_id{std::move(profile_id)},
        m_claims{std::move(claims)}
    {
    }

    [[nodiscard]] const BearerToken& token() const noexcept
    {
        return m_token;
    }

    [[nodiscard]] const std::string& merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    [[nodiscard]] const std::string& org_id() const noexcept
    {
        return m_org_id;
    }

    [[nodiscard]] const std::string& profile_id() const noexcept
    {
        return m_profile_id;
    }

    [[nodiscard]] const std::map<std::string, std::string>& claims() const noexcept
    {
        return m_claims;
    }

    [[nodiscard]] std::optional<std::string_view> claim(std::string_view key) const
    {
        const auto it = m_claims.find(std::string{key});
        if (it == m_claims.end()) {
            return std::nullopt;
        }
        return std::optional<std::string_view>{it->second};
    }

    void set_token(BearerToken value)
    {
        m_token = std::move(value);
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    void set_org_id(std::string value)
    {
        m_org_id = std::move(value);
    }

    void set_profile_id(std::string value)
    {
        m_profile_id = std::move(value);
    }

    void add_claim(std::string key, std::string value)
    {
        m_claims.insert_or_assign(std::move(key), std::move(value));
    }

private:
    BearerToken m_token{};
    std::string m_merchant_id{};
    std::string m_org_id{};
    std::string m_profile_id{};
    std::map<std::string, std::string> m_claims{};
};

/** @brief Instance functor deriving an AuthContext from an IRequest. */
class AuthContextBuilder
{
public:
    [[nodiscard]] AuthContext operator()(interfaces::io::IRequest& request) const
    {
        BearerToken token = TokenParser{}(request);
        std::string merchant_id{};
        std::string org_id{};
        std::string profile_id{};
        std::map<std::string, std::string> claims{};

        const auto read = [&request](std::string_view name, std::string_view alternate)
        {
            std::string_view value = request.find_header(name);
            if (value.empty() && !alternate.empty()) {
                value = request.find_header(alternate);
            }
            return value;
        };

        const auto capture =
            [&claims, &read](std::string_view name, std::string_view alternate, std::string& target)
        {
            const std::string_view value = read(name, alternate);
            if (value.empty()) {
                return;
            }
            target = std::string{value};
            claims.insert_or_assign(std::string{name}, std::string{value});
        };

        const auto capture_named =
            [&claims,
             &read](std::string_view canonical, std::string_view name, std::string_view alternate)
        {
            const std::string_view value = read(name, alternate);
            if (!value.empty()) {
                claims.insert_or_assign(std::string{canonical}, std::string{value});
            }
        };

        capture("x-merchant-id", "X-Merchant-Id", merchant_id);
        capture("x-organization-id", "X-Organization-Id", org_id);
        capture("x-profile-id", "X-Profile-Id", profile_id);
        capture_named(
            kClaimConnectedMerchantId,
            "x-connected-merchant-id",
            "X-Connected-Merchant-Id"
        );
        capture_named(kClaimPublishableKey, "x-publishable-key", "X-Publishable-Key");
        capture_named(kClaimClientSecret, "x-client-secret", "X-Client-Secret");
        capture_named(kClaimApiKey, "x-api-key", "X-API-KEY");
        capture_named(kClaimApiKey, "api-key", "API-KEY");
        capture_named(kClaimApiKey, "apikey", "ApiKey");
        capture_named(kClaimInternalApiKey, "x-internal-api-key", "X-Internal-Api-Key");

        // Upstream SdkAuthorizationAuth reads key=value pairs from the Authorization header
        // (services/authentication.rs:3103-3120). TokenParser already handles "Bearer ".
        const std::string_view authorization = request.find_header("authorization");
        constexpr std::string_view bearer_prefix{"Bearer "};
        const bool is_bearer = authorization.size() > bearer_prefix.size() &&
                               std::equal(
                                   bearer_prefix.begin(),
                                   bearer_prefix.end(),
                                   authorization.begin(),
                                   [](char lhs, char rhs)
                                   {
                                       return std::tolower(static_cast<unsigned char>(lhs)) ==
                                              std::tolower(static_cast<unsigned char>(rhs));
                                   }
                               );
        if (!authorization.empty() && !is_bearer) {
            const auto trim = [](std::string_view text)
            {
                while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) {
                    text.remove_prefix(1);
                }
                while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) {
                    text.remove_suffix(1);
                }
                return text;
            };
            std::string_view rest = authorization;
            while (!rest.empty()) {
                const std::size_t comma = rest.find(',');
                const std::string_view pair = trim(rest.substr(0, comma));
                const std::size_t equals = pair.find('=');
                if (equals != std::string_view::npos) {
                    const std::string_view key = trim(pair.substr(0, equals));
                    const std::string_view value = trim(pair.substr(equals + 1));
                    if (!value.empty()) {
                        if (key == "publishable-key") {
                            claims.insert_or_assign(
                                std::string{kClaimPublishableKey},
                                std::string{value}
                            );
                        } else if (key == "client-secret") {
                            claims.insert_or_assign(
                                std::string{kClaimClientSecret},
                                std::string{value}
                            );
                        } else if (key == "api-key") {
                            claims.insert_or_assign(std::string{kClaimApiKey}, std::string{value});
                        } else if (key == "sdk-key" || key == "token") {
                            claims.insert_or_assign(std::string{kClaimSdkKey}, std::string{value});
                        }
                    }
                }
                if (comma == std::string_view::npos) {
                    break;
                }
                rest = rest.substr(comma + 1);
            }
        }

        return AuthContext{
            std::move(token),
            std::move(merchant_id),
            std::move(org_id),
            std::move(profile_id),
            std::move(claims),
        };
    }

    [[nodiscard]] AuthContext from_request(interfaces::io::IRequest& request) const
    {
        return this->operator()(request);
    }

    /**
     * @brief Header extraction plus body-derived credentials used by SDK/client-secret flows.
     * @note Upstream overwrites payload.client_secret with the authenticated value
     * (routes/pm_auth.rs:62-64, :119-121); callers pass the already-read request body here.
     */
    [[nodiscard]] AuthContext
    operator()(interfaces::io::IRequest& request, std::string_view body) const
    {
        AuthContext context = this->operator()(request);
        const auto json_string = [body](std::string_view key) -> std::optional<std::string>
        {
            const std::string pattern{std::format("\"{}\"", key)};
            const std::size_t key_pos = body.find(pattern);
            if (key_pos == std::string_view::npos) {
                return std::nullopt;
            }
            std::size_t cursor = key_pos + pattern.size();
            while (cursor < body.size() && std::isspace(static_cast<unsigned char>(body[cursor]))) {
                ++cursor;
            }
            if (cursor >= body.size() || body[cursor] != ':') {
                return std::nullopt;
            }
            ++cursor;
            while (cursor < body.size() && std::isspace(static_cast<unsigned char>(body[cursor]))) {
                ++cursor;
            }
            if (cursor >= body.size() || body[cursor] != '"') {
                return std::nullopt;
            }
            ++cursor;
            const std::size_t end = body.find('"', cursor);
            if (end == std::string_view::npos) {
                return std::nullopt;
            }
            return std::string{body.substr(cursor, end - cursor)};
        };

        if (const auto value = json_string("api_key")) {
            context.add_claim(std::string{kClaimApiKey}, *value);
        } else if (const auto alt = json_string("apiKey")) {
            context.add_claim(std::string{kClaimApiKey}, *alt);
        }
        if (const auto value = json_string("client_secret")) {
            context.add_claim(std::string{kClaimClientSecret}, *value);
        }
        if (const auto value = json_string("publishable_key")) {
            context.add_claim(std::string{kClaimPublishableKey}, *value);
        }
        if (const auto value = json_string("token")) {
            context.add_claim("token", *value);
        }
        return context;
    }

    [[nodiscard]] AuthContext
    from_request_with_body(interfaces::io::IRequest& request, std::string_view body) const
    {
        return this->operator()(request, body);
    }
};

} // namespace app::auth

#ifdef CONGELADO_TEST
namespace app::auth::tests {
using namespace boost::ut;

suite<"app_auth_context"> context_suite = []
{
    "builder extracts bearer token and identity headers into an AuthContext"_test = []
    {
        struct FakeRequest : interfaces::io::IRequest
        {
            explicit FakeRequest(std::map<std::string, std::string> headers) :
                m_headers{std::move(headers)}
            {
            }

            [[nodiscard]] std::string_view find_header(
                std::variant<std::string_view, interfaces::io::types::Token> name
            ) const noexcept override
            {
                const auto* raw = std::get_if<std::string_view>(&name);
                if (raw == nullptr) {
                    return {};
                }
                const auto it = m_headers.find(std::string{*raw});
                return it == m_headers.end() ? std::string_view{} : std::string_view{it->second};
            }

            std::map<std::string, std::string> m_headers{};
        };

        FakeRequest request{{
            {"authorization", "Bearer abc.def"},
            {"x-merchant-id", "merchant_1"},
            {"x-organization-id", "org_1"},
            {"x-profile-id", "profile_1"},
            {"x-connected-merchant-id", "connected_1"},
            {"x-publishable-key", "pk_1"},
            {"x-client-secret", "cs_1"},
        }};

        const AuthContext context = AuthContextBuilder{}(request);
        expect(context.token().value() == "abc.def");
        expect(context.merchant_id() == "merchant_1");
        expect(context.org_id() == "org_1");
        expect(context.profile_id() == "profile_1");
        expect(context.claim("x-connected-merchant-id") == "connected_1");
        expect(context.claim("x-publishable-key") == "pk_1");
        expect(context.claim("x-client-secret") == "cs_1");
        expect(context.claims().size() == 6);
    };

    "empty headers yield an empty context"_test = []
    {
        struct EmptyRequest : interfaces::io::IRequest
        {
            [[nodiscard]] std::string_view find_header(
                std::variant<std::string_view, interfaces::io::types::Token>
            ) const noexcept override
            {
                return {};
            }
        };

        EmptyRequest request;
        const AuthContext context = AuthContextBuilder{}.from_request(request);
        expect(context.token().is_empty());
        expect(context.merchant_id().empty());
        expect(context.org_id().empty());
        expect(context.profile_id().empty());
        expect(!context.claim("x-merchant-id").has_value());
    };

    "builder accepts upstream title-case header names"_test = []
    {
        struct TitleRequest : interfaces::io::IRequest
        {
            [[nodiscard]] std::string_view find_header(
                std::variant<std::string_view, interfaces::io::types::Token> name
            ) const noexcept override
            {
                const auto* raw = std::get_if<std::string_view>(&name);
                if (raw == nullptr) {
                    return {};
                }
                if (*raw == "X-Merchant-Id") {
                    return "merchant_2";
                }
                return {};
            }
        };

        TitleRequest request;
        const AuthContext context = AuthContextBuilder{}(request);
        expect(context.merchant_id() == "merchant_2");
        expect(context.claim("x-merchant-id") == "merchant_2");
    };

    "builder extracts API key, internal key and SDK authorization headers"_test = []
    {
        struct AuthHeadersRequest : interfaces::io::IRequest
        {
            explicit AuthHeadersRequest(std::map<std::string, std::string> headers) :
                m_headers{std::move(headers)}
            {
            }

            [[nodiscard]] std::string_view find_header(
                std::variant<std::string_view, interfaces::io::types::Token> name
            ) const noexcept override
            {
                const auto* raw = std::get_if<std::string_view>(&name);
                if (raw == nullptr) {
                    return {};
                }
                const auto it = m_headers.find(std::string{*raw});
                return it == m_headers.end() ? std::string_view{} : std::string_view{it->second};
            }

            std::map<std::string, std::string> m_headers{};
        };

        AuthHeadersRequest api_key_request{{
            {"x-api-key", "key_1"},
            {"x-internal-api-key", "internal_1"},
        }};
        const AuthContext api_key = AuthContextBuilder{}(api_key_request);
        expect(api_key.claim(kClaimApiKey) == "key_1");
        expect(api_key.claim(kClaimInternalApiKey) == "internal_1");

        AuthHeadersRequest sdk_request{{
            {"authorization", "publishable-key=pk_2, client-secret=cs_2, sdk-key=sdk_2"},
        }};
        const AuthContext sdk = AuthContextBuilder{}(sdk_request);
        expect(sdk.token().is_empty());
        expect(sdk.claim(kClaimPublishableKey) == "pk_2");
        expect(sdk.claim(kClaimClientSecret) == "cs_2");
        expect(sdk.claim(kClaimSdkKey) == "sdk_2");
    };

    "body overload extracts credentials without reading IRequest body"_test = []
    {
        struct EmptyRequest : interfaces::io::IRequest
        {
            [[nodiscard]] std::string_view find_header(
                std::variant<std::string_view, interfaces::io::types::Token>
            ) const noexcept override
            {
                return {};
            }
        };

        EmptyRequest request;
        const AuthContext context = AuthContextBuilder{}.from_request_with_body(
            request,
            R"({"client_secret":"cs_body","publishable_key":"pk_body","api_key":"key_body"})"
        );
        expect(context.claim(kClaimClientSecret) == "cs_body");
        expect(context.claim(kClaimPublishableKey) == "pk_body");
        expect(context.claim(kClaimApiKey) == "key_body");
    };
};

} // namespace app::auth::tests
#endif
