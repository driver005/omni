export module medusa_core.modules.module;

import std;
import database.connection;
import database.types;

export namespace medusa_core::modules {

class Module {
public:
    virtual ~Module() = default;
    virtual std::string name() const = 0;
    virtual std::string version() const = 0;
    virtual std::vector<std::string> dependencies() const { return {}; }
    virtual bool initialize(database::Database& db) = 0;
    virtual void shutdown() = 0;
};

class ModuleManager {
    std::map<std::string, std::unique_ptr<Module>> m_modules{};
    std::vector<std::string> m_load_order{};
    database::Database* m_db{nullptr};
    
public:
    explicit ModuleManager(database::Database& db) noexcept : m_db(&db) {}
    
    template<typename ModuleType, typename... Args>
    ModuleType* register_module(Args&&... args) {
        static_assert(std::is_base_of_v<Module, ModuleType>, "ModuleType must inherit from Module");
        
        auto module = std::make_unique<ModuleType>(std::forward<Args>(args)...);
        auto name = module->name();
        
        // Check dependencies
        for (const auto& dep : module->dependencies()) {
            if (m_modules.find(dep) == m_modules.end()) {
                throw std::runtime_error("Missing dependency: " + dep);
            }
        }
        
        m_modules[name] = std::move(module);
        m_load_order.push_back(name);
        return static_cast<ModuleType*>(m_modules[name].get());
    }
    
    template<typename ModuleType>
    ModuleType* resolve(const std::string& name) {
        auto it = m_modules.find(name);
        if (it != m_modules.end()) {
            return dynamic_cast<ModuleType*>(it->second.get());
        }
        return nullptr;
    }
    
    bool initialize_all() {
        for (const auto& name : m_load_order) {
            auto* module = m_modules[name].get();
            if (!module->initialize(*m_db)) {
                return false;
            }
        }
        return true;
    }
    
    void shutdown_all() {
        // Shutdown in reverse order
        for (auto it = m_load_order.rbegin(); it != m_load_order.rend(); ++it) {
            m_modules[*it]->shutdown();
        }
    }
    
    const std::vector<std::string>& load_order() const noexcept { return m_load_order; }
    size_t module_count() const noexcept { return m_modules.size(); }
};

// Core module implementations
class ProductModule : public Module {
public:
    std::string name() const override { return "product"; }
    std::string version() const override { return "1.0.0"; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::Product>();
        db.create_table<medusa_core::entities::ProductVariant>();
        db.create_table<medusa_core::entities::ProductImage>();
        db.create_table<medusa_core::entities::ProductOption>();
        db.create_table<medusa_core::entities::ProductTag>();
        db.create_table<medusa_core::entities::ProductType>();
        db.create_table<medusa_core::entities::Collection>();
        db.create_table<medusa_core::entities::ProductCategory>();
        return true;
    }
    
    void shutdown() override {}
};

class OrderModule : public Module {
public:
    std::string name() const override { return "order"; }
    std::string version() const override { return "1.0.0"; }
    std::vector<std::string> dependencies() const override { return {"product", "customer", "region"}; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::Order>();
        db.create_table<medusa_core::entities::LineItem>();
        db.create_table<medusa_core::entities::Return>();
        db.create_table<medusa_core::entities::Claim>();
        db.create_table<medusa_core::entities::Exchange>();
        return true;
    }
    
    void shutdown() override {}
};

class CustomerModule : public Module {
public:
    std::string name() const override { return "customer"; }
    std::string version() const override { return "1.0.0"; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::Customer>();
        db.create_table<medusa_core::entities::Address>();
        db.create_table<medusa_core::entities::CustomerGroup>();
        return true;
    }
    
    void shutdown() override {}
};

class CartModule : public Module {
public:
    std::string name() const override { return "cart"; }
    std::string version() const override { return "1.0.0"; }
    std::vector<std::string> dependencies() const override { return {"product", "customer", "region"}; }
    
    bool initialize(database::Database& db) override {
        // Cart uses in-memory storage for simplicity
        return true;
    }
    
    void shutdown() override {}
};

class RegionModule : public Module {
public:
    std::string name() const override { return "region"; }
    std::string version() const override { return "1.0.0"; }
    std::vector<std::string> dependencies() const override { return {"currency"}; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::Region>();
        db.create_table<medusa_core::entities::Country>();
        db.create_table<medusa_core::entities::Province>();
        return true;
    }
    
    void shutdown() override {}
};

class CurrencyModule : public Module {
public:
    std::string name() const override { return "currency"; }
    std::string version() const override { return "1.0.0"; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::Currency>();
        return true;
    }
    
    void shutdown() override {}
};

class AuthModule : public Module {
public:
    std::string name() const override { return "auth"; }
    std::string version() const override { return "1.0.0"; }
    std::vector<std::string> dependencies() const override { return {"user", "api_key"}; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::User>();
        db.create_table<medusa_core::entities::ApiKey>();
        db.create_table<medusa_core::entities::PublishableApiKey>();
        db.create_table<medusa_core::entities::SalesChannel>();
        return true;
    }
    
