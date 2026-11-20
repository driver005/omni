module;
export module app_model:user;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::user {

class AcceptInviteFromEmailRequest
{
public:
    void set_token(std::string value) { token = std::move(value); }
    [[nodiscard]] const std::string& get_token() const noexcept { return token; }
    std::string token{};
};

enum class AcceptInviteResponse
{
    Token,
    Status,
};

class ApiVersionValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class AuthIdAndThemeIdQueryParam
{
public:
    void set_auth_id(std::optional<std::string> value) { auth_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_auth_id() const noexcept { return auth_id; }
    void set_theme_id(std::optional<std::string> value) { theme_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_theme_id() const noexcept { return theme_id; }
    std::optional<std::string> auth_id{};
    std::optional<std::string> theme_id{};
};

class AuthSelectRequest
{
public:
    void set_id(std::optional<std::string> value) { id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_id() const noexcept { return id; }
    std::optional<std::string> id{};
};

class AuthorizeResponse
{
public:
    void set_is_email_sent(bool value) { is_email_sent = std::move(value); }
    [[nodiscard]] const bool& get_is_email_sent() const noexcept { return is_email_sent; }
    void set_user_id(std::string value) { user_id = std::move(value); }
    [[nodiscard]] const std::string& get_user_id() const noexcept { return user_id; }
    bool is_email_sent{};
    std::string user_id{};
};

class AuthorizeTokenRequest
{
public:
    void set_token(std::string value) { token = std::move(value); }
    [[nodiscard]] const std::string& get_token() const noexcept { return token; }
    void set_permission(std::string value) { permission = std::move(value); }
    [[nodiscard]] const std::string& get_permission() const noexcept { return permission; }
    std::string token{};
    std::string permission{};
};

class ChangePasswordRequest
{
public:
    void set_new_password(std::string value) { new_password = std::move(value); }
    [[nodiscard]] const std::string& get_new_password() const noexcept { return new_password; }
    void set_old_password(std::string value) { old_password = std::move(value); }
    [[nodiscard]] const std::string& get_old_password() const noexcept { return old_password; }
    std::string new_password{};
    std::string old_password{};
};

class EmailValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class EntityTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class GetSsoAuthUrlRequest
{
public:
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    std::string id{};
};

class GetUserAuthenticationMethodsRequest
{
public:
    void set_auth_id(std::optional<std::string> value) { auth_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_auth_id() const noexcept { return auth_id; }
    void set_email_domain(std::optional<std::string> value) { email_domain = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_email_domain() const noexcept { return email_domain; }
    std::optional<std::string> auth_id{};
    std::optional<std::string> email_domain{};
};

enum class InvitationAcceptanceStatus
{
    AlreadyAccepted,
    SuccessfullyAccepted,
};

class IssueEmbeddedTokenResponse
{
public:
    void set_token(std::string value) { token = std::move(value); }
    [[nodiscard]] const std::string& get_token() const noexcept { return token; }
    std::string token{};
};

class ListUsersInternalRequest
{
public:
    void set_user_ids(std::vector<std::string> value) { user_ids = std::move(value); }
    [[nodiscard]] const std::vector<std::string>& get_user_ids() const noexcept { return user_ids; }
    std::vector<std::string> user_ids{};
};

class MerchantAccountRequestTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MerchantAccountTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MerchantConnectorAccountIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MerchantIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MerchantProductTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class NameIdUnit
{
public:
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    std::string name{};
    std::string id{};
};

class OfferEngineCredentialSourceValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class OpenIdConnectPrivateConfig
{
public:
    void set_base_url(std::string value) { base_url = std::move(value); }
    [[nodiscard]] const std::string& get_base_url() const noexcept { return base_url; }
    void set_client_id(std::string value) { client_id = std::move(value); }
    [[nodiscard]] const std::string& get_client_id() const noexcept { return client_id; }
    void set_client_secret(std::string value) { client_secret = std::move(value); }
    [[nodiscard]] const std::string& get_client_secret() const noexcept { return client_secret; }
    void set_private_key(std::optional<std::string> value) { private_key = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_private_key() const noexcept { return private_key; }
    std::string base_url{};
    std::string client_id{};
    std::string client_secret{};
    std::optional<std::string> private_key{};
};

enum class OpenIdProvider
{
    Okta,
};

class OptionalEncryptableNameValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class OrganizationIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class OrganizationTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class OwnerValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PlatformAccountCreateRequest
{
public:
    void set_organization_name(std::string value) { organization_name = std::move(value); }
    [[nodiscard]] const std::string& get_organization_name() const noexcept { return organization_name; }
    std::string organization_name{};
};

class ProfileIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class RecoveryCodes
{
public:
    void set_recovery_codes(std::vector<std::string> value) { recovery_codes = std::move(value); }
    [[nodiscard]] const std::vector<std::string>& get_recovery_codes() const noexcept { return recovery_codes; }
    std::vector<std::string> recovery_codes{};
};

class ResetPasswordRequest
{
public:
    void set_token(std::string value) { token = std::move(value); }
    [[nodiscard]] const std::string& get_token() const noexcept { return token; }
    void set_password(std::string value) { password = std::move(value); }
    [[nodiscard]] const std::string& get_password() const noexcept { return password; }
    std::string token{};
    std::string password{};
};

class RotatePasswordRequest
{
public:
    void set_password(std::string value) { password = std::move(value); }
    [[nodiscard]] const std::string& get_password() const noexcept { return password; }
    std::string password{};
};

class SecretSerdeValueValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class SkipTwoFactorAuthQueryParam
{
public:
    void set_skip_two_factor_auth(std::optional<bool> value) { skip_two_factor_auth = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_skip_two_factor_auth() const noexcept { return skip_two_factor_auth; }
    std::optional<bool> skip_two_factor_auth{};
};

class SsoSignInRequest
{
public:
    void set_state(std::string value) { state = std::move(value); }
    [[nodiscard]] const std::string& get_state() const noexcept { return state; }
    void set_code(std::string value) { code = std::move(value); }
    [[nodiscard]] const std::string& get_code() const noexcept { return code; }
    std::string state{};
    std::string code{};
};

class TokenPurposeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class TotpSecret
{
public:
    void set_secret(std::string value) { secret = std::move(value); }
    [[nodiscard]] const std::string& get_secret() const noexcept { return secret; }
    void set_totp_url(std::string value) { totp_url = std::move(value); }
    [[nodiscard]] const std::string& get_totp_url() const noexcept { return totp_url; }
    std::string secret{};
    std::string totp_url{};
};

class TwoFactorAuthAttempts
{
public:
    void set_is_completed(bool value) { is_completed = std::move(value); }
    [[nodiscard]] const bool& get_is_completed() const noexcept { return is_completed; }
    void set_remaining_attempts(std::uint8_t value) { remaining_attempts = std::move(value); }
    [[nodiscard]] const std::uint8_t& get_remaining_attempts() const noexcept { return remaining_attempts; }
    bool is_completed{};
    std::uint8_t remaining_attempts{};
};

class TwoFactorAuthStatusResponse
{
public:
    void set_totp(bool value) { totp = std::move(value); }
    [[nodiscard]] const bool& get_totp() const noexcept { return totp; }
    void set_recovery_code(bool value) { recovery_code = std::move(value); }
    [[nodiscard]] const bool& get_recovery_code() const noexcept { return recovery_code; }
    bool totp{};
    bool recovery_code{};
};

class UpdateUserAccountDetailsRequest
{
public:
    void set_name(std::optional<std::string> value) { name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_name() const noexcept { return name; }
    std::optional<std::string> name{};
};

enum class UpdateUserAuthenticationMethodRequest
{
    AuthMethod,
    EmailDomain,
};

class UserAuthTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class UserFromEmailRequest
{
public:
    void set_token(std::string value) { token = std::move(value); }
    [[nodiscard]] const std::string& get_token() const noexcept { return token; }
    std::string token{};
};

class UserKeyTransferRequest
{
public:
    void set_from(std::uint32_t value) { from = std::move(value); }
    [[nodiscard]] const std::uint32_t& get_from() const noexcept { return from; }
    void set_limit(std::uint32_t value) { limit = std::move(value); }
    [[nodiscard]] const std::uint32_t& get_limit() const noexcept { return limit; }
    std::uint32_t from{};
    std::uint32_t limit{};
};

class UserStatusValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class UserTransferKeyResponse
{
public:
    void set_total_transferred(std::size_t value) { total_transferred = std::move(value); }
    [[nodiscard]] const std::size_t& get_total_transferred() const noexcept { return total_transferred; }
    std::size_t total_transferred{};
};

class ValidateOnlyQueryParam
{
public:
    void set_status_check(std::optional<bool> value) { status_check = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_status_check() const noexcept { return status_check; }
    std::optional<bool> status_check{};
};

class VerifyEmailRequest
{
public:
    void set_token(std::string value) { token = std::move(value); }
    [[nodiscard]] const std::string& get_token() const noexcept { return token; }
    std::string token{};
};

class VerifyRecoveryCodeRequest
{
public:
    void set_recovery_code(std::string value) { recovery_code = std::move(value); }
    [[nodiscard]] const std::string& get_recovery_code() const noexcept { return recovery_code; }
    std::string recovery_code{};
};

class VerifyTotpRequest
{
public:
    void set_totp(std::string value) { totp = std::move(value); }
    [[nodiscard]] const std::string& get_totp() const noexcept { return totp; }
    std::string totp{};
};

class AuthMethodDetails
{
public:
    void set_auth_type(UserAuthTypeValue value) { auth_type = std::move(value); }
    [[nodiscard]] const UserAuthTypeValue& get_auth_type() const noexcept { return auth_type; }
    void set_name(std::optional<OpenIdProvider> value) { name = std::move(value); }
    [[nodiscard]] const std::optional<OpenIdProvider>& get_name() const noexcept { return name; }
    UserAuthTypeValue auth_type{};
    std::optional<OpenIdProvider> name{};
};

class BeginTotpResponse
{
public:
    void set_secret(std::optional<TotpSecret> value) { secret = std::move(value); }
    [[nodiscard]] const std::optional<TotpSecret>& get_secret() const noexcept { return secret; }
    std::optional<TotpSecret> secret{};
};

class CloneConnectorRequest
{
public:
    void set_source_mca_id(MerchantConnectorAccountIdValue value) { source_mca_id = std::move(value); }
    [[nodiscard]] const MerchantConnectorAccountIdValue& get_source_mca_id() const noexcept { return source_mca_id; }
    void set_destination_profile_id(ProfileIdValue value) { destination_profile_id = std::move(value); }
    [[nodiscard]] const ProfileIdValue& get_destination_profile_id() const noexcept { return destination_profile_id; }
    void set_connector_label(std::optional<std::string> value) { connector_label = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_label() const noexcept { return connector_label; }
    MerchantConnectorAccountIdValue source_mca_id{};
    ProfileIdValue destination_profile_id{};
    std::optional<std::string> connector_label{};
};

class ConnectAccountRequest
{
public:
    void set_email(EmailValue value) { email = std::move(value); }
    [[nodiscard]] const EmailValue& get_email() const noexcept { return email; }
    EmailValue email{};
};

class CreateInternalUserRequest
{
public:
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_email(EmailValue value) { email = std::move(value); }
    [[nodiscard]] const EmailValue& get_email() const noexcept { return email; }
    void set_password(std::string value) { password = std::move(value); }
    [[nodiscard]] const std::string& get_password() const noexcept { return password; }
    void set_role_id(std::string value) { role_id = std::move(value); }
    [[nodiscard]] const std::string& get_role_id() const noexcept { return role_id; }
    std::string name{};
    EmailValue email{};
    std::string password{};
    std::string role_id{};
};

class CreateTenantUserRequest
{
public:
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_email(EmailValue value) { email = std::move(value); }
    [[nodiscard]] const EmailValue& get_email() const noexcept { return email; }
    void set_password(std::string value) { password = std::move(value); }
    [[nodiscard]] const std::string& get_password() const noexcept { return password; }
    std::string name{};
    EmailValue email{};
    std::string password{};
};

class CreateUserAuthenticationMethodResponse
{
public:
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_auth_id(std::string value) { auth_id = std::move(value); }
    [[nodiscard]] const std::string& get_auth_id() const noexcept { return auth_id; }
    void set_owner_id(std::string value) { owner_id = std::move(value); }
    [[nodiscard]] const std::string& get_owner_id() const noexcept { return owner_id; }
    void set_owner_type(OwnerValue value) { owner_type = std::move(value); }
    [[nodiscard]] const OwnerValue& get_owner_type() const noexcept { return owner_type; }
    void set_auth_type(UserAuthTypeValue value) { auth_type = std::move(value); }
    [[nodiscard]] const UserAuthTypeValue& get_auth_type() const noexcept { return auth_type; }
    void set_email_domain(std::optional<std::string> value) { email_domain = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_email_domain() const noexcept { return email_domain; }
    void set_allow_signup(bool value) { allow_signup = std::move(value); }
    [[nodiscard]] const bool& get_allow_signup() const noexcept { return allow_signup; }
    std::string id{};
    std::string auth_id{};
    std::string owner_id{};
    OwnerValue owner_type{};
    UserAuthTypeValue auth_type{};
    std::optional<std::string> email_domain{};
    bool allow_signup{};
};

class EmbeddedTokenInfoResponse
{
public:
    void set_org_id(OrganizationIdValue value) { org_id = std::move(value); }
    [[nodiscard]] const OrganizationIdValue& get_org_id() const noexcept { return org_id; }
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_merchant_account_version(ApiVersionValue value) { merchant_account_version = std::move(value); }
    [[nodiscard]] const ApiVersionValue& get_merchant_account_version() const noexcept { return merchant_account_version; }
    void set_profile_id(ProfileIdValue value) { profile_id = std::move(value); }
    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept { return profile_id; }
    OrganizationIdValue org_id{};
    MerchantIdValue merchant_id{};
    ApiVersionValue merchant_account_version{};
    ProfileIdValue profile_id{};
};

class ForgotPasswordRequest
{
public:
    void set_email(EmailValue value) { email = std::move(value); }
    [[nodiscard]] const EmailValue& get_email() const noexcept { return email; }
    EmailValue email{};
};

class GetUserDetailsResponse
{
public:
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_email(EmailValue value) { email = std::move(value); }
    [[nodiscard]] const EmailValue& get_email() const noexcept { return email; }
    void set_verification_days_left(std::optional<std::int64_t> value) { verification_days_left = std::move(value); }
    [[nodiscard]] const std::optional<std::int64_t>& get_verification_days_left() const noexcept { return verification_days_left; }
    void set_role_id(std::string value) { role_id = std::move(value); }
    [[nodiscard]] const std::string& get_role_id() const noexcept { return role_id; }
    void set_user_id(std::string value) { user_id = std::move(value); }
    [[nodiscard]] const std::string& get_user_id() const noexcept { return user_id; }
    void set_org_id(OrganizationIdValue value) { org_id = std::move(value); }
    [[nodiscard]] const OrganizationIdValue& get_org_id() const noexcept { return org_id; }
    void set_is_two_factor_auth_setup(bool value) { is_two_factor_auth_setup = std::move(value); }
    [[nodiscard]] const bool& get_is_two_factor_auth_setup() const noexcept { return is_two_factor_auth_setup; }
    void set_recovery_codes_left(std::optional<std::size_t> value) { recovery_codes_left = std::move(value); }
    [[nodiscard]] const std::optional<std::size_t>& get_recovery_codes_left() const noexcept { return recovery_codes_left; }
    void set_profile_id(ProfileIdValue value) { profile_id = std::move(value); }
    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept { return profile_id; }
    void set_entity_type(EntityTypeValue value) { entity_type = std::move(value); }
    [[nodiscard]] const EntityTypeValue& get_entity_type() const noexcept { return entity_type; }
    void set_theme_id(std::optional<std::string> value) { theme_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_theme_id() const noexcept { return theme_id; }
    void set_version(ApiVersionValue value) { version = std::move(value); }
    [[nodiscard]] const ApiVersionValue& get_version() const noexcept { return version; }
    void set_offer_engine_credential_source(OfferEngineCredentialSourceValue value) { offer_engine_credential_source = std::move(value); }
    [[nodiscard]] const OfferEngineCredentialSourceValue& get_offer_engine_credential_source() const noexcept { return offer_engine_credential_source; }
    MerchantIdValue merchant_id{};
    std::string name{};
    EmailValue email{};
    std::optional<std::int64_t> verification_days_left{};
    std::string role_id{};
    std::string user_id{};
    OrganizationIdValue org_id{};
    bool is_two_factor_auth_setup{};
    std::optional<std::size_t> recovery_codes_left{};
    ProfileIdValue profile_id{};
    EntityTypeValue entity_type{};
    std::optional<std::string> theme_id{};
    ApiVersionValue version{};
    OfferEngineCredentialSourceValue offer_engine_credential_source{};
};

class GetUserInternalDetailsResponse
{
public:
    void set_user_id(std::string value) { user_id = std::move(value); }
    [[nodiscard]] const std::string& get_user_id() const noexcept { return user_id; }
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_email(EmailValue value) { email = std::move(value); }
    [[nodiscard]] const EmailValue& get_email() const noexcept { return email; }
    void set_is_active(bool value) { is_active = std::move(value); }
    [[nodiscard]] const bool& get_is_active() const noexcept { return is_active; }
    std::string user_id{};
    std::string name{};
    EmailValue email{};
    bool is_active{};
};

class GetUserRoleDetailsRequest
{
public:
    void set_email(EmailValue value) { email = std::move(value); }
    [[nodiscard]] const EmailValue& get_email() const noexcept { return email; }
    EmailValue email{};
};

class GetUserRoleDetailsResponseV2
{
public:
    void set_role_id(std::string value) { role_id = std::move(value); }
    [[nodiscard]] const std::string& get_role_id() const noexcept { return role_id; }
    void set_org(NameIdUnit value) { org = std::move(value); }
    [[nodiscard]] const NameIdUnit& get_org() const noexcept { return org; }
    void set_merchant(std::optional<NameIdUnit> value) { merchant = std::move(value); }
    [[nodiscard]] const std::optional<NameIdUnit>& get_merchant() const noexcept { return merchant; }
    void set_profile(std::optional<NameIdUnit> value) { profile = std::move(value); }
    [[nodiscard]] const std::optional<NameIdUnit>& get_profile() const noexcept { return profile; }
    void set_status(UserStatusValue value) { status = std::move(value); }
    [[nodiscard]] const UserStatusValue& get_status() const noexcept { return status; }
    void set_entity_type(EntityTypeValue value) { entity_type = std::move(value); }
    [[nodiscard]] const EntityTypeValue& get_entity_type() const noexcept { return entity_type; }
    void set_role_name(std::string value) { role_name = std::move(value); }
    [[nodiscard]] const std::string& get_role_name() const noexcept { return role_name; }
    std::string role_id{};
    NameIdUnit org{};
    std::optional<NameIdUnit> merchant{};
    std::optional<NameIdUnit> profile{};
    UserStatusValue status{};
    EntityTypeValue entity_type{};
    std::string role_name{};
};

class InviteMultipleUserResponse
{
public:
    void set_email(EmailValue value) { email = std::move(value); }
    [[nodiscard]] const EmailValue& get_email() const noexcept { return email; }
    void set_is_email_sent(bool value) { is_email_sent = std::move(value); }
    [[nodiscard]] const bool& get_is_email_sent() const noexcept { return is_email_sent; }
    void set_password(std::optional<std::string> value) { password = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_password() const noexcept { return password; }
    void set_error(std::optional<std::string> value) { error = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error() const noexcept { return error; }
    EmailValue email{};
    bool is_email_sent{};
    std::optional<std::string> password{};
    std::optional<std::string> error{};
};

class InviteUserRequest
{
public:
    void set_email(EmailValue value) { email = std::move(value); }
    [[nodiscard]] const EmailValue& get_email() const noexcept { return email; }
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_role_id(std::string value) { role_id = std::move(value); }
    [[nodiscard]] const std::string& get_role_id() const noexcept { return role_id; }
    EmailValue email{};
    std::string name{};
    std::string role_id{};
};

class ListMembersQueryParam
{
public:
    void set_access_level(EntityTypeValue value) { access_level = std::move(value); }
    [[nodiscard]] const EntityTypeValue& get_access_level() const noexcept { return access_level; }
    EntityTypeValue access_level{};
};

class ListOrgsForUserResponse
{
public:
    void set_org_id(OrganizationIdValue value) { org_id = std::move(value); }
    [[nodiscard]] const OrganizationIdValue& get_org_id() const noexcept { return org_id; }
    void set_org_name(std::optional<std::string> value) { org_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_org_name() const noexcept { return org_name; }
    void set_org_type(OrganizationTypeValue value) { org_type = std::move(value); }
    [[nodiscard]] const OrganizationTypeValue& get_org_type() const noexcept { return org_type; }
    OrganizationIdValue org_id{};
    std::optional<std::string> org_name{};
    OrganizationTypeValue org_type{};
};

class ListProfilesForUserInOrgAndMerchantAccountResponse
{
public:
    void set_profile_id(ProfileIdValue value) { profile_id = std::move(value); }
    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept { return profile_id; }
    void set_profile_name(std::string value) { profile_name = std::move(value); }
    [[nodiscard]] const std::string& get_profile_name() const noexcept { return profile_name; }
    ProfileIdValue profile_id{};
    std::string profile_name{};
};

class OpenIdConnect
{
public:
    void set_name(OpenIdProvider value) { name = std::move(value); }
    [[nodiscard]] const OpenIdProvider& get_name() const noexcept { return name; }
    void set_base_url(std::string value) { base_url = std::move(value); }
    [[nodiscard]] const std::string& get_base_url() const noexcept { return base_url; }
    void set_client_id(std::string value) { client_id = std::move(value); }
    [[nodiscard]] const std::string& get_client_id() const noexcept { return client_id; }
    void set_client_secret(std::string value) { client_secret = std::move(value); }
    [[nodiscard]] const std::string& get_client_secret() const noexcept { return client_secret; }
    void set_private_key(std::optional<std::string> value) { private_key = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_private_key() const noexcept { return private_key; }
    OpenIdProvider name{};
    std::string base_url{};
    std::string client_id{};
    std::string client_secret{};
    std::optional<std::string> private_key{};
};

class OpenIdConnectPublicConfig
{
public:
    void set_name(OpenIdProvider value) { name = std::move(value); }
    [[nodiscard]] const OpenIdProvider& get_name() const noexcept { return name; }
    OpenIdProvider name{};
};

class PlatformAccountCreateResponse
{
public:
    void set_org_id(OrganizationIdValue value) { org_id = std::move(value); }
    [[nodiscard]] const OrganizationIdValue& get_org_id() const noexcept { return org_id; }
    void set_org_name(std::optional<std::string> value) { org_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_org_name() const noexcept { return org_name; }
    void set_org_type(OrganizationTypeValue value) { org_type = std::move(value); }
    [[nodiscard]] const OrganizationTypeValue& get_org_type() const noexcept { return org_type; }
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_merchant_account_type(MerchantAccountTypeValue value) { merchant_account_type = std::move(value); }
    [[nodiscard]] const MerchantAccountTypeValue& get_merchant_account_type() const noexcept { return merchant_account_type; }
    OrganizationIdValue org_id{};
    std::optional<std::string> org_name{};
    OrganizationTypeValue org_type{};
    MerchantIdValue merchant_id{};
    MerchantAccountTypeValue merchant_account_type{};
};

class ReInviteUserRequest
{
public:
    void set_email(EmailValue value) { email = std::move(value); }
    [[nodiscard]] const EmailValue& get_email() const noexcept { return email; }
    EmailValue email{};
};

class SendVerifyEmailRequest
{
public:
    void set_email(EmailValue value) { email = std::move(value); }
    [[nodiscard]] const EmailValue& get_email() const noexcept { return email; }
    EmailValue email{};
};

class SignUpRequest
{
public:
    void set_email(EmailValue value) { email = std::move(value); }
    [[nodiscard]] const EmailValue& get_email() const noexcept { return email; }
    void set_password(std::string value) { password = std::move(value); }
    [[nodiscard]] const std::string& get_password() const noexcept { return password; }
    EmailValue email{};
    std::string password{};
};

class SignUpWithMerchantIdRequest
{
public:
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_email(EmailValue value) { email = std::move(value); }
    [[nodiscard]] const EmailValue& get_email() const noexcept { return email; }
    void set_password(std::string value) { password = std::move(value); }
    [[nodiscard]] const std::string& get_password() const noexcept { return password; }
    void set_company_name(std::string value) { company_name = std::move(value); }
    [[nodiscard]] const std::string& get_company_name() const noexcept { return company_name; }
    void set_organization_type(std::optional<OrganizationTypeValue> value) { organization_type = std::move(value); }
    [[nodiscard]] const std::optional<OrganizationTypeValue>& get_organization_type() const noexcept { return organization_type; }
    std::string name{};
    EmailValue email{};
    std::string password{};
    std::string company_name{};
    std::optional<OrganizationTypeValue> organization_type{};
};

class SwitchMerchantRequest
{
public:
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    MerchantIdValue merchant_id{};
};

class SwitchOrganizationRequest
{
public:
    void set_org_id(OrganizationIdValue value) { org_id = std::move(value); }
    [[nodiscard]] const OrganizationIdValue& get_org_id() const noexcept { return org_id; }
    OrganizationIdValue org_id{};
};

class SwitchProfileRequest
{
public:
    void set_profile_id(ProfileIdValue value) { profile_id = std::move(value); }
    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept { return profile_id; }
    ProfileIdValue profile_id{};
};

class TokenResponse
{
public:
    void set_token(std::string value) { token = std::move(value); }
    [[nodiscard]] const std::string& get_token() const noexcept { return token; }
    void set_token_type(TokenPurposeValue value) { token_type = std::move(value); }
    [[nodiscard]] const TokenPurposeValue& get_token_type() const noexcept { return token_type; }
    std::string token{};
    TokenPurposeValue token_type{};
};

class TwoFactorAuthStatusResponseWithAttempts
{
public:
    void set_totp(TwoFactorAuthAttempts value) { totp = std::move(value); }
    [[nodiscard]] const TwoFactorAuthAttempts& get_totp() const noexcept { return totp; }
    void set_recovery_code(TwoFactorAuthAttempts value) { recovery_code = std::move(value); }
    [[nodiscard]] const TwoFactorAuthAttempts& get_recovery_code() const noexcept { return recovery_code; }
    TwoFactorAuthAttempts totp{};
    TwoFactorAuthAttempts recovery_code{};
};

class UserMerchantAccountResponse
{
public:
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_merchant_name(OptionalEncryptableNameValue value) { merchant_name = std::move(value); }
    [[nodiscard]] const OptionalEncryptableNameValue& get_merchant_name() const noexcept { return merchant_name; }
    void set_product_type(std::optional<MerchantProductTypeValue> value) { product_type = std::move(value); }
    [[nodiscard]] const std::optional<MerchantProductTypeValue>& get_product_type() const noexcept { return product_type; }
    void set_merchant_account_type(MerchantAccountTypeValue value) { merchant_account_type = std::move(value); }
    [[nodiscard]] const MerchantAccountTypeValue& get_merchant_account_type() const noexcept { return merchant_account_type; }
    void set_version(ApiVersionValue value) { version = std::move(value); }
    [[nodiscard]] const ApiVersionValue& get_version() const noexcept { return version; }
    MerchantIdValue merchant_id{};
    OptionalEncryptableNameValue merchant_name{};
    std::optional<MerchantProductTypeValue> product_type{};
    MerchantAccountTypeValue merchant_account_type{};
    ApiVersionValue version{};
};

class UserMerchantCreate
{
public:
    void set_company_name(std::string value) { company_name = std::move(value); }
    [[nodiscard]] const std::string& get_company_name() const noexcept { return company_name; }
    void set_product_type(std::optional<MerchantProductTypeValue> value) { product_type = std::move(value); }
    [[nodiscard]] const std::optional<MerchantProductTypeValue>& get_product_type() const noexcept { return product_type; }
    void set_merchant_account_type(std::optional<MerchantAccountRequestTypeValue> value) { merchant_account_type = std::move(value); }
    [[nodiscard]] const std::optional<MerchantAccountRequestTypeValue>& get_merchant_account_type() const noexcept { return merchant_account_type; }
    std::string company_name{};
    std::optional<MerchantProductTypeValue> product_type{};
    std::optional<MerchantAccountRequestTypeValue> merchant_account_type{};
};

class UserMerchantDetailsResponse
{
public:
    void set_product_type(std::optional<MerchantProductTypeValue> value) { product_type = std::move(value); }
    [[nodiscard]] const std::optional<MerchantProductTypeValue>& get_product_type() const noexcept { return product_type; }
    void set_merchant_account_type(MerchantAccountTypeValue value) { merchant_account_type = std::move(value); }
    [[nodiscard]] const MerchantAccountTypeValue& get_merchant_account_type() const noexcept { return merchant_account_type; }
    std::optional<MerchantProductTypeValue> product_type{};
    MerchantAccountTypeValue merchant_account_type{};
};

class UserOrgMerchantCreateRequest
{
public:
    void set_organization_name(std::string value) { organization_name = std::move(value); }
    [[nodiscard]] const std::string& get_organization_name() const noexcept { return organization_name; }
    void set_organization_details(std::optional<SecretSerdeValueValue> value) { organization_details = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_organization_details() const noexcept { return organization_details; }
    void set_metadata(std::optional<SecretSerdeValueValue> value) { metadata = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept { return metadata; }
    void set_merchant_name(std::string value) { merchant_name = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_name() const noexcept { return merchant_name; }
    std::string organization_name{};
    std::optional<SecretSerdeValueValue> organization_details{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::string merchant_name{};
};

enum class AuthConfig
{
    OpenIdConnect,
    MagicLink,
    Password,
};

class ListUsersInternalResponse
{
public:
    void set_users(std::vector<GetUserInternalDetailsResponse> value) { users = std::move(value); }
    [[nodiscard]] const std::vector<GetUserInternalDetailsResponse>& get_users() const noexcept { return users; }
    std::vector<GetUserInternalDetailsResponse> users{};
};

class TwoFactorStatus
{
public:
    void set_status(std::optional<TwoFactorAuthStatusResponseWithAttempts> value) { status = std::move(value); }
    [[nodiscard]] const std::optional<TwoFactorAuthStatusResponseWithAttempts>& get_status() const noexcept { return status; }
    void set_is_skippable(bool value) { is_skippable = std::move(value); }
    [[nodiscard]] const bool& get_is_skippable() const noexcept { return is_skippable; }
    std::optional<TwoFactorAuthStatusResponseWithAttempts> status{};
    bool is_skippable{};
};

class UserAuthenticationMethodResponse
{
public:
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_auth_id(std::string value) { auth_id = std::move(value); }
    [[nodiscard]] const std::string& get_auth_id() const noexcept { return auth_id; }
    void set_auth_method(AuthMethodDetails value) { auth_method = std::move(value); }
    [[nodiscard]] const AuthMethodDetails& get_auth_method() const noexcept { return auth_method; }
    void set_allow_signup(bool value) { allow_signup = std::move(value); }
    [[nodiscard]] const bool& get_allow_signup() const noexcept { return allow_signup; }
    std::string id{};
    std::string auth_id{};
    AuthMethodDetails auth_method{};
    bool allow_signup{};
};

class CreateUserAuthenticationMethodRequest
{
public:
    void set_owner_id(std::string value) { owner_id = std::move(value); }
    [[nodiscard]] const std::string& get_owner_id() const noexcept { return owner_id; }
    void set_owner_type(OwnerValue value) { owner_type = std::move(value); }
    [[nodiscard]] const OwnerValue& get_owner_type() const noexcept { return owner_type; }
    void set_auth_method(AuthConfig value) { auth_method = std::move(value); }
    [[nodiscard]] const AuthConfig& get_auth_method() const noexcept { return auth_method; }
    void set_allow_signup(bool value) { allow_signup = std::move(value); }
    [[nodiscard]] const bool& get_allow_signup() const noexcept { return allow_signup; }
    void set_email_domain(std::optional<std::string> value) { email_domain = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_email_domain() const noexcept { return email_domain; }
    std::string owner_id{};
    OwnerValue owner_type{};
    AuthConfig auth_method{};
    bool allow_signup{};
    std::optional<std::string> email_domain{};
};
} // namespace app::model::user

template<>
struct serde::Serializable<app::model::user::SignUpWithMerchantIdRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"name", &app::model::user::SignUpWithMerchantIdRequest::get_name, &app::model::user::SignUpWithMerchantIdRequest::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &app::model::user::SignUpWithMerchantIdRequest::get_email, &app::model::user::SignUpWithMerchantIdRequest::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"password", &app::model::user::SignUpWithMerchantIdRequest::get_password, &app::model::user::SignUpWithMerchantIdRequest::set_password, serde::FieldOptions{}>{},
            serde::FieldDesc<"company_name", &app::model::user::SignUpWithMerchantIdRequest::get_company_name, &app::model::user::SignUpWithMerchantIdRequest::set_company_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"organization_type", &app::model::user::SignUpWithMerchantIdRequest::get_organization_type, &app::model::user::SignUpWithMerchantIdRequest::set_organization_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "sign_up_with_merchant_id_request"; }
};

template<>
struct serde::Serializable<app::model::user::SignUpRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"email", &app::model::user::SignUpRequest::get_email, &app::model::user::SignUpRequest::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"password", &app::model::user::SignUpRequest::get_password, &app::model::user::SignUpRequest::set_password, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "sign_up_request"; }
};

template<>
struct serde::Serializable<app::model::user::ConnectAccountRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"email", &app::model::user::ConnectAccountRequest::get_email, &app::model::user::ConnectAccountRequest::set_email, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "connect_account_request"; }
};

template<>
struct serde::Serializable<app::model::user::AuthorizeResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"is_email_sent", &app::model::user::AuthorizeResponse::get_is_email_sent, &app::model::user::AuthorizeResponse::set_is_email_sent, serde::FieldOptions{}>{},
            serde::FieldDesc<"user_id", &app::model::user::AuthorizeResponse::get_user_id, &app::model::user::AuthorizeResponse::set_user_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authorize_response"; }
};

template<>
struct serde::Serializable<app::model::user::ChangePasswordRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"new_password", &app::model::user::ChangePasswordRequest::get_new_password, &app::model::user::ChangePasswordRequest::set_new_password, serde::FieldOptions{}>{},
            serde::FieldDesc<"old_password", &app::model::user::ChangePasswordRequest::get_old_password, &app::model::user::ChangePasswordRequest::set_old_password, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "change_password_request"; }
};

template<>
struct serde::Serializable<app::model::user::ForgotPasswordRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"email", &app::model::user::ForgotPasswordRequest::get_email, &app::model::user::ForgotPasswordRequest::set_email, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "forgot_password_request"; }
};

template<>
struct serde::Serializable<app::model::user::ResetPasswordRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"token", &app::model::user::ResetPasswordRequest::get_token, &app::model::user::ResetPasswordRequest::set_token, serde::FieldOptions{}>{},
            serde::FieldDesc<"password", &app::model::user::ResetPasswordRequest::get_password, &app::model::user::ResetPasswordRequest::set_password, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "reset_password_request"; }
};

template<>
struct serde::Serializable<app::model::user::RotatePasswordRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"password", &app::model::user::RotatePasswordRequest::get_password, &app::model::user::RotatePasswordRequest::set_password, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "rotate_password_request"; }
};

template<>
struct serde::Serializable<app::model::user::InviteUserRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"email", &app::model::user::InviteUserRequest::get_email, &app::model::user::InviteUserRequest::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &app::model::user::InviteUserRequest::get_name, &app::model::user::InviteUserRequest::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"role_id", &app::model::user::InviteUserRequest::get_role_id, &app::model::user::InviteUserRequest::set_role_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "invite_user_request"; }
};

template<>
struct serde::Serializable<app::model::user::InviteMultipleUserResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"email", &app::model::user::InviteMultipleUserResponse::get_email, &app::model::user::InviteMultipleUserResponse::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_email_sent", &app::model::user::InviteMultipleUserResponse::get_is_email_sent, &app::model::user::InviteMultipleUserResponse::set_is_email_sent, serde::FieldOptions{}>{},
            serde::FieldDesc<"password", &app::model::user::InviteMultipleUserResponse::get_password, &app::model::user::InviteMultipleUserResponse::set_password, serde::FieldOptions{}>{},
            serde::FieldDesc<"error", &app::model::user::InviteMultipleUserResponse::get_error, &app::model::user::InviteMultipleUserResponse::set_error, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "invite_multiple_user_response"; }
};

template<>
struct serde::Serializable<app::model::user::ReInviteUserRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"email", &app::model::user::ReInviteUserRequest::get_email, &app::model::user::ReInviteUserRequest::set_email, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "re_invite_user_request"; }
};

template<>
struct serde::Serializable<app::model::user::AcceptInviteFromEmailRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"token", &app::model::user::AcceptInviteFromEmailRequest::get_token, &app::model::user::AcceptInviteFromEmailRequest::set_token, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "accept_invite_from_email_request"; }
};

