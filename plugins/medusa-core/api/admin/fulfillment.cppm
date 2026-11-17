export module medusa_core.api.admin.fulfillment;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_fulfillment_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto fulfillment_repo = db.repository<Fulfillment>();
    auto fulfillment_item_repo = db.repository<FulfillmentItem>();
    auto order_repo = db.repository<Order>();
    auto line_item_repo = db.repository<LineItem>();
    auto shipping_profile_repo = db.repository<ShippingProfile>();
    auto shipping_option_repo = db.repository<ShippingOption>();
    
    // GET /admin/fulfillments - List fulfillments
    router.add_route("/admin/fulfillments", router::HttpMethod::GET,
        [fulfillment_repo = std::move(fulfillment_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto fulfillments = fulfillment_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<Fulfillment> paginated;
                auto start = std::min(offset, fulfillments.size());
                auto end = std::min(start + limit, fulfillments.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(fulfillments[i]));
                }
                
                res.json({
                    {"fulfillments", paginated},
                    {"count", fulfillments.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/fulfillments/:id - Get fulfillment
    router.add_route("/admin/fulfillments/:id", router::HttpMethod::GET,
        [fulfillment_repo = std::move(fulfillment_repo), fulfillment_item_repo = std::move(fulfillment_item_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto fulfillment = fulfillment_repo.find(id);
                
                if (!fulfillment) {
                    res.status(404).json(http::ErrorResponse::not_found("Fulfillment not found"));
                    return;
                }
                
                auto items = fulfillment_item_repo.find_by({{"fulfillment_id", id}});
                
                res.json({
                    {"id", fulfillment->get_id()},
                    {"order_id", fulfillment->get_order_id()},
                    {"status", fulfillment->get_status()},
                    {"tracking_number", fulfillment->get_tracking_number()},
                    {"items", items},
                    {"created_at", fulfillment->get_created_at()},
                    {"updated_at", fulfillment->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/fulfillments - Create fulfillment
    router.add_route("/admin/fulfillments", router::HttpMethod::POST,
        [fulfillment_repo = std::move(fulfillment_repo), order_repo = std::move(order_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                std::string order_id = req.body_as<std::string>().value_or("");
                
                auto order = order_repo.find(order_id);
                if (!order) {
                    res.status(404).json(http::ErrorResponse::not_found("Order not found"));
                    return;
                }
                
                Fulfillment fulfillment;
                fulfillment.add_id(generate_id());
                fulfillment.add_order_id(order_id);
                fulfillment.add_status(std::string("pending"));
                
                fulfillment = fulfillment_repo.save(std::move(fulfillment));
                
                res.status(201).json({{"fulfillment", fulfillment}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/fulfillments/:id/shipment - Create shipment
    router.add_route("/admin/fulfillments/:id/shipment", router::HttpMethod::POST,
        [fulfillment_repo = std::move(fulfillment_repo), fulfillment_item_repo = std::move(fulfillment_item_repo),
         order_repo = std::move(order_repo), line_item_repo = std::move(line_item_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto fulfillment = fulfillment_repo.find(id);
                
                if (!fulfillment) {
                    res.status(404).json(http::ErrorResponse::not_found("Fulfillment not found"));
                    return;
                }
                
                // Parse items to ship
                // In real implementation, parse JSON array of items with quantities
                
                // Create fulfillment items
                FulfillmentItem item;
                item.add_id(generate_id());
                item.add_fulfillment_id(id);
                item.add_line_item_id(req.body_as<std::string>().value_or(""));
                item.add_quantity(req.body_as<int64_t>().value_or(1));
                
                item = fulfillment_item_repo.save(std::move(item));
                
                // Update fulfillment status
                fulfillment->set_status(std::string("shipped"));
                fulfillment->set_tracking_number(req.body_as<std::string>().value_or(""));
                fulfillment_repo.save(std::move(*fulfillment));
                
                res.json({{"fulfillment", *fulfillment}, {"item", item}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/fulfillments/:id/cancel - Cancel fulfillment
    router.add_route("/admin/fulfillments/:id/cancel", router::HttpMethod::POST,
        [fulfillment_repo = std::move(fulfillment_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto fulfillment = fulfillment_repo.find(id);
                
                if (!fulfillment) {
                    res.status(404).json(http::ErrorResponse::not_found("Fulfillment not found"));
                    return;
                }
                
                if (fulfillment->get_status() == "cancelled") {
                    res.status(400).json(http::ErrorResponse::invalid_data("Fulfillment already cancelled"));
                    return;
                }
                
                fulfillment->set_status(std::string("cancelled"));
                fulfillment = fulfillment_repo.save(std::move(*fulfillment));
                
                res.json({{"fulfillment", *fulfillment}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/fulfillment-providers - List fulfillment providers
    router.add_route("/admin/fulfillment-providers", router::HttpMethod::GET,
        [provider_repo = db.repository<FulfillmentProvider>()]
        (const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto providers = provider_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<FulfillmentProvider> paginated;
                auto start = std::min(offset, providers.size());
                auto end = std::min(start + limit, providers.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(providers[i]));
                }
                
                res.json({
                    {"fulfillment_providers", paginated},
                    {"count", providers.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
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

} // namespace medusa_core::api::admin