export module http.types;

import std;

export namespace http {

class Headers {
    std::map<std::string, std::vector<std::string>, std::less<>> m_headers;
    
public:
    Headers() noexcept = default;
    Headers(const std::map<std::string, std::string>& headers) {
        for (const auto& [key, value] : headers) {
            m_headers[key].push_back(value);
        }
    }
    
    void set(const std::string& key, const std::string& value) noexcept {
        m_headers[key] = {value};
    }
    
    void add(const std::string& key, const std::string& value) noexcept {
        m_headers[key].push_back(value);
    }
    
    std::optional<std::string> get(const std::string& key) const noexcept {
        auto it = m_headers.find(key);
        if (it == m_headers.end() || it->second.empty()) return std::nullopt;
        return it->second.front();
    }
    
    std::vector<std::string> get_all(const std::string& key) const noexcept {
        auto it = m_headers.find(key);
        if (it == m_headers.end()) return {};
        return it->second;
    }
    
    bool has(const std::string& key) const noexcept {
        return m_headers.contains(key);
    }
    
    void remove(const std::string& key) noexcept {
        m_headers.erase(key);
    }
    
    void clear() noexcept {
        m_headers.clear();
    }
    
    std::map<std::string, std::string> to_map() const noexcept {
        std::map<std::string, std::string> result;
        for (const auto& [key, values] : m_headers) {
            if (!values.empty()) result[key] = values.front();
        }
        return result;
    }
    
    auto begin() const noexcept { return m_headers.begin(); }
    auto end() const noexcept { return m_headers.end(); }
    size_t size() const noexcept { return m_headers.size(); }
    bool empty() const noexcept { return m_headers.empty(); }
};

using QueryParams = std::map<std::string, std::vector<std::string>>;

using BodyValue = std::variant<
    std::monostate,
    int64_t,
    double,
    bool,
    std::string,
    std::vector<uint8_t>,
    std::map<std::string, BodyValue>,
    std::vector<BodyValue>
>;

class Request {
public:
    std::string m_method{"GET"};
    std::string m_path{"/"};
    std::string m_version{"HTTP/1.1"};
    std::string m_url{};
    Headers m_headers{};
    QueryParams m_query{};
    std::map<std::string, std::string> m_params{};
    BodyValue m_body{};
    std::string m_raw_body{};
    
    struct AuthContext {
        std::string m_actor_type{};
        std::string m_actor_id{};
        std::string m_auth_type{};
        std::map<std::string, BodyValue> m_claims{};
        
        AuthContext() noexcept = default;
        AuthContext(std::string&& actor_type, std::string&& actor_id, std::string&& auth_type) noexcept
            : m_actor_type(std::move(actor_type))
            , m_actor_id(std::move(actor_id))
            , m_auth_type(std::move(auth_type))
        {}
        
        std::string& actor_type() noexcept { return m_actor_type; }
        const std::string& actor_type() const noexcept { return m_actor_type; }
        std::string& actor_id() noexcept { return m_actor_id; }
        const std::string& actor_id() const noexcept { return m_actor_id; }
        std::string& auth_type() noexcept { return m_auth_type; }
        const std::string& auth_type() const noexcept { return m_auth_type; }
        std::map<std::string, BodyValue>& claims() noexcept { return m_claims; }
        const std::map<std::string, BodyValue>& claims() const noexcept { return m_claims; }
    };
    
    std::optional<AuthContext> m_auth_context{};
    std::map<std::string, BodyValue> m_additional_data{};
    
    class Scope;
    std::shared_ptr<Scope> m_scope{};
    
    struct QueryConfig {
        std::vector<std::string> m_fields{};
        std::map<std::string, std::string> m_filters{};
        std::optional<size_t> m_limit{};
        std::optional<size_t> m_offset{};
        std::map<std::string, std::string> m_order{};
        std::vector<std::string> m_relations{};
        size_t m_relations_limit{3};
        
        QueryConfig() noexcept = default;
        
        std::vector<std::string>& fields() noexcept { return m_fields; }
        const std::vector<std::string>& fields() const noexcept { return m_fields; }
        std::map<std::string, std::string>& filters() noexcept { return m_filters; }
        const std::map<std::string, std::string>& filters() const noexcept { return m_filters; }
        std::optional<size_t>& limit() noexcept { return m_limit; }
        const std::optional<size_t>& limit() const noexcept { return m_limit; }
        std::optional<size_t>& offset() noexcept { return m_offset; }
        const std::optional<size_t>& offset() const noexcept { return m_offset; }
        std::map<std::string, std::string>& order() noexcept { return m_order; }
        const std::map<std::string, std::string>& order() const noexcept { return m_order; }
        std::vector<std::string>& relations() noexcept { return m_relations; }
        const std::vector<std::string>& relations() const noexcept { return m_relations; }
        size_t relations_limit() const noexcept { return m_relations_limit; }
        void set_relations_limit(size_t limit) noexcept { m_relations_limit = limit; }
    };
    