template<>
struct serde::Serializable<app::model::user::ValidateOnlyQueryParam>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"status_check", &app::model::user::ValidateOnlyQueryParam::get_status_check, &app::model::user::ValidateOnlyQueryParam::set_status_check, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "validate_only_query_param"; }
};

template<>
struct serde::Serializable<app::model::user::SwitchOrganizationRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"org_id", &app::model::user::SwitchOrganizationRequest::get_org_id, &app::model::user::SwitchOrganizationRequest::set_org_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "switch_organization_request"; }
};

template<>
struct serde::Serializable<app::model::user::SwitchMerchantRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_id", &app::model::user::SwitchMerchantRequest::get_merchant_id, &app::model::user::SwitchMerchantRequest::set_merchant_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "switch_merchant_request"; }
};

template<>
struct serde::Serializable<app::model::user::SwitchProfileRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"profile_id", &app::model::user::SwitchProfileRequest::get_profile_id, &app::model::user::SwitchProfileRequest::set_profile_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "switch_profile_request"; }
};

template<>
struct serde::Serializable<app::model::user::CloneConnectorRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"source_mca_id", &app::model::user::CloneConnectorRequest::get_source_mca_id, &app::model::user::CloneConnectorRequest::set_source_mca_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"destination_profile_id", &app::model::user::CloneConnectorRequest::get_destination_profile_id, &app::model::user::CloneConnectorRequest::set_destination_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_label", &app::model::user::CloneConnectorRequest::get_connector_label, &app::model::user::CloneConnectorRequest::set_connector_label, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "clone_connector_request"; }
};

