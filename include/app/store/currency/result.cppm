export module app_store:currency_result;

import std;

export namespace app::handlers::currency {

// CurrencyConversionResponse owns these fields; the list is kept on the route
// so the handler can filter the serializer to the exact retired ConvertForexResult
// wire order.
[[nodiscard]] inline constexpr auto convert_forex_wire_fields()
{
    return std::array<std::string_view, 2>{"converted_amount", "currency"};
}

} // namespace app::handlers::currency
