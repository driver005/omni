export module app_payment:payments_runtime;

import std;
import core_error;
import app_context;

import :payments_input;
import :payments_result;
import :payments_runtime_common;
import :payments_runtime_confirm_intent_with_external_vault_proxy;
import :payments_runtime_get_filters_for_payments;
import :payments_runtime_get_payment_filters;
import :payments_runtime_get_payment_filters_profile;
import :payments_runtime_get_payments_aggregates;
import :payments_runtime_get_payments_aggregates_profile;
import :payments_runtime_list_payment_attempts;
import :payments_runtime_list_payment_methods;
import :payments_runtime_payment_confirm_intent;
import :payments_runtime_payment_get_intent_using_merchant_reference_id;
import :payments_runtime_payment_link_create;
import :payments_runtime_payment_status;
import :payments_runtime_payments_apply_pm_data;
import :payments_runtime_payments_approve;
import :payments_runtime_payments_cancel;
import :payments_runtime_payments_cancel_post_capture;
import :payments_runtime_payments_cancel_post_capture_retrieve;
import :payments_runtime_payments_capture;
import :payments_runtime_payments_complete_authorize;
import :payments_runtime_payments_complete_authorize_redirect;
import :payments_runtime_payments_complete_authorize_redirect_with_creds_identifier;
import :payments_runtime_payments_confirm;
import :payments_runtime_payments_connector_session;
import :payments_runtime_payments_create;
import :payments_runtime_payments_create_and_confirm_intent;
import :payments_runtime_payments_create_intent;
import :payments_runtime_payments_dynamic_tax_calculation;
import :payments_runtime_payments_extend_authorization;
import :payments_runtime_payments_external_authentication;
import :payments_runtime_payments_finish_redirection;
import :payments_runtime_payments_get_intent;
import :payments_runtime_payments_incremental_authorization;
import :payments_runtime_payments_list;
import :payments_runtime_payments_list_by_filter;
import :payments_runtime_payments_list_for_platform;
import :payments_runtime_payments_list_for_platform_filters;
import :payments_runtime_payments_manual_status_update;
import :payments_runtime_payments_manual_status_update_eligible_statuses;
import :payments_runtime_payments_manual_update;
import :payments_runtime_payments_post_session_tokens;
import :payments_runtime_payments_redirect_response;
import :payments_runtime_payments_redirect_response_with_creds_identifier;
import :payments_runtime_payments_reject;
import :payments_runtime_payments_retrieve;
import :payments_runtime_payments_retrieve_with_gateway_creds;
import :payments_runtime_payments_start;
import :payments_runtime_payments_start_redirection;
import :payments_runtime_payments_status_with_gateway_creds;
import :payments_runtime_payments_submit_eligibility;
import :payments_runtime_payments_submit_eligibility_check;
import :payments_runtime_payments_update;
import :payments_runtime_payments_update_intent;
import :payments_runtime_payments_update_metadata;
import :payments_runtime_post_3ds_payments_authorize;
import :payments_runtime_profile_payments_list;
import :payments_runtime_profile_payments_list_by_filter;
import :payments_runtime_proxy_confirm_intent;
import :payments_runtime_recovery_payments_create;
import :payments_runtime_retrieve_extended_card_info;
import :payments_runtime_revenue_recovery_get_intent;
import :payments_runtime_revenue_recovery_invoices_list;
import :payments_runtime_revenue_recovery_invoices_list_profile;
import :payments_runtime_get_payment_list_from_opensearch;
import :payments_runtime_get_profile_payment_list_from_opensearch;
import :payments_runtime_list_payment_methods_for_payments_client;