template<>
struct serde::Serializable<app::model::user::CreateInternalUserRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"name", &app::model::user::CreateInternalUserRequest::get_name, &app::model::user::CreateInternalUserRequest::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &app::model::user::CreateInternalUserRequest::get_email, &app::model::user::CreateInternalUserRequest::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"password", &app::model::user::CreateInternalUserRequest::get_password, &app::model::user::CreateInternalUserRequest::set_password, serde::FieldOptions{}>{},
            serde::FieldDesc<"role_id", &app::model::user::CreateInternalUserRequest::get_role_id, &app::model::user::CreateInternalUserRequest::set_role_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "create_internal_user_request"; }
};

template<>
struct serde::Serializable<app::model::user::CreateTenantUserRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"name", &app::model::user::CreateTenantUserRequest::get_name, &app::model::user::CreateTenantUserRequest::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &app::model::user::CreateTenantUserRequest::get_email, &app::model::user::CreateTenantUserRequest::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"password", &app::model::user::CreateTenantUserRequest::get_password, &app::model::user::CreateTenantUserRequest::set_password, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "create_tenant_user_request"; }
};

template<>
struct serde::Serializable<app::model::user::UserOrgMerchantCreateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"organization_name", &app::model::user::UserOrgMerchantCreateRequest::get_organization_name, &app::model::user::UserOrgMerchantCreateRequest::set_organization_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"organization_details", &app::model::user::UserOrgMerchantCreateRequest::get_organization_details, &app::model::user::UserOrgMerchantCreateRequest::set_organization_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"metadata", &app::model::user::UserOrgMerchantCreateRequest::get_metadata, &app::model::user::UserOrgMerchantCreateRequest::set_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_name", &app::model::user::UserOrgMerchantCreateRequest::get_merchant_name, &app::model::user::UserOrgMerchantCreateRequest::set_merchant_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "user_org_merchant_create_request"; }
};