    std::optional<QueryConfig> m_query_config{};
    std::vector<std::string> m_allowed_fields{};
    
    struct RestrictedFields {
        std::vector<std::string> m_fields{};
        
        void add(const std::vector<std::string>& fields) noexcept {
            m_fields.insert(m_fields.end(), fields.begin(), fields.end());
        }
        
        void add(const std::string& field) noexcept {
            m_fields.push_back(field);
        }
        
        bool is_restricted(const std::string& field) const noexcept {
            return std::find(m_fields.begin(), m_fields.end(), field) != m_fields.end();
        }
        
        std::vector<std::string>& fields() noexcept { return m_fields; }
        const std::vector<std::string>& fields() const noexcept { return m_fields; }
    };
    
    RestrictedFields m_restricted_fields{};
    size_t m_store_relations_limit{3};
    
    struct PublishableKeyContext {
        std::string m_key{};
        std::string m_sales_channel_id{};
        std::map<std::string, BodyValue> m_metadata{};
        
        PublishableKeyContext() noexcept = default;
        PublishableKeyContext(std::string&& key, std::string&& sales_channel_id) noexcept
            : m_key(std::move(key)), m_sales_channel_id(std::move(sales_channel_id)) {}
    };
    
    std::optional<PublishableKeyContext> m_publishable_key_context{};
    std::string m_ip{};
    std::string m_request_id{};
    std::chrono::system_clock::time_point m_timestamp{std::chrono::system_clock::now()};
    
    Request() noexcept = default;
    
    std::string get_header(const std::string& key) const noexcept {
        return m_headers.get(key).value_or("");
    }
    
    std::optional<std::string> get_query(const std::string& key) const noexcept {
        auto it = m_query.find(key);
        if (it == m_query.end() || it->second.empty()) return std::nullopt;
        return it->second.front();
    }
    
    std::vector<std::string> get_query_all(const std::string& key) const noexcept {
        auto it = m_query.find(key);
        if (it == m_query.end()) return {};
        return it->second;
    }
    
    std::string get_param(const std::string& key) const noexcept {
        auto it = m_params.find(key);
        if (it == m_params.end()) return "";
        return it->second;
    }
    
    template<typename T>
    std::optional<T> body_as() const noexcept {
        if (std::holds_alternative<T>(m_body)) {
            return std::get<T>(m_body);
        }
        return std::nullopt;
    }
    
    std::string& method() noexcept { return m_method; }
    const std::string& method() const noexcept { return m_method; }
    std::string& path() noexcept { return m_path; }
    const std::string& path() const noexcept { return m_path; }
    Headers& headers() noexcept { return m_headers; }
    const Headers& headers() const noexcept { return m_headers; }
    QueryParams& query() noexcept { return m_query; }
    const QueryParams& query() const noexcept { return m_query; }
    std::map<std::string, std::string>& params() noexcept { return m_params; }
    const std::map<std::string, std::string>& params() const noexcept { return m_params; }
    BodyValue& body() noexcept { return m_body; }
    const BodyValue& body() const noexcept { return m_body; }
    std::string& raw_body() noexcept { return m_raw_body; }
    const std::string& raw_body() const noexcept { return m_raw_body; }
    std::optional<AuthContext>& auth_context() noexcept { return m_auth_context; }
    const std::optional<AuthContext>& auth_context() const noexcept { return m_auth_context; }
    std::map<std::string, BodyValue>& additional_data() noexcept { return m_additional_data; }
    const std::map<std::string, BodyValue>& additional_data() const noexcept { return m_additional_data; }
    std::shared_ptr<Scope> scope() noexcept { return m_scope; }
    const std::shared_ptr<Scope> scope() const noexcept { return m_scope; }
    std::optional<QueryConfig>& query_config() noexcept { return m_query_config; }
    const std::optional<QueryConfig>& query_config() const noexcept { return m_query_config; }
    std::vector<std::string>& allowed_fields() noexcept { return m_allowed_fields; }
    const std::vector<std::string>& allowed_fields() const noexcept { return m_allowed_fields; }
    RestrictedFields& restricted_fields() noexcept { return m_restricted_fields; }
    const RestrictedFields& restricted_fields() const noexcept { return m_restricted_fields; }
    size_t store_relations_limit() const noexcept { return m_store_relations_limit; }
    void set_store_relations_limit(size_t limit) noexcept { m_store_relations_limit = limit; }
    std::optional<PublishableKeyContext>& publishable_key_context() noexcept { return m_publishable_key_context; }
    const std::optional<PublishableKeyContext>& publishable_key_context() const noexcept { return m_publishable_key_context; }
    std::string& ip() noexcept { return m_ip; }
    const std::string& ip() const noexcept { return m_ip; }
    std::string& request_id() noexcept { return m_request_id; }
    const std::string& request_id() const noexcept { return m_request_id; }
    std::chrono::system_clock::time_point timestamp() const noexcept { return m_timestamp; }
};

class Response {
public:
    int m_status_code{200};
    std::string m_status_message{"OK"};
    Headers m_headers{};
    BodyValue m_body{};
    std::string m_raw_body{};
    
