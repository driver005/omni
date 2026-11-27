export module app_user:user_runtime;
import core_behavior;

import std;
import core_error;
import core_serde;
import app_context;

import :user_input;
import :user_result;
import :user_runtime_common;
import :user_runtime_get_active_user_details;
import :user_runtime_launch_sage;
import :user_runtime_user_signup_with_merchant_id;
import :user_runtime_user_signup;
import :user_runtime_user_signin;
import :user_runtime_user_connect_account;
import :user_runtime_signout;
import :user_runtime_change_password;
import :user_runtime_set_dashboard_metadata;
import :user_runtime_get_multiple_dashboard_metadata;
import :user_runtime_internal_user_signup;
import :user_runtime_create_tenant_user;
import :user_runtime_create_platform;
import :user_runtime_user_org_create;
import :user_runtime_user_merchant_account_create;
import :user_runtime_generate_sample_data;
import :user_runtime_delete_sample_data;
import :user_runtime_list_user_roles_details;
import :user_runtime_rotate_password;
import :user_runtime_forgot_password;
import :user_runtime_reset_password;
import :user_runtime_invite_multiple_user;
import :user_runtime_resend_invite;
import :user_runtime_accept_invite_from_email;
import :user_runtime_terminate_accept_invite;
import :user_runtime_verify_email;
import :user_runtime_verify_email_request;
import :user_runtime_update_user_account_details;
import :user_runtime_user_from_email;
import :user_runtime_totp_begin;
import :user_runtime_totp_reset;
import :user_runtime_totp_verify;
import :user_runtime_verify_recovery_code;
import :user_runtime_totp_update;
import :user_runtime_generate_recovery_codes;
import :user_runtime_terminate_two_factor_auth;
import :user_runtime_check_two_factor_auth_status;
import :user_runtime_check_two_factor_auth_status_with_attempts;
import :user_runtime_get_sso_auth_url;
import :user_runtime_sso_sign;
import :user_runtime_create_user_authentication_method;
import :user_runtime_update_user_authentication_method;
import :user_runtime_list_user_authentication_methods;
import :user_runtime_terminate_auth_select;
import :user_runtime_transfer_user_key;
import :user_runtime_list_orgs_for_user;
import :user_runtime_list_merchants_for_user_in_org;
import :user_runtime_list_profiles_for_user_in_org_and_merchant;
import :user_runtime_switch_org_for_user;
import :user_runtime_switch_merchant_for_user_in_org;
import :user_runtime_switch_profile_for_user_in_org_and_merchant;
import :user_runtime_clone_connector;
import :user_runtime_issue_embedded_token;
import :user_runtime_embedded_token_info;
import :user_runtime_get_user_details_internal;
import :user_runtime_list_users_internal;
import :user_runtime_list_members_for_entity;
import :user_runtime_authorize_token;
import :user_runtime_get_user_merchant_details;

