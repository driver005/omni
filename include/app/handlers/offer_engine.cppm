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

/**
 * Strict local port of upstream Hyperswitch `routes/offer_engine.rs` and
 * `core/offer_engine/{browse,connectivity,config,client,types}.rs`.
 *
 * The only substitution is the transport: `OfferEngineClient`'s HTTP calls to the
 * external Offer Engine are served by an in-process backend held in ConfigStore.
 * Everything else — dimension resolution, credential-source resolution, request
 * wire shape (Basic auth header, trace header, POST `v1/offers/list`, body with
 * `currency` omitted when absent), response types, eligibility filter,
 * `BrowseOffer::from` mapping, error wire shapes and persistence — follows the
 * upstream logic exactly. Evidence is recorded in each helper's comment.
 */
export namespace app::handlers::offer_engine {

inline constexpr std::string_view kOffersListPath{"v1/offers/list"};
inline constexpr std::string_view kClientNamespace{"offer_engine_client"};
inline constexpr std::string_view kRequestNamespace{"offer_engine_client_requests"};

/** @brief Port of upstream `types::OfferEngineError` (types.rs:32-46). */
class OfferEngineError
{
public:
    enum class Kind
    {
        MissingApplicationConfig,
        MissingMerchantConfig,
        RequestFailed,
        ResponseParseFailed,
    };

    OfferEngineError() = default;
    OfferEngineError(Kind kind, std::string message = {}) :
        m_kind{kind},
        m_message{std::move(message)}
    {
    }

    [[nodiscard]] Kind kind() const noexcept { return m_kind; }
    [[nodiscard]] const std::string& message() const noexcept { return m_message; }