template<>
struct serde::Serializable<app::model::user::PlatformAccountCreateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"organization_name", &app::model::user::PlatformAccountCreateRequest::get_organization_name, &app::model::user::PlatformAccountCreateRequest::set_organization_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "platform_account_create_request"; }
};

template<>
struct serde::Serializable<app::model::user::PlatformAccountCreateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"org_id", &app::model::user::PlatformAccountCreateResponse::get_org_id, &app::model::user::PlatformAccountCreateResponse::set_org_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"org_name", &app::model::user::PlatformAccountCreateResponse::get_org_name, &app::model::user::PlatformAccountCreateResponse::set_org_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"org_type", &app::model::user::PlatformAccountCreateResponse::get_org_type, &app::model::user::PlatformAccountCreateResponse::set_org_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::model::user::PlatformAccountCreateResponse::get_merchant_id, &app::model::user::PlatformAccountCreateResponse::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_account_type", &app::model::user::PlatformAccountCreateResponse::get_merchant_account_type, &app::model::user::PlatformAccountCreateResponse::set_merchant_account_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "platform_account_create_response"; }
};

template<>
struct serde::Serializable<app::model::user::UserMerchantCreate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"company_name", &app::model::user::UserMerchantCreate::get_company_name, &app::model::user::UserMerchantCreate::set_company_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"product_type", &app::model::user::UserMerchantCreate::get_product_type, &app::model::user::UserMerchantCreate::set_product_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_account_type", &app::model::user::UserMerchantCreate::get_merchant_account_type, &app::model::user::UserMerchantCreate::set_merchant_account_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "user_merchant_create"; }
};

