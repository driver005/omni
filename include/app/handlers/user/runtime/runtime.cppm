export module app_handlers_user:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_get_active_user_details;
import :runtime_launch_sage;
import :runtime_user_signup_with_merchant_id;
import :runtime_user_signup;
import :runtime_user_signin;
import :runtime_user_connect_account;
import :runtime_signout;
import :runtime_change_password;
import :runtime_set_dashboard_metadata;
import :runtime_get_multiple_dashboard_metadata;
import :runtime_internal_user_signup;
import :runtime_create_tenant_user;
import :runtime_create_platform;
import :runtime_user_org_create;
import :runtime_user_merchant_account_create;
import :runtime_generate_sample_data;
import :runtime_delete_sample_data;
import :runtime_list_user_roles_details;
import :runtime_rotate_password;
import :runtime_forgot_password;
import :runtime_reset_password;
import :runtime_invite_multiple_user;
import :runtime_resend_invite;
import :runtime_accept_invite_from_email;
import :runtime_terminate_accept_invite;
import :runtime_verify_email;
import :runtime_verify_email_request;
import :runtime_update_user_account_details;
import :runtime_user_from_email;
import :runtime_totp_begin;
import :runtime_totp_reset;
import :runtime_totp_verify;
import :runtime_verify_recovery_code;
import :runtime_totp_update;
import :runtime_generate_recovery_codes;
import :runtime_terminate_two_factor_auth;
import :runtime_check_two_factor_auth_status;
import :runtime_check_two_factor_auth_status_with_attempts;
import :runtime_get_sso_auth_url;
import :runtime_sso_sign;
import :runtime_create_user_authentication_method;
import :runtime_update_user_authentication_method;
import :runtime_list_user_authentication_methods;
import :runtime_terminate_auth_select;
import :runtime_transfer_user_key;
import :runtime_list_orgs_for_user;
import :runtime_list_merchants_for_user_in_org;
import :runtime_list_profiles_for_user_in_org_and_merchant;
import :runtime_switch_org_for_user;
import :runtime_switch_merchant_for_user_in_org;
import :runtime_switch_profile_for_user_in_org_and_merchant;
import :runtime_clone_connector;
import :runtime_issue_embedded_token;
import :runtime_embedded_token_info;
import :runtime_get_user_details_internal;
import :runtime_list_users_internal;
import :runtime_list_members_for_entity;
import :runtime_authorize_token;
import :runtime_get_user_merchant_details;

export namespace app::handlers::user {

class UserRuntime
{
public:
    explicit UserRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_get_active_user_details{context},
        m_launch_sage{context},
        m_user_signup_with_merchant_id{context},
        m_user_signup{context},
        m_user_signin{context},
        m_user_connect_account{context},
        m_signout{context},
        m_change_password{context},
        m_set_dashboard_metadata{context},
        m_get_multiple_dashboard_metadata{context},
        m_internal_user_signup{context},
        m_create_tenant_user{context},
        m_create_platform{context},
        m_user_org_create{context},
        m_user_merchant_account_create{context},
        m_generate_sample_data{context},
        m_delete_sample_data{context},
        m_list_user_roles_details{context},
        m_rotate_password{context},
        m_forgot_password{context},
        m_reset_password{context},
        m_invite_multiple_user{context},
        m_resend_invite{context},
        m_accept_invite_from_email{context},
        m_terminate_accept_invite{context},
        m_verify_email{context},
        m_verify_email_request{context},
        m_update_user_account_details{context},
        m_user_from_email{context},
        m_totp_begin{context},
        m_totp_reset{context},
        m_totp_verify{context},
        m_verify_recovery_code{context},
        m_totp_update{context},
        m_generate_recovery_codes{context},
        m_terminate_two_factor_auth{context},
        m_check_two_factor_auth_status{context},
        m_check_two_factor_auth_status_with_attempts{context},
        m_get_sso_auth_url{context},
        m_sso_sign{context},
        m_create_user_authentication_method{context},
        m_update_user_authentication_method{context},
        m_list_user_authentication_methods{context},
        m_terminate_auth_select{context},
        m_transfer_user_key{context},
        m_list_orgs_for_user{context},
        m_list_merchants_for_user_in_org{context},
        m_list_profiles_for_user_in_org_and_merchant{context},
        m_switch_org_for_user{context},
        m_switch_merchant_for_user_in_org{context},
        m_switch_profile_for_user_in_org_and_merchant{context},
        m_clone_connector{context},
        m_issue_embedded_token{context},
        m_embedded_token_info{context},
        m_get_user_details_internal{context},
        m_list_users_internal{context},
        m_list_members_for_entity{context},
        m_authorize_token{context},
        m_get_user_merchant_details{context}
    {
    }