    /** @brief Rust `Debug` rendering used by connectivity.rs:48's `{err:?}` format. */
    [[nodiscard]] std::string debug_string() const
    {
        switch (m_kind) {
        case Kind::MissingApplicationConfig:
            return std::format("MissingApplicationConfig(\"{}\")", m_message);
        case Kind::MissingMerchantConfig:
            return std::format("MissingMerchantConfig(\"{}\")", m_message);
        case Kind::RequestFailed:
            return "RequestFailed";
        case Kind::ResponseParseFailed:
            return "ResponseParseFailed";
        }
        return "RequestFailed";
    }

private:
    Kind m_kind{Kind::RequestFailed};
    std::string m_message{};
};

/** @brief Port of upstream `types::ResolvedOfferEngineConfig` (types.rs:25-30). */
struct ResolvedOfferEngineConfig
{
    std::string base_url{};
    app::crypto::SecretString api_key{};
    std::string merchant_id{};
};

/** @brief Port of upstream `client::ConnectivityResult` (client.rs:337-342). */
struct ConnectivityResult
{
    bool reachable{};
    std::optional<std::int64_t> status_code{};
    std::string detail{};
};

/** @brief Port of upstream `OfferEngineConnectivityResponse` (connectivity.rs:13-24). */
class ConnectivityResponse
{
public:
    void set_enabled(bool value) noexcept { enabled = value; }
    [[nodiscard]] const bool& get_enabled() const noexcept { return enabled; }
    void set_reachable(std::optional<bool> value) { reachable = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_reachable() const noexcept { return reachable; }
    void set_status_code(std::optional<std::int64_t> value) { status_code = value; }
    [[nodiscard]] const std::optional<std::int64_t>& get_status_code() const noexcept { return status_code; }
    void set_detail(std::string value) { detail = std::move(value); }
    [[nodiscard]] const std::string& get_detail() const noexcept { return detail; }

    bool enabled{};
    std::optional<bool> reachable{};
    std::optional<std::int64_t> status_code{};
    std::string detail{};
};

/** @brief Port of upstream `BrowseOfferListRequest` (types.rs:333-339). */
struct BrowseOfferListRequest
{
    std::optional<std::string> currency{};
};

/** @brief Port of upstream `OfferDescription` (types.rs:140-149). */
class OfferDescription
{
public:
    void set_title(std::optional<std::string> value) { title = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_title() const noexcept { return title; }
    void set_description(std::optional<std::string> value) { description = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept { return description; }
    void set_display_title(std::optional<std::string> value) { display_title = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_display_title() const noexcept { return display_title; }
    std::optional<std::string> title{};
    std::optional<std::string> description{};
    std::optional<std::string> display_title{};
};

/** @brief Port of upstream `BrowseOfferAmount` (types.rs:367-371). */
class BrowseOfferAmount
{
public:
    void set_currency(std::optional<std::string> value) { currency = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_currency() const noexcept { return currency; }
    std::optional<std::string> currency{};
};

/** @brief Port of upstream `BrowseOfferRules` (types.rs:361-365). */
class BrowseOfferRules
{
public:
    void set_amount(std::optional<BrowseOfferAmount> value) { amount = std::move(value); }
    [[nodiscard]] const std::optional<BrowseOfferAmount>& get_amount() const noexcept { return amount; }
    std::optional<BrowseOfferAmount> amount{};
};

/** @brief Port of upstream `BrowseOfferListEntry` (types.rs:348-359). */
class BrowseOfferListEntry
{
public:
    void set_status(std::string value) { status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return status; }
    void set_offer_code(std::string value) { offer_code = std::move(value); }
    [[nodiscard]] const std::string& get_offer_code() const noexcept { return offer_code; }
    void set_offer_description(std::optional<OfferDescription> value) { offer_description = std::move(value); }
    [[nodiscard]] const std::optional<OfferDescription>& get_offer_description() const noexcept { return offer_description; }
    void set_offer_rules(std::optional<BrowseOfferRules> value) { offer_rules = std::move(value); }
    [[nodiscard]] const std::optional<BrowseOfferRules>& get_offer_rules() const noexcept { return offer_rules; }
    void set_end_time_utc(std::optional<std::string> value) { end_time_utc = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_end_time_utc() const noexcept { return end_time_utc; }

    // Upstream `OfferStatus` is `#[serde(rename="ELIGIBLE")] Eligible` with
    // `#[serde(other)] Unknown` (types.rs:90-96), so only the literal "ELIGIBLE"
    // is an eligible offer; the status is kept as a string for that exact test.
    std::string status{};
    std::string offer_code{};
    std::optional<OfferDescription> offer_description{};
    std::optional<BrowseOfferRules> offer_rules{};
    std::optional<std::string> end_time_utc{};
};

/** @brief Port of upstream `BrowseOfferListResponse` (types.rs:341-346). */
class BrowseOfferListResponse
{
public:
    void set_offers(std::vector<BrowseOfferListEntry> value) { offers = std::move(value); }
    [[nodiscard]] const std::vector<BrowseOfferListEntry>& get_offers() const noexcept { return offers; }
    std::vector<BrowseOfferListEntry> offers{};
};

/** @brief Local mirror of `api_models::admin::OfferEngineMerchantConfig` (config.rs:135). */
class OfferEngineMerchantConfig
{
public:
    void set_api_key(std::string value) { api_key = std::move(value); }
    [[nodiscard]] const std::string& get_api_key() const noexcept { return api_key; }
    void set_merchant_id(std::string value) { merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return merchant_id; }
    std::string api_key{};
    std::string merchant_id{};
};

/** @brief Exact outbound microservice request shape (client.rs:31-48, 179-191, 325-335). */
class MicroserviceRequest
{
public:
    void set_method(std::string value) { method = std::move(value); }
    [[nodiscard]] const std::string& get_method() const noexcept { return method; }
    void set_path(std::string value) { path = std::move(value); }
    [[nodiscard]] const std::string& get_path() const noexcept { return path; }
    void set_base_url(std::string value) { base_url = std::move(value); }
    [[nodiscard]] const std::string& get_base_url() const noexcept { return base_url; }
    void set_authorization(std::string value) { authorization = std::move(value); }
    [[nodiscard]] const std::string& get_authorization() const noexcept { return authorization; }
    void set_trace_header_name(std::string value) { trace_header_name = std::move(value); }
    [[nodiscard]] const std::string& get_trace_header_name() const noexcept { return trace_header_name; }
    void set_merchant_id(std::string value) { merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return merchant_id; }
    void set_body(std::string value) { body = std::move(value); }
    [[nodiscard]] const std::string& get_body() const noexcept { return body; }

    std::string method{"POST"};
    std::string path{};
    std::string base_url{};
    std::string authorization{};
    std::string trace_header_name{};
    std::string merchant_id{};
    std::string body{};
};

} // namespace app::handlers::offer_engine

// ─── serde reflections (wire order matches the upstream struct field order) ───

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

template<>
struct serde::Serializable<app::handlers::offer_engine::OfferDescription>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"title", &app::handlers::offer_engine::OfferDescription::get_title, &app::handlers::offer_engine::OfferDescription::set_title, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &app::handlers::offer_engine::OfferDescription::get_description, &app::handlers::offer_engine::OfferDescription::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"display_title", &app::handlers::offer_engine::OfferDescription::get_display_title, &app::handlers::offer_engine::OfferDescription::set_display_title, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "offer_description"; }
};

template<>
struct serde::Serializable<app::handlers::offer_engine::BrowseOfferAmount>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"currency", &app::handlers::offer_engine::BrowseOfferAmount::get_currency, &app::handlers::offer_engine::BrowseOfferAmount::set_currency, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "browse_offer_amount"; }
};

template<>
struct serde::Serializable<app::handlers::offer_engine::BrowseOfferRules>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"amount", &app::handlers::offer_engine::BrowseOfferRules::get_amount, &app::handlers::offer_engine::BrowseOfferRules::set_amount, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "browse_offer_rules"; }
};

