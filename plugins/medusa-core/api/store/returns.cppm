export module medusa_core.api.store.returns;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::store {

void register_return_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto return_repo = db.repository<Return>();
    auto return_item_repo = db.repository<ReturnItem>();
    auto order_repo = db.repository<Order>();
    auto line_item_repo = db.repository<LineItem>();
    auto variant_repo = db.repository<ProductVariant>();
    auto product_repo = db.repository<Product>();
    auto return_reason_repo = db.repository<ReturnReason>();
    auto shipping_option_repo = db.repository<ShippingOption>();
    
    // GET /store/returns - List customer's returns
    router.add_route("/store/returns", router::HttpMethod::GET,
        [return_repo = std::move(return_repo), order_repo = std::move(order_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                
                // Get customer's orders first
                auto orders = order_repo.find_by({{"customer_id", customer_id}});
                
                // Then get returns for those orders
                std::vector<Return> returns;
                for (const auto& order : orders) {
                    auto order_returns = return_repo.find_by({{"order_id", order.get_id()}});
                    returns.insert(returns.end(), 
                        std::make_move_iterator(order_returns.begin()),
                        std::make_move_iterator(order_returns.end()));
                }
                
                res.json({{"returns", returns}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /store/returns/:id - Get return
    router.add_route("/store/returns/:id", router::HttpMethod::GET,
        [return_repo = std::move(return_repo), return_item_repo = std::move(return_item_repo),
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
                
                auto ret = return_repo.find(id);
                
                if (!ret) {
                    res.status(404).json(http::ErrorResponse::not_found("Return not found"));
                    return;
                }
                
                // Verify ownership via order
                auto order = order_repo.find(ret->get_order_id());
                if (!order || order->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                // Fetch related entities
                auto items = return_item_repo.find_by({{"return_id", id}});
                
                res.json({
                    {"id", ret->get_id()},
                    {"order_id", ret->get_order_id()},
                    {"status", ret->get_status()},
                    {"reason", ret->get_reason()},
                    {"items", items},
                    {"created_at", ret->get_created_at()},
                    {"updated_at", ret->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/returns - Create return
    router.add_route("/store/returns", router::HttpMethod::POST,
        [return_repo = std::move(return_repo), return_item_repo = std::move(return_item_repo),
         order_repo = std::move(order_repo), line_item_repo = std::move(line_item_repo),
         return_reason_repo = std::move(return_reason_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                
                // Parse request body
                std::string order_id = req.body_as<std::string>().value_or("");
                std::string reason_id = req.body_as<std::string>().value_or("");
                std::string reason = req.body_as<std::string>().value_or("");
                
                auto order = order_repo.find(order_id);
                if (!order || order->get_customer_id() != customer_id) {
                    res.status(404).json(http::ErrorResponse::not_found("Order not found"));
                    return;
                }
                
                // Validate return reason
                auto return_reason = return_reason_repo.find(reason_id);
                if (!return_reason) {
                    res.status(400).json(http::ErrorResponse::invalid_data("Invalid return reason"));
                    return;
                }
                
                Return ret(generate_id(), order_id);
                ret.set_status(std::string("requested"));
                ret.set_reason(std::move(reason));
                ret.set_created_at(std::chrono::system_clock::now());
                ret.set_updated_at(std::chrono::system_clock::now());
                
                ret = return_repo.save(std::move(ret));
                
                res.status(201).json({{"return", ret}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/returns/:id/items - Add return item
    router.add_route("/store/returns/:id/items", router::HttpMethod::POST,
        [return_repo = std::move(return_repo), return_item_repo = std::move(return_item_repo),
         line_item_repo = std::move(line_item_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto ret = return_repo.find(id);
                if (!ret) {
                    res.status(404).json(http::ErrorResponse::not_found("Return not found"));
                    return;
                }
                
                // Verify ownership
                auto order = order_repo.find(ret->get_order_id());
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
                
                ReturnItem item(generate_id(), id, line_item_id);
                item.set_quantity(quantity);
                item.set_reason(req.body_as<std::string>().value_or(""));
                item.set_created_at(std::chrono::system_clock::now());
                item.set_updated_at(std::chrono::system_clock::now());
                
                item = return_item_repo.save(std::move(item));
                
                res.status(201).json({{"item", item}});
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