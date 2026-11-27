export module app_payment:payment_methods_runtime;

import std;
import core_error;
import app_context;

import :payment_methods_input;
import :payment_methods_result;
import :payment_methods_runtime_common;
import :payment_methods_runtime_create_payment_method_api;
import :payment_methods_runtime_get_pm_nt_eligibility_api;
import :payment_methods_runtime_create_payment_method_intent_api;
import :payment_methods_runtime_payment_method_update_api;
import :payment_methods_runtime_payment_method_retrieve_api;
import :payment_methods_runtime_payment_method_delete_api;
import :payment_methods_runtime_migrate_payment_method_api;
import :payment_methods_runtime_migrate_payment_methods;
import :payment_methods_runtime_update_payment_methods;
import :payment_methods_runtime_payment_methods_batch_retrieve_api;
import :payment_methods_runtime_save_payment_method_api;
import :payment_methods_runtime_list_payment_method_api;
import :payment_methods_runtime_list_customer_payment_method_api;
import :payment_methods_runtime_list_customer_payment_method_api_client;
import :payment_methods_runtime_initiate_pm_collect_link_flow;
import :payment_methods_runtime_payment_method_retrieve_olap_api;
import :payment_methods_runtime_get_payment_method_token_data;
import :payment_methods_runtime_get_total_payment_method_count;
import :payment_methods_runtime_render_pm_collect_link;
import :payment_methods_runtime_list_countries_currencies_for_connector_payment_method;
import :payment_methods_runtime_default_payment_method_set_api;
import :payment_methods_runtime_tokenize_card_api;
import :payment_methods_runtime_tokenize_card_using_pm_api;
import :payment_methods_runtime_tokenize_card_batch_api;
import :payment_methods_runtime_payment_methods_session_create;
import :payment_methods_runtime_payment_methods_session_update;
import :payment_methods_runtime_payment_methods_session_retrieve;
import :payment_methods_runtime_payment_method_session_list_payment_methods;
import :payment_methods_runtime_payment_method_session_confirm;
import :payment_methods_runtime_payment_method_session_update_saved_payment_method;
import :payment_methods_runtime_payment_method_session_delete_saved_payment_method;
import :payment_methods_runtime_network_token_status_check_api;
import :payment_methods_runtime_payment_method_get_token_details_api;
import :payment_methods_runtime_list_payment_methods_for_payments_client;
import :payment_methods_runtime_modular_migrate_payment_methods;

export namespace app::handlers::payment_methods {

class PaymentMethodsRuntime
{
public:
    explicit PaymentMethodsRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_create_payment_method_api{context},
        m_get_pm_nt_eligibility_api{context},
        m_create_payment_method_intent_api{context},
        m_payment_method_update_api{context},
        m_payment_method_retrieve_api{context},
        m_payment_method_delete_api{context},
        m_migrate_payment_method_api{context},
        m_migrate_payment_methods{context},
        m_update_payment_methods{context},
        m_payment_methods_batch_retrieve_api{context},
        m_save_payment_method_api{context},
        m_list_payment_method_api{context},
        m_list_customer_payment_method_api{context},
        m_list_customer_payment_method_api_client{context},
        m_initiate_pm_collect_link_flow{context},
        m_payment_method_retrieve_olap_api{context},
        m_get_payment_method_token_data{context},
        m_get_total_payment_method_count{context},
        m_render_pm_collect_link{context},
        m_list_countries_currencies_for_connector_payment_method{context},
        m_default_payment_method_set_api{context},
        m_tokenize_card_api{context},
        m_tokenize_card_using_pm_api{context},
        m_tokenize_card_batch_api{context},
        m_payment_methods_session_create{context},
        m_payment_methods_session_update{context},
        m_payment_methods_session_retrieve{context},
        m_payment_method_session_list_payment_methods{context},
        m_payment_method_session_confirm{context},
        m_payment_method_session_update_saved_payment_method{context},
        m_payment_method_session_delete_saved_payment_method{context},
        m_network_token_status_check_api{context},
        m_payment_method_get_token_details_api{context},
        m_list_payment_methods_for_payments_client{context},
        m_modular_migrate_payment_methods{context}
    {
    }

#define OMNI_PM_FORWARD(method, result_type)                                           \
    template<class Dto>                                                                            \
    [[nodiscard]] core::behavior::Outcome<result_type> method(                                     \
        const Dto& dto, const core::behavior::CallContext& ctx                                     \
    ) const                                                                                        \
    {                                                                                              \
        return m_##method.method(dto, ctx);                                                        \
    }