template<>
struct serde::Serializable<app::handlers::offer_engine::BrowseOfferListEntry>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"status", &app::handlers::offer_engine::BrowseOfferListEntry::get_status, &app::handlers::offer_engine::BrowseOfferListEntry::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"offer_code", &app::handlers::offer_engine::BrowseOfferListEntry::get_offer_code, &app::handlers::offer_engine::BrowseOfferListEntry::set_offer_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"offer_description", &app::handlers::offer_engine::BrowseOfferListEntry::get_offer_description, &app::handlers::offer_engine::BrowseOfferListEntry::set_offer_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"offer_rules", &app::handlers::offer_engine::BrowseOfferListEntry::get_offer_rules, &app::handlers::offer_engine::BrowseOfferListEntry::set_offer_rules, serde::FieldOptions{}>{},
            serde::FieldDesc<"end_time_utc", &app::handlers::offer_engine::BrowseOfferListEntry::get_end_time_utc, &app::handlers::offer_engine::BrowseOfferListEntry::set_end_time_utc, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "browse_offer_list_entry"; }
};

template<>
struct serde::Serializable<app::handlers::offer_engine::BrowseOfferListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"offers", &app::handlers::offer_engine::BrowseOfferListResponse::get_offers, &app::handlers::offer_engine::BrowseOfferListResponse::set_offers, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "browse_offer_list_response"; }
};

template<>
struct serde::Serializable<app::handlers::offer_engine::OfferEngineMerchantConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"api_key", &app::handlers::offer_engine::OfferEngineMerchantConfig::get_api_key, &app::handlers::offer_engine::OfferEngineMerchantConfig::set_api_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::handlers::offer_engine::OfferEngineMerchantConfig::get_merchant_id, &app::handlers::offer_engine::OfferEngineMerchantConfig::set_merchant_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "offer_engine_merchant_config"; }
};

template<>
struct serde::Serializable<app::handlers::offer_engine::MicroserviceRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"method", &app::handlers::offer_engine::MicroserviceRequest::get_method, &app::handlers::offer_engine::MicroserviceRequest::set_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"path", &app::handlers::offer_engine::MicroserviceRequest::get_path, &app::handlers::offer_engine::MicroserviceRequest::set_path, serde::FieldOptions{}>{},
            serde::FieldDesc<"base_url", &app::handlers::offer_engine::MicroserviceRequest::get_base_url, &app::handlers::offer_engine::MicroserviceRequest::set_base_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"authorization", &app::handlers::offer_engine::MicroserviceRequest::get_authorization, &app::handlers::offer_engine::MicroserviceRequest::set_authorization, serde::FieldOptions{}>{},
            serde::FieldDesc<"trace_header_name", &app::handlers::offer_engine::MicroserviceRequest::get_trace_header_name, &app::handlers::offer_engine::MicroserviceRequest::set_trace_header_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::handlers::offer_engine::MicroserviceRequest::get_merchant_id, &app::handlers::offer_engine::MicroserviceRequest::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"body", &app::handlers::offer_engine::MicroserviceRequest::get_body, &app::handlers::offer_engine::MicroserviceRequest::set_body, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "offer_engine_microservice_request"; }
};

