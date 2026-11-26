export module app_webhook:recovery_webhooks_input;

import std;
import interfaces;
import core_error;
import app_context;
import app_auth;
import app_http;

export namespace app::handlers::recovery_webhooks {

/** Upstream `IncomingWebhookRequestDetails` (hyperswitch_interfaces/webhooks.rs). */
struct IncomingWebhookRequestDetails
{
    std::string method{};
    std::string uri{};
    std::string headers{};
    std::string query_params{};
    std::string body{};
};

/** URL-bound ids (MerchantIdAndProfileIdAuth) for the recovery webhook route. */
struct RecoveryWebhookIds
{
    std::string merchant_id{};
    std::string profile_id{};
    std::string connector_id{};
};

/** Plain input: everything the old handle_recovery_receive_incoming_webhook read off the request.
 */
struct RecoveryReceiveIncomingWebhookInput
{
    RecoveryWebhookIds ids{};
    std::string body{};
    IncomingWebhookRequestDetails request{};
    std::string org_id{};
    std::string lock_action{};
    bool network_verification{};
};

/**
 * @brief Parse/auth/path/query support for the recovery webhook handler.
 *
 * Free of business logic: only turns request data into the plain Input struct
 * consumed by the domain runtime.
 */
class RecoveryWebhooksInputSupport
{
public:
    explicit RecoveryWebhooksInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] std::size_t id_base(const ::app::http::PathSegments& segments) const
    {
        std::size_t index = segments.at(0).value_or("") == "v2" ? 1 : 0;
        while (index < segments.count()) {
            const auto value = segments.at(index).value_or("");
            if (value == "webhooks" || value == "recovery") {
                ++index;
                continue;
            }
            break;
        }
        return index;
    }

    [[nodiscard]] RecoveryWebhookIds recovery_ids(interfaces::io::IRequest& req) const
    {
        const auto path = ::app::http::RequestPath{}(req);
        const std::size_t base = id_base(path);
        RecoveryWebhookIds result;
        result.merchant_id = std::string{path.at(base).value_or("")};
        result.profile_id = std::string{path.at(base + 1).value_or("")};
        result.connector_id = std::string{path.at(base + 2).value_or("")};
        return result;
    }

    /** Mirrors upstream_error("InvalidRequestData", detail). */
    [[nodiscard]] core::error::AppError invalid_request_data(std::string detail) const
    {
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INVALID_REQUEST,
            std::move(detail)
        );
        error.set_kind_name("InvalidRequestData");
        return error;
    }

    /** MerchantIdAndProfileIdAuth (recovery_webhooks.rs:42-45): bind URL ids, then authorize. */
    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req, const RecoveryWebhookIds& ids) const
    {
        const auto rule = m_context.auth_rules().rule_for("recovery_receive_incoming_webhook");
        auto auth_context = ::app::auth::AuthContextBuilder{}(req);
        auth_context.set_merchant_id(ids.merchant_id);
        auth_context.set_profile_id(ids.profile_id);
        auth_context.add_claim("x-merchant-id", ids.merchant_id);
        auth_context.add_claim("x-profile-id", ids.profile_id);
        return m_context.authorizer().authorize_route(auth_context, rule);
    }

    /** Builds `IncomingWebhookRequestDetails` (incoming_v2.rs:128-134). */
    [[nodiscard]] IncomingWebhookRequestDetails
    build_request_details(interfaces::io::IRequest& req, const std::string& body) const
    {
        IncomingWebhookRequestDetails details{};
        details.method = std::string{req.get_method()};
        details.uri = std::string{req.get_path()};
        const std::string_view path = req.get_path();
        const std::size_t question = path.find('?');
        details.query_params = question == std::string_view::npos
                                   ? std::string{}
                                   : std::string{path.substr(question + 1)};
        details.body = body;
        // The local verifier consumes a small fixed set of webhook headers.
        const ::app::http::HeaderReader read{};
        std::string headers{};
        for (const auto name:
             {"x-webhook-signature",
              "x-webhook-secret",
              "x-hub-signature-256",
              "x-merchant-id",
              "x-profile-id",
              "content-type"}) {
            const std::string value = read.get(req, name);
            if (!value.empty()) {
                headers += std::string{name} + ":" + value + "\n";
            }
        }
        details.headers = std::move(headers);
        return details;
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::recovery_webhooks
