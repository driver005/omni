export module router.router;

import std;
import http.types;

export namespace router {

enum class HttpMethod {
    GET,
    POST,
    PUT,
    PATCH,
    DELETE,
    OPTIONS,
    HEAD,
    ANY
};

using RouteHandler = std::function<void(const http::Request&, http::Response&)>;

using Middleware = std::function<void(const http::Request&, http::Response&, std::function<void()>)>;

using RouteParams = std::map<std::string, std::string>;

struct RouteMatch {
    bool m_matched{false};
    RouteHandler m_handler{};
    std::vector<Middleware> m_middlewares{};
    RouteParams m_params{};
    std::map<std::string, std::string> m_route_config{};
    
    RouteMatch() noexcept = default;
    
    RouteMatch(
        bool matched,
        RouteHandler&& handler,
        std::vector<Middleware>&& middlewares,
        RouteParams&& params,
        std::map<std::string, std::string>&& route_config
    ) noexcept
        : m_matched(matched)
        , m_handler(std::move(handler))
        , m_middlewares(std::move(middlewares))
        , m_params(std::move(params))
        , m_route_config(std::move(route_config))
    {}
    
    bool matched() const noexcept { return m_matched; }
    RouteHandler& handler() noexcept { return m_handler; }
    const RouteHandler& handler() const noexcept { return m_handler; }
    std::vector<Middleware>& middlewares() noexcept { return m_middlewares; }
    const std::vector<Middleware>& middlewares() const noexcept { return m_middlewares; }
    RouteParams& params() noexcept { return m_params; }
    const RouteParams& params() const noexcept { return m_params; }
    std::map<std::string, std::string>& route_config() noexcept { return m_route_config; }
    const std::map<std::string, std::string>& route_config() const noexcept { return m_route_config; }
};

struct RouteConfig {
    std::string m_pattern{};
    HttpMethod m_method{HttpMethod::ANY};
    RouteHandler m_handler{};
    std::vector<Middleware> m_middlewares{};
    bool m_authenticate{true};
    bool m_cors{true};
    std::string m_route_type{};
    
    RouteConfig() noexcept = default;
    
    RouteConfig(
        std::string&& pattern,
        HttpMethod method,
        RouteHandler&& handler,
        std::vector<Middleware>&& middlewares,
        bool authenticate,
        bool cors,
        std::string&& route_type
    ) noexcept
        : m_pattern(std::move(pattern))
        , m_method(method)
        , m_handler(std::move(handler))
        , m_middlewares(std::move(middlewares))
        , m_authenticate(authenticate)
        , m_cors(cors)
        , m_route_type(std::move(route_type))
    {}
    
    std::string& pattern() noexcept { return m_pattern; }
    const std::string& pattern() const noexcept { return m_pattern; }
    HttpMethod method() const noexcept { return m_method; }
    void set_method(HttpMethod method) noexcept { m_method = method; }
    RouteHandler& handler() noexcept { return m_handler; }
    const RouteHandler& handler() const noexcept { return m_handler; }
    std::vector<Middleware>& middlewares() noexcept { return m_middlewares; }
    const std::vector<Middleware>& middlewares() const noexcept { return m_middlewares; }
    bool authenticate() const noexcept { return m_authenticate; }
    void set_authenticate(bool value) noexcept { m_authenticate = value; }
    bool cors() const noexcept { return m_cors; }
    void set_cors(bool value) noexcept { m_cors = value; }
    std::string& route_type() noexcept { return m_route_type; }
    const std::string& route_type() const noexcept { return m_route_type; }
};

struct RouterConfig {
    std::string m_base_path{};
    std::vector<std::string> m_admin_cors_origins{};
    std::vector<std::string> m_store_cors_origins{};
    std::vector<std::string> m_auth_cors_origins{};
    std::vector<std::string> m_rbac_cors_origins{};
    std::vector<std::string> m_restricted_fields{};
    size_t m_store_relations_limit{3};
    
    RouterConfig() noexcept = default;
    
    std::string& base_path() noexcept { return m_base_path; }
    const std::string& base_path() const noexcept { return m_base_path; }
    std::vector<std::string>& admin_cors_origins() noexcept { return m_admin_cors_origins; }
    const std::vector<std::string>& admin_cors_origins() const noexcept { return m_admin_cors_origins; }
    std::vector<std::string>& store_cors_origins() noexcept { return m_store_cors_origins; }
    const std::vector<std::string>& store_cors_origins() const noexcept { return m_store_cors_origins; }
    std::vector<std::string>& auth_cors_origins() noexcept { return m_auth_cors_origins; }
    const std::vector<std::string>& auth_cors_origins() const noexcept { return m_auth_cors_origins; }
    std::vector<std::string>& rbac_cors_origins() noexcept { return m_rbac_cors_origins; }
    const std::vector<std::string>& rbac_cors_origins() const noexcept { return m_rbac_cors_origins; }
    std::vector<std::string>& restricted_fields() noexcept { return m_restricted_fields; }
    const std::vector<std::string>& restricted_fields() const noexcept { return m_restricted_fields; }
    size_t store_relations_limit() const noexcept { return m_store_relations_limit; }
    void set_store_relations_limit(size_t limit) noexcept { m_store_relations_limit = limit; }
};

class Router {
public:
    static std::unique_ptr<Router> create(const RouterConfig& config = {});
    