    [[nodiscard]] std::expected<GetActiveUserDetailsResult, core::error::AppError>
    get_active_user_details(const GetActiveUserDetailsInput& input) const
    {
        return m_get_active_user_details.get_active_user_details(input);
    }

    [[nodiscard]] std::expected<LaunchSageResult, core::error::AppError>
    launch_sage(const LaunchSageInput& input) const
    {
        return m_launch_sage.launch_sage(input);
    }

    [[nodiscard]] std::expected<UserSignupWithMerchantIdResult, core::error::AppError>
    user_signup_with_merchant_id(const UserSignupWithMerchantIdInput& input) const
    {
        return m_user_signup_with_merchant_id.user_signup_with_merchant_id(input);
    }

    [[nodiscard]] std::expected<UserSignupResult, core::error::AppError>
    user_signup(const UserSignupInput& input) const
    {
        return m_user_signup.user_signup(input);
    }

    [[nodiscard]] std::expected<UserSigninResult, core::error::AppError>
    user_signin(const UserSigninInput& input) const
    {
        return m_user_signin.user_signin(input);
    }

    [[nodiscard]] std::expected<UserConnectAccountResult, core::error::AppError>
    user_connect_account(const UserConnectAccountInput& input) const
    {
        return m_user_connect_account.user_connect_account(input);
    }

    [[nodiscard]] std::expected<SignoutResult, core::error::AppError>
    signout(const SignoutInput& input) const
    {
        return m_signout.signout(input);
    }

    [[nodiscard]] std::expected<ChangePasswordResult, core::error::AppError>
    change_password(const ChangePasswordInput& input) const
    {
        return m_change_password.change_password(input);
    }

    [[nodiscard]] std::expected<SetDashboardMetadataResult, core::error::AppError>
    set_dashboard_metadata(const SetDashboardMetadataInput& input) const
    {
        return m_set_dashboard_metadata.set_dashboard_metadata(input);
    }

    [[nodiscard]] std::expected<GetMultipleDashboardMetadataResult, core::error::AppError>
    get_multiple_dashboard_metadata(const GetMultipleDashboardMetadataInput& input) const
    {
        return m_get_multiple_dashboard_metadata.get_multiple_dashboard_metadata(input);
    }

    [[nodiscard]] std::expected<InternalUserSignupResult, core::error::AppError>
    internal_user_signup(const InternalUserSignupInput& input) const
    {
        return m_internal_user_signup.internal_user_signup(input);
    }

    [[nodiscard]] std::expected<CreateTenantUserResult, core::error::AppError>
    create_tenant_user(const CreateTenantUserInput& input) const
    {
        return m_create_tenant_user.create_tenant_user(input);
    }

    [[nodiscard]] std::expected<CreatePlatformResult, core::error::AppError>
    create_platform(const CreatePlatformInput& input) const
    {
        return m_create_platform.create_platform(input);
    }

    [[nodiscard]] std::expected<UserOrgCreateResult, core::error::AppError>
    user_org_create(const UserOrgCreateInput& input) const
    {
        return m_user_org_create.user_org_create(input);
    }

    [[nodiscard]] std::expected<UserMerchantAccountCreateResult, core::error::AppError>
    user_merchant_account_create(const UserMerchantAccountCreateInput& input) const
    {
        return m_user_merchant_account_create.user_merchant_account_create(input);
    }

    [[nodiscard]] std::expected<GenerateSampleDataResult, core::error::AppError>
    generate_sample_data(const GenerateSampleDataInput& input) const
    {
        return m_generate_sample_data.generate_sample_data(input);
    }

    [[nodiscard]] std::expected<DeleteSampleDataResult, core::error::AppError>
    delete_sample_data(const DeleteSampleDataInput& input) const
    {
        return m_delete_sample_data.delete_sample_data(input);
    }

    [[nodiscard]] std::expected<ListUserRolesDetailsResult, core::error::AppError>
    list_user_roles_details(const ListUserRolesDetailsInput& input) const
    {
        return m_list_user_roles_details.list_user_roles_details(input);
    }

    [[nodiscard]] std::expected<RotatePasswordResult, core::error::AppError>
    rotate_password(const RotatePasswordInput& input) const
    {
        return m_rotate_password.rotate_password(input);
    }

    [[nodiscard]] std::expected<ForgotPasswordResult, core::error::AppError>
    forgot_password(const ForgotPasswordInput& input) const
    {
        return m_forgot_password.forgot_password(input);
    }