export namespace app::handlers::payments {

class PaymentsRuntime
{
public:
    explicit PaymentsRuntime(
        ::app::context::AppContext& context,
        PaymentsScopeState& scope
    ) noexcept :
        m_context{context},
        m_support{context, scope},
        m_confirm_intent_with_external_vault_proxy{context, m_support},
        m_get_filters_for_payments{context, m_support},
        m_get_payment_filters{context, m_support},
        m_get_payment_filters_profile{context, m_support},
        m_get_payments_aggregates{context, m_support},
        m_get_payments_aggregates_profile{context, m_support},
        m_list_payment_attempts{context, m_support},
        m_list_payment_methods{context, m_support},
        m_payment_confirm_intent{context, m_support},
        m_payment_get_intent_using_merchant_reference_id{context, m_support},
        m_payment_link_create{context, m_support},
        m_payment_status{context, m_support},
        m_payments_apply_pm_data{context, m_support},
        m_payments_approve{context, m_support},
        m_payments_cancel{context, m_support},
        m_payments_cancel_post_capture{context, m_support},
        m_payments_cancel_post_capture_retrieve{context, m_support},
        m_payments_capture{context, m_support},
        m_payments_complete_authorize{context, m_support},
        m_payments_complete_authorize_redirect{context, m_support},
        m_payments_complete_authorize_redirect_with_creds_identifier{context, m_support},
        m_payments_confirm{context, m_support},
        m_payments_connector_session{context, m_support},
        m_payments_create{context, m_support},
        m_payments_create_and_confirm_intent{context, m_support},
        m_payments_create_intent{context, m_support},
        m_payments_dynamic_tax_calculation{context, m_support},
        m_payments_extend_authorization{context, m_support},
        m_payments_external_authentication{context, m_support},
        m_payments_finish_redirection{context, m_support},
        m_payments_get_intent{context, m_support},
        m_payments_incremental_authorization{context, m_support},
        m_payments_list{context, m_support},
        m_payments_list_by_filter{context, m_support},
        m_payments_list_for_platform{context, m_support},
        m_payments_list_for_platform_filters{context, m_support},
        m_payments_manual_status_update{context, m_support},
        m_payments_manual_status_update_eligible_statuses{context, m_support},
        m_payments_manual_update{context, m_support},
        m_payments_post_session_tokens{context, m_support},
        m_payments_redirect_response{context, m_support},
        m_payments_redirect_response_with_creds_identifier{context, m_support},
        m_payments_reject{context, m_support},
        m_payments_retrieve{context, m_support},
        m_payments_retrieve_with_gateway_creds{context, m_support},
        m_payments_start{context, m_support},
        m_payments_start_redirection{context, m_support},
        m_payments_status_with_gateway_creds{context, m_support},
        m_payments_submit_eligibility{context, m_support},
        m_payments_submit_eligibility_check{context, m_support},
        m_payments_update{context, m_support},
        m_payments_update_intent{context, m_support},
        m_payments_update_metadata{context, m_support},
        m_post_3ds_payments_authorize{context, m_support},
        m_profile_payments_list{context, m_support},
        m_profile_payments_list_by_filter{context, m_support},
        m_proxy_confirm_intent{context, m_support},
        m_recovery_payments_create{context, m_support},
        m_retrieve_extended_card_info{context, m_support},
        m_revenue_recovery_get_intent{context, m_support},
        m_revenue_recovery_invoices_list{context, m_support},
        m_revenue_recovery_invoices_list_profile{context, m_support},
        m_get_payment_list_from_opensearch{context, m_support},
        m_get_profile_payment_list_from_opensearch{context, m_support},
        m_list_payment_methods_for_payments_client{context, m_support}
    {
    }

    [[nodiscard]] std::expected<ConfirmIntentWithExternalVaultProxyResult, core::error::AppError>
    confirm_intent_with_external_vault_proxy(
        const ConfirmIntentWithExternalVaultProxyInput& input
    ) const
    {
        return m_confirm_intent_with_external_vault_proxy.confirm_intent_with_external_vault_proxy(
            input
        );
    }

    [[nodiscard]] std::expected<GetFiltersForPaymentsResult, core::error::AppError>
    get_filters_for_payments(const GetFiltersForPaymentsInput& input) const
    {
        return m_get_filters_for_payments.get_filters_for_payments(input);
    }

    [[nodiscard]] std::expected<GetPaymentFiltersResult, core::error::AppError>
    get_payment_filters(const GetPaymentFiltersInput& input) const
    {
        return m_get_payment_filters.get_payment_filters(input);
    }

    [[nodiscard]] std::expected<GetPaymentFiltersProfileResult, core::error::AppError>
    get_payment_filters_profile(const GetPaymentFiltersProfileInput& input) const
    {
        return m_get_payment_filters_profile.get_payment_filters_profile(input);
    }

    [[nodiscard]] std::expected<GetPaymentsAggregatesResult, core::error::AppError>
    get_payments_aggregates(const GetPaymentsAggregatesInput& input) const
    {
        return m_get_payments_aggregates.get_payments_aggregates(input);
    }