template<>
struct serde::Serializable<app::model::user::GetUserDetailsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_id", &app::model::user::GetUserDetailsResponse::get_merchant_id, &app::model::user::GetUserDetailsResponse::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &app::model::user::GetUserDetailsResponse::get_name, &app::model::user::GetUserDetailsResponse::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &app::model::user::GetUserDetailsResponse::get_email, &app::model::user::GetUserDetailsResponse::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"verification_days_left", &app::model::user::GetUserDetailsResponse::get_verification_days_left, &app::model::user::GetUserDetailsResponse::set_verification_days_left, serde::FieldOptions{}>{},
            serde::FieldDesc<"role_id", &app::model::user::GetUserDetailsResponse::get_role_id, &app::model::user::GetUserDetailsResponse::set_role_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"user_id", &app::model::user::GetUserDetailsResponse::get_user_id, &app::model::user::GetUserDetailsResponse::set_user_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"org_id", &app::model::user::GetUserDetailsResponse::get_org_id, &app::model::user::GetUserDetailsResponse::set_org_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_two_factor_auth_setup", &app::model::user::GetUserDetailsResponse::get_is_two_factor_auth_setup, &app::model::user::GetUserDetailsResponse::set_is_two_factor_auth_setup, serde::FieldOptions{}>{},
            serde::FieldDesc<"recovery_codes_left", &app::model::user::GetUserDetailsResponse::get_recovery_codes_left, &app::model::user::GetUserDetailsResponse::set_recovery_codes_left, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::model::user::GetUserDetailsResponse::get_profile_id, &app::model::user::GetUserDetailsResponse::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"entity_type", &app::model::user::GetUserDetailsResponse::get_entity_type, &app::model::user::GetUserDetailsResponse::set_entity_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"theme_id", &app::model::user::GetUserDetailsResponse::get_theme_id, &app::model::user::GetUserDetailsResponse::set_theme_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"version", &app::model::user::GetUserDetailsResponse::get_version, &app::model::user::GetUserDetailsResponse::set_version, serde::FieldOptions{}>{},
            serde::FieldDesc<"offer_engine_credential_source", &app::model::user::GetUserDetailsResponse::get_offer_engine_credential_source, &app::model::user::GetUserDetailsResponse::set_offer_engine_credential_source, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "get_user_details_response"; }
};

