export module app_webhook:verification_runtime_retrieve_apple_pay_verified_domains;

import std;
import core_behavior;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :verification_input;
import :verification_result;
import :verification_runtime_common;

export namespace app::handlers::verification {

class RetrieveApplePayVerifiedDomainsRuntime
{
public:
    explicit RetrieveApplePayVerifiedDomainsRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief Port of `get_verified_apple_domains_with_mid_mca_id`
     *  (core/verification.rs:95-119). */
    [[nodiscard]] core::behavior::Outcome<
        ::app::dto::verifications::ApplepayVerifiedDomainsResponse>
    get_verified_apple_domains_with_mid_mca_id(const ::core::behavior::CallContext& ctx) const
    {
        const auto connector = ctx.query.find("merchant_connector_account_id");
        auto account = m_support.get_apple_pay_merchant_connector_account(
            ctx.merchant_id,
            connector == ctx.query.end() ? std::string{} : connector->second,
            ctx.profile_id
        );
        if (!account.has_value()) {
            return std::unexpected(account.error());
        }

        // `.applepay_verified_domains.unwrap_or_default()` (:112-114).
        ::app::dto::verifications::ApplepayVerifiedDomainsResponse response{};
        response.set_verified_domains(
            m_support.decode_domains(account->account.get_applepay_verified_domains())
        );

        // #[instrument(fields(flow = ?Flow::Verification))] on the route.
        m_context.logger().record_flow("Verification");
        return response;
    }

private:
    ::app::context::AppContext& m_context;
    VerificationRuntimeSupport m_support;
};

} // namespace app::handlers::verification