    [[nodiscard]] std::expected<GetPaymentsAggregatesProfileResult, core::error::AppError>
    get_payments_aggregates_profile(const GetPaymentsAggregatesProfileInput& input) const
    {
        return m_get_payments_aggregates_profile.get_payments_aggregates_profile(input);
    }

    [[nodiscard]] std::expected<ListPaymentAttemptsResult, core::error::AppError>
    list_payment_attempts(const ListPaymentAttemptsInput& input) const
    {
        return m_list_payment_attempts.list_payment_attempts(input);
    }

    [[nodiscard]] std::expected<ListPaymentMethodsResult, core::error::AppError>
    list_payment_methods(const ListPaymentMethodsInput& input) const
    {
        return m_list_payment_methods.list_payment_methods(input);
    }

    [[nodiscard]] std::expected<PaymentConfirmIntentResult, core::error::AppError>
    payment_confirm_intent(const PaymentConfirmIntentInput& input) const
    {
        return m_payment_confirm_intent.payment_confirm_intent(input);
    }

    [[nodiscard]] std::
        expected<PaymentGetIntentUsingMerchantReferenceIdResult, core::error::AppError>
        payment_get_intent_using_merchant_reference_id(
            const PaymentGetIntentUsingMerchantReferenceIdInput& input
        ) const
    {
        return m_payment_get_intent_using_merchant_reference_id
            .payment_get_intent_using_merchant_reference_id(input);
    }

    [[nodiscard]] std::expected<PaymentLinkCreateResult, core::error::AppError>
    payment_link_create(const PaymentLinkCreateInput& input) const
    {
        return m_payment_link_create.payment_link_create(input);
    }

    [[nodiscard]] std::expected<PaymentStatusResult, core::error::AppError>
    payment_status(const PaymentStatusInput& input) const
    {
        return m_payment_status.payment_status(input);
    }

    [[nodiscard]] std::expected<PaymentsApplyPmDataResult, core::error::AppError>
    payments_apply_pm_data(const PaymentsApplyPmDataInput& input) const
    {
        return m_payments_apply_pm_data.payments_apply_pm_data(input);
    }

    [[nodiscard]] std::expected<PaymentsApproveResult, core::error::AppError>
    payments_approve(const PaymentsApproveInput& input) const
    {
        return m_payments_approve.payments_approve(input);
    }

    [[nodiscard]] std::expected<PaymentsCancelResult, core::error::AppError>
    payments_cancel(const PaymentsCancelInput& input) const
    {
        return m_payments_cancel.payments_cancel(input);
    }

    [[nodiscard]] std::expected<PaymentsCancelPostCaptureResult, core::error::AppError>
    payments_cancel_post_capture(const PaymentsCancelPostCaptureInput& input) const
    {
        return m_payments_cancel_post_capture.payments_cancel_post_capture(input);
    }

    [[nodiscard]] std::expected<PaymentsCancelPostCaptureRetrieveResult, core::error::AppError>
    payments_cancel_post_capture_retrieve(const PaymentsCancelPostCaptureRetrieveInput& input) const
    {
        return m_payments_cancel_post_capture_retrieve.payments_cancel_post_capture_retrieve(input);
    }

    [[nodiscard]] std::expected<PaymentsCaptureResult, core::error::AppError>
    payments_capture(const PaymentsCaptureInput& input) const
    {
        return m_payments_capture.payments_capture(input);
    }

    [[nodiscard]] std::expected<PaymentsCompleteAuthorizeResult, core::error::AppError>
    payments_complete_authorize(const PaymentsCompleteAuthorizeInput& input) const
    {
        return m_payments_complete_authorize.payments_complete_authorize(input);
    }

    [[nodiscard]] std::expected<PaymentsCompleteAuthorizeRedirectResult, core::error::AppError>
    payments_complete_authorize_redirect(const PaymentsCompleteAuthorizeRedirectInput& input) const
    {
        return m_payments_complete_authorize_redirect.payments_complete_authorize_redirect(input);
    }

    [[nodiscard]] std::
        expected<PaymentsCompleteAuthorizeRedirectWithCredsIdentifierResult, core::error::AppError>
        payments_complete_authorize_redirect_with_creds_identifier(
            const PaymentsCompleteAuthorizeRedirectWithCredsIdentifierInput& input
        ) const
    {
        return m_payments_complete_authorize_redirect_with_creds_identifier
            .payments_complete_authorize_redirect_with_creds_identifier(input);
    }

