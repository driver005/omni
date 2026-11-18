export module blog_handlers:stats_handler;

import std;
import interfaces;
import blog_context;
import :handler;
import :special;
import :stripe;

export namespace blog::api {

/** @brief Domain handler for /stats. Owns this endpoint's Ghost response. */
class StatsRouteHandler
{
public:
    StatsRouteHandler(blog::BlogContext& context, CoreHandler& core, SpecialEndpoints& special, StripeHandler& stripe) noexcept :
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
        if (method == "GET" && (path.starts_with("/stats/") || path.ends_with("/performance-stats"))) {
            const auto result =
                path == "/stats/mrr" || path == "/members/stats/mrr"
                    ? m_special.stats_mrr()
                    : path == "/stats/member_count" || path == "/members/stats/count"
                          ? m_special.stats_member()
                          : m_special.stats_empty();
            write(response, result.status(), result.body());
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
