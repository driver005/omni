module;
export module app_handlers:offer_engine;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_connector;
import app_services;

export namespace app::handlers::offer_engine {

/**
 * @brief Typed counterpart of upstream OfferEngineConnectivityResponse.
 *
 * The old handler hand-built this JSON and left reachable/status_code/detail permanently
 * null. The fields are now populated from a real probe result (or from config state when
 * no probe runs), and the response is encoded through serde::Ser like every other DTO.
 */
struct ConnectivityResponse
{
    void set_enabled(bool value) noexcept { enabled = value; }
    [[nodiscard]] const bool& get_enabled() const noexcept { return enabled; }
    void set_reachable(std::optional<bool> value) { reachable = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_reachable() const noexcept { return reachable; }
    void set_status_code(std::optional<std::int64_t> value) { status_code = value; }
    [[nodiscard]] const std::optional<std::int64_t>& get_status_code() const noexcept { return status_code; }
    void set_detail(std::optional<std::string> value) { detail = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_detail() const noexcept { return detail; }

    bool enabled{};
    std::optional<bool> reachable{};
    std::optional<std::int64_t> status_code{};
    std::optional<std::string> detail{};
};

/** @brief Deterministic result of one Offer Engine client probe. */
struct ProbeResult
{
    std::optional<bool> reachable{};
    std::optional<std::int64_t> status_code{};
    std::optional<std::string> detail{};
};

/** @brief Local stand-in for upstream OfferStatus-tagged client entries. */
struct CandidateOffer
{
    std::string code{};
    std::string status{"Ineligible"};
    std::optional<std::string> title{};
    std::optional<std::string> display_title{};
    std::optional<std::string> description{};
    std::optional<std::string> currency{};
    std::optional<std::string> valid_till{};
};

} // namespace app::handlers::offer_engine

template<>
struct serde::Serializable<app::handlers::offer_engine::ConnectivityResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"enabled", &app::handlers::offer_engine::ConnectivityResponse::get_enabled, &app::handlers::offer_engine::ConnectivityResponse::set_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"reachable", &app::handlers::offer_engine::ConnectivityResponse::get_reachable, &app::handlers::offer_engine::ConnectivityResponse::set_reachable, serde::FieldOptions{}>{},
            serde::FieldDesc<"status_code", &app::handlers::offer_engine::ConnectivityResponse::get_status_code, &app::handlers::offer_engine::ConnectivityResponse::set_status_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"detail", &app::handlers::offer_engine::ConnectivityResponse::get_detail, &app::handlers::offer_engine::ConnectivityResponse::set_detail, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "offer_engine_connectivity_response"; }
};

export namespace app::handlers::offer_engine {

class OfferEngineRouteHandlers
{
    using Source = app::dto::offer_engine::OfferEngineCredentialSource;

public:
    void handle_offer_engine_connectivity_check(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream uses &auth::AdminApiAuth for this route (no merchant/profile context).
        // The rule now comes from RouteAuthRegistry, which mirrors that declaration.
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize(auth_context, "offer_engine_connectivity_check")) {
            send_error(res, send, *auth_error);
            return;
        }

        // Upstream takes no payload (`()` in server_wrap). This is a global check: merchant
        // and org dimensions may still be present on the request and are respected by the
        // superposition config resolution below.
        const std::string merchant_id = app::http::merchant_id_from_header{}(req);
        const std::string org_id = app::http::org_id_from_header{}(req);
        const std::string scope = dimension_scope(merchant_id, org_id, {});

        const Source source = credential_source(scope);
        ConnectivityResponse response{};
        std::optional<std::string> reason{};

        if (source == Source::Merchant && merchant_id.empty()) {
            reason = "Offer Engine config could not be resolved: credential source is "
                     "'merchant' but no merchant context is available in a global "
                     "connectivity check";
        } else if (source == Source::Application) {
            if (api_key("application").value().empty()) {
                reason = "Offer Engine config could not be resolved: application api key is missing";
            } else {
                response.set_enabled(true);
            }
        } else if (source == Source::Merchant) {
            if (api_key(scope).value().empty() && api_key(merchant_scope(merchant_id)).value().empty()) {
                reason = "Offer Engine config could not be resolved: merchant api key is missing";
            } else {
                response.set_enabled(true);
            }
        } else {
            reason = "Offer Engine is not enabled in resolved config (offer_engine.enabled is "
                     "false or credential source is none)";
        }

        // A disabled or unresolvable config means no probe ran, so the probe fields stay null
        // exactly as upstream. When enabled, the fields are filled from the connector probe.
        if (response.get_enabled()) {
            const auto probe = probe_connectivity(scope, merchant_id);
            response.set_reachable(probe.reachable);
            response.set_status_code(probe.status_code);
            response.set_detail(probe.detail);
        } else {
            response.set_detail(reason);
        }

        auto text = serde::Ser::encode_json(response);

        m_context->metrics().record_flow("OfferEngineConnectivityCheck");
        m_context->metrics().counter("OFFER_ENGINE_CONNECTIVITY_CHECK").increment();
        send_json(res, send, text, true);
    }