    [[nodiscard]] std::expected<PaymentsConfirmResult, core::error::AppError>
    payments_confirm(const PaymentsConfirmInput& input) const
    {
        return m_payments_confirm.payments_confirm(input);
    }

    [[nodiscard]] std::expected<PaymentsConnectorSessionResult, core::error::AppError>
    payments_connector_session(const PaymentsConnectorSessionInput& input) const
    {
        return m_payments_connector_session.payments_connector_session(input);
    }

    [[nodiscard]] std::expected<PaymentsCreateResult, core::error::AppError>
    payments_create(const PaymentsCreateInput& input) const
    {
        return m_payments_create.payments_create(input);
    }

    [[nodiscard]] std::expected<PaymentsCreateAndConfirmIntentResult, core::error::AppError>
    payments_create_and_confirm_intent(const PaymentsCreateAndConfirmIntentInput& input) const
    {
        return m_payments_create_and_confirm_intent.payments_create_and_confirm_intent(input);
    }

    [[nodiscard]] std::expected<PaymentsCreateIntentResult, core::error::AppError>
    payments_create_intent(const PaymentsCreateIntentInput& input) const
    {
        return m_payments_create_intent.payments_create_intent(input);
    }

    [[nodiscard]] std::expected<PaymentsDynamicTaxCalculationResult, core::error::AppError>
    payments_dynamic_tax_calculation(const PaymentsDynamicTaxCalculationInput& input) const
    {
        return m_payments_dynamic_tax_calculation.payments_dynamic_tax_calculation(input);
    }

    [[nodiscard]] std::expected<PaymentsExtendAuthorizationResult, core::error::AppError>
    payments_extend_authorization(const PaymentsExtendAuthorizationInput& input) const
    {
        return m_payments_extend_authorization.payments_extend_authorization(input);
    }

    [[nodiscard]] std::expected<PaymentsExternalAuthenticationResult, core::error::AppError>
    payments_external_authentication(const PaymentsExternalAuthenticationInput& input) const
    {
        return m_payments_external_authentication.payments_external_authentication(input);
    }

    [[nodiscard]] std::expected<PaymentsFinishRedirectionResult, core::error::AppError>
    payments_finish_redirection(const PaymentsFinishRedirectionInput& input) const
    {
        return m_payments_finish_redirection.payments_finish_redirection(input);
    }

    [[nodiscard]] std::expected<PaymentsGetIntentResult, core::error::AppError>
    payments_get_intent(const PaymentsGetIntentInput& input) const
    {
        return m_payments_get_intent.payments_get_intent(input);
    }

    [[nodiscard]] std::expected<PaymentsIncrementalAuthorizationResult, core::error::AppError>
    payments_incremental_authorization(const PaymentsIncrementalAuthorizationInput& input) const
    {
        return m_payments_incremental_authorization.payments_incremental_authorization(input);
    }

    [[nodiscard]] std::expected<PaymentsListResult, core::error::AppError>
    payments_list(const PaymentsListInput& input) const
    {
        return m_payments_list.payments_list(input);
    }

    [[nodiscard]] std::expected<PaymentsListByFilterResult, core::error::AppError>
    payments_list_by_filter(const PaymentsListByFilterInput& input) const
    {
        return m_payments_list_by_filter.payments_list_by_filter(input);
    }

    [[nodiscard]] std::expected<PaymentsListForPlatformResult, core::error::AppError>
    payments_list_for_platform(const PaymentsListForPlatformInput& input) const
    {
        return m_payments_list_for_platform.payments_list_for_platform(input);
    }

    [[nodiscard]] std::expected<PaymentsListForPlatformFiltersResult, core::error::AppError>
    payments_list_for_platform_filters(const PaymentsListForPlatformFiltersInput& input) const
    {
        return m_payments_list_for_platform_filters.payments_list_for_platform_filters(input);
    }

    [[nodiscard]] std::expected<PaymentsManualStatusUpdateResult, core::error::AppError>
    payments_manual_status_update(const PaymentsManualStatusUpdateInput& input) const
    {
        return m_payments_manual_status_update.payments_manual_status_update(input);
    }

    [[nodiscard]] std::
        expected<PaymentsManualStatusUpdateEligibleStatusesResult, core::error::AppError>
        payments_manual_status_update_eligible_statuses(
            const PaymentsManualStatusUpdateEligibleStatusesInput& input
        ) const
    {
        return m_payments_manual_status_update_eligible_statuses
            .payments_manual_status_update_eligible_statuses(input);
    }

