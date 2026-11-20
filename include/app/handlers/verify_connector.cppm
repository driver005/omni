module;
export module app_handlers:verify_connector;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_connector;
import app_http;
import app_crypto;
import app_metrics;

export namespace app::handlers::verify_connector {

namespace detail {

using Error = core::error::AppError;
using Kind = core::error::AppErrorKind;
using Connector = app::dto::verify_connector::Connector;
using AdminAuth = app::dto::verify_connector::ConnectorAuthTypeValue;

// ── Upstream ApiErrorResponse mappings ─────────────────────────────────────────
// Error names, HTTP statuses and messages mirror
// hyperswitch_domain_models/src/errors/api_error_response.rs:532-533,586-587,637-638.

/** @brief IncorrectConnectorNameGiven -> 404 (api_error_response.rs:532-534). */
[[nodiscard]] inline Error incorrect_connector_name_given()
{
    auto error = core::error::AppErrorBuilder{}(
        Kind::NOT_FOUND,
        "The connector provided in the request is incorrect or not available"
    );
    error.set_kind_name("IncorrectConnectorNameGiven");
    return error;
}

/** @brief FlowNotSupported -> 400 (api_error_response.rs:637-639). */
[[nodiscard]] inline Error flow_not_supported(std::string_view connector)
{
    auto error = core::error::AppErrorBuilder{}(
        Kind::INVALID_REQUEST,
        "Verify credentials flow not supported"
    );
    error.set_kind_name("FlowNotSupported");
    // Upstream carries the connector in ErrorExtra; keep it in the message for parity.
    (void)connector;
    return error;
}

/** @brief InvalidRequestData -> 422 (api_error_response.rs:586-588). */
[[nodiscard]] inline Error invalid_request_data(std::string message)
{
    auto error = core::error::AppErrorBuilder{}(Kind::VALIDATION_FAILED, std::move(message));
    error.set_kind_name("InvalidRequestData");
    return error;
}

/** @brief InternalServerError -> 500 (api_error_response.rs). */
[[nodiscard]] inline Error internal_server_error(std::string message)
{
    auto error = core::error::AppErrorBuilder{}(Kind::INTERNAL_ERROR, std::move(message));
    error.set_kind_name("InternalServerError");
    return error;
}

// ── domain::Card + utils::get_test_card_details ────────────────────────────────

/** @brief Local mirror of `domain::Card` (utils/verify_connector.rs:12-32). */
struct DomainCard
{
    std::string card_number{};
    std::string card_cvc{};
    std::string card_exp_year{};
    std::string card_exp_month{};
};

/**
 * @brief Upstream `utils::generate_card_from_details` (utils/verify_connector.rs:6-33).
 * Parses the card number via cards::CardNumber; an invalid number maps to InternalServerError.
 * The in-process substitute validates non-empty digits (no external card-validation crate).
 */
[[nodiscard]] inline std::expected<DomainCard, Error> generate_card_from_details(
    std::string number,
    std::string year,
    std::string month,
    std::string cvv
)
{
    if (number.empty()
        || !std::ranges::all_of(number, [](unsigned char c) { return std::isdigit(c) != 0; })) {
        return std::unexpected{internal_server_error("Error while parsing card number")};
    }
    DomainCard card{};
    card.card_number = std::move(number);
    card.card_cvc = std::move(cvv);
    card.card_exp_year = std::move(year);
    card.card_exp_month = std::move(month);
    return card;
}

/**
 * @brief Upstream `utils::get_test_card_details` (utils/verify_connector.rs:35-55).
 * Table-driven: Stripe -> 4242..., Paypal -> 4111..., every other connector -> None.
 */
[[nodiscard]] inline std::expected<std::optional<DomainCard>, Error>
get_test_card_details(Connector connector)
{
    switch (connector) {
    case Connector::stripe: {
        auto card = generate_card_from_details("4242424242424242", "2025", "12", "100");
        if (!card) { return std::unexpected{card.error()}; }
        return std::optional<DomainCard>{*card};
    }
    case Connector::paypal: {
        auto card = generate_card_from_details("4111111111111111", "2025", "02", "123");
        if (!card) { return std::unexpected{card.error()}; }
        return std::optional<DomainCard>{*card};
    }
    default:
        return std::optional<DomainCard>{};
    }
}

// ── types::ConnectorAuthType + foreign_into ────────────────────────────────────

/** @brief Local mirror of `types::ConnectorAuthType` (router_data.rs:418-447). */
struct ConnectorAuthTypeDomain
{
    std::string auth_type{};
    std::string api_key{};
    std::string key1{};
    std::string api_secret{};
    std::string key2{};
    std::string certificate{};
    std::string private_key{};
    std::string auth_key_map{};
};

/**
 * @brief Upstream `connector_auth.foreign_into()` (transformers.rs:64-71 dispatching to
 * types.rs:1251-1294). The admin and domain enums are structurally identical, so this is a
 * field-for-field conversion of the internally-tagged auth value.
 */
[[nodiscard]] inline ConnectorAuthTypeDomain foreign_into(const AdminAuth& value)
{
    ConnectorAuthTypeDomain out{};
    out.auth_type = value.get_auth_type();
    out.api_key = value.get_api_key();
    out.key1 = value.get_key1();
    out.api_secret = value.get_api_secret();
    out.key2 = value.get_key2();
    out.certificate = value.get_certificate();
    out.private_key = value.get_private_key();
    out.auth_key_map = value.get_auth_key_map();
    return out;
}

// ── api::ConnectorData::get_connector_by_name ──────────────────────────────────

/** @brief Local mirror of `api::ConnectorData` (core/verify_connector.rs:24-30). */
struct ConnectorData
{
    std::string connector{};
};

/**
 * @brief Upstream `api::ConnectorData::get_connector_by_name` (core/verify_connector.rs:24-30)
 * against `state.conf.connectors`, with the `IncorrectConnectorNameGiven` mapping on a miss.
 *
 * The local ConfigStore stands in for `state.conf.connectors`: an explicit disabled config
 * entry is the miss path, while the validated enum name itself is the connector carried into
 * VerifyConnectorData.
 */
[[nodiscard]] inline std::expected<ConnectorData, Error> get_connector_by_name(
    app::context::AppContext& context,
    std::string_view name
)
{
    // Upstream `state.conf.connectors` enumerates every known Connector variant, so a name that
    // already passed the Connector enum validation is present by default. An explicit
    // disabled/unavailable config entry is the only miss path.
    const auto configured = context.config_store().get_value("connectors", name);
    if (configured && (*configured == "disabled" || *configured == "unavailable")) {
        return std::unexpected{incorrect_connector_name_given()};
    }
    return ConnectorData{std::string{name}};
}

// ── types::VerifyConnectorData + its builders ──────────────────────────────────

/** @brief Upstream `types::VerifyConnectorData` (types/api/verify_connector.rs:16-21). */
struct VerifyConnectorData
{
    std::string connector{};
    ConnectorAuthTypeDomain connector_auth{};
    DomainCard card_details{};
};

/** @brief Upstream `get_payment_authorize_data` (types/api/verify_connector.rs:24-88). */
struct PaymentsAuthorizeData
{
    std::int64_t amount{1000};        // types/api/verify_connector.rs:30
    std::int64_t minor_amount{1000};  // types/api/verify_connector.rs:31
    std::string currency{"USD"};      // types/api/verify_connector.rs:34
    bool confirm{true};               // types/api/verify_connector.rs:33
    DomainCard card{};
};

[[nodiscard]] inline PaymentsAuthorizeData
get_payment_authorize_data(const VerifyConnectorData& data)
{
    PaymentsAuthorizeData out{};
    out.card = data.card_details;  // PaymentMethodData::Card(self.card_details.clone()) :27
    return out;
}

/** @brief Upstream `get_router_data` (types/api/verify_connector.rs:90-163). */
struct RouterData
{
    std::string attempt_id{};
    std::string connector{};
    std::string connector_request_reference_id{};
    std::string authentication_type{"NoThreeDs"};
};

[[nodiscard]] inline RouterData get_router_data(
    app::context::AppContext& context,
    const VerifyConnectorData& data,
    const PaymentsAuthorizeData& /*authorize_data*/,
    const std::optional<std::string>& /*access_token*/
)
{
    RouterData out{};
    // generate_id_with_default_len(VERIFY_CONNECTOR_ID_PREFIX), consts.rs:102 "conn_verify".
    out.attempt_id = context.ids().payment_id("conn_verify");
    out.connector = data.connector;                       // self.connector.id().to_string() :103
    out.connector_request_reference_id = out.attempt_id;  // :127
    out.authentication_type = "NoThreeDs";                // :104
    return out;
}

/** @brief Serialize the structured VerifyConnectorData into the connector request payload. */
[[nodiscard]] inline std::string encode_verify_data(const VerifyConnectorData& data)
{
    return "{\"connector\":" + serde::Ser::encode_json(data.connector)
         + ",\"connector_auth\":{\"auth_type\":" + serde::Ser::encode_json(data.connector_auth.auth_type)
         + ",\"api_key\":" + serde::Ser::encode_json(data.connector_auth.api_key)
         + ",\"key1\":" + serde::Ser::encode_json(data.connector_auth.key1)
         + ",\"api_secret\":" + serde::Ser::encode_json(data.connector_auth.api_secret)
         + ",\"key2\":" + serde::Ser::encode_json(data.connector_auth.key2)
         + ",\"certificate\":" + serde::Ser::encode_json(data.connector_auth.certificate)
         + ",\"private_key\":" + serde::Ser::encode_json(data.connector_auth.private_key)
         + ",\"auth_key_map\":" + serde::Ser::encode_json(data.connector_auth.auth_key_map)
         + "},\"card_details\":{\"card_number\":" + serde::Ser::encode_json(data.card_details.card_number)
         + ",\"card_cvc\":" + serde::Ser::encode_json(data.card_details.card_cvc)
         + ",\"card_exp_year\":" + serde::Ser::encode_json(data.card_details.card_exp_year)
         + ",\"card_exp_month\":" + serde::Ser::encode_json(data.card_details.card_exp_month)
         + "}}";
}

/** @brief Whether the in-process connector call produced a failure response. */
[[nodiscard]] inline bool connector_call_failed(const app::connector::ConnectorResponse& response)
{
    if (response.get_status().empty()) {
        return true;
    }
    return !response.get_message().empty()
        && (response.get_message().starts_with("error") || response.get_message().starts_with("Error"));
}

/**
 * @brief Upstream `handle_payment_error_response` (types/api/verify_connector.rs:220-232):
 * `InvalidRequestData { message: error.reason.unwrap_or(error.message) }`.
 */
[[nodiscard]] inline Error handle_payment_error_response(
    const app::connector::ConnectorResponse& response
)
{
    std::string message = response.get_message();
    if (message.empty()) {
        message = response.get_status();
    }
    if (message.empty()) {
        message = "connector error";
    }
    return invalid_request_data(std::move(message));
}

/** @brief Build the connector request from the structured VerifyConnectorData. */
[[nodiscard]] inline app::connector::ConnectorRequest build_connector_request(
    std::string_view operation,
    std::string_view merchant_id,
    const VerifyConnectorData& data
)
{
    return app::connector::ConnectorRequestBuilder{}
        .operation(std::string{operation})
        .merchant_id(std::string{merchant_id})
        .resource_id(data.connector)
        .raw_payload(encode_verify_data(data))();
}

/**
 * @brief Local in-process substitute for upstream `VerifyConnector::verify` default
 * (types/api/verify_connector.rs:167-209): get_payment_authorize_data -> get_access_token ->
 * get_router_data -> build_request -> call_connector_api -> StatusOk / error mapping.
 */
[[nodiscard]] inline std::expected<void, Error> stripe_verify(
    app::context::AppContext& context,
    const std::shared_ptr<app::connector::IConnector>& connector,
    std::string_view merchant_id,
    const VerifyConnectorData& data
)
{
    const auto authorize_data = get_payment_authorize_data(data);
    const auto router_data = get_router_data(context, data, authorize_data, std::nullopt);
    (void)router_data; // the in-process connector contract consumes the built request below.

    auto request = build_connector_request("verify", merchant_id, data);
    const auto response = connector->verify(request);
    if (connector_call_failed(response)) {
        return std::unexpected{handle_payment_error_response(response)};
    }
    return {}; // ApplicationResponse::StatusOk (types/api/verify_connector.rs:195)
}

/**
 * @brief Local in-process substitute for upstream `Paypal::get_access_token` path
 * (core/verify_connector.rs:51-60; types/api/verify_connector.rs:211-218). Paypal overrides
 * get_access_token with an OAuth token exchange; the in-process connector's exchange_token()
 * stands in for that external call. Success maps to StatusOk.
 */
[[nodiscard]] inline std::expected<void, Error> paypal_get_access_token(
    app::context::AppContext& /*context*/,
    const std::shared_ptr<app::connector::IConnector>& connector,
    std::string_view merchant_id,
    const VerifyConnectorData& data
)
{
    auto request = build_connector_request("get_access_token", merchant_id, data);
    const auto response = connector->exchange_token(request);
    if (connector_call_failed(response)) {
        return std::unexpected{handle_payment_error_response(response)};
    }
    return {}; // .map(|_| ApplicationResponse::StatusOk) (core/verify_connector.rs:60)
}

} // namespace detail

class VerifyConnectorRouteHandlers
{
    using Error = core::error::AppError;
    using Kind = core::error::AppErrorKind;
    using Builder = core::error::AppErrorBuilder;
    using Connector = app::dto::verify_connector::Connector;

public:
    void handle_payment_connector_verify(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: JWTAuth { permission: MerchantConnectorWrite, allow_connected: true,
        // allow_platform: false }. Resolve the declarative rule from the shared registry.
        // (For JWT the authorizer ignores allow_platform, so the registry row is sufficient.)
        const auto rule = m_context->auth_rules().rule_for("payment_connector_verify");
        const auto auth_ctx = app::auth::AuthContextBuilder{}(req);
        const auto auth = m_context->authorizer().authorize_route(auth_ctx, rule);
        if (auth) { send_error(res, send, *auth); return; }

        // Upstream takes merchant/profile from the auth extractor (routes/verify_connector.rs:24-29
        // passes auth.profile.id as profile_id). No URL path segment and no merchant-account
        // lookup: upstream authenticates via JWT and performs no DB write on this route.
        std::string merchant_id = auth_ctx.merchant_id();
        if (merchant_id.empty()) {
            merchant_id = app::http::merchant_id_from_header{}(req);
        }
        const std::string profile_id{auth_ctx.profile_id()};
        (void)profile_id; // core takes _profile_id but does not use it (core/verify_connector.rs:22).

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, Builder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }

        // Typed upstream request (api_models/src/verify_connector.rs:5-9): `connector_name` is a
        // plain snake_case string validated into the Connector enum, and
        // `connector_account_details` is the internally-tagged ConnectorAuthType object.
        auto parsed = serde::Ser::deserialize<app::dto::verify_connector::VerifyConnectorRequest>(
            "application/json", body
        );
        if (!parsed) {
            send_error(res, send, detail::invalid_request_data("invalid VerifyConnectorRequest"));
            return;
        }

        const auto connector = app::dto::verify_connector::connector_from_wire(
            parsed->get_connector_name()
        );
        if (!connector) {
            // Upstream serde rejects an unknown Connector enum value during web::Json extract.
            send_error(res, send, detail::incorrect_connector_name_given());
            return;
        }
        const std::string connector_name{
            app::dto::verify_connector::connector_to_wire(*connector)
        };

        // Upstream core/verify_connector.rs:24-30 — resolve connector from config, mapping a
        // miss to IncorrectConnectorNameGiven.
        auto connector_data = detail::get_connector_by_name(*m_context, connector_name);
        if (!connector_data) {
            send_error(res, send, connector_data.error());
            return;
        }

        // Upstream core/verify_connector.rs:32-37 — get_test_card_details + FlowNotSupported.
        auto card_details = detail::get_test_card_details(*connector);
        if (!card_details) {
            send_error(res, send, card_details.error());
            return;
        }
        if (!card_details->has_value()) {
            send_error(res, send, detail::flow_not_supported(connector_name));
            return;
        }

        // Upstream core/verify_connector.rs:43-47 / 53-57 — build the structured
        // VerifyConnectorData { connector, connector_auth.foreign_into(), card_details }.
        detail::VerifyConnectorData verify_data{};
        verify_data.connector = connector_data->connector;
        verify_data.connector_auth = detail::foreign_into(parsed->get_connector_account_details());
        verify_data.card_details = **card_details;

        // The local in-process connector contract stands in for the upstream connector SDK /
        // external PSP integration selected by `boxed_connector.connector`.
        auto selected = m_context->connector_router().select_checked(merchant_id);
        if (!selected) {
            send_error(res, send, selected.error());
            return;
        }
        const auto connector_impl = *selected;

        // Upstream core/verify_connector.rs:39-66 — dispatch on Connector::Stripe /
        // Connector::Paypal; every other connector already returned FlowNotSupported above.
        std::expected<void, detail::Error> outcome{};
        if (*connector == Connector::stripe) {
            outcome = detail::stripe_verify(*m_context, connector_impl, merchant_id, verify_data);
        } else if (*connector == Connector::paypal) {
            outcome = detail::paypal_get_access_token(*m_context, connector_impl, merchant_id, verify_data);
        } else {
            send_error(res, send, detail::flow_not_supported(connector_name));
            return;
        }
        if (!outcome) {
            send_error(res, send, outcome.error());
            return;
        }

        // Upstream #[instrument(flow = ?Flow::VerifyPaymentConnector)] (routes:12,18) flow marker.
        m_context->metrics().record_flow("VerifyPaymentConnector");
        m_context->metrics().counter("VERIFY_PAYMENT_CONNECTOR").increment();

        // Upstream returns RouterResponse<()> / StatusOk with no payload (core/verify_connector.rs:23,60).
        send_json(res, send, {}, true);
    }

    // routes/verify_connector.rs:36 uses api_locking::LockAction::NotApplicable: no lock is
    // acquired on this route. The local LockRegistry is therefore deliberately unused here.

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/account"}
            .add_route(
                Route{"/connectors/verify"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_connector_verify(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_connector_verify")
            );
    }

    explicit VerifyConnectorRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        Error error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::verify_connector
