export module app_payment:payments_result;

import std;
import core_serde;
import app_model;

export namespace app::handlers::payments {

// Each Result derives from the exact model DTO the runtime builds, so wire fields
// stay identical. Serializable reuses the DTO FieldDesc tuple (no raw JSON).

// Result for handle_confirm_intent_with_external_vault_proxy — typed PaymentsResponse.
class ConfirmIntentWithExternalVaultProxyResult : public ::app::model::payments::PaymentsResponse
{
public:
    ConfirmIntentWithExternalVaultProxyResult() = default;

    explicit ConfirmIntentWithExternalVaultProxyResult(
        ::app::model::payments::PaymentsResponse value
    ) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_get_filters_for_payments — typed PaymentsResponse.
class GetFiltersForPaymentsResult : public ::app::model::payments::PaymentsResponse
{
public:
    GetFiltersForPaymentsResult() = default;

    explicit GetFiltersForPaymentsResult(::app::model::payments::PaymentsResponse value) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_get_payment_filters — typed PaymentsResponse.
class GetPaymentFiltersResult : public ::app::model::payments::PaymentsResponse
{
public:
    GetPaymentFiltersResult() = default;

    explicit GetPaymentFiltersResult(::app::model::payments::PaymentsResponse value) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_get_payment_filters_profile — typed PaymentsResponse.
class GetPaymentFiltersProfileResult : public ::app::model::payments::PaymentsResponse
{
public:
    GetPaymentFiltersProfileResult() = default;

    explicit GetPaymentFiltersProfileResult(::app::model::payments::PaymentsResponse value) noexcept
        :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_get_payments_aggregates — typed PaymentsAggregateResponse.
class GetPaymentsAggregatesResult : public ::app::model::payments::PaymentsAggregateResponse
{
public:
    GetPaymentsAggregatesResult() = default;

    explicit GetPaymentsAggregatesResult(
        ::app::model::payments::PaymentsAggregateResponse value
    ) noexcept :
        PaymentsAggregateResponse{std::move(value)}
    {
    }
};

// Result for handle_get_payments_aggregates_profile — typed PaymentsAggregateResponse.
class GetPaymentsAggregatesProfileResult : public ::app::model::payments::PaymentsAggregateResponse
{
public:
    GetPaymentsAggregatesProfileResult() = default;

    explicit GetPaymentsAggregatesProfileResult(
        ::app::model::payments::PaymentsAggregateResponse value
    ) noexcept :
        PaymentsAggregateResponse{std::move(value)}
    {
    }
};

// Result for handle_list_payment_attempts — typed PaymentListResponseV2.
class ListPaymentAttemptsResult : public ::app::model::payments::PaymentListResponseV2
{
public:
    ListPaymentAttemptsResult() = default;

    explicit ListPaymentAttemptsResult(::app::model::payments::PaymentListResponseV2 value) noexcept
        :
        PaymentListResponseV2{std::move(value)}
    {
    }
};

// Result for handle_list_payment_methods — typed PaymentListResponseV2.
class ListPaymentMethodsResult : public ::app::model::payments::PaymentListResponseV2
{
public:
    ListPaymentMethodsResult() = default;

    explicit ListPaymentMethodsResult(::app::model::payments::PaymentListResponseV2 value) noexcept
        :
        PaymentListResponseV2{std::move(value)}
    {
    }
};

// Result for handle_payment_confirm_intent — typed PaymentsResponse.
class PaymentConfirmIntentResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentConfirmIntentResult() = default;

    explicit PaymentConfirmIntentResult(::app::model::payments::PaymentsResponse value) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payment_get_intent_using_merchant_reference_id — typed PaymentListResponseV2.
class PaymentGetIntentUsingMerchantReferenceIdResult :
    public ::app::model::payments::PaymentListResponseV2
{
public:
    PaymentGetIntentUsingMerchantReferenceIdResult() = default;

    explicit PaymentGetIntentUsingMerchantReferenceIdResult(
        ::app::model::payments::PaymentListResponseV2 value
    ) noexcept :
        PaymentListResponseV2{std::move(value)}
    {
    }
};

// Result for handle_payment_link_create — typed PaymentLinkResponse.
class PaymentLinkCreateResult : public ::app::model::payments::PaymentLinkResponse
{
public:
    PaymentLinkCreateResult() = default;

