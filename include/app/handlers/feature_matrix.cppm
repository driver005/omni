module;
export module app_handlers:feature_matrix;

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
import app_http;
import app_connector;
import app_metrics;
import app_store;
import app_services;

export namespace app::handlers::feature_matrix {

/**
 * @brief GET /feature_matrix domain handler.
 *
 * Upstream `routes/feature_matrix.rs` registers exactly one route (`fetch_feature_matrix`,
 * `app.rs:3433-3435`) under NoAuth + LockAction::NotApplicable. All generation happens in
 * the internal helper `generate_feature_matrix` (`routes/feature_matrix.rs:42-161`). The two
 * public methods below share one private generation path; the second route (`/generate_...`)
 * has no upstream counterpart and is kept only for wire compatibility with the existing port.
 */
class FeatureMatrixRouteHandlers
{
public:
    void handle_fetch_feature_matrix(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream `&auth::NoAuth` (routes/feature_matrix.rs:36-37) resolves to a NO_AUTH
        // rule in RouteAuthRegistry; authorize_route is a pass-through for that scheme.
        const auto rule = m_context->auth_rules().rule_for("fetch_feature_matrix");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        generate_matrix(
            req,
            res,
            send,
            "feature_matrix.fetch_feature_matrix",
            "feature_matrix_fetch"
        );
    }

    void handle_generate_feature_matrix(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Alias of the internal upstream helper. Same NO_AUTH rule and same generation path
        // as fetch_feature_matrix so the two routes cannot drift apart.
        const auto rule = m_context->auth_rules().rule_for("fetch_feature_matrix");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        generate_matrix(
            req,
            res,
            send,
            "feature_matrix.generate_feature_matrix",
            "feature_matrix_generate"
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/feature_matrix"}
            .add_route(
                Route{"/"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_fetch_feature_matrix(req, res, std::move(send));
                    })
                    .summary("Hyperswitch fetch_feature_matrix")
            )
            .add_route(
                Route{"/generate_feature_matrix"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_generate_feature_matrix(req, res, std::move(send));
                    })
                    .summary("Hyperswitch generate_feature_matrix (internal helper alias)")
            );
    }

