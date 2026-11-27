export module app_auth:apple_pay_certificates_migration_result;

import std;

export namespace app::handlers::apple_pay_certificates_migration {

/**
 * @brief Exact wire fields of ApplePayCertificatesMigrationResponse, in declaration order.
 *
 * Captured from the retired ApplePayCertificatesMigrationResult Serializable field list so the
 * handler can emit the same document through Serializer<T>::serialize(fields).
 */
[[nodiscard]] inline constexpr auto wire_fields()
{
    return std::array<std::string_view, 2>{"migration_successful", "migration_failed"};
}

} // namespace app::handlers::apple_pay_certificates_migration
