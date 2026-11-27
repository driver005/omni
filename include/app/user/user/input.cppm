export module app_user:user_input;

import std;
import interfaces;
import core_behavior;
import core_error;
import app_context;
import app_dto;
import app_model;
import app_auth;
import app_http;

export namespace app::handlers::user {

// Plain request-derived values consumed by the route runtimes. No HTTP response types.
struct UserRequestData
{
    std::string body{};
    std::string merchant_id{};
    std::string org_id{};
    std::string client_ip{};
    ::app::auth::BearerToken token{};
    ::app::auth::AuthContext auth{};
    ::app::http::QueryParams query{};
    std::string path{};
};

/** @brief Parse/auth/header/path/query support shared by all user handlers. */
class UserInputSupport
{
public:
    explicit UserInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // Auth denial carries the upstream wire code in kind_name and the exact message;
    // the handler renders the nested ApiErrorResponse body from those fields.
    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req, std::string_view operation) const
    {
        (void)operation;
        const auto auth_ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context.auth_rules().rule_for(std::string{operation});
        if (!m_context.authorizer().authorize_route(auth_ctx, rule)) {
            return std::nullopt;
        }
        if (rule.scheme() == ::app::auth::AuthScheme::PLATFORM_ADMIN) {
            auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED,
                "API key not provided or invalid API key used"
            );
            error.set_kind_name("IR_01");
            return error;
        }
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::UNAUTHORIZED,
            "Access forbidden, invalid JWT token was used"
        );
        error.set_kind_name("IR_17");
        return error;
    }

    [[nodiscard]] UserRequestData
    request_data(interfaces::io::IRequest& req, std::string body) const
    {
        UserRequestData data{};
        data.body = std::move(body);
        data.merchant_id = std::string{::app::http::merchant_id_from_header{}(req)};
        data.org_id = std::string{::app::http::org_id_from_header{}(req)};
        data.client_ip = std::string{req.find_header("x-forwarded-for")};
        data.token = ::app::auth::TokenParser{}(req);
        data.auth = ::app::auth::AuthContextBuilder{}(req);
        data.query = ::app::http::RequestQuery{}(req);
        const auto raw_path = req.get_path();
        const auto query_at = raw_path.find('?');
        data.path = std::string{
            query_at == std::string_view::npos ? raw_path : raw_path.substr(0, query_at)
        };
        return data;
    }

private:
    ::app::context::AppContext& m_context;
};

// One plain input per route: the parsed request plus all request-derived values.

struct GetActiveUserDetailsInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct LaunchSageInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct UserSignupWithMerchantIdInput
{
    UserRequestData request_context{};
    ::app::model::user::SignUpWithMerchantIdRequest request{};
};

struct UserSignupInput
{
    UserRequestData request_context{};
    ::app::model::user::SignUpRequest request{};
};

struct UserSigninInput
{
    UserRequestData request_context{};
    ::app::model::user::SignInRequest request{};
};

struct UserConnectAccountInput
{
    UserRequestData request_context{};
    ::app::model::user::ConnectAccountRequest request{};
};

struct SignoutInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct ChangePasswordInput
{
    UserRequestData request_context{};
    ::app::model::user::ChangePasswordRequest request{};
};

struct SetDashboardMetadataInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct GetMultipleDashboardMetadataInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct InternalUserSignupInput
{
    UserRequestData request_context{};
    ::app::model::user::CreateInternalUserRequest request{};
};

struct CreateTenantUserInput
{
    UserRequestData request_context{};
    ::app::model::user::CreateTenantUserRequest request{};
};

struct CreatePlatformInput
{
    UserRequestData request_context{};
    ::app::model::user::PlatformAccountCreateRequest request{};
};

struct UserOrgCreateInput
{
    UserRequestData request_context{};
    ::app::model::user::UserOrgMerchantCreateRequest request{};
};

struct UserMerchantAccountCreateInput
{
    UserRequestData request_context{};
    ::app::model::user::UserMerchantCreate request{};
};

struct GenerateSampleDataInput
{
    UserRequestData request_context{};
    ::app::model::user::SampleDataRequest request{};
};

struct DeleteSampleDataInput
{
    UserRequestData request_context{};
    ::app::model::user::SampleDataRequest request{};
};

struct ListUserRolesDetailsInput
{
    UserRequestData request_context{};
    ::app::model::user::GetUserRoleDetailsRequest request{};
};

struct RotatePasswordInput
{
    UserRequestData request_context{};
    ::app::model::user::RotatePasswordRequest request{};
};

struct ForgotPasswordInput
{
    UserRequestData request_context{};
    ::app::model::user::ForgotPasswordRequest request{};
};

struct ResetPasswordInput
{
    UserRequestData request_context{};
    ::app::model::user::ResetPasswordRequest request{};
};

struct InviteMultipleUserInput
{
    UserRequestData request_context{};
    std::vector<::app::model::user::InviteUserRequest> request{};
};

