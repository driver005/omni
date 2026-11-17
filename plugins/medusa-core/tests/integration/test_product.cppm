module;

import std;
import medusa_core.main;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import database.connection;
import database.types;
import router.router;
import http.types;

export module medusa_core.tests.integration.product;

export namespace medusa_core::tests::integration {

class ProductTestSuite {
    std::unique_ptr<medusa_core::Application> m_app;
    database::Database* m_db;
    router::Router* m_router;
    
public:
    ProductTestSuite() = default;
    
    bool setup() {
        database::DatabaseConfig db_config;
        db_config.host = "localhost";
        db_config.port = 5432;
        db_config.database = "medusa_test";
        db_config.username = "postgres";
        db_config.password = "postgres";
        db_config.schema = "public";
        
        router::RouterConfig router_config;
        
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
    
    void test_create_product() {
        auto& repo = m_db->repository<entities::Product>();
        
        entities::Product product;
        product.add_id(generate_id());
        product.add_title("Test Product");
        product.add_description("Test Description");
        product.add_handle("test-product");
        product.add_status("published");
        product.add_weight(100);
        
        auto saved = repo.save(std::move(product));
        
        // Verify product was created
        assert(saved.get_id() != "");
        assert(saved.get_title() == "Test Product");
        assert(saved.get_status() == "published");
        
        std::cout << "test_create_product: PASSED" << std::endl;
    }
    
    void test_get_product() {
        auto& repo = m_db->repository<entities::Product>();
        
        // Create product first
        entities::Product product;
        product.add_id(generate_id());
        product.add_title("Get Test Product");
        product.add_status("published");
        auto saved = repo.save(std::move(product));
        std::string id = saved.get_id();
        
        // Fetch product
        auto found = repo.find(id);
        assert(found.has_value());
        assert(found->get_title() == "Get Test Product");
        
        std::cout << "test_get_product: PASSED" << std::endl;
    }
    
    void test_list_products() {
        auto& repo = m_db->repository<entities::Product>();
        
        // Create multiple products
        for (int i = 0; i < 5; ++i) {
            entities::Product product;
            product.add_id(generate_id());
            product.add_title("Product " + std::to_string(i));
            product.add_status(i % 2 == 0 ? "published" : "draft");
            repo.save(std::move(product));
        }
        
        // List all
        auto all = repo.find_all();
        assert(all.size() >= 5);
        
        // Filter by status
        auto published = repo.find_by({{"status", std::string("published")}});
        for (const auto& p : published) {
            assert(p.get_status() == "published");
        }
        
        std::cout << "test_list_products: PASSED" << std::endl;
    }
    
    void test_update_product() {
        auto& repo = m_db->repository<entities::Product>();
        
        entities::Product product;
        product.add_id(generate_id());
        product.add_title("Original Title");
        product.add_status("draft");
        auto saved = repo.save(std::move(product));
        std::string id = saved.get_id();
        
        // Update product
        auto found = repo.find(id);
        assert(found.has_value());
        found->set_title(std::string("Updated Title"));
        found->set_status(std::string("published"));
        auto updated = repo.save(std::move(*found));
        
        assert(updated.get_title() == "Updated Title");
        assert(updated.get_status() == "published");
        
        std::cout << "test_update_product: PASSED" << std::endl;
    }
    
    void test_delete_product() {
        auto& repo = m_db->repository<entities::Product>();
        
        entities::Product product;
        product.add_id(generate_id());
        product.add_title("To Delete");
        auto saved = repo.save(std::move(product));
        std::string id = saved.get_id();
        
        // Verify exists
        assert(repo.find(id).has_value());
        
        // Delete
        repo.remove(*repo.find(id).value());
        
        // Verify deleted (soft delete)
        auto deleted = repo.find(id);
        assert(deleted.has_value());
        assert(deleted->get_deleted_at().has_value());
        
        std::cout << "test_delete_product: PASSED" << std::endl;
    }
    
    void run_all_tests() {
        std::cout << "Running Product Integration Tests..." << std::endl;
        
        test_create_product();
        test_get_product();
        test_list_products();
        test_update_product();
        test_delete_product();
        
        std::cout << "All Product tests passed!" << std::endl;
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

// Standalone test runner
int run_product_tests() {
    try {
        medusa_core::tests::integration::ProductTestSuite suite;
        if (!suite.setup()) {
            std::cerr << "Failed to setup test suite" << std::endl;
            return 1;
        }
        
        suite.run_all_tests();
        suite.teardown();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}