template<>
struct serde::Serializable<app::model::user::GetUserRoleDetailsRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"email", &app::model::user::GetUserRoleDetailsRequest::get_email, &app::model::user::GetUserRoleDetailsRequest::set_email, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "get_user_role_details_request"; }
};

template<>
struct serde::Serializable<app::model::user::NameIdUnit>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"name", &app::model::user::NameIdUnit::get_name, &app::model::user::NameIdUnit::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"id", &app::model::user::NameIdUnit::get_id, &app::model::user::NameIdUnit::set_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "name_id_unit"; }
};

template<>
struct serde::Serializable<app::model::user::GetUserRoleDetailsResponseV2>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"role_id", &app::model::user::GetUserRoleDetailsResponseV2::get_role_id, &app::model::user::GetUserRoleDetailsResponseV2::set_role_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"org", &app::model::user::GetUserRoleDetailsResponseV2::get_org, &app::model::user::GetUserRoleDetailsResponseV2::set_org, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant", &app::model::user::GetUserRoleDetailsResponseV2::get_merchant, &app::model::user::GetUserRoleDetailsResponseV2::set_merchant, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile", &app::model::user::GetUserRoleDetailsResponseV2::get_profile, &app::model::user::GetUserRoleDetailsResponseV2::set_profile, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::model::user::GetUserRoleDetailsResponseV2::get_status, &app::model::user::GetUserRoleDetailsResponseV2::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"entity_type", &app::model::user::GetUserRoleDetailsResponseV2::get_entity_type, &app::model::user::GetUserRoleDetailsResponseV2::set_entity_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"role_name", &app::model::user::GetUserRoleDetailsResponseV2::get_role_name, &app::model::user::GetUserRoleDetailsResponseV2::set_role_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "get_user_role_details_response_v2"; }
};

template<>
struct serde::Serializable<app::model::user::VerifyEmailRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"token", &app::model::user::VerifyEmailRequest::get_token, &app::model::user::VerifyEmailRequest::set_token, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "verify_email_request"; }
};

template<>
struct serde::Serializable<app::model::user::SendVerifyEmailRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"email", &app::model::user::SendVerifyEmailRequest::get_email, &app::model::user::SendVerifyEmailRequest::set_email, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "send_verify_email_request"; }
};

template<>
struct serde::Serializable<app::model::user::UpdateUserAccountDetailsRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"name", &app::model::user::UpdateUserAccountDetailsRequest::get_name, &app::model::user::UpdateUserAccountDetailsRequest::set_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "update_user_account_details_request"; }
};

template<>
struct serde::Serializable<app::model::user::SkipTwoFactorAuthQueryParam>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"skip_two_factor_auth", &app::model::user::SkipTwoFactorAuthQueryParam::get_skip_two_factor_auth, &app::model::user::SkipTwoFactorAuthQueryParam::set_skip_two_factor_auth, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "skip_two_factor_auth_query_param"; }
};

template<>
struct serde::Serializable<app::model::user::TokenResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"token", &app::model::user::TokenResponse::get_token, &app::model::user::TokenResponse::set_token, serde::FieldOptions{}>{},
            serde::FieldDesc<"token_type", &app::model::user::TokenResponse::get_token_type, &app::model::user::TokenResponse::set_token_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "token_response"; }
};

template<>
struct serde::Serializable<app::model::user::TwoFactorAuthStatusResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"totp", &app::model::user::TwoFactorAuthStatusResponse::get_totp, &app::model::user::TwoFactorAuthStatusResponse::set_totp, serde::FieldOptions{}>{},
            serde::FieldDesc<"recovery_code", &app::model::user::TwoFactorAuthStatusResponse::get_recovery_code, &app::model::user::TwoFactorAuthStatusResponse::set_recovery_code, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "two_factor_auth_status_response"; }
};

