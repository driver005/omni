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

/**
 * @brief Local serializable shape for the upstream typed `VerifyConnectorRequest`.
 * @note Upstream fields are enum-typed: `connector_name: Connector` and
 * `connector_account_details: admin::ConnectorAuthType`. The local generated DTO wrappers
 * (ConnectorValue / ConnectorAuthTypeValue) carry no serde::Serializable, so decode into this
 * normalized shape instead: `connector_name` is the enum wire name, and the structured auth
 * object is carried as its JSON text so serde can move it as a plain std::string.
 */
class VerifyConnectorRequestWire
{
public:
    void set_connector_name(std::string value) { connector_name = std::move(value); }
    [[nodiscard]] const std::string& get_connector_name() const noexcept { return connector_name; }
    void set_connector_account_details(std::string value) { connector_account_details = std::move(value); }
    [[nodiscard]] const std::string& get_connector_account_details() const noexcept { return connector_account_details; }

    std::string connector_name{};
    std::string connector_account_details{};
};

/** @brief Local mirror of upstream `utils::generate_card_from_details` test card. */
class TestCardDetailsWire
{
public:
    void set_card_number(std::string value) { card_number = std::move(value); }
    [[nodiscard]] const std::string& get_card_number() const noexcept { return card_number; }
    void set_card_exp_year(std::string value) { card_exp_year = std::move(value); }
    [[nodiscard]] const std::string& get_card_exp_year() const noexcept { return card_exp_year; }
    void set_card_exp_month(std::string value) { card_exp_month = std::move(value); }
    [[nodiscard]] const std::string& get_card_exp_month() const noexcept { return card_exp_month; }
    void set_card_cvv(std::string value) { card_cvv = std::move(value); }
    [[nodiscard]] const std::string& get_card_cvv() const noexcept { return card_cvv; }

    std::string card_number{};
    std::string card_exp_year{};
    std::string card_exp_month{};
    std::string card_cvv{};
};

/**
 * @brief Local serializable shape matching upstream `types::VerifyConnectorData`:
 * `{connector, connector_auth, card_details}`. Built from the normalized request and handed
 * to the connector layer as one structured payload instead of a raw request echo.
 */
class VerifyConnectorDataWire
{
public:
    void set_connector(std::string value) { connector = std::move(value); }
    [[nodiscard]] const std::string& get_connector() const noexcept { return connector; }
    void set_connector_auth(std::string value) { connector_auth = std::move(value); }
    [[nodiscard]] const std::string& get_connector_auth() const noexcept { return connector_auth; }
    void set_card_details(std::string value) { card_details = std::move(value); }
    [[nodiscard]] const std::string& get_card_details() const noexcept { return card_details; }

    std::string connector{};
    std::string connector_auth{};
    std::string card_details{};
};

/**
 * @brief Deterministic substitute for upstream `utils::get_test_card_details(Connector)`.
 * Returns the same fixed test cards for Stripe and Paypal and nothing for every other
 * connector, which is what drives the upstream `FlowNotSupported` gate.
 */
class TestCardDetailsLookup
{
public:
    [[nodiscard]] std::optional<TestCardDetailsWire> operator()(std::string_view connector_name) const
    {
        if (connector_name == "stripe") {
            return card("4242424242424242", "2025", "12", "100");
        }
        if (connector_name == "paypal") {
            return card("4111111111111111", "2025", "02", "123");
        }
        return std::nullopt;
    }

private:
    [[nodiscard]] TestCardDetailsWire card(
        std::string_view number,
        std::string_view year,
        std::string_view month,
        std::string_view cvv
    ) const
    {
        TestCardDetailsWire result{};
        result.set_card_number(std::string{number});
        result.set_card_exp_year(std::string{year});
        result.set_card_exp_month(std::string{month});
        result.set_card_cvv(std::string{cvv});
        return result;
    }
};

} // namespace app::handlers::verify_connector

