module;

import std;
import medusa_core.main;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import database.connection;
import database.types;
import router.router;
import http.types;

export module medusa_core.tests.integration.order;

export namespace medusa_core::tests::integration {

class OrderTestSuite {
    std::unique_ptr<medusa_core::Application> m_app;
    database::Database* m_db;
    router::Router* m_router;
    
public:
    OrderTestSuite() = default;
    
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
    
    void test_create_order() {
        auto& order_repo = m_db->repository<entities::Order>();
        auto& customer_repo = m_db->repository<entities::Customer>();
        auto& region_repo = m_db->repository<entities::Region>();
        
        // Create prerequisite entities
        entities::Customer customer;
        customer.add_id(generate_id());
        customer.add_email("test@example.com");
        customer = customer_repo.save(std::move(customer));
        
        entities::Region region;
        region.add_id(generate_id());
        region.add_name("Test Region");
        region.add_currency_code("usd");
        region = region_repo.save(std::move(region));
        
        entities::Currency currency;
        currency.add_id("usd");
        currency.add_name("US Dollar");
        currency.add_symbol("$");
        auto& currency_repo = m_db->repository<entities::Currency>();
        currency_repo.save(std::move(currency));
        
        // Create order
        entities::Order order;
        order.add_id(generate_id());
        order.add_customer_id(customer.get_id());
        order.add_region_id(region.get_id());
        order.add_currency_code("usd");
        order.add_status("pending");
        order.add_payment_status("pending");
        order.add_fulfillment_status("not_fulfilled");
        order.add_total("10000"); // $100.00 in cents
        order.add_subtotal("9000");
        order.add_tax_total("1000");
        order.add_shipping_total("0");
        order.add_discount_total("0");
        
        auto saved = order_repo.save(std::move(order));
        
        assert(saved.get_id() != "");
        assert(saved.get_customer_id() == customer.get_id());
        assert(saved.get_status() == "pending");
        assert(saved.get_total() == "10000");
        
        std::cout << "test_create_order: PASSED" << std::endl;
    }
    
    void test_get_order() {
        auto& order_repo = m_db->repository<entities::Order>();
        auto& customer_repo = m_db->repository<entities::Customer>();
        auto& region_repo = m_db->repository<entities::Region>();
        
        // Create prerequisite entities
        entities::Customer customer;
        customer.add_id(generate_id());
        customer.add_email("get_test@example.com");
        customer = customer_repo.save(std::move(customer));
        
        entities::Region region;
        region.add_id(generate_id());
        region.add_name("Get Test Region");
        region.add_currency_code("usd");
        region = region_repo.save(std::move(region));
        
        // Create order
        entities::Order order;
        order.add_id(generate_id());
        order.add_customer_id(customer.get_id());
        order.add_region_id(region.get_id());
        order.add_currency_code("usd");
        order.add_status("completed");
        order.add_total("5000");
        auto saved = order_repo.save(std::move(order));
        std::string id = saved.get_id();
        
        // Fetch order
        auto found = order_repo.find(id);
        assert(found.has_value());
        assert(found->get_status() == "completed");
        assert(found->get_total() == "5000");
        
        std::cout << "test_get_order: PASSED" << std::endl;
    }
    
    void test_list_orders() {
        auto& order_repo = m_db->repository<entities::Order>();
        auto& customer_repo = m_db->repository<entities::Customer>();
        auto& region_repo = m_db->repository<entities::Region>();
        
        // Create prerequisite entities
        entities::Customer customer;
        customer.add_id(generate_id());
        customer.add_email("list_test@example.com");
        customer = customer_repo.save(std::move(customer));
        
        entities::Region region;
        region.add_id(generate_id());
        region.add_name("List Test Region");
        region.add_currency_code("usd");
        region = region_repo.save(std::move(region));
        
        // Create multiple orders
        for (int i = 0; i < 5; ++i) {
            entities::Order order;
            order.add_id(generate_id());
            order.add_customer_id(customer.get_id());
            order.add_region_id(region.get_id());
            order.add_currency_code("usd");
            order.add_status(i % 2 == 0 ? "completed" : "pending");
            order.add_total(std::to_string((i + 1) * 1000));
            order_repo.save(std::move(order));
        }
        
        // List all
        auto all = order_repo.find_by({{"customer_id", customer.get_id()}});
        assert(all.size() == 5);
        
        // Filter by status
        auto completed = order_repo.find_by({{"customer_id", customer.get_id()}, {"status", std::string("completed")}});
        assert(completed.size() == 3); // 0, 2, 4 are completed
        
        for (const auto& o : completed) {
            assert(o.get_status() == "completed");
        }
        
        std::cout << "test_list_orders: PASSED" << std::endl;
    }
    