    [[nodiscard]] std::expected<ResetPasswordResult, core::error::AppError>
    reset_password(const ResetPasswordInput& input) const
    {
        return m_reset_password.reset_password(input);
    }

    [[nodiscard]] std::expected<InviteMultipleUserResult, core::error::AppError>
    invite_multiple_user(const InviteMultipleUserInput& input) const
    {
        return m_invite_multiple_user.invite_multiple_user(input);
    }

    [[nodiscard]] std::expected<ResendInviteResult, core::error::AppError>
    resend_invite(const ResendInviteInput& input) const
    {
        return m_resend_invite.resend_invite(input);
    }

    [[nodiscard]] std::expected<AcceptInviteFromEmailResult, core::error::AppError>
    accept_invite_from_email(const AcceptInviteFromEmailInput& input) const
    {
        return m_accept_invite_from_email.accept_invite_from_email(input);
    }

    [[nodiscard]] std::expected<TerminateAcceptInviteResult, core::error::AppError>
    terminate_accept_invite(const TerminateAcceptInviteInput& input) const
    {
        return m_terminate_accept_invite.terminate_accept_invite(input);
    }

    [[nodiscard]] std::expected<VerifyEmailResult, core::error::AppError>
    verify_email(const VerifyEmailInput& input) const
    {
        return m_verify_email.verify_email(input);
    }

    [[nodiscard]] std::expected<VerifyEmailRequestResult, core::error::AppError>
    verify_email_request(const VerifyEmailRequestInput& input) const
    {
        return m_verify_email_request.verify_email_request(input);
    }

    [[nodiscard]] std::expected<UpdateUserAccountDetailsResult, core::error::AppError>
    update_user_account_details(const UpdateUserAccountDetailsInput& input) const
    {
        return m_update_user_account_details.update_user_account_details(input);
    }

    [[nodiscard]] std::expected<UserFromEmailResult, core::error::AppError>
    user_from_email(const UserFromEmailInput& input) const
    {
        return m_user_from_email.user_from_email(input);
    }

    [[nodiscard]] std::expected<TotpBeginResult, core::error::AppError>
    totp_begin(const TotpBeginInput& input) const
    {
        return m_totp_begin.totp_begin(input);
    }

    [[nodiscard]] std::expected<TotpResetResult, core::error::AppError>
    totp_reset(const TotpResetInput& input) const
    {
        return m_totp_reset.totp_reset(input);
    }

    [[nodiscard]] std::expected<TotpVerifyResult, core::error::AppError>
    totp_verify(const TotpVerifyInput& input) const
    {
        return m_totp_verify.totp_verify(input);
    }

    [[nodiscard]] std::expected<VerifyRecoveryCodeResult, core::error::AppError>
    verify_recovery_code(const VerifyRecoveryCodeInput& input) const
    {
        return m_verify_recovery_code.verify_recovery_code(input);
    }

    [[nodiscard]] std::expected<TotpUpdateResult, core::error::AppError>
    totp_update(const TotpUpdateInput& input) const
    {
        return m_totp_update.totp_update(input);
    }

    [[nodiscard]] std::expected<GenerateRecoveryCodesResult, core::error::AppError>
    generate_recovery_codes(const GenerateRecoveryCodesInput& input) const
    {
        return m_generate_recovery_codes.generate_recovery_codes(input);
    }

    [[nodiscard]] std::expected<TerminateTwoFactorAuthResult, core::error::AppError>
    terminate_two_factor_auth(const TerminateTwoFactorAuthInput& input) const
    {
        return m_terminate_two_factor_auth.terminate_two_factor_auth(input);
    }

    [[nodiscard]] std::expected<CheckTwoFactorAuthStatusResult, core::error::AppError>
    check_two_factor_auth_status(const CheckTwoFactorAuthStatusInput& input) const
    {
        return m_check_two_factor_auth_status.check_two_factor_auth_status(input);
    }

    [[nodiscard]] std::expected<CheckTwoFactorAuthStatusWithAttemptsResult, core::error::AppError>
    check_two_factor_auth_status_with_attempts(
        const CheckTwoFactorAuthStatusWithAttemptsInput& input
    ) const
    {
        return m_check_two_factor_auth_status_with_attempts
            .check_two_factor_auth_status_with_attempts(input);
    }

    [[nodiscard]] std::expected<GetSsoAuthUrlResult, core::error::AppError>
    get_sso_auth_url(const GetSsoAuthUrlInput& input) const
    {
        return m_get_sso_auth_url.get_sso_auth_url(input);
    }

