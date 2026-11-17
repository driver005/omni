export module medusa_core.api.store_products;

import database.connection;
import database.types;
import medusa_core.database.entities;
import router.router;
import http.types;

export namespace medusa_core::api::store {

void register_product_routes(router::Router& router, database::Database& db) {
    auto repo = db.repository<entities::Product>();
    
    // GET /store/products - List products (public, only published)
    router.add_route("/store/products", router::HttpMethod::GET,
        [repo = std::move(repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto products = repo.find_by({{"status", std::string("published")}});
                
                res.json({
                    {"products", products}
                });
            } catch (const std::exception& e) {
                res.status(500).json({{"error", e.what()}});
            }
        });
    
    // GET /store/products/:handle - Get product by handle
    router.add_route("/store/products/:handle", router::HttpMethod::GET,
        [repo = std::move(repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto handle = req.get_param("handle");
                auto products = repo.find_by({{"handle", handle}, {"status", std::string("published")}});
                
                if (products.empty()) {
                    res.status(404).json({{"error", "Product not found"}});
                    return;
                }
                
                const auto& product = products[0];
                res.json({
                    {"id", product.get_id()},
                    {"title", product.get_title()},
                    {"description", product.get_description()},
                    {"handle", product.get_handle()},
                    {"thumbnail", product.get_thumbnail()},
                });
            } catch (const std::exception& e) {
                res.status(500).json({{"error", e.what()}});
            }
        });
}

void register_cart_routes(router::Router& router, database::Database& db) {
    // Cart routes placeholder
    router.add_route("/store/carts", router::HttpMethod::POST,
        [](const http::Request& req, http::Response& res) {
            res.status(201).json({{"cart_id", generate_id()}});
        });
    
    router.add_route("/store/carts/:id", router::HttpMethod::GET,
        [](const http::Request& req, http::Response& res) {
            auto id = req.get_param("id");
            res.json({{"id", id}, {"items", std::vector<int>{}}});
        });
}

void register_customer_routes(router::Router& router, database::Database& db) {
    auto repo = db.repository<entities::Customer>();
    
    // GET /store/customers/me - Get authenticated customer
    router.add_route("/store/customers/me", router::HttpMethod::GET,
        [repo = std::move(repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer = repo.find(req.auth_context()->actor_id());
                
                if (!customer) {
                    res.status(404).json(http::ErrorResponse::not_found("Customer not found"));
                    return;
                }
                
                res.json({
                    {"id", customer->get_id()},
                    {"email", customer->get_email()},
                    {"first_name", customer->get_first_name()},
                    {"last_name", customer->get_last_name()},
                    {"has_account", customer->get_has_account()},
                });
            } catch (const std::exception& e) {
                res.status(500).json({{"error", e.what()}});
            }
        });
}

std::string generate_id() noexcept {
    return "00000000-0000-0000-0000-000000000000";
}

} // namespace medusa_core::api::store