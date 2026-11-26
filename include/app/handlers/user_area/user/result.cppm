module;
#include <rfl/to_generic.hpp>

export module app_user:user_result;

import std;
import core_serde;

export namespace app::handlers::user {

/** @brief Builds typed dynamic response documents; no hand-written JSON strings. */
class ResponseDocument
{
public:
    [[nodiscard]] static serde::Value empty()
    {
        return serde::Value{serde::Value::Object{}};
    }

    template<typename T>
    [[nodiscard]] static serde::Value from(const T& value)
    {
        return rfl::to_generic(value);
    }
};

// Each Result carries a typed serde::Value document (rfl::Generic) built from
// reflected response values, plus the mirrored res.add_set_cookie() side effect.

class GetActiveUserDetailsResult
{
public:
    GetActiveUserDetailsResult() = default;

    GetActiveUserDetailsResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class LaunchSageResult
{
public:
    LaunchSageResult() = default;

    LaunchSageResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class UserSignupWithMerchantIdResult
{
public:
    UserSignupWithMerchantIdResult() = default;

    UserSignupWithMerchantIdResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class UserSignupResult
{
public:
    UserSignupResult() = default;

    UserSignupResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class UserSigninResult
{
public:
    UserSigninResult() = default;

    UserSigninResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class UserConnectAccountResult
{
public:
    UserConnectAccountResult() = default;

    UserConnectAccountResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class SignoutResult
{
public:
    SignoutResult() = default;

    SignoutResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class ChangePasswordResult
{
public:
    ChangePasswordResult() = default;

    ChangePasswordResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class SetDashboardMetadataResult
{
public:
    SetDashboardMetadataResult() = default;

    SetDashboardMetadataResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class GetMultipleDashboardMetadataResult
{
public:
    GetMultipleDashboardMetadataResult() = default;

    GetMultipleDashboardMetadataResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class InternalUserSignupResult
{
public:
    InternalUserSignupResult() = default;

    InternalUserSignupResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class CreateTenantUserResult
{
public:
    CreateTenantUserResult() = default;

    CreateTenantUserResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class CreatePlatformResult
{
public:
    CreatePlatformResult() = default;

    CreatePlatformResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class UserOrgCreateResult
{
public:
    UserOrgCreateResult() = default;

    UserOrgCreateResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class UserMerchantAccountCreateResult
{
public:
    UserMerchantAccountCreateResult() = default;

    UserMerchantAccountCreateResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class GenerateSampleDataResult
{
public:
    GenerateSampleDataResult() = default;

    GenerateSampleDataResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class DeleteSampleDataResult
{
public:
    DeleteSampleDataResult() = default;

    DeleteSampleDataResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class ListUserRolesDetailsResult
{
public:
    ListUserRolesDetailsResult() = default;

    ListUserRolesDetailsResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class RotatePasswordResult
{
public:
    RotatePasswordResult() = default;

    RotatePasswordResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class ForgotPasswordResult
{
public:
    ForgotPasswordResult() = default;

    ForgotPasswordResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class ResetPasswordResult
{
public:
    ResetPasswordResult() = default;

    ResetPasswordResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class InviteMultipleUserResult
{
public:
    InviteMultipleUserResult() = default;

    InviteMultipleUserResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class ResendInviteResult
{
public:
    ResendInviteResult() = default;

    ResendInviteResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class AcceptInviteFromEmailResult
{
public:
    AcceptInviteFromEmailResult() = default;

    AcceptInviteFromEmailResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class TerminateAcceptInviteResult
{
public:
    TerminateAcceptInviteResult() = default;

    TerminateAcceptInviteResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class VerifyEmailResult
{
public:
    VerifyEmailResult() = default;

    VerifyEmailResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class VerifyEmailRequestResult
{
public:
    VerifyEmailRequestResult() = default;

    VerifyEmailRequestResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class UpdateUserAccountDetailsResult
{
public:
    UpdateUserAccountDetailsResult() = default;

    UpdateUserAccountDetailsResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class UserFromEmailResult
{
public:
    UserFromEmailResult() = default;

    UserFromEmailResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class TotpBeginResult
{
public:
    TotpBeginResult() = default;

    TotpBeginResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class TotpResetResult
{
public:
    TotpResetResult() = default;

    TotpResetResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class TotpVerifyResult
{
public:
    TotpVerifyResult() = default;

    TotpVerifyResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class VerifyRecoveryCodeResult
{
public:
    VerifyRecoveryCodeResult() = default;

    VerifyRecoveryCodeResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class TotpUpdateResult
{
public:
    TotpUpdateResult() = default;

    TotpUpdateResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class GenerateRecoveryCodesResult
{
public:
    GenerateRecoveryCodesResult() = default;

    GenerateRecoveryCodesResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class TerminateTwoFactorAuthResult
{
public:
    TerminateTwoFactorAuthResult() = default;

    TerminateTwoFactorAuthResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class CheckTwoFactorAuthStatusResult
{
public:
    CheckTwoFactorAuthStatusResult() = default;

    CheckTwoFactorAuthStatusResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class CheckTwoFactorAuthStatusWithAttemptsResult
{
public:
    CheckTwoFactorAuthStatusWithAttemptsResult() = default;

    CheckTwoFactorAuthStatusWithAttemptsResult(
        serde::Value document,
        std::string set_cookie
    ) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class GetSsoAuthUrlResult
{
public:
    GetSsoAuthUrlResult() = default;

    GetSsoAuthUrlResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class SsoSignResult
{
public:
    SsoSignResult() = default;

    SsoSignResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class CreateUserAuthenticationMethodResult
{
public:
    CreateUserAuthenticationMethodResult() = default;

    CreateUserAuthenticationMethodResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class UpdateUserAuthenticationMethodResult
{
public:
    UpdateUserAuthenticationMethodResult() = default;

    UpdateUserAuthenticationMethodResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class ListUserAuthenticationMethodsResult
{
public:
    ListUserAuthenticationMethodsResult() = default;

    ListUserAuthenticationMethodsResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class TerminateAuthSelectResult
{
public:
    TerminateAuthSelectResult() = default;

    TerminateAuthSelectResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class TransferUserKeyResult
{
public:
    TransferUserKeyResult() = default;

    TransferUserKeyResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class ListOrgsForUserResult
{
public:
    ListOrgsForUserResult() = default;

    ListOrgsForUserResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class ListMerchantsForUserInOrgResult
{
public:
    ListMerchantsForUserInOrgResult() = default;

    ListMerchantsForUserInOrgResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class ListProfilesForUserInOrgAndMerchantResult
{
public:
    ListProfilesForUserInOrgAndMerchantResult() = default;

    ListProfilesForUserInOrgAndMerchantResult(
        serde::Value document,
        std::string set_cookie
    ) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class SwitchOrgForUserResult
{
public:
    SwitchOrgForUserResult() = default;

    SwitchOrgForUserResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class SwitchMerchantForUserInOrgResult
{
public:
    SwitchMerchantForUserInOrgResult() = default;

    SwitchMerchantForUserInOrgResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class SwitchProfileForUserInOrgAndMerchantResult
{
public:
    SwitchProfileForUserInOrgAndMerchantResult() = default;

    SwitchProfileForUserInOrgAndMerchantResult(
        serde::Value document,
        std::string set_cookie
    ) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class CloneConnectorResult
{
public:
    CloneConnectorResult() = default;

    CloneConnectorResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class IssueEmbeddedTokenResult
{
public:
    IssueEmbeddedTokenResult() = default;

    IssueEmbeddedTokenResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class EmbeddedTokenInfoResult
{
public:
    EmbeddedTokenInfoResult() = default;

    EmbeddedTokenInfoResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class GetUserDetailsInternalResult
{
public:
    GetUserDetailsInternalResult() = default;

    GetUserDetailsInternalResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class ListUsersInternalResult
{
public:
    ListUsersInternalResult() = default;

    ListUsersInternalResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class ListMembersForEntityResult
{
public:
    ListMembersForEntityResult() = default;

    ListMembersForEntityResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class AuthorizeTokenResult
{
public:
    AuthorizeTokenResult() = default;

    AuthorizeTokenResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

class GetUserMerchantDetailsResult
{
public:
    GetUserMerchantDetailsResult() = default;

    GetUserMerchantDetailsResult(serde::Value document, std::string set_cookie) noexcept :
        m_document{std::move(document)},
        m_set_cookie{std::move(set_cookie)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

    [[nodiscard]] const std::string& get_set_cookie() const noexcept
    {
        return m_set_cookie;
    }

    void set_set_cookie(std::string value)
    {
        m_set_cookie = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_set_cookie{};
};

} // namespace app::handlers::user
