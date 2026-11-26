export module app_payment:revenue_recovery_redis_input;

import std;
import interfaces;
import core_error;
import app_context;
import app_auth;
import app_http;

export namespace app::handlers::revenue_recovery_redis {

/**
 * @brief Parse/auth/path/query support for the revenue recovery Redis handler.
 *
 * Turns request data into the plain Input consumed by the domain runtime. No
 * business logic and no HTTP response writes live here.
 */
class RevenueRecoveryRedisInputSupport
{
public:
    explicit RevenueRecoveryRedisInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // Upstream route uses V2AdminApiAuth (revenue_recovery_redis.rs:30). The
    // declarative rule in RouteAuthRegistry is PLATFORM_ADMIN / ADMIN; use it
    // instead of building a rule inline so the permission stays in one place.
    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req) const
    {
        const auto rule = m_context.auth_rules().rule_for("get_revenue_recovery_redis_data");
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    // Upstream route: /:connector_customer_id with web::Path<String>. The id is
    // the final non-empty path segment, read via RequestPath rather than a manual
    // rfind on get_path (which also failed to strip a trailing query string).
    [[nodiscard]] std::expected<std::string, core::error::AppError>
    connector_customer_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        const auto id_index = segments.count() == 0 ? std::size_t{0} : segments.count() - 1;
        const auto connector_customer_id = std::string{segments.at(id_index).value_or("")};
        if (connector_customer_id.empty()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing connector_customer_id"
            )};
        }
        return connector_customer_id;
    }

    // Upstream input: web::Query<GetRedisDataQuery>.key_type with enum
    // RedisKeyType { Status, Tokens } (api_models:204-213). serde rejects a
    // missing or unknown variant, so do the same instead of silently defaulting
    // to Status. Returns true for Tokens, false for Status.
    [[nodiscard]] std::expected<bool, core::error::AppError>
    key_type_from_query(interfaces::io::IRequest& req) const
    {
        const auto key_type_raw = ::app::http::RequestQuery{}(req).get("key_type");
        if (!key_type_raw) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing key_type; expected Status or Tokens"
            )};
        }
        if (*key_type_raw == "Status") {
            return false;
        }
        if (*key_type_raw == "Tokens") {
            return true;
        }
        return std::unexpected{core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INVALID_REQUEST,
            "invalid key_type; expected Status or Tokens"
        )};
    }

private:
    ::app::context::AppContext& m_context;
};

// Everything the old core block received after auth/path/query parsing.
struct GetRevenueRecoveryRedisDataInput
{
    std::string connector_customer_id{};
    bool is_tokens{false};
};

} // namespace app::handlers::revenue_recovery_redis
