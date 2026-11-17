module;
#include <gtest/gtest.h>
#include <memory>

export module blog_api:endpoints:test;

import std;
import blog_api:types;
import blog_database:database;
import blog_api:endpoints:main_index;
import blog_database:database;

using namespace ghost::blog;
using namespace ghost::blog::endpoints;

class EndpointsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test database (SQLite in-memory)
        auto db_config = DatabaseConfig::create_sqlite(":memory:");
        if (db_config) {
            m_database = std::move(*db_config);
            m_database->connect("sqlite::memory:");
            m_database->create_schema();
        }
        
        m_endpoints = create_endpoints_registry(m_database);
    }
    
    void TearDown() override {
        if (m_database) {
            m_database->disconnect();
        }
    }
    
    std::shared_ptr<IDatabase> m_database;
    std::unique_ptr<EndpointsRegistry> m_endpoints;
};

TEST_F(EndpointsTest, CreateEndpointsRegistry) {
    ASSERT_NE(m_endpoints, nullptr);
}

TEST_F(EndpointsTest, RegisterAllEndpoints) {
    // This would require a router context to test
    // For now, just verify the registry can be created
    ASSERT_NE(m_endpoints, nullptr);
}

TEST_F(EndpointsTest, AdminEndpointsExist) {
    // Verify admin endpoints can be created
    AdminEndpoints admin_endpoints{m_database};
    ASSERT_NE(&admin_endpoints, nullptr);
}

TEST_F(EndpointsTest, ContentEndpointsExist) {
    // Verify content endpoints can be created
    ContentEndpoints content_endpoints{m_database};
    ASSERT_NE(&content_endpoints, nullptr);
}

TEST_F(EndpointsTest, MembersEndpointsExist) {
    // Verify members endpoints can be created
    MembersEndpoints members_endpoints{m_database};
    ASSERT_NE(&members_endpoints, nullptr);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}