export module app_webhook:verification_input;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_auth;
import app_http;

export namespace app::handlers::verification {

/** @brief Local serializable shape of the upstream request wire.
 *
 *  `ApplepayMerchantVerificationRequest` (api_models/verifications.rs:13-19) is
 *  snake_case with a plain-string `merchant_connector_account_id` newtype. The
 *  converted app_dto instead carries a `MerchantConnectorAccountIdValue` wrapper
 *  with no serde support, so this local shape restores the exact upstream wire.
 *  The converted wrapper form is intentionally NOT accepted: upstream would
 *  reject it as a deserialization error.
 */
class VerificationRequestWire
{
public:
    void set_domain_names(std::vector<std::string> value)
    {
        domain_names = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_domain_names() const noexcept
    {
        return domain_names;
    }

    void set_merchant_connector_account_id(std::string value)
    {
        merchant_connector_account_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_connector_account_id() const noexcept
    {
        return merchant_connector_account_id;
    }

    std::vector<std::string> domain_names{};
    std::string merchant_connector_account_id{};
};

/** @brief Parse/auth/path/query support for the verification handlers.
 *
 *  Kept free of business logic: it only turns request data into the plain Input
 *  structs consumed by the domain runtime. Path/query helpers return errors instead
 *  of writing them so every handler keeps a single response-writing path.
 */
class VerificationInputSupport
{
public:
    explicit VerificationInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    /** @brief Port of the `auth_type` branches in routes/verification.rs.
     *
     *  `is_jwt_auth(headers)` selects JWTAuth (ProfileAccountWrite /
     *  MerchantAccountRead, allow_connected: true, allow_platform: false);
     *  otherwise HeaderAuth(ApiKeyAuth) on v1 or V2ApiKeyAuth on v2 is used,
     *  both with allow_connected_scope_operation=false and
     *  allow_platform_self_operation=false.
     */
    [[nodiscard]] std::optional<core::error::AppError> authorize_verification(
        const ::app::auth::AuthContext& auth,
        std::string_view operation,
        bool v2
    ) const
    {
        const bool is_jwt = !auth.token().is_empty(); // is_jwt_auth: Bearer prefix
        const auto permission = operation == "apple_pay_merchant_registration"
                                    ? ::app::auth::AppPermission::PROFILE_WRITE
                                    : ::app::auth::AppPermission::MERCHANT_READ;

        if (!is_jwt) {
            // Connected-scope operations are denied for both API-key branches.
            if (auth.claim(::app::auth::kClaimConnectedMerchantId).has_value()) {
                return core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::UNAUTHORIZED,
                    "Unauthorized"
                );
            }
            const auto scheme =
                v2 ? ::app::auth::AuthScheme::V2_API_KEY : ::app::auth::AuthScheme::API_KEY;
            const auto rule = ::app::auth::RouteAuthRuleBuilder{}
                                  .scheme(scheme)
                                  .permission(permission)
                                  .allow_connected(false)
                                  .allow_platform(false)
                                  .build();
            // The local authorizer requires a credential for API_KEY /
            // V2_API_KEY rules. Upstream authenticates these branches from the
            // api-key header, so carry that header as the local credential when
            // no Bearer token was supplied (the JWT branch always has one).
            auto effective = auth;
            if (effective.token().is_empty()) {
                if (const auto api_key = effective.claim(::app::auth::kClaimApiKey);
                    api_key && !api_key->empty()) {
                    effective.set_token(::app::auth::BearerToken{std::string{*api_key}});
                }
            }
            return m_context.authorizer().authorize_route(effective, rule);
        }

        const auto rule = ::app::auth::RouteAuthRuleBuilder{}
                              .scheme(::app::auth::AuthScheme::JWT)
                              .permission(permission)
                              .allow_connected(true)
                              .allow_platform(false)
                              .build();
        return m_context.authorizer().authorize_route(auth, rule);
    }

    [[nodiscard]] std::string authenticated_merchant_id(
        const ::app::auth::AuthContext& auth,
        interfaces::io::IRequest& req
    ) const
    {
        // For JWT allow_connected=true the effective processor account is the
        // connected merchant when acting on its behalf.
        if (!auth.merchant_id().empty()) {
            return auth.merchant_id();
        }
        if (const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId)) {
            if (!connected->empty()) {
                return std::string{*connected};
            }
        }
        return ::app::http::merchant_id_from_header{}(req);
    }

    // The merchant id is the last path segment on both the v1 and v2 routes.
    [[nodiscard]] std::expected<std::string, core::error::AppError>
    merchant_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        const std::string merchant_id =
            segments.count() > 0
                ? std::string{segments.at(segments.count() - 1).value_or(std::string_view{})}
                : std::string{};
        if (merchant_id.empty()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "ResourceIdNotFound"
            )};
        }
        return merchant_id;
    }

    // Query<ApplepayGetVerifiedDomainsParam>: both snake_case fields are
    // required by the upstream model (api_models/verifications.rs:28-34).
    [[nodiscard]] std::expected<std::pair<std::string, std::string>, core::error::AppError>
    verified_domains_query(interfaces::io::IRequest& req) const
    {
        const auto query = ::app::http::RequestQuery{}(req);
        const auto merchant_param = query.get("merchant_id");
        const auto connector_param = query.get("merchant_connector_account_id");
        if (!merchant_param || merchant_param->empty()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "merchant_id query parameter required"
            )};
        }
        if (!connector_param || connector_param->empty()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "merchant_connector_account_id query parameter required"
            )};
        }
        return std::make_pair(*merchant_param, *connector_param);
    }

private:
    ::app::context::AppContext& m_context;
};

// Each struct carries exactly what the old core call received.

struct ApplePayMerchantRegistrationInput
{
    VerificationRequestWire request{};
    std::string merchant_id{};
    std::string profile_id{};
};

struct RetrieveApplePayVerifiedDomainsInput
{
    std::string merchant_id{};
    std::string profile_id{};
    std::string merchant_connector_id{};
};

} // namespace app::handlers::verification

// Serialization declared next to the wire type it describes.
template<>
struct serde::Serializable<::app::handlers::verification::VerificationRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "domain_names",
                &::app::handlers::verification::VerificationRequestWire::get_domain_names,
                &::app::handlers::verification::VerificationRequestWire::set_domain_names,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_account_id",
                &::app::handlers::verification::VerificationRequestWire::
                    get_merchant_connector_account_id,
                &::app::handlers::verification::VerificationRequestWire::
                    set_merchant_connector_account_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "verification_request_wire";
    }
};
