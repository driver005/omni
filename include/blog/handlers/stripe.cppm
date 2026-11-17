export module blog_handlers:stripe;

import std;
import blog_context;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::api {

/** @brief Real state for the four Stripe Admin API endpoints. */
class StripeHandler
{
public:
    [[nodiscard]] std::string config() const
    {
        return std::format(
            R"({{"config":{{"session_mode":"{}","currency":"{}","payment_methods":[{}]}}}})",
            m_session_mode,
            m_currency,
            m_payment_methods.empty() ? std::string{} : JsonBody{}.quote(m_payment_methods)
        );
    }

    void configure(std::string_view body)
    {
        for (const auto& [key, value]: JsonBody{}.parse(body)) {
            if (key == "session_mode") {
                m_session_mode = value;
            } else if (key == "currency") {
                m_currency = value;
            } else if (key == "payment_methods") {
                m_payment_methods = value;
            }
        }
    }

    [[nodiscard]] std::string preview(std::string_view body) const
    {
        const auto values = JsonBody{}.parse(body);
        const std::string amount = values.contains("amount") ? values.at("amount") : "0";
        return std::format(
            R"({{"preview":{{"amount":{},"currency":"{}","mode":"{}"}}}})",
            amount,
            m_currency,
            m_session_mode
        );
    }

    [[nodiscard]] std::string branding() const
    {
        return std::format(
            R"({{"branding":{{"name":"{}","currency":"{}"}}}})",
            m_brand_name,
            m_currency
        );
    }

private:
    std::string m_session_mode{"payment"};
    std::string m_currency{"usd"};
    std::string m_payment_methods{"card"};
    std::string m_brand_name{"Ghost"};
};

} // namespace blog::api

#ifdef CONGELADO_TEST
namespace blog::api::tests {
using namespace boost::ut;
suite<"blog_api_stripe"> blog_api_stripe_suite = []
{
    "config changes are observable in preview"_test = []
    {
        StripeHandler stripe;
        expect(stripe.config().contains("\"session_mode\":\"payment\""));
        stripe.configure(R"({"session_mode":"subscription","currency":"eur"})");
        expect(stripe.config().contains("\"session_mode\":\"subscription\""));
        expect(stripe.preview(R"({"amount":500})").contains("\"currency\":\"eur\""));
        expect(stripe.branding().contains("\"currency\":\"eur\""));
    };
};
} // namespace blog::api::tests
#endif
