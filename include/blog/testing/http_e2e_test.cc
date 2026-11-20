import std;
import blog_testing;
import blog_handlers_routes;
import blog_handlers;
import core_router;
import io_layer_http2;
import interfaces;

/**
 * End-to-end harness: drives Ghost API requests through the real HTTP2 Server
 * route table + RouterExecutor, using concrete HttpRequest/HttpResponse objects.
 * No sockets needed — this exercises the same dispatch path the wire uses.
 */
int main()
{
    blog::api::BlogHandlers handler;

    // The API itself installs all 261 Ghost routes into core_router.
    core::router::RouterContext<> context;
    blog::api::ApiRoutes{}.install(context, handler);

    io::layer::http2::Server server;
    server.build(&context);

    const auto run = [&server](
        std::string_view method,
        std::string_view path,
        std::string_view body = {},
        std::string_view token = {}
    ) {
        io::layer::http2::HttpRequest request{1};
        io::layer::http2::HttpResponse response{1};
        request.set_header(interfaces::io::types::Token::METHOD, method);
        request.set_header(interfaces::io::types::Token::PATH, path);
        request.set_header(interfaces::io::types::Token::CONTENT_TYPE, "application/json");
        request.set_header(interfaces::io::types::Token::ACCEPT, "application/json");
        if (!token.empty()) {
            request.set_header(interfaces::io::types::Token::AUTHORIZATION, token);
        }
        if (!body.empty()) {
            std::vector<std::byte> bytes;
            bytes.reserve(body.size());
            for (const char chr : body) {
                bytes.push_back(static_cast<std::byte>(chr));
            }
            request.set_body(std::move(bytes));
        }

        bool sent = false;
        server.executor().enqueue(request, response, [&sent] { sent = true; });
        server.executor().on_execute()();

        std::string output;
        for (const std::byte byte : response.get_body()) {
            output += static_cast<char>(byte);
        }
        int status = -1;
        for (const auto& entry : response.get_headers()) {
            std::visit([&status](const auto& field) {
                if (field == nullptr) {
                    return;
                }
                const auto& name = field->get_name();
                if constexpr (std::is_same_v<
                    std::decay_t<decltype(name)>, interfaces::io::types::Token
                >) {
                    if (name == interfaces::io::types::Token::STATUS) {
                        status = std::stoi(field->get_value());
                    }
                }
            }, entry);
        }
        return std::pair<int, std::string>{sent ? status : -1, std::move(output)};
    };

    const auto posts = run("GET", "/posts", "{}", "admin");
    if (posts.first != 200 || !posts.second.contains("Hello Ghost")) {
        std::println("posts {} {}", posts.first, posts.second.substr(0,160));
        return 1;
    }
    if (!posts.second.contains("\"meta\":{\"pagination\":")) return 2;

    const auto filtered = run("GET", "/posts?fields=title", "{}", "admin");
    if (filtered.first != 200 || !filtered.second.contains("\"title\"") ||
        filtered.second.contains("\"html\"")) return 3;

    const auto created = run(
        "POST", "/posts",
        R"({"title":"E2E Post","slug":"e2e-post","status":"draft"})",
        "admin"
    );
    if (created.first != 201 && created.first != 200) return 4;
    if (!created.second.contains("E2E Post")) return 5;

    const auto browse = run("GET", "/posts", "{}", "admin");
    if (!browse.second.contains("E2E Post")) return 6;

    const auto unauthorized = run("GET", "/members");
    if (unauthorized.first != 401 || !unauthorized.second.contains("Unauthorized")) return 9;

    std::size_t missing{0};
    for (const auto& route: blog::testing::RouteCatalog::values) {
        std::string concrete{route.path()};
        for (;;) {
            const auto colon = concrete.find(':');
            if (colon == std::string::npos) break;
            const auto slash = concrete.find('/', colon);
            concrete.replace(colon, slash == std::string::npos ? concrete.size() - colon : slash - colon, "1");
        }
        const std::string method = std::string{route.method() == "ALL" ? "GET" : route.method()};
        const auto r = run(method, concrete, "{}", "admin");
        if (r.first == 404 || r.first == -1) {
            const bool stateful = method == "PUT" || method == "DELETE" ||
                                  concrete.find("/1") != std::string::npos;
            if (!stateful) {
                ++missing;
                std::println("MISS {} {}", method, concrete);
            }
        }
    }
    std::println("unrouted {}/261", missing);
    if (missing != 0) return 10;

    return 0;
}