export namespace app::handlers::user {

class UserRuntime
{
public:
    explicit UserRuntime(::app::context::AppContext& context) noexcept :
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

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    get_active_user_details(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_get_active_user_details.get_active_user_details(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    launch_sage(const interfaces::Value& generic, const ::core::behavior::CallContext& ctx) const
    {
        return m_launch_sage.launch_sage(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    user_signup_with_merchant_id(
        const ::app::model::user::SignUpWithMerchantIdRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_user_signup_with_merchant_id.user_signup_with_merchant_id(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> user_signup(
        const ::app::model::user::SignUpRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_user_signup.user_signup(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> user_signin(
        const ::app::model::user::SignInRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_user_signin.user_signin(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    user_connect_account(
        const ::app::model::user::ConnectAccountRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_user_connect_account.user_connect_account(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    signout(const interfaces::Value& generic, const ::core::behavior::CallContext& ctx) const
    {
        return m_signout.signout(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> change_password(
        const ::app::model::user::ChangePasswordRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_change_password.change_password(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    set_dashboard_metadata(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_set_dashboard_metadata.set_dashboard_metadata(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    get_multiple_dashboard_metadata(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_get_multiple_dashboard_metadata.get_multiple_dashboard_metadata(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    internal_user_signup(
        const ::app::model::user::CreateInternalUserRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_internal_user_signup.internal_user_signup(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> create_tenant_user(
        const ::app::model::user::CreateTenantUserRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_create_tenant_user.create_tenant_user(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> create_platform(
        const ::app::model::user::PlatformAccountCreateRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_create_platform.create_platform(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> user_org_create(
        const ::app::model::user::UserOrgMerchantCreateRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_user_org_create.user_org_create(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    user_merchant_account_create(
        const ::app::model::user::UserMerchantCreate& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_user_merchant_account_create.user_merchant_account_create(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    generate_sample_data(
        const ::app::model::user::SampleDataRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_generate_sample_data.generate_sample_data(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> delete_sample_data(
        const ::app::model::user::SampleDataRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_delete_sample_data.delete_sample_data(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    list_user_roles_details(
        const ::app::model::user::GetUserRoleDetailsRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_list_user_roles_details.list_user_roles_details(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> rotate_password(
        const ::app::model::user::RotatePasswordRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_rotate_password.rotate_password(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> forgot_password(
        const ::app::model::user::ForgotPasswordRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_forgot_password.forgot_password(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> reset_password(
        const ::app::model::user::ResetPasswordRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_reset_password.reset_password(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    invite_multiple_user(
        const std::vector<::app::model::user::InviteUserRequest>& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_invite_multiple_user.invite_multiple_user(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> resend_invite(
        const ::app::model::user::ReInviteUserRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_resend_invite.resend_invite(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    accept_invite_from_email(
        const ::app::model::user::AcceptInviteFromEmailRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_accept_invite_from_email.accept_invite_from_email(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    terminate_accept_invite(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_terminate_accept_invite.terminate_accept_invite(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> verify_email(
        const ::app::model::user::VerifyEmailRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_verify_email.verify_email(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    verify_email_request(
        const ::app::model::user::SendVerifyEmailRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_verify_email_request.verify_email_request(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    update_user_account_details(
        const ::app::model::user::UpdateUserAccountDetailsRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_update_user_account_details.update_user_account_details(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> user_from_email(
        const ::app::model::user::UserFromEmailRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_user_from_email.user_from_email(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    totp_begin(const interfaces::Value& generic, const ::core::behavior::CallContext& ctx) const
    {
        return m_totp_begin.totp_begin(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    totp_reset(const interfaces::Value& generic, const ::core::behavior::CallContext& ctx) const
    {
        return m_totp_reset.totp_reset(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> totp_verify(
        const ::app::model::user::VerifyTotpRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_totp_verify.totp_verify(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    verify_recovery_code(
        const ::app::model::user::VerifyRecoveryCodeRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_verify_recovery_code.verify_recovery_code(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> totp_update(
        const ::app::model::user::VerifyTotpRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_totp_update.totp_update(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    generate_recovery_codes(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_generate_recovery_codes.generate_recovery_codes(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    terminate_two_factor_auth(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_terminate_two_factor_auth.terminate_two_factor_auth(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    check_two_factor_auth_status(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_check_two_factor_auth_status.check_two_factor_auth_status(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    check_two_factor_auth_status_with_attempts(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_check_two_factor_auth_status_with_attempts
            .check_two_factor_auth_status_with_attempts(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> get_sso_auth_url(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_get_sso_auth_url.get_sso_auth_url(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> sso_sign(
        const ::app::model::user::SsoSignInRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_sso_sign.sso_sign(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    create_user_authentication_method(
        const ::app::model::user::CreateUserAuthenticationMethodRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_create_user_authentication_method.create_user_authentication_method(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    update_user_authentication_method(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_update_user_authentication_method.update_user_authentication_method(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    list_user_authentication_methods(const ::core::behavior::CallContext& ctx) const
    {
        return m_list_user_authentication_methods.list_user_authentication_methods(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    terminate_auth_select(
        const ::app::model::user::AuthSelectRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_terminate_auth_select.terminate_auth_select(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> transfer_user_key(
        const ::app::model::user::UserKeyTransferRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_transfer_user_key.transfer_user_key(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> list_orgs_for_user(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_list_orgs_for_user.list_orgs_for_user(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    list_merchants_for_user_in_org(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_list_merchants_for_user_in_org.list_merchants_for_user_in_org(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    list_profiles_for_user_in_org_and_merchant(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_list_profiles_for_user_in_org_and_merchant
            .list_profiles_for_user_in_org_and_merchant(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> switch_org_for_user(
        const ::app::model::user::SwitchOrganizationRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_switch_org_for_user.switch_org_for_user(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    switch_merchant_for_user_in_org(
        const ::app::model::user::SwitchMerchantRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_switch_merchant_for_user_in_org.switch_merchant_for_user_in_org(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    switch_profile_for_user_in_org_and_merchant(
        const ::app::model::user::SwitchProfileRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_switch_profile_for_user_in_org_and_merchant
            .switch_profile_for_user_in_org_and_merchant(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> clone_connector(
        const ::app::model::user::CloneConnectorRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_clone_connector.clone_connector(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    issue_embedded_token(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_issue_embedded_token.issue_embedded_token(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> embedded_token_info(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_embedded_token_info.embedded_token_info(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    get_user_details_internal(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_get_user_details_internal.get_user_details_internal(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> list_users_internal(
        const ::app::model::user::ListUsersInternalRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_list_users_internal.list_users_internal(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    list_members_for_entity(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_list_members_for_entity.list_members_for_entity(generic, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>> authorize_token(
        const ::app::model::user::AuthorizeTokenRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_authorize_token.authorize_token(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    get_user_merchant_details(
        const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_get_user_merchant_details.get_user_merchant_details(generic, ctx);
    }

private:
    ::app::context::AppContext& m_context;
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
