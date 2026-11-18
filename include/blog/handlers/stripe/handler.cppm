export module blog_handlers:stripe_handler;

import std;
import interfaces;
import blog_context;
import :handler;
import :special;
import :stripe;

export namespace blog::api {

/** @brief Domain handler for /stripe/checkout. Owns the Stripe config state. */
class StripeRouteHandler
{
public:
    StripeRouteHandler(blog::BlogContext& context, CoreHandler& core,
                       SpecialEndpoints& special, StripeHandler& stripe) noexcept :
        m_ctx{context}, m_core{core}, m_special{special}, m_stripe{stripe}
    {
    }

    void handle(
        interfaces::io::IRequest& request,
        interfaces::io::IResponse& response,
        std::function<void()> send
    )
    {
        const std::string method{request.get_method()};
        const auto raw = std::string{request.get_path()};
        const auto q = raw.find('?');
        std::string path = q == std::string::npos ? raw : raw.substr(0, q);
        std::string body;
        for (const std::byte byte : request.get_body()) body += static_cast<char>(byte);

        if (path.starts_with("/stripe/")) {
            std::string out;
            if (path == "/stripe/checkout/config" && method == "GET") {
                out = m_stripe.config();
            } else if (path == "/stripe/checkout/config" && method == "PUT") {
                m_stripe.configure(body);
                out = m_stripe.config();
            } else if (method == "POST") {
                out = m_stripe.preview(body);
            } else {
                out = m_stripe.branding();
            }
            write(response, 200, out);
            send();
            return;
        }
        m_core.handle(request, response, std::move(send));
    }

private:
    void write(interfaces::io::IResponse& response, int status, std::string_view body) const
    {
        response.set_status(static_cast<interfaces::io::types::Status>(status));
        std::vector<std::byte> bytes;
        for (const char c : body) bytes.push_back(static_cast<std::byte>(c));
        response.set_body(std::move(bytes));
    }

    blog::BlogContext& m_ctx;
    CoreHandler& m_core;
    SpecialEndpoints& m_special;
    StripeHandler& m_stripe;
};

} // namespace blog::api
