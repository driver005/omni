export module medusa_core.api.store.exchanges;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::store {

void register_exchange_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto exchange_repo = db.repository<Exchange>();
    auto exchange_item_repo = db.repository<ExchangeItem>();
    auto order_repo = db.repository<Order>();
    auto line_item_repo = db.repository<LineItem>();
    auto variant_repo = db.repository<ProductVariant>();
    auto product_repo = db.repository<Product>();
    auto return_repo = db.repository<Return>();
    auto shipping_option_repo = db.repository<ShippingOption>();
    auto fulfillment_set_repo = db.repository<FulfillmentSet>();
    auto stock_location_repo = db.repository<StockLocation>();
    auto shipping_profile_repo = db.repository<ShippingProfile>();
    auto fulfillment_provider_repo = db.repository<FulfillmentProvider>();
    
    // GET /store/exchanges - List customer's exchanges
    router.add_route("/store/exchanges", router::HttpMethod::GET,
        [exchange_repo = std::move(exchange_repo), order_repo = std::move(order_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                
                // Get customer's orders first
                auto orders = order_repo.find_by({{"customer_id", customer_id}});
                
                // Then get exchanges for those orders
                std::vector<Exchange> exchanges;
                for (const auto& order : orders) {
                    auto order_exchanges = exchange_repo.find_by({{"order_id", order.get_id()}});
                    exchanges.insert(exchanges.end(), 
                        std::make_move_iterator(order_exchanges.begin()),
                        std::make_move_iterator(order_exchanges.end()));
                }
                
                res.json({{"exchanges", exchanges}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /store/exchanges/:id - Get exchange
    router.add_route("/store/exchanges/:id", router::HttpMethod::GET,
        [exchange_repo = std::move(exchange_repo), exchange_item_repo = std::move(exchange_item_repo),
         line_item_repo = std::move(line_item_repo), variant_repo = std::move(variant_repo),
         product_repo = std::move(product_repo), order_repo = std::move(order_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto ex = exchange_repo.find(id);
                
                if (!ex) {
                    res.status(404).json(http::ErrorResponse::not_found("Exchange not found"));
                    return;
                }
                
                // Verify ownership via order
                auto order = order_repo.find(ex->get_order_id());
                if (!order || order->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                // Fetch related entities
                auto items = exchange_item_repo.find_by({{"exchange_id", id}});
                
                res.json({
                    {"id", ex->get_id()},
                    {"order_id", ex->get_order_id()},
                    {"status", ex->get_status()},
                    {"items", items},
                    {"created_at", ex->get_created_at()},
                    {"updated_at", ex->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/exchanges - Create exchange
    router.add_route("/store/exchanges", router::HttpMethod::POST,
        [exchange_repo = std::move(exchange_repo), exchange_item_repo = std::move(exchange_item_repo),
         order_repo = std::move(order_repo), line_item_repo = std::move(line_item_repo),
         return_repo = std::move(return_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                
                // Parse request body
                std::string order_id = req.body_as<std::string>().value_or("");
                std::string return_id = req.body_as<std::string>().value_or("");
                
                auto order = order_repo.find(order_id);
                if (!order || order->get_customer_id() != customer_id) {
                    res.status(404).json(http::ErrorResponse::not_found("Order not found"));
                    return;
                }
                
                // If return_id provided, validate it
                if (!return_id.empty()) {
                    auto ret = return_repo.find(return_id);
                    if (!ret || ret->get_order_id() != order_id) {
                        res.status(400).json(http::ErrorResponse::invalid_data("Invalid return"));
                        return;
                    }
                }
                
                Exchange ex(generate_id(), order_id);
                ex.set_status(std::string("pending"));
                ex.set_created_at(std::chrono::system_clock::now());
                ex.set_updated_at(std::chrono::system_clock::now());
                
                ex = exchange_repo.save(std::move(ex));
                
                res.status(201).json({{"exchange", ex}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/exchanges/:id/items - Add exchange item
    router.add_route("/store/exchanges/:id/items", router::HttpMethod::POST,
        [exchange_repo = std::move(exchange_repo), exchange_item_repo = std::move(exchange_item_repo),
         line_item_repo = std::move(line_item_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto ex = exchange_repo.find(id);
                if (!ex) {
                    res.status(404).json(http::ErrorResponse::not_found("Exchange not found"));
                    return;
                }
                
                // Verify ownership
                auto order = order_repo.find(ex->get_order_id());
                if (!order || order->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                // Parse request body
                std::string line_item_id = req.body_as<std::string>().value_or("");
                int64_t quantity = 1;
                
                auto line_item = line_item_repo.find(line_item_id);
                if (!line_item || line_item->get_order_id() != order->get_id()) {
                    res.status(404).json(http::ErrorResponse::not_found("Line item not found"));
                    return;
                }
                
                ExchangeItem item(generate_id(), id, line_item_id);
                item.set_quantity(quantity);
                item.set_created_at(std::chrono::system_clock::now());
                item.set_updated_at(std::chrono::system_clock::now());
                
                item = exchange_item_repo.save(std::move(item));
                
                res.status(201).json({{"item", item}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/exchanges/:id/confirm - Confirm exchange
    router.add_route("/store/exchanges/:id/confirm", router::HttpMethod::POST,
        [exchange_repo = std::move(exchange_repo), exchange_item_repo = std::move(exchange_item_repo),
         order_repo = std::move(order_repo), line_item_repo = std::move(line_item_repo),
         variant_repo = std::move(variant_repo), product_repo = std::move(product_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto ex = exchange_repo.find(id);
                if (!ex) {
                    res.status(404).json(http::ErrorResponse::not_found("Exchange not found"));
                    return;
                }
                
                // Verify ownership
                auto order = order_repo.find(ex->get_order_id());
                if (!order || order->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                if (ex->get_status() != "pending") {
                    res.status(400).json(http::ErrorResponse::invalid_data("Exchange already confirmed"));
                    return;
                }
                
                ex->set_status(std::string("confirmed"));
                ex->set_updated_at(std::chrono::system_clock::now());
                ex = exchange_repo.save(std::move(*ex));
                
                res.json({{"exchange", *ex}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/exchanges/:id/cancel - Cancel exchange
    router.add_route("/store/exchanges/:id/cancel", router::HttpMethod::POST,
        [exchange_repo = std::move(exchange_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto ex = exchange_repo.find(id);
                if (!ex) {
                    res.status(404).json(http::ErrorResponse::not_found("Exchange not found"));
                    return;
                }
                
                // Verify ownership
                auto order = order_repo.find(ex->get_order_id());
                if (!order || order->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                if (ex->get_status() != "pending") {
                    res.status(400).json(http::ErrorResponse::invalid_data("Exchange cannot be cancelled"));
                    return;
                }
                
                ex->set_status(std::string("cancelled"));
                ex->set_updated_at(std::chrono::system_clock::now());
                ex = exchange_repo.save(std::move(*ex));
                
                res.json({{"exchange", *ex}});
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