    [[nodiscard]] std::expected<PaymentsManualUpdateResult, core::error::AppError>
    payments_manual_update(const PaymentsManualUpdateInput& input) const
    {
        return m_payments_manual_update.payments_manual_update(input);
    }

    [[nodiscard]] std::expected<PaymentsPostSessionTokensResult, core::error::AppError>
    payments_post_session_tokens(const PaymentsPostSessionTokensInput& input) const
    {
        return m_payments_post_session_tokens.payments_post_session_tokens(input);
    }

    [[nodiscard]] std::expected<PaymentsRedirectResponseResult, core::error::AppError>
    payments_redirect_response(const PaymentsRedirectResponseInput& input) const
    {
        return m_payments_redirect_response.payments_redirect_response(input);
    }

    [[nodiscard]] std::
        expected<PaymentsRedirectResponseWithCredsIdentifierResult, core::error::AppError>
        payments_redirect_response_with_creds_identifier(
            const PaymentsRedirectResponseWithCredsIdentifierInput& input
        ) const
    {
        return m_payments_redirect_response_with_creds_identifier
            .payments_redirect_response_with_creds_identifier(input);
    }

    [[nodiscard]] std::expected<PaymentsRejectResult, core::error::AppError>
    payments_reject(const PaymentsRejectInput& input) const
    {
        return m_payments_reject.payments_reject(input);
    }

    [[nodiscard]] std::expected<PaymentsRetrieveResult, core::error::AppError>
    payments_retrieve(const PaymentsRetrieveInput& input) const
    {
        return m_payments_retrieve.payments_retrieve(input);
    }

    [[nodiscard]] std::expected<PaymentsRetrieveWithGatewayCredsResult, core::error::AppError>
    payments_retrieve_with_gateway_creds(const PaymentsRetrieveWithGatewayCredsInput& input) const
    {
        return m_payments_retrieve_with_gateway_creds.payments_retrieve_with_gateway_creds(input);
    }

    [[nodiscard]] std::expected<PaymentsStartResult, core::error::AppError>
    payments_start(const PaymentsStartInput& input) const
    {
        return m_payments_start.payments_start(input);
    }

    [[nodiscard]] std::expected<PaymentsStartRedirectionResult, core::error::AppError>
    payments_start_redirection(const PaymentsStartRedirectionInput& input) const
    {
        return m_payments_start_redirection.payments_start_redirection(input);
    }

    [[nodiscard]] std::expected<PaymentsStatusWithGatewayCredsResult, core::error::AppError>
    payments_status_with_gateway_creds(const PaymentsStatusWithGatewayCredsInput& input) const
    {
        return m_payments_status_with_gateway_creds.payments_status_with_gateway_creds(input);
    }

    [[nodiscard]] std::expected<PaymentsSubmitEligibilityResult, core::error::AppError>
    payments_submit_eligibility(const PaymentsSubmitEligibilityInput& input) const
    {
        return m_payments_submit_eligibility.payments_submit_eligibility(input);
    }

    [[nodiscard]] std::expected<PaymentsSubmitEligibilityCheckResult, core::error::AppError>
    payments_submit_eligibility_check(const PaymentsSubmitEligibilityCheckInput& input) const
    {
        return m_payments_submit_eligibility_check.payments_submit_eligibility_check(input);
    }

    [[nodiscard]] std::expected<PaymentsUpdateResult, core::error::AppError>
    payments_update(const PaymentsUpdateInput& input) const
    {
        return m_payments_update.payments_update(input);
    }

    [[nodiscard]] std::expected<PaymentsUpdateIntentResult, core::error::AppError>
    payments_update_intent(const PaymentsUpdateIntentInput& input) const
    {
        return m_payments_update_intent.payments_update_intent(input);
    }

    [[nodiscard]] std::expected<PaymentsUpdateMetadataResult, core::error::AppError>
    payments_update_metadata(const PaymentsUpdateMetadataInput& input) const
    {
        return m_payments_update_metadata.payments_update_metadata(input);
    }

    [[nodiscard]] std::expected<Post3dsPaymentsAuthorizeResult, core::error::AppError>
    post_3ds_payments_authorize(const Post3dsPaymentsAuthorizeInput& input) const
    {
        return m_post_3ds_payments_authorize.post_3ds_payments_authorize(input);
    }

