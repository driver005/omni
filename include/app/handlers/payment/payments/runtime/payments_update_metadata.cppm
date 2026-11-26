export module app_payment:payments_runtime_payments_update_metadata;

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

class PaymentsUpdateMetadataRuntime
{
public:
    explicit PaymentsUpdateMetadataRuntime(
        ::app::context::AppContext& context,
        PaymentsRuntimeSupport& support
    ) noexcept :
        m_context{context},
        m_support{support}
    {
    }

    [[nodiscard]] std::expected<PaymentsUpdateMetadataResult, core::error::AppError>
    payments_update_metadata(const PaymentsUpdateMetadataInput& input) const
    {
        if (const auto err = m_support.ensure_payment_exists(input.data)) {
            return std::unexpected{*err};
        }
        return PaymentsUpdateMetadataResult{m_support.payment_response_view(input.data)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