struct ResendInviteInput
{
    UserRequestData request_context{};
    ::app::model::user::ReInviteUserRequest request{};
};

struct AcceptInviteFromEmailInput
{
    UserRequestData request_context{};
    ::app::model::user::AcceptInviteFromEmailRequest request{};
};

struct TerminateAcceptInviteInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct VerifyEmailInput
{
    UserRequestData request_context{};
    ::app::model::user::VerifyEmailRequest request{};
};

struct VerifyEmailRequestInput
{
    UserRequestData request_context{};
    ::app::model::user::SendVerifyEmailRequest request{};
};

struct UpdateUserAccountDetailsInput
{
    UserRequestData request_context{};
    ::app::model::user::UpdateUserAccountDetailsRequest request{};
};

struct UserFromEmailInput
{
    UserRequestData request_context{};
    ::app::model::user::UserFromEmailRequest request{};
};

struct TotpBeginInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct TotpResetInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct TotpVerifyInput
{
    UserRequestData request_context{};
    ::app::model::user::VerifyTotpRequest request{};
};

struct VerifyRecoveryCodeInput
{
    UserRequestData request_context{};
    ::app::model::user::VerifyRecoveryCodeRequest request{};
};

struct TotpUpdateInput
{
    UserRequestData request_context{};
    ::app::model::user::VerifyTotpRequest request{};
};

struct GenerateRecoveryCodesInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct TerminateTwoFactorAuthInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct CheckTwoFactorAuthStatusInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct CheckTwoFactorAuthStatusWithAttemptsInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct GetSsoAuthUrlInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct SsoSignInput
{
    UserRequestData request_context{};
    ::app::model::user::SsoSignInRequest request{};
};

struct CreateUserAuthenticationMethodInput
{
    UserRequestData request_context{};
    ::app::model::user::CreateUserAuthenticationMethodRequest request{};
};

struct UpdateUserAuthenticationMethodInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct ListUserAuthenticationMethodsInput
{
    UserRequestData request_context{};
};

struct TerminateAuthSelectInput
{
    UserRequestData request_context{};
    ::app::model::user::AuthSelectRequest request{};
};

struct TransferUserKeyInput
{
    UserRequestData request_context{};
    ::app::model::user::UserKeyTransferRequest request{};
};

struct ListOrgsForUserInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct ListMerchantsForUserInOrgInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct ListProfilesForUserInOrgAndMerchantInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct SwitchOrgForUserInput
{
    UserRequestData request_context{};
    ::app::model::user::SwitchOrganizationRequest request{};
};

struct SwitchMerchantForUserInOrgInput
{
    UserRequestData request_context{};
    ::app::model::user::SwitchMerchantRequest request{};
};

struct SwitchProfileForUserInOrgAndMerchantInput
{
    UserRequestData request_context{};
    ::app::model::user::SwitchProfileRequest request{};
};

struct CloneConnectorInput
{
    UserRequestData request_context{};
    ::app::model::user::CloneConnectorRequest request{};
};

struct IssueEmbeddedTokenInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct EmbeddedTokenInfoInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct GetUserDetailsInternalInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct ListUsersInternalInput
{
    UserRequestData request_context{};
    ::app::model::user::ListUsersInternalRequest request{};
};

struct ListMembersForEntityInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

struct AuthorizeTokenInput
{
    UserRequestData request_context{};
    ::app::model::user::AuthorizeTokenRequest request{};
};

struct GetUserMerchantDetailsInput
{
    UserRequestData request_context{};
    interfaces::Value generic{};
};

/**
 * @brief Domain authorization policy for User.
 *
 * The route operation string selects the registered auth rule; the scope is
 * resolved from the request once, for every handler in the domain.
 */
class UserPolicy
{
public:
    explicit UserPolicy(UserInputSupport& input) noexcept :
        m_input{input}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const ::core::behavior::AuthSpec&
    ) const
    {
        return m_input.authorize(req, operation);
    }

    [[nodiscard]] ::core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        ::core::behavior::Scope out{};
        out.merchant_id = std::string{::app::http::merchant_id_from_header{}(req)};
        if (out.merchant_id.empty()) {
            out.merchant_id = auth.merchant_id();
        }
        out.org_id = std::string{::app::http::org_id_from_header{}(req)};
        if (out.org_id.empty()) {
            out.org_id = auth.org_id();
        }
        out.profile_id = auth.profile_id();
        const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId);
        if (connected.has_value()) {
            out.connected_merchant_id = std::string{*connected};
        }
        return out;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const ::core::behavior::Scope&) const
    {
        return std::nullopt;
    }


private:
    UserInputSupport& m_input;
};

using UserRequestSupport = ::core::behavior::RequestSupport<UserPolicy>;

} // namespace app::handlers::user
