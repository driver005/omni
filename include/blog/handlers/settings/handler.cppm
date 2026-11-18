export module blog_handlers:settings_handler;

import std;
import interfaces;
import blog_context;
import :handler;
import :special;
import :stripe;

export namespace blog::api {

/** @brief Domain handler for /settings. Owns the key/value settings envelope. */
class SettingsRouteHandler
{
public:
    SettingsRouteHandler(blog::BlogContext& context, CoreHandler& core, SpecialEndpoints& special, StripeHandler& stripe) noexcept :
        m_ctx{context},
        m_core{core},
        m_special{special},
        m_stripe{stripe}
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
        if (path.size() > 1 && path.back() == '/') path.pop_back();
        if (method == "GET" && path == "/settings") {
            const auto body = m_ctx.store().json_for("settings");
            write(response, 200, std::format(R"({{"settings":{},"meta":{{}}}})", body));
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