    struct Cookie {
        std::string m_name{};
        std::string m_value{};
        std::optional<std::chrono::system_clock::time_point> m_expires{};
        std::optional<size_t> m_max_age{};
        std::string m_domain{};
        std::string m_path{"/"};
        bool m_secure{false};
        bool m_http_only{true};
        std::string m_same_site{"lax"};
        
        Cookie() noexcept = default;
        Cookie(std::string&& name, std::string&& value) noexcept
            : m_name(std::move(name)), m_value(std::move(value)) {}
    };
    
    std::vector<Cookie> m_cookies{};
    bool m_sent{false};
    
    Response() noexcept = default;
    
    void set_header(const std::string& key, const std::string& value) noexcept {
        m_headers.set(key, value);
    }
    
    void add_header(const std::string& key, const std::string& value) noexcept {
        m_headers.add(key, value);
    }
    
    void remove_header(const std::string& key) noexcept {
        m_headers.remove(key);
    }
    
    void set_cookie(const Cookie& cookie) noexcept {
        m_cookies.push_back(cookie);
    }
    
    void clear_cookie(const std::string& name, const std::string& path = "/", const std::string& domain = "") noexcept {
        Cookie cookie;
        cookie.m_name = name;
        cookie.m_value = "";
        cookie.m_path = path;
        cookie.m_domain = domain;
        cookie.m_max_age = 0;
        cookie.m_expires = std::chrono::system_clock::from_time_t(0);
        m_cookies.push_back(cookie);
    }
    
    template<typename T>
    void json(const T& value) noexcept {
        m_headers.set("Content-Type", "application/json");
    }
    
    void text(const std::string& text) noexcept {
        m_headers.set("Content-Type", "text/plain");
        m_body = text;
    }
    
    void html(const std::string& html) noexcept {
        m_headers.set("Content-Type", "text/html");
        m_body = html;
    }
    
    void binary(const std::vector<uint8_t>& data, const std::string& content_type = "application/octet-stream") noexcept {
        m_headers.set("Content-Type", content_type);
        m_body = data;
    }
    
    void redirect(const std::string& url, int code = 302) noexcept {
        m_status_code = code;
        m_headers.set("Location", url);
    }
    
    void status(int code, const std::string& message = "") noexcept {
        m_status_code = code;
        if (!message.empty()) m_status_message = message;
    }
    
    void send() noexcept { m_sent = true; }
    void end() noexcept { m_sent = true; }
    bool sent() const noexcept { return m_sent; }
    
    int status_code() const noexcept { return m_status_code; }
    const std::string& status_message() const noexcept { return m_status_message; }
    Headers& headers() noexcept { return m_headers; }
    const Headers& headers() const noexcept { return m_headers; }
    BodyValue& body() noexcept { return m_body; }
    const BodyValue& body() const noexcept { return m_body; }
    std::vector<Cookie>& cookies() noexcept { return m_cookies; }
    const std::vector<Cookie>& cookies() const noexcept { return m_cookies; }
};

class Request::Scope {
    std::map<std::type_index, std::shared_ptr<void>> m_services{};
    std::weak_ptr<Scope> m_parent{};
    
public:
    template<typename T>
    std::shared_ptr<T> resolve() noexcept {
        auto it = m_services.find(std::type_index(typeid(T)));
        if (it != m_services.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        return nullptr;
    }
    
    template<typename T>
    void register_service(std::shared_ptr<T> service) noexcept {
        m_services[std::type_index(typeid(T))] = service;
    }
    
    template<typename T, typename... Args>
    std::shared_ptr<T> create_service(Args&&... args) noexcept {
        auto service = std::make_shared<T>(std::forward<Args>(args)...);
        register_service(service);
        return service;
    }
    
    std::shared_ptr<Scope> create_child_scope() noexcept {
        auto child = std::make_shared<Scope>();
        child->m_parent = shared_from_this();
        return child;
    }
};

struct ErrorResponse {
    std::string m_type{};
    std::string m_message{};
    std::optional<std::string> m_code{};
    std::optional<BodyValue> m_details{};
    