export namespace app::handlers::offer_engine {

class OfferEngineRouteHandlers
{
    using Source = app::dto::offer_engine::OfferEngineCredentialSource;
    using Error = OfferEngineError;

public:
    // ─────────────────────────── Routes ──────────────────────────────────────

    /** @brief Port of `routes::offer_engine::offer_engine_connectivity_check` (routes:11-27). */
    void handle_offer_engine_connectivity_check(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: &auth::AdminApiAuth (routes/offer_engine.rs:23). The registry entry
        // `offer_engine_connectivity_check` carries the equivalent platform-admin rule
        // (auth/registry.cppm:244) and `authorize_route` enforces it.
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize(auth, "offer_engine_connectivity_check")) {
            send_error(res, send, *auth_error);
            return;
        }

        m_context->metrics().record_flow("OfferEngineConnectivityCheck");
        m_context->metrics().counter("OFFER_ENGINE_CONNECTIVITY_CHECK").increment();

        // Upstream `check_offer_engine_connectivity(state)` (connectivity.rs:26-72):
        // global dimensions, so no merchant/profile context is folded in.
        const Dimensions global{};
        const Source source = resolve_offer_engine_credential_source(global);

        std::optional<std::expected<ResolvedOfferEngineConfig, Error>> resolved{};
        switch (source) {
        case Source::None:
            break;
        case Source::Application:
            resolved = resolve_application_offer_config();
            break;
        case Source::Merchant:
            resolved = std::unexpected{Error{
                Error::Kind::MissingMerchantConfig,
                "credential source is 'merchant' but no merchant context is available in a global connectivity check"
            }};
            break;
        }

        ConnectivityResponse response{};
        response.set_reachable(std::nullopt);
        response.set_status_code(std::nullopt);

        if (resolved.has_value() && !resolved->has_value()) {
            // Some(Err(err)) -> detail "Offer Engine config could not be resolved: {err:?}"
            response.set_enabled(false);
            response.set_detail(std::format(
                "Offer Engine config could not be resolved: {}",
                resolved->error().debug_string()
            ));
        } else if (!resolved.has_value()) {
            // None -> exact disabled detail from connectivity.rs:54-56.
            response.set_enabled(false);
            response.set_detail(
                "Offer Engine is not enabled in global config "
                "(offer_engine.enabled is false or credential source is none)"
            );
        } else {
            // Some(Ok(config)) -> OfferEngineClient::new(...).check_connectivity()
            auto client = OfferEngineClient::new_client(*m_context, **resolved, trace_header_name());
            const auto result = client.check_connectivity();
            response.set_enabled(true);
            response.set_reachable(result.reachable);
            response.set_status_code(result.status_code);
            response.set_detail(result.detail);
        }

        send_json(res, send, serde::Ser::encode_json(response));
    }

