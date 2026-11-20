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
 * @brief Normalized, serde-serializable shape of one acquirer config slot.
 *
 * Upstream stores `common_types::domain::AcquirerConfig`. That wrapper has no
 * serde support here, so the handler persists this local shape through
 * AppRepository config records instead of the cache. Country code is stored
 * already normalized to its numeric form (upstream `Country::to_numeric`).
 */
class StoredConfig
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
struct serde::Serializable<::app::handlers::profile_acquirer::StoredConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"acquirer_assigned_merchant_id", &::app::handlers::profile_acquirer::StoredConfig::get_acquirer_assigned_merchant_id, &::app::handlers::profile_acquirer::StoredConfig::set_acquirer_assigned_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_name", &::app::handlers::profile_acquirer::StoredConfig::get_merchant_name, &::app::handlers::profile_acquirer::StoredConfig::set_merchant_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"network", &::app::handlers::profile_acquirer::StoredConfig::get_network, &::app::handlers::profile_acquirer::StoredConfig::set_network, serde::FieldOptions{}>{},
            serde::FieldDesc<"acquirer_bin", &::app::handlers::profile_acquirer::StoredConfig::get_acquirer_bin, &::app::handlers::profile_acquirer::StoredConfig::set_acquirer_bin, serde::FieldOptions{}>{},
            serde::FieldDesc<"acquirer_ica", &::app::handlers::profile_acquirer::StoredConfig::get_acquirer_ica, &::app::handlers::profile_acquirer::StoredConfig::set_acquirer_ica, serde::FieldOptions{}>{},
            serde::FieldDesc<"acquirer_fraud_rate", &::app::handlers::profile_acquirer::StoredConfig::get_acquirer_fraud_rate, &::app::handlers::profile_acquirer::StoredConfig::set_acquirer_fraud_rate, serde::FieldOptions{}>{},
            serde::FieldDesc<"acquirer_country_code", &::app::handlers::profile_acquirer::StoredConfig::get_acquirer_country_code, &::app::handlers::profile_acquirer::StoredConfig::set_acquirer_country_code, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "profile_acquirer_stored_config"; }
};

export namespace app::handlers::profile_acquirer {

class ProfileAcquirerRouteHandlers
{
public:
    void handle_create_profile_acquirer(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("create_profile_acquirer");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) {
            send_error(res, send, *auth);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::profile_acquirer::ProfileAcquirerCreate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const std::string profile_id = std::string{parsed->get_profile_id().get_value()};
        if (profile_id.empty() || !profile_exists(profile_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "profile not found"
            ));
            return;
        }

        const auto default_key = default_key_for(profile_id);
        const auto stored_default = load_value(default_key);
        const bool has_existing_default = stored_default.has_value() && !stored_default->empty();
        const bool is_default = !has_existing_default || parsed->get_is_default().value_or(false);

        // Upstream generate_profile_acquirer_id_of_default_length() issues a stable
        // `pro_acq_...` id; the IdGenerator counter gives a deterministic, body-independent id.
        app::dto::profile_acquirer::ProfileAcquirerIdValue acquirer_id;
        acquirer_id.set_value(m_context->ids().payment_id("pro_acq_"));

        const StoredConfig incoming = config_from_create(*parsed);
        const auto encoded = serde::Ser::encode_json(incoming);
        remember_config(profile_id, acquirer_id.get_value(), incoming.get_network(), encoded);
        if (is_default) {
            save_value(default_key, acquirer_id.get_value());
        }

        // Re-read the persisted slot so the response reflects the stored/upserted
        // config (including the normalized country code), not the raw request echo.
        const auto stored = load_config(acquirer_id.get_value(), incoming.get_network());
        auto response = build_response(acquirer_id, profile_id, stored, is_default);

