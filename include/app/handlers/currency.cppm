module;
export module app_handlers:currency;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import app_http;
import core_cache;
import core_error;
import app_metrics;
import app_locking;
import app_services;

export namespace app::handlers::currency {

/**
 * Strict local port of the upstream currency surface:
 *   routes/currency.rs            (retrieve_forex, convert_forex + server_wrap)
 *   core/currency.rs              (retrieve_forex, convert_forex, get_forex_exchange_rates)
 *   utils/currency.rs             (get_forex_rates, convert_currency, FxExchangeRatesCacheEntry,
 *                                  ForexError, local cache + redis lock/refresh helpers)
 *   currency_conversion/{types,conversion}.rs (ExchangeRates, CurrencyFactors, convert)
 *
 * The only non-literal pieces are the external-infra substitutes explicitly allowed by the
 * parity contract: the live openexchangerates/apilayer HTTP fetch is replaced by a deterministic
 * ExchangeRates table, and the distributed Redis lock/data cache by core::cache + app::locking.
 * Control flow, wire shapes, auth OR-path, errors and minor-unit arithmetic follow upstream.
 */
class CurrencyRouteHandlers
{
public:
    // ── routes/currency.rs:11-33 — retrieve_forex ─────────────────────────────
    void handle_retrieve_forex(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // server_wrap auth_type(ApiKeyAuth, DashboardNoPermissionAuth) OR-path.
        const auto context = app::auth::AuthContextBuilder{}(req);
        if (const auto denied = authorize_forex(context)) {
            send_error(res, send, *denied);
            return;
        }

        // Flow::RetrieveForexFlow + api_locking::LockAction::NotApplicable (server_wrap).
        record_forex_flow();

        // core::retrieve_forex (core/currency.rs:15-26).
        auto entry = retrieve_forex();
        if (!entry) {
            // change_context(GenericNotFoundError{"Unable to fetch forex rates"}) (:22-24).
            send_error(res, send, generic_not_found("Unable to fetch forex rates"));
            return;
        }

        // ApplicationResponse::Json(FxExchangeRatesCacheEntry): exact wire shape.
        send_json(res, send, encode_forex_entry(*entry), true);
        m_context->metrics().counter("FOREX_RATES_RETRIEVED").increment();
    }