    /** @brief Port of `routes::offer_engine::offer_engine_browse_offers` (routes:29-53). */
    void handle_offer_engine_browse_offers(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: HeaderAuth(ApiKeyAuth { allow_connected_scope_operation: true,
        // allow_platform_self_operation: false }) (routes/offer_engine.rs:46-49).
        // The registry entry `offer_engine_browse_offers` carries those flags
        // (auth/registry.cppm:243).
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize(auth, "offer_engine_browse_offers")) {
            send_error(res, send, *auth_error);
            return;
        }

        m_context->metrics().record_flow("OfferEngineBrowseOffers");
        m_context->metrics().counter("OFFER_ENGINE_BROWSE_OFFERS").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::offer_engine::BrowseOffersRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream `browse_offers(state, auth.platform, profile_id, payload)`
        // (browse.rs:22-28): dimensions come from the authenticated platform.
        Dimensions dimensions{};
        dimensions.processor_merchant_id = auth.merchant_id();
        dimensions.organization_id = auth.org_id();
        if (!auth.profile_id().empty()) {
            dimensions.profile_id = auth.profile_id();
        }

        // resolve_offer_engine_credential_source + config resolution with the
        // AccessForbidden change_context (browse.rs:36-64).
        ResolvedOfferEngineConfig config{};
        const Source source = resolve_offer_engine_credential_source(dimensions);
        switch (source) {
        case Source::None:
            send_access_forbidden(res, send);
            return;
        case Source::Application: {
            auto resolved = resolve_application_offer_config();
            if (!resolved) {
                send_access_forbidden(res, send);
                return;
            }
            config = std::move(*resolved);
            break;
        }
        case Source::Merchant: {
            auto resolved = resolve_merchant_offer_config(dimensions.processor_merchant_id);
            if (!resolved) {
                send_access_forbidden(res, send);
                return;
            }
            config = std::move(*resolved);
            break;
        }
        }

        // BrowseOfferListRequest { currency: request.offer_payment_info.map(|i| i.currency) }
        // (browse.rs:66-68).
        BrowseOfferListRequest list_request{};
        if (parsed->get_offer_payment_info().has_value()) {
            list_request.currency = parsed->get_offer_payment_info()->get_currency();
        }

        auto client = OfferEngineClient::new_client(*m_context, std::move(config), trace_header_name());
        auto client_response = client.browse_offers(list_request);
        if (!client_response) {
            // .change_context(InternalServerError).attach_printable("Offer Engine /list failed")
            // (browse.rs:71-75).
            send_internal_server_error(res, send);
            return;
        }

        // Upstream response: filter status == OfferStatus::Eligible, map BrowseOffer::from
        // (browse.rs:77-85, types.rs:373-392).
        using Response = app::dto::offer_engine::BrowseOffersResponse;
        Response typed_response{};
        std::vector<app::dto::offer_engine::BrowseOffer> offers{};
        for (const auto& entry : client_response->get_offers()) {
            if (entry.get_status() != "ELIGIBLE") {
                continue;
            }
            offers.push_back(browse_offer_from(entry));
        }
        typed_response.set_offers(std::move(offers));

        m_context->metrics().record_flow("OfferEngineEligibilityFilter");
        m_context->metrics().counter("OFFER_ENGINE_ELIGIBILITY_FILTER").increment();

        send_json(res, send, serde::Ser::encode_json(typed_response));
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Upstream scope `/offer_engine` with `/connectivity` and `/offers/list`
        // POST resources (routes/app.rs:838-847). No C++-only routes are installed.
        Router{router, "offer_engine"}
            .add_route(
                Route{"connectivity"}
                    .post([this](
                              interfaces::io::IRequest& req,
                              interfaces::io::IResponse& res,
                              std::function<void()> send
                          ) {
                        handle_offer_engine_connectivity_check(req, res, std::move(send));
                    })
                    .summary("Hyperswitch offer_engine_connectivity_check")
            )
            .add_router(Router{router, "offers"}.add_route(
                Route{"list"}
                    .post([this](
                              interfaces::io::IRequest& req,
                              interfaces::io::IResponse& res,
                              std::function<void()> send
                          ) {
                        handle_offer_engine_browse_offers(req, res, std::move(send));
                    })
                    .summary("Hyperswitch offer_engine_browse_offers")
            ));
    }

    explicit OfferEngineRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    /** @brief Local mirror of `core::configs::dimension_state::Dimensions`. */
    struct Dimensions
    {
        std::string processor_merchant_id{};
        std::string organization_id{};
        std::optional<std::string> profile_id{};

        [[nodiscard]] std::string context() const
        {
            std::string out{};
            if (!processor_merchant_id.empty()) {
                out += "merchant:" + processor_merchant_id;
            }
            if (!organization_id.empty()) {
                if (!out.empty()) {
                    out += '/';
                }
                out += "org:" + organization_id;
            }
            if (profile_id.has_value() && !profile_id->empty()) {
                if (!out.empty()) {
                    out += '/';
                }
                out += "profile:" + *profile_id;
            }
            return out;
        }
    };

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        const app::auth::AuthContext& auth,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        return m_context->authorizer().authorize_route(auth, rule);
    }

    [[nodiscard]] std::string trace_header_name() const
    {
        return config_value("", "trace_header_name").value_or("x-request-id");
    }