    explicit PaymentLinkCreateResult(::app::model::payments::PaymentLinkResponse value) noexcept :
        PaymentLinkResponse{std::move(value)}
    {
    }
};

// Result for handle_payment_status — typed PaymentListResponseV2.
class PaymentStatusResult : public ::app::model::payments::PaymentListResponseV2
{
public:
    PaymentStatusResult() = default;

    explicit PaymentStatusResult(::app::model::payments::PaymentListResponseV2 value) noexcept :
        PaymentListResponseV2{std::move(value)}
    {
    }
};

// Result for handle_payments_apply_pm_data — typed PaymentsResponse.
class PaymentsApplyPmDataResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsApplyPmDataResult() = default;

    explicit PaymentsApplyPmDataResult(::app::model::payments::PaymentsResponse value) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_approve — typed PaymentsResponse.
class PaymentsApproveResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsApproveResult() = default;

    explicit PaymentsApproveResult(::app::model::payments::PaymentsResponse value) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_cancel — typed PaymentsResponse.
class PaymentsCancelResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsCancelResult() = default;

    explicit PaymentsCancelResult(::app::model::payments::PaymentsResponse value) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_cancel_post_capture — typed PaymentsResponse.
class PaymentsCancelPostCaptureResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsCancelPostCaptureResult() = default;

    explicit PaymentsCancelPostCaptureResult(
        ::app::model::payments::PaymentsResponse value
    ) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_cancel_post_capture_retrieve — typed PaymentListResponseV2.
class PaymentsCancelPostCaptureRetrieveResult : public ::app::model::payments::PaymentListResponseV2
{
public:
    PaymentsCancelPostCaptureRetrieveResult() = default;

    explicit PaymentsCancelPostCaptureRetrieveResult(
        ::app::model::payments::PaymentListResponseV2 value
    ) noexcept :
        PaymentListResponseV2{std::move(value)}
    {
    }
};

// Result for handle_payments_capture — typed PaymentsResponse.
class PaymentsCaptureResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsCaptureResult() = default;

    explicit PaymentsCaptureResult(::app::model::payments::PaymentsResponse value) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_complete_authorize — typed PaymentsResponse.
class PaymentsCompleteAuthorizeResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsCompleteAuthorizeResult() = default;

