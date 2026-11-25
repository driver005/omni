export module app_handlers_verification:result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::verification {

// Both cores build flat upstream DTOs. Each Result reuses the matching app_dto
// serializable shape so the handler writes it with Ser::serialize and keeps the
// exact field names and wire bytes of the old encoded pass-through.

// ── apple_pay_merchant_registration ─────────────────────────────────────────
class ApplePayMerchantRegistrationResult : public app::dto::verifications::ApplepayMerchantResponse
{
public:
    ApplePayMerchantRegistrationResult() = default;

    explicit ApplePayMerchantRegistrationResult(
        app::dto::verifications::ApplepayMerchantResponse value
    ) noexcept :
        ApplepayMerchantResponse{std::move(value)}
    {
    }
};

// ── retrieve_apple_pay_verified_domains ─────────────────────────────────────
class RetrieveApplePayVerifiedDomainsResult :
    public app::dto::verifications::ApplepayVerifiedDomainsResponse
{
public:
    RetrieveApplePayVerifiedDomainsResult() = default;

    explicit RetrieveApplePayVerifiedDomainsResult(
        app::dto::verifications::ApplepayVerifiedDomainsResponse value
    ) noexcept :
        ApplepayVerifiedDomainsResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::verification

template<>
struct serde::Serializable<app::handlers::verification::ApplePayMerchantRegistrationResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::verifications::ApplepayMerchantResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "apple_pay_merchant_registration_result";
    }
};

template<>
struct serde::Serializable<app::handlers::verification::RetrieveApplePayVerifiedDomainsResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            app::dto::verifications::ApplepayVerifiedDomainsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "retrieve_apple_pay_verified_domains_result";
    }
};