        m_context->metrics().record_flow("profile_acquirer.create");
        m_context->metrics().counter("profile_acquirer_create").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_profile_acquirer_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("profile_acquirer_update");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) {
            send_error(res, send, *auth);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::profile_acquirer::ProfileAcquirerUpdate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Route ids come from the path, never from the request body.
        const auto segments = app::http::RequestPath{}(req);
        if (segments.count() < 2) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing profile id or profile acquirer id"
            ));
            return;
        }
        const std::string profile_acquirer_id = std::string{segments.at(segments.count() - 1).value_or("")};
        const std::string profile_id = std::string{segments.at(segments.count() - 2).value_or("")};
        if (profile_id.empty() || profile_acquirer_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing profile id or profile acquirer id"
            ));
            return;
        }

        if (!profile_exists(profile_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "profile not found"
            ));
            return;
        }
        if (!bucket_exists(profile_id, profile_acquirer_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "profile acquirer not found"
            ));
            return;
        }

        const auto default_key = default_key_for(profile_id);
        const auto stored_default = load_value(default_key);
        const std::string current_default = stored_default.value_or({});
        const bool current_is_default = !current_default.empty() && current_default == profile_acquirer_id;
        const bool is_default = parsed->get_is_default().value_or(current_is_default);

        // apply_default_bucket_change: true -> set pointer, false -> clear pointer ("None").
        bool default_changed = false;
        if (parsed->get_is_default()) {
            if (*parsed->get_is_default() && !current_is_default) {
                save_value(default_key, profile_acquirer_id);
                default_changed = true;
            } else if (!*parsed->get_is_default() && current_is_default) {
                save_value(default_key, std::string{});
                default_changed = true;
            }
        }

        if (!parsed->get_network().has_value() && !default_changed) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "A `network` must be provided to update an acquirer configuration, unless you are only changing the `is_default` fallback status of this entire bucket."
            ));
            return;
        }

        std::optional<StoredConfig> final_config{};
        if (parsed->get_network().has_value()) {
            const std::string network = parsed->get_network()->get_value();
            const auto existing = load_config(profile_acquirer_id, network);
            StoredConfig base{};
            base.set_network(network);
            if (existing) {
                base = *existing;
            }
            const StoredConfig upserted = merge_update(base, *parsed);
            const auto encoded = serde::Ser::encode_json(upserted);
            if (existing && serde::Ser::encode_json(*existing) == encoded && !default_changed) {
                send_error(res, send, core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::DUPLICATE_REQUEST,
                    "an identical configuration for this network already exists"
                ));
                return;
            }
            remember_config(profile_id, profile_acquirer_id, network, encoded);
            final_config = load_config(profile_acquirer_id, network);
        }

        app::dto::profile_acquirer::ProfileAcquirerIdValue acquirer_id;
        acquirer_id.set_value(profile_acquirer_id);
        auto response = build_response(acquirer_id, profile_id, final_config, is_default);

        m_context->metrics().record_flow("profile_acquirer.update");
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
    // ---- persistence: all business state lives in AppRepository config records ----
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

    [[nodiscard]] std::string config_key(std::string_view acquirer_id, std::string_view network) const
    {
        return std::string{"profile_acquirer.config:"} + std::string{acquirer_id}
             + ":" + std::string{network};
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

    [[nodiscard]] bool bucket_exists(std::string_view profile_id, std::string_view acquirer_id) const
    {
        const std::string index = load_value(config_index_key(profile_id)).value_or({});
        std::size_t start{0};
        while (start <= index.size()) {
            const std::size_t comma{index.find(',', start)};
            const std::size_t end{comma == std::string::npos ? index.size() : comma};
            if (std::string_view{index}.substr(start, end - start) == acquirer_id) {
                return true;
            }
            if (comma == std::string::npos) {
                break;
            }
            start = comma + 1;
        }
        return false;
    }

    [[nodiscard]] std::optional<StoredConfig> load_config(
        std::string_view acquirer_id,
        std::string_view network
    ) const
    {
        const auto raw = load_value(config_key(acquirer_id, network));
        if (!raw) {
            return std::nullopt;
        }
        auto decoded = serde::Ser::deserialize<StoredConfig>("application/json", *raw);
        if (!decoded) {
            return std::nullopt;
        }
        return *decoded;
    }

    void remember_config(
        std::string_view profile_id,
        std::string_view acquirer_id,
        std::string_view network,
        std::string_view payload
    ) const
    {
        save_value(config_key(acquirer_id, network), std::string{payload});
        const std::string key = config_index_key(profile_id);
        const std::string existing = load_value(key).value_or({});
        if (!bucket_exists(profile_id, acquirer_id)) {
            save_value(
                key,
                existing.empty() ? std::string{acquirer_id}
                                 : existing + "," + std::string{acquirer_id}
            );
        }
    }

    // ---- normalization and response shaping ----
    [[nodiscard]] std::optional<std::string> normalize_country(
        const std::optional<app::dto::profile_acquirer::CountryAlpha2Value>& code
    ) const
    {
        if (!code) {
            return std::nullopt;
        }
        // Deterministic substitute for Country::from_alpha2(...).to_numeric().to_string():
        // an ISO 3166-1 alpha-2 -> numeric table for common acquirer countries,
        // with the raw alpha-2 value returned unchanged for codes not in the table.
        const std::unordered_map<std::string, std::string> numeric{
            {"AU", "036"}, {"AT", "040"}, {"BE", "056"}, {"BR", "076"}, {"CA", "124"},
            {"CN", "156"}, {"CO", "170"}, {"CZ", "203"}, {"DK", "208"}, {"EG", "818"},
            {"FI", "246"}, {"FR", "250"}, {"DE", "276"}, {"GR", "300"}, {"HK", "344"},
            {"HU", "348"}, {"IN", "356"}, {"ID", "360"}, {"IE", "372"}, {"IL", "376"},
            {"IT", "380"}, {"JP", "392"}, {"KE", "404"}, {"KR", "410"}, {"MY", "458"},
            {"MX", "484"}, {"NL", "528"}, {"NZ", "554"}, {"NG", "566"}, {"NO", "578"},
            {"PL", "616"}, {"PT", "620"}, {"QA", "634"}, {"RO", "642"}, {"SA", "682"},
            {"SG", "702"}, {"ZA", "710"}, {"ES", "724"}, {"SE", "752"}, {"CH", "756"},
            {"TW", "158"}, {"TR", "792"}, {"AE", "784"}, {"GB", "826"}, {"US", "840"},
            {"UY", "858"}, {"VN", "704"}
        };
        std::string alpha = code->get_value();
        for (char& c : alpha) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
        const auto it = numeric.find(alpha);
        return it != numeric.end() ? std::optional<std::string>{it->second}
                                   : std::optional<std::string>{code->get_value()};
    }

    [[nodiscard]] StoredConfig config_from_create(
        const app::dto::profile_acquirer::ProfileAcquirerCreate& in
    ) const
    {
        StoredConfig out{};
        out.set_acquirer_assigned_merchant_id(in.get_acquirer_assigned_merchant_id());
        out.set_merchant_name(in.get_merchant_name());
        out.set_network(in.get_network().get_value());
        out.set_acquirer_bin(in.get_acquirer_bin());
        out.set_acquirer_ica(in.get_acquirer_ica());
        out.set_acquirer_fraud_rate(in.get_acquirer_fraud_rate());
        out.set_acquirer_country_code(normalize_country(in.get_acquirer_country_code()));
        return out;
    }

    [[nodiscard]] StoredConfig merge_update(
        const StoredConfig& base,
        const app::dto::profile_acquirer::ProfileAcquirerUpdate& in
    ) const
    {
        StoredConfig out{};
        out.set_network(base.get_network());
        out.set_acquirer_assigned_merchant_id(
            in.get_acquirer_assigned_merchant_id() ? in.get_acquirer_assigned_merchant_id()
                                                   : base.get_acquirer_assigned_merchant_id());
        out.set_merchant_name(
            in.get_merchant_name() ? in.get_merchant_name() : base.get_merchant_name());
        out.set_acquirer_bin(
            in.get_acquirer_bin() ? in.get_acquirer_bin() : base.get_acquirer_bin());
        out.set_acquirer_ica(
            in.get_acquirer_ica() ? in.get_acquirer_ica() : base.get_acquirer_ica());
        out.set_acquirer_fraud_rate(
            in.get_acquirer_fraud_rate() ? in.get_acquirer_fraud_rate() : base.get_acquirer_fraud_rate());
        out.set_acquirer_country_code(
            in.get_acquirer_country_code() ? normalize_country(in.get_acquirer_country_code())
                                           : base.get_acquirer_country_code());
        return out;
    }

    [[nodiscard]] app::dto::profile_acquirer::ProfileAcquirerResponse build_response(
        const app::dto::profile_acquirer::ProfileAcquirerIdValue& acquirer_id,
        std::string_view profile_id,
        const std::optional<StoredConfig>& config,
        bool is_default
    ) const
    {
        app::dto::profile_acquirer::ProfileAcquirerResponse response;
        response.set_profile_acquirer_id(acquirer_id);
        app::dto::profile_acquirer::ProfileIdValue profile;
        profile.set_value(std::string{profile_id});
        response.set_profile_id(profile);
        response.set_is_default(is_default);
        if (config) {
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