    /**
     * @brief Reads one dimension-aware config value. Upstream keys are
     * `offer_engine.enabled` / `offer_engine.credential_source`
     * (dimension_config.rs:1285-1322); locally the namespace is `offer_engine`
     * and the context is the dimensions string. Superposition's context override
     * wins over the base key, matching the DB + superposition lookup.
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
        return m_context->config_store().get_value(kNamespace, key);
    }

    /** @brief Port of `config::resolve_offer_engine_credential_source` (config.rs:12-47). */
    [[nodiscard]] Source resolve_offer_engine_credential_source(const Dimensions& dimensions) const
    {
        const auto enabled = config_value(dimensions.context(), "enabled");
        if (!enabled.has_value() || *enabled != "true") {
            return Source::None;
        }
        const auto raw = config_value(dimensions.context(), "credential_source");
        if (raw.has_value() && *raw == "application") {
            return Source::Application;
        }
        if (raw.has_value() && *raw == "merchant") {
            return Source::Merchant;
        }
        // Unparseable credential source defaults to None (config.rs:38-43).
        return Source::None;
    }

    /** @brief Port of `resolve_application_offer_config` (config.rs:75-108). */
    [[nodiscard]] std::expected<ResolvedOfferEngineConfig, Error>
    resolve_application_offer_config() const
    {
        const auto api_key = config_value("", "api_key");
        if (!api_key.has_value() || api_key->empty()) {
            return std::unexpected{Error{
                Error::Kind::MissingApplicationConfig,
                "offer_engine.api_key is required when credential source is application"
            }};
        }
        const auto merchant_id = config_value("", "merchant_id");
        if (!merchant_id.has_value() || merchant_id->empty()) {
            return std::unexpected{Error{
                Error::Kind::MissingApplicationConfig,
                "offer_engine.merchant_id is required when credential source is application"
            }};
        }
        ResolvedOfferEngineConfig config{};
        config.base_url = config_value("", "base_url").value_or("");
        config.api_key = app::crypto::SecretString{*api_key};
        config.merchant_id = *merchant_id;
        return config;
    }

    /**
     * @brief Port of `resolve_merchant_offer_config` (config.rs:111-147).
     *
     * Upstream reads `merchant_account.get_offer_engine_config()` (a JSON blob on the
     * merchant record). `app::store::merchant_accounts::MerchantAccountRecord` has no
     * such column, so the same JSON is persisted as a ConfigStore record in the
     * `offer_engine_merchant_config` namespace, keyed by merchant id. The parsed
     * `OfferEngineMerchantConfig` shape and every error message are upstream-exact.
     */
    [[nodiscard]] std::expected<ResolvedOfferEngineConfig, Error>
    resolve_merchant_offer_config(std::string_view merchant_id) const
    {
        ResolvedOfferEngineConfig config{};
        config.base_url = config_value("", "base_url").value_or("");

        const auto raw = m_context->config_store().get_value(
            "offer_engine_merchant_config", merchant_id);
        if (!raw.has_value()) {
            return std::unexpected{Error{
                Error::Kind::MissingMerchantConfig,
                "merchant Offer Engine config is not set on the merchant account"
            }};
        }
        auto parsed = serde::Ser::deserialize<OfferEngineMerchantConfig>("application/json", *raw);
        if (!parsed) {
            return std::unexpected{Error{
                Error::Kind::MissingMerchantConfig,
                "merchant Offer Engine config is invalid"
            }};
        }
        config.api_key = app::crypto::SecretString{parsed->get_api_key()};
        config.merchant_id = parsed->get_merchant_id();
        return config;
    }

    /** @brief Local port of `OfferEngineClient` (client.rs:23-151). */
    class OfferEngineClient
    {
    public:
        static OfferEngineClient new_client(
            app::context::AppContext& context,
            ResolvedOfferEngineConfig config,
            std::string_view trace_header_name
        )
        {
            OfferEngineClient client{};
            client.m_context = &context;
            client.m_base_url = std::move(config.base_url);
            client.m_merchant_id = std::move(config.merchant_id);
            client.m_trace_header_name = std::string{trace_header_name};
            // OfferEngineClient::new (client.rs:32-48): Basic base64("{api_key}:").
            client.m_authorization = "Basic " + base64_encode(config.api_key.value() + ":");
            return client;
        }