    [[nodiscard]] std::expected<SsoSignResult, core::error::AppError>
    sso_sign(const SsoSignInput& input) const
    {
        return m_sso_sign.sso_sign(input);
    }

    [[nodiscard]] std::expected<CreateUserAuthenticationMethodResult, core::error::AppError>
    create_user_authentication_method(const CreateUserAuthenticationMethodInput& input) const
    {
        return m_create_user_authentication_method.create_user_authentication_method(input);
    }

    [[nodiscard]] std::expected<UpdateUserAuthenticationMethodResult, core::error::AppError>
    update_user_authentication_method(const UpdateUserAuthenticationMethodInput& input) const
    {
        return m_update_user_authentication_method.update_user_authentication_method(input);
    }

    [[nodiscard]] std::expected<ListUserAuthenticationMethodsResult, core::error::AppError>
    list_user_authentication_methods(const ListUserAuthenticationMethodsInput& input) const
    {
        return m_list_user_authentication_methods.list_user_authentication_methods(input);
    }

    [[nodiscard]] std::expected<TerminateAuthSelectResult, core::error::AppError>
    terminate_auth_select(const TerminateAuthSelectInput& input) const
    {
        return m_terminate_auth_select.terminate_auth_select(input);
    }

    [[nodiscard]] std::expected<TransferUserKeyResult, core::error::AppError>
    transfer_user_key(const TransferUserKeyInput& input) const
    {
        return m_transfer_user_key.transfer_user_key(input);
    }

    [[nodiscard]] std::expected<ListOrgsForUserResult, core::error::AppError>
    list_orgs_for_user(const ListOrgsForUserInput& input) const
    {
        return m_list_orgs_for_user.list_orgs_for_user(input);
    }

    [[nodiscard]] std::expected<ListMerchantsForUserInOrgResult, core::error::AppError>
    list_merchants_for_user_in_org(const ListMerchantsForUserInOrgInput& input) const
    {
        return m_list_merchants_for_user_in_org.list_merchants_for_user_in_org(input);
    }

    [[nodiscard]] std::expected<ListProfilesForUserInOrgAndMerchantResult, core::error::AppError>
    list_profiles_for_user_in_org_and_merchant(
        const ListProfilesForUserInOrgAndMerchantInput& input
    ) const
    {
        return m_list_profiles_for_user_in_org_and_merchant
            .list_profiles_for_user_in_org_and_merchant(input);
    }

    [[nodiscard]] std::expected<SwitchOrgForUserResult, core::error::AppError>
    switch_org_for_user(const SwitchOrgForUserInput& input) const
    {
        return m_switch_org_for_user.switch_org_for_user(input);
    }

    [[nodiscard]] std::expected<SwitchMerchantForUserInOrgResult, core::error::AppError>
    switch_merchant_for_user_in_org(const SwitchMerchantForUserInOrgInput& input) const
    {
        return m_switch_merchant_for_user_in_org.switch_merchant_for_user_in_org(input);
    }

    [[nodiscard]] std::expected<SwitchProfileForUserInOrgAndMerchantResult, core::error::AppError>
    switch_profile_for_user_in_org_and_merchant(
        const SwitchProfileForUserInOrgAndMerchantInput& input
    ) const
    {
        return m_switch_profile_for_user_in_org_and_merchant
            .switch_profile_for_user_in_org_and_merchant(input);
    }

    [[nodiscard]] std::expected<CloneConnectorResult, core::error::AppError>
    clone_connector(const CloneConnectorInput& input) const
    {
        return m_clone_connector.clone_connector(input);
    }

    [[nodiscard]] std::expected<IssueEmbeddedTokenResult, core::error::AppError>
    issue_embedded_token(const IssueEmbeddedTokenInput& input) const
    {
        return m_issue_embedded_token.issue_embedded_token(input);
    }

    [[nodiscard]] std::expected<EmbeddedTokenInfoResult, core::error::AppError>
    embedded_token_info(const EmbeddedTokenInfoInput& input) const
    {
        return m_embedded_token_info.embedded_token_info(input);
    }

    [[nodiscard]] std::expected<GetUserDetailsInternalResult, core::error::AppError>
    get_user_details_internal(const GetUserDetailsInternalInput& input) const
    {
        return m_get_user_details_internal.get_user_details_internal(input);
    }

    [[nodiscard]] std::expected<ListUsersInternalResult, core::error::AppError>
    list_users_internal(const ListUsersInternalInput& input) const
    {
        return m_list_users_internal.list_users_internal(input);
    }

    [[nodiscard]] std::expected<ListMembersForEntityResult, core::error::AppError>
    list_members_for_entity(const ListMembersForEntityInput& input) const
    {
        return m_list_members_for_entity.list_members_for_entity(input);
    }

