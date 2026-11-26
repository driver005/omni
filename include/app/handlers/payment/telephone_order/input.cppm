export module app_payment:telephone_order_input;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_http;
import app_auth;
import app_metrics;

export namespace app::handlers::telephone_order {

using Parameters = std::unordered_map<std::string, std::string>;
using ResponseParameters = std::unordered_map<std::string, std::string>;

/** @brief api_models/telephone_order/twilio.rs:16-69 — the provider request body. */
class TwilioGenericPayRequest
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

    void set_transaction_id(std::string value)
    {
        transaction_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_transaction_id() const noexcept
    {
        return transaction_id;
    }

    void set_cardnumber(std::optional<std::string> value)
    {
        cardnumber = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_cardnumber() const noexcept
    {
        return cardnumber;
    }

    void set_cvv(std::optional<std::string> value)
    {
        cvv = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_cvv() const noexcept
    {
        return cvv;
    }

    void set_expiry_month(std::optional<std::string> value)
    {
        expiry_month = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_expiry_month() const noexcept
    {
        return expiry_month;
    }

    void set_expiry_year(std::optional<std::string> value)
    {
        expiry_year = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_expiry_year() const noexcept
    {
        return expiry_year;
    }

    void set_postal_code(std::optional<std::string> value)
    {
        postal_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_postal_code() const noexcept
    {
        return postal_code;
    }

    void set_bankaccountnumber(std::optional<std::string> value)
    {
        bankaccountnumber = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bankaccountnumber() const noexcept
    {
        return bankaccountnumber;
    }

    void set_routingnumber(std::optional<std::string> value)
    {
        routingnumber = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_routingnumber() const noexcept
    {
        return routingnumber;
    }

    void set_amount(std::optional<std::string> value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_amount() const noexcept
    {
        return amount;
    }

    void set_currency_code(std::optional<std::string> value)
    {
        currency_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_currency_code() const noexcept
    {
        return currency_code;
    }

    void set_description(std::optional<std::string> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept
    {
        return description;
    }

    void set_parameters(std::optional<Parameters> value)
    {
        parameters = std::move(value);
    }

    [[nodiscard]] const std::optional<Parameters>& get_parameters() const noexcept
    {
        return parameters;
    }

    std::string method{};
    std::string transaction_id{};
    std::optional<std::string> cardnumber{};
    std::optional<std::string> cvv{};
    std::optional<std::string> expiry_month{};
    std::optional<std::string> expiry_year{};
    std::optional<std::string> postal_code{};
    std::optional<std::string> bankaccountnumber{};
    std::optional<std::string> routingnumber{};
    std::optional<std::string> amount{};
    std::optional<std::string> currency_code{};
    std::optional<std::string> description{};
    std::optional<Parameters> parameters{};
};

/** @brief Everything the old charge<Adapter> call received. */
struct TelephoneOrderChargeInput
{
    TwilioGenericPayRequest request{};
    std::string merchant_id{};
    std::string transaction_reference{};
    ::app::auth::AuthContext auth_context{};
};

struct ChargePath
{
    std::string merchant_id{};
    std::string provider{};
};

/**
 * @brief Parse/path/auth support for the telephone_order handlers.
 *
 * No business logic: it turns request data into the plain Input struct and the
 * AuthContext consumed by the domain runtime.
 */
class TelephoneOrderInputSupport
{
public:
    explicit TelephoneOrderInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    /** @brief routes/telephone_order.rs:19-36 — :merchant_id / :provider segments. */
    [[nodiscard]] ChargePath charge_path(interfaces::io::IRequest& req) const
    {
        const ::app::http::PathSegments segments{::app::http::RequestPath{}(req)};
        const std::size_t count{segments.count()};
        const auto merchant_segment = count >= 2 ? segments.at(count - 2) : std::nullopt;
        const auto provider_segment = count >= 1 ? segments.at(count - 1) : std::nullopt;
        return ChargePath{
            merchant_segment ? std::string{*merchant_segment} : std::string{},
            provider_segment ? std::string{*provider_segment} : std::string{}
        };
    }

    /**
     * @brief services/authentication.rs — ApiKeyBasicAuthWithMerchantIdFromRoute.
     * The route decodes Basic user[:password] into the api-key claim, then the
     * runtime applies MERCHANT_ID / PAYMENT_WRITE with the decoded key as id source.
     */
    [[nodiscard]] ::app::auth::AuthContext
    charge_auth_context(interfaces::io::IRequest& req, std::string_view merchant_id) const
    {
        ::app::auth::AuthContext auth_ctx = ::app::auth::AuthContextBuilder{}(req);
        if (const auto key = basic_api_key(req)) {
            auth_ctx.add_claim(std::string{::app::auth::kClaimApiKey}, *key);
        }
        auth_ctx.set_merchant_id(std::string{merchant_id});
        return auth_ctx;
    }

private:
    /** @brief Basic / x-api-key decoding (route-level helper). */
    [[nodiscard]] std::string decode_base64(std::string_view text) const
    {
        static constexpr std::string_view table{
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
        };
        std::string out{};
        std::uint32_t accumulator{};
        int bits{};
        for (const char character: text) {
            if (character == '=' || std::isspace(static_cast<unsigned char>(character))) {
                continue;
            }
            const std::size_t value = table.find(character);
            if (value == std::string_view::npos) {
                return {};
            }
            accumulator = (accumulator << 6) | static_cast<std::uint32_t>(value);
            bits += 6;
            if (bits >= 8) {
                bits -= 8;
                out.push_back(static_cast<char>((accumulator >> bits) & 0xFFU));
            }
        }
        return out;
    }

    [[nodiscard]] std::optional<std::string> basic_api_key(interfaces::io::IRequest& req) const
    {
        const std::string_view authorization = req.find_header("authorization");
        constexpr std::string_view prefix{"Basic "};
        if (authorization.size() > prefix.size() &&
            std::equal(
                prefix.begin(),
                prefix.end(),
                authorization.begin(),
                [](char lhs, char rhs)
                {
                    return std::tolower(static_cast<unsigned char>(lhs)) ==
                           std::tolower(static_cast<unsigned char>(rhs));
                }
            )) {
            const std::string decoded = decode_base64(authorization.substr(prefix.size()));
            const std::size_t colon = decoded.find(':');
            const std::string key = colon == std::string::npos ? decoded : decoded.substr(0, colon);
            if (!key.empty()) {
                return key;
            }
        }
        const std::string_view header = req.find_header("x-api-key");
        if (!header.empty()) {
            return std::string{header};
        }
        return std::nullopt;
    }

    [[maybe_unused]] ::app::context::AppContext& m_context;
};

} // namespace app::handlers::telephone_order

// serde reflection for the provider request wire shape.
template<>
struct serde::Serializable<::app::handlers::telephone_order::TwilioGenericPayRequest>
{
    static constexpr auto fields()
    {
        using Wire = ::app::handlers::telephone_order::TwilioGenericPayRequest;
        return std::tuple{
            serde::
                FieldDesc<"method", &Wire::get_method, &Wire::set_method, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_id",
                &Wire::get_transaction_id,
                &Wire::set_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cardnumber",
                &Wire::get_cardnumber,
                &Wire::set_cardnumber,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"cvv", &Wire::get_cvv, &Wire::set_cvv, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expiry_month",
                &Wire::get_expiry_month,
                &Wire::set_expiry_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expiry_year",
                &Wire::get_expiry_year,
                &Wire::set_expiry_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "postal_code",
                &Wire::get_postal_code,
                &Wire::set_postal_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bankaccountnumber",
                &Wire::get_bankaccountnumber,
                &Wire::set_bankaccountnumber,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "routingnumber",
                &Wire::get_routingnumber,
                &Wire::set_routingnumber,
                serde::FieldOptions{}>{},
            serde::
                FieldDesc<"amount", &Wire::get_amount, &Wire::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency_code",
                &Wire::get_currency_code,
                &Wire::set_currency_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &Wire::get_description,
                &Wire::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "parameters",
                &Wire::get_parameters,
                &Wire::set_parameters,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "twilio_generic_pay_request";
    }
};