        /**
         * @brief Port of `OfferEngineClient::browse_offers` + `OfferBrowseList` flow
         * (client.rs:65-74, 301-335): POST `v1/offers/list` with the serialized
         * `BrowseOfferListRequest` body. The in-process backend is the local
         * substitute for the external network call.
         */
        [[nodiscard]] std::expected<BrowseOfferListResponse, Error>
        browse_offers(const BrowseOfferListRequest& request) const
        {
            MicroserviceRequest wire{};
            wire.set_path(std::string{kOffersListPath});
            wire.set_base_url(m_base_url);
            wire.set_authorization(m_authorization);
            wire.set_trace_header_name(m_trace_header_name);
            wire.set_merchant_id(m_merchant_id);
            // BrowseOfferListRequest: `currency` skipped when None (types.rs:335-339).
            wire.set_body(request.currency.has_value()
                ? std::format(R"({{"currency":{}}})", serde::Ser::encode_json(*request.currency))
                : std::string{"{}"});
            record_request(wire);

            const auto stored = m_context->config_store().get_value(
                kClientNamespace,
                m_merchant_id + ":" + std::string{kOffersListPath}
            );
            if (!stored.has_value()) {
                return std::unexpected{Error{Error::Kind::RequestFailed}};
            }
            auto parsed = serde::Ser::deserialize<BrowseOfferListResponse>(
                "application/json", *stored);
            if (!parsed) {
                return std::unexpected{Error{Error::Kind::ResponseParseFailed}};
            }
            return *parsed;
        }

        /**
         * @brief Port of `OfferEngineClient::check_connectivity` (client.rs:98-129).
         * The local backend yields one of: "ok" / "deserialize" (reached),
         * "upstream:<status>" (HTTP status, 401/403 = auth failed), "transport:<msg>"
         * (unreachable). This reproduces the exact `MicroserviceClientErrorKind`
         * match and detail strings of upstream.
         */
        [[nodiscard]] ConnectivityResult check_connectivity() const
        {
            MicroserviceRequest wire{};
            wire.set_path(std::string{kOffersListPath});
            wire.set_base_url(m_base_url);
            wire.set_authorization(m_authorization);
            wire.set_trace_header_name(m_trace_header_name);
            wire.set_merchant_id(m_merchant_id);
            wire.set_body("{}"); // OfferConnectivity body = serde_json::json!({}) (client.rs:188-190)
            record_request(wire);

            const auto stored = m_context->config_store().get_value(
                kClientNamespace,
                m_merchant_id + ":" + std::string{kOffersListPath} + ":connectivity"
            );
            if (!stored.has_value()) {
                return connectivity_from_token("transport:local backend unavailable");
            }
            return connectivity_from_token(*stored);
        }

    private:
        void record_request(const MicroserviceRequest& wire) const
        {
            m_context->config_store().put(
                kRequestNamespace, m_merchant_id, serde::Ser::encode_json(wire));
        }

        [[nodiscard]] static ConnectivityResult connectivity_from_token(std::string_view token)
        {
            ConnectivityResult result{};
            if (token == "ok" || token == "deserialize") {
                result.reachable = true;
                result.detail = "Reached Offer Engine";
                return result;
            }
            if (token.starts_with("upstream:")) {
                const std::string raw{token.substr(9)};
                const auto status = parse_status(raw);
                result.reachable = true;
                if (status.has_value()) {
                    result.status_code = *status;
                }
                if (status == 401 || status == 403) {
                    result.detail = "Reached Offer Engine, but authentication failed";
                } else {
                    result.detail = status.has_value()
                        ? std::format("Reached Offer Engine (HTTP {})", *status)
                        : std::format("Reached Offer Engine (HTTP {})", raw);
                }
                return result;
            }
            if (token.starts_with("transport:")) {
                result.reachable = false;
                result.detail = std::format("Could not reach Offer Engine: {}", token.substr(10));
                return result;
            }
            result.reachable = false;
            result.detail = std::format("Could not reach Offer Engine: {}", token);
            return result;
        }

