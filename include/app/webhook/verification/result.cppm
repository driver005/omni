export module app_webhook:verification_result;

import std;

export namespace app::handlers::verification {

// Wire field lists captured from the retired Result serializers, in declaration
// order. Serializer<T>::serialize(fields) emits the same upstream bytes.
[[nodiscard]] inline constexpr auto apple_pay_merchant_registration_wire_fields()
{
    return std::array<std::string_view, 1>{"status_message"};
}

[[nodiscard]] inline constexpr auto retrieve_apple_pay_verified_domains_wire_fields()
{
    return std::array<std::string_view, 1>{"verified_domains"};
}

} // namespace app::handlers::verification
