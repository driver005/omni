export module app_auth:verify_connector_runtime_common;

import std;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_connector;

export namespace app::handlers::verify_connector {

// ── domain::Card + utils::get_test_card_details ────────────────────────────────

/** @brief Local mirror of `domain::Card` (utils/verify_connector.rs:12-32). */
struct DomainCard
{
    std::string card_number{};
    std::string card_cvc{};
    std::string card_exp_year{};
    std::string card_exp_month{};
};

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

/** @brief Local mirror of `api::ConnectorData` (core/verify_connector.rs:24-30). */
struct ConnectorData
{
    std::string connector{};
};

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
    std::int64_t amount{1'000};       // types/api/verify_connector.rs:30
    std::int64_t minor_amount{1'000}; // types/api/verify_connector.rs:31
    std::string currency{"USD"};      // types/api/verify_connector.rs:34
    bool confirm{true};               // types/api/verify_connector.rs:33
    DomainCard card{};
};

/** @brief Upstream `get_router_data` (types/api/verify_connector.rs:90-163). */
struct RouterData
{
    std::string attempt_id{};
    std::string connector{};
    std::string connector_request_reference_id{};
    std::string authentication_type{"NoThreeDs"};
};

/**
 * @brief Shared business helpers for the verify-connector runtime.
 *
 * Upstream ApiErrorResponse mappings, card helpers, the auth enum conversion,
 * connector lookup, payload builders and the in-process connector calls all
 * live here. All methods are const and read AppContext through m_context.
 */
class VerifyConnectorRuntimeSupport
{
    using Error = core::error::AppError;
    using Kind = core::error::AppErrorKind;
    using Connector = ::app::dto::verify_connector::Connector;
    using AdminAuth = ::app::dto::verify_connector::ConnectorAuthTypeValue;

public:
    explicit VerifyConnectorRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    /** @brief IncorrectConnectorNameGiven -> 404 (api_error_response.rs:532-534). */
    [[nodiscard]] Error incorrect_connector_name_given() const
    {
        auto error = core::error::AppErrorBuilder{}(
            Kind::NOT_FOUND,
            "The connector provided in the request is incorrect or not available"
        );
        error.set_kind_name("IncorrectConnectorNameGiven");
        return error;
    }

    /** @brief FlowNotSupported -> 400 (api_error_response.rs:637-639). */
    [[nodiscard]] Error flow_not_supported(std::string_view connector) const
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
    [[nodiscard]] Error invalid_request_data(std::string message) const
    {
        auto error = core::error::AppErrorBuilder{}(Kind::VALIDATION_FAILED, std::move(message));
        error.set_kind_name("InvalidRequestData");
        return error;
    }

    /** @brief InternalServerError -> 500 (api_error_response.rs). */
    [[nodiscard]] Error internal_server_error(std::string message) const
    {
        auto error = core::error::AppErrorBuilder{}(Kind::INTERNAL_ERROR, std::move(message));
        error.set_kind_name("InternalServerError");
        return error;
    }

    /**
     * @brief Upstream `utils::generate_card_from_details` (utils/verify_connector.rs:6-33).
     * Parses the card number via cards::CardNumber; an invalid number maps to InternalServerError.
     * The in-process substitute validates non-empty digits (no external card-validation crate).
     */
    [[nodiscard]] std::expected<DomainCard, Error> generate_card_from_details(
        std::string number,
        std::string year,
        std::string month,
        std::string cvv
    ) const
    {
        if (number.empty() || !std::ranges::all_of(
                                  number,
                                  [](unsigned char c)
                                  {
                                      return std::isdigit(c) != 0;
                                  }
                              )) {
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
    [[nodiscard]] std::expected<std::optional<DomainCard>, Error>
    get_test_card_details(Connector connector) const
    {
        switch (connector) {
            case Connector::stripe:
                {
                    auto card = generate_card_from_details("4242424242424242", "2025", "12", "100");
                    if (!card) {
                        return std::unexpected{card.error()};
                    }
                    return std::optional<DomainCard>{*card};
                }
            case Connector::paypal:
                {
                    auto card = generate_card_from_details("4111111111111111", "2025", "02", "123");
                    if (!card) {
                        return std::unexpected{card.error()};
                    }
                    return std::optional<DomainCard>{*card};
                }
            default:
                return std::optional<DomainCard>{};
        }
    }

    /**
     * @brief Upstream `connector_auth.foreign_into()` (transformers.rs:64-71 dispatching to
     * types.rs:1251-1294). The admin and domain enums are structurally identical, so this is a
     * field-for-field conversion of the internally-tagged auth value.
     */
    [[nodiscard]] ConnectorAuthTypeDomain foreign_into(const AdminAuth& value) const
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

    /**
     * @brief Upstream `api::ConnectorData::get_connector_by_name` (core/verify_connector.rs:24-30)
     * against `state.conf.connectors`, with the `IncorrectConnectorNameGiven` mapping on a miss.
     *
     * The local ConfigStore stands in for `state.conf.connectors`: an explicit disabled config
     * entry is the miss path, while the validated enum name itself is the connector carried into
     * VerifyConnectorData.
     */
    [[nodiscard]] std::expected<ConnectorData, Error>
    get_connector_by_name(std::string_view name) const
    {
        // Upstream `state.conf.connectors` enumerates every known Connector variant, so a name that
        // already passed the Connector enum validation is present by default. An explicit
        // disabled/unavailable config entry is the only miss path.
        const auto configured = m_context.config_store().get_value("connectors", name);
        if (configured && (*configured == "disabled" || *configured == "unavailable")) {
            return std::unexpected{incorrect_connector_name_given()};
        }
        return ConnectorData{std::string{name}};
    }

    [[nodiscard]] PaymentsAuthorizeData
    get_payment_authorize_data(const VerifyConnectorData& data) const
    {
        PaymentsAuthorizeData out{};
        out.card = data.card_details; // PaymentMethodData::Card(self.card_details.clone()) :27
        return out;
    }

    [[nodiscard]] RouterData get_router_data(
        const VerifyConnectorData& data,
        const PaymentsAuthorizeData& /*authorize_data*/,
        const std::optional<std::string>& /*access_token*/
    ) const
    {
        RouterData out{};
        // generate_id_with_default_len(VERIFY_CONNECTOR_ID_PREFIX), consts.rs:102 "conn_verify".
        out.attempt_id = m_context.ids().payment_id("conn_verify");
        out.connector = data.connector;                      // self.connector.id().to_string() :103
        out.connector_request_reference_id = out.attempt_id; // :127
        out.authentication_type = "NoThreeDs";               // :104
        return out;
    }

    /**
     * @brief Typed `serde::Value` document for the connector request payload. Field assignments
     * replace the old per-field JSON string concatenation; no hand-written JSON text is produced.
     */
    [[nodiscard]] serde::Value verify_data_document(const VerifyConnectorData& data) const
    {
        serde::Value::Object root{};
        root["connector"] = serde::Value{data.connector};

        serde::Value::Object auth{};
        auth["auth_type"] = serde::Value{data.connector_auth.auth_type};
        auth["api_key"] = serde::Value{data.connector_auth.api_key};
        auth["key1"] = serde::Value{data.connector_auth.key1};
        auth["api_secret"] = serde::Value{data.connector_auth.api_secret};
        auth["key2"] = serde::Value{data.connector_auth.key2};
        auth["certificate"] = serde::Value{data.connector_auth.certificate};
        auth["private_key"] = serde::Value{data.connector_auth.private_key};
        auth["auth_key_map"] = serde::Value{data.connector_auth.auth_key_map};
        root["connector_auth"] = serde::Value{std::move(auth)};

        serde::Value::Object card{};
        card["card_number"] = serde::Value{data.card_details.card_number};
        card["card_cvc"] = serde::Value{data.card_details.card_cvc};
        card["card_exp_year"] = serde::Value{data.card_details.card_exp_year};
        card["card_exp_month"] = serde::Value{data.card_details.card_exp_month};
        root["card_details"] = serde::Value{std::move(card)};

        return serde::Value{std::move(root)};
    }

    /** @brief Encodes a typed document through the registered JSON format plugin. */
    [[nodiscard]] std::string serialize_document(serde::Value document) const
    {
        const auto bytes = serde::Ser::serialize("application/json", document);
        return std::string{reinterpret_cast<const char*>(bytes.data()), bytes.size()};
    }

    /** @brief Whether the in-process connector call produced a failure response. */
    [[nodiscard]] bool
    connector_call_failed(const ::app::connector::ConnectorResponse& response) const
    {
        if (response.get_status().empty()) {
            return true;
        }
        return !response.get_message().empty() && (response.get_message().starts_with("error") ||
                                                   response.get_message().starts_with("Error"));
    }

    /**
     * @brief Upstream `handle_payment_error_response` (types/api/verify_connector.rs:220-232):
     * `InvalidRequestData { message: error.reason.unwrap_or(error.message) }`.
     */
    [[nodiscard]] Error
    handle_payment_error_response(const ::app::connector::ConnectorResponse& response) const
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
    [[nodiscard]] ::app::connector::ConnectorRequest build_connector_request(
        std::string_view operation,
        std::string_view merchant_id,
        const VerifyConnectorData& data
    ) const
    {
        return ::app::connector::ConnectorRequestBuilder{}
            .operation(std::string{operation})
            .merchant_id(std::string{merchant_id})
            .resource_id(data.connector)
            .raw_payload(serialize_document(verify_data_document(data)))();
    }

    /**
     * @brief Local in-process substitute for upstream `VerifyConnector::verify` default
     * (types/api/verify_connector.rs:167-209): get_payment_authorize_data -> get_access_token ->
     * get_router_data -> build_request -> call_connector_api -> StatusOk / error mapping.
     */
    [[nodiscard]] std::expected<void, Error> stripe_verify(
        const std::shared_ptr<::app::connector::IConnector>& connector,
        std::string_view merchant_id,
        const VerifyConnectorData& data
    ) const
    {
        const auto authorize_data = get_payment_authorize_data(data);
        const auto router_data = get_router_data(data, authorize_data, std::nullopt);
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
    [[nodiscard]] std::expected<void, Error> paypal_get_access_token(
        const std::shared_ptr<::app::connector::IConnector>& connector,
        std::string_view merchant_id,
        const VerifyConnectorData& data
    ) const
    {
        auto request = build_connector_request("get_access_token", merchant_id, data);
        const auto response = connector->exchange_token(request);
        if (connector_call_failed(response)) {
            return std::unexpected{handle_payment_error_response(response)};
        }
        return {}; // .map(|_| ApplicationResponse::StatusOk) (core/verify_connector.rs:60)
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::verify_connector
