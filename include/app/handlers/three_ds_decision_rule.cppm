module;
export module app_handlers:three_ds_decision_rule;

import std;
import interfaces;
import core_router;
import core_serde;
import core_error;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import app_http;
import app_metrics;
import app_services;

export namespace app::handlers::three_ds_decision_rule {

class ThreeDsDecisionRuleRouteHandlers
{
public:
    void handle_execute_decision_rule(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream route uses HeaderAuth(ApiKeyAuth { allow_connected_scope_operation:
        // false, allow_platform_self_operation: false }). The rule is resolved from the
        // auditable RouteAuthRegistry rather than built inline. For the API_KEY scheme the
        // two scope flags do not affect authorization (AppAuthorizer only consults them for
        // HEADER_MERCHANT/HEADER_ORG), but the registry row is recorded as a shared-file
        // follow-up so its evidence matches upstream.
        const auto rule = m_context->auth_rules().rule_for("execute_decision_rule");
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto auth = m_context->authorizer().authorize_route(auth_context, rule);
        if (auth) {
            send_error(res, send, *auth);
            return;
        }

        // Flow::ThreeDsDecisionRuleExecute.
        m_context->metrics().record_flow("ThreeDsDecisionRuleExecute");
        m_context->metrics().counter("THREE_DS_DECISION_RULE_EXECUTED").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto routing_id = parsed->get_routing_id().get_value();

        // Persistence + merchant scoping. Upstream reads
        // find_routing_algorithm_by_algorithm_id_processor_merchant_id(routing_id,
        // processor_merchant_id) where the processor id comes from the authenticated
        // Platform. Locally the merchant id is taken from the authenticated AuthContext
        // (the x-merchant-id header origin), which is the single identity source used by
        // authorization. A miss maps to ResourceIdNotFound -> NOT_FOUND.
        const auto& merchant_id = auth_context.merchant_id();
        const auto config = m_context->repository().find_routing_config_by_string(routing_id);
        if (!config || (!merchant_id.empty() && !config->get_merchant_id().empty()
                        && config->get_merchant_id() != merchant_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "routing algorithm not found"));
            return;
        }

        // DSL rule execution. Upstream parses algorithm_data -> Algorithm -> ast::Program,
        // builds dsl_inputs::BackendInput via ForeignFrom, initializes
        // VirInterpreterBackend::with_program and calls interpreter.execute. This runtime
        // evaluates the stored program with the local deterministic DecisionEngine DSL
        // subset (rules over amount/currency/payment_method/country/connector). A program
        // that cannot be decoded or executed maps to InternalServerError -> INTERNAL_ERROR,
        // matching upstream's distinct parse/init/execute failure mapping.
        auto program = core_serde::JsonCodec{}.decode<app::services::DecisionProgram>(config->get_config_json());
        if (!program) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INTERNAL_ERROR, "failed to parse three_ds decision program"));
            return;
        }

        // Full backend-input construction: feed the typed request fields into the engine
        // context. The local DecisionContext subset exposes amount, currency,
        // payment_method, country and connector; payment.business_country,
        // payment_method.card_network/card_discovery and issuer fields are mapped onto those
        // slots. customer_device, metadata, card_bin, extended_card_bin and acquirer
        // fraud_rate have no field in the local DSL context and remain a documented subset
        // limitation (see shared-file follow-up to extend app_services::DecisionContext).
        app::services::DecisionContext decision_context;
        const auto& payment = parsed->get_payment();
        decision_context.amount = payment.get_amount().get_value();
        decision_context.currency = payment.get_currency().get_value();
        if (payment.get_business_country()) {
            decision_context.country = payment.get_business_country()->get_value();
        }
        const auto& payment_method = parsed->get_payment_method();
        if (payment_method) {
            if (payment_method->get_card_network()) {
                decision_context.payment_method = payment_method->get_card_network()->get_value();
            } else if (payment_method->get_card_discovery()) {
                decision_context.payment_method = payment_method->get_card_discovery()->get_value();
            }
        }

        const auto& issuer = parsed->get_issuer();
        const auto& acquirer = parsed->get_acquirer();
        if (issuer) {
            if (decision_context.country.empty() && issuer->get_country()) {
                decision_context.country = issuer->get_country()->get_value();
            }
            if (issuer->get_name()) {
                decision_context.connector = *issuer->get_name();
            }
        }
        if (decision_context.country.empty() && acquirer && acquirer->get_country()) {
            decision_context.country = acquirer->get_country()->get_value();
        }

        auto evaluated = m_context->decision_engine().evaluate_program(*program, decision_context);
        if (!evaluated) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INTERNAL_ERROR, evaluated.error().get_message()));
            return;
        }

        // The engine action must be one of the seven upstream ThreeDSDecision variants;
        // an unrecognized action is an execution failure (InternalServerError upstream).
        constexpr std::string_view valid_decisions[] = {
            "challenge_requested", "challenge_preferred", "no_preference",
            "three_ds_exemption_requested_tra", "three_ds_exemption_requested_low_value",
            "issuer_three_ds_exemption_requested", "no_three_ds"
        };
        std::string decision = evaluated->get_action();
        bool valid_decision = false;
        for (auto candidate : valid_decisions) {
            if (decision == candidate) {
                valid_decision = true;
                break;
            }
        }
        if (!valid_decision) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INTERNAL_ERROR, "decision program produced an invalid three_ds decision"));
            return;
        }

        // SCA validations ported from upstream
        // utils::apply_sca_validations_during_execute / apply_sca_validations_to_decision.
        // A mandate applies only when BOTH issuer and acquirer are in an SCA-mandated country.
        const auto in_sca_country = [](
            const std::optional<app::dto::three_ds_decision_rule::CountryValue>& country
        ) -> bool {
            if (!country) {
                return false;
            }
            std::string code = country->get_value();
            for (char& c : code) {
                c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            }
            constexpr std::string_view sca_countries[] = {
                "AT", "BE", "BG", "HR", "CY", "CZ", "DK", "EE", "FI", "FR", "DE", "GR", "HU",
                "IE", "IT", "LV", "LT", "LU", "MT", "NL", "PL", "PT", "RO", "SK", "SI", "ES",
                "SE", "IS", "LI", "NO", "AX", "GF", "GP", "MQ", "YT", "RE", "MF", "GB", "GI"
            };
            for (auto sca : sca_countries) {
                if (code == sca) {
                    return true;
                }
            }
            return false;
        };
        const bool issuer_sca = issuer ? in_sca_country(issuer->get_country()) : false;
        const bool acquirer_sca = acquirer ? in_sca_country(acquirer->get_country()) : false;
        const bool sca_mandate_applies = issuer_sca && acquirer_sca;

        if (sca_mandate_applies) {
            // NoThreeDs is not allowed when both PSPs are in an SCA-mandated region.
            if (decision == "no_three_ds") {
                decision = "no_preference";
            }
        } else {
            // Outside regulated regions exemption / preference decisions are forced to a
            // challenge. NoPreference is preserved because it does not request an exemption.
            if (decision != "no_three_ds" && decision != "no_preference") {
                decision = "challenge_requested";
            }
        }

        app::dto::three_ds_decision_rule::ThreeDSDecisionValue decision_value;
        decision_value.set_value(decision);
        app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteResponse response;
        response.set_decision(decision_value);

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/three_ds_decision"}
            .add_route(
                Route{"/execute"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_execute_decision_rule(req, res, std::move(send));
                    })
                    .summary("Hyperswitch execute_decision_rule")
            );
    }

    explicit ThreeDsDecisionRuleRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
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

} // namespace app::handlers::three_ds_decision_rule