    [[nodiscard]] std::expected<ProfilePaymentsListResult, core::error::AppError>
    profile_payments_list(const ProfilePaymentsListInput& input) const
    {
        return m_profile_payments_list.profile_payments_list(input);
    }

    [[nodiscard]] std::expected<ProfilePaymentsListByFilterResult, core::error::AppError>
    profile_payments_list_by_filter(const ProfilePaymentsListByFilterInput& input) const
    {
        return m_profile_payments_list_by_filter.profile_payments_list_by_filter(input);
    }

    [[nodiscard]] std::expected<ProxyConfirmIntentResult, core::error::AppError>
    proxy_confirm_intent(const ProxyConfirmIntentInput& input) const
    {
        return m_proxy_confirm_intent.proxy_confirm_intent(input);
    }

    [[nodiscard]] std::expected<RecoveryPaymentsCreateResult, core::error::AppError>
    recovery_payments_create(const RecoveryPaymentsCreateInput& input) const
    {
        return m_recovery_payments_create.recovery_payments_create(input);
    }

    [[nodiscard]] std::expected<RetrieveExtendedCardInfoResult, core::error::AppError>
    retrieve_extended_card_info(const RetrieveExtendedCardInfoInput& input) const
    {
        return m_retrieve_extended_card_info.retrieve_extended_card_info(input);
    }

    [[nodiscard]] std::expected<RevenueRecoveryGetIntentResult, core::error::AppError>
    revenue_recovery_get_intent(const RevenueRecoveryGetIntentInput& input) const
    {
        return m_revenue_recovery_get_intent.revenue_recovery_get_intent(input);
    }

    [[nodiscard]] std::expected<RevenueRecoveryInvoicesListResult, core::error::AppError>
    revenue_recovery_invoices_list(const RevenueRecoveryInvoicesListInput& input) const
    {
        return m_revenue_recovery_invoices_list.revenue_recovery_invoices_list(input);
    }

    [[nodiscard]] std::expected<RevenueRecoveryInvoicesListProfileResult, core::error::AppError>
    revenue_recovery_invoices_list_profile(
        const RevenueRecoveryInvoicesListProfileInput& input
    ) const
    {
        return m_revenue_recovery_invoices_list_profile.revenue_recovery_invoices_list_profile(
            input
        );
    }

    [[nodiscard]] std::expected<GetPaymentListFromOpensearchResult, core::error::AppError>
    get_payment_list_from_opensearch(const GetPaymentListFromOpensearchInput& input) const
    {
        return m_get_payment_list_from_opensearch.get_payment_list_from_opensearch(input);
    }

    [[nodiscard]] std::expected<GetProfilePaymentListFromOpensearchResult, core::error::AppError>
    get_profile_payment_list_from_opensearch(
        const GetProfilePaymentListFromOpensearchInput& input
    ) const
    {
        return m_get_profile_payment_list_from_opensearch.get_profile_payment_list_from_opensearch(
            input
        );
    }