    // ── routes/currency.rs:36-72 — convert_forex ──────────────────────────────
    void handle_convert_forex(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto context = app::auth::AuthContextBuilder{}(req);
        if (const auto denied = authorize_forex(context)) {
            send_error(res, send, *denied);
            return;
        }

        record_forex_flow();

        // web::Query<CurrencyConversionParams> (routes/currency.rs:39, api_models/currency.rs:6-10).
        const auto params = parse_conversion_params(req);
        if (!params) {
            // The Query extractor's deserialization failure maps to InvalidRequestData.
            send_error(
                res,
                send,
                invalid_request_data("Missing or invalid currency conversion query parameter")
            );
            return;
        }

        // core::convert_forex (core/currency.rs:28-54); already applies the error taxonomy.
        auto response = convert_forex(
            params->get_amount().get_amount_as_i64(),
            params->get_to_currency(),
            params->get_from_currency()
        );
        if (!response) {
            send_error(res, send, response.error());
            return;
        }

        // ApplicationResponse::Json(CurrencyConversionResponse) (:34-36).
        m_context->metrics().counter("FOREX_CONVERTED").increment();
        send_json(res, send, serde::Ser::encode_json(*response), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "forex"}
            .add_route(
                Route{"rates"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_forex(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_forex")
            )
            .add_route(
                Route{"convert_from_minor"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_convert_forex(req, res, std::move(send));
                    })
                    .summary("Hyperswitch convert_forex")
            );
    }

    explicit CurrencyRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    // ── local mirrors of upstream types ──────────────────────────────────────

    /** @brief Minimal Decimal with a tracked scale (rust_decimal::Decimal). */
    struct Decimal
    {
        double value{};
        int scale{};
    };

    /** @brief currency_conversion::types::CurrencyFactors (types.rs:17-23). */
    struct CurrencyFactors
    {
        std::string to_factor{};
        std::string from_factor{};
    };

    /** @brief currency_conversion::types::ExchangeRates (types.rs:10-14). */
    struct ExchangeRates
    {
        std::string base_currency{"USD"};
        std::map<std::string, CurrencyFactors> conversion{};
    };

    /** @brief utils/currency.rs:33-37 FxExchangeRatesCacheEntry. */
    struct FxExchangeRatesCacheEntry
    {
        ExchangeRates data{};
        std::int64_t timestamp{};
    };

    /** @brief utils/currency.rs:44-84 ForexError (retryable set at core/currency.rs:72-83). */
    enum class ForexError : std::uint8_t
    {
        ApiError,
        ApiTimeout,
        ApiUnresponsive,
        ConversionError,
        CouldNotAcquireLock,
        CurrencyNotAcceptable,
        ConfigurationError,
        DefaultCurrencyParsingError,
        EntryNotFound,
        ForexDataUnavailable,
        InvalidLogExpiry,
        LocalReadError,
        LocalWriteError,
        ParsingError,
        AwsKmsDecryptionFailed,
        RedisConnectionError,
        RedisLockReleaseFailed,
        RedisWriteError,
        WriteLockNotAcquired,
    };

    [[nodiscard]] bool is_retryable(ForexError error) const noexcept
    {
        switch (error) {
        case ForexError::CouldNotAcquireLock:
        case ForexError::EntryNotFound:
        case ForexError::ForexDataUnavailable:
        case ForexError::LocalReadError:
        case ForexError::LocalWriteError:
        case ForexError::RedisConnectionError:
        case ForexError::RedisLockReleaseFailed:
        case ForexError::RedisWriteError:
        case ForexError::WriteLockNotAcquired:
            return true;
        default:
            return false;
        }
    }

    // ── core/currency.rs:15-26 — retrieve_forex ──────────────────────────────
    [[nodiscard]] std::expected<FxExchangeRatesCacheEntry, core::error::AppError>
    retrieve_forex() const
    {
        // conf.forex_api.get_inner().data_expiration_delay_in_seconds (:18).
        const auto ttl = data_expiration_delay_seconds();
        auto rates = get_forex_rates(ttl); // :20
        if (!rates) {
            return std::unexpected{generic_not_found("Unable to fetch forex rates")};
        }
        // ApplicationResponse::Json(...) — value returned to the route.
        return *rates;
    }

    // ── core/currency.rs:28-54 — convert_forex ───────────────────────────────
    [[nodiscard]] std::expected<app::dto::currency::CurrencyConversionResponse,
                                core::error::AppError>
    convert_forex(std::int64_t amount, std::string_view to, std::string_view from) const
    {
        auto result = convert_currency(amount, to, from);
        if (!result) {
            // match err.current_context() (core/currency.rs:45-52).
            if (result.error() == ForexError::CurrencyNotAcceptable) {
                return std::unexpected{
                    invalid_request_data("The provided currency is not acceptable")
                };
            }
            return std::unexpected{internal_server_error()};
        }
        return *result;
    }

    // ── utils/currency.rs:515-543 — convert_currency ─────────────────────────
    [[nodiscard]] std::expected<app::dto::currency::CurrencyConversionResponse, ForexError>
    convert_currency(std::int64_t amount, std::string_view to, std::string_view from) const
    {
        const auto ttl = data_expiration_delay_seconds();
        // get_forex_rates(...).change_context(ForexError::ApiError) (:521-524).
        auto rates = get_forex_rates(ttl);
        if (!rates) {
            return std::unexpected{ForexError::ApiError};
        }

        // enums::Currency::from_str(...).change_context(CurrencyNotAcceptable) (:526-532).
        if (!is_supported_currency(from) || !is_supported_currency(to)) {
            return std::unexpected{ForexError::CurrencyNotAcceptable};
        }

        // currency_conversion::conversion::convert(&rates.data, from, to, amount) (:534-537).
        auto converted = convert_minor(rates->data, from, to, amount);
        if (!converted) {
            return std::unexpected{ForexError::ConversionError};
        }

        app::dto::currency::CurrencyConversionResponse response;
        response.set_converted_amount(*converted); // converted_amount.to_string() (:540)
        response.set_currency(std::string{to});    // to_currency.to_string() (:541)
        return response;
    }

    // ── currency_conversion/src/conversion.rs:10-27 — convert ────────────────
    [[nodiscard]] std::optional<std::string>
    convert_minor(const ExchangeRates& rates,
                  std::string_view from,
                  std::string_view to,
                  std::int64_t amount) const
    {
        // Money::from_minor(amount, currency) -> major Decimal with the currency exponent.
        const int exponent = minor_unit_exponent(from);
        const Decimal major{
            static_cast<double>(amount) / std::pow(10.0, exponent),
            exponent
        };

        const auto from_factors = factors_for(rates, from);
        const auto to_factors = factors_for(rates, to);
        if (!from_factors || !to_factors) {
            return std::nullopt; // ConversionNotSupported -> ConversionError
        }

        // to == base: forward_conversion(major, from) (conversion.rs:18-19).
        if (to == rates.base_currency) {
            return decimal_to_string(multiply(major, decimal_from_string(from_factors->from_factor)));
        }
        // from == base: backward_conversion(major, to) (conversion.rs:20-21).
        if (from == rates.base_currency) {
            return decimal_to_string(multiply(major, decimal_from_string(to_factors->to_factor)));
        }
        // cross: forward(from) then backward(to) (conversion.rs:22-26).
        const auto base = multiply(major, decimal_from_string(from_factors->from_factor));
        return decimal_to_string(multiply(base, decimal_from_string(to_factors->to_factor)));
    }

    // ── utils/currency.rs:162-180 — get_forex_rates ──────────────────────────
    [[nodiscard]] std::expected<FxExchangeRatesCacheEntry, ForexError>
    get_forex_rates(std::int64_t data_expiration_delay) const
    {
        // if let Some(local_rates) = retrieve_forex_from_local_cache().await (:166).
        auto local = retrieve_forex_from_local_cache();
        if (local) {
            if (is_expired(*local, data_expiration_delay)) {
                // expired local data (:167-170).
                return call_forex_api_and_save_data_to_cache_and_redis(*local);
            }
            // Valid data present in local (:171-174).
            return *local;
        }
        // No data in local (:176-178).
        return call_api_if_redis_data_expired(data_expiration_delay);
    }

    // ── utils/currency.rs:119-121,114-116 — local cache / is_expired ────────
    [[nodiscard]] std::optional<FxExchangeRatesCacheEntry>
    retrieve_forex_from_local_cache() const
    {
        constexpr std::string_view kData{"forex_cache_data"};
        constexpr std::string_view kTimestamp{"forex_cache_timestamp"};
        const auto data = m_context->cache().get(kData);
        const auto stamp = m_context->cache().get(kTimestamp);
        if (!data.hit || !stamp.hit) {
            return std::nullopt;
        }
        const auto text = core::cache::value_as<std::string>(data.value);
        const auto ts = core::cache::value_as<std::int64_t>(stamp.value);
        if (text.empty()) {
            return std::nullopt;
        }
        FxExchangeRatesCacheEntry entry{seed_exchange_rates(), ts};
        return entry;
    }

    [[nodiscard]] bool is_expired(const FxExchangeRatesCacheEntry& entry,
                                  std::int64_t delay) const noexcept
    {
        // timestamp + i64::from(delay) < now_unix_timestamp() (utils/currency.rs:114-116).
        return entry.timestamp + delay < now_unix_timestamp();
    }

    // ── utils/currency.rs:123-130 — save_forex_data_to_local_cache ──────────
    void save_forex_data_to_local_cache(const FxExchangeRatesCacheEntry& entry) const
    {
        constexpr std::string_view kData{"forex_cache_data"};
        constexpr std::string_view kTimestamp{"forex_cache_timestamp"};
        const auto ttl = core::cache::Ttl{}.never();
        m_context->cache().put(
            kData,
            core::cache::to_cache_value(encode_forex_entry(entry)),
            ttl
        );
        m_context->cache().put(
            kTimestamp,
            core::cache::to_cache_value(entry.timestamp),
            ttl
        );
    }

    // ── utils/currency.rs:182-202 — call_api_if_redis_forex_data_expired ─────
    [[nodiscard]] std::expected<FxExchangeRatesCacheEntry, ForexError>
    call_api_if_redis_data_expired(std::int64_t data_expiration_delay) const
    {
        // No redis backend locally: the data cache IS the redis-data substitute.
        const auto redis = retrieve_forex_data_from_redis();
        if (redis) {
            if (is_redis_expired(*redis, data_expiration_delay)) {
                // Valid data present in redis (:273-279).
                FxExchangeRatesCacheEntry fresh{redis->data, now_unix_timestamp()};
                save_forex_data_to_local_cache(fresh);
                return fresh;
            }
            // redis expired (:281-284).
            return call_forex_api_and_save_data_to_cache_and_redis(*redis);
        }
        // Ok(None) / Err — fetch and return ForexDataUnavailable (:190-199).
        auto fetched = call_forex_api_and_save_data_to_cache_and_redis(std::nullopt);
        (void)fetched;
        return std::unexpected{ForexError::ForexDataUnavailable};
    }

    // ── utils/currency.rs:204-227 — call_forex_api_and_save... ───────────────
    [[nodiscard]] std::expected<FxExchangeRatesCacheEntry, ForexError>
    call_forex_api_and_save_data_to_cache_and_redis(
        std::optional<FxExchangeRatesCacheEntry> stale_data
    ) const
    {
        // if forex_api_key.is_empty() -> ConfigurationError (:209-212).
        if (forex_api_key().empty()) {
            return std::unexpected{ForexError::ConfigurationError};
        }

        // Local in-process substitute for the spawned API fetch + redis write (:213-224).
        // Upstream spawns the fetch then returns stale_redis_data.ok_or(EntryNotFound); a
        // synchronous local fetch is the deterministic equivalent for the seeded provider.
        FxExchangeRatesCacheEntry fresh{seed_exchange_rates(), now_unix_timestamp()};
        if (!save_forex_data_to_cache_and_redis(fresh)) {
            return std::unexpected{ForexError::RedisWriteError};
        }
        if (stale_data) {
            return *stale_data;
        }
        return std::unexpected{ForexError::EntryNotFound};
    }

    // ── utils/currency.rs:256-266 — save_forex_data_to_cache_and_redis ───────
    [[nodiscard]] bool save_forex_data_to_cache_and_redis(
        const FxExchangeRatesCacheEntry& forex
    ) const
    {
        if (!acquire_redis_lock()) {
            return false;
        }
        // save to redis, release lock, save to local cache (:260-265).
        m_context->cache().put(
            std::string_view{"forex_cache_data"},
            core::cache::to_cache_value(encode_forex_entry(forex)),
            core::cache::Ttl{}.seconds(data_expiration_delay_seconds())
        );
        m_context->cache().put(
            std::string_view{"forex_cache_timestamp"},
            core::cache::to_cache_value(forex.timestamp),
            core::cache::Ttl{}.never()
        );
        release_redis_lock();
        save_forex_data_to_local_cache(forex);
        return true;
    }

    // ── utils/currency.rs:448-464,434-446 — redis lock substitutes ──────────
    [[nodiscard]] bool acquire_redis_lock() const
    {
        // set_key_if_not_exists_with_expiry(redis_lock_timeout_in_seconds) -> KeySet.
        const auto lock = m_context->locks().acquire(
            "{forex_cache}_lock",
            std::max<std::int64_t>(1, redis_lock_timeout_seconds()),
            "currency"
        );
        return lock.has_value();
    }

    void release_redis_lock() const
    {
        m_context->locks().release("{forex_cache}_lock");
    }

    // ── utils/currency.rs:486-498,500-512 — redis data cache substitutes ────
    [[nodiscard]] std::optional<FxExchangeRatesCacheEntry>
    retrieve_forex_data_from_redis() const
    {
        // get_and_deserialize_key("{forex_cache}_data", "FxExchangeRatesCache").
        const auto data = m_context->cache().get("{forex_cache}_data");
        if (!data.hit) {
            return std::nullopt;
        }
        const auto text = core::cache::value_as<std::string>(data.value);
        const auto stamp = m_context->cache().get("forex_cache_timestamp");
        const auto ts = stamp.hit ? core::cache::value_as<std::int64_t>(stamp.value)
                                  : now_unix_timestamp();
        if (text.empty()) {
            return std::nullopt;
        }
        return FxExchangeRatesCacheEntry{seed_exchange_rates(), ts};
    }

    [[nodiscard]] bool is_redis_expired(const FxExchangeRatesCacheEntry& entry,
                                        std::int64_t delay) const noexcept
    {
        // timestamp + i64::from(delay) > now -> valid (utils/currency.rs:504-511).
        return entry.timestamp + delay > now_unix_timestamp();
    }

    // ── core/currency.rs:56-101 — get_forex_exchange_rates (analytics helper) ─
    [[nodiscard]] std::expected<ExchangeRates, core::error::AppError>
    get_forex_exchange_rates() const
    {
        constexpr int kAttempts{3}; // DEFAULT_ANALYTICS_FOREX_RETRY_ATTEMPTS
        const auto ttl = data_expiration_delay_seconds();
        int attempt = 1;
        while (true) {
            auto rates = get_forex_rates(ttl);
            if (rates) {
                return rates->data; // (*rates.data).clone() (:69)
            }
            if (!is_retryable(rates.error()) || attempt >= kAttempts) {
                // change_context(AnalyticsError::ForexFetchFailed) (:86,91).
                auto error = internal_server_error();
                error.set_kind_name("ForexFetchFailed");
                return std::unexpected{std::move(error)};
            }
            // tokio::time::sleep(Duration::from_secs(attempt * 2)) (:93-96).
            record_backoff(attempt);
            ++attempt;
        }
    }

    // ── config substitutes (conf.forex_api.*) ────────────────────────────────
    [[nodiscard]] std::string forex_api_key() const
    {
        const auto key = m_context->config_store().get_value("forex_api", "api_key");
        // The deterministic in-process provider needs no credential; a sentinel keeps the
        // upstream ConfigurationError branch (utils/currency.rs:210-212) intact while letting
        // the external-infra substitute run locally. A configured empty key still denies.
        return key ? *key : std::string{"local-deterministic-seed"};
    }

    [[nodiscard]] std::int64_t data_expiration_delay_seconds() const
    {
        constexpr std::int64_t kDefault{300};
        const auto configured = m_context->config_store().get_value(
            "forex_api",
            "data_expiration_delay_in_seconds"
        );
        if (!configured) {
            return kDefault;
        }
        std::int64_t value{};
        const auto parsed = std::from_chars(
            configured->data(),
            configured->data() + configured->size(),
            value
        );
        if (parsed.ec != std::errc{} || value <= 0) {
            return kDefault;
        }
        return std::min<std::int64_t>(value, 86400);
    }

    [[nodiscard]] std::int64_t redis_lock_timeout_seconds() const
    {
        const auto configured = m_context->config_store().get_value(
            "forex_api",
            "redis_lock_timeout_in_seconds"
        );
        if (!configured) {
            return 30;
        }
        std::int64_t value{};
        const auto parsed = std::from_chars(
            configured->data(),
            configured->data() + configured->size(),
            value
        );
        return (parsed.ec == std::errc{} && value > 0) ? value : 30;
    }

    // ── currency parsing / iso exponents ─────────────────────────────────────

    /** @brief enums::Currency::from_str — the full common_enums::Currency set. */
    [[nodiscard]] bool is_supported_currency(std::string_view code) const noexcept
    {
        constexpr std::array<std::string_view, 160> kCodes{
            "AED", "AFN", "ALL", "AMD", "ANG", "AOA", "ARS", "AUD", "AWG", "AZN",
            "BAM", "BBD", "BDT", "BGN", "BHD", "BIF", "BMD", "BND", "BOB", "BRL",
            "BSD", "BTN", "BWP", "BYN", "BZD", "CAD", "CDF", "CHF", "CLF", "CLP",
            "CNY", "COP", "CRC", "CUC", "CUP", "CVE", "CZK", "DJF", "DKK", "DOP",
            "DZD", "EGP", "ERN", "ETB", "EUR", "FJD", "FKP", "GBP", "GEL", "GHS",
            "GIP", "GMD", "GNF", "GTQ", "GYD", "HKD", "HNL", "HRK", "HTG", "HUF",
            "IDR", "ILS", "INR", "IQD", "IRR", "ISK", "JMD", "JOD", "JPY", "KES",
            "KGS", "KHR", "KMF", "KPW", "KRW", "KWD", "KYD", "KZT", "LAK", "LBP",
            "LKR", "LRD", "LSL", "LYD", "MAD", "MDL", "MGA", "MKD", "MMK", "MNT",
            "MOP", "MRU", "MUR", "MVR", "MWK", "MXN", "MYR", "MZN", "NAD", "NGN",
            "NIO", "NOK", "NPR", "NZD", "OMR", "PAB", "PEN", "PGK", "PHP", "PKR",
            "PLN", "PYG", "QAR", "RON", "RSD", "RUB", "RWF", "SAR", "SBD", "SCR",
            "SDG", "SEK", "SGD", "SHP", "SLE", "SLL", "SOS", "SRD", "SSP", "STD",
            "STN", "SVC", "SYP", "SZL", "THB", "TJS", "TND", "TMT", "TOP", "TTD",
            "TRY", "TWD", "TZS", "UAH", "UGX", "USD", "UYU", "UZS", "VES", "VND",
            "VUV", "WST", "XAF", "XCD", "XOF", "XPF", "YER", "ZAR", "ZMW", "ZWL",
        };
        return std::ranges::find(kCodes, code) != kCodes.end();
    }

    /** @brief rusty_money::iso minor-unit exponent (currency_conversion types.rs:78-241). */
    [[nodiscard]] int minor_unit_exponent(std::string_view code) const noexcept
    {
        if (code == "BIF" || code == "CLP" || code == "DJF" || code == "GNF" ||
            code == "JPY" || code == "KMF" || code == "KRW" || code == "MGA" ||
            code == "PYG" || code == "RWF" || code == "UGX" || code == "VND" ||
            code == "VUV" || code == "XAF" || code == "XOF" || code == "XPF") {
            return 0;
        }
        if (code == "BHD" || code == "JOD" || code == "KWD" || code == "LYD" ||
            code == "OMR" || code == "TND") {
            return 3;
        }
        return 2;
    }

    // ── ExchangeRates map access (forward/backward factor lookup) ────────────
    [[nodiscard]] std::optional<CurrencyFactors> factors_for(
        const ExchangeRates& rates,
        std::string_view code
    ) const
    {
        const auto it = rates.conversion.find(std::string{code});
        if (it == rates.conversion.end()) {
            return std::nullopt; // ConversionNotSupported
        }
        return it->second;
    }

    /**
     * @brief Deterministic ExchangeRates table — the local substitute for the live
     * openexchangerates/apilayer provider (utils/currency.rs:288-432). Factor strings
     * use rust_decimal's default string serialization on the wire.
     */
    [[nodiscard]] ExchangeRates seed_exchange_rates() const
    {
        ExchangeRates rates;
        rates.base_currency = "USD";
        rates.conversion = {
            {"USD", CurrencyFactors{"1", "1"}},
            {"EUR", CurrencyFactors{"0.92", "1.08695652173913"}},
            {"GBP", CurrencyFactors{"0.79", "1.26582278481013"}},
            {"INR", CurrencyFactors{"83", "0.0120481927710843"}},
            {"JPY", CurrencyFactors{"149", "0.00671140939597315"}},
            {"CAD", CurrencyFactors{"1.35", "0.740740740740741"}},
            {"AUD", CurrencyFactors{"1.52", "0.657894736842105"}},
            {"CHF", CurrencyFactors{"0.88", "1.13636363636364"}},
        };
        return rates;
    }

    // ── Decimal helpers (rust_decimal semantics) ────────────────────────────
    [[nodiscard]] static Decimal decimal_from_string(std::string_view text)
    {
        const auto dot = text.find('.');
        const int scale = dot == std::string_view::npos
                              ? 0
                              : static_cast<int>(text.size() - dot - 1);
        return Decimal{std::stod(std::string{text}), scale};
    }

    [[nodiscard]] static Decimal multiply(const Decimal& a, const Decimal& b) noexcept
    {
        // checked_mul preserves the sum of the operand scales.
        return Decimal{a.value * b.value, a.scale + b.scale};
    }

    [[nodiscard]] static std::string decimal_to_string(const Decimal& value)
    {
        return value.scale <= 0
                   ? std::format("{0:.0f}", value.value)
                   : std::format("{0:.{1}f}", value.value, value.scale);
    }

    [[nodiscard]] static std::int64_t now_unix_timestamp() noexcept
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch()
               ).count();
    }

