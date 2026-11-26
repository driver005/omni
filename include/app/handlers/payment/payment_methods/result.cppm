export module app_payment:payment_methods_result;

import std;
import core_serde;
import app_model;

export namespace app::handlers::payment_methods {

// Typed results inherit the wire fields of the exact DTO the old core built and
// encoded. Their Serializable specialization reuses the DTO's FieldDesc tuple,
// so field names, order and wrapper shapes stay byte-identical.

#define OMNI_PM_TYPED_RESULT(name, base)                                                           \
    class name : public base                                                                       \
    {                                                                                              \
    public:                                                                                        \
        name() = default;                                                                          \
        explicit name(base value) noexcept :                                                       \
            base{std::move(value)}                                                                 \
        {                                                                                          \
        }                                                                                          \
    };

OMNI_PM_TYPED_RESULT(
    CreatePaymentMethodApiResult,
    ::app::model::payment_methods::PaymentMethodResponse
)
OMNI_PM_TYPED_RESULT(
    GetPmNtEligibilityApiResult,
    ::app::model::payment_methods::GetNetworkTokenEiligibilityResponse
)
OMNI_PM_TYPED_RESULT(
    CreatePaymentMethodIntentApiResult,
    ::app::model::payment_methods::PaymentMethodResponse
)
OMNI_PM_TYPED_RESULT(
    PaymentMethodUpdateApiResult,
    ::app::model::payment_methods::PaymentMethodUpdateResponse
)
OMNI_PM_TYPED_RESULT(
    PaymentMethodRetrieveApiResult,
    ::app::model::payment_methods::PaymentMethodResponse
)
OMNI_PM_TYPED_RESULT(
    PaymentMethodDeleteApiResult,
    ::app::model::payment_methods::PaymentMethodDeleteResponse
)
OMNI_PM_TYPED_RESULT(
    MigratePaymentMethodApiResult,
    ::app::model::payment_methods::PaymentMethodMigrateResponse
)
OMNI_PM_TYPED_RESULT(
    MigratePaymentMethodsResult,
    ::app::model::payment_methods::ModularPaymentMethodMigrationResponse
)
OMNI_PM_TYPED_RESULT(
    UpdatePaymentMethodsResult,
    ::app::model::payment_methods::PaymentMethodRecordUpdateResponse
)
OMNI_PM_TYPED_RESULT(
    SavePaymentMethodApiResult,
    ::app::model::payment_methods::PaymentMethodResponse
)
OMNI_PM_TYPED_RESULT(
    ListPaymentMethodApiResult,
    ::app::model::payment_methods::PaymentMethodListResponse
)
OMNI_PM_TYPED_RESULT(
    ListCustomerPaymentMethodApiResult,
    ::app::model::payment_methods::CustomerPaymentMethodsListResponse
)
OMNI_PM_TYPED_RESULT(
    ListCustomerPaymentMethodApiClientResult,
    ::app::model::payment_methods::ClientPaymentMethodsListResponse
)
OMNI_PM_TYPED_RESULT(
    InitiatePmCollectLinkFlowResult,
    ::app::model::payment_methods::PaymentMethodCollectLinkResponse
)
OMNI_PM_TYPED_RESULT(
    PaymentMethodRetrieveOlapApiResult,
    ::app::model::payment_methods::PaymentMethodDetailsResponse
)
OMNI_PM_TYPED_RESULT(
    GetPaymentMethodTokenDataResult,
    ::app::model::payment_methods::TokenDataResponse
)
OMNI_PM_TYPED_RESULT(
    GetTotalPaymentMethodCountResult,
    ::app::model::payment_methods::TotalPaymentMethodCountResponse
)
OMNI_PM_TYPED_RESULT(
    RenderPmCollectLinkResult,
    ::app::model::payment_methods::PaymentMethodCollectLinkRenderRequest
)
OMNI_PM_TYPED_RESULT(
    ListCountriesCurrenciesForConnectorPaymentMethodResult,
    ::app::model::payment_methods::ListCountriesCurrenciesResponse
)
OMNI_PM_TYPED_RESULT(
    DefaultPaymentMethodSetApiResult,
    ::app::model::payment_methods::CustomerDefaultPaymentMethodResponse
)
OMNI_PM_TYPED_RESULT(
    TokenizeCardApiResult,
    ::app::model::payment_methods::CardNetworkTokenizeResponse
)
OMNI_PM_TYPED_RESULT(
    TokenizeCardUsingPmApiResult,
    ::app::model::payment_methods::TokenizedCardValue1
)
OMNI_PM_TYPED_RESULT(
    PaymentMethodsSessionCreateResult,
    ::app::model::payment_methods::PaymentMethodSessionResponse
)
OMNI_PM_TYPED_RESULT(
    PaymentMethodsSessionUpdateResult,
    ::app::model::payment_methods::PaymentMethodSessionResponse
)
OMNI_PM_TYPED_RESULT(
    PaymentMethodsSessionRetrieveResult,
    ::app::model::payment_methods::PaymentMethodSessionResponse
)
OMNI_PM_TYPED_RESULT(
    PaymentMethodSessionListPaymentMethodsResult,
    ::app::model::payment_methods::PaymentMethodListResponseForSession
)
OMNI_PM_TYPED_RESULT(
    PaymentMethodSessionConfirmResult,
    ::app::model::payment_methods::PaymentMethodSessionResponse
)
OMNI_PM_TYPED_RESULT(
    PaymentMethodSessionUpdateSavedPaymentMethodResult,
    ::app::model::payment_methods::PaymentMethodResponse
)
OMNI_PM_TYPED_RESULT(
    PaymentMethodSessionDeleteSavedPaymentMethodResult,
    ::app::model::payment_methods::PaymentMethodDeleteSessionResponse
)
OMNI_PM_TYPED_RESULT(
    NetworkTokenStatusCheckApiResult,
    ::app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse
)
OMNI_PM_TYPED_RESULT(
    PaymentMethodGetTokenDetailsApiResult,
    ::app::model::payment_methods::PaymentMethodGetTokenDetailsResponse
)
OMNI_PM_TYPED_RESULT(
    ListPaymentMethodsForPaymentsClientResult,
    ::app::model::payment_methods::PaymentMethodListResponseForSession
)
OMNI_PM_TYPED_RESULT(
    ModularMigratePaymentMethodsResult,
    ::app::model::payment_methods::ModularPaymentMethodMigrationResponse
)

// These two routes return a bare top-level JSON array. The result carries the
// typed vector; the handler serializes it with serde::Ser::serialize.
#define OMNI_PM_VECTOR_RESULT(name, item)                                                          \
    class name                                                                                     \
    {                                                                                              \
    public:                                                                                        \
        name() = default;                                                                          \
        explicit name(std::vector<item> items) noexcept :                                          \
            m_items{std::move(items)}                                                              \
        {                                                                                          \
        }                                                                                          \
        [[nodiscard]] const std::vector<item>& get_items() const noexcept                          \
        {                                                                                          \
            return m_items;                                                                        \
        }                                                                                          \
        void set_items(std::vector<item> value)                                                    \
        {                                                                                          \
            m_items = std::move(value);                                                            \
        }                                                                                          \
                                                                                                   \
    private:                                                                                       \
        std::vector<item> m_items{};                                                               \
    };

OMNI_PM_VECTOR_RESULT(
    PaymentMethodsBatchRetrieveApiResult,
    ::app::model::payment_methods::PaymentMethodsBatchRetrieveResponse
)
OMNI_PM_VECTOR_RESULT(
    TokenizeCardBatchApiResult,
    ::app::model::payment_methods::TokenizedCardValue1
)

#undef OMNI_PM_TYPED_RESULT
#undef OMNI_PM_VECTOR_RESULT

} // namespace app::handlers::payment_methods