template<>
struct serde::Serializable<app::model::user::TwoFactorAuthAttempts>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"is_completed", &app::model::user::TwoFactorAuthAttempts::get_is_completed, &app::model::user::TwoFactorAuthAttempts::set_is_completed, serde::FieldOptions{}>{},
            serde::FieldDesc<"remaining_attempts", &app::model::user::TwoFactorAuthAttempts::get_remaining_attempts, &app::model::user::TwoFactorAuthAttempts::set_remaining_attempts, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "two_factor_auth_attempts"; }
};

template<>
struct serde::Serializable<app::model::user::TwoFactorAuthStatusResponseWithAttempts>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"totp", &app::model::user::TwoFactorAuthStatusResponseWithAttempts::get_totp, &app::model::user::TwoFactorAuthStatusResponseWithAttempts::set_totp, serde::FieldOptions{}>{},
            serde::FieldDesc<"recovery_code", &app::model::user::TwoFactorAuthStatusResponseWithAttempts::get_recovery_code, &app::model::user::TwoFactorAuthStatusResponseWithAttempts::set_recovery_code, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "two_factor_auth_status_response_with_attempts"; }
};

template<>
struct serde::Serializable<app::model::user::TwoFactorStatus>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"status", &app::model::user::TwoFactorStatus::get_status, &app::model::user::TwoFactorStatus::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_skippable", &app::model::user::TwoFactorStatus::get_is_skippable, &app::model::user::TwoFactorStatus::set_is_skippable, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "two_factor_status"; }
};

template<>
struct serde::Serializable<app::model::user::UserFromEmailRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"token", &app::model::user::UserFromEmailRequest::get_token, &app::model::user::UserFromEmailRequest::set_token, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "user_from_email_request"; }
};

template<>
struct serde::Serializable<app::model::user::TotpSecret>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"secret", &app::model::user::TotpSecret::get_secret, &app::model::user::TotpSecret::set_secret, serde::FieldOptions{}>{},
            serde::FieldDesc<"totp_url", &app::model::user::TotpSecret::get_totp_url, &app::model::user::TotpSecret::set_totp_url, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "totp_secret"; }
};

template<>
struct serde::Serializable<app::model::user::BeginTotpResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"secret", &app::model::user::BeginTotpResponse::get_secret, &app::model::user::BeginTotpResponse::set_secret, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "begin_totp_response"; }
};

template<>
struct serde::Serializable<app::model::user::VerifyTotpRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"totp", &app::model::user::VerifyTotpRequest::get_totp, &app::model::user::VerifyTotpRequest::set_totp, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "verify_totp_request"; }
};

template<>
struct serde::Serializable<app::model::user::VerifyRecoveryCodeRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"recovery_code", &app::model::user::VerifyRecoveryCodeRequest::get_recovery_code, &app::model::user::VerifyRecoveryCodeRequest::set_recovery_code, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "verify_recovery_code_request"; }
};

template<>
struct serde::Serializable<app::model::user::RecoveryCodes>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"recovery_codes", &app::model::user::RecoveryCodes::get_recovery_codes, &app::model::user::RecoveryCodes::set_recovery_codes, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "recovery_codes"; }
};

template<>
struct serde::Serializable<app::model::user::OpenIdConnectPrivateConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"base_url", &app::model::user::OpenIdConnectPrivateConfig::get_base_url, &app::model::user::OpenIdConnectPrivateConfig::set_base_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"client_id", &app::model::user::OpenIdConnectPrivateConfig::get_client_id, &app::model::user::OpenIdConnectPrivateConfig::set_client_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"client_secret", &app::model::user::OpenIdConnectPrivateConfig::get_client_secret, &app::model::user::OpenIdConnectPrivateConfig::set_client_secret, serde::FieldOptions{}>{},
            serde::FieldDesc<"private_key", &app::model::user::OpenIdConnectPrivateConfig::get_private_key, &app::model::user::OpenIdConnectPrivateConfig::set_private_key, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "open_id_connect_private_config"; }
};

template<>
struct serde::Serializable<app::model::user::OpenIdConnectPublicConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"name", &app::model::user::OpenIdConnectPublicConfig::get_name, &app::model::user::OpenIdConnectPublicConfig::set_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "open_id_connect_public_config"; }
};

template<>
struct serde::Serializable<app::model::user::OpenIdConnect>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"name", &app::model::user::OpenIdConnect::get_name, &app::model::user::OpenIdConnect::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"base_url", &app::model::user::OpenIdConnect::get_base_url, &app::model::user::OpenIdConnect::set_base_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"client_id", &app::model::user::OpenIdConnect::get_client_id, &app::model::user::OpenIdConnect::set_client_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"client_secret", &app::model::user::OpenIdConnect::get_client_secret, &app::model::user::OpenIdConnect::set_client_secret, serde::FieldOptions{}>{},
            serde::FieldDesc<"private_key", &app::model::user::OpenIdConnect::get_private_key, &app::model::user::OpenIdConnect::set_private_key, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "open_id_connect"; }
};

template<>
struct serde::Serializable<app::model::user::CreateUserAuthenticationMethodRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"owner_id", &app::model::user::CreateUserAuthenticationMethodRequest::get_owner_id, &app::model::user::CreateUserAuthenticationMethodRequest::set_owner_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"owner_type", &app::model::user::CreateUserAuthenticationMethodRequest::get_owner_type, &app::model::user::CreateUserAuthenticationMethodRequest::set_owner_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"auth_method", &app::model::user::CreateUserAuthenticationMethodRequest::get_auth_method, &app::model::user::CreateUserAuthenticationMethodRequest::set_auth_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"allow_signup", &app::model::user::CreateUserAuthenticationMethodRequest::get_allow_signup, &app::model::user::CreateUserAuthenticationMethodRequest::set_allow_signup, serde::FieldOptions{}>{},
            serde::FieldDesc<"email_domain", &app::model::user::CreateUserAuthenticationMethodRequest::get_email_domain, &app::model::user::CreateUserAuthenticationMethodRequest::set_email_domain, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "create_user_authentication_method_request"; }
};

template<>
struct serde::Serializable<app::model::user::CreateUserAuthenticationMethodResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::model::user::CreateUserAuthenticationMethodResponse::get_id, &app::model::user::CreateUserAuthenticationMethodResponse::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"auth_id", &app::model::user::CreateUserAuthenticationMethodResponse::get_auth_id, &app::model::user::CreateUserAuthenticationMethodResponse::set_auth_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"owner_id", &app::model::user::CreateUserAuthenticationMethodResponse::get_owner_id, &app::model::user::CreateUserAuthenticationMethodResponse::set_owner_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"owner_type", &app::model::user::CreateUserAuthenticationMethodResponse::get_owner_type, &app::model::user::CreateUserAuthenticationMethodResponse::set_owner_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"auth_type", &app::model::user::CreateUserAuthenticationMethodResponse::get_auth_type, &app::model::user::CreateUserAuthenticationMethodResponse::set_auth_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"email_domain", &app::model::user::CreateUserAuthenticationMethodResponse::get_email_domain, &app::model::user::CreateUserAuthenticationMethodResponse::set_email_domain, serde::FieldOptions{}>{},
            serde::FieldDesc<"allow_signup", &app::model::user::CreateUserAuthenticationMethodResponse::get_allow_signup, &app::model::user::CreateUserAuthenticationMethodResponse::set_allow_signup, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "create_user_authentication_method_response"; }
};

template<>
struct serde::Serializable<app::model::user::GetUserAuthenticationMethodsRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"auth_id", &app::model::user::GetUserAuthenticationMethodsRequest::get_auth_id, &app::model::user::GetUserAuthenticationMethodsRequest::set_auth_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"email_domain", &app::model::user::GetUserAuthenticationMethodsRequest::get_email_domain, &app::model::user::GetUserAuthenticationMethodsRequest::set_email_domain, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "get_user_authentication_methods_request"; }
};

template<>
struct serde::Serializable<app::model::user::AuthMethodDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"type", &app::model::user::AuthMethodDetails::get_auth_type, &app::model::user::AuthMethodDetails::set_auth_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &app::model::user::AuthMethodDetails::get_name, &app::model::user::AuthMethodDetails::set_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "auth_method_details"; }
};