    OMNI_PM_FORWARD(
        create_payment_method_api,
        ::app::model::payment_methods::PaymentMethodResponse
    )
    OMNI_PM_FORWARD(
        get_pm_nt_eligibility_api,
        ::app::model::payment_methods::GetNetworkTokenEiligibilityResponse
    )
    OMNI_PM_FORWARD(
        create_payment_method_intent_api,
        ::app::model::payment_methods::PaymentMethodResponse
    )
    OMNI_PM_FORWARD(
        payment_method_update_api,
        ::app::model::payment_methods::PaymentMethodUpdateResponse
    )
    OMNI_PM_FORWARD(
        payment_method_retrieve_api,
        ::app::model::payment_methods::PaymentMethodResponse
    )
    OMNI_PM_FORWARD(
        payment_method_delete_api,
        ::app::model::payment_methods::PaymentMethodDeleteResponse
    )
    OMNI_PM_FORWARD(
        migrate_payment_method_api,
        ::app::model::payment_methods::PaymentMethodMigrateResponse
    )
    OMNI_PM_FORWARD(
        migrate_payment_methods,
        ::app::model::payment_methods::ModularPaymentMethodMigrationResponse
    )
    OMNI_PM_FORWARD(
        update_payment_methods,
        ::app::model::payment_methods::PaymentMethodRecordUpdateResponse
    )
    OMNI_PM_FORWARD(
        payment_methods_batch_retrieve_api,
        std::vector<::app::model::payment_methods::PaymentMethodsBatchRetrieveResponse>
    )
    OMNI_PM_FORWARD(
        save_payment_method_api,
        ::app::model::payment_methods::PaymentMethodResponse
    )
    OMNI_PM_FORWARD(
        list_payment_method_api,
        ::app::model::payment_methods::PaymentMethodListResponse
    )
    OMNI_PM_FORWARD(
        list_customer_payment_method_api,
        ::app::model::payment_methods::CustomerPaymentMethodsListResponse
    )
    OMNI_PM_FORWARD(
        list_customer_payment_method_api_client,
        ::app::model::payment_methods::ClientPaymentMethodsListResponse
    )
    OMNI_PM_FORWARD(
        initiate_pm_collect_link_flow,
        ::app::model::payment_methods::PaymentMethodCollectLinkResponse
    )
    OMNI_PM_FORWARD(
        payment_method_retrieve_olap_api,
        ::app::model::payment_methods::PaymentMethodDetailsResponse
    )
    OMNI_PM_FORWARD(
        get_payment_method_token_data,
        ::app::model::payment_methods::TokenDataResponse
    )
    OMNI_PM_FORWARD(
        get_total_payment_method_count,
        ::app::model::payment_methods::TotalPaymentMethodCountResponse
    )
    OMNI_PM_FORWARD(
        render_pm_collect_link,
        ::app::model::payment_methods::PaymentMethodCollectLinkRenderRequest
    )
    OMNI_PM_FORWARD(
        list_countries_currencies_for_connector_payment_method,
        ::app::model::payment_methods::ListCountriesCurrenciesResponse
    )
    OMNI_PM_FORWARD(
        default_payment_method_set_api,
        ::app::model::payment_methods::CustomerDefaultPaymentMethodResponse
    )
    OMNI_PM_FORWARD(
        tokenize_card_api,
        ::app::model::payment_methods::CardNetworkTokenizeResponse
    )
    OMNI_PM_FORWARD(
        tokenize_card_using_pm_api,
        ::app::model::payment_methods::TokenizedCardValue1
    )
    OMNI_PM_FORWARD(
        tokenize_card_batch_api,
        std::vector<::app::model::payment_methods::TokenizedCardValue1>
    )
    OMNI_PM_FORWARD(
        payment_methods_session_create,
        ::app::model::payment_methods::PaymentMethodSessionResponse
    )
    OMNI_PM_FORWARD(
        payment_methods_session_update,
        ::app::model::payment_methods::PaymentMethodSessionResponse
    )
    OMNI_PM_FORWARD(
        payment_methods_session_retrieve,
        ::app::model::payment_methods::PaymentMethodSessionResponse
    )
    OMNI_PM_FORWARD(
        payment_method_session_list_payment_methods,
        ::app::model::payment_methods::PaymentMethodListResponseForSession
    )
    OMNI_PM_FORWARD(
        payment_method_session_confirm,
        ::app::model::payment_methods::PaymentMethodSessionResponse
    )
    OMNI_PM_FORWARD(
        payment_method_session_update_saved_payment_method,
        ::app::model::payment_methods::PaymentMethodResponse
    )
    OMNI_PM_FORWARD(
        payment_method_session_delete_saved_payment_method,
        ::app::model::payment_methods::PaymentMethodDeleteSessionResponse
    )
    OMNI_PM_FORWARD(
        network_token_status_check_api,
        ::app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse
    )
    OMNI_PM_FORWARD(
        payment_method_get_token_details_api,
        ::app::model::payment_methods::PaymentMethodGetTokenDetailsResponse
    )
    OMNI_PM_FORWARD(
        list_payment_methods_for_payments_client,
        ::app::model::payments::PaymentsClientMethodsResponse
    )
    OMNI_PM_FORWARD(
        modular_migrate_payment_methods,
        ::app::model::payment_methods::ModularPaymentMethodMigrationResponse
    )

#undef OMNI_PM_FORWARD

private:
    ::app::context::AppContext& m_context;
    CreatePaymentMethodApiRuntime m_create_payment_method_api;
    GetPmNtEligibilityApiRuntime m_get_pm_nt_eligibility_api;
    CreatePaymentMethodIntentApiRuntime m_create_payment_method_intent_api;
    PaymentMethodUpdateApiRuntime m_payment_method_update_api;
    PaymentMethodRetrieveApiRuntime m_payment_method_retrieve_api;
    PaymentMethodDeleteApiRuntime m_payment_method_delete_api;
    MigratePaymentMethodApiRuntime m_migrate_payment_method_api;
    MigratePaymentMethodsRuntime m_migrate_payment_methods;
    UpdatePaymentMethodsRuntime m_update_payment_methods;
    PaymentMethodsBatchRetrieveApiRuntime m_payment_methods_batch_retrieve_api;
    SavePaymentMethodApiRuntime m_save_payment_method_api;
    ListPaymentMethodApiRuntime m_list_payment_method_api;
    ListCustomerPaymentMethodApiRuntime m_list_customer_payment_method_api;
    ListCustomerPaymentMethodApiClientRuntime m_list_customer_payment_method_api_client;
    InitiatePmCollectLinkFlowRuntime m_initiate_pm_collect_link_flow;
    PaymentMethodRetrieveOlapApiRuntime m_payment_method_retrieve_olap_api;
    GetPaymentMethodTokenDataRuntime m_get_payment_method_token_data;
    GetTotalPaymentMethodCountRuntime m_get_total_payment_method_count;
    RenderPmCollectLinkRuntime m_render_pm_collect_link;
    ListCountriesCurrenciesForConnectorPaymentMethodRuntime
        m_list_countries_currencies_for_connector_payment_method;
    DefaultPaymentMethodSetApiRuntime m_default_payment_method_set_api;
    TokenizeCardApiRuntime m_tokenize_card_api;
    TokenizeCardUsingPmApiRuntime m_tokenize_card_using_pm_api;
    TokenizeCardBatchApiRuntime m_tokenize_card_batch_api;
    PaymentMethodsSessionCreateRuntime m_payment_methods_session_create;
    PaymentMethodsSessionUpdateRuntime m_payment_methods_session_update;
    PaymentMethodsSessionRetrieveRuntime m_payment_methods_session_retrieve;
    PaymentMethodSessionListPaymentMethodsRuntime m_payment_method_session_list_payment_methods;
    PaymentMethodSessionConfirmRuntime m_payment_method_session_confirm;
    PaymentMethodSessionUpdateSavedPaymentMethodRuntime
        m_payment_method_session_update_saved_payment_method;
    PaymentMethodSessionDeleteSavedPaymentMethodRuntime
        m_payment_method_session_delete_saved_payment_method;
    NetworkTokenStatusCheckApiRuntime m_network_token_status_check_api;
    PaymentMethodGetTokenDetailsApiRuntime m_payment_method_get_token_details_api;
    ListPaymentMethodsForPaymentsClientRuntime m_list_payment_methods_for_payments_client;
    ModularMigratePaymentMethodsRuntime m_modular_migrate_payment_methods;
};

} // namespace app::handlers::payment_methods
