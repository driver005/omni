export module app_payment:telephone_order_runtime;

import std;
import core_error;
import app_context;

import :telephone_order_input;
import :telephone_order_result;
import :telephone_order_runtime_common;
import :telephone_order_runtime_telephone_order_charge;

export namespace app::handlers::telephone_order {

class TelephoneOrderRuntime
{
public:
    explicit TelephoneOrderRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_telephone_order_charge{context}
    {
    }

    [[nodiscard]] std::expected<TwilioGenericPayResponse, core::error::AppError>
    telephone_order_charge(const TelephoneOrderChargeInput& input) const
    {
        return m_telephone_order_charge.telephone_order_charge(input);
    }

private:
    [[maybe_unused]] ::app::context::AppContext& m_context;
    TelephoneOrderChargeRuntime m_telephone_order_charge;
};

} // namespace app::handlers::telephone_order
