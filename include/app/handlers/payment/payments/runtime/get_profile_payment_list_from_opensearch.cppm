export module app_payment:payments_runtime_get_profile_payment_list_from_opensearch;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_model;
import app_dto;
import app_store;
import app_connector;
import app_crypto;
import core_cache;
import app_http;

import :payments_input;
import :payments_result;
import :payments_runtime_common;

export namespace app::handlers::payments {

class GetProfilePaymentListFromOpensearchRuntime
{
public:
    explicit GetProfilePaymentListFromOpensearchRuntime(
        ::app::context::AppContext& context,
        PaymentsRuntimeSupport& support
    ) noexcept :
        m_context{context},
        m_support{support}
    {
    }

    [[nodiscard]] std::expected<GetProfilePaymentListFromOpensearchResult, core::error::AppError>
    get_profile_payment_list_from_opensearch(
        const GetProfilePaymentListFromOpensearchInput& input
    ) const
    {
        auto text = m_support.advanced_list_response(input.data, true);
        if (!text) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Invalid profile payment list constraints"
            )};
        }
        return GetProfilePaymentListFromOpensearchResult{*text};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
