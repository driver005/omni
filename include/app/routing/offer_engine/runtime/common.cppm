export module app_routing:offer_engine_runtime_common;

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

import :offer_engine_input;

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

/** @brief Local mirror of `core::configs::dimension_state::Dimensions`. */
struct OfferEngineDimensions
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

    [[nodiscard]] Kind kind() const noexcept
    {
        return m_kind;
    }

    [[nodiscard]] const std::string& message() const noexcept
    {
        return m_message;
    }

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
    ::app::crypto::SecretString api_key{};
    std::string merchant_id{};
};

/** @brief Port of upstream `client::ConnectivityResult` (client.rs:337-342). */
struct ConnectivityResult
{
    bool reachable{};
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
    void set_title(std::optional<std::string> value)
    {
        title = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_title() const noexcept
    {
        return title;
    }

    void set_description(std::optional<std::string> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept
    {
        return description;
    }

    void set_display_title(std::optional<std::string> value)
    {
        display_title = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_display_title() const noexcept
    {
        return display_title;
    }

    std::optional<std::string> title{};
    std::optional<std::string> description{};
    std::optional<std::string> display_title{};
};

/** @brief Port of upstream `BrowseOfferAmount` (types.rs:367-371). */
class BrowseOfferAmount
{
public:
    void set_currency(std::optional<std::string> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_currency() const noexcept
    {
        return currency;
    }

    std::optional<std::string> currency{};
};

/** @brief Port of upstream `BrowseOfferRules` (types.rs:361-365). */
class BrowseOfferRules
{
public:
    void set_amount(std::optional<BrowseOfferAmount> value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const std::optional<BrowseOfferAmount>& get_amount() const noexcept
    {
        return amount;
    }

    std::optional<BrowseOfferAmount> amount{};
};

/** @brief Port of upstream `BrowseOfferListEntry` (types.rs:348-359). */
class BrowseOfferListEntry
{
public:
    void set_status(std::string value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return status;
    }

    void set_offer_code(std::string value)
    {
        offer_code = std::move(value);
    }

    [[nodiscard]] const std::string& get_offer_code() const noexcept
    {
        return offer_code;
    }

    void set_offer_description(std::optional<OfferDescription> value)
    {
        offer_description = std::move(value);
    }

    [[nodiscard]] const std::optional<OfferDescription>& get_offer_description() const noexcept
    {
        return offer_description;
    }

    void set_offer_rules(std::optional<BrowseOfferRules> value)
    {
        offer_rules = std::move(value);
    }

    [[nodiscard]] const std::optional<BrowseOfferRules>& get_offer_rules() const noexcept
    {
        return offer_rules;
    }

    void set_end_time_utc(std::optional<std::string> value)
    {
        end_time_utc = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_end_time_utc() const noexcept
    {
        return end_time_utc;
    }

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
    void set_offers(std::vector<BrowseOfferListEntry> value)
    {
        offers = std::move(value);
    }

    [[nodiscard]] const std::vector<BrowseOfferListEntry>& get_offers() const noexcept
    {
        return offers;
    }

    std::vector<BrowseOfferListEntry> offers{};
};

/** @brief Local mirror of `api_models::admin::OfferEngineMerchantConfig` (config.rs:135). */
class OfferEngineMerchantConfig
{
public:
    void set_api_key(std::string value)
    {
        api_key = std::move(value);
    }

    [[nodiscard]] const std::string& get_api_key() const noexcept
    {
        return api_key;
    }

    void set_merchant_id(std::string value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    std::string api_key{};
    std::string merchant_id{};
};

/** @brief Exact outbound microservice request shape (client.rs:31-48, 179-191, 325-335). */
class MicroserviceRequest
{
public:
    void set_method(std::string value)
    {
        method = std::move(value);
    }

    [[nodiscard]] const std::string& get_method() const noexcept
    {
        return method;
    }

    void set_path(std::string value)
    {
        path = std::move(value);
    }

    [[nodiscard]] const std::string& get_path() const noexcept
    {
        return path;
    }

    void set_base_url(std::string value)
    {
        base_url = std::move(value);
    }

    [[nodiscard]] const std::string& get_base_url() const noexcept
    {
        return base_url;
    }

    void set_authorization(std::string value)
    {
        authorization = std::move(value);
    }

    [[nodiscard]] const std::string& get_authorization() const noexcept
    {
        return authorization;
    }

    void set_trace_header_name(std::string value)
    {
        trace_header_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_trace_header_name() const noexcept
    {
        return trace_header_name;
    }

    void set_merchant_id(std::string value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_body(std::string value)
    {
        body = std::move(value);
    }

    [[nodiscard]] const std::string& get_body() const noexcept
    {
        return body;
    }

    std::string method{"POST"};
    std::string path{};
    std::string base_url{};
    std::string authorization{};
    std::string trace_header_name{};
    std::string merchant_id{};
    std::string body{};
};

/** @brief Local port of `OfferEngineClient` (client.rs:23-151). */
class OfferEngineClient
{
public:
    OfferEngineClient(
        ::app::context::AppContext& context,
        ResolvedOfferEngineConfig config,
        std::string_view trace_header_name
    ) noexcept :
        m_context{context},
        m_base_url{std::move(config.base_url)},
        // OfferEngineClient::new (client.rs:32-48): Basic base64("{api_key}:").
        m_authorization{"Basic " + base64_encode(config.api_key.value() + ":")},
        m_trace_header_name{std::string{trace_header_name}},
        m_merchant_id{std::move(config.merchant_id)}
    {
    }

    /**
     * @brief Port of `OfferEngineClient::browse_offers` + `OfferBrowseList` flow
     * (client.rs:65-74, 301-335): POST `v1/offers/list` with the serialized
     * `BrowseOfferListRequest` body. The in-process backend is the local
     * substitute for the external network call.
     */
    [[nodiscard]] std::expected<BrowseOfferListResponse, OfferEngineError>
    browse_offers(const BrowseOfferListRequest& request) const
    {
        MicroserviceRequest wire{};
        wire.set_path(std::string{kOffersListPath});
        wire.set_base_url(m_base_url);
        wire.set_authorization(m_authorization);
        wire.set_trace_header_name(m_trace_header_name);
        wire.set_merchant_id(m_merchant_id);
        // BrowseOfferListRequest: `currency` skipped when None (types.rs:335-339).
        // Body is a typed document serialized by the JSON plugin, never hand-built text.
        wire.set_body(browse_list_body(request));
        record_request(wire);

        const auto stored = m_context.config_store().get_value(
            kClientNamespace,
            m_merchant_id + ":" + std::string{kOffersListPath}
        );
        if (!stored.has_value()) {
            return std::unexpected{OfferEngineError{OfferEngineError::Kind::RequestFailed}};
        }
        auto parsed = serde::Ser::deserialize<BrowseOfferListResponse>("application/json", *stored);
        if (!parsed) {
            return std::unexpected{OfferEngineError{OfferEngineError::Kind::ResponseParseFailed}};
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

        const auto stored = m_context.config_store().get_value(
            kClientNamespace,
            m_merchant_id + ":" + std::string{kOffersListPath} + ":connectivity"
        );
        if (!stored.has_value()) {
            return connectivity_from_token("transport:local backend unavailable");
        }
        return connectivity_from_token(*stored);
    }

private:
    /** @brief Typed `BrowseOfferListRequest` wire body: `{"currency":...}` or `{}`. */
    [[nodiscard]] std::string browse_list_body(const BrowseOfferListRequest& request) const
    {
        serde::Value::Object body{};
        if (request.currency.has_value()) {
            body.insert(std::string{"currency"}, serde::Value{*request.currency});
        }
        const auto bytes = serde::Ser::serialize("application/json", serde::Value{std::move(body)});
        return std::string{reinterpret_cast<const char*>(bytes.data()), bytes.size()};
    }

    void record_request(const MicroserviceRequest& wire) const
    {
        m_context.config_store()
            .put(kRequestNamespace, m_merchant_id, serde::Ser::encode_json(wire));
    }

    [[nodiscard]] ConnectivityResult connectivity_from_token(std::string_view token) const
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

    [[nodiscard]] std::optional<int> parse_status(std::string_view text) const
    {
        int value{};
        auto result = std::from_chars(text.data(), text.data() + text.size(), value);
        if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
            return std::nullopt;
        }
        return value;
    }

    /** @brief Standard base64 (client.rs:33-36 uses BASE64_ENGINE.encode). */
    [[nodiscard]] std::string base64_encode(std::string_view input) const
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

    ::app::context::AppContext& m_context;
    std::string m_base_url{};
    std::string m_authorization{};
    std::string m_trace_header_name{};
    std::string m_merchant_id{};
};

/** @brief Shared business helpers for the two offer_engine sub-runtimes. */
class OfferEngineRuntimeSupport
{
public:
    explicit OfferEngineRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
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
    [[nodiscard]] std::optional<std::string>
    config_value(std::string_view scope, std::string_view key) const
    {
        constexpr std::string_view kNamespace{"offer_engine"};
        if (const auto resolved = m_context.superposition().resolve(kNamespace, key, scope)) {
            return resolved->get_value();
        }
        return m_context.config_store().get_value(kNamespace, key);
    }

    /** @brief Port of `config::resolve_offer_engine_credential_source` (config.rs:12-47). */
    [[nodiscard]] ::app::dto::offer_engine::OfferEngineCredentialSource
    resolve_offer_engine_credential_source(const OfferEngineDimensions& dimensions) const
    {
        using Source = ::app::dto::offer_engine::OfferEngineCredentialSource;
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
    [[nodiscard]] std::expected<ResolvedOfferEngineConfig, OfferEngineError>
    resolve_application_offer_config() const
    {
        const auto api_key = config_value("", "api_key");
        if (!api_key.has_value() || api_key->empty()) {
            return std::unexpected{OfferEngineError{
                OfferEngineError::Kind::MissingApplicationConfig,
                "offer_engine.api_key is required when credential source is application"
            }};
        }
        const auto merchant_id = config_value("", "merchant_id");
        if (!merchant_id.has_value() || merchant_id->empty()) {
            return std::unexpected{OfferEngineError{
                OfferEngineError::Kind::MissingApplicationConfig,
                "offer_engine.merchant_id is required when credential source is application"
            }};
        }
        ResolvedOfferEngineConfig config{};
        config.base_url = config_value("", "base_url").value_or("");
        config.api_key = ::app::crypto::SecretString{*api_key};
        config.merchant_id = *merchant_id;
        return config;
    }

    /**
     * @brief Port of `resolve_merchant_offer_config` (config.rs:111-147).
     *
     * Upstream reads `merchant_account.get_offer_engine_config()` (a JSON blob on the
     * merchant record). `::app::store::merchant_accounts::MerchantAccountRecord` has no
     * such column, so the same JSON is persisted as a ConfigStore record in the
     * `offer_engine_merchant_config` namespace, keyed by merchant id. The parsed
     * `OfferEngineMerchantConfig` shape and every error message are upstream-exact.
     */
    [[nodiscard]] std::expected<ResolvedOfferEngineConfig, OfferEngineError>
    resolve_merchant_offer_config(std::string_view merchant_id) const
    {
        ResolvedOfferEngineConfig config{};
        config.base_url = config_value("", "base_url").value_or("");

        const auto raw =
            m_context.config_store().get_value("offer_engine_merchant_config", merchant_id);
        if (!raw.has_value()) {
            return std::unexpected{OfferEngineError{
                OfferEngineError::Kind::MissingMerchantConfig,
                "merchant Offer Engine config is not set on the merchant account"
            }};
        }
        auto parsed = serde::Ser::deserialize<OfferEngineMerchantConfig>("application/json", *raw);
        if (!parsed) {
            return std::unexpected{OfferEngineError{
                OfferEngineError::Kind::MissingMerchantConfig,
                "merchant Offer Engine config is invalid"
            }};
        }
        config.api_key = ::app::crypto::SecretString{parsed->get_api_key()};
        config.merchant_id = parsed->get_merchant_id();
        return config;
    }

    /** @brief Exact `From<BrowseOfferListEntry> for BrowseOffer` (types.rs:373-392). */
    [[nodiscard]] ::app::dto::offer_engine::BrowseOffer
    browse_offer_from(const BrowseOfferListEntry& entry) const
    {
        using Offer = ::app::dto::offer_engine::BrowseOffer;
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

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::offer_engine

// ─── serde reflections (wire order matches the upstream struct field order) ───

template<>
struct serde::Serializable<::app::handlers::offer_engine::OfferDescription>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::offer_engine::OfferDescription;
        return std::tuple{
            serde::FieldDesc<"title", &T::get_title, &T::set_title, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &T::get_description,
                &T::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "display_title",
                &T::get_display_title,
                &T::set_display_title,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "offer_description";
    }
};

template<>
struct serde::Serializable<::app::handlers::offer_engine::BrowseOfferAmount>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::offer_engine::BrowseOfferAmount;
        return std::tuple{
            serde::
                FieldDesc<"currency", &T::get_currency, &T::set_currency, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "browse_offer_amount";
    }
};

template<>
struct serde::Serializable<::app::handlers::offer_engine::BrowseOfferRules>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::offer_engine::BrowseOfferRules;
        return std::tuple{
            serde::FieldDesc<"amount", &T::get_amount, &T::set_amount, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "browse_offer_rules";
    }
};

template<>
struct serde::Serializable<::app::handlers::offer_engine::BrowseOfferListEntry>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::offer_engine::BrowseOfferListEntry;
        return std::tuple{
            serde::FieldDesc<"status", &T::get_status, &T::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offer_code",
                &T::get_offer_code,
                &T::set_offer_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offer_description",
                &T::get_offer_description,
                &T::set_offer_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offer_rules",
                &T::get_offer_rules,
                &T::set_offer_rules,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "end_time_utc",
                &T::get_end_time_utc,
                &T::set_end_time_utc,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "browse_offer_list_entry";
    }
};

template<>
struct serde::Serializable<::app::handlers::offer_engine::BrowseOfferListResponse>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::offer_engine::BrowseOfferListResponse;
        return std::tuple{
            serde::FieldDesc<"offers", &T::get_offers, &T::set_offers, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "browse_offer_list_response";
    }
};

template<>
struct serde::Serializable<::app::handlers::offer_engine::OfferEngineMerchantConfig>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::offer_engine::OfferEngineMerchantConfig;
        return std::tuple{
            serde::FieldDesc<"api_key", &T::get_api_key, &T::set_api_key, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &T::get_merchant_id,
                &T::set_merchant_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "offer_engine_merchant_config";
    }
};

template<>
struct serde::Serializable<::app::handlers::offer_engine::MicroserviceRequest>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::offer_engine::MicroserviceRequest;
        return std::tuple{
            serde::FieldDesc<"method", &T::get_method, &T::set_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"path", &T::get_path, &T::set_path, serde::FieldOptions{}>{},
            serde::
                FieldDesc<"base_url", &T::get_base_url, &T::set_base_url, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authorization",
                &T::get_authorization,
                &T::set_authorization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "trace_header_name",
                &T::get_trace_header_name,
                &T::set_trace_header_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &T::get_merchant_id,
                &T::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"body", &T::get_body, &T::set_body, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "offer_engine_microservice_request";
    }
};
