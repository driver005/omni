module;

import std;
import medusa_core.tests.integration.product;
import medusa_core.tests.integration.order;
import medusa_core.tests.integration.customer;
import medusa_core.tests.integration.cart;
import medusa_core.tests.integration.auth;
import medusa_core.tests.integration.api_test_runner;

export module medusa_core.tests.integration.main;

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Medusa C++ Backend Integration Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    
    int failed = 0;
    
    // Run product tests
    std::cout << "\n--- Product Tests ---" << std::endl;
    if (run_product_tests() != 0) {
        failed++;
        std::cerr << "Product tests FAILED" << std::endl;
    }
    
    // Run order tests
    std::cout << "\n--- Order Tests ---" << std::endl;
    if (run_order_tests() != 0) {
        failed++;
        std::cerr << "Order tests FAILED" << std::endl;
    }
    
    // Run customer tests
    std::cout << "\n--- Customer Tests ---" << std::endl;
    if (run_customer_tests() != 0) {
        failed++;
        std::cerr << "Customer tests FAILED" << std::endl;
    }
    
    // Run cart tests
    std::cout << "\n--- Cart Tests ---" << std::endl;
    if (run_cart_tests() != 0) {
        failed++;
        std::cerr << "Cart tests FAILED" << std::endl;
    }
    
    // Run auth tests
    std::cout << "\n--- Auth Tests ---" << std::endl;
    if (run_auth_tests() != 0) {
        failed++;
        std::cerr << "Auth tests FAILED" << std::endl;
    }
    
    // Run API tests
    std::cout << "\n--- API Tests ---" << std::endl;
    if (run_api_tests() != 0) {
        failed++;
        std::cerr << "API tests FAILED" << std::endl;
    }
    
    std::cout << "\n========================================" << std::endl;
    if (failed == 0) {
        std::cout << "ALL TESTS PASSED!" << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    } else {
        std::cerr << failed << " TEST SUITE(S) FAILED!" << std::endl;
        std::cout << "========================================" << std::endl;
        return 1;
    }
}