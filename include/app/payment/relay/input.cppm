export module app_payment:relay_input;

import std;
import interfaces;
import core_error;
import app_context;
import app_dto;
import app_auth;
import app_http;

export namespace app::handlers::relay {

/** @brief Identity scope carried by the authenticated headers on relay routes. */
struct RelayScope
{
    std::string merchant_id{};
    std::string org_id{};
    std::string profile_id{};
};

/** @brief Path identity for POST /webhooks/relay/{merchant_id}/{connector_id}. */
struct WebhookIds
{
    std::string merchant_id{};
    std::string connector_id{};
};

/**
 * @brief Parse/auth/path/query support for the relay handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime.
 */
class RelayInputSupport
{
public:
    explicit RelayInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // Upstream routes relay.rs:14-47 / :51-81 / :85-119 all use ApiKeyAuth with a
    // rule registered under the matching operation name. The rule registry is
    // authoritative, exactly as in the original handler.
    [[nodiscard]] std::optional<core::error::AppError>
    authorize_route(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    /** @brief Extracts {merchant_id}/{connector_id}, skipping literal prefixes. */
    [[nodiscard]] WebhookIds webhook_path_ids(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        std::size_t base = segments.at(0).value_or("") == "v2" ? 1U : 0U;
        while (base < segments.count()) {
            const auto value = segments.at(base).value_or("");
            if (value == "webhooks" || value == "relay" || value == "recovery") {
                ++base;
                continue;
            }
            break;
        }
        WebhookIds ids{};
        ids.merchant_id = std::string{segments.at(base).value_or("")};
        ids.connector_id = std::string{segments.at(base + 1).value_or("")};
        return ids;
    }

    // MerchantIdAuth(merchant_id): the path id is authoritative, so it is injected
    // into the auth context before the registered rule is evaluated.
    [[nodiscard]] std::optional<core::error::AppError>
    authorize_webhook(interfaces::io::IRequest& req, const WebhookIds& ids) const
    {
        const auto rule = m_context.auth_rules().rule_for("receive_incoming_relay_webhook");
        auto auth = ::app::auth::AuthContextBuilder{}(req);
        auth.set_merchant_id(ids.merchant_id);
        auth.add_claim("x-merchant-id", ids.merchant_id);
        return m_context.authorizer().authorize_route(auth, rule);
    }

    [[nodiscard]] std::string webhook_signature(interfaces::io::IRequest& req) const
    {
        return std::string{::app::http::HeaderReader{}.get(req, "x-webhook-signature")};
    }

    /** @brief Extract the `{relay_id}` segment from "GET /relay/{relay_id}". */
    [[nodiscard]] std::optional<std::string> relay_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        for (std::size_t i{0}; i < segments.count(); ++i) {
            if (segments.at(i).value_or(std::string_view{}) != "relay") {
                continue;
            }
            for (std::size_t j{i + 1}; j < segments.count(); ++j) {
                const auto candidate = segments.at(j).value_or(std::string_view{});
                if (!candidate.empty()) {
                    return std::string{candidate};
                }
            }
        }
        if (segments.count() > 0) {
            return std::string{segments.at(segments.count() - 1).value_or(std::string_view{})};
        }
        return std::nullopt;
    }

    [[nodiscard]] bool force_sync_from_query(interfaces::io::IRequest& req) const
    {
        const auto value = ::app::http::RequestQuery{}(req).get("force_sync");
        return value.has_value() && (*value == "true" || *value == "1");
    }

    [[nodiscard]] RelayScope resolve_scope(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        const auto header_merchant_id = ::app::http::merchant_id_from_header{}(req);
        const auto header_org_id = ::app::http::org_id_from_header{}(req);
        RelayScope scope{};
        scope.merchant_id = header_merchant_id.empty() ? auth.merchant_id() : header_merchant_id;
        scope.org_id = header_org_id.empty() ? auth.org_id() : header_org_id;
        scope.profile_id = auth.profile_id();
        return scope;
    }

private:
    ::app::context::AppContext& m_context;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old handler handed to its *_core block.

struct RelayUnreferencedRefundInput
{
    ::app::dto::unreferenced_refund::UnreferencedRefundRequest request{};
    std::string body{};
    RelayScope scope{};
};

struct RelayCreateInput
{
    std::string body{};
    RelayScope scope{};
};

struct RelayRetrieveInput
{
    std::string relay_id{};
    bool force_sync{false};
    RelayScope scope{};
};

struct RelayIncomingWebhookInput
{
    std::string merchant_id{};
    std::string connector_id{};
    std::string body{};
    std::string signature{};
};

} // namespace app::handlers::relay