    explicit PaymentsCompleteAuthorizeResult(
        ::app::model::payments::PaymentsResponse value
    ) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_complete_authorize_redirect — typed PaymentsRedirectionResponse.
class PaymentsCompleteAuthorizeRedirectResult :
    public ::app::model::payments::PaymentsRedirectionResponse
{
public:
    PaymentsCompleteAuthorizeRedirectResult() = default;

    explicit PaymentsCompleteAuthorizeRedirectResult(
        ::app::model::payments::PaymentsRedirectionResponse value
    ) noexcept :
        PaymentsRedirectionResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_complete_authorize_redirect_with_creds_identifier — typed
// PaymentsRedirectionResponse.
class PaymentsCompleteAuthorizeRedirectWithCredsIdentifierResult :
    public ::app::model::payments::PaymentsRedirectionResponse
{
public:
    PaymentsCompleteAuthorizeRedirectWithCredsIdentifierResult() = default;

    explicit PaymentsCompleteAuthorizeRedirectWithCredsIdentifierResult(
        ::app::model::payments::PaymentsRedirectionResponse value
    ) noexcept :
        PaymentsRedirectionResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_confirm — typed PaymentsResponse.
class PaymentsConfirmResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsConfirmResult() = default;

    explicit PaymentsConfirmResult(::app::model::payments::PaymentsResponse value) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_connector_session — typed PaymentsResponse.
class PaymentsConnectorSessionResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsConnectorSessionResult() = default;

    explicit PaymentsConnectorSessionResult(::app::model::payments::PaymentsResponse value) noexcept
        :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_create — typed PaymentsResponse.
class PaymentsCreateResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsCreateResult() = default;

    explicit PaymentsCreateResult(::app::model::payments::PaymentsResponse value) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_create_and_confirm_intent — typed PaymentsResponse.
class PaymentsCreateAndConfirmIntentResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsCreateAndConfirmIntentResult() = default;

    explicit PaymentsCreateAndConfirmIntentResult(
        ::app::model::payments::PaymentsResponse value
    ) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_create_intent — typed PaymentsIntentResponse.
class PaymentsCreateIntentResult : public ::app::model::payments::PaymentsIntentResponse
{
public:
    PaymentsCreateIntentResult() = default;

    explicit PaymentsCreateIntentResult(
        ::app::model::payments::PaymentsIntentResponse value
    ) noexcept :
        PaymentsIntentResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_dynamic_tax_calculation — typed PaymentsResponse.
class PaymentsDynamicTaxCalculationResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsDynamicTaxCalculationResult() = default;

    explicit PaymentsDynamicTaxCalculationResult(
        ::app::model::payments::PaymentsResponse value
    ) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_extend_authorization — typed PaymentsResponse.
class PaymentsExtendAuthorizationResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsExtendAuthorizationResult() = default;

    explicit PaymentsExtendAuthorizationResult(
        ::app::model::payments::PaymentsResponse value
    ) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_external_authentication — typed PaymentsResponse.
class PaymentsExternalAuthenticationResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsExternalAuthenticationResult() = default;

    explicit PaymentsExternalAuthenticationResult(
        ::app::model::payments::PaymentsResponse value
    ) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_finish_redirection — typed PaymentsRedirectionResponse.
class PaymentsFinishRedirectionResult : public ::app::model::payments::PaymentsRedirectionResponse
{
public:
    PaymentsFinishRedirectionResult() = default;

    explicit PaymentsFinishRedirectionResult(
        ::app::model::payments::PaymentsRedirectionResponse value
    ) noexcept :
        PaymentsRedirectionResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_get_intent — typed PaymentListResponseV2.
class PaymentsGetIntentResult : public ::app::model::payments::PaymentListResponseV2
{
public:
    PaymentsGetIntentResult() = default;

    explicit PaymentsGetIntentResult(::app::model::payments::PaymentListResponseV2 value) noexcept :
        PaymentListResponseV2{std::move(value)}
    {
    }
};

// Result for handle_payments_incremental_authorization — typed PaymentsResponse.
class PaymentsIncrementalAuthorizationResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsIncrementalAuthorizationResult() = default;

    explicit PaymentsIncrementalAuthorizationResult(
        ::app::model::payments::PaymentsResponse value
    ) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_list — typed PaymentListResponseV2.
class PaymentsListResult : public ::app::model::payments::PaymentListResponseV2
{
public:
    PaymentsListResult() = default;

    explicit PaymentsListResult(::app::model::payments::PaymentListResponseV2 value) noexcept :
        PaymentListResponseV2{std::move(value)}
    {
    }
};

// Result for handle_payments_list_by_filter — typed PaymentListResponseV2.
class PaymentsListByFilterResult : public ::app::model::payments::PaymentListResponseV2
{
public:
    PaymentsListByFilterResult() = default;

    explicit PaymentsListByFilterResult(
        ::app::model::payments::PaymentListResponseV2 value
    ) noexcept :
        PaymentListResponseV2{std::move(value)}
    {
    }
};

// Result for handle_payments_list_for_platform — typed PaymentListResponseV2.
class PaymentsListForPlatformResult : public ::app::model::payments::PaymentListResponseV2
{
public:
    PaymentsListForPlatformResult() = default;

    explicit PaymentsListForPlatformResult(
        ::app::model::payments::PaymentListResponseV2 value
    ) noexcept :
        PaymentListResponseV2{std::move(value)}
    {
    }
};

// Result for handle_payments_list_for_platform_filters — typed PaymentsResponse.
class PaymentsListForPlatformFiltersResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsListForPlatformFiltersResult() = default;

    explicit PaymentsListForPlatformFiltersResult(
        ::app::model::payments::PaymentsResponse value
    ) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_manual_status_update — typed PaymentsResponse.
class PaymentsManualStatusUpdateResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsManualStatusUpdateResult() = default;

    explicit PaymentsManualStatusUpdateResult(
        ::app::model::payments::PaymentsResponse value
    ) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_manual_status_update_eligible_statuses — typed PaymentsResponse.
class PaymentsManualStatusUpdateEligibleStatusesResult :
    public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsManualStatusUpdateEligibleStatusesResult() = default;

    explicit PaymentsManualStatusUpdateEligibleStatusesResult(
        ::app::model::payments::PaymentsResponse value
    ) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_manual_update — typed PaymentsResponse.
class PaymentsManualUpdateResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsManualUpdateResult() = default;

    explicit PaymentsManualUpdateResult(::app::model::payments::PaymentsResponse value) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_post_session_tokens — typed PaymentsResponse.
class PaymentsPostSessionTokensResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsPostSessionTokensResult() = default;

    explicit PaymentsPostSessionTokensResult(
        ::app::model::payments::PaymentsResponse value
    ) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_redirect_response — typed PaymentsRedirectionResponse.
class PaymentsRedirectResponseResult : public ::app::model::payments::PaymentsRedirectionResponse
{
public:
    PaymentsRedirectResponseResult() = default;

    explicit PaymentsRedirectResponseResult(
        ::app::model::payments::PaymentsRedirectionResponse value
    ) noexcept :
        PaymentsRedirectionResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_redirect_response_with_creds_identifier — typed
// PaymentsRedirectionResponse.
class PaymentsRedirectResponseWithCredsIdentifierResult :
    public ::app::model::payments::PaymentsRedirectionResponse
{
public:
    PaymentsRedirectResponseWithCredsIdentifierResult() = default;

    explicit PaymentsRedirectResponseWithCredsIdentifierResult(
        ::app::model::payments::PaymentsRedirectionResponse value
    ) noexcept :
        PaymentsRedirectionResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_reject — typed PaymentsResponse.
class PaymentsRejectResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsRejectResult() = default;

    explicit PaymentsRejectResult(::app::model::payments::PaymentsResponse value) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_retrieve — typed PaymentListResponseV2.
class PaymentsRetrieveResult : public ::app::model::payments::PaymentListResponseV2
{
public:
    PaymentsRetrieveResult() = default;

    explicit PaymentsRetrieveResult(::app::model::payments::PaymentListResponseV2 value) noexcept :
        PaymentListResponseV2{std::move(value)}
    {
    }
};

// Result for handle_payments_retrieve_with_gateway_creds — typed PaymentListResponseV2.
class PaymentsRetrieveWithGatewayCredsResult : public ::app::model::payments::PaymentListResponseV2
{
public:
    PaymentsRetrieveWithGatewayCredsResult() = default;

    explicit PaymentsRetrieveWithGatewayCredsResult(
        ::app::model::payments::PaymentListResponseV2 value
    ) noexcept :
        PaymentListResponseV2{std::move(value)}
    {
    }
};

// Result for handle_payments_start — typed PaymentsRedirectionResponse.
class PaymentsStartResult : public ::app::model::payments::PaymentsRedirectionResponse
{
public:
    PaymentsStartResult() = default;

    explicit PaymentsStartResult(::app::model::payments::PaymentsRedirectionResponse value) noexcept
        :
        PaymentsRedirectionResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_start_redirection — typed PaymentsRedirectionResponse.
class PaymentsStartRedirectionResult : public ::app::model::payments::PaymentsRedirectionResponse
{
public:
    PaymentsStartRedirectionResult() = default;

    explicit PaymentsStartRedirectionResult(
        ::app::model::payments::PaymentsRedirectionResponse value
    ) noexcept :
        PaymentsRedirectionResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_status_with_gateway_creds — typed PaymentListResponseV2.
class PaymentsStatusWithGatewayCredsResult : public ::app::model::payments::PaymentListResponseV2
{
public:
    PaymentsStatusWithGatewayCredsResult() = default;

    explicit PaymentsStatusWithGatewayCredsResult(
        ::app::model::payments::PaymentListResponseV2 value
    ) noexcept :
        PaymentListResponseV2{std::move(value)}
    {
    }
};

// Result for handle_payments_submit_eligibility — typed PaymentsResponse.
class PaymentsSubmitEligibilityResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsSubmitEligibilityResult() = default;

    explicit PaymentsSubmitEligibilityResult(
        ::app::model::payments::PaymentsResponse value
    ) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_submit_eligibility_check — typed PaymentsResponse.
class PaymentsSubmitEligibilityCheckResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsSubmitEligibilityCheckResult() = default;

    explicit PaymentsSubmitEligibilityCheckResult(
        ::app::model::payments::PaymentsResponse value
    ) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_update — typed PaymentsResponse.
class PaymentsUpdateResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsUpdateResult() = default;

    explicit PaymentsUpdateResult(::app::model::payments::PaymentsResponse value) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_update_intent — typed PaymentsResponse.
class PaymentsUpdateIntentResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsUpdateIntentResult() = default;

    explicit PaymentsUpdateIntentResult(::app::model::payments::PaymentsResponse value) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_payments_update_metadata — typed PaymentsResponse.
class PaymentsUpdateMetadataResult : public ::app::model::payments::PaymentsResponse
{
public:
    PaymentsUpdateMetadataResult() = default;

    explicit PaymentsUpdateMetadataResult(::app::model::payments::PaymentsResponse value) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_post_3ds_payments_authorize — typed PaymentsRedirectionResponse.
class Post3dsPaymentsAuthorizeResult : public ::app::model::payments::PaymentsRedirectionResponse
{
public:
    Post3dsPaymentsAuthorizeResult() = default;

    explicit Post3dsPaymentsAuthorizeResult(
        ::app::model::payments::PaymentsRedirectionResponse value
    ) noexcept :
        PaymentsRedirectionResponse{std::move(value)}
    {
    }
};

// Result for handle_profile_payments_list — typed PaymentListResponseV2.
class ProfilePaymentsListResult : public ::app::model::payments::PaymentListResponseV2
{
public:
    ProfilePaymentsListResult() = default;

    explicit ProfilePaymentsListResult(::app::model::payments::PaymentListResponseV2 value) noexcept
        :
        PaymentListResponseV2{std::move(value)}
    {
    }
};

// Result for handle_profile_payments_list_by_filter — typed PaymentListResponseV2.
class ProfilePaymentsListByFilterResult : public ::app::model::payments::PaymentListResponseV2
{
public:
    ProfilePaymentsListByFilterResult() = default;

    explicit ProfilePaymentsListByFilterResult(
        ::app::model::payments::PaymentListResponseV2 value
    ) noexcept :
        PaymentListResponseV2{std::move(value)}
    {
    }
};

// Result for handle_proxy_confirm_intent — typed PaymentsResponse.
class ProxyConfirmIntentResult : public ::app::model::payments::PaymentsResponse
{
public:
    ProxyConfirmIntentResult() = default;

    explicit ProxyConfirmIntentResult(::app::model::payments::PaymentsResponse value) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_recovery_payments_create — typed PaymentsResponse.
class RecoveryPaymentsCreateResult : public ::app::model::payments::PaymentsResponse
{
public:
    RecoveryPaymentsCreateResult() = default;

    explicit RecoveryPaymentsCreateResult(::app::model::payments::PaymentsResponse value) noexcept :
        PaymentsResponse{std::move(value)}
    {
    }
};

// Result for handle_retrieve_extended_card_info — typed ExtendedCardInfoResponse.
class RetrieveExtendedCardInfoResult : public ::app::model::payments::ExtendedCardInfoResponse
{
public:
    RetrieveExtendedCardInfoResult() = default;

    explicit RetrieveExtendedCardInfoResult(
        ::app::model::payments::ExtendedCardInfoResponse value
    ) noexcept :
        ExtendedCardInfoResponse{std::move(value)}
    {
    }
};

// Result for handle_revenue_recovery_get_intent — typed PaymentListResponseV2.
class RevenueRecoveryGetIntentResult : public ::app::model::payments::PaymentListResponseV2
{
public:
    RevenueRecoveryGetIntentResult() = default;

    explicit RevenueRecoveryGetIntentResult(
        ::app::model::payments::PaymentListResponseV2 value
    ) noexcept :
        PaymentListResponseV2{std::move(value)}
    {
    }
};

// Result for handle_revenue_recovery_invoices_list — typed PaymentListResponseV2.
class RevenueRecoveryInvoicesListResult : public ::app::model::payments::PaymentListResponseV2
{
public:
    RevenueRecoveryInvoicesListResult() = default;

    explicit RevenueRecoveryInvoicesListResult(
        ::app::model::payments::PaymentListResponseV2 value
    ) noexcept :
        PaymentListResponseV2{std::move(value)}
    {
    }
};

// Result for handle_revenue_recovery_invoices_list_profile — typed PaymentListResponseV2.
class RevenueRecoveryInvoicesListProfileResult :
    public ::app::model::payments::PaymentListResponseV2
{
public:
    RevenueRecoveryInvoicesListProfileResult() = default;

    explicit RevenueRecoveryInvoicesListProfileResult(
        ::app::model::payments::PaymentListResponseV2 value
    ) noexcept :
        PaymentListResponseV2{std::move(value)}
    {
    }
};

// Result for handle_get_payment_list_from_opensearch — typed PaymentsAdvancedListResponse.
class GetPaymentListFromOpensearchResult :
    public ::app::model::payments::PaymentsAdvancedListResponse
{
public:
    GetPaymentListFromOpensearchResult() = default;

    explicit GetPaymentListFromOpensearchResult(
        ::app::model::payments::PaymentsAdvancedListResponse value
    ) noexcept :
        PaymentsAdvancedListResponse{std::move(value)}
    {
    }
};

// Result for handle_get_profile_payment_list_from_opensearch — typed PaymentsAdvancedListResponse.
class GetProfilePaymentListFromOpensearchResult :
    public ::app::model::payments::PaymentsAdvancedListResponse
{
public:
    GetProfilePaymentListFromOpensearchResult() = default;

    explicit GetProfilePaymentListFromOpensearchResult(
        ::app::model::payments::PaymentsAdvancedListResponse value
    ) noexcept :
        PaymentsAdvancedListResponse{std::move(value)}
    {
    }
};

// Result for handle_list_payment_methods_for_payments_client — typed PaymentsClientMethodsResponse.
class ListPaymentMethodsForPaymentsClientResult :
    public ::app::model::payments::PaymentsClientMethodsResponse
{
public:
    ListPaymentMethodsForPaymentsClientResult() = default;

    explicit ListPaymentMethodsForPaymentsClientResult(
        ::app::model::payments::PaymentsClientMethodsResponse value
    ) noexcept :
        PaymentsClientMethodsResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::payments

template<>
struct serde::Serializable<::app::handlers::payments::ConfirmIntentWithExternalVaultProxyResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "ConfirmIntentWithExternalVaultProxyResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::GetFiltersForPaymentsResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "GetFiltersForPaymentsResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::GetPaymentFiltersResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "GetPaymentFiltersResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::GetPaymentFiltersProfileResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "GetPaymentFiltersProfileResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::GetPaymentsAggregatesResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsAggregateResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "GetPaymentsAggregatesResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::GetPaymentsAggregatesProfileResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsAggregateResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "GetPaymentsAggregatesProfileResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::ListPaymentAttemptsResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentListResponseV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "ListPaymentAttemptsResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::ListPaymentMethodsResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentListResponseV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "ListPaymentMethodsResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentConfirmIntentResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentConfirmIntentResult";
    }
};

template<>
struct serde::Serializable<
    ::app::handlers::payments::PaymentGetIntentUsingMerchantReferenceIdResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentListResponseV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentGetIntentUsingMerchantReferenceIdResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentLinkCreateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentLinkResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentLinkCreateResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentStatusResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentListResponseV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentStatusResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsApplyPmDataResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsApplyPmDataResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsApproveResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsApproveResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsCancelResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsCancelResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsCancelPostCaptureResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsCancelPostCaptureResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsCancelPostCaptureRetrieveResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentListResponseV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsCancelPostCaptureRetrieveResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsCaptureResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsCaptureResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsCompleteAuthorizeResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsCompleteAuthorizeResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsCompleteAuthorizeRedirectResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsRedirectionResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsCompleteAuthorizeRedirectResult";
    }
};

template<>
struct serde::Serializable<
    ::app::handlers::payments::PaymentsCompleteAuthorizeRedirectWithCredsIdentifierResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsRedirectionResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsCompleteAuthorizeRedirectWithCredsIdentifierResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsConfirmResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsConfirmResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsConnectorSessionResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsConnectorSessionResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsCreateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsCreateResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsCreateAndConfirmIntentResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsCreateAndConfirmIntentResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsCreateIntentResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsIntentResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsCreateIntentResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsDynamicTaxCalculationResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsDynamicTaxCalculationResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsExtendAuthorizationResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsExtendAuthorizationResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsExternalAuthenticationResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsExternalAuthenticationResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsFinishRedirectionResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsRedirectionResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsFinishRedirectionResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsGetIntentResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentListResponseV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsGetIntentResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsIncrementalAuthorizationResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsIncrementalAuthorizationResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsListResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentListResponseV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsListResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsListByFilterResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentListResponseV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsListByFilterResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsListForPlatformResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentListResponseV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsListForPlatformResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsListForPlatformFiltersResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsListForPlatformFiltersResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsManualStatusUpdateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsManualStatusUpdateResult";
    }
};

template<>
struct serde::Serializable<
    ::app::handlers::payments::PaymentsManualStatusUpdateEligibleStatusesResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsManualStatusUpdateEligibleStatusesResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsManualUpdateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsManualUpdateResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsPostSessionTokensResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsPostSessionTokensResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsRedirectResponseResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsRedirectionResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsRedirectResponseResult";
    }
};

template<>
struct serde::Serializable<
    ::app::handlers::payments::PaymentsRedirectResponseWithCredsIdentifierResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsRedirectionResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsRedirectResponseWithCredsIdentifierResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsRejectResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsRejectResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsRetrieveResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentListResponseV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsRetrieveResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsRetrieveWithGatewayCredsResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentListResponseV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsRetrieveWithGatewayCredsResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsStartResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsRedirectionResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsStartResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsStartRedirectionResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsRedirectionResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsStartRedirectionResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsStatusWithGatewayCredsResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentListResponseV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsStatusWithGatewayCredsResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsSubmitEligibilityResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsSubmitEligibilityResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsSubmitEligibilityCheckResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsSubmitEligibilityCheckResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsUpdateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsUpdateResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsUpdateIntentResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsUpdateIntentResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::PaymentsUpdateMetadataResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "PaymentsUpdateMetadataResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::Post3dsPaymentsAuthorizeResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsRedirectionResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "Post3dsPaymentsAuthorizeResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::ProfilePaymentsListResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentListResponseV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "ProfilePaymentsListResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::ProfilePaymentsListByFilterResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentListResponseV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "ProfilePaymentsListByFilterResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::ProxyConfirmIntentResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "ProxyConfirmIntentResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::RecoveryPaymentsCreateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "RecoveryPaymentsCreateResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::RetrieveExtendedCardInfoResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::ExtendedCardInfoResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "RetrieveExtendedCardInfoResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::RevenueRecoveryGetIntentResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentListResponseV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "RevenueRecoveryGetIntentResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::RevenueRecoveryInvoicesListResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentListResponseV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "RevenueRecoveryInvoicesListResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::RevenueRecoveryInvoicesListProfileResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentListResponseV2>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "RevenueRecoveryInvoicesListProfileResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::GetPaymentListFromOpensearchResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsAdvancedListResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "GetPaymentListFromOpensearchResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::GetProfilePaymentListFromOpensearchResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsAdvancedListResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "GetProfilePaymentListFromOpensearchResult";
    }
};

template<>
struct serde::Serializable<::app::handlers::payments::ListPaymentMethodsForPaymentsClientResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::payments::PaymentsClientMethodsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "ListPaymentMethodsForPaymentsClientResult";
    }
};