#define OMNI_PM_TYPED_SERIALIZER(name, base, table)                                                \
    template<>                                                                                     \
    struct serde::Serializable<::app::handlers::payment_methods::name>                             \
    {                                                                                              \
        static constexpr auto fields()                                                             \
        {                                                                                          \
            return serde::Serializable<base>::fields();                                            \
        }                                                                                          \
        static constexpr std::string_view table_name()                                             \
        {                                                                                          \
            return table;                                                                          \
        }                                                                                          \
    };

OMNI_PM_TYPED_SERIALIZER(
    CreatePaymentMethodApiResult,
    ::app::model::payment_methods::PaymentMethodResponse,
    "create_payment_method_api_result"
)
OMNI_PM_TYPED_SERIALIZER(
    GetPmNtEligibilityApiResult,
    ::app::model::payment_methods::GetNetworkTokenEiligibilityResponse,
    "get_pm_nt_eligibility_api_result"
)
OMNI_PM_TYPED_SERIALIZER(
    CreatePaymentMethodIntentApiResult,
    ::app::model::payment_methods::PaymentMethodResponse,
    "create_payment_method_intent_api_result"
)
OMNI_PM_TYPED_SERIALIZER(
    PaymentMethodUpdateApiResult,
    ::app::model::payment_methods::PaymentMethodUpdateResponse,
    "payment_method_update_api_result"
)
OMNI_PM_TYPED_SERIALIZER(
    PaymentMethodRetrieveApiResult,
    ::app::model::payment_methods::PaymentMethodResponse,
    "payment_method_retrieve_api_result"
)
OMNI_PM_TYPED_SERIALIZER(
    PaymentMethodDeleteApiResult,
    ::app::model::payment_methods::PaymentMethodDeleteResponse,
    "payment_method_delete_api_result"
)
OMNI_PM_TYPED_SERIALIZER(
    MigratePaymentMethodApiResult,
    ::app::model::payment_methods::PaymentMethodMigrateResponse,
    "migrate_payment_method_api_result"
)
OMNI_PM_TYPED_SERIALIZER(
    MigratePaymentMethodsResult,
    ::app::model::payment_methods::ModularPaymentMethodMigrationResponse,
    "migrate_payment_methods_result"
)
OMNI_PM_TYPED_SERIALIZER(
    UpdatePaymentMethodsResult,
    ::app::model::payment_methods::PaymentMethodRecordUpdateResponse,
    "update_payment_methods_result"
)
OMNI_PM_TYPED_SERIALIZER(
    SavePaymentMethodApiResult,
    ::app::model::payment_methods::PaymentMethodResponse,
    "save_payment_method_api_result"
)
OMNI_PM_TYPED_SERIALIZER(
    ListPaymentMethodApiResult,
    ::app::model::payment_methods::PaymentMethodListResponse,
    "list_payment_method_api_result"
)
OMNI_PM_TYPED_SERIALIZER(
    ListCustomerPaymentMethodApiResult,
    ::app::model::payment_methods::CustomerPaymentMethodsListResponse,
    "list_customer_payment_method_api_result"
)
OMNI_PM_TYPED_SERIALIZER(
    ListCustomerPaymentMethodApiClientResult,
    ::app::model::payment_methods::ClientPaymentMethodsListResponse,
    "list_customer_payment_method_api_client_result"
)
OMNI_PM_TYPED_SERIALIZER(
    InitiatePmCollectLinkFlowResult,
    ::app::model::payment_methods::PaymentMethodCollectLinkResponse,
    "initiate_pm_collect_link_flow_result"
)
OMNI_PM_TYPED_SERIALIZER(
    PaymentMethodRetrieveOlapApiResult,
    ::app::model::payment_methods::PaymentMethodDetailsResponse,
    "payment_method_retrieve_olap_api_result"
)
OMNI_PM_TYPED_SERIALIZER(
    GetPaymentMethodTokenDataResult,
    ::app::model::payment_methods::TokenDataResponse,
    "get_payment_method_token_data_result"
)
OMNI_PM_TYPED_SERIALIZER(
    GetTotalPaymentMethodCountResult,
    ::app::model::payment_methods::TotalPaymentMethodCountResponse,
    "get_total_payment_method_count_result"
)
OMNI_PM_TYPED_SERIALIZER(
    RenderPmCollectLinkResult,
    ::app::model::payment_methods::PaymentMethodCollectLinkRenderRequest,
    "render_pm_collect_link_result"
)
OMNI_PM_TYPED_SERIALIZER(
    ListCountriesCurrenciesForConnectorPaymentMethodResult,
    ::app::model::payment_methods::ListCountriesCurrenciesResponse,
    "list_countries_currencies_for_connector_payment_method_result"
)
OMNI_PM_TYPED_SERIALIZER(
    DefaultPaymentMethodSetApiResult,
    ::app::model::payment_methods::CustomerDefaultPaymentMethodResponse,
    "default_payment_method_set_api_result"
)
OMNI_PM_TYPED_SERIALIZER(
    TokenizeCardApiResult,
    ::app::model::payment_methods::CardNetworkTokenizeResponse,
    "tokenize_card_api_result"
)
OMNI_PM_TYPED_SERIALIZER(
    TokenizeCardUsingPmApiResult,
    ::app::model::payment_methods::TokenizedCardValue1,
    "tokenize_card_using_pm_api_result"
)
OMNI_PM_TYPED_SERIALIZER(
    PaymentMethodsSessionCreateResult,
    ::app::model::payment_methods::PaymentMethodSessionResponse,
    "payment_methods_session_create_result"
)
OMNI_PM_TYPED_SERIALIZER(
    PaymentMethodsSessionUpdateResult,
    ::app::model::payment_methods::PaymentMethodSessionResponse,
    "payment_methods_session_update_result"
)
OMNI_PM_TYPED_SERIALIZER(
    PaymentMethodsSessionRetrieveResult,
    ::app::model::payment_methods::PaymentMethodSessionResponse,
    "payment_methods_session_retrieve_result"
)
OMNI_PM_TYPED_SERIALIZER(
    PaymentMethodSessionListPaymentMethodsResult,
    ::app::model::payment_methods::PaymentMethodListResponseForSession,
    "payment_method_session_list_payment_methods_result"
)
OMNI_PM_TYPED_SERIALIZER(
    PaymentMethodSessionConfirmResult,
    ::app::model::payment_methods::PaymentMethodSessionResponse,
    "payment_method_session_confirm_result"
)
OMNI_PM_TYPED_SERIALIZER(
    PaymentMethodSessionUpdateSavedPaymentMethodResult,
    ::app::model::payment_methods::PaymentMethodResponse,
    "payment_method_session_update_saved_payment_method_result"
)
OMNI_PM_TYPED_SERIALIZER(
    PaymentMethodSessionDeleteSavedPaymentMethodResult,
    ::app::model::payment_methods::PaymentMethodDeleteSessionResponse,
    "payment_method_session_delete_saved_payment_method_result"
)
OMNI_PM_TYPED_SERIALIZER(
    NetworkTokenStatusCheckApiResult,
    ::app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse,
    "network_token_status_check_api_result"
)
OMNI_PM_TYPED_SERIALIZER(
    PaymentMethodGetTokenDetailsApiResult,
    ::app::model::payment_methods::PaymentMethodGetTokenDetailsResponse,
    "payment_method_get_token_details_api_result"
)
OMNI_PM_TYPED_SERIALIZER(
    ListPaymentMethodsForPaymentsClientResult,
    ::app::model::payment_methods::PaymentMethodListResponseForSession,
    "list_payment_methods_for_payments_client_result"
)
OMNI_PM_TYPED_SERIALIZER(
    ModularMigratePaymentMethodsResult,
    ::app::model::payment_methods::ModularPaymentMethodMigrationResponse,
    "modular_migrate_payment_methods_result"
)

#undef OMNI_PM_TYPED_SERIALIZER