    ~Router();
    
    Router(const Router&) = delete;
    Router& operator=(const Router&) = delete;
    
    Router(Router&&) noexcept;
    Router& operator=(Router&&) noexcept;
    
    void add_route(const RouteConfig& route);
    void add_route(const std::string& pattern, HttpMethod method, RouteHandler handler);
    void add_route(const std::string& pattern, HttpMethod method, RouteHandler handler, std::vector<Middleware> middlewares);
    
    void use(const std::string& pattern, Middleware middleware);
    void use(Middleware middleware);
    
    std::unique_ptr<Router> group(const std::string& prefix, const RouterConfig& config = {});
    
    void load_routes_from_directory(const std::string& path);
    void load_routes_from_directories(const std::vector<std::string>& paths);
    
    void handle(const http::Request& req, http::Response& res);
    
    RouteMatch match(const std::string& path, HttpMethod method) const;
    
    std::vector<RouteConfig> routes() const;
    
    void clear();
    
    const RouterConfig& config() const noexcept;
    void set_config(const RouterConfig& config);
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    
    explicit Router(const RouterConfig& config);
};

class RouteBuilder {
public:
    RouteBuilder(Router& router, const std::string& pattern);
    
    RouteBuilder& get(RouteHandler handler);
    RouteBuilder& post(RouteHandler handler);
    RouteBuilder& put(RouteHandler handler);
    RouteBuilder& patch(RouteHandler handler);
    RouteBuilder& delete_(RouteHandler handler);
    RouteBuilder& options(RouteHandler handler);
    RouteBuilder& head(RouteHandler handler);
    RouteBuilder& any(RouteHandler handler);
    
    RouteBuilder& middleware(Middleware middleware);
    RouteBuilder& middlewares(std::vector<Middleware> middlewares);
    
    RouteBuilder& authenticate(bool value = true);
    RouteBuilder& cors(bool value = true);
    RouteBuilder& route_type(const std::string& type);
    
    void register_route();
    
private:
    Router& m_router;
    std::string m_pattern;
    HttpMethod m_method{HttpMethod::ANY};
    RouteHandler m_handler{};
    std::vector<Middleware> m_middlewares{};
    bool m_authenticate{true};
    bool m_cors{true};
    std::string m_route_type{};
    bool m_registered{false};
};

namespace middleware {
Middleware authenticate(const std::string& actor_type, const std::vector<std::string>& auth_types);
Middleware cors(const std::vector<std::string>& origins);
Middleware body_parser(size_t max_size = 1024 * 1024);
Middleware validate_query(const std::map<std::string, std::string>& schema);
Middleware logger();
Middleware error_handler();
Middleware rate_limit(size_t max_requests, std::chrono::seconds window);
Middleware request_id();
Middleware compression();
}

class RouteLoader {
public:
    RouteLoader();
    ~RouteLoader();
    
    void scan_directory(const std::string& path);
    void scan_directories(const std::vector<std::string>& paths);
    
    std::vector<RouteConfig> get_routes() const;
    
    std::vector<RouteConfig> reload_file(const std::string& file_path);
    
    void clear();
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

class RouteFinder {
public:
    RouteFinder();
    explicit RouteFinder(const std::vector<RouteConfig>& routes);
    
    void add_route(const RouteConfig& route);
    void add_routes(const std::vector<RouteConfig>& routes);
    
    RouteMatch find(const std::string& path, HttpMethod method) const;
    
    void clear();
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

class RouteSorter {
public:
    explicit RouteSorter(const std::vector<RouteConfig>& routes);
    
    std::vector<RouteConfig> sort(const std::vector<std::string>& priority = {"static", "params", "regex", "wildcard", "global"}) const;
    
private:
    std::vector<RouteConfig> m_routes;
    
    int route_priority(const RouteConfig& route) const;
    bool is_static_route(const RouteConfig& route) const;
    bool is_param_route(const RouteConfig& route) const;
    bool is_regex_route(const RouteConfig& route) const;
    bool is_wildcard_route(const RouteConfig& route) const;
};

} // namespace router