        [[nodiscard]] static std::optional<int> parse_status(std::string_view text)
        {
            int value{};
            auto result = std::from_chars(text.data(), text.data() + text.size(), value);
            if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
                return std::nullopt;
            }
            return value;
        }

        /** @brief Standard base64 (client.rs:33-36 uses BASE64_ENGINE.encode). */
        [[nodiscard]] static std::string base64_encode(std::string_view input)
        {
            static constexpr std::string_view alphabet =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            std::string out{};
            out.reserve(((input.size() + 2) / 3) * 4);
            std::size_t index{};
            while (index + 3 <= input.size()) {
                const auto chunk = (static_cast<unsigned char>(input[index]) << 16) |
                                   (static_cast<unsigned char>(input[index + 1]) << 8) |
                                   static_cast<unsigned char>(input[index + 2]);
                out.push_back(alphabet[(chunk >> 18) & 0x3F]);
                out.push_back(alphabet[(chunk >> 12) & 0x3F]);
                out.push_back(alphabet[(chunk >> 6) & 0x3F]);
                out.push_back(alphabet[chunk & 0x3F]);
                index += 3;
            }
            const std::size_t remaining = input.size() - index;
            if (remaining == 1) {
                const auto chunk = static_cast<unsigned char>(input[index]) << 16;
                out.push_back(alphabet[(chunk >> 18) & 0x3F]);
                out.push_back(alphabet[(chunk >> 12) & 0x3F]);
                out += "==";
            } else if (remaining == 2) {
                const auto chunk = (static_cast<unsigned char>(input[index]) << 16) |
                                   (static_cast<unsigned char>(input[index + 1]) << 8);
                out.push_back(alphabet[(chunk >> 18) & 0x3F]);
                out.push_back(alphabet[(chunk >> 12) & 0x3F]);
                out.push_back(alphabet[(chunk >> 6) & 0x3F]);
                out += '=';
            }
            return out;
        }

        app::context::AppContext* m_context{nullptr};
        std::string m_base_url{};
        std::string m_authorization{};
        std::string m_trace_header_name{};
        std::string m_merchant_id{};
    };

    /** @brief Exact `From<BrowseOfferListEntry> for BrowseOffer` (types.rs:373-392). */
    [[nodiscard]] app::dto::offer_engine::BrowseOffer
    browse_offer_from(const BrowseOfferListEntry& entry) const
    {
        using Offer = app::dto::offer_engine::BrowseOffer;
        Offer offer{};
        offer.set_code(entry.get_offer_code());

        std::optional<std::string> title{};
        std::optional<std::string> description{};
        std::optional<std::string> display_title{};
        if (entry.get_offer_description().has_value()) {
            title = entry.get_offer_description()->get_title();
            description = entry.get_offer_description()->get_description();
            display_title = entry.get_offer_description()->get_display_title();
        }
        offer.set_title(title);
        offer.set_display_title(display_title);
        offer.set_description(description);

        std::optional<std::string> currency{};
        if (entry.get_offer_rules().has_value() &&
            entry.get_offer_rules()->get_amount().has_value()) {
            currency = entry.get_offer_rules()->get_amount()->get_currency();
        }
        offer.set_currency(currency);
        offer.set_valid_till(entry.get_end_time_utc());
        return offer;
    }

    /** @brief Exact `ApiErrorResponse::AccessForbidden` wire shape (api_error_response.rs:643-646). */
    void send_access_forbidden(
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) const
    {
        constexpr std::string_view body =
            R"({"type":"invalid_request","message":"Access forbidden. Not authorized to access this resource offer_engine","code":"IR_22"})";
        auto bytes = serde::Ser::serialize_raw("application/json", body);
        res.set_status(interfaces::io::types::Status::FORBIDDEN);
        res.set_body(std::move(bytes));
        send();
    }

    /** @brief Exact `ApiErrorResponse::InternalServerError` wire shape (api_error_response.rs:422-424). */
    void send_internal_server_error(
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) const
    {
        constexpr std::string_view body =
            R"({"type":"api","message":"Something went wrong","code":"HE_00"})";
        auto bytes = serde::Ser::serialize_raw("application/json", body);
        res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
        res.set_body(std::move(bytes));
        send();
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
        std::string_view text
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::offer_engine
