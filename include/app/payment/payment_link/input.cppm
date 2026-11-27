export module app_payment:payment_link_input;

import std;
import core_behavior;
import interfaces;
import core_error;
import app_context;
import app_dto;
import app_model;
import app_auth;
import app_http;

import :context;
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
struct PaymentLinkRetrieveData
{
    std::optional<::app::model::payments::RetrievePaymentLinkRequest> request{};
    std::optional<std::string> client_secret{};
    std::optional<std::string> secret_claim{};
    std::string payment_link_id{};
    std::string merchant_from_credential{};
    explicit PaymentLinkRetrieveData(const core::behavior::CallContext& ctx) :
        request{},
        client_secret{std::optional<std::string>{ctx.client_secret}},
        secret_claim{std::nullopt},
        payment_link_id{::app::payment::detail::last_path(ctx)},
        merchant_from_credential{ctx.merchant_id}
    {
    }
};
struct PaymentLinkInitiateData
{
    std::string merchant_id{};
    std::string payment_id{};
    explicit PaymentLinkInitiateData(const core::behavior::CallContext& ctx) :
        merchant_id{ctx.merchant_id},
        payment_id{::app::payment::detail::last_path(ctx)}
    {
    }
};
struct PaymentLinkInitiateSecureData
{
    std::string merchant_id{};
    std::string payment_id{};
    SecureLinkHeaders headers{};
    explicit PaymentLinkInitiateSecureData(const core::behavior::CallContext& ctx) :
        merchant_id{ctx.merchant_id},
        payment_id{::app::payment::detail::last_path(ctx)},
        headers{}
    {
    }
};
struct PaymentLinkListData
{
    ::app::model::payments::PaymentLinkListConstraints request{};
    std::optional<std::string> query_limit{};
    std::optional<std::string> query_offset{};
    std::optional<std::string> query_time_range{};
    std::optional<std::string> query_profile_id{};
    std::string header_merchant_id{};
    std::string auth_merchant_id{};
    std::string auth_profile_id{};
    bool profile_scoped{};
    template<class Dto>
    explicit PaymentLinkListData(const Dto& dto, const core::behavior::CallContext& ctx) :
        request{::app::payment::detail::as_dto<::app::model::payments::PaymentLinkListConstraints>(dto)},
        query_limit{std::nullopt},
        query_offset{std::nullopt},
        query_time_range{std::nullopt},
        query_profile_id{std::nullopt},
        header_merchant_id{ctx.merchant_id},
        auth_merchant_id{ctx.merchant_id},
        auth_profile_id{ctx.profile_id},
        profile_scoped{}
    {
    }
};
struct PaymentLinkStatusData
{
    std::string merchant_id{};
    std::string payment_id{};
    explicit PaymentLinkStatusData(const core::behavior::CallContext& ctx) :
        merchant_id{ctx.merchant_id},
        payment_id{::app::payment::detail::last_path(ctx)}
    {
    }
};


/** @brief Request policy for the payment_link domain: wraps the domain auth rules. */
class PaymentLinkPolicy
{
public:
    PaymentLinkPolicy(
        ::app::context::AppContext& /*context*/,
        PaymentLinkInputSupport& support
    ) noexcept :
        m_support{support}
    {
    }

    [[nodiscard]] core::behavior::CallContext context(interfaces::io::IRequest& req) const
    {
        return ::app::payment::detail::make_call_context(req);
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec& spec
    ) const
    {
        (void)spec;
        if (operation == "initiate_payment_link" || operation == "initiate_secure_payment_link" ||
            operation == "payment_link_status") {
            const auto ids = m_support.path_merchant_payment(req);
            std::string merchant_id = ids.first;
            if (merchant_id.empty()) {
                merchant_id = m_support.merchant_id_from_header(req);
            }
            return m_support.authorize_operation(req, operation, merchant_id);
        }
        return m_support.authorize_operation(req, operation);
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        core::behavior::Scope result;
        result.merchant_id = std::string{auth.merchant_id()};
        result.profile_id = std::string{auth.profile_id()};
        result.org_id = std::string{auth.org_id()};
        const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId);
        result.connected_merchant_id = connected ? std::string{*connected} : std::string{};
        return result;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope&) const
    {
        return std::nullopt;
    }


private:
    PaymentLinkInputSupport& m_support;
};

using PaymentLinkRequestSupport = core::behavior::RequestSupport<PaymentLinkPolicy>;
using PaymentLinkPolicySpec = core::behavior::AuthSpec;

[[nodiscard]] PaymentLinkPolicySpec spec_for(std::string_view operation) noexcept
{
    return PaymentLinkPolicySpec{.permission = operation};
}
} // namespace app::handlers::payment_link
