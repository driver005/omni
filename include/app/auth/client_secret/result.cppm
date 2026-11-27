export module app_auth:client_secret_result;

import std;

export namespace app::handlers::client_secret {

/**
 * @brief Exact wire fields of ClientSecretResponse, in declaration order.
 *
 * Both routes used the same DTO-derived Result; this list is captured from its retired
 * Serializable specialization so Serializer<T>::serialize(fields) emits the same bytes.
 */
[[nodiscard]] inline constexpr auto wire_fields()
{
    return std::array<std::string_view, 5>{"id", "resource_id", "created_at", "expires", "secret"};
}

} // namespace app::handlers::client_secret
