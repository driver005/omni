export module blog_testing:test_api;

import std;
import interfaces;
import core_router;
import io_layer_http2;
import utils_buffering;
import blog_handlers_routes;
import blog_handlers;
import blog_context;
import :route_catalog;
import :request;
import :response;

export namespace blog::api {

/** @brief Test-only facade: drives CoreHandler through concrete HTTP2 messages. */
class TestApi
{
public:
    TestApi() = default;

    [[nodiscard]] const auto& routes() const noexcept
    {
        return blog::testing::RouteCatalog::values;
    }

    [[nodiscard]] std::size_t route_count() const noexcept
    {
        return blog::testing::RouteCatalog::values.size();
    }

    [[nodiscard]] std::size_t content_route_count() const noexcept
    {
        std::size_t n{0};
        for (const auto& r: blog::testing::RouteCatalog::values) {
            if (!r.admin()) ++n;
        }
        return n;
    }

    [[nodiscard]] ApiResponse dispatch(const ApiRequest& request)
    {
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_header(interfaces::io::types::Token::METHOD, request.method());
        req.set_header(interfaces::io::types::Token::PATH, request.path());
        req.set_header(interfaces::io::types::Token::CONTENT_TYPE, "application/json");
        req.set_header(interfaces::io::types::Token::ACCEPT, "application/json");
        if (!request.token().empty()) {
            req.set_header(interfaces::io::types::Token::AUTHORIZATION, request.token());
        }
        std::vector<std::byte> bytes;
        for (const char chr : request.body()) bytes.push_back(static_cast<std::byte>(chr));
        if (!bytes.empty()) {
            auto* node = new utils::buffering::BufferNode{std::move(bytes)};
            req.get_body().push_back(node, 0, request.body().size());
        }
        if (!m_installed) {
            blog::api::ApiRoutes{}.install(m_context_router, m_handlers);
            m_server = std::make_unique<io::layer::http2::Server>();
            m_server->build(&m_context_router);
            m_installed = true;
        }
        m_server->executor().enqueue(req, res, [] {});
        m_server->executor().on_execute()();
        std::string body;
        for (const std::byte byte : res.get_body()) body += static_cast<char>(byte);
        int status = 200;
        for (const auto& entry : res.get_headers()) {
            std::visit([&status](const auto& field) {
                if (field == nullptr) return;
                using NameType = std::decay_t<decltype(field->get_name())>;
                std::string_view name;
                if constexpr (std::is_same_v<NameType, interfaces::io::types::Token>) {
                    name = interfaces::io::types::token_to_string(field->get_name());
                } else {
                    name = field->get_name();
                }
                if (name == ":status") {
                    status = std::stoi(std::string{field->get_value()});
                }
            }, entry);
        }
        return ApiResponse{status, body};
    }

private:
    blog::BlogContext m_context{};
    core::router::RouterContext<> m_context_router{};
    blog::api::BlogHandlers m_handlers{};
    std::unique_ptr<io::layer::http2::Server> m_server{};
    bool m_installed{false};
};

} // namespace blog::api
