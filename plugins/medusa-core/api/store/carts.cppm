export module medusa_core.api.store.carts;

import database.connection;
import database.types;
import medusa_core.database.entities;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::store {

struct Cart {
    std::string m_id{};
    std::string m_customer_id{};
    std::string m_region_id{};
    std::string m_currency_code{"usd"};
    std::string m_status{"open"};
    std::string m_total{"0"};
    std::string m_subtotal{"0"};
    std::string m_tax_total{"0"};
    std::string m_shipping_total{"0"};
    std::string m_discount_total{"0"};
    std::vector<LineItem> m_items{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Cart() noexcept = default;
    Cart(std::string&& id, std::string&& customer_id, std::string&& region_id) noexcept
        : m_id(std::move(id)), m_customer_id(std::move(customer_id)), m_region_id(std::move(region_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_customer_id() noexcept { return m_customer_id; }
    const std::string& get_customer_id() const noexcept { return m_customer_id; }
    std::string& get_region_id() noexcept { return m_region_id; }
    const std::string& get_region_id() const noexcept { return m_region_id; }
    std::string& get_currency_code() noexcept { return m_currency_code; }
    const std::string& get_currency_code() const noexcept { return m_currency_code; }
    std::string& get_status() noexcept { return m_status; }
    const std::string& get_status() const noexcept { return m_status; }
    std::string& get_total() noexcept { return m_total; }
    const std::string& get_total() const noexcept { return m_total; }
    std::string& get_subtotal() noexcept { return m_subtotal; }
    const std::string& get_subtotal() const noexcept { return m_subtotal; }
    std::string& get_tax_total() noexcept { return m_tax_total; }
    const std::string& get_tax_total() const noexcept { return m_tax_total; }
    std::string& get_shipping_total() noexcept { return m_shipping_total; }
    const std::string& get_shipping_total() const noexcept { return m_shipping_total; }
    std::string& get_discount_total() noexcept { return m_discount_total; }
    const std::string& get_discount_total() const noexcept { return m_discount_total; }
    std::vector<LineItem>& get_items() noexcept { return m_items; }
    const std::vector<LineItem>& get_items() const noexcept { return m_items; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_total(std::string&& total) noexcept { m_total = std::move(total); }
};

struct CartRepository {
    database::Database& m_db;
    std::map<std::string, Cart> m_carts{};
    
    explicit CartRepository(database::Database& db) noexcept : m_db(db) {}
    
    std::optional<Cart> find(const std::string& id) noexcept {
        auto it = m_carts.find(id);
        if (it != m_carts.end()) return it->second;
        return std::nullopt;
    }
    
    std::optional<Cart> find_by_customer(const std::string& customer_id) noexcept {
        for (const auto& [id, cart] : m_carts) {
            if (cart.get_customer_id() == customer_id && cart.get_status() == "open") {
                return cart;
            }
        }
        return std::nullopt;
    }
    
    Cart save(Cart cart) noexcept {
        m_carts[cart.get_id()] = cart;
        return cart;
    }
    
    void remove(const std::string& id) noexcept {
        m_carts.erase(id);
    }
};

void register_cart_routes(router::Router& router, database::Database& db) {
    CartRepository cart_repo(db);
    auto line_item_repo = db.repository<LineItem>();
    auto variant_repo = db.repository<ProductVariant>();
    auto product_repo = db.repository<Product>();
    auto region_repo = db.repository<Region>();
    
    // GET /store/carts - Get current customer's cart
    router.add_route("/store/carts", router::HttpMethod::GET,
        [cart_repo = std::move(cart_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto cart = cart_repo.find_by_customer(customer_id);
                
                if (!cart) {
                    // Create new cart
                    entities::Region region;
                    // Get default region
                    
                    Cart new_cart(generate_id(), customer_id, "reg_default");
                    new_cart = cart_repo.save(std::move(new_cart));
                    cart = new_cart;
                }
                
                res.json({{"cart", *cart}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /store/carts/:id - Get cart by ID
    router.add_route("/store/carts/:id", router::HttpMethod::GET,
        [cart_repo = std::move(cart_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto cart = cart_repo.find(id);
                
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                // Verify ownership or allow if admin
                if (cart->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                res.json({{"cart", *cart}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/carts - Create cart
    router.add_route("/store/carts", router::HttpMethod::POST,
        [cart_repo = std::move(cart_repo), region_repo = std::move(region_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                std::string customer_id;
                if (req.auth_context()) {
                    customer_id = req.auth_context()->actor_id();
                }
                
                std::string region_id = "reg_default"; // Default region
                
                Cart cart(generate_id(), std::move(customer_id), std::move(region_id));
                cart = cart_repo.save(std::move(cart));
                
                res.status(201).json({{"cart", cart}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/carts/:id/line-items - Add line item to cart
    router.add_route("/store/carts/:id/line-items", router::HttpMethod::POST,
        [cart_repo = std::move(cart_repo), line_item_repo = std::move(line_item_repo),
         variant_repo = std::move(variant_repo), product_repo = std::move(product_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto cart = cart_repo.find(id);
                
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                if (!cart->get_customer_id().empty() && cart->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                // Parse variant_id and quantity from request body
                std::string variant_id = req.body_as<std::string>().value_or("");
                int64_t quantity = 1;
                
                auto variant = variant_repo.find(variant_id);
                if (!variant) {
                    res.status(404).json(http::ErrorResponse::not_found("Variant not found"));
                    return;
                }
                
                auto product = product_repo.find(variant->get_product_id());
                
                entities::LineItem line_item;
                line_item.add_id(generate_id());
                line_item.add_cart_id(id);
                line_item.add_variant_id(variant_id);
                line_item.add_title(variant->get_title());
                line_item.add_quantity(quantity);
                line_item.add_unit_price(variant->get_price());
                line_item.add_total(std::to_string(std::stoll(variant->get_price()) * quantity));
                
                cart->get_items().push_back(line_item);
                cart = cart_repo.save(std::move(*cart));
                
                res.status(201).json({{"cart", *cart}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // PUT /store/carts/:id/line-items/:item_id - Update line item
    router.add_route("/store/carts/:id/line-items/:item_id", router::HttpMethod::PUT,
        [cart_repo = std::move(cart_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                auto item_id = req.get_param("item_id");
                
                auto cart = cart_repo.find(id);
                
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                if (!cart->get_customer_id().empty() && cart->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                // Update line item quantity
                for (auto& item : cart->get_items()) {
                    if (item.get_id() == item_id) {
                        int64_t quantity = req.body_as<int64_t>().value_or(1);
                        item.set_quantity(quantity);
                        item.set_total(std::to_string(std::stoll(item.get_unit_price()) * quantity));
                        break;
                    }
                }
                
                cart = cart_repo.save(std::move(*cart));
                res.json({{"cart", *cart}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /store/carts/:id/line-items/:item_id - Remove line item
    router.add_route("/store/carts/:id/line-items/:item_id", router::HttpMethod::DELETE,
        [cart_repo = std::move(cart_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                auto item_id = req.get_param("item_id");
                
                auto cart = cart_repo.find(id);
                
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                if (!cart->get_customer_id().empty() && cart->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                cart->get_items().erase(
                    std::remove_if(cart->get_items().begin(), cart->get_items().end(),
                        [&item_id](const LineItem& item) { return item.get_id() == item_id; }),
                    cart->get_items().end()
                );
                
                cart = cart_repo.save(std::move(*cart));
                res.status(200).json({{"cart", *cart}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/carts/:id/complete - Complete cart (convert to order)
    router.add_route("/store/carts/:id/complete", router::HttpMethod::POST,
        [cart_repo = std::move(cart_repo), line_item_repo = std::move(line_item_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto cart = cart_repo.find(id);
                
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                if (cart->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                // Convert cart to order
                // In real implementation, run completeCartWorkflow
                
                cart->set_status(std::string("completed"));
                cart = cart_repo.save(std::move(*cart));
                
                res.json({{"cart", *cart}, {"order_id", generate_id()}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /store/carts/:id - Delete cart
    router.add_route("/store/carts/:id", router::HttpMethod::DELETE,
        [cart_repo = std::move(cart_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto cart = cart_repo.find(id);
                
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                if (cart->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                cart_repo.remove(id);
                res.status(200).json({{"id", id}, {"deleted", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
}

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

} // namespace medusa_core::api::store