template<>
struct serde::Serializable<app::handlers::verify_connector::VerifyConnectorRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector_name", &app::handlers::verify_connector::VerifyConnectorRequestWire::get_connector_name, &app::handlers::verify_connector::VerifyConnectorRequestWire::set_connector_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_account_details", &app::handlers::verify_connector::VerifyConnectorRequestWire::get_connector_account_details, &app::handlers::verify_connector::VerifyConnectorRequestWire::set_connector_account_details, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "verify_connector_request_wire"; }
};

template<>
struct serde::Serializable<app::handlers::verify_connector::TestCardDetailsWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"card_number", &app::handlers::verify_connector::TestCardDetailsWire::get_card_number, &app::handlers::verify_connector::TestCardDetailsWire::set_card_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_exp_year", &app::handlers::verify_connector::TestCardDetailsWire::get_card_exp_year, &app::handlers::verify_connector::TestCardDetailsWire::set_card_exp_year, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_exp_month", &app::handlers::verify_connector::TestCardDetailsWire::get_card_exp_month, &app::handlers::verify_connector::TestCardDetailsWire::set_card_exp_month, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_cvv", &app::handlers::verify_connector::TestCardDetailsWire::get_card_cvv, &app::handlers::verify_connector::TestCardDetailsWire::set_card_cvv, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "verify_connector_test_card_wire"; }
};

template<>
struct serde::Serializable<app::handlers::verify_connector::VerifyConnectorDataWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector", &app::handlers::verify_connector::VerifyConnectorDataWire::get_connector, &app::handlers::verify_connector::VerifyConnectorDataWire::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_auth", &app::handlers::verify_connector::VerifyConnectorDataWire::get_connector_auth, &app::handlers::verify_connector::VerifyConnectorDataWire::set_connector_auth, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_details", &app::handlers::verify_connector::VerifyConnectorDataWire::get_card_details, &app::handlers::verify_connector::VerifyConnectorDataWire::set_card_details, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "verify_connector_data_wire"; }
};

export namespace app::handlers::verify_connector {

class VerifyConnectorRouteHandlers
{
    using Error = core::error::AppError;
    using Kind = core::error::AppErrorKind;
    using Builder = core::error::AppErrorBuilder;

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

