export module medusa_core.main;

import std;
import database.connection;
import database.types;
import router.router;
import http.types;
import medusa_core.api.routes;
import medusa_core.modules.module;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;

export namespace medusa_core {

class Application {
    std::unique_ptr<database::Database> m_db;
    std::unique_ptr<router::Router> m_router;
    std::unique_ptr<modules::ModuleManager> m_module_manager;
    database::DatabaseConfig m_db_config;
    router::RouterConfig m_router_config;
    bool m_initialized{false};
    
public:
    Application() noexcept = default;
    
    explicit Application(const database::DatabaseConfig& db_config, 
                         const router::RouterConfig& router_config = {}) noexcept
        : m_db_config(db_config)
        , m_router_config(router_config) {}
    
    ~Application() {
        shutdown();
    }
    
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    
    Application(Application&&) noexcept = default;
    Application& operator=(Application&&) noexcept = default;
    
    bool initialize() {
        if (m_initialized) return true;
        
        try {
            // Initialize database
            m_db = database::Database::create(m_db_config);
            if (!m_db->connect()) {
                return false;
            }
            
            // Initialize router
            m_router = router::Router::create(m_router_config);
            
            // Initialize module manager
            m_module_manager = std::make_unique<modules::ModuleManager>(*m_db);
            
            // Register core modules
            register_core_modules();
            
            // Initialize all modules
            if (!m_module_manager->initialize_all()) {
                return false;
            }
            
            // Register all API routes
            medusa_core::api::register_all_routes(*m_router, *m_db);
            
            m_initialized = true;
            return true;
        } catch (const std::exception& e) {
            return false;
        }
    }
    
    void shutdown() {
        if (m_module_manager) {
            m_module_manager->shutdown_all();
        }
        if (m_db) {
            m_db->disconnect();
        }
        m_initialized = false;
    }
    
    bool is_initialized() const noexcept { return m_initialized; }
    
    database::Database& database() noexcept { return *m_db; }
    const database::Database& database() const noexcept { return *m_db; }
    
    router::Router& router() noexcept { return *m_router; }
    const router::Router& router() const noexcept { return *m_router; }
    
    modules::ModuleManager& module_manager() noexcept { return *m_module_manager; }
    const modules::ModuleManager& module_manager() const noexcept { return *m_module_manager; }
    
    void run(uint16_t port = 9000, const std::string& host = "0.0.0.0") {
        if (!m_initialized) {
            if (!initialize()) {
                throw std::runtime_error("Failed to initialize application");
            }
        }
        
        // In real implementation, start HTTP server here
        // For now, just block
        std::cout << "Medusa C++ Backend running on " << host << ":" << port << std::endl;
        std::cout << "Admin API: http://" << host << ":" << port << "/admin" << std::endl;
        std::cout << "Store API: http://" << host << ":" << port << "/store" << std::endl;
        std::cout << "Auth API: http://" << host << ":" << port << "/auth" << std::endl;
        std::cout << "Health: http://" << host << ":" << port << "/health" << std::endl;
        
        // Wait for shutdown signal
        std::signal(SIGINT, [](int) { std::exit(0); });
        std::signal(SIGTERM, [](int) { std::exit(0); });
        
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
private:
    void register_core_modules() {
        m_module_manager->register_module<modules::CurrencyModule>();
        m_module_manager->register_module<modules::RegionModule>();
        m_module_manager->register_module<modules::ProductModule>();
        m_module_manager->register_module<modules::CustomerModule>();
        m_module_manager->register_module<modules::CartModule>();
        m_module_manager->register_module<modules::OrderModule>();
        m_module_manager->register_module<modules::InventoryModule>();
        m_module_manager->register_module<modules::FulfillmentModule>();
        m_module_manager->register_module<modules::PaymentModule>();
        m_module_manager->register_module<modules::PromotionModule>();
        m_module_manager->register_module<modules::SalesChannelModule>();
        m_module_manager->register_module<modules::StockLocationModule>();
        m_module_manager->register_module<modules::SettingsModule>();
        m_module_manager->register_module<modules::NotificationModule>();
        m_module_manager->register_module<modules::FileModule>();
        m_module_manager->register_module<modules::WorkflowEngineModule>();
        m_module_manager->register_module<modules::EventBusModule>();
        m_module_manager->register_module<modules::CacheModule>();
        m_module_manager->register_module<modules::LockingModule>();
        m_module_manager->register_module<modules::SearchModule>();
        m_module_manager->register_module<modules::TaxModule>();
        m_module_manager->register_module<modules::PricingModule>();
        m_module_manager->register_module<modules::AuthModule>();
    }
};

} // namespace medusa_core

// Main entry point
int main(int argc, char* argv[]) {
    try {
        // Parse command line arguments
        std::string host = "0.0.0.0";
        uint16_t port = 9000;
        std::string db_host = "localhost";
        uint16_t db_port = 5432;
        std::string db_name = "medusa";
        std::string db_user = "postgres";
        std::string db_password = "postgres";
        std::string db_schema = "public";
        
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--host" && i + 1 < argc) host = argv[++i];
            else if (arg == "--port" && i + 1 < argc) port = static_cast<uint16_t>(std::stoi(argv[++i]));
            else if (arg == "--db-host" && i + 1 < argc) db_host = argv[++i];
            else if (arg == "--db-port" && i + 1 < argc) db_port = static_cast<uint16_t>(std::stoi(argv[++i]));
            else if (arg == "--db-name" && i + 1 < argc) db_name = argv[++i];
            else if (arg == "--db-user" && i + 1 < argc) db_user = argv[++i];
            else if (arg == "--db-password" && i + 1 < argc) db_password = argv[++i];
            else if (arg == "--db-schema" && i + 1 < argc) db_schema = argv[++i];
            else if (arg == "--help") {
                std::cout << "Medusa C++ Backend\n";
                std::cout << "Usage: medusa_server [options]\n";
                std::cout << "Options:\n";
                std::cout << "  --host HOST           Server host (default: 0.0.0.0)\n";
                std::cout << "  --port PORT           Server port (default: 9000)\n";
                std::cout << "  --db-host HOST        Database host (default: localhost)\n";
                std::cout << "  --db-port PORT        Database port (default: 5432)\n";
                std::cout << "  --db-name NAME        Database name (default: medusa)\n";
                std::cout << "  --db-user USER        Database user (default: postgres)\n";
                std::cout << "  --db-password PASS    Database password (default: postgres)\n";
                std::cout << "  --db-schema SCHEMA    Database schema (default: public)\n";
                std::cout << "  --help                Show this help\n";
                return 0;
            }
        }
        
        database::DatabaseConfig db_config;
        db_config.host = db_host;
        db_config.port = db_port;
        db_config.database = db_name;
        db_config.username = db_user;
        db_config.password = db_password;
        db_config.schema = db_schema;
        
        router::RouterConfig router_config;
        router_config.m_admin_cors_origins = {"http://localhost:7000", "http://localhost:3000"};
        router_config.m_store_cors_origins = {"http://localhost:8000", "http://localhost:3000"};
        router_config.m_auth_cors_origins = {"http://localhost:7000", "http://localhost:3000"};
        
        medusa_core::Application app(db_config, router_config);
        
        if (!app.initialize()) {
            std::cerr << "Failed to initialize application" << std::endl;
            return 1;
        }
        
        app.run(port, host);
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}