    [[nodiscard]] std::expected<ListPaymentMethodsForPaymentsClientResult, core::error::AppError>
    list_payment_methods_for_payments_client(
        const ListPaymentMethodsForPaymentsClientInput& input
    ) const
    {
        return m_list_payment_methods_for_payments_client.list_payment_methods_for_payments_client(
            input
        );
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsRuntimeSupport m_support;
    ConfirmIntentWithExternalVaultProxyRuntime m_confirm_intent_with_external_vault_proxy;
    GetFiltersForPaymentsRuntime m_get_filters_for_payments;
    GetPaymentFiltersRuntime m_get_payment_filters;
    GetPaymentFiltersProfileRuntime m_get_payment_filters_profile;
    GetPaymentsAggregatesRuntime m_get_payments_aggregates;
    GetPaymentsAggregatesProfileRuntime m_get_payments_aggregates_profile;
    ListPaymentAttemptsRuntime m_list_payment_attempts;
    ListPaymentMethodsRuntime m_list_payment_methods;
    PaymentConfirmIntentRuntime m_payment_confirm_intent;
    PaymentGetIntentUsingMerchantReferenceIdRuntime
        m_payment_get_intent_using_merchant_reference_id;
    PaymentLinkCreateRuntime m_payment_link_create;
    PaymentStatusRuntime m_payment_status;
    PaymentsApplyPmDataRuntime m_payments_apply_pm_data;
    PaymentsApproveRuntime m_payments_approve;
    PaymentsCancelRuntime m_payments_cancel;
    PaymentsCancelPostCaptureRuntime m_payments_cancel_post_capture;
    PaymentsCancelPostCaptureRetrieveRuntime m_payments_cancel_post_capture_retrieve;
    PaymentsCaptureRuntime m_payments_capture;
    PaymentsCompleteAuthorizeRuntime m_payments_complete_authorize;
    PaymentsCompleteAuthorizeRedirectRuntime m_payments_complete_authorize_redirect;
    PaymentsCompleteAuthorizeRedirectWithCredsIdentifierRuntime
        m_payments_complete_authorize_redirect_with_creds_identifier;
    PaymentsConfirmRuntime m_payments_confirm;
    PaymentsConnectorSessionRuntime m_payments_connector_session;
    PaymentsCreateRuntime m_payments_create;
    PaymentsCreateAndConfirmIntentRuntime m_payments_create_and_confirm_intent;
    PaymentsCreateIntentRuntime m_payments_create_intent;
    PaymentsDynamicTaxCalculationRuntime m_payments_dynamic_tax_calculation;
    PaymentsExtendAuthorizationRuntime m_payments_extend_authorization;
    PaymentsExternalAuthenticationRuntime m_payments_external_authentication;
    PaymentsFinishRedirectionRuntime m_payments_finish_redirection;
    PaymentsGetIntentRuntime m_payments_get_intent;
    PaymentsIncrementalAuthorizationRuntime m_payments_incremental_authorization;
    PaymentsListRuntime m_payments_list;
    PaymentsListByFilterRuntime m_payments_list_by_filter;
    PaymentsListForPlatformRuntime m_payments_list_for_platform;
    PaymentsListForPlatformFiltersRuntime m_payments_list_for_platform_filters;
    PaymentsManualStatusUpdateRuntime m_payments_manual_status_update;
    PaymentsManualStatusUpdateEligibleStatusesRuntime
        m_payments_manual_status_update_eligible_statuses;
    PaymentsManualUpdateRuntime m_payments_manual_update;
    PaymentsPostSessionTokensRuntime m_payments_post_session_tokens;
    PaymentsRedirectResponseRuntime m_payments_redirect_response;
    PaymentsRedirectResponseWithCredsIdentifierRuntime
        m_payments_redirect_response_with_creds_identifier;
    PaymentsRejectRuntime m_payments_reject;
    PaymentsRetrieveRuntime m_payments_retrieve;
    PaymentsRetrieveWithGatewayCredsRuntime m_payments_retrieve_with_gateway_creds;
    PaymentsStartRuntime m_payments_start;
    PaymentsStartRedirectionRuntime m_payments_start_redirection;
    PaymentsStatusWithGatewayCredsRuntime m_payments_status_with_gateway_creds;
    PaymentsSubmitEligibilityRuntime m_payments_submit_eligibility;
    PaymentsSubmitEligibilityCheckRuntime m_payments_submit_eligibility_check;
    PaymentsUpdateRuntime m_payments_update;
    PaymentsUpdateIntentRuntime m_payments_update_intent;
    PaymentsUpdateMetadataRuntime m_payments_update_metadata;
    Post3dsPaymentsAuthorizeRuntime m_post_3ds_payments_authorize;
    ProfilePaymentsListRuntime m_profile_payments_list;
    ProfilePaymentsListByFilterRuntime m_profile_payments_list_by_filter;
    ProxyConfirmIntentRuntime m_proxy_confirm_intent;
    RecoveryPaymentsCreateRuntime m_recovery_payments_create;
    RetrieveExtendedCardInfoRuntime m_retrieve_extended_card_info;
    RevenueRecoveryGetIntentRuntime m_revenue_recovery_get_intent;
    RevenueRecoveryInvoicesListRuntime m_revenue_recovery_invoices_list;
    RevenueRecoveryInvoicesListProfileRuntime m_revenue_recovery_invoices_list_profile;
    GetPaymentListFromOpensearchRuntime m_get_payment_list_from_opensearch;
    GetProfilePaymentListFromOpensearchRuntime m_get_profile_payment_list_from_opensearch;
    ListPaymentMethodsForPaymentsClientRuntime m_list_payment_methods_for_payments_client;
};

} // namespace app::handlers::payments
