export module blog_handlers:webhooks_handler;

import std;
import interfaces;
import blog_context;
import blog_dto;
import :handler;
import :special;
import :stripe;

export namespace blog::api {

/**
 * @brief Domain handler for /webhooks. Owns read/response-shaping for this
 * resource; write and special endpoints fall through to CoreHandler.
 */
class WebhooksRouteHandler
{
public:
    WebhooksRouteHandler(blog::BlogContext& context, CoreHandler& core, SpecialEndpoints& special, StripeHandler& stripe) noexcept :
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
        if (method != "GET") {
            m_core.handle(request, response, std::move(send));
            return;
        }
        const auto raw = std::string{request.get_path()};
        const auto q = raw.find('?');
        std::string path = q == std::string::npos ? raw : raw.substr(0, q);
        if (path.size() > 1 && path.back() == '/') path.pop_back();
        if (path != "/webhooks" && !path.starts_with("/webhooks/")) {
            m_core.handle(request, response, std::move(send));
            return;
        }
        if (std::ranges::count(path, '/') > 2) {
            // sub-resources (subscriptions, signin_urls, gift_links, ...) belong to core
            m_core.handle(request, response, std::move(send));
            return;
        }
        // Special sub-resources keep their CoreHandler behaviour.
        static constexpr std::string_view kSpecial[] = {"/webhooks/export", "/webhooks/upload", "/webhooks/1/gift_links"};
        for (const auto s : kSpecial) {
            if (path.starts_with(s)) {
                m_core.handle(request, response, std::move(send));
                return;
            }
        }

        std::map<std::string, std::string> params;
        if (q != std::string::npos) {
            for (auto pair : std::string_view{raw}.substr(q + 1) | std::views::split('&')) {
                const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
                const auto eq = text.find('=');
                if (eq != std::string::npos) params[text.substr(0, eq)] = text.substr(eq + 1);
            }
        }

        std::string id;
        if (path != "/webhooks") {
            const auto rest = std::string_view{path}.substr(std::string{"/webhooks/"}.size());
            const auto slash = rest.find('/');
            id = std::string{rest.substr(0, slash)};
            if (id == "slug" || id == "email") {
                id.clear(); // slug/email lookups degrade to browse as in CoreHandler
            } else if (!std::ranges::all_of(id, [](char c) {
                   return std::isdigit(static_cast<unsigned char>(c));
               })) {
                id.clear();
            }
        }

        std::string out;
        if (!id.empty()) {
            const auto one = m_ctx.store().find_by_resource("webhooks", id);
            if (one) {
                out = std::format(R"({{"webhooks":[{}]}})", *one);
            } else {
                out = std::format(R"({{"webhooks":[{}]}})", blog::dto::GhostJson::webhook());
            }
        } else {
            const auto body = m_ctx.store().json_for("webhooks", params);
            std::size_t page = 1;
            std::size_t limit = 15;
            if (const auto it = params.find("page"); it != params.end()) {
                try { page = std::max<std::size_t>(1, std::stoul(it->second)); } catch (...) {}
            }
            if (const auto it = params.find("limit"); it != params.end()) {
                try { limit = std::max<std::size_t>(1, std::stoul(it->second)); } catch (...) {}
            }
            blog::dto::PaginationMetaDto meta;
            const auto total = m_ctx.store().total("webhooks");
            meta.limit = static_cast<int>(limit);
            meta.page = static_cast<int>(page);
            meta.total = static_cast<int>(total);
            meta.pages = total == 0 ? 1 : static_cast<int>((total + limit - 1) / limit);
            if (page < static_cast<std::size_t>(meta.pages)) meta.next = static_cast<int>(page + 1);
            if (page > 1) meta.prev = static_cast<int>(page - 1);
            const auto pagination = blog::dto::ModelJson<blog::dto::PaginationMetaDto>{}.to_json(meta);
            out = std::format(R"({{"webhooks":{},"meta":{{"pagination":{}}}}})", body, pagination);
        }
        write(response, 200, out);
        send();
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
