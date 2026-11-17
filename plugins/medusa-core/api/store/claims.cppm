export module medusa_core.api.store.claims;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::store {

void register_claim_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto claim_repo = db.repository<Claim>();
    auto return_repo = db.repository<Return>();
    auto order_repo = db.repository<Order>();
    auto line_item_repo = db.repository<LineItem>();
    auto variant_repo = db.repository<ProductVariant>();
    auto product_repo = db.repository<Product>();
    auto shipping_option_repo = db.repository<ShippingOption>();
    auto fulfillment_set_repo = db.repository<FulfillmentSet>();
    auto stock_location_repo = db.repository<StockLocation>();
    auto shipping_profile_repo = db.repository<ShippingProfile>();
    auto fulfillment_provider_repo = db.repository<FulfillmentProvider>();
    
    // GET /store/claims - List customer's claims
    router.add_route("/store/claims", router::HttpMethod::GET,
        [claim_repo = std::move(claim_repo), order_repo = std::move(order_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                
                // Get customer's orders first
                auto orders = order_repo.find_by({{"customer_id", customer_id}});
                
                // Then get claims for those orders
                std::vector<Claim> claims;
                for (const auto& order : orders) {
                    auto order_claims = claim_repo.find_by({{"order_id", order.get_id()}});
                    claims.insert(claims.end(), 
                        std::make_move_iterator(order_claims.begin()),
                        std::make_move_iterator(order_claims.end()));
                }
                
                res.json({{"claims", claims}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /store/claims/:id - Get claim
    router.add_route("/store/claims/:id", router::HttpMethod::GET,
        [claim_repo = std::move(claim_repo), line_item_repo = std::move(line_item_repo),
         variant_repo = std::move(variant_repo), product_repo = std::move(product_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto claim = claim_repo.find(id);
                
                if (!claim) {
                    res.status(404).json(http::ErrorResponse::not_found("Claim not found"));
                    return;
                }
                
                // Verify ownership via order
                auto order = order_repo.find(claim->get_order_id());
                if (!order || order->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                // Fetch related entities
                auto items = line_item_repo.find_by({{"claim_id", id}});
                
                res.json({
                    {"id", claim->get_id()},
                    {"order_id", claim->get_order_id()},
                    {"type", claim->get_type()},
                    {"status", claim->get_status()},
                    {"items", items},
                    {"created_at", claim->get_created_at()},
                    {"updated_at", claim->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/claims - Create claim
    router.add_route("/store/claims", router::HttpMethod::POST,
        [claim_repo = std::move(claim_repo), order_repo = std::move(order_repo),
         line_item_repo = std::move(line_item_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                
                // Parse request body
                // In real implementation, parse JSON from req.body()
                std::string order_id = req.body_as<std::string>().value_or("");
                std::string type = req.body_as<std::string>().value_or("refund");
                
                auto order = order_repo.find(order_id);
                if (!order || order->get_customer_id() != customer_id) {
                    res.status(404).json(http::ErrorResponse::not_found("Order not found"));
                    return;
                }
                
                Claim claim(generate_id(), order_id);
                claim.set_type(std::move(type));
                claim.set_status(std::string("pending"));
                claim.set_created_at(std::chrono::system_clock::now());
                claim.set_updated_at(std::chrono::system_clock::now());
                
                claim = claim_repo.save(std::move(claim));
                
                res.status(201).json({{"claim", claim}});
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