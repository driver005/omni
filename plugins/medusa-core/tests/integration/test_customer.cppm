module;

import std;
import medusa_core.main;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import database.connection;
import database.types;
import router.router;
import http.types;

export module medusa_core.tests.integration.customer;

export namespace medusa_core::tests::integration {

class CustomerTestSuite {
    std::unique_ptr<medusa_core::Application> m_app;
    database::Database* m_db;
    router::Router* m_router;
    
public:
    CustomerTestSuite() = default;
    
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
    
    void test_create_customer() {
        auto& repo = m_db->repository<entities::Customer>();
        
        entities::Customer customer;
        customer.add_id(generate_id());
        customer.add_email("create_test@example.com");
        customer.add_first_name("John");
        customer.add_last_name("Doe");
        customer.add_phone("+1234567890");
        customer.add_has_account(true);
        
        auto saved = repo.save(std::move(customer));
        
        assert(saved.get_id() != "");
        assert(saved.get_email() == "create_test@example.com");
        assert(saved.get_first_name() == "John");
        assert(saved.get_has_account() == true);
        
        std::cout << "test_create_customer: PASSED" << std::endl;
    }
    
    void test_get_customer() {
        auto& repo = m_db->repository<entities::Customer>();
        
        entities::Customer customer;
        customer.add_id(generate_id());
        customer.add_email("get_test@example.com");
        customer.add_first_name("Jane");
        auto saved = repo.save(std::move(customer));
        std::string id = saved.get_id();
        
        auto found = repo.find(id);
        assert(found.has_value());
        assert(found->get_email() == "get_test@example.com");
        assert(found->get_first_name() == "Jane");
        
        std::cout << "test_get_customer: PASSED" << std::endl;
    }
    
    void test_list_customers() {
        auto& repo = m_db->repository<entities::Customer>();
        
        for (int i = 0; i < 5; ++i) {
            entities::Customer customer;
            customer.add_id(generate_id());
            customer.add_email("list" + std::to_string(i) + "@example.com");
            customer.add_first_name("User" + std::to_string(i));
            customer.add_has_account(i % 2 == 0);
            repo.save(std::move(customer));
        }
        
        auto all = repo.find_all();
        assert(all.size() >= 5);
        
        auto with_account = repo.find_by({{"has_account", true}});
        for (const auto& c : with_account) {
            assert(c.get_has_account() == true);
        }
        
        std::cout << "test_list_customers: PASSED" << std::endl;
    }
    
    void test_update_customer() {
        auto& repo = m_db->repository<entities::Customer>();
        
        entities::Customer customer;
        customer.add_id(generate_id());
        customer.add_email("update_test@example.com");
        customer.add_first_name("Original");
        auto saved = repo.save(std::move(customer));
        std::string id = saved.get_id();
        
        auto found = repo.find(id);
        assert(found.has_value());
        found->set_first_name(std::string("Updated"));
        found->set_last_name(std::string("Name"));
        found->set_phone(std::string("+1987654321"));
        auto updated = repo.save(std::move(*found));
        
        assert(updated.get_first_name() == "Updated");
        assert(updated.get_last_name() == "Name");
        assert(updated.get_phone() == "+1987654321");
        
        std::cout << "test_update_customer: PASSED" << std::endl;
    }
    
    void test_delete_customer() {
        auto& repo = m_db->repository<entities::Customer>();
        
        entities::Customer customer;
        customer.add_id(generate_id());
        customer.add_email("delete_test@example.com");
        auto saved = repo.save(std::move(customer));
        std::string id = saved.get_id();
        
        assert(repo.find(id).has_value());
        
        repo.remove(*repo.find(id).value());
        
        auto deleted = repo.find(id);
        assert(deleted.has_value());
        assert(deleted->get_deleted_at().has_value());
        
        std::cout << "test_delete_customer: PASSED" << std::endl;
    }
    
    void test_customer_addresses() {
        auto& customer_repo = m_db->repository<entities::Customer>();
        auto& address_repo = m_db->repository<entities::Address>();
        
        entities::Customer customer;
        customer.add_id(generate_id());
        customer.add_email("address_test@example.com");
        customer = customer_repo.save(std::move(customer));
        std::string customer_id = customer.get_id();
        
        // Add addresses
        entities::Address addr1;
        addr1.add_id(generate_id());
        addr1.add_customer_id(customer_id);
        addr1.add_first_name("John");
        addr1.add_address_1("123 Main St");
        addr1.add_city("New York");
        addr1.add_country_code("US");
        addr1.add_is_default_billing(true);
        addr1.add_is_default_shipping(true);
        address_repo.save(std::move(addr1));
        
        entities::Address addr2;
        addr2.add_id(generate_id());
        addr2.add_customer_id(customer_id);
        addr2.add_first_name("Jane");
        addr2.add_address_1("456 Oak Ave");
        addr2.add_city("Los Angeles");
        addr2.add_country_code("US");
        address_repo.save(std::move(addr2));
        
        // Fetch addresses
        auto addresses = address_repo.find_by({{"customer_id", customer_id}});
        assert(addresses.size() == 2);
        
        std::cout << "test_customer_addresses: PASSED" << std::endl;
    }
    
    void run_all_tests() {
        std::cout << "Running Customer Integration Tests..." << std::endl;
        
        test_create_customer();
        test_get_customer();
        test_list_customers();
        test_update_customer();
        test_delete_customer();
        test_customer_addresses();
        
        std::cout << "All Customer tests passed!" << std::endl;
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

int run_customer_tests() {
    try {
        medusa_core::tests::integration::CustomerTestSuite suite;
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