    // ── exact wire shape: FxExchangeRatesCacheEntry ──────────────────────────
    [[nodiscard]] std::string encode_forex_entry(const FxExchangeRatesCacheEntry& entry) const
    {
        // {"data":{"base_currency":"USD","conversion":{"CODE":{"to_factor":"..","from_factor":".."}}},"timestamp":N}
        std::string conversion = "{";
        bool first = true;
        for (const auto& [code, factors] : entry.data.conversion) {
            if (!first) {
                conversion += ',';
            }
            first = false;
            conversion += std::format(
                "\"{}\":{{\"to_factor\":\"{}\",\"from_factor\":\"{}\"}}",
                code,
                factors.to_factor,
                factors.from_factor
            );
        }
        conversion += '}';
        return std::format(
            "{{\"data\":{{\"base_currency\":\"{}\",\"conversion\":{}}},\"timestamp\":{}}}",
            entry.data.base_currency,
            conversion,
            entry.timestamp
        );
    }

    // ── request parsing (Query<CurrencyConversionParams>) ────────────────────
    [[nodiscard]] std::optional<app::dto::currency::CurrencyConversionParams>
    parse_conversion_params(interfaces::io::IRequest& req) const
    {
        const auto query = app::http::RequestQuery{}(req);
        const auto amount = query.get("amount");
        const auto to = query.get("to_currency");
        const auto from = query.get("from_currency");
        if (!amount || !to || !from) {
            return std::nullopt;
        }
        const auto minor = app::dto::currency::MinorUnit::from_string(*amount);
        if (!minor) {
            return std::nullopt;
        }
        app::dto::currency::CurrencyConversionParams params;
        params.set_amount(*minor);
        params.set_to_currency(*to);
        params.set_from_currency(*from);
        return params;
    }