    [[nodiscard]] std::expected<AuthorizeTokenResult, core::error::AppError>
    authorize_token(const AuthorizeTokenInput& input) const
    {
        return m_authorize_token.authorize_token(input);
    }

    [[nodiscard]] std::expected<GetUserMerchantDetailsResult, core::error::AppError>
    get_user_merchant_details(const GetUserMerchantDetailsInput& input) const
    {
        return m_get_user_merchant_details.get_user_merchant_details(input);
    }

private:
    app::context::AppContext& m_context;
    GetActiveUserDetailsRuntime m_get_active_user_details;
    LaunchSageRuntime m_launch_sage;
    UserSignupWithMerchantIdRuntime m_user_signup_with_merchant_id;
    UserSignupRuntime m_user_signup;
    UserSigninRuntime m_user_signin;
    UserConnectAccountRuntime m_user_connect_account;
    SignoutRuntime m_signout;
    ChangePasswordRuntime m_change_password;
    SetDashboardMetadataRuntime m_set_dashboard_metadata;
    GetMultipleDashboardMetadataRuntime m_get_multiple_dashboard_metadata;
    InternalUserSignupRuntime m_internal_user_signup;
    CreateTenantUserRuntime m_create_tenant_user;
    CreatePlatformRuntime m_create_platform;
    UserOrgCreateRuntime m_user_org_create;
    UserMerchantAccountCreateRuntime m_user_merchant_account_create;
    GenerateSampleDataRuntime m_generate_sample_data;
    DeleteSampleDataRuntime m_delete_sample_data;
    ListUserRolesDetailsRuntime m_list_user_roles_details;
    RotatePasswordRuntime m_rotate_password;
    ForgotPasswordRuntime m_forgot_password;
    ResetPasswordRuntime m_reset_password;
    InviteMultipleUserRuntime m_invite_multiple_user;
    ResendInviteRuntime m_resend_invite;
    AcceptInviteFromEmailRuntime m_accept_invite_from_email;
    TerminateAcceptInviteRuntime m_terminate_accept_invite;
    VerifyEmailRuntime m_verify_email;
    VerifyEmailRequestRuntime m_verify_email_request;
    UpdateUserAccountDetailsRuntime m_update_user_account_details;
    UserFromEmailRuntime m_user_from_email;
    TotpBeginRuntime m_totp_begin;
    TotpResetRuntime m_totp_reset;
    TotpVerifyRuntime m_totp_verify;
    VerifyRecoveryCodeRuntime m_verify_recovery_code;
    TotpUpdateRuntime m_totp_update;
    GenerateRecoveryCodesRuntime m_generate_recovery_codes;
    TerminateTwoFactorAuthRuntime m_terminate_two_factor_auth;
    CheckTwoFactorAuthStatusRuntime m_check_two_factor_auth_status;
    CheckTwoFactorAuthStatusWithAttemptsRuntime m_check_two_factor_auth_status_with_attempts;
    GetSsoAuthUrlRuntime m_get_sso_auth_url;
    SsoSignRuntime m_sso_sign;
    CreateUserAuthenticationMethodRuntime m_create_user_authentication_method;
    UpdateUserAuthenticationMethodRuntime m_update_user_authentication_method;
    ListUserAuthenticationMethodsRuntime m_list_user_authentication_methods;
    TerminateAuthSelectRuntime m_terminate_auth_select;
    TransferUserKeyRuntime m_transfer_user_key;
    ListOrgsForUserRuntime m_list_orgs_for_user;
    ListMerchantsForUserInOrgRuntime m_list_merchants_for_user_in_org;
    ListProfilesForUserInOrgAndMerchantRuntime m_list_profiles_for_user_in_org_and_merchant;
    SwitchOrgForUserRuntime m_switch_org_for_user;
    SwitchMerchantForUserInOrgRuntime m_switch_merchant_for_user_in_org;
    SwitchProfileForUserInOrgAndMerchantRuntime m_switch_profile_for_user_in_org_and_merchant;
    CloneConnectorRuntime m_clone_connector;
    IssueEmbeddedTokenRuntime m_issue_embedded_token;
    EmbeddedTokenInfoRuntime m_embedded_token_info;
    GetUserDetailsInternalRuntime m_get_user_details_internal;
    ListUsersInternalRuntime m_list_users_internal;
    ListMembersForEntityRuntime m_list_members_for_entity;
    AuthorizeTokenRuntime m_authorize_token;
    GetUserMerchantDetailsRuntime m_get_user_merchant_details;
};

} // namespace app::handlers::user