    void test_cancel_order() {
        auto& order_repo = m_db->repository<entities::Order>();
        auto& customer_repo = m_db->repository<entities::Customer>();
        auto& region_repo = m_db->repository<entities::Region>();
        
        // Create prerequisite entities
        entities::Customer customer;
        customer.add_id(generate_id());
        customer.add_email("cancel_test@example.com");
        customer = customer_repo.save(std::move(customer));
        
        entities::Region region;
        region.add_id(generate_id());
        region.add_name("Cancel Test Region");
        region.add_currency_code("usd");
        region = region_repo.save(std::move(region));
        
        // Create order
        entities::Order order;
        order.add_id(generate_id());
        order.add_customer_id(customer.get_id());
        order.add_region_id(region.get_id());
        order.add_currency_code("usd");
        order.add_status("pending");
        auto saved = order_repo.save(std::move(order));
        std::string id = saved.get_id();
        
        // Cancel order
        auto found = order_repo.find(id);
        assert(found.has_value());
        found->set_status(std::string("cancelled"));
        auto cancelled = order_repo.save(std::move(*found));
        
        assert(cancelled.get_status() == "cancelled");
        
        // Try to cancel again - should fail in real implementation
        found = order_repo.find(id);
        assert(found.has_value());
        assert(found->get_status() == "cancelled");
        
        std::cout << "test_cancel_order: PASSED" << std::endl;
    }
    
    void test_order_line_items() {
        auto& order_repo = m_db->repository<entities::Order>();
        auto& line_item_repo = m_db->repository<entities::LineItem>();
        auto& variant_repo = m_db->repository<entities::ProductVariant>();
        auto& product_repo = m_db->repository<entities::Product>();
        auto& customer_repo = m_db->repository<entities::Customer>();
        auto& region_repo = m_db->repository<entities::Region>();
        
        // Create prerequisite entities
        entities::Customer customer;
        customer.add_id(generate_id());
        customer.add_email("lineitem_test@example.com");
        customer = customer_repo.save(std::move(customer));
        
        entities::Region region;
        region.add_id(generate_id());
        region.add_name("LineItem Test Region");
        region.add_currency_code("usd");
        region = region_repo.save(std::move(region));
        
        entities::Product product;
        product.add_id(generate_id());
        product.add_title("Test Product");
        product = product_repo.save(std::move(product));
        
        entities::ProductVariant variant;
        variant.add_id(generate_id());
        variant.add_product_id(product.get_id());
        variant.add_title("Test Variant");
        variant.add_price("5000");
        variant = variant_repo.save(std::move(variant));
        
        // Create order
        entities::Order order;
        order.add_id(generate_id());
        order.add_customer_id(customer.get_id());
        order.add_region_id(region.get_id());
        order.add_currency_code("usd");
        order.add_status("pending");
        order.add_total("10000");
        order = order_repo.save(std::move(order));
        
        // Add line items
        entities::LineItem item1;
        item1.add_id(generate_id());
        item1.add_order_id(order.get_id());
        item1.add_variant_id(variant.get_id());
        item1.add_title("Test Variant");
        item1.add_quantity(2);
        item1.add_unit_price("5000");
        item1.add_total("10000");
        line_item_repo.save(std::move(item1));
        
        // Verify line items
        auto items = line_item_repo.find_by({{"order_id", order.get_id()}});
        assert(items.size() == 1);
        assert(items[0].get_quantity() == 2);
        assert(items[0].get_total() == "10000");
        
        std::cout << "test_order_line_items: PASSED" << std::endl;
    }
    
    void run_all_tests() {
        std::cout << "Running Order Integration Tests..." << std::endl;
        
        test_create_order();
        test_get_order();
        test_list_orders();
        test_cancel_order();
        test_order_line_items();
        
        std::cout << "All Order tests passed!" << std::endl;
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

int run_order_tests() {
    try {
        medusa_core::tests::integration::OrderTestSuite suite;
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