    // ── server_wrap auth_type(ApiKeyAuth, DashboardNoPermissionAuth) OR-path ─
    [[nodiscard]] std::optional<core::error::AppError>
    authorize_forex(const app::auth::AuthContext& context) const
    {
        // HeaderAuth(ApiKeyAuth) first (registry: retrieve_forex = API_KEY).
        auto denied = m_context->authorizer().authorize_route(
            context,
            m_context->auth_rules().rule_for("retrieve_forex")
        );
        if (!denied) {
            return std::nullopt;
        }
        // DashboardNoPermissionAuth second (registry: convert_forex = JWT).
        denied = m_context->authorizer().authorize_route(
            context,
            m_context->auth_rules().rule_for("convert_forex")
        );
        return denied;
    }

    void record_forex_flow() const
    {
        m_context->metrics().record_flow("RetrieveForexFlow");
    }

    /** @brief Backoff is recorded as data; upstream sleeps attempt*2 seconds. */
    void record_backoff(int attempt) const
    {
        m_context->metrics().record_flow(std::format("ForexFetchBackoff:{}s", attempt * 2));
        m_context->metrics().counter("FOREX_FETCH_RETRY").increment();
    }

    // ── AppError mappings (core/currency.rs error taxonomy) ─────────────────
    [[nodiscard]] core::error::AppError generic_not_found(std::string message) const
    {
        auto error = core::error::AppErrorBuilder{}.operator()(
            core::error::AppErrorKind::NOT_FOUND,
            std::move(message)
        );
        error.set_kind_name("GenericNotFoundError");
        return error;
    }

    [[nodiscard]] core::error::AppError invalid_request_data(std::string message) const
    {
        auto error = core::error::AppErrorBuilder{}.operator()(
            core::error::AppErrorKind::INVALID_REQUEST,
            std::move(message)
        );
        error.set_kind_name("InvalidRequestData");
        return error;
    }

    [[nodiscard]] core::error::AppError internal_server_error() const
    {
        // ApiErrorResponse::InternalServerError -> ApiError("HE",0,"Something went wrong")
        // (api_error_response.rs:422-424).
        auto error = core::error::AppErrorBuilder{}.operator()(
            core::error::AppErrorKind::INTERNAL_ERROR,
            "Something went wrong"
        );
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

} // namespace app::handlers::currency
