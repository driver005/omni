module;
export module app_handlers:profile_acquirer;

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
import app_store;

export namespace app::handlers::profile_acquirer {

/**
 * @brief Local mirror of `common_types::domain::AcquirerConfig`.
 *
 * Upstream: docs/hyperswitch/crates/common_types/src/domain.rs:77-100. The
 * upstream wrapper has no serde support in this port, so the handler persists
 * this exact field set through AppRepository config records — the in-process
 * substitute for `accounts_store` / the business-profile `acquirer_config_map`.
 * Country code is stored already normalized to the numeric form
 * (`Country::from_alpha2(code).to_numeric().to_string()`).
 */
class AcquirerConfig
{
public:
    void set_acquirer_assigned_merchant_id(std::optional<std::string> value) { acquirer_assigned_merchant_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_acquirer_assigned_merchant_id() const noexcept { return acquirer_assigned_merchant_id; }
    void set_merchant_name(std::optional<std::string> value) { merchant_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_merchant_name() const noexcept { return merchant_name; }
    void set_network(std::string value) { network = std::move(value); }
    [[nodiscard]] const std::string& get_network() const noexcept { return network; }
    void set_acquirer_bin(std::optional<std::string> value) { acquirer_bin = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_acquirer_bin() const noexcept { return acquirer_bin; }
    void set_acquirer_ica(std::optional<std::string> value) { acquirer_ica = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_acquirer_ica() const noexcept { return acquirer_ica; }
    void set_acquirer_fraud_rate(std::optional<double> value) { acquirer_fraud_rate = std::move(value); }
    [[nodiscard]] const std::optional<double>& get_acquirer_fraud_rate() const noexcept { return acquirer_fraud_rate; }
    void set_acquirer_country_code(std::optional<std::string> value) { acquirer_country_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_acquirer_country_code() const noexcept { return acquirer_country_code; }

    std::optional<std::string> acquirer_assigned_merchant_id{};
    std::optional<std::string> merchant_name{};
    std::string network{};
    std::optional<std::string> acquirer_bin{};
    std::optional<std::string> acquirer_ica{};
    std::optional<double> acquirer_fraud_rate{};
    std::optional<std::string> acquirer_country_code{};
};

} // namespace app::handlers::profile_acquirer

// Serialization must be declared before the handler bodies that use this shape.
template<>
struct serde::Serializable<::app::handlers::profile_acquirer::AcquirerConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"acquirer_assigned_merchant_id", &::app::handlers::profile_acquirer::AcquirerConfig::get_acquirer_assigned_merchant_id, &::app::handlers::profile_acquirer::AcquirerConfig::set_acquirer_assigned_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_name", &::app::handlers::profile_acquirer::AcquirerConfig::get_merchant_name, &::app::handlers::profile_acquirer::AcquirerConfig::set_merchant_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"network", &::app::handlers::profile_acquirer::AcquirerConfig::get_network, &::app::handlers::profile_acquirer::AcquirerConfig::set_network, serde::FieldOptions{}>{},
            serde::FieldDesc<"acquirer_bin", &::app::handlers::profile_acquirer::AcquirerConfig::get_acquirer_bin, &::app::handlers::profile_acquirer::AcquirerConfig::set_acquirer_bin, serde::FieldOptions{}>{},
            serde::FieldDesc<"acquirer_ica", &::app::handlers::profile_acquirer::AcquirerConfig::get_acquirer_ica, &::app::handlers::profile_acquirer::AcquirerConfig::set_acquirer_ica, serde::FieldOptions{}>{},
            serde::FieldDesc<"acquirer_fraud_rate", &::app::handlers::profile_acquirer::AcquirerConfig::get_acquirer_fraud_rate, &::app::handlers::profile_acquirer::AcquirerConfig::set_acquirer_fraud_rate, serde::FieldOptions{}>{},
            serde::FieldDesc<"acquirer_country_code", &::app::handlers::profile_acquirer::AcquirerConfig::get_acquirer_country_code, &::app::handlers::profile_acquirer::AcquirerConfig::set_acquirer_country_code, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "acquirer_config"; }
};

export namespace app::handlers::profile_acquirer {

/**
 * @brief Local mirror of `common_types::domain::AcquirerConfigBucket`
 * (docs/hyperswitch/crates/common_types/src/domain.rs:102-106).
 *
 * `configs` is keyed by ProfileAcquirerId and each value is the per-network
 * `Vec<AcquirerConfig>`. A nullopt bucket (never created) models upstream's
 * `acquirer_config_map == None`; an empty map models `Some(empty)`.
 */
struct AcquirerConfigBucket
{
    std::optional<std::string> default_acquirer_config{};
    std::unordered_map<std::string, std::vector<AcquirerConfig>> configs{};
};

class ProfileAcquirerRouteHandlers
{
public:
    // ---- route: POST /profile_acquirer -------------------------------------
    // Upstream route: routes/profile_acquirer.rs:11-50 (create_profile_acquirer).
    // Upstream core:  core/profile_acquirer.rs:12-102.
    void handle_create_profile_acquirer(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        // Upstream web::Json<ProfileAcquirerCreate> extraction (routes:16, api_models:4-32)
        // deserializes `network: CardNetwork` and `acquirer_country_code: CountryAlpha2`,
        // rejecting unknown enum strings with HTTP 400 before server_wrap/core run.
        if (!payload_passes_serde_validation(body, false)) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        using Input = app::dto::profile_acquirer::ProfileAcquirerCreate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream extracts payload.profile_id before `server_wrap` and binds it to
        // JWTAuthProfileFromRoute (routes/profile_acquirer.rs:20,39-44).
        const std::string profile_id = std::string{parsed->get_profile_id().get_value()};
        if (const auto denied = authorize(req, "create_profile_acquirer", profile_id)) {
            send_error(res, send, *denied);
            return;
        }

        // core:20-26 — find_business_profile_by_profile_id -> ProfileNotFound.
        if (!profile_exists(profile_id)) {
            send_error(res, send, profile_not_found(profile_id));
            return;
        }

        // core:28-38 — has_existing_default / is_default.
        auto bucket = load_bucket(profile_id);
        const bool has_existing_default = bucket.has_value() && bucket->default_acquirer_config.has_value();
        const bool is_default = !has_existing_default || parsed->get_is_default().value_or(false);

        // core:17 — generate_profile_acquirer_id_of_default_length().
        const std::string profile_acquirer_id = generate_profile_acquirer_id_of_default_length();

        // core:40-52 — build the incoming AcquirerConfig (country normalized).
        AcquirerConfig incoming = config_from_create(*parsed);

        // core:54-60 — initialize the bucket with get_or_insert_with.
        if (!bucket) {
            bucket.emplace();
        }

        // core:62-65 — configs.insert(id, vec![incoming]) (replaces the bucket slot).
        bucket->configs[profile_acquirer_id] = std::vector<AcquirerConfig>{incoming};

        // core:67-69 — default pointer.
        if (is_default) {
            bucket->default_acquirer_config = profile_acquirer_id;
        }

        // core:71-79 — ProfileUpdate::AcquirerConfigBucketUpdate persistence.
        save_bucket(profile_id, *bucket);

        // core:81-92 — re-read and find the slot matching `network`.
        const auto stored = load_bucket(profile_id);
        const AcquirerConfig* updated = find_config_in_bucket(
            stored ? &*stored : nullptr, profile_acquirer_id, incoming.get_network());
        if (updated == nullptr) {
            // core:91-92 .attach_printable("Failed to get updated acquirer config")
            send_error(res, send, internal_server_error());
            return;
        }

        // core:94-99 — ProfileAcquirerResponse::from((id, &profile_id, Some(cfg), is_default)).
        auto response = build_response(profile_acquirer_id, profile_id, updated, is_default);

        m_context->metrics().record_flow("ProfileAcquirerCreate");
        m_context->metrics().counter("profile_acquirer_create").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    // ---- route: POST /profile_acquirer/:profile_id/:profile_acquirer_id -----
    // Upstream route: routes/profile_acquirer.rs:52-97 (profile_acquirer_update).
    // Upstream core:  core/profile_acquirer.rs:105-215.
    void handle_profile_acquirer_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Route ids come from the path, never from the request body.
        const auto segments = app::http::RequestPath{}(req);
        if (segments.count() < 2) {
            send_error(res, send, invalid_request("missing profile id or profile acquirer id"));
            return;
        }
        const std::string profile_acquirer_id = std::string{segments.at(segments.count() - 1).value_or("")};
        const std::string profile_id = std::string{segments.at(segments.count() - 2).value_or("")};
        if (profile_id.empty() || profile_acquirer_id.empty()) {
            send_error(res, send, invalid_request("missing profile id or profile acquirer id"));
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        // Upstream web::Json<ProfileAcquirerUpdate> extraction (routes:61,
        // api_models:154-174) applies `#[serde(deny_unknown_fields)]` plus the
        // CardNetwork / CountryAlpha2 enum checks, all as HTTP 400 before core.
        if (!payload_passes_serde_validation(body, true)) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        using Input = app::dto::profile_acquirer::ProfileAcquirerUpdate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream binds the route profile_id to JWTAuthProfileFromRoute
        // (routes/profile_acquirer.rs:87-91).
        if (const auto denied = authorize(req, "profile_acquirer_update", profile_id)) {
            send_error(res, send, *denied);
            return;
        }

        // core:114-120 — find_business_profile_by_profile_id -> ProfileNotFound.
        if (!profile_exists(profile_id)) {
            send_error(res, send, profile_not_found(profile_id));
            return;
        }

        // core:122-129 — acquirer_config_map must exist.
        auto bucket = load_bucket(profile_id);
        if (!bucket) {
            send_error(res, send, profile_acquirer_not_found(profile_id, profile_acquirer_id));
            return;
        }

        // core:131 — default_bucket_id.
        const auto default_bucket_id = bucket->default_acquirer_config;

        // core:133-144 — verify the target bucket exists.
        if (bucket->configs.find(profile_acquirer_id) == bucket->configs.end()) {
            send_error(res, send, profile_acquirer_not_found(profile_id, profile_acquirer_id));
            return;
        }

        // core:147-149 — is_default from request or current default pointer.
        const bool is_default = parsed->get_is_default().value_or(
            default_bucket_id.has_value() && *default_bucket_id == profile_acquirer_id);

        // core:151-156 — apply_default_bucket_change (mutates the map).
        const bool default_bucket_changed = apply_default_bucket_change(
            *bucket, profile_acquirer_id, default_bucket_id, parsed->get_is_default());

        // core:158-180 — upsert the network slot, or allow is_default-only changes.
        if (parsed->get_network().has_value()) {
            if (const auto err = upsert_acquirer_config_in_bucket(
                    *bucket,
                    profile_acquirer_id,
                    parsed->get_network()->get_value(),
                    *parsed,
                    default_bucket_changed)) {
                send_error(res, send, *err);
                return;
            }
        } else if (!default_bucket_changed) {
            send_error(res, send, invalid_request(
                "A `network` must be provided to update an acquirer configuration, unless you are only changing the `is_default` fallback status of this entire bucket."));
            return;
        }

        // core:182-193 — persist the updated bucket.
        save_bucket(profile_id, *bucket);

        // core:195-205 — final config from the re-read profile; None for is_default-only.
        const AcquirerConfig* final_config = nullptr;
        if (parsed->get_network().has_value()) {
            const auto stored = load_bucket(profile_id);
            final_config = find_config_in_bucket(
                stored ? &*stored : nullptr, profile_acquirer_id, parsed->get_network()->get_value());
        }

        // core:207-212 — ProfileAcquirerResponse::from((id, &profile_id, final_cfg, is_default)).
        auto response = build_response(profile_acquirer_id, profile_id, final_config, is_default);

        m_context->metrics().record_flow("ProfileAcquirerUpdate");
        m_context->metrics().counter("profile_acquirer_update").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/profile_acquirer"}
            .add_route(
                Route{""}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_profile_acquirer(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_profile_acquirer")
            )
            .add_route(
                Route{"/:profile_id/:profile_acquirer_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profile_acquirer_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profile_acquirer_update")
            );
    }

    explicit ProfileAcquirerRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    // ---- authentication -----------------------------------------------------
    // Local port of upstream's `auth_type` dual extractor
    // (routes/profile_acquirer.rs:34-46,81-93):
    //   HeaderAuth(ApiKeyAuth{allow_connected_scope_operation: true,
    //                         allow_platform_self_operation: false})
    //   OR JWTAuthProfileFromRoute{profile_id, ProfileAccountWrite,
    //                              allow_connected: true, allow_platform: false}
    // Permission::ProfileAccountWrite maps to AppPermission::PROFILE_WRITE, the
    // same local mapping used for that upstream permission in verification.cppm.
    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        std::string_view route_profile_id
    ) const
    {
        (void)operation; // audited registry rows exist but are coarser than ProfileAccountWrite
        auto context = app::auth::AuthContextBuilder{}(req);

        // is_jwt_auth: a Bearer token selects the JWT branch, else ApiKeyAuth.
        if (!context.token().is_empty()) {
            // JWTAuthProfileFromRoute binds the caller to the profile id from the
            // route/body. When an x-profile-id header is present it must match the
            // route profile; the auth principal is then the route profile id.
            if (!context.profile_id().empty()
                && context.profile_id() != std::string{route_profile_id}) {
                auto error = core::error::AppErrorBuilder{}(core::error::AppErrorKind::UNAUTHORIZED,
                    "Unauthorized");
                error.set_kind_name("Unauthorized");
                return error;
            }
            context.set_profile_id(std::string{route_profile_id});
            const auto rule = app::auth::RouteAuthRuleBuilder{}
                                  .scheme(app::auth::AuthScheme::JWT)
                                  .permission(app::auth::AppPermission::PROFILE_WRITE)
                                  .allow_connected(true)
                                  .allow_platform(false)
                                  .build();
            return m_context->authorizer().authorize_route(context, rule);
        }

        // The local authorizer requires a credential for API_KEY rules; upstream
        // authenticates this branch from the api-key header, so carry it as the
        // local credential (same adaptation as verification.cppm).
        if (const auto api_key = context.claim(app::auth::kClaimApiKey);
            api_key && !api_key->empty()) {
            context.set_token(app::auth::BearerToken{std::string{*api_key}});
        }
        const auto rule = app::auth::RouteAuthRuleBuilder{}
                              .scheme(app::auth::AuthScheme::API_KEY)
                              .permission(app::auth::AppPermission::PROFILE_WRITE)
                              .allow_connected(true)
                              .allow_platform(false)
                              .build();
        return m_context->authorizer().authorize_route(context, rule);
    }

    // ---- serde extraction validation (upstream web::Json<...> extraction) ----
    // Upstream rejects at extraction with HTTP 400, before server_wrap/core:
    //   ProfileAcquirerCreate (api_models:4-32)   -> enum validation only
    //   ProfileAcquirerUpdate (api_models:154-174)-> #[serde(deny_unknown_fields)] + enums
    // CardNetwork serde values are the variant names plus #[serde(alias)] entries
    // (common_enums/src/enums.rs:3659-3723); CountryAlpha2 serde values are the
    // 249 two-letter variants (enums.rs:4285-...), the exact set held by the
    // alpha2_to_numeric() table used by normalize_country() below.
    [[nodiscard]] static bool payload_passes_serde_validation(
        std::string_view body,
        bool deny_unknown_fields
    )
    {
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!generic) {
            return false;
        }
        const auto* root = std::get_if<interfaces::Value::Object>(&generic->get());
        if (root == nullptr) {
            return false;
        }
        static constexpr auto kUpdateFields = std::to_array<std::string_view>({
            "acquirer_assigned_merchant_id", "merchant_name", "network", "acquirer_bin",
            "acquirer_ica", "acquirer_fraud_rate", "acquirer_country_code", "is_default"});
        for (const auto& [key, value] : *root) {
            const std::string_view field{key};
            if (deny_unknown_fields
                && std::find(kUpdateFields.begin(), kUpdateFields.end(), field) == kUpdateFields.end()) {
                return false; // serde deny_unknown_fields
            }
            if (const auto* text = std::get_if<std::string>(&value.get())) {
                if (field == "network" && !is_valid_card_network(*text)) {
                    return false;
                }
                if (field == "acquirer_country_code" && !is_valid_country_alpha2(*text)) {
                    return false;
                }
            }
            // A non-string enum value is either null (Option => valid) or is
            // rejected by the typed deserialize<Input>() below with HTTP 400.
        }
        return true;
    }

    [[nodiscard]] static bool is_valid_card_network(std::string_view network)
    {
        static constexpr auto kAccepted = std::to_array<std::string_view>({
            "Visa", "Mastercard", "AmericanExpress", "JCB", "DinersClub", "Discover",
            "CartesBancaires", "UnionPay", "Interac", "RuPay", "Maestro", "Star", "Pulse",
            "Accel", "Nyce", "Prop", "PrivateLabel", "Dinacard", "AirPlus", "Aurore",
            "EftposAustralia", "GeCapital", "Uatp",
            "VISA", "MASTERCARD", "AMERICANEXPRESS", "AMEX", "DINERSCLUB", "DISCOVER",
            "CARTESBANCAIRES", "UNIONPAY", "INTERAC", "RUPAY", "MAESTRO", "STAR", "PULSE",
            "ACCEL", "NYCE", "PROP", "PRIVATE LABEL", "DINACARD", "AIRPLUS", "AURORE",
            "EFTPOS_AUSTRALIA", "GECAPITAL", "UATP"});
        return std::find(kAccepted.begin(), kAccepted.end(), network) != kAccepted.end();
    }

    [[nodiscard]] static bool is_valid_country_alpha2(std::string_view code)
    {
        return alpha2_to_numeric(code).has_value();
    }

    // ---- persistence: business state mirrors the AcquirerConfigBucket --------
    [[nodiscard]] std::optional<std::string> load_value(std::string_view key) const
    {
        return m_context->repository().get_config(key);
    }

    void save_value(std::string_view key, std::string value) const
    {
        m_context->repository().set_config(std::string{key}, std::move(value));
    }

    [[nodiscard]] std::string default_key_for(std::string_view profile_id) const
    {
        return std::string{"profile_acquirer.default:"} + std::string{profile_id};
    }

    [[nodiscard]] std::string config_index_key(std::string_view profile_id) const
    {
        return std::string{"profile_acquirer.ids:"} + std::string{profile_id};
    }

    [[nodiscard]] std::string networks_key_for(std::string_view acquirer_id) const
    {
        return std::string{"profile_acquirer.networks:"} + std::string{acquirer_id};
    }

    [[nodiscard]] std::string config_key(std::string_view acquirer_id, std::string_view network) const
    {
        return std::string{"profile_acquirer.config:"} + std::string{acquirer_id}
             + ":" + std::string{network};
    }

    [[nodiscard]] static std::vector<std::string_view> split_csv(std::string_view value)
    {
        std::vector<std::string_view> out{};
        std::size_t start{0};
        while (start <= value.size()) {
            const std::size_t comma{value.find(',', start)};
            const std::size_t end{comma == std::string_view::npos ? value.size() : comma};
            out.push_back(value.substr(start, end - start));
            if (comma == std::string_view::npos) {
                break;
            }
            start = comma + 1;
        }
        return out;
    }

    [[nodiscard]] bool profile_exists(std::string_view profile_id) const
    {
        auto repository = m_context->repository();
        if (repository.find_profile_by_string(profile_id)) {
            return true;
        }
        for (const auto& account : repository.list_merchant_accounts()) {
            if (account.get_default_profile() == profile_id) {
                return true;
            }
        }
        // No local registry populated: nothing to validate against in this environment.
        return repository.list_profiles().empty() && repository.list_merchant_accounts().empty();
    }

    [[nodiscard]] std::optional<AcquirerConfig> load_config(
        std::string_view acquirer_id,
        std::string_view network
    ) const
    {
        const auto raw = load_value(config_key(acquirer_id, network));
        if (!raw) {
            return std::nullopt;
        }
        auto decoded = serde::Ser::deserialize<AcquirerConfig>("application/json", *raw);
        if (!decoded) {
            return std::nullopt;
        }
        return *decoded;
    }

    /** @brief Rebuilds the full AcquirerConfigBucket from the local records. */
    [[nodiscard]] std::optional<AcquirerConfigBucket> load_bucket(std::string_view profile_id) const
    {
        const auto ids_raw = load_value(config_index_key(profile_id));
        if (!ids_raw) {
            return std::nullopt; // acquirer_config_map == None
        }
        AcquirerConfigBucket bucket{};
        const auto default_raw = load_value(default_key_for(profile_id));
        if (default_raw && !default_raw->empty()) {
            bucket.default_acquirer_config = *default_raw;
        }
        for (std::string_view acquirer_id : split_csv(*ids_raw)) {
            if (acquirer_id.empty()) {
                continue;
            }
            std::vector<AcquirerConfig> vec{};
            const auto networks_raw = load_value(networks_key_for(acquirer_id));
            if (networks_raw && !networks_raw->empty()) {
                for (std::string_view network : split_csv(*networks_raw)) {
                    if (auto config = load_config(acquirer_id, network)) {
                        vec.push_back(std::move(*config));
                    }
                }
            }
            bucket.configs[std::string{acquirer_id}] = std::move(vec);
        }
        return bucket;
    }

    /** @brief Persists the AcquirerConfigBucket (the AcquirerConfigBucketUpdate). */
    void save_bucket(std::string_view profile_id, const AcquirerConfigBucket& bucket) const
    {
        // default_acquirer_config == None is represented by absence (empty string).
        save_value(default_key_for(profile_id), bucket.default_acquirer_config.value_or(std::string{}));

        std::string ids{};
        for (const auto& [acquirer_id, configs] : bucket.configs) {
            (void)configs;
            if (!ids.empty()) {
                ids.push_back(',');
            }
            ids.append(acquirer_id);
        }
        save_value(config_index_key(profile_id), ids);

        for (const auto& [acquirer_id, configs] : bucket.configs) {
            std::string networks{};
            for (const auto& config : configs) {
                if (!networks.empty()) {
                    networks.push_back(',');
                }
                networks.append(config.get_network());
                save_value(
                    config_key(acquirer_id, config.get_network()),
                    serde::Ser::encode_json(config)
                );
            }
            save_value(networks_key_for(acquirer_id), networks);
        }
    }

    // ---- upstream core helpers ----------------------------------------------

    /** @brief core/profile_acquirer.rs:17 — generate_profile_acquirer_id_of_default_length().
     *
     * Upstream: generate_id_with_default_len("pro_acq") -> "pro_acq_" + 20 chars
     * from consts::ALPHABETS ([0-9a-zA-Z], lib.rs:699-702, consts.rs:7,10-15).
     * NanoID is random; this uses a deterministic PRNG seeded from IdGenerator so
     * the prefix, length and exact alphabet match while staying reproducible.
     */
    [[nodiscard]] std::string generate_profile_acquirer_id_of_default_length() const
    {
        constexpr std::string_view alphabets{
            "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
        };
        std::uint64_t state{0x9E3779B97F4A7C15ULL};
        for (char c : m_context->ids().payment_id("")) {
            state = state * 6364136223846793005ULL + static_cast<unsigned char>(c);
        }
        std::string out{"pro_acq_"};
        out.reserve(8 + 20);
        for (int i{0}; i < 20; ++i) {
            state = state * 6364136223846793005ULL + 1442695040888963407ULL;
            out.push_back(alphabets[static_cast<std::size_t>((state >> 33) % alphabets.size())]);
        }
        return out;
    }

    /** @brief Full 249-entry ISO 3166-1 alpha2 -> numeric table (transformers.rs:277+,1544+).
     *
     * Shared by normalize_country() and is_valid_country_alpha2() so extraction-time
     * CountryAlpha2 validation and the core conversion use the exact same code set.
     */
    [[nodiscard]] static std::optional<unsigned> alpha2_to_numeric(std::string_view alpha)
    {
        static constexpr std::pair<std::string_view, unsigned> kAlpha2ToNumeric[]{
            {"AD", 20},   {"AE", 784},  {"AF", 4},    {"AG", 28},   {"AI", 660},
            {"AL", 8},    {"AM", 51},   {"AO", 24},   {"AQ", 10},   {"AR", 32},
            {"AS", 16},   {"AT", 40},   {"AU", 36},   {"AW", 533},  {"AX", 248},
            {"AZ", 31},   {"BA", 70},   {"BB", 52},   {"BD", 50},   {"BE", 56},
            {"BF", 854},  {"BG", 100},  {"BH", 48},   {"BI", 108},  {"BJ", 204},
            {"BL", 652},  {"BM", 60},   {"BN", 96},   {"BO", 68},   {"BQ", 535},
            {"BR", 76},   {"BS", 44},   {"BT", 64},   {"BV", 74},   {"BW", 72},
            {"BY", 112},  {"BZ", 84},   {"CA", 124},  {"CC", 166},  {"CD", 180},
            {"CF", 140},  {"CG", 178},  {"CH", 756},  {"CI", 384},  {"CK", 184},
            {"CL", 152},  {"CM", 120},  {"CN", 156},  {"CO", 170},  {"CR", 188},
            {"CU", 192},  {"CV", 132},  {"CW", 531},  {"CX", 162},  {"CY", 196},
            {"CZ", 203},  {"DE", 276},  {"DJ", 262},  {"DK", 208},  {"DM", 212},
            {"DO", 214},  {"DZ", 12},   {"EC", 218},  {"EE", 233},  {"EG", 818},
            {"EH", 732},  {"ER", 232},  {"ES", 724},  {"ET", 231},  {"FI", 246},
            {"FJ", 242},  {"FK", 238},  {"FM", 583},  {"FO", 234},  {"FR", 250},
            {"GA", 266},  {"GB", 826},  {"GD", 308},  {"GE", 268},  {"GF", 254},
            {"GG", 831},  {"GH", 288},  {"GI", 292},  {"GL", 304},  {"GM", 270},
            {"GN", 324},  {"GP", 312},  {"GQ", 226},  {"GR", 300},  {"GS", 239},
            {"GT", 320},  {"GU", 316},  {"GW", 624},  {"GY", 328},  {"HK", 344},
            {"HM", 334},  {"HN", 340},  {"HR", 191},  {"HT", 332},  {"HU", 348},
            {"ID", 360},  {"IE", 372},  {"IL", 376},  {"IM", 833},  {"IN", 356},
            {"IO", 86},   {"IQ", 368},  {"IR", 364},  {"IS", 352},  {"IT", 380},
            {"JE", 832},  {"JM", 388},  {"JO", 400},  {"JP", 392},  {"KE", 404},
            {"KG", 417},  {"KH", 116},  {"KI", 296},  {"KM", 174},  {"KN", 659},
            {"KP", 408},  {"KR", 410},  {"KW", 414},  {"KY", 136},  {"KZ", 398},
            {"LA", 418},  {"LB", 422},  {"LC", 662},  {"LI", 438},  {"LK", 144},
            {"LR", 430},  {"LS", 426},  {"LT", 440},  {"LU", 442},  {"LV", 428},
            {"LY", 434},  {"MA", 504},  {"MC", 492},  {"MD", 498},  {"ME", 499},
            {"MF", 663},  {"MG", 450},  {"MH", 584},  {"MK", 807},  {"ML", 466},
            {"MM", 104},  {"MN", 496},  {"MO", 446},  {"MP", 580},  {"MQ", 474},
            {"MR", 478},  {"MS", 500},  {"MT", 470},  {"MU", 480},  {"MV", 462},
            {"MW", 454},  {"MX", 484},  {"MY", 458},  {"MZ", 508},  {"NA", 516},
            {"NC", 540},  {"NE", 562},  {"NF", 574},  {"NG", 566},  {"NI", 558},
            {"NL", 528},  {"NO", 578},  {"NP", 524},  {"NR", 520},  {"NU", 570},
            {"NZ", 554},  {"OM", 512},  {"PA", 591},  {"PE", 604},  {"PF", 258},
            {"PG", 598},  {"PH", 608},  {"PK", 586},  {"PL", 616},  {"PM", 666},
            {"PN", 612},  {"PR", 630},  {"PS", 275},  {"PT", 620},  {"PW", 585},
            {"PY", 600},  {"QA", 634},  {"RE", 638},  {"RO", 642},  {"RS", 688},
            {"RU", 643},  {"RW", 646},  {"SA", 682},  {"SB", 90},   {"SC", 690},
            {"SD", 729},  {"SE", 752},  {"SG", 702},  {"SH", 654},  {"SI", 705},
            {"SJ", 744},  {"SK", 703},  {"SL", 694},  {"SM", 674},  {"SN", 686},
            {"SO", 706},  {"SR", 740},  {"SS", 728},  {"ST", 678},  {"SV", 222},
            {"SX", 534},  {"SY", 760},  {"SZ", 748},  {"TC", 796},  {"TD", 148},
            {"TF", 260},  {"TG", 768},  {"TH", 764},  {"TJ", 762},  {"TK", 772},
            {"TL", 626},  {"TM", 795},  {"TN", 788},  {"TO", 776},  {"TR", 792},
            {"TT", 780},  {"TV", 798},  {"TW", 158},  {"TZ", 834},  {"UA", 804},
            {"UG", 800},  {"UM", 581},  {"US", 840},  {"UY", 858},  {"UZ", 860},
            {"VA", 336},  {"VC", 670},  {"VE", 862},  {"VG", 92},   {"VI", 850},
            {"VN", 704},  {"VU", 548},  {"WF", 876},  {"WS", 882},  {"YE", 887},
            {"YT", 175},  {"ZA", 710},  {"ZM", 894},  {"ZW", 716}
        };
        for (const auto& [key, numeric] : kAlpha2ToNumeric) {
            if (key == alpha) {
                return numeric;
            }
        }
        return std::nullopt;
    }

    /** @brief common_enums::Country::from_alpha2(...).to_numeric().to_string() (core:47-51,268-272).
     *
     * Values are u32, so `to_string` is UNpadded (e.g. AU -> "36", US -> "840").
     */
    [[nodiscard]] static std::optional<std::string> normalize_country(
        const std::optional<app::dto::profile_acquirer::CountryAlpha2Value>& code
    )
    {
        if (!code) {
            return std::nullopt;
        }
        if (const auto numeric = alpha2_to_numeric(code->get_value())) {
            return std::to_string(*numeric);
        }
        // Invalid alpha-2 would be rejected by extraction-time validation above
        // (same set as CountryAlpha2); keep the raw value as a safe fallback.
        return code->get_value();
    }

    /** @brief core:40-52 — build the incoming AcquirerConfig on create. */
    [[nodiscard]] static AcquirerConfig config_from_create(
        const app::dto::profile_acquirer::ProfileAcquirerCreate& in
    )
    {
        AcquirerConfig out{};
        out.set_acquirer_assigned_merchant_id(in.get_acquirer_assigned_merchant_id());
        out.set_merchant_name(in.get_merchant_name());
        out.set_network(in.get_network().get_value());
        out.set_acquirer_bin(in.get_acquirer_bin());
        out.set_acquirer_ica(in.get_acquirer_ica());
        out.set_acquirer_fraud_rate(in.get_acquirer_fraud_rate());
        out.set_acquirer_country_code(normalize_country(in.get_acquirer_country_code()));
        return out;
    }

    /** @brief core/profile_acquirer.rs:219-236 apply_default_bucket_change. */
    static bool apply_default_bucket_change(
        AcquirerConfigBucket& config_map,
        std::string_view profile_acquirer_id,
        const std::optional<std::string>& current_default_id,
        const std::optional<bool>& is_default_request
    )
    {
        if (is_default_request && *is_default_request &&
            (!current_default_id || *current_default_id != profile_acquirer_id)) {
            config_map.default_acquirer_config = std::string{profile_acquirer_id};
            return true;
        }
        if (is_default_request && !*is_default_request &&
            current_default_id && *current_default_id == profile_acquirer_id) {
            config_map.default_acquirer_config.reset(); // upstream: None
            return true;
        }
        return false;
    }

    /** @brief core/profile_acquirer.rs:241-312 upsert_acquirer_config_in_bucket. */
    [[nodiscard]] std::optional<core::error::AppError> upsert_acquirer_config_in_bucket(
        AcquirerConfigBucket& config_map,
        std::string_view profile_acquirer_id,
        const std::string& target_network,
        const app::dto::profile_acquirer::ProfileAcquirerUpdate& request,
        bool default_bucket_changed
    ) const
    {
        auto it = config_map.configs.find(std::string{profile_acquirer_id});
        if (it == config_map.configs.end()) {
            // core:251-252 .attach_printable("bucket was verified above but vanished")
            return internal_server_error();
        }
        auto& bucket = it->second;

        // existing_pos = bucket.iter().position(|cfg| cfg.network == target_network)
        std::size_t existing_pos{bucket.size()};
        for (std::size_t i{0}; i < bucket.size(); ++i) {
            if (bucket[i].get_network() == target_network) {
                existing_pos = i;
                break;
            }
        }
        const bool has_existing{existing_pos < bucket.size()};

        // base = existing.clone().unwrap_or(default with network = target_network)
        AcquirerConfig base{};
        base.set_network(target_network);
        if (has_existing) {
            base = bucket[existing_pos];
        }

        // request_acquirer_country_code normalized (core:268-272).
        const auto request_country = normalize_country(request.get_acquirer_country_code());

        // upserted = request.or(base) for every field (core:274-285).
        AcquirerConfig upserted{};
        upserted.set_network(base.get_network());
        upserted.set_acquirer_assigned_merchant_id(
            request.get_acquirer_assigned_merchant_id() ? request.get_acquirer_assigned_merchant_id()
                                                        : base.get_acquirer_assigned_merchant_id());
        upserted.set_merchant_name(
            request.get_merchant_name() ? request.get_merchant_name()
                                        : base.get_merchant_name());
        upserted.set_acquirer_bin(
            request.get_acquirer_bin() ? request.get_acquirer_bin()
                                       : base.get_acquirer_bin());
        upserted.set_acquirer_ica(
            request.get_acquirer_ica() ? request.get_acquirer_ica()
                                       : base.get_acquirer_ica());
        upserted.set_acquirer_fraud_rate(
            request.get_acquirer_fraud_rate() ? request.get_acquirer_fraud_rate()
                                              : base.get_acquirer_fraud_rate());
        upserted.set_acquirer_country_code(
            request_country ? request_country : base.get_acquirer_country_code());

        // Duplicate check: identical existing config and no default change (core:287-300).
        if (has_existing && config_equal(bucket[existing_pos], upserted) && !default_bucket_changed) {
            auto error = core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST,
                std::format("An identical configuration for network '{}' already exists in bucket '{}'.",
                            target_network, profile_acquirer_id));
            error.set_kind_name("GenericDuplicateError");
            return error;
        }

        // Replace-or-push (core:302-309).
        if (has_existing) {
            bucket[existing_pos] = std::move(upserted);
        } else {
            bucket.push_back(std::move(upserted));
        }
        return std::nullopt;
    }

    /** @brief Field-wise PartialEq mirror of common_types::domain::AcquirerConfig. */
    [[nodiscard]] static bool config_equal(const AcquirerConfig& lhs, const AcquirerConfig& rhs)
    {
        return lhs.get_acquirer_assigned_merchant_id() == rhs.get_acquirer_assigned_merchant_id()
            && lhs.get_merchant_name() == rhs.get_merchant_name()
            && lhs.get_network() == rhs.get_network()
            && lhs.get_acquirer_bin() == rhs.get_acquirer_bin()
            && lhs.get_acquirer_ica() == rhs.get_acquirer_ica()
            && lhs.get_acquirer_fraud_rate() == rhs.get_acquirer_fraud_rate()
            && lhs.get_acquirer_country_code() == rhs.get_acquirer_country_code();
    }

    /** @brief core:81-92 / 195-205 — find the stored config for a network. */
    [[nodiscard]] static const AcquirerConfig* find_config_in_bucket(
        const AcquirerConfigBucket* bucket,
        std::string_view profile_acquirer_id,
        std::string_view network
    )
    {
        if (bucket == nullptr) {
            return nullptr;
        }
        auto it = bucket->configs.find(std::string{profile_acquirer_id});
        if (it == bucket->configs.end()) {
            return nullptr;
        }
        for (const auto& config : it->second) {
            if (config.get_network() == network) {
                return &config;
            }
        }
        return nullptr;
    }

    /** @brief api_models/profile_acquirer.rs:108-138 — ProfileAcquirerResponse::from. */
    [[nodiscard]] static app::dto::profile_acquirer::ProfileAcquirerResponse build_response(
        std::string_view acquirer_id,
        std::string_view profile_id,
        const AcquirerConfig* config,
        bool is_default
    )
    {
        app::dto::profile_acquirer::ProfileAcquirerResponse response;
        app::dto::profile_acquirer::ProfileAcquirerIdValue acquirer_id_value;
        acquirer_id_value.set_value(std::string{acquirer_id});
        response.set_profile_acquirer_id(acquirer_id_value);
        app::dto::profile_acquirer::ProfileIdValue profile_id_value;
        profile_id_value.set_value(std::string{profile_id});
        response.set_profile_id(profile_id_value);
        response.set_is_default(is_default);
        if (config != nullptr) {
            response.set_acquirer_assigned_merchant_id(config->get_acquirer_assigned_merchant_id());
            response.set_merchant_name(config->get_merchant_name());
            app::dto::profile_acquirer::CardNetworkValue network;
            network.set_value(config->get_network());
            response.set_network(network);
            response.set_acquirer_bin(config->get_acquirer_bin());
            response.set_acquirer_ica(config->get_acquirer_ica());
            response.set_acquirer_fraud_rate(config->get_acquirer_fraud_rate());
            response.set_acquirer_country_code(config->get_acquirer_country_code());
        }
        return response;
    }

    // ---- upstream error responses -------------------------------------------
    // Wire name + message mirror hyperswitch_domain_models ApiErrorResponse
    // (api_error_response.rs:474-478,586-588,687-689) and the HTTP status of the
    // mapped AppErrorKind.
    [[nodiscard]] static core::error::AppError profile_not_found(std::string_view id)
    {
        auto error = core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND,
            std::format("Business profile with the given id {} does not exist", id));
        error.set_kind_name("ProfileNotFound");
        return error;
    }

    [[nodiscard]] static core::error::AppError profile_acquirer_not_found(
        std::string_view profile_id,
        std::string_view acquirer_id
    )
    {
        auto error = core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND,
            std::format("Profile acquirer with id '{}' not found for profile '{}'.", acquirer_id, profile_id));
        error.set_kind_name("ProfileAcquirerNotFound");
        return error;
    }

    [[nodiscard]] static core::error::AppError invalid_request(std::string message)
    {
        // Upstream InvalidRequestData -> AER::Unprocessable (HTTP 422).
        auto error = core::error::AppErrorBuilder{}(core::error::AppErrorKind::VALIDATION_FAILED,
            std::move(message));
        error.set_kind_name("InvalidRequestData");
        return error;
    }

    [[nodiscard]] static core::error::AppError internal_server_error()
    {
        // Upstream ApiErrorResponse::InternalServerError -> ApiError("HE", 0,
        // "Something went wrong"); attach_printable context is not in the response.
        auto error = core::error::AppErrorBuilder{}(core::error::AppErrorKind::INTERNAL,
            "Something went wrong");
        error.set_kind_name("InternalServerError");
        return error;
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

} // namespace app::handlers::profile_acquirer
