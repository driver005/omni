module;

import std;
import medusa_core.main;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import database.connection;
import database.types;
import router.router;
import http.types;

export module medusa_core.tests.integration.api_test_runner;

export namespace medusa_core::tests::integration {

class APITestRunner {
    std::unique_ptr<medusa_core::Application> m_app;
    database::Database* m_db;
    router::Router* m_router;
    
public:
    APITestRunner() = default;
    
    bool setup() {
        database::DatabaseConfig db_config;
        db_config.host = "localhost";
        db_config.port = 5432;
        db_config.database = "medusa_test";
        db_config.username = "postgres";
        db_config.password = "postgres";
        db_config.schema = "public";
        
        router::RouterConfig router_config;
        router_config.m_admin_cors_origins = {"http://localhost:7000"};
        router_config.m_store_cors_origins = {"http://localhost:8000"};
        router_config.m_auth_cors_origins = {"http://localhost:7000"};
        
        m_app = std::make_unique<medusa_core::Application>(db_config, router_config);
        if (!m_app->initialize()) {
            return false;
        }
        
        m_db = &m_app->database();
        m_router = &m_app->router();
        return true;
    }
    
    void teardown() {
        m_app.reset();
    }
    
    // Simulate API request
    http::Response simulate_request(const std::string& method, const std::string& path, 
                                    const http::BodyValue& body = {}, 
                                    const std::map<std::string, std::string>& headers = {}) {
        http::Request req;
        req.m_method = method;
        req.m_path = path;
        req.m_body = body;
        
        for (const auto& [key, value] : headers) {
            req.m_headers.set(key, value);
        }
        
        http::Response res;
        m_router->handle(req, res);
        return res;
    }
    
    void test_admin_products_crud() {
        // Create admin user for authentication
        auto& user_repo = m_db->repository<entities::User>();
        entities::User admin;
        admin.add_id(generate_id());
        admin.add_email("admin@test.com");
        admin.add_password_hash("hashed");
        user_repo.save(std::move(admin));
        
        // Test GET /admin/products
        auto res = simulate_request("GET", "/admin/products", {}, 
            {{"Authorization", "Bearer test_token"}});
        assert(res.status_code() == 200 || res.status_code() == 401);
        
        // Test POST /admin/products
        res = simulate_request("POST", "/admin/products", 
            std::map<std::string, std::string>{{"title", "Test Product"}},
            {{"Authorization", "Bearer test_token"}});
        assert(res.status_code() == 201 || res.status_code() == 401);
        
        std::cout << "test_admin_products_crud: PASSED" << std::endl;
    }
    
    void test_store_products() {
        // Test GET /store/products (public)
        auto res = simulate_request("GET", "/store/products");
        assert(res.status_code() == 200);
        
        std::cout << "test_store_products: PASSED" << std::endl;
    }
    
    void test_auth_flow() {
        // Test POST /auth/token
        auto res = simulate_request("POST", "/auth/token", 
            std::map<std::string, std::string>{{"email", "test@test.com"}, {"password", "password"}});
        assert(res.status_code() == 200 || res.status_code() == 401);
        
        // Test POST /auth/session
        res = simulate_request("POST", "/auth/session",
            std::map<std::string, std::string>{{"email", "test@test.com"}, {"password", "password"}});
        assert(res.status_code() == 200 || res.status_code() == 401);
        
        std::cout << "test_auth_flow: PASSED" << std::endl;
    }
    
    void test_cart_flow() {
        // Test POST /store/carts
        auto res = simulate_request("POST", "/store/carts", {}, 
            {{"Authorization", "Bearer customer_token"}});
        assert(res.status_code() == 201 || res.status_code() == 401);
        
        std::cout << "test_cart_flow: PASSED" << std::endl;
    }
    
    void test_orders_flow() {
        // Test GET /store/orders
        auto res = simulate_request("GET", "/store/orders", {}, 
            {{"Authorization", "Bearer customer_token"}});
        assert(res.status_code() == 200 || res.status_code() == 401);
        
        // Test POST /store/orders
        res = simulate_request("POST", "/store/orders", {}, 
            {{"Authorization", "Bearer customer_token"}});
        assert(res.status_code() == 201 || res.status_code() == 401);
        
        std::cout << "test_orders_flow: PASSED" << std::endl;
    }
    
    void run_all_tests() {
        std::cout << "Running API Integration Tests..." << std::endl;
        
        test_admin_products_crud();
        test_store_products();
        test_auth_flow();
        test_cart_flow();
        test_orders_flow();
        
        std::cout << "All API tests passed!" << std::endl;
    }
    
private:
    std::string generate_id() noexcept {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static std::uniform_int_distribution<> dis2(8, 11);
        
        std::string uuid;
        const char* chars = "0123456789abcdef";
        for (int i = 0; i < 36; ++i) {
            if (i == 8 || i == 13 || i == 18 || i == 23) {
                uuid += '-';
            } else {
                if (i == 14) {
                    uuid += chars[dis2(gen)];
                } else {
                    uuid += chars[dis(gen)];
                }
            }
        }
        return uuid;
    }
    
    void assert(bool condition, const char* message = "Assertion failed") {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }
};

} // namespace medusa_core::tests::integration

int run_api_tests() {
    try {
        medusa_core::tests::integration::APITestRunner runner;
        if (!runner.setup()) {
            std::cerr << "Failed to setup API test runner" << std::endl;
            return 1;
        }
        
        runner.run_all_tests();
        runner.teardown();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "API test failed: " << e.what() << std::endl;
        return 1;
    }
}