    void handle_offer_engine_browse_offers(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream uses HeaderAuth(ApiKeyAuth { allow_connected_scope_operation:
        // true, allow_platform_self_operation: false }) for browse offers; the registry
        // entry carries those flags.
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize(auth_context, "offer_engine_browse_offers")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::offer_engine::BrowseOffersRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream resolves processor merchant, org and optional profile dimensions from
        // the auth extractor. Merchant/org come from headers, profile from AuthContext.
        const std::string merchant_id = app::http::merchant_id_from_header{}(req);
        const std::string org_id = app::http::org_id_from_header{}(req);
        const std::string profile_id{auth_context.profile_id()};
        const std::string scope = dimension_scope(merchant_id, org_id, profile_id);

        const Source source = credential_source(scope);
        bool allowed{false};
        if (source == Source::Application) {
            allowed = !api_key("application").value().empty();
        } else if (source == Source::Merchant && !merchant_id.empty()) {
            // Upstream reads api_key/merchant_id from merchant_account.offer_engine_config.
            // No such store record exists locally; the resolved secret is persisted as a
            // dimension-scoped config record via ConfigStore instead of an ephemeral cache key.
            allowed = !api_key(scope).value().empty() ||
                      !api_key(merchant_scope(merchant_id)).value().empty();
        }
        if (!allowed) {
            // Upstream maps credential_source::none / unresolvable config to
            // ApiErrorResponse::AccessForbidden { resource: "offer_engine" }. It does not
            // probe merchant existence first — the old 404 branch was added behavior.
            m_context->metrics().counter("OFFER_ENGINE_ACCESS_DENIED").increment();
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::FORBIDDEN,
                "Access forbidden for resource offer_engine"
            ));
            return;
        }

        // The currency filter is now carried into the offer-engine request and the
        // eligibility decision instead of being parsed and discarded.
        std::string currency{};
        if (parsed->get_offer_payment_info().has_value()) {
            currency = parsed->get_offer_payment_info()->get_currency().get_value();
        }

        // OfferEngineClient::browse_offers is represented by the deterministic connector
        // backend. A missing backend is a typed CONNECTOR_ERROR, not a fake empty success.
        auto connector = m_context->connector_router().select(merchant_id);
        if (!connector) {
            const auto error = m_context->connector_router().last_error();
            const std::string message = error.has_value()
                ? error->get_message()
                : std::string{"offer engine client backend is unavailable"};
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::CONNECTOR_ERROR, message));
            return;
        }

        const auto request = app::connector::ConnectorRequestBuilder{}
                                 .operation("offer_engine_browse_offers")
                                 .merchant_id(merchant_id)
                                 .currency(currency)
                                 .resource_id(profile_id.empty() ? merchant_id : profile_id)
                                 .raw_payload(serde::Ser::encode_json(*parsed))();
        const auto client_response = connector->check_feature_matrix(request);

        // Upstream drops every entry whose status != Eligible and maps survivors through
        // BrowseOffer::from. Eligibility is evaluated by the injected decision engine over
        // the request currency, using a config-held program when one is present.
        std::vector<CandidateOffer> candidates{};
        CandidateOffer candidate{};
        candidate.code = client_response.get_reference_id().empty()
            ? client_response.get_status()
            : client_response.get_reference_id();
        candidate.status = eligibility_status(scope, currency);
        if (!currency.empty()) {
            candidate.currency = currency;
        }
        candidates.push_back(std::move(candidate));

        using Response = app::dto::offer_engine::BrowseOffersResponse;
        Response typed_response{};
        std::vector<app::dto::offer_engine::BrowseOffer> offers{};
        for (const auto& entry : candidates) {
            if (entry.status != "Eligible") {
                continue;
            }
            app::dto::offer_engine::BrowseOffer offer{};
            offer.set_code(entry.code);
            offer.set_title(entry.title);
            offer.set_display_title(entry.display_title);
            offer.set_description(entry.description);
            if (entry.currency.has_value()) {
                app::dto::offer_engine::CurrencyValue value{};
                value.set_value(*entry.currency);
                offer.set_currency(value);
            }
            if (entry.valid_till.has_value()) {
                app::dto::offer_engine::PrimitiveDateTimeValue value{};
                value.set_value(*entry.valid_till);
                offer.set_valid_till(value);
            }
            offers.push_back(std::move(offer));
        }
        typed_response.set_offers(std::move(offers));
        auto text = serde::Ser::encode_json(typed_response);

        m_context->metrics().record_flow("OfferEngineBrowseOffers");
        m_context->metrics().counter("OFFER_ENGINE_BROWSE_OFFERS").increment();
        m_context->metrics().record_flow("OfferEngineEligibilityFilter");
        m_context->metrics().counter("OFFER_ENGINE_ELIGIBILITY_FILTER").increment();
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/offer_engine"}
            .add_route(
                Route{"/connectivity"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_offer_engine_connectivity_check(req, res, std::move(send));
                    })
                    .summary("Hyperswitch offer_engine_connectivity_check")
            )
            .add_route(
                Route{"/offers/list"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_offer_engine_browse_offers(req, res, std::move(send));
                    })
                    .summary("Hyperswitch offer_engine_browse_offers")
            );
    }

    explicit OfferEngineRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    [[nodiscard]] std::optional<core::error::AppError> authorize(
        const app::auth::AuthContext& auth,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        return m_context->authorizer().authorize_route(auth, rule);
    }

    [[nodiscard]] std::string dimension_scope(
        std::string_view merchant_id,
        std::string_view org_id,
        std::string_view profile_id
    ) const
    {
        std::string scope{};
        if (!merchant_id.empty()) {
            scope += "merchant:";
            scope.append(merchant_id);
        }
        if (!org_id.empty()) {
            if (!scope.empty()) {
                scope += '/';
            }
            scope += "org:";
            scope.append(org_id);
        }
        if (!profile_id.empty()) {
            if (!scope.empty()) {
                scope += '/';
            }
            scope += "profile:";
            scope.append(profile_id);
        }
        return scope;
    }

    [[nodiscard]] std::string merchant_scope(std::string_view merchant_id) const
    {
        return std::string{"merchant:"} + std::string{merchant_id};
    }

    /**
     * @brief Reads one config value through superposition (dimension-aware), then falls
     * back to ConfigStore's scoped and base keys. ConfigStore and SuperpositionClient hold
     * separate copies of the store; consulting both keeps either writer visible.
     */
    [[nodiscard]] std::optional<std::string> config_value(
        std::string_view scope,
        std::string_view key
    ) const
    {
        constexpr std::string_view kNamespace{"offer_engine"};
        if (const auto resolved = m_context->superposition().resolve(kNamespace, key, scope)) {
            return resolved->get_value();
        }
        const std::string scoped_key{std::string{key} + "@" + std::string{scope}};
        if (const auto scoped = m_context->config_store().get_value(kNamespace, scoped_key)) {
            return scoped;
        }
        return m_context->config_store().get_value(kNamespace, key);
    }

    [[nodiscard]] app::crypto::SecretString api_key(std::string_view scope) const
    {
        return app::crypto::SecretString{config_value(scope, "api_key").value_or("")};
    }

    [[nodiscard]] Source credential_source(std::string_view scope) const
    {
        const auto enabled = config_value(scope, "enabled");
        if (!enabled.has_value() || *enabled != "true") {
            return Source::None;
        }
        const auto raw = config_value(scope, "credential_source");
        if (raw.has_value() && *raw == "application") {
            return Source::Application;
        }
        if (raw.has_value() && *raw == "merchant") {
            return Source::Merchant;
        }
        return Source::None;
    }

    /** @brief OfferEngineClient::check_connectivity substitute over the connector router. */
    [[nodiscard]] ProbeResult probe_connectivity(
        std::string_view scope,
        std::string_view merchant_id
    ) const
    {
        ProbeResult result{};
        auto connector = m_context->connector_router().select(merchant_id);
        if (!connector) {
            const auto error = m_context->connector_router().last_error();
            result.reachable = false;
            result.status_code = 502;
            result.detail = error.has_value()
                ? error->get_message()
                : std::string{"offer engine probe backend is unavailable"};
            return result;
        }

        // Upstream builds the client from state.conf.offer_engine api_key/merchant_id/base_url.
        // The local connector needs no network address, but the configured base_url and
        // merchant id are still resolved and carried in the probe request for parity.
        const auto base_url = config_value("application", "base_url").value_or("");
        const auto configured_merchant = config_value("application", "merchant_id").value_or("");
        const std::string probe_target = base_url.empty() ? std::string{scope} : base_url;
        const std::string probe_merchant = merchant_id.empty() ? configured_merchant : std::string{merchant_id};
        const auto request = app::connector::ConnectorRequestBuilder{}
                                 .operation("offer_engine_connectivity_check")
                                 .merchant_id(probe_merchant)
                                 .resource_id(probe_target)
                                 .raw_payload("{}")();
        const auto response = connector->verify(request);
        const bool reachable = !response.get_retryable() && !response.get_status().empty();
        result.reachable = reachable;
        result.status_code = reachable ? 200 : 502;
        result.detail = response.get_message();
        return result;
    }

    /** @brief Deterministic eligibility decision; defaults to Eligible without a program. */
    [[nodiscard]] std::string eligibility_status(
        std::string_view scope,
        std::string_view currency
    ) const
    {
        const auto program = config_value(scope, "eligibility_program");
        if (!program.has_value() || program->empty()) {
            return "Eligible";
        }
        const std::string context_json = std::string{R"({"amount":"0","currency":)"} +
            serde::Ser::encode_json(std::string{currency}) +
            R"(,"payment_method":"","country":"","connector":""})";
        const auto decision = m_context->decision_engine().evaluate(*program, context_json);
        const auto& action = decision.get_action();
        if (action == "eligible" || action == "Eligible" || action == "allow") {
            return "Eligible";
        }
        return "Ineligible";
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

} // namespace app::handlers::offer_engine
