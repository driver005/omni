module;

import std;
import medusa_core.main;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import database.connection;
import database.types;
import router.router;
import http.types;

export module medusa_core.tests.integration.auth;

export namespace medusa_core::tests::integration {

class AuthTestSuite {
    std::unique_ptr<medusa_core::Application> m_app;
    database::Database* m_db;
    router::Router* m_router;
    
public:
    AuthTestSuite() = default;
    
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
    
    void test_user_registration() {
        auto& user_repo = m_db->repository<entities::User>();
        
        entities::User user;
        user.add_id(generate_id());
        user.add_email("newuser@example.com");
        user.add_first_name("New");
        user.add_last_name("User");
        user.add_password_hash("hashed_password");
        
        auto saved = user_repo.save(std::move(user));
        
        assert(saved.get_id() != "");
        assert(saved.get_email() == "newuser@example.com");
        
        std::cout << "test_user_registration: PASSED" << std::endl;
    }
    
    void test_customer_registration() {
        auto& customer_repo = m_db->repository<entities::Customer>();
        
        entities::Customer customer;
        customer.add_id(generate_id());
        customer.add_email("newcustomer@example.com");
        customer.add_first_name("New");
        customer.add_last_name("Customer");
        customer.add_has_account(true);
        
        auto saved = customer_repo.save(std::move(customer));
        
        assert(saved.get_id() != "");
        assert(saved.get_email() == "newcustomer@example.com");
        assert(saved.get_has_account() == true);
        
        std::cout << "test_customer_registration: PASSED" << std::endl;
    }
    
    void test_api_key_creation() {
        auto& user_repo = m_db->repository<entities::User>();
        auto& api_key_repo = m_db->repository<entities::ApiKey>();
        
        entities::User user;
        user.add_id(generate_id());
        user.add_email("apikey_test@example.com");
        user = user_repo.save(std::move(user));
        
        entities::ApiKey api_key;
        api_key.add_id(generate_id());
        api_key.add_user_id(user.get_id());
        api_key.add_title("Test API Key");
        api_key.add_key_hash("hashed_key");
        api_key = api_key_repo.save(std::move(api_key));
        
        assert(api_key.get_id() != "");
        assert(api_key.get_user_id() == user.get_id());
        assert(api_key.get_title() == "Test API Key");
        
        std::cout << "test_api_key_creation: PASSED" << std::endl;
    }
    
    void test_publishable_key() {
        auto& sales_channel_repo = m_db->repository<entities::SalesChannel>();
        auto& pub_key_repo = m_db->repository<entities::PublishableApiKey>();
        
        entities::SalesChannel channel;
        channel.add_id(generate_id());
        channel.add_name("Test Channel");
        channel = sales_channel_repo.save(std::move(channel));
        
        entities::PublishableApiKey pub_key;
        pub_key.add_id(generate_id());
        pub_key.add_sales_channel_id(channel.get_id());
        pub_key.add_key("pk_test_1234567890");
        pub_key = pub_key_repo.save(std::move(pub_key));
        
        assert(pub_key.get_id() != "");
        assert(pub_key.get_key() == "pk_test_1234567890");
        assert(pub_key.get_sales_channel_id() == channel.get_id());
        
        std::cout << "test_publishable_key: PASSED" << std::endl;
    }
    
    void run_all_tests() {
        std::cout << "Running Auth Integration Tests..." << std::endl;
        
        test_user_registration();
        test_customer_registration();
        test_api_key_creation();
        test_publishable_key();
        
        std::cout << "All Auth tests passed!" << std::endl;
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

int run_auth_tests() {
    try {
        medusa_core::tests::integration::AuthTestSuite suite;
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