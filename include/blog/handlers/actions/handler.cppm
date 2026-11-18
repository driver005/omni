export module blog_handlers:actions_handler;

import std;
import interfaces;
import blog_context;
import blog_dto;
import :handler;
import :special;
import :stripe;

export namespace blog::api {

/** @brief Domain handler for /actions. Owns this resource's browse envelope. */
class ActionsRouteHandler
{
public:
    ActionsRouteHandler(blog::BlogContext& context, CoreHandler& core, SpecialEndpoints& special, StripeHandler& stripe) noexcept :
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
        if (request.get_authorization().empty()) {
            write(response, 401, R"({"error":"Unauthorized"})");
            send();
            return;
        }
        if (method == "GET") {
            const auto raw = std::string{request.get_path()};
            const auto q = raw.find('?');
            std::string path = q == std::string::npos ? raw : raw.substr(0, q);
            if (path.size() > 1 && path.back() == '/') path.pop_back();
            if (std::ranges::count(path, '/') > 2) {
                m_core.handle(request, response, std::move(send));
                return;
            }
            if (path == "/actions" || path.starts_with("/actions/")) {
                std::map<std::string, std::string> params;
                if (q != std::string::npos) {
                    for (auto pair : std::string_view{raw}.substr(q + 1) | std::views::split('&')) {
                        const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
                        const auto eq = text.find('=');
                        if (eq != std::string::npos) params[text.substr(0, eq)] = text.substr(eq + 1);
                    }
                }
                const auto body = m_ctx.store().json_for("actions", params);
                std::size_t page = 1;
                std::size_t limit = 15;
                if (const auto it = params.find("page"); it != params.end()) {
                    try { page = std::max<std::size_t>(1, std::stoul(it->second)); } catch (...) {}
                }
                if (const auto it = params.find("limit"); it != params.end()) {
                    try { limit = std::max<std::size_t>(1, std::stoul(it->second)); } catch (...) {}
                }
                blog::dto::PaginationMetaDto meta;
                const auto total = m_ctx.store().total("actions");
                meta.limit = static_cast<int>(limit);
                meta.page = static_cast<int>(page);
                meta.total = static_cast<int>(total);
                meta.pages = total == 0 ? 1 : static_cast<int>((total + limit - 1) / limit);
                if (page < static_cast<std::size_t>(meta.pages)) meta.next = static_cast<int>(page + 1);
                if (page > 1) meta.prev = static_cast<int>(page - 1);
                const auto pagination = blog::dto::ModelJson<blog::dto::PaginationMetaDto>{}.to_json(meta);
                write(response, 200, std::format(
                    R"({{"actions":{},"meta":{{"pagination":{}}}}})", body, pagination
                ));
                send();
                return;
            }
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