    explicit FeatureMatrixRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    /** @brief One shared implementation of the upstream `generate_feature_matrix` helper. */
    void generate_matrix(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view flow_name,
        std::string_view counter_name
    )
    {
        // Upstream takes an optional FeatureMatrixRequest (connectors = None). Normalize the
        // wire payload into the DTO's wrapped shape: clients send connectors as enum strings,
        // the DTO wraps each in ConnectorValue.
        const auto body = req.body_as_string();
        std::string normalized = R"({})";
        if (!body.empty()) {
            auto generic = serde::Ser::decode_generic("application/json", body);
            if (!generic) {
                send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            if (const auto* object = std::get_if<interfaces::Value::Object>(&generic->get())) {
                if (const auto connectors_field = object->get("connectors")) {
                    if (const auto* array = std::get_if<interfaces::Value::Array>(&connectors_field.value().get())) {
                        std::string items;
                        for (const auto& element : *array) {
                            std::string name;
                            if (const auto* text = std::get_if<std::string>(&element.get())) {
                                name = *text;
                            } else if (const auto* item_object = std::get_if<interfaces::Value::Object>(&element.get())) {
                                if (const auto value_field = item_object->get("value")) {
                                    if (const auto* value_text = std::get_if<std::string>(&value_field.value().get())) {
                                        name = *value_text;
                                    }
                                }
                            }
                            if (!name.empty()) {
                                if (!items.empty()) {
                                    items += ',';
                                }
                                items += R"({"value":)" + serde::Ser::encode_json(name) + "}";
                            }
                        }
                        normalized = R"({"connectors":[)" + items + "]}";
                    }
                }
            }
        }

        using Input = app::dto::feature_matrix::FeatureMatrixRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", normalized);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream: req.connectors, or Connector::iter() when unspecified
        // (routes/feature_matrix.rs:46-48). The local substitute for the compile-time enum is
        // the documented known_connector_names() registry below. Persisted merchant connectors
        // are consulted for metadata only (see build_entry); they no longer define the default
        // connector set.
        std::vector<std::string> names{};
        if (parsed->get_connectors()) {
            for (const auto& connector : *parsed->get_connectors()) {
                names.push_back(connector.get_value());
            }
        }
        if (names.empty()) {
            names = known_connector_names();
        }

        // Upstream filter_map drops connectors whose convert_connector fails
        // (routes/feature_matrix.rs:53-64). is_known_connector is the local equivalent of
        // FeatureMatrixConnectorData::convert_connector validation.
        const auto records = m_context->repository().list_merchant_connectors();
        std::vector<app::dto::feature_matrix::ConnectorFeatureMatrixResponse> entries{};
        entries.reserve(names.size());
        for (const auto& name : names) {
            if (!is_known_connector(name)) {
                continue;
            }
            entries.push_back(build_entry(name, records));
        }

        app::dto::feature_matrix::FeatureMatrixListResponse response;
        response.set_connector_count(entries.size());
        response.set_connectors(std::move(entries));

        // Upstream server_wrap instruments Flow::FeatureMatrix; record_flow is the local
        // equivalent. LockAction::NotApplicable means no lock is acquired (read-only GET).
        m_context->metrics().record_flow(flow_name);
        m_context->metrics().counter(counter_name).increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    /**
     * @brief Local substitute for the upstream Connector enum registry (`Connector::iter()`).
     * The upstream enum lives in common_enums and is not exported to handlers; this list
     * mirrors its strum snake_case names (common_enums/src/connector_enums.rs:32-235).
     */
    [[nodiscard]] std::vector<std::string> known_connector_names() const
    {
        return std::vector<std::string>{
            "absa_sanlam", "authipay", "adyenplatform",
            "stripe_billing_test", "phonypay", "fauxpay", "pretendpay", "stripe_test",
            "adyen_test", "checkout_test", "paypal_test",
            "aci", "adyen", "affirm", "airwallex", "amazonpay", "archipel", "authorizedotnet",
            "bambora", "bamboraapac", "bankofamerica", "barclaycard", "betterpayment", "billwerk",
            "bitpay", "bluesnap", "blackhawknetwork", "calida", "boku", "braintree", "breadpay",
            "cardinal", "cashtocode", "celero", "chargebee", "checkbook", "checkout", "citigate",
            "coinbase", "coingate", "custombilling", "cryptopay", "ctp_mastercard", "ctp_visa",
            "cybersource", "cybersourcedecisionmanager", "d24", "datatrans", "deutschebank",
            "digitalvirgo", "dlocal", "dwolla", "ebanx", "ecommpay", "envoy", "elavon", "etisalat",
            "elavon_pg", "facilitapay", "finix", "fiserv", "fiservemea", "fiservcommercehub",
            "fiuu", "flexiti", "forte", "getnet", "gigadat", "givepayments", "globalpay",
            "globalpayments_heartland", "globalpayments_realex", "globepay", "gocardless",
            "gotyme_sanlam", "gpayments", "hipay", "helcim", "hyperpg", "hyperswitch_vault",
            "interpayments", "inespay", "iatapay", "ilixium", "imerchantsolutions", "itaubank",
            "jpmorgan", "jpmorgan_orbital", "juspay", "juspaythreedsserver", "klarna", "loonio",
            "merchante", "mifinity", "mollie", "moneris", "multisafepay", "netcetera", "nexinets",
            "nexixpay", "nmi", "nomupay", "noon", "nordea", "novalnet", "nuvei", "opennode",
            "paybox", "paydotcom", "payhound", "payload", "payme", "paynearme", "payone",
            "paypal", "paysafe", "paystack", "paytm", "payconex", "payu", "peachpayments",
            "payjustnow", "payjustnowinstore", "phonepe", "placetopay", "powertranz", "prophetpay",
            "rapyd", "razorpay", "recurly", "redsys", "revolv3", "saferpay", "santander",
            "shift4", "silverflow", "square", "stax", "stripe", "stripebilling", "taxjar",
            "threedsecureio", "tesouro", "tokenex", "tokenio", "trustly", "truelayer", "trustpay",
            "trustpayments", "tsys", "tsys_transit", "vgs", "volt", "wellsfargo", "wise",
            "worldline", "worldpay", "worldpayraft", "worldpayvantiv", "worldpayxml",
            "worldpaymodular", "signifyd", "nsure", "plaid", "riskified", "sanlam_payshield",
            "xendit", "zen", "zift", "zsl",
        };
    }

    /** @brief Local equivalent of upstream Connector::from_str validation. */
    [[nodiscard]] bool is_known_connector(std::string_view name) const
    {
        if (name.empty()) {
            return false;
        }
        const auto names = known_connector_names();
        for (const auto& known : names) {
            if (std::string_view{known} == name) {
                return true;
            }
        }
        return false;
    }

    /** @brief Small CSV helper for config-derived feature values. */
    [[nodiscard]] std::vector<std::string> split_csv(std::string_view csv) const
    {
        std::vector<std::string> values{};
        while (!csv.empty()) {
            const auto comma = csv.find(',');
            std::string_view token = csv.substr(0, comma);
            while (!token.empty() && token.front() == ' ') {
                token.remove_prefix(1);
            }
            while (!token.empty() && token.back() == ' ') {
                token.remove_suffix(1);
            }
            if (!token.empty()) {
                values.emplace_back(token);
            }
            if (comma == std::string_view::npos) {
                break;
            }
            csv.remove_prefix(comma + 1);
        }
        return values;
    }

    /** @brief Deterministic substitute for `connector.get_supported_webhook_flows()`. */
    [[nodiscard]] std::vector<app::dto::feature_matrix::EventClassValue>
    build_webhook_flows(std::string_view name) const
    {
        const auto configured = m_context->config_store().get_value("connector_webhook_flows", name);
        const auto classes = split_csv(configured.value_or(std::string{"PAYMENT,REFUND,DISPUTE"}));
        std::vector<app::dto::feature_matrix::EventClassValue> flows{};
        flows.reserve(classes.size());
        for (const auto& value : classes) {
            app::dto::feature_matrix::EventClassValue item{};
            item.set_value(value);
            flows.push_back(std::move(item));
        }
        return flows;
    }

    /**
     * @brief Deterministic substitute for `connector.get_supported_payment_methods()` plus
     * the `state.conf.pm_filters` country/currency lookups
     * (routes/feature_matrix.rs:114-161). Upstream static capability tables are not exposed
     * to handlers, so the same wire shape is derived from config_store with a documented
     * default card row.
     */
    [[nodiscard]] std::vector<app::dto::feature_matrix::SupportedPaymentMethod>
    build_payment_methods(std::string_view name) const
    {
        using namespace app::dto::feature_matrix;

        std::vector<std::string> country_values = split_csv(
            m_context->config_store()
                .get_value("connector_supported_countries", name)
                .value_or(std::string{"USA,IND"})
        );
        std::vector<std::string> currency_values = split_csv(
            m_context->config_store()
                .get_value("connector_supported_currencies", name)
                .value_or(std::string{"USD,EUR"})
        );
        const auto capture_values = split_csv(std::string_view{"automatic,manual"});

        std::vector<CountryAlpha3Value> countries{};
        countries.reserve(country_values.size());
        for (auto& value : country_values) {
            CountryAlpha3Value item{};
            item.set_value(std::move(value));
            countries.push_back(std::move(item));
        }
        std::vector<CurrencyValue> currencies{};
        currencies.reserve(currency_values.size());
        for (auto& value : currency_values) {
            CurrencyValue item{};
            item.set_value(std::move(value));
            currencies.push_back(std::move(item));
        }
        std::vector<CaptureMethodValue> captures{};
        captures.reserve(capture_values.size());
        for (const auto& value : capture_values) {
            CaptureMethodValue item{};
            item.set_value(value);
            captures.push_back(std::move(item));
        }

        PaymentMethodValue payment_method{};
        payment_method.set_value("card");
        PaymentMethodTypeValue payment_method_type{};
        payment_method_type.set_value("credit");
        FeatureStatusValue mandates{};
        mandates.set_value("supported");
        FeatureStatusValue refunds{};
        refunds.set_value("supported");

        SupportedPaymentMethod row{};
        row.set_payment_method(std::move(payment_method));
        row.set_payment_method_type(std::move(payment_method_type));
        row.set_payment_method_type_display_name("Credit Card");
        row.set_mandates(std::move(mandates));
        row.set_refunds(std::move(refunds));
        row.set_supported_capture_methods(std::move(captures));
        row.set_supported_countries(std::move(countries));
        row.set_supported_currencies(std::move(currencies));

        std::vector<SupportedPaymentMethod> methods{};
        methods.push_back(std::move(row));
        return methods;
    }

    /** @brief Builds one ConnectorFeatureMatrixResponse entry. */
    [[nodiscard]] app::dto::feature_matrix::ConnectorFeatureMatrixResponse
    build_entry(
        std::string_view name,
        const std::vector<app::store::merchant_connectors::MerchantConnectorRecord>& records
    ) const
    {
        using namespace app::dto::feature_matrix;

        std::size_t metadata_index{records.size()};
        for (std::size_t index{0}; index < records.size(); ++index) {
            if (std::string_view{records[index].get_connector_name()} == name) {
                metadata_index = index;
                break;
            }
        }
        const bool has_metadata{metadata_index < records.size()};

        ConnectorFeatureMatrixResponse entry;
        std::string upper{name};
        std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char character) {
            return static_cast<char>(std::toupper(character));
        });
        entry.set_name(std::move(upper));

        // display_name/description come from connector.get_connector_about() upstream; the
        // persisted merchant connector label is the local metadata source, with the connector
        // name as the deterministic fallback.
        entry.set_display_name(
            has_metadata && !records[metadata_index].get_connector_label().empty()
                ? records[metadata_index].get_connector_label()
                : std::string{name}
        );
        entry.set_description(std::string{name});

        // base_url is always Some(connector.base_url(&conf.connectors)) upstream.
        entry.set_base_url(
            m_context->config_store()
                .get_value("connector_base_url", name)
                .value_or(std::format("https://api.{}.com", name))
        );

        ConnectorIntegrationStatusValue status;
        status.set_value(
            has_metadata && records[metadata_index].get_test_mode() ? "sandbox" : "live"
        );
        entry.set_integration_status(std::move(status));

        HyperswitchConnectorCategoryValue category;
        std::string_view category_name{"payment_gateway"};
        if (has_metadata) {
            const auto& connector_type = records[metadata_index].get_connector_type();
            if (connector_type.find("payout") != std::string::npos) {
                category_name = "payout_processor";
            } else if (connector_type.find("auth") != std::string::npos) {
                category_name = "authentication_provider";
            } else if (connector_type.find("fraud") != std::string::npos ||
                       connector_type.find("frm") != std::string::npos) {
                category_name = "fraud_and_risk_management_provider";
            }
        }
        category.set_value(std::string{category_name});
        entry.set_category(std::move(category));

        entry.set_supported_webhook_flows(build_webhook_flows(name));
        entry.set_supported_payment_methods(build_payment_methods(name));
        return entry;
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
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

} // namespace app::handlers::feature_matrix
