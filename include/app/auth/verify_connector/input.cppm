export module app_auth:verify_connector_input;

import std;
import interfaces;
import core_error;
import core_behavior;
import app_context;
import app_dto;
import app_context_core;
import app_http;

import :call_context;

export namespace app::handlers::verify_connector {

/**
 * @brief Error builders shared with the runtime so both agree on wire kind names.
 */
class VerifyConnectorInputSupport
{
    using Error = core::error::AppError;
    using Kind = core::error::AppErrorKind;

public:
    /** @brief InvalidRequestData -> 422 (api_error_response.rs:586-588). */
    [[nodiscard]] Error invalid_request_data_error(std::string message) const
    {
        auto error = core::error::AppErrorBuilder{}(Kind::VALIDATION_FAILED, std::move(message));
        error.set_kind_name("InvalidRequestData");
        return error;
    }

    /** @brief IncorrectConnectorNameGiven -> 404 (api_error_response.rs:532-534). */
    [[nodiscard]] Error incorrect_connector_name_given_error() const
    {
        auto error = core::error::AppErrorBuilder{}(
            Kind::NOT_FOUND,
            "The connector provided in the request is incorrect or not available"
        );
        error.set_kind_name("IncorrectConnectorNameGiven");
        return error;
    }
};

/**
 * @brief Domain policy: JWT registry rule and the merchant identity carried by
 * the authenticated request (or the merchant-id header fallback).
 */
class VerifyConnectorPolicy
{
public:
    explicit VerifyConnectorPolicy(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec& spec
    ) const
    {
        (void)operation;
        (void)spec;
        const auto rule = m_context.auth_rules().rule_for("payment_connector_verify");
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        core::behavior::Scope out{};
        const auto auth_ctx = ::app::auth::AuthContextBuilder{}(req);
        out.merchant_id = auth_ctx.merchant_id().empty()
                              ? ::app::http::merchant_id_from_header{}(req)
                              : std::string{auth_ctx.merchant_id()};
        return out;
    }

    [[nodiscard]] core::behavior::CallContext
    context(interfaces::io::IRequest& req) const
    {
        return detail::make_call_context(req, "payment_connector_verify");
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope& scope) const
    {
        (void)scope;
        return std::nullopt;
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::verify_connector