    void shutdown() override {}
};

class InventoryModule : public Module {
public:
    std::string name() const override { return "inventory"; }
    std::string version() const override { return "1.0.0"; }
    std::vector<std::string> dependencies() const override { return {"product", "stock_location"}; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::InventoryItem>();
        db.create_table<medusa_core::entities::InventoryLevel>();
        db.create_table<medusa_core::entities::Reservation>();
        db.create_table<medusa_core::entities::StockLocation>();
        return true;
    }
    
    void shutdown() override {}
};

class FulfillmentModule : public Module {
public:
    std::string name() const override { return "fulfillment"; }
    std::string version() const override { return "1.0.0"; }
    std::vector<std::string> dependencies() const override { return {"order", "inventory"}; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::Fulfillment>();
        db.create_table<medusa_core::entities::FulfillmentItem>();
        db.create_table<medusa_core::entities::ShippingProfile>();
        db.create_table<medusa_core::entities::ShippingOption>();
        return true;
    }
    
    void shutdown() override {}
};

class PaymentModule : public Module {
public:
    std::string name() const override { return "payment"; }
    std::string version() const override { return "1.0.0"; }
    std::vector<std::string> dependencies() const override { return {"order", "customer"}; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::PaymentCollection>();
        db.create_table<medusa_core::entities::PaymentSession>();
        db.create_table<medusa_core::entities::Payment>();
        db.create_table<medusa_core::entities::Refund>();
        return true;
    }
    
    void shutdown() override {}
};

class PromotionModule : public Module {
public:
    std::string name() const override { return "promotion"; }
    std::string version() const override { return "1.0.0"; }
    std::vector<std::string> dependencies() const override { return {"product", "customer"}; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::Promotion>();
        db.create_table<medusa_core::entities::Discount>();
        db.create_table<medusa_core::entities::Campaign>();
        return true;
    }
    
    void shutdown() override {}
};

class SalesChannelModule : public Module {
public:
    std::string name() const override { return "sales_channel"; }
    std::string version() const override { return "1.0.0"; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::SalesChannel>();
        return true;
    }
    
    void shutdown() override {}
};

class StockLocationModule : public Module {
public:
    std::string name() const override { return "stock_location"; }
    std::string version() const override { return "1.0.0"; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::StockLocation>();
        return true;
    }
    
    void shutdown() override {}
};

class SettingsModule : public Module {
public:
    std::string name() const override { return "settings"; }
    std::string version() const override { return "1.0.0"; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::Setting>();
        return true;
    }
    
    void shutdown() override {}
};

class NotificationModule : public Module {
public:
    std::string name() const override { return "notification"; }
    std::string version() const override { return "1.0.0"; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::Notification>();
        return true;
    }
    
    void shutdown() override {}
};

class FileModule : public Module {
public:
    std::string name() const override { return "file"; }
    std::string version() const override { return "1.0.0"; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::File>();
        return true;
    }
    
    void shutdown() override {}
};

class WorkflowEngineModule : public Module {
public:
    std::string name() const override { return "workflow_engine"; }
    std::string version() const override { return "1.0.0"; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::WorkflowExecution>();
        return true;
    }
    
    void shutdown() override {}
};

class EventBusModule : public Module {
public:
    std::string name() const override { return "event_bus"; }
    std::string version() const override { return "1.0.0"; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::Event>();
        return true;
    }
    
    void shutdown() override {}
};

class CacheModule : public Module {
public:
    std::string name() const override { return "cache"; }
    std::string version() const override { return "1.0.0"; }
    
    bool initialize(database::Database& db) override {
        // In-memory cache
        return true;
    }
    
    void shutdown() override {}
};

class LockingModule : public Module {
public:
    std::string name() const override { return "locking"; }
    std::string version() const override { return "1.0.0"; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::Lock>();
        return true;
    }
    
    void shutdown() override {}
};

class SearchModule : public Module {
public:
    std::string name() const override { return "search"; }
    std::string version() const override { return "1.0.0"; }
    std::vector<std::string> dependencies() const override { return {"product"}; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::SearchIndex>();
        return true;
    }
    
    void shutdown() override {}
};

class TaxModule : public Module {
public:
    std::string name() const override { return "tax"; }
    std::string version() const override { return "1.0.0"; }
    std::vector<std::string> dependencies() const override { return {"region"}; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::TaxRate>();
        db.create_table<medusa_core::entities::TaxRegion>();
        return true;
    }
    
    void shutdown() override {}
};

class PricingModule : public Module {
public:
    std::string name() const override { return "pricing"; }
    std::string version() const override { return "1.0.0"; }
    std::vector<std::string> dependencies() const override { return {"product", "region", "currency"}; }
    
    bool initialize(database::Database& db) override {
        db.create_table<medusa_core::entities::PriceList>();
        db.create_table<medusa_core::entities::Price>();
        return true;
    }
    
    void shutdown() override {}
};

} // namespace medusa_core::modules