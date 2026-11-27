export module app_routing:superposition_sdk_config_runtime_get_profile_sdk_config;

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

class GetProfileSdkConfigRuntime
{
public:
    explicit GetProfileSdkConfigRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: get_profile_sdk_config ─────────────────────────
    // Upstream core::get_superposition_sdk_config profile path (core:152-345).
    [[nodiscard]] std::expected<
        ::app::dto::superposition_sdk_config::SuperPositionConfigResponse,
        core::error::AppError>
    get_profile_sdk_config(const GetProfileSdkConfigInput& input) const
    {
        using Response = ::app::dto::superposition_sdk_config::SuperPositionConfigResponse;

        // Upstream find_business_profile_by_profile_id (core:152-161).
        const auto profile = m_context.repository().find_profile_by_string(input.profile_id);
        if (!profile) {
            return std::unexpected{m_errors.profile_not_found(input.profile_id)};
        }
        const std::string merchant_id = !profile->get_merchant_id().empty()
                                            ? profile->get_merchant_id()
                                            : input.auth.merchant_id();

        // Upstream list_enabled_merchant_connector_accounts_without_encrypted... (core:163-170)
        // and build_merchant_enabled_pms_context (core:172-289).
        const auto enabled_mcas =
            m_support.list_enabled_merchant_connector_accounts(merchant_id, input.profile_id);
        const auto pms_context = m_support.build_merchant_enabled_pms_context(enabled_mcas);
        const auto payment_methods = m_support.translate_to_sdk_payment_methods(pms_context);

        // Upstream dimension filter (core:291-322): connector[] is the raw mca connector list.
        const auto filter = m_support.build_dimension_filter(
            input.profile_id,
            merchant_id,
            input.provider_merchant_id,
            m_support.organization_id(merchant_id, input.auth),
            m_support.enabled_connectors(enabled_mcas)
        );

        Response response{};
        response.set_raw_configs(m_support.resolve_raw_config(filter.key));
        response.set_context_used(filter.value);
        response.set_payment_methods(
            std::vector<::app::dto::superposition_sdk_config::SdkPaymentMethod>{payment_methods}
        );
        response.set_account_config(
            m_support.build_account_config(*profile, input.profile_id, merchant_id, input.auth)
        );

        m_context.metrics().record_flow("GetSuperpositionSdkConfig");
        m_context.metrics().counter("get_profile_superposition_sdk_config").increment();

        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    SuperpositionSdkConfigRuntimeSupport m_support;
    SdkConfigErrorSupport m_errors{};
};

} // namespace app::handlers::superposition_sdk_config
