export module app_payment:payment_link_input;

import std;
import interfaces;
import core_error;
import app_context;
import app_dto;
import app_model;
import app_auth;
import app_http;

export namespace app::handlers::payment_link {

/** @brief Header values the secure-render validator needs (no HTTP types in runtime). */
struct SecureLinkHeaders
{
    std::string sec_fetch_dest{};
    std::string origin{};
    std::string referer{};
};

/**
 * @brief Parse/auth/path/query support for the payment_link handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime.
 */
class PaymentLinkInputSupport
{
public:
    explicit PaymentLinkInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    /** @brief Auth rules come from RouteAuthRegistry, one row per operation. */
    [[nodiscard]] std::optional<core::error::AppError>
    authorize_operation(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    /**
     * @brief MerchantIdAuth(merchant_id): bind the auth principal to the path
     * merchant id and reject a conflicting credential merchant id.
     */
    [[nodiscard]] std::optional<core::error::AppError> authorize_operation(
        interfaces::io::IRequest& req,
        std::string_view operation,
        std::string_view path_merchant_id
    ) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        auto context = ::app::auth::AuthContextBuilder{}(req);
        if (!context.merchant_id().empty() &&
            context.merchant_id() != std::string{path_merchant_id}) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "merchant id header does not match payment link path"
            );
        }
        context.set_merchant_id(std::string{path_merchant_id});
        return m_context.authorizer().authorize_route(context, rule);
    }

    /** @brief SDK client-secret assertion claim (owned copy, no dangling view). */
    [[nodiscard]] std::optional<std::string>
    client_secret_claim(interfaces::io::IRequest& req) const
    {
        const auto auth_context = ::app::auth::AuthContextBuilder{}(req);
        const auto claim = auth_context.claim("x-client-secret");
        if (!claim) {
            return std::nullopt;
        }
        return std::optional<std::string>{std::string{*claim}};
    }

    [[nodiscard]] std::string merchant_id_from_header(interfaces::io::IRequest& req) const
    {
        return ::app::http::merchant_id_from_header{}(req);
    }

    [[nodiscard]] std::string path_link_id(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        std::size_t index{0};
        while (index < segments.count() && segments.at(index).value_or("") != "payment_link") {
            ++index;
        }
        if (index < segments.count()) {
            ++index;
        }
        while (index < segments.count()) {
            const std::string token{segments.at(index).value_or("")};
            if (token.empty() || token == "list" || token == "profile" || token == "s" ||
                token == "status") {
                ++index;
                continue;
            }
            return token;
        }
        if (segments.count() > 0) {
            return std::string{segments.at(segments.count() - 1).value_or("")};
        }
        return {};
    }

    [[nodiscard]] std::pair<std::string, std::string>
    path_merchant_payment(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        std::size_t index{0};
        while (index < segments.count() && segments.at(index).value_or("") != "payment_link") {
            ++index;
        }
        if (index < segments.count()) {
            ++index;
        }
        while (index < segments.count() && (segments.at(index).value_or("") == "s" ||
                                            segments.at(index).value_or("") == "status")) {
            ++index;
        }
        if (index + 1 < segments.count()) {
            const std::string merchant{segments.at(index).value_or("")};
            const std::string payment{segments.at(index + 1).value_or("")};
            if (!merchant.empty() && !payment.empty()) {
                return std::pair<std::string, std::string>{merchant, payment};
            }
        }
        if (segments.count() >= 2) {
            return std::pair<std::string, std::string>{
                std::string{segments.at(segments.count() - 2).value_or("")},
                std::string{segments.at(segments.count() - 1).value_or("")}
            };
        }
        return {};
    }

    [[nodiscard]] std::optional<std::string>
    query_value(interfaces::io::IRequest& req, std::string_view key) const
    {
        const auto query = ::app::http::RequestQuery{}(req);
        return query.get(key);
    }

    [[nodiscard]] SecureLinkHeaders secure_headers(interfaces::io::IRequest& req) const
    {
        const ::app::http::HeaderReader read{};
        return SecureLinkHeaders{
            read.get(req, "sec-fetch-dest"),
            read.get(req, "origin"),
            read.get(req, "referer")
        };
    }

    [[nodiscard]] std::pair<std::string, std::string>
    auth_merchant_and_profile(interfaces::io::IRequest& req) const
    {
        const auto context = ::app::auth::AuthContextBuilder{}(req);
        return std::make_pair(
            std::string{context.merchant_id()},
            std::string{context.profile_id()}
        );
    }

private:
    ::app::context::AppContext& m_context;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old core call received (plus the
// request-derived query/header/auth values the old handler read directly).

struct PaymentLinkRetrieveInput
{
    std::optional<::app::model::payments::RetrievePaymentLinkRequest> request{};
    std::optional<std::string> client_secret{};
    std::optional<std::string> secret_claim{};
    std::string payment_link_id{};
    std::string merchant_from_credential{};
};

struct PaymentLinkInitiateInput
{
    std::string merchant_id{};
    std::string payment_id{};
};

struct PaymentLinkInitiateSecureInput
{
    std::string merchant_id{};
    std::string payment_id{};
    SecureLinkHeaders headers{};
};

struct PaymentLinkListInput
{
    ::app::model::payments::PaymentLinkListConstraints request{};
    std::optional<std::string> query_limit{};
    std::optional<std::string> query_offset{};
    std::optional<std::string> query_time_range{};
    std::optional<std::string> query_profile_id{};
    std::string header_merchant_id{};
    std::string auth_merchant_id{};
    std::string auth_profile_id{};
    bool profile_scoped{false};
};

struct PaymentLinkStatusInput
{
    std::string merchant_id{};
    std::string payment_id{};
};

} // namespace app::handlers::payment_link
