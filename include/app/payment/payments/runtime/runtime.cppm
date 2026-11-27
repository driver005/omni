export module app_payment:payments_runtime;
import core_behavior;

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

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    confirm_intent_with_external_vault_proxy(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_confirm_intent_with_external_vault_proxy.confirm_intent_with_external_vault_proxy(
            dto,
            ctx
        );
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    get_filters_for_payments(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_get_filters_for_payments.get_filters_for_payments(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    get_payment_filters(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_get_payment_filters.get_payment_filters(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    get_payment_filters_profile(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_get_payment_filters_profile.get_payment_filters_profile(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsAggregateResponse>
    get_payments_aggregates(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_get_payments_aggregates.get_payments_aggregates(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsAggregateResponse>
    get_payments_aggregates_profile(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_get_payments_aggregates_profile.get_payments_aggregates_profile(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentListResponseV2>
    list_payment_attempts(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_list_payment_attempts.list_payment_attempts(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentListResponseV2>
    list_payment_methods(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_list_payment_methods.list_payment_methods(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payment_confirm_intent(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payment_confirm_intent.payment_confirm_intent(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentListResponseV2>
    payment_get_intent_using_merchant_reference_id(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payment_get_intent_using_merchant_reference_id
            .payment_get_intent_using_merchant_reference_id(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentLinkResponse>
    payment_link_create(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payment_link_create.payment_link_create(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentListResponseV2>
    payment_status(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payment_status.payment_status(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_apply_pm_data(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_apply_pm_data.payments_apply_pm_data(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_approve(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_approve.payments_approve(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse> payments_cancel(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_cancel.payments_cancel(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_cancel_post_capture(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_cancel_post_capture.payments_cancel_post_capture(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentListResponseV2>
    payments_cancel_post_capture_retrieve(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_cancel_post_capture_retrieve.payments_cancel_post_capture_retrieve(
            dto,
            ctx
        );
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_capture(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_capture.payments_capture(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_complete_authorize(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_complete_authorize.payments_complete_authorize(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsRedirectionResponse>
    payments_complete_authorize_redirect(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_complete_authorize_redirect.payments_complete_authorize_redirect(
            dto,
            ctx
        );
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsRedirectionResponse>
    payments_complete_authorize_redirect_with_creds_identifier(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_complete_authorize_redirect_with_creds_identifier
            .payments_complete_authorize_redirect_with_creds_identifier(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_confirm(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_confirm.payments_confirm(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_connector_session(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_connector_session.payments_connector_session(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse> payments_create(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_create.payments_create(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_create_and_confirm_intent(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_create_and_confirm_intent.payments_create_and_confirm_intent(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsIntentResponse>
    payments_create_intent(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_create_intent.payments_create_intent(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_dynamic_tax_calculation(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_dynamic_tax_calculation.payments_dynamic_tax_calculation(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_extend_authorization(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_extend_authorization.payments_extend_authorization(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_external_authentication(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_external_authentication.payments_external_authentication(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsRedirectionResponse>
    payments_finish_redirection(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_finish_redirection.payments_finish_redirection(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentListResponseV2>
    payments_get_intent(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_get_intent.payments_get_intent(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_incremental_authorization(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_incremental_authorization.payments_incremental_authorization(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentListResponseV2>
    payments_list(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_list.payments_list(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentListResponseV2>
    payments_list_by_filter(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_list_by_filter.payments_list_by_filter(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentListResponseV2>
    payments_list_for_platform(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_list_for_platform.payments_list_for_platform(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_list_for_platform_filters(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_list_for_platform_filters.payments_list_for_platform_filters(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_manual_status_update(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_manual_status_update.payments_manual_status_update(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_manual_status_update_eligible_statuses(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_manual_status_update_eligible_statuses
            .payments_manual_status_update_eligible_statuses(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_manual_update(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_manual_update.payments_manual_update(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_post_session_tokens(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_post_session_tokens.payments_post_session_tokens(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsRedirectionResponse>
    payments_redirect_response(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_redirect_response.payments_redirect_response(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsRedirectionResponse>
    payments_redirect_response_with_creds_identifier(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_redirect_response_with_creds_identifier
            .payments_redirect_response_with_creds_identifier(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse> payments_reject(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_reject.payments_reject(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentListResponseV2>
    payments_retrieve(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_retrieve.payments_retrieve(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentListResponseV2>
    payments_retrieve_with_gateway_creds(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_retrieve_with_gateway_creds.payments_retrieve_with_gateway_creds(
            dto,
            ctx
        );
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsRedirectionResponse>
    payments_start(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_start.payments_start(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsRedirectionResponse>
    payments_start_redirection(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_start_redirection.payments_start_redirection(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentListResponseV2>
    payments_status_with_gateway_creds(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_status_with_gateway_creds.payments_status_with_gateway_creds(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_submit_eligibility(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_submit_eligibility.payments_submit_eligibility(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_submit_eligibility_check(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_submit_eligibility_check.payments_submit_eligibility_check(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse> payments_update(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_update.payments_update(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_update_intent(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_update_intent.payments_update_intent(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    payments_update_metadata(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payments_update_metadata.payments_update_metadata(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsRedirectionResponse>
    post_3ds_payments_authorize(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_post_3ds_payments_authorize.post_3ds_payments_authorize(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentListResponseV2>
    profile_payments_list(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_profile_payments_list.profile_payments_list(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentListResponseV2>
    profile_payments_list_by_filter(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_profile_payments_list_by_filter.profile_payments_list_by_filter(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    proxy_confirm_intent(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_proxy_confirm_intent.proxy_confirm_intent(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsResponse>
    recovery_payments_create(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_recovery_payments_create.recovery_payments_create(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::ExtendedCardInfoResponse>
    retrieve_extended_card_info(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_retrieve_extended_card_info.retrieve_extended_card_info(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentListResponseV2>
    revenue_recovery_get_intent(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_revenue_recovery_get_intent.revenue_recovery_get_intent(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentListResponseV2>
    revenue_recovery_invoices_list(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_revenue_recovery_invoices_list.revenue_recovery_invoices_list(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentListResponseV2>
    revenue_recovery_invoices_list_profile(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_revenue_recovery_invoices_list_profile.revenue_recovery_invoices_list_profile(
            dto,
            ctx
        );
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsAdvancedListResponse>
    get_payment_list_from_opensearch(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_get_payment_list_from_opensearch.get_payment_list_from_opensearch(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsAdvancedListResponse>
    get_profile_payment_list_from_opensearch(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_get_profile_payment_list_from_opensearch.get_profile_payment_list_from_opensearch(
            dto,
            ctx
        );
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsClientMethodsResponse>
    list_payment_methods_for_payments_client(
        const ::app::model::payments::PaymentsRequest& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_list_payment_methods_for_payments_client.list_payment_methods_for_payments_client(
            dto,
            ctx
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