template<>
struct serde::Serializable<app::model::user::UserAuthenticationMethodResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::model::user::UserAuthenticationMethodResponse::get_id, &app::model::user::UserAuthenticationMethodResponse::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"auth_id", &app::model::user::UserAuthenticationMethodResponse::get_auth_id, &app::model::user::UserAuthenticationMethodResponse::set_auth_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"auth_method", &app::model::user::UserAuthenticationMethodResponse::get_auth_method, &app::model::user::UserAuthenticationMethodResponse::set_auth_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"allow_signup", &app::model::user::UserAuthenticationMethodResponse::get_allow_signup, &app::model::user::UserAuthenticationMethodResponse::set_allow_signup, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "user_authentication_method_response"; }
};

template<>
struct serde::Serializable<app::model::user::GetSsoAuthUrlRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::model::user::GetSsoAuthUrlRequest::get_id, &app::model::user::GetSsoAuthUrlRequest::set_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "get_sso_auth_url_request"; }
};

template<>
struct serde::Serializable<app::model::user::SsoSignInRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"state", &app::model::user::SsoSignInRequest::get_state, &app::model::user::SsoSignInRequest::set_state, serde::FieldOptions{}>{},
            serde::FieldDesc<"code", &app::model::user::SsoSignInRequest::get_code, &app::model::user::SsoSignInRequest::set_code, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "sso_sign_in_request"; }
};

template<>
struct serde::Serializable<app::model::user::AuthIdAndThemeIdQueryParam>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"auth_id", &app::model::user::AuthIdAndThemeIdQueryParam::get_auth_id, &app::model::user::AuthIdAndThemeIdQueryParam::set_auth_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"theme_id", &app::model::user::AuthIdAndThemeIdQueryParam::get_theme_id, &app::model::user::AuthIdAndThemeIdQueryParam::set_theme_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "auth_id_and_theme_id_query_param"; }
};

template<>
struct serde::Serializable<app::model::user::AuthSelectRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::model::user::AuthSelectRequest::get_id, &app::model::user::AuthSelectRequest::set_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "auth_select_request"; }
};

template<>
struct serde::Serializable<app::model::user::UserKeyTransferRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"from", &app::model::user::UserKeyTransferRequest::get_from, &app::model::user::UserKeyTransferRequest::set_from, serde::FieldOptions{}>{},
            serde::FieldDesc<"limit", &app::model::user::UserKeyTransferRequest::get_limit, &app::model::user::UserKeyTransferRequest::set_limit, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "user_key_transfer_request"; }
};

template<>
struct serde::Serializable<app::model::user::ListUsersInternalRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"user_ids", &app::model::user::ListUsersInternalRequest::get_user_ids, &app::model::user::ListUsersInternalRequest::set_user_ids, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "list_users_internal_request"; }
};

template<>
struct serde::Serializable<app::model::user::ListMembersQueryParam>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"access_level", &app::model::user::ListMembersQueryParam::get_access_level, &app::model::user::ListMembersQueryParam::set_access_level, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "list_members_query_param"; }
};

template<>
struct serde::Serializable<app::model::user::UserTransferKeyResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"total_transferred", &app::model::user::UserTransferKeyResponse::get_total_transferred, &app::model::user::UserTransferKeyResponse::set_total_transferred, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "user_transfer_key_response"; }
};

template<>
struct serde::Serializable<app::model::user::ListOrgsForUserResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"org_id", &app::model::user::ListOrgsForUserResponse::get_org_id, &app::model::user::ListOrgsForUserResponse::set_org_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"org_name", &app::model::user::ListOrgsForUserResponse::get_org_name, &app::model::user::ListOrgsForUserResponse::set_org_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"org_type", &app::model::user::ListOrgsForUserResponse::get_org_type, &app::model::user::ListOrgsForUserResponse::set_org_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "list_orgs_for_user_response"; }
};

template<>
struct serde::Serializable<app::model::user::UserMerchantDetailsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"product_type", &app::model::user::UserMerchantDetailsResponse::get_product_type, &app::model::user::UserMerchantDetailsResponse::set_product_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_account_type", &app::model::user::UserMerchantDetailsResponse::get_merchant_account_type, &app::model::user::UserMerchantDetailsResponse::set_merchant_account_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "user_merchant_details_response"; }
};

template<>
struct serde::Serializable<app::model::user::UserMerchantAccountResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_id", &app::model::user::UserMerchantAccountResponse::get_merchant_id, &app::model::user::UserMerchantAccountResponse::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_name", &app::model::user::UserMerchantAccountResponse::get_merchant_name, &app::model::user::UserMerchantAccountResponse::set_merchant_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"product_type", &app::model::user::UserMerchantAccountResponse::get_product_type, &app::model::user::UserMerchantAccountResponse::set_product_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_account_type", &app::model::user::UserMerchantAccountResponse::get_merchant_account_type, &app::model::user::UserMerchantAccountResponse::set_merchant_account_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"version", &app::model::user::UserMerchantAccountResponse::get_version, &app::model::user::UserMerchantAccountResponse::set_version, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "user_merchant_account_response"; }
};

template<>
struct serde::Serializable<app::model::user::ListProfilesForUserInOrgAndMerchantAccountResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"profile_id", &app::model::user::ListProfilesForUserInOrgAndMerchantAccountResponse::get_profile_id, &app::model::user::ListProfilesForUserInOrgAndMerchantAccountResponse::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_name", &app::model::user::ListProfilesForUserInOrgAndMerchantAccountResponse::get_profile_name, &app::model::user::ListProfilesForUserInOrgAndMerchantAccountResponse::set_profile_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "list_profiles_for_user_in_org_and_merchant_account_response"; }
};

template<>
struct serde::Serializable<app::model::user::IssueEmbeddedTokenResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"token", &app::model::user::IssueEmbeddedTokenResponse::get_token, &app::model::user::IssueEmbeddedTokenResponse::set_token, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "issue_embedded_token_response"; }
};

template<>
struct serde::Serializable<app::model::user::EmbeddedTokenInfoResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"org_id", &app::model::user::EmbeddedTokenInfoResponse::get_org_id, &app::model::user::EmbeddedTokenInfoResponse::set_org_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::model::user::EmbeddedTokenInfoResponse::get_merchant_id, &app::model::user::EmbeddedTokenInfoResponse::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_account_version", &app::model::user::EmbeddedTokenInfoResponse::get_merchant_account_version, &app::model::user::EmbeddedTokenInfoResponse::set_merchant_account_version, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::model::user::EmbeddedTokenInfoResponse::get_profile_id, &app::model::user::EmbeddedTokenInfoResponse::set_profile_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "embedded_token_info_response"; }
};

template<>
struct serde::Serializable<app::model::user::GetUserInternalDetailsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"user_id", &app::model::user::GetUserInternalDetailsResponse::get_user_id, &app::model::user::GetUserInternalDetailsResponse::set_user_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &app::model::user::GetUserInternalDetailsResponse::get_name, &app::model::user::GetUserInternalDetailsResponse::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &app::model::user::GetUserInternalDetailsResponse::get_email, &app::model::user::GetUserInternalDetailsResponse::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_active", &app::model::user::GetUserInternalDetailsResponse::get_is_active, &app::model::user::GetUserInternalDetailsResponse::set_is_active, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "get_user_internal_details_response"; }
};

template<>
struct serde::Serializable<app::model::user::ListUsersInternalResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"users", &app::model::user::ListUsersInternalResponse::get_users, &app::model::user::ListUsersInternalResponse::set_users, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "list_users_internal_response"; }
};

template<>
struct serde::Serializable<app::model::user::AuthorizeTokenRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"token", &app::model::user::AuthorizeTokenRequest::get_token, &app::model::user::AuthorizeTokenRequest::set_token, serde::FieldOptions{}>{},
            serde::FieldDesc<"permission", &app::model::user::AuthorizeTokenRequest::get_permission, &app::model::user::AuthorizeTokenRequest::set_permission, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authorize_token_request"; }
};

#ifdef CONGELADO_TEST
namespace app::model::user::tests {
using namespace boost::ut;
suite<"app_model_user"> app_model_user_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::model::user::SignUpWithMerchantIdRequest>::fields())> > 0);
    };
};
}
#endif
