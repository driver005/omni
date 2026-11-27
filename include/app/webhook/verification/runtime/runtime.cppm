export module app_webhook:verification_runtime;

import std;
import core_behavior;
import core_error;
import app_context;
import app_dto;

import :verification_input;
import :verification_result;
import :verification_runtime_common;
import :verification_runtime_apple_pay_merchant_registration;
import :verification_runtime_retrieve_apple_pay_verified_domains;

export namespace app::handlers::verification {

class VerificationRuntime
{
public:
    explicit VerificationRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_apple_pay_merchant_registration{context},
        m_retrieve_apple_pay_verified_domains{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::verifications::ApplepayMerchantResponse>
    apple_pay_merchant_registration(
        const VerificationRequestWire& wire,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_apple_pay_merchant_registration.verify_merchant_creds_for_applepay(wire, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<
        ::app::dto::verifications::ApplepayVerifiedDomainsResponse>
    retrieve_apple_pay_verified_domains(const ::core::behavior::CallContext& ctx) const
    {
        return m_retrieve_apple_pay_verified_domains.get_verified_apple_domains_with_mid_mca_id(
            ctx
        );
    }

private:
    ::app::context::AppContext& m_context;
    ApplePayMerchantRegistrationRuntime m_apple_pay_merchant_registration;
    RetrieveApplePayVerifiedDomainsRuntime m_retrieve_apple_pay_verified_domains;
};

} // namespace app::handlers::verification
