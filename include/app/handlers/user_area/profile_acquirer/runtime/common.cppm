export module app_user:profile_acquirer_runtime_common;

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
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :profile_acquirer_input;

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
    void set_acquirer_assigned_merchant_id(std::optional<std::string> value)
    {
        acquirer_assigned_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_acquirer_assigned_merchant_id() const noexcept
    {
        return acquirer_assigned_merchant_id;
    }

    void set_merchant_name(std::optional<std::string> value)
    {
        merchant_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_name() const noexcept
    {
        return merchant_name;
    }

    void set_network(std::string value)
    {
        network = std::move(value);
    }

    [[nodiscard]] const std::string& get_network() const noexcept
    {
        return network;
    }

    void set_acquirer_bin(std::optional<std::string> value)
    {
        acquirer_bin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_acquirer_bin() const noexcept
    {
        return acquirer_bin;
    }

    void set_acquirer_ica(std::optional<std::string> value)
    {
        acquirer_ica = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_acquirer_ica() const noexcept
    {
        return acquirer_ica;
    }

    void set_acquirer_fraud_rate(std::optional<double> value)
    {
        acquirer_fraud_rate = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_acquirer_fraud_rate() const noexcept
    {
        return acquirer_fraud_rate;
    }

    void set_acquirer_country_code(std::optional<std::string> value)
    {
        acquirer_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_acquirer_country_code() const noexcept
    {
        return acquirer_country_code;
    }

    std::optional<std::string> acquirer_assigned_merchant_id{};
    std::optional<std::string> merchant_name{};
    std::string network{};
    std::optional<std::string> acquirer_bin{};
    std::optional<std::string> acquirer_ica{};
    std::optional<double> acquirer_fraud_rate{};
    std::optional<std::string> acquirer_country_code{};
};

} // namespace app::handlers::profile_acquirer

// Serialization must be declared before the support bodies that use this shape.
template<>
struct serde::Serializable<::app::handlers::profile_acquirer::AcquirerConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "acquirer_assigned_merchant_id",
                &::app::handlers::profile_acquirer::AcquirerConfig::
                    get_acquirer_assigned_merchant_id,
                &::app::handlers::profile_acquirer::AcquirerConfig::
                    set_acquirer_assigned_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_name",
                &::app::handlers::profile_acquirer::AcquirerConfig::get_merchant_name,
                &::app::handlers::profile_acquirer::AcquirerConfig::set_merchant_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network",
                &::app::handlers::profile_acquirer::AcquirerConfig::get_network,
                &::app::handlers::profile_acquirer::AcquirerConfig::set_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_bin",
                &::app::handlers::profile_acquirer::AcquirerConfig::get_acquirer_bin,
                &::app::handlers::profile_acquirer::AcquirerConfig::set_acquirer_bin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_ica",
                &::app::handlers::profile_acquirer::AcquirerConfig::get_acquirer_ica,
                &::app::handlers::profile_acquirer::AcquirerConfig::set_acquirer_ica,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_fraud_rate",
                &::app::handlers::profile_acquirer::AcquirerConfig::get_acquirer_fraud_rate,
                &::app::handlers::profile_acquirer::AcquirerConfig::set_acquirer_fraud_rate,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_country_code",
                &::app::handlers::profile_acquirer::AcquirerConfig::get_acquirer_country_code,
                &::app::handlers::profile_acquirer::AcquirerConfig::set_acquirer_country_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "acquirer_config";
    }
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

/**
 * @brief Shared persistence and upstream-core helpers for the profile_acquirer sub-runtimes.
 */
class ProfileAcquirerRuntimeSupport
{
public:
    explicit ProfileAcquirerRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_input{context}
    {
    }

    // ---- persistence: business state mirrors the AcquirerConfigBucket --------
    [[nodiscard]] std::optional<std::string> load_value(std::string_view key) const
    {
        return m_context.repository().get_config(key);
    }

    void save_value(std::string_view key, std::string value) const
    {
        m_context.repository().set_config(std::string{key}, std::move(value));
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

    [[nodiscard]] std::string
    config_key(std::string_view acquirer_id, std::string_view network) const
    {
        return std::string{"profile_acquirer.config:"} + std::string{acquirer_id} + ":" +
               std::string{network};
    }

    [[nodiscard]] std::vector<std::string_view> split_csv(std::string_view value) const
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
        auto repository = m_context.repository();
        if (repository.find_profile_by_string(profile_id)) {
            return true;
        }
        for (const auto& account: repository.list_merchant_accounts()) {
            if (account.get_default_profile() == profile_id) {
                return true;
            }
        }
        // No local registry populated: nothing to validate against in this environment.
        return repository.list_profiles().empty() && repository.list_merchant_accounts().empty();
    }

    [[nodiscard]] std::optional<AcquirerConfig>
    load_config(std::string_view acquirer_id, std::string_view network) const
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
        for (std::string_view acquirer_id: split_csv(*ids_raw)) {
            if (acquirer_id.empty()) {
                continue;
            }
            std::vector<AcquirerConfig> vec{};
            const auto networks_raw = load_value(networks_key_for(acquirer_id));
            if (networks_raw && !networks_raw->empty()) {
                for (std::string_view network: split_csv(*networks_raw)) {
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
        save_value(
            default_key_for(profile_id),
            bucket.default_acquirer_config.value_or(std::string{})
        );

        std::string ids{};
        for (const auto& [acquirer_id, configs]: bucket.configs) {
            (void)configs;
            if (!ids.empty()) {
                ids.push_back(',');
            }
            ids.append(acquirer_id);
        }
        save_value(config_index_key(profile_id), ids);

        for (const auto& [acquirer_id, configs]: bucket.configs) {
            std::string networks{};
            for (const auto& config: configs) {
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
        std::uint64_t state{0x9E'37'79'B9'7F'4A'7C'15ULL};
        for (char c: m_context.ids().payment_id("")) {
            state = state * 6'364'136'223'846'793'005ULL + static_cast<unsigned char>(c);
        }
        std::string out{"pro_acq_"};
        out.reserve(8 + 20);
        for (int i{0}; i < 20; ++i) {
            state = state * 6'364'136'223'846'793'005ULL + 1'442'695'040'888'963'407ULL;
            out.push_back(alphabets[static_cast<std::size_t>((state >> 33) % alphabets.size())]);
        }
        return out;
    }

    /** @brief common_enums::Country::from_alpha2(...).to_numeric().to_string()
     * (core:47-51,268-272).
     *
     * Values are u32, so `to_string` is UNpadded (e.g. AU -> "36", US -> "840").
     */
    [[nodiscard]] std::optional<std::string> normalize_country(
        const std::optional<::app::dto::profile_acquirer::CountryAlpha2Value>& code
    ) const
    {
        if (!code) {
            return std::nullopt;
        }
        if (const auto numeric = m_input.alpha2_to_numeric(code->get_value())) {
            return std::to_string(*numeric);
        }
        // Invalid alpha-2 would be rejected by extraction-time validation above
        // (same set as CountryAlpha2); keep the raw value as a safe fallback.
        return code->get_value();
    }

    /** @brief core:40-52 — build the incoming AcquirerConfig on create. */
    [[nodiscard]] AcquirerConfig
    config_from_create(const ::app::dto::profile_acquirer::ProfileAcquirerCreate& in) const
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
    bool apply_default_bucket_change(
        AcquirerConfigBucket& config_map,
        std::string_view profile_acquirer_id,
        const std::optional<std::string>& current_default_id,
        const std::optional<bool>& is_default_request
    ) const
    {
        if (is_default_request && *is_default_request &&
            (!current_default_id || *current_default_id != profile_acquirer_id)) {
            config_map.default_acquirer_config = std::string{profile_acquirer_id};
            return true;
        }
        if (is_default_request && !*is_default_request && current_default_id &&
            *current_default_id == profile_acquirer_id) {
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
        const ::app::dto::profile_acquirer::ProfileAcquirerUpdate& request,
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
            request.get_acquirer_assigned_merchant_id()
                ? request.get_acquirer_assigned_merchant_id()
                : base.get_acquirer_assigned_merchant_id()
        );
        upserted.set_merchant_name(
            request.get_merchant_name() ? request.get_merchant_name() : base.get_merchant_name()
        );
        upserted.set_acquirer_bin(
            request.get_acquirer_bin() ? request.get_acquirer_bin() : base.get_acquirer_bin()
        );
        upserted.set_acquirer_ica(
            request.get_acquirer_ica() ? request.get_acquirer_ica() : base.get_acquirer_ica()
        );
        upserted.set_acquirer_fraud_rate(
            request.get_acquirer_fraud_rate() ? request.get_acquirer_fraud_rate()
                                              : base.get_acquirer_fraud_rate()
        );
        upserted.set_acquirer_country_code(
            request_country ? request_country : base.get_acquirer_country_code()
        );

        // Duplicate check: identical existing config and no default change (core:287-300).
        if (has_existing && config_equal(bucket[existing_pos], upserted) &&
            !default_bucket_changed) {
            auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                std::format(
                    "An identical configuration for network '{}' already exists in bucket '{}'.",
                    target_network,
                    profile_acquirer_id
                )
            );
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
    [[nodiscard]] bool config_equal(const AcquirerConfig& lhs, const AcquirerConfig& rhs) const
    {
        return lhs.get_acquirer_assigned_merchant_id() == rhs.get_acquirer_assigned_merchant_id() &&
               lhs.get_merchant_name() == rhs.get_merchant_name() &&
               lhs.get_network() == rhs.get_network() &&
               lhs.get_acquirer_bin() == rhs.get_acquirer_bin() &&
               lhs.get_acquirer_ica() == rhs.get_acquirer_ica() &&
               lhs.get_acquirer_fraud_rate() == rhs.get_acquirer_fraud_rate() &&
               lhs.get_acquirer_country_code() == rhs.get_acquirer_country_code();
    }

    /** @brief core:81-92 / 195-205 — find the stored config for a network. */
    [[nodiscard]] const AcquirerConfig* find_config_in_bucket(
        const AcquirerConfigBucket* bucket,
        std::string_view profile_acquirer_id,
        std::string_view network
    ) const
    {
        if (bucket == nullptr) {
            return nullptr;
        }
        auto it = bucket->configs.find(std::string{profile_acquirer_id});
        if (it == bucket->configs.end()) {
            return nullptr;
        }
        for (const auto& config: it->second) {
            if (config.get_network() == network) {
                return &config;
            }
        }
        return nullptr;
    }

    /** @brief api_models/profile_acquirer.rs:108-138 — ProfileAcquirerResponse::from. */
    [[nodiscard]] ::app::dto::profile_acquirer::ProfileAcquirerResponse build_response(
        std::string_view acquirer_id,
        std::string_view profile_id,
        const AcquirerConfig* config,
        bool is_default
    ) const
    {
        ::app::dto::profile_acquirer::ProfileAcquirerResponse response;
        ::app::dto::profile_acquirer::ProfileAcquirerIdValue acquirer_id_value;
        acquirer_id_value.set_value(std::string{acquirer_id});
        response.set_profile_acquirer_id(acquirer_id_value);
        ::app::dto::profile_acquirer::ProfileIdValue profile_id_value;
        profile_id_value.set_value(std::string{profile_id});
        response.set_profile_id(profile_id_value);
        response.set_is_default(is_default);
        if (config != nullptr) {
            response.set_acquirer_assigned_merchant_id(config->get_acquirer_assigned_merchant_id());
            response.set_merchant_name(config->get_merchant_name());
            ::app::dto::profile_acquirer::CardNetworkValue network;
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
    [[nodiscard]] core::error::AppError profile_not_found(std::string_view id) const
    {
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::NOT_FOUND,
            std::format("Business profile with the given id {} does not exist", id)
        );
        error.set_kind_name("ProfileNotFound");
        return error;
    }

    [[nodiscard]] core::error::AppError
    profile_acquirer_not_found(std::string_view profile_id, std::string_view acquirer_id) const
    {
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::NOT_FOUND,
            std::format(
                "Profile acquirer with id '{}' not found for profile '{}'.",
                acquirer_id,
                profile_id
            )
        );
        error.set_kind_name("ProfileAcquirerNotFound");
        return error;
    }

    [[nodiscard]] core::error::AppError invalid_request(std::string message) const
    {
        // Upstream InvalidRequestData -> AER::Unprocessable (HTTP 422).
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::VALIDATION_FAILED,
            std::move(message)
        );
        error.set_kind_name("InvalidRequestData");
        return error;
    }

    [[nodiscard]] core::error::AppError internal_server_error() const
    {
        // Upstream ApiErrorResponse::InternalServerError -> ApiError("HE", 0,
        // "Something went wrong"); attach_printable context is not in the response.
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INTERNAL,
            "Something went wrong"
        );
        error.set_kind_name("InternalServerError");
        return error;
    }

private:
    ::app::context::AppContext& m_context;
    ProfileAcquirerInputSupport m_input;
};

} // namespace app::handlers::profile_acquirer
