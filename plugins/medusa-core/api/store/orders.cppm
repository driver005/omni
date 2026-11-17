export module medusa_core.api.store.orders;

import database.connection;
import database.types;
import medusa_core.database.entities;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::store {

void register_order_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto order_repo = db.repository<Order>();
    auto line_item_repo = db.repository<LineItem>();
    auto customer_repo = db.repository<Customer>();
    auto return_repo = db.repository<Return>(); // Need to add Return entity
    
    // GET /store/orders - List customer's orders
    router.add_route("/store/orders", router::HttpMethod::GET,
        [order_repo = std::move(order_repo), customer_repo = std::move(customer_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                // Require authentication
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                filters["customer_id"] = customer_id;
                
                // Add additional filters from query
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto orders = order_repo.find_by(filters);
                
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<Order> paginated;
                auto start = std::min(offset, orders.size());
                auto end = std::min(start + limit, orders.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(orders[i]));
                }
                
                res.json({
                    {"orders", paginated},
                    {"count", orders.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /store/orders/:id - Get order
    router.add_route("/store/orders/:id", router::HttpMethod::GET,
        [order_repo = std::move(order_repo), line_item_repo = std::move(line_item_repo),
         customer_repo = std::move(customer_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto order = order_repo.find(id);
                
                if (!order) {
                    res.status(404).json(http::ErrorResponse::not_found("Order not found"));
                    return;
                }
                
                // Verify ownership
                if (order->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized to view this order"));
                    return;
                }
                
                auto line_items = line_item_repo.find_by({{"order_id", id}});
                auto customer = customer_repo.find(customer_id);
                
                res.json({
                    {"id", order->get_id()},
                    {"customer_id", order->get_customer_id()},
                    {"customer", customer ? *customer : std::monostate{}},
                    {"status", order->get_status()},
                    {"payment_status", order->get_payment_status()},
                    {"fulfillment_status", order->get_fulfillment_status()},
                    {"total", order->get_total()},
                    {"subtotal", order->get_subtotal()},
                    {"tax_total", order->get_tax_total()},
                    {"shipping_total", order->get_shipping_total()},
                    {"discount_total", order->get_discount_total()},
                    {"line_items", line_items},
                    {"created_at", order->get_created_at()},
                    {"updated_at", order->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/orders - Create order (from cart)
    router.add_route("/store/orders", router::HttpMethod::POST,
        [order_repo = std::move(order_repo), line_item_repo = std::move(line_item_repo),
         customer_repo = std::move(customer_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                
                entities::Order order;
                order.add_id(generate_id());
                order.add_customer_id(customer_id);
                order.add_region_id(req.body_as<std::string>().value_or(""));
                order.add_currency_code("usd");
                order.add_status("pending");
                order.add_payment_status("pending");
                order.add_fulfillment_status("not_fulfilled");
                order.add_total("0");
                order.add_subtotal("0");
                order.add_tax_total("0");
                order.add_shipping_total("0");
                order.add_discount_total("0");
                
                order = order_repo.save(std::move(order));
                
                res.status(201).json({{"order", order}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/orders/:id/cancel - Cancel order
    router.add_route("/store/orders/:id/cancel", router::HttpMethod::POST,
        [order_repo = std::move(order_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto order = order_repo.find(id);
                
                if (!order) {
                    res.status(404).json(http::ErrorResponse::not_found("Order not found"));
                    return;
                }
                
                if (order->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                if (order->get_status() == "cancelled") {
                    res.status(400).json(http::ErrorResponse::invalid_data("Order already cancelled"));
                    return;
                }
                
                order->set_status(std::string("cancelled"));
                order = order_repo.save(std::move(*order));
                
                res.json({{"order", order}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /store/orders/:id/returns - Get order returns
    router.add_route("/store/orders/:id/returns", router::HttpMethod::GET,
        [](const http::Request& req, http::Response& res) {
            if (!req.auth_context()) {
                res.status(401).json(http::ErrorResponse::unauthorized());
                return;
            }
            res.json({{"returns", std::vector<int>{}}});
        });
    
    // POST /store/orders/:id/returns - Create return
    router.add_route("/store/orders/:id/returns", router::HttpMethod::POST,
        [](const http::Request& req, http::Response& res) {
            if (!req.auth_context()) {
                res.status(401).json(http::ErrorResponse::unauthorized());
                return;
            }
            res.status(201).json({{"return", generate_id()}});
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