export module app_auth:external_service_auth_result;

import std;

export namespace app::handlers::external_service_auth {

/**
 * @brief Exact wire fields of ExternalVerifyTokenResponse, in declaration order.
 *
 * Captured from the retired ValidateTokenResult Serializable specialization so the handler can
 * emit the same document through Serializer<T>::serialize(fields).
 */
[[nodiscard]] inline constexpr auto validate_token_wire_fields()
{
    return std::array<std::string_view, 4>{"merchant_id", "context", "token", "permissions"};
}

} // namespace app::handlers::external_service_auth
