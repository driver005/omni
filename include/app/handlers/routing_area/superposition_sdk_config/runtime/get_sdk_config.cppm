export module app_routing:superposition_sdk_config_runtime_get_sdk_config;

import std;
import core_error;
import app_context;
import app_dto;
import app_store;
import app_auth;
import app_metrics;

import :superposition_sdk_config_input;
import :superposition_sdk_config_result;
import :superposition_sdk_config_runtime_common;

export namespace app::handlers::superposition_sdk_config {

class GetSdkConfigRuntime
{
public:
    explicit GetSdkConfigRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: get_sdk_config ─────────────────────────
    // Upstream core::get_superposition_sdk_config (core:353-462).
    [[nodiscard]] std::expected<GetSdkConfigResult, core::error::AppError>
    get_sdk_config(const GetSdkConfigInput& input) const
    {
        using Response = ::app::dto::superposition_sdk_config::SuperPositionConfigResponse;

        // find_payment_intent_by_payment_id_processor_merchant_id (core:360-368): the local
        // store has no processor-merchant composite key, so the id-scoped lookup is used.
        const auto stored_payment = m_context.repository().find_payment_by_string(input.payment_id);
        if (!stored_payment) {
            return std::unexpected{m_errors.payment_not_found()};
        }
        const auto& payment = *stored_payment;
        const std::string merchant_id = payment.get_merchant_id();

        // Upstream get_payment_context (core:35-137) -> SdkPaymentContext.
        const auto payment_context = m_support.get_payment_context(input.payment_id, payment);

        // Upstream payment_intent.profile_id.ok_or(GenericNotFoundError { "Profile id not found" })
        // (core:372-376). The local PaymentIntentRecord has no profile_id column, so the id is
        // taken from the SDK claim, the authenticated context, then the intent metadata.
        std::string profile_id = input.sdk_profile_id;
        if (profile_id.empty()) {
            profile_id = input.auth.profile_id();
        }
        if (profile_id.empty()) {
            profile_id = m_support.json_string_field(
                m_support.parse_json(payment.get_metadata()),
                "profile_id"
            );
        }
        if (profile_id.empty()) {
            return std::unexpected{m_errors.generic_not_found("Profile id not found")};
        }

        // Upstream find_business_profile_by_profile_id (core:378-384).
        const auto profile = m_context.repository().find_profile_by_string(profile_id);
        if (!profile) {
            return std::unexpected{m_errors.profile_not_found(profile_id)};
        }

        // Upstream list_enabled_merchant_connector_accounts_without_encrypted...
        // (core:163-170) and build_merchant_enabled_pms_context (core:386-397).
        const auto enabled_mcas =
            m_support.list_enabled_merchant_connector_accounts(merchant_id, profile_id);
        const auto pms_context = m_support.build_merchant_enabled_pms_context(enabled_mcas);
        const auto payment_methods = m_support.translate_to_sdk_payment_methods(pms_context);

        // Upstream dimension filter (core:399-433): connector[] comes directly from
        // MerchantEnabledPmsContext::get_eligible_connectors().
        const auto filter = m_support.build_dimension_filter(
            profile_id,
            merchant_id,
            input.provider_merchant_id,
            m_support.organization_id(merchant_id, input.auth),
            pms_context.get_eligible_connectors()
        );

        // is_cit_transaction (core:122-128) feeds the local superposition context key: the
        // in-process SuperpositionClient has no eligibility dimension, so the CIT state narrows
        // the scoped lookup here.
        const std::string config_key =
            m_support.filter_key_with_cit(filter.key, payment_context.is_cit_transaction);

        Response response{};
        response.set_raw_configs(m_support.resolve_raw_config(config_key));
        response.set_context_used(filter.value);
        response.set_payment_methods(
            std::vector<::app::dto::superposition_sdk_config::SdkPaymentMethod>{payment_methods}
        );
        response.set_account_config(
            m_support.build_account_config(*profile, profile_id, merchant_id, input.auth)
        );

        m_context.metrics().record_flow("GetSuperpositionSdkConfig");
        m_context.metrics().counter("get_superposition_sdk_config").increment();

        return GetSdkConfigResult{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    SuperpositionSdkConfigRuntimeSupport m_support;
    SdkConfigErrorSupport m_errors{};
};

} // namespace app::handlers::superposition_sdk_config