    ErrorResponse() noexcept = default;
    ErrorResponse(std::string&& type, std::string&& message) noexcept
        : m_type(std::move(type)), m_message(std::move(message)) {}
    
    static ErrorResponse not_found(const std::string& message = "Resource not found") noexcept {
        return {"NOT_FOUND", message};
    }
    
    static ErrorResponse invalid_data(const std::string& message = "Invalid data") noexcept {
        return {"INVALID_DATA", message};
    }
    
    static ErrorResponse not_allowed(const std::string& message = "Operation not allowed") noexcept {
        return {"NOT_ALLOWED", message};
    }
    
    static ErrorResponse unauthorized(const std::string& message = "Unauthorized") noexcept {
        return {"UNAUTHORIZED", message};
    }
    
    static ErrorResponse forbidden(const std::string& message = "Forbidden") noexcept {
        return {"FORBIDDEN", message};
    }
    
    static ErrorResponse internal(const std::string& message = "Internal server error") noexcept {
        return {"INTERNAL_ERROR", message};
    }
};

struct Pagination {
    size_t m_page{1};
    size_t m_limit{50};
    size_t m_total{0};
    size_t m_total_pages{0};
    
    Pagination() noexcept = default;
    Pagination(size_t page, size_t limit, size_t total) noexcept
        : m_page(page), m_limit(limit), m_total(total) {
        m_total_pages = (total + limit - 1) / limit;
    }
    
    size_t offset() const noexcept { return (m_page - 1) * m_limit; }
    bool has_next() const noexcept { return m_page < m_total_pages; }
    bool has_prev() const noexcept { return m_page > 1; }
    
    size_t page() const noexcept { return m_page; }
    size_t limit() const noexcept { return m_limit; }
    size_t total() const noexcept { return m_total; }
    size_t total_pages() const noexcept { return m_total_pages; }
};

template<typename T>
struct PaginatedResponse {
    std::vector<T> m_data{};
    Pagination m_pagination{};
    
    PaginatedResponse() noexcept = default;
    PaginatedResponse(std::vector<T>&& data, Pagination&& pagination) noexcept
        : m_data(std::move(data)), m_pagination(std::move(pagination)) {}
    
    std::vector<T>& data() noexcept { return m_data; }
    const std::vector<T>& data() const noexcept { return m_data; }
    Pagination& pagination() noexcept { return m_pagination; }
    const Pagination& pagination() const noexcept { return m_pagination; }
};

struct FieldSelection {
    std::vector<std::string> m_fields{};
    std::vector<std::string> m_relations{};
    
    FieldSelection() noexcept = default;
    
    static FieldSelection from_query(const QueryParams& query) noexcept {
        FieldSelection fs;
        // Parse fields and relations from query
        return fs;
    }
    
    bool includes(const std::string& field) const noexcept {
        return std::find(m_fields.begin(), m_fields.end(), field) != m_fields.end();
    }
    
    bool includes_relation(const std::string& relation) const noexcept {
        return std::find(m_relations.begin(), m_relations.end(), relation) != m_relations.end();
    }
    
    std::vector<std::string>& fields() noexcept { return m_fields; }
    const std::vector<std::string>& fields() const noexcept { return m_fields; }
    std::vector<std::string>& relations() noexcept { return m_relations; }
    const std::vector<std::string>& relations() const noexcept { return m_relations; }
};

struct Filter {
    std::map<std::string, BodyValue> m_conditions{};
    
    Filter() noexcept = default;
    
    static Filter from_query(const QueryParams& query, const std::vector<std::string>& allowed_fields) noexcept {
        Filter f;
        return f;
    }
    
    std::string to_sql_where() const noexcept { return ""; }
    std::vector<BodyValue> to_sql_params() const noexcept { return {}; }
    
    std::map<std::string, BodyValue>& conditions() noexcept { return m_conditions; }
    const std::map<std::string, BodyValue>& conditions() const noexcept { return m_conditions; }
};

} // namespace http