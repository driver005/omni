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

class CurrencyRouteHandlers
{
public:
    void handle_retrieve_forex(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream routes/currency.rs:19-29 runs server_wrap with HeaderAuth(ApiKeyAuth)
        // OR DashboardNoPermissionAuth. The registry rows are the declarative form of
        // those two paths; try the API-key path first, then the dashboard (JWT) path.
        const auto context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for("retrieve_forex");
        auto auth = m_context->authorizer().authorize_route(context, rule);
        if (auth) {
            auth = m_context->authorizer().authorize_route(
                context,
                m_context->auth_rules().rule_for("convert_forex")
            );
        }
        if (auth) { send_error(res, send, *auth); return; }

        // Upstream retrieve_forex takes no request params (routes/currency.rs:11-18);
        // the cached FxExchangeRatesCacheEntry is the whole response.
        auto rates = load_rates();
        if (!rates) {
            // Every get_forex_rates failure collapses to GenericNotFoundError
            // "Unable to fetch forex rates" (core/currency.rs:20-24).
            auto error = core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND,
                "Unable to fetch forex rates"
            );
            error.set_kind_name("GenericNotFoundError");
            send_error(res, send, error);
            return;
        }

        m_context->metrics().record_flow("RetrieveForexFlow");
        m_context->metrics().counter("FOREX_RATES_RETRIEVED").increment();
        send_json(res, send, *rates, true);
    }

    void handle_convert_forex(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for("convert_forex");
        auto auth = m_context->authorizer().authorize_route(context, rule);
        if (auth) {
            auth = m_context->authorizer().authorize_route(
                context,
                m_context->auth_rules().rule_for("retrieve_forex")
            );
        }
        if (auth) { send_error(res, send, *auth); return; }

        // Upstream GET /forex/convert_from_minor reads Query<CurrencyConversionParams>
        // (routes/currency.rs:36-40, api_models/currency.rs:6-10).
        const auto query = app::http::RequestQuery{}(req);
        const auto amount_param = query.get("amount");
        const auto to_param = query.get("to_currency");
        const auto from_param = query.get("from_currency");
        if (!amount_param || !to_param || !from_param) {
            send_error(
                res,
                send,
                invalid_request_data("Missing required currency conversion query parameter")
            );
            return;
        }

        // Normalize query strings into the local serializable DTO. The shared
        // MinorUnitValue wrapper holds a string and has no MinorUnit/i64 converter,
        // so the integer parse happens after normalization (documented substitute).
        using Input = app::dto::currency::CurrencyConversionParams;
        Input params;
        app::dto::currency::MinorUnitValue minor_unit;
        minor_unit.set_value(*amount_param);
        params.set_amount(std::move(minor_unit));
        params.set_to_currency(*to_param);
        params.set_from_currency(*from_param);

        const auto& amount_value = params.get_amount().get_value();
        const auto& to_currency = params.get_to_currency();
        const auto& from_currency = params.get_from_currency();

        // Upstream convert_currency reads the forex cache first (core/currency.rs:37-44).
        // Any ForexCacheError other than CurrencyNotAcceptable maps to
        // InternalServerError (core/currency.rs:45-52).
        auto rates = load_rates();
        if (!rates) {
            send_error(
                res,
                send,
                internal_server_error("Unable to fetch forex rates")
            );
            return;
        }

        // MinorUnit::get_amount_as_i64 returns the integer minor-unit amount
        // (common_utils/src/types.rs:426-432); parse it without going through double.
        const auto minor = parse_minor_i64(amount_value);
        const auto from_rate = factors_for(from_currency);
        const auto to_rate = factors_for(to_currency);
        if (!minor || !from_rate || !to_rate) {
            send_error(
                res,
                send,
                invalid_request_data("The provided currency is not acceptable")
            );
            return;
        }

        const auto [from_to_factor, from_from_factor, from_exp] = *from_rate;
        const auto [to_to_factor, to_from_factor, to_exp] = *to_rate;
        (void)from_to_factor;
        (void)to_from_factor;
        (void)to_exp;

        // Port of currency_conversion::conversion::convert():
        // from_currency -> base (forward_conversion) -> to_currency (backward_conversion).
        const auto major = static_cast<double>(*minor) / std::pow(10.0, from_exp);
        double converted = 0.0;
        if (to_currency == "USD") {
            converted = major * from_from_factor;
        } else if (from_currency == "USD") {
            converted = major * to_to_factor;
        } else {
            converted = major * from_from_factor * to_to_factor;
        }

        const auto converted_text = std::format("{:.10}", converted);

        app::dto::currency::CurrencyConversionResponse response;
        response.set_converted_amount(converted_text);
        response.set_currency(to_currency);

        m_context->metrics().record_flow("RetrieveForexFlow");
        m_context->metrics().counter("FOREX_CONVERTED").increment();
        const auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/forex"}
            .add_route(
                Route{"/rates"}
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
                Route{"/convert_from_minor"}
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
    /** @brief Typed wire shape of FxExchangeRatesCacheEntry (utils/currency.rs:33-37). */
    struct CurrencyFactorsView
    {
        double to_factor{};
        double from_factor{};
    };

    struct ExchangeRatesView
    {
        std::string base_currency{};
        std::map<std::string, CurrencyFactorsView> conversion{};
    };

    struct ForexRatesView
    {
        ExchangeRatesView data{};
        std::int64_t timestamp{};
    };

    /** @brief Upstream InvalidRequestData error mapping (core/currency.rs:46-52). */
    [[nodiscard]] core::error::AppError invalid_request_data(std::string message) const
    {
        auto error = core::error::AppErrorBuilder{}.operator()(
            core::error::AppErrorKind::INVALID_REQUEST,
            std::move(message)
        );
        error.set_kind_name("InvalidRequestData");
        return error;
    }

    /** @brief Upstream InternalServerError mapping for forex cache/provider/lock failures. */
    [[nodiscard]] core::error::AppError internal_server_error(std::string message) const
    {
        auto error = core::error::AppErrorBuilder{}.operator()(
            core::error::AppErrorKind::INTERNAL_ERROR,
            std::move(message)
        );
        error.set_kind_name("InternalServerError");
        return error;
    }

    /** @brief conf.forex_api.data_expiration_delay_in_seconds substitute. */
    [[nodiscard]] std::int64_t forex_ttl_seconds() const
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

    /**
     * @brief Deterministic get_forex_rates/get_forex_exchange_rates substitute.
     * Models the upstream retry helper (core/currency.rs:56-101): three attempts,
     * retryable lock/cache failures, attempt-based backoff as metric data (no sleep).
     */
    [[nodiscard]] std::expected<std::string, core::error::AppError> load_rates() const
    {
        constexpr std::string_view kKey{"currency.forex_rates"};
        constexpr int kMaxAttempts{3}; // DEFAULT_ANALYTICS_FOREX_RETRY_ATTEMPTS (consts.rs:314)
        const auto ttl = forex_ttl_seconds();

        const auto cached = m_context->cache().get(kKey);
        if (cached.hit) {
            const auto text = core::cache::value_as<std::string>(cached.value);
            if (!text.empty()) {
                return text;
            }
        }

        for (int attempt = 1; attempt <= kMaxAttempts; ++attempt) {
            const std::string lock_key = "forex.rates.refresh";
            const auto lock = m_context->locks().acquire(lock_key, ttl, "currency");
            if (!lock) {
                // ForexError::CouldNotAcquireLock / WriteLockNotAcquired: retryable.
                record_backoff(attempt);
                continue;
            }
            std::string encoded;
            try {
                // No live provider/credential exists locally; the deterministic
                // ExchangeRates table is the documented substitute for the API fetch.
                encoded = serde::Ser::encode_json(seed_forex_rates_view());
                m_context->cache().put(
                    kKey,
                    core::cache::to_cache_value(encoded),
                    core::cache::Ttl{}.seconds(ttl)
                );
            } catch (...) {
                // ForexError::LocalWriteError / RedisWriteError: retryable.
                m_context->locks().release(lock_key);
                record_backoff(attempt);
                continue;
            }
            m_context->locks().release(lock_key);
            return encoded;
        }
        return std::unexpected{internal_server_error("Unable to fetch forex rates")};
    }

    /** @brief Backoff is recorded as data; upstream sleeps attempt*2 seconds. */
    void record_backoff(int attempt) const
    {
        m_context->metrics().record_flow(std::format("ForexFetchBackoff:{}s", attempt * 2));
        m_context->metrics().counter("FOREX_FETCH_RETRY").increment();
    }

    /** @brief Deterministic ExchangeRates table used to seed the cache entry. */
    [[nodiscard]] ForexRatesView seed_forex_rates_view() const
    {
        ForexRatesView view{};
        view.data.base_currency = "USD";
        view.data.conversion = {
            {"USD", CurrencyFactorsView{1.0, 1.0}},
            {"EUR", CurrencyFactorsView{0.92, 1.08695652173913}},
            {"GBP", CurrencyFactorsView{0.79, 1.26582278481013}},
            {"INR", CurrencyFactorsView{83.0, 0.0120481927710843}},
            {"JPY", CurrencyFactorsView{149.0, 0.00671140939597315}},
            {"CAD", CurrencyFactorsView{1.35, 0.740740740740741}},
            {"AUD", CurrencyFactorsView{1.52, 0.657894736842105}},
            {"CHF", CurrencyFactorsView{0.88, 1.13636363636364}},
        };
        view.timestamp = now_seconds();
        return view;
    }

    [[nodiscard]] std::int64_t now_seconds() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch()
               ).count();
    }

    /** @brief Parse a MinorUnit value as the upstream i64 minor-unit integer. */
    [[nodiscard]] std::optional<std::int64_t> parse_minor_i64(std::string_view text) const
    {
        std::int64_t value{};
        const auto parsed = std::from_chars(
            text.data(),
            text.data() + text.size(),
            value
        );
        if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() || value < 0) {
            return std::nullopt;
        }
        return value;
    }

    /**
     * @brief Canonical ISO 4217 minor-unit exponents used by rusty_money::iso in
     * currency_conversion::currency_match (crate types.rs:78-241). Rate factors
     * remain limited to the seeded provider substitute set.
     */
    [[nodiscard]] int minor_unit_exponent(std::string_view code) const
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

    /** @brief (to_factor, from_factor, minor-unit exponent), base USD. */
    [[nodiscard]] std::optional<std::tuple<double, double, int>> factors_for(
        std::string_view code
    ) const
    {
        const int exponent = minor_unit_exponent(code);
        if (code == "USD") return std::tuple{1.0, 1.0, exponent};
        if (code == "EUR") return std::tuple{0.92, 1.08695652173913, exponent};
        if (code == "GBP") return std::tuple{0.79, 1.26582278481013, exponent};
        if (code == "INR") return std::tuple{83.0, 0.0120481927710843, exponent};
        if (code == "JPY") return std::tuple{149.0, 0.00671140939597315, exponent};
        if (code == "CAD") return std::tuple{1.35, 0.740740740740741, exponent};
        if (code == "AUD") return std::tuple{1.52, 0.657894736842105, exponent};
        if (code == "CHF") return std::tuple{0.88, 1.13636363636364, exponent};
        return std::nullopt;
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
