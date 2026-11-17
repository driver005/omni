export module medusa_core.api.admin_products;

import database.connection;
import database.types;
import medusa_core.database.entities;
import router.router;
import http.types;

export namespace medusa_core::api::admin {

void register_product_routes(router::Router& router, database::Database& db) {
    auto repo = db.repository<entities::Product>();
    
    // GET /admin/products - List products
    router.add_route("/admin/products", router::HttpMethod::GET, 
        [repo = std::move(repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto products = repo.find_all();
                
                // Build JSON response
                // Note: Would use nlohmann::json in real implementation
                res.status(200);
                res.json({
                    {"products", products}
                });
            } catch (const std::exception& e) {
                res.status(500).json({{"error", e.what()}});
            }
        });
    
    // GET /admin/products/:id - Get product
    router.add_route("/admin/products/:id", router::HttpMethod::GET,
        [repo = std::move(repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto product = repo.find(id);
                
                if (!product) {
                    res.status(404).json({{"error", "Product not found"}});
                    return;
                }
                
                res.json({
                    {"id", product->get_id()},
                    {"title", product->get_title()},
                    {"description", product->get_description()},
                    {"handle", product->get_handle()},
                    {"status", product->get_status()},
                    {"weight", product->get_weight()},
                    {"thumbnail", product->get_thumbnail()},
                    {"created_at", product->get_created_at()},
                    {"updated_at", product->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json({{"error", e.what()}});
            }
        });
    
    // POST /admin/products - Create product
    router.add_route("/admin/products", router::HttpMethod::POST,
        [repo = std::move(repo)](const http::Request& req, http::Response& res) mutable {
            try {
                // Parse request body
                // In real implementation, would parse JSON from req.body()
                
                entities::Product product;
                product.add_id(generate_id());
                product.add_title("New Product");
                product.add_status("draft");
                
                product = repo.save(std::move(product));
                
                res.status(201).json({
                    {"id", product.get_id()},
                    {"title", product.get_title()},
                    {"handle", product.get_handle()},
                    {"status", product.get_status()},
                });
            } catch (const std::exception& e) {
                res.status(500).json({{"error", e.what()}});
            }
        });
    
    // PUT /admin/products/:id - Update product
    router.add_route("/admin/products/:id", router::HttpMethod::PUT,
        [repo = std::move(repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto product = repo.find(id);
                
                if (!product) {
                    res.status(404).json({{"error", "Product not found"}});
                    return;
                }
                
                // Update fields from request body
                // product->set_title(...);
                
                product = repo.save(std::move(*product));
                
                res.json({
                    {"id", product.get_id()},
                    {"title", product.get_title()},
                    {"handle", product.get_handle()},
                    {"status", product.get_status()},
                });
            } catch (const std::exception& e) {
                res.status(500).json({{"error", e.what()}});
            }
        });
    
    // DELETE /admin/products/:id - Delete product
    router.add_route("/admin/products/:id", router::HttpMethod::DELETE,
        [repo = std::move(repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto product = repo.find(id);
                
                if (!product) {
                    res.status(404).json({{"error", "Product not found"}});
                    return;
                }
                
                repo.remove(*product);
                res.status(200).json({{"id", id}, {"deleted", true}});
            } catch (const std::exception& e) {
                res.status(500).json({{"error", e.what()}});
            }
        });
}

void register_order_routes(router::Router& router, database::Database& db) {
    auto repo = db.repository<entities::Order>();
    
    router.add_route("/admin/orders", router::HttpMethod::GET,
        [repo = std::move(repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto orders = repo.find_all();
                res.json({{"orders", orders}});
            } catch (const std::exception& e) {
                res.status(500).json({{"error", e.what()}});
            }
        });
    
    router.add_route("/admin/orders/:id", router::HttpMethod::GET,
        [repo = std::move(repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto order = repo.find(id);
                
                if (!order) {
                    res.status(404).json({{"error", "Order not found"}});
                    return;
                }
                
                res.json({
                    {"id", order->get_id()},
                    {"customer_id", order->get_customer_id()},
                    {"status", order->get_status()},
                    {"total", order->get_total()},
                    {"created_at", order->get_created_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json({{"error", e.what()}});
            }
        });
}

void register_customer_routes(router::Router& router, database::Database& db) {
    auto repo = db.repository<entities::Customer>();
    
    router.add_route("/admin/customers", router::HttpMethod::GET,
        [repo = std::move(repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto customers = repo.find_all();
                res.json({{"customers", customers}});
            } catch (const std::exception& e) {
                res.status(500).json({{"error", e.what()}});
            }
        });
}

std::string generate_id() noexcept {
    // Simple UUID generation placeholder
    return "00000000-0000-0000-0000-000000000000";
}

} // namespace medusa_core::api::admin