        // Upstream takes merchant/profile from the auth extractor (auth.profile.id is passed
        // as profile_id), never from the URL path of /account/connectors/verify.
        std::string merchant_id = auth_ctx.merchant_id();
        if (merchant_id.empty()) {
            merchant_id = app::http::merchant_id_from_header{}(req);
        }
        const std::string profile_id{auth_ctx.profile_id()};
        (void)profile_id; // upstream passes profile_id through to core; no local profile store.

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, Builder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto document = serde::Ser::decode_generic("application/json", body);
        if (!document) {
            send_error(res, send, Builder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Normalize the untyped JSON tree into the typed wire DTO. connector_name is an enum
        // wire string; connector_account_details is a structured ConnectorAuthType object, so
        // carry its JSON text as a string that serde can reflect into a plain std::string field.
        std::string name_raw{};
        std::string details_json{};
        const auto& root = document->get();
        if (const auto* object = std::get_if<interfaces::Value::Object>(&root)) {
            if (const auto name = object->get("connector_name")) {
                const auto& name_value = name.value().get();
                if (const auto* text = std::get_if<std::string>(&name_value)) {
                    name_raw = *text;
                }
            }
            if (const auto details = object->get("connector_account_details")) {
                details_json = serde::Ser::encode_json(details.value());
            }
        }
        const auto normalized = std::string{"{\"connector_name\":"}
                              + serde::Ser::encode_json(name_raw)
                              + ",\"connector_account_details\":"
                              + serde::Ser::encode_json(details_json)
                              + "}";
        auto input = serde::Ser::deserialize<VerifyConnectorRequestWire>("application/json", normalized);
        if (!input) {
            send_error(res, send, Builder{}(Kind::INVALID_JSON, "invalid VerifyConnectorRequest"));
            return;
        }

        const std::string connector_name = input->get_connector_name();
        if (connector_name.empty()) {
            // Upstream: IncorrectConnectorNameGiven.
            send_error(res, send, Builder{}(Kind::INVALID_REQUEST, "incorrect connector name given"));
            return;
        }

        // Upstream: get_test_card_details(req.connector_name) -> FlowNotSupported when the
        // connector has no test-card details (only Stripe and Paypal do).
        const auto card_details = TestCardDetailsLookup{}(connector_name);
        if (!card_details) {
            send_error(res, send, Builder{}(
                Kind::INVALID_REQUEST,
                "flow not supported for connector: " + connector_name
            ));
            return;
        }

        // Resolve the merchant identity from the persisted account instead of trusting a raw
        // header value. Not fatal: upstream authenticates via JWT on this route.
        if (!merchant_id.empty()) {
            if (const auto account = m_context->repository().find_merchant_account_by_string(merchant_id)) {
                merchant_id = account->get_merchant_id();
            }
        }

        // Upstream: generate_id_with_default_len(VERIFY_CONNECTOR_ID_PREFIX) for the attempt.
        const auto attempt_id = m_context->ids().payment_id("conn_verify_");

        // Credentials are secret-bearing: wrap for fingerprinting and keep the secret out of
        // any cache key (the fingerprint is connector-scoped keyed material, not a business
        // record). This is the local stand-in for `connector_auth.foreign_into()`.
        const app::crypto::SecretString secret_credentials{input->get_connector_account_details()};
        const auto fingerprint = app::crypto::KeyedHasher{"verify_connector." + connector_name}
                                     .hash(secret_credentials.value());

        // Build the upstream VerifyConnectorData {connector, connector_auth, card_details}
        // payload. This replaces the old generic ConnectorRequestBuilder request echo.
        VerifyConnectorDataWire verify_data{};
        verify_data.set_connector(connector_name);
        verify_data.set_connector_auth(secret_credentials.value());
        verify_data.set_card_details(serde::Ser::encode_json(*card_details));
        const auto verify_data_json = serde::Ser::encode_json(verify_data);

        const std::string operation = connector_name == "stripe" ? "verify" : "get_access_token";
        auto connector_request = app::connector::ConnectorRequestBuilder{}
                                     .operation(operation)
                                     .merchant_id(merchant_id)
                                     .resource_id(connector_name)
                                     .raw_payload(verify_data_json)();

        // Upstream: ConnectorData::get_connector_by_name(...) against state.conf.connectors,
        // mapping a miss to IncorrectConnectorNameGiven. The local ConnectorRouter is the only
        // service surface for connector selection; select_checked returns a semantic
        // CONNECTOR_ERROR when no backend is registered or the backend is killed/rolled out.
        auto selected = m_context->connector_router().select_checked(merchant_id);
        if (!selected) {
            send_error(res, send, selected.error());
            return;
        }
        const auto connector = *selected;

        // Upstream dispatch: Stripe::verify and Paypal::get_access_token. The local connector
        // contract exposes the equivalent instance operations verify() (Stripe credential
        // check) and exchange_token() (Paypal access-token fetch). Both receive the structured
        // VerifyConnectorData payload as their raw request.
        app::connector::ConnectorResponse connector_response{};
        if (connector_name == "stripe") {
            connector_response = connector->verify(connector_request);
        } else { // paypal
            connector_response = connector->exchange_token(connector_request);
        }
        if (connector_response.get_status().empty()) {
            send_error(res, send, Builder{}(Kind::CONNECTOR_ERROR, "connector verification returned no status"));
            return;
        }

        // Ephemeral state only: attempt -> credential fingerprint for correlation/dedupe.
        m_context->cache().put(
            "verify_connector.attempt:" + attempt_id,
            core::cache::to_cache_value(fingerprint)
        );

        // Upstream #[instrument(flow = ?Flow::VerifyPaymentConnector)] substitute.
        m_context->metrics().record_flow("VerifyPaymentConnector");
        m_context->metrics().counter("VERIFY_PAYMENT_CONNECTOR").increment();

        // Upstream returns RouterResponse<()> / StatusOk with no payload. Emit an empty 200
        // body: no fabricated status object and no echo of the request.
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
