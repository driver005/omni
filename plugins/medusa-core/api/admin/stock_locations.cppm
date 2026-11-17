export module medusa_core.api.admin.stock_locations;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_stock_location_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto location_repo = db.repository<StockLocation>();
    auto fulfillment_set_repo = db.repository<FulfillmentSet>();
    auto provider_repo = db.repository<FulfillmentProvider>();
    auto inventory_item_repo = db.repository<InventoryItem>();
    auto inventory_level_repo = db.repository<InventoryLevel>();
    auto sales_channel_repo = db.repository<SalesChannel>();
    auto address_repo = db.repository<Address>();
    
    // GET /admin/stock-locations - List stock locations
    router.add_route("/admin/stock-locations", router::HttpMethod::GET,
        [location_repo = std::move(location_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto locations = location_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<StockLocation> paginated;
                auto start = std::min(offset, locations.size());
                auto end = std::min(start + limit, locations.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(locations[i]));
                }
                
                res.json({
                    {"stock_locations", paginated},
                    {"count", locations.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/stock-locations/:id - Get stock location
    router.add_route("/admin/stock-locations/:id", router::HttpMethod::GET,
        [location_repo = std::move(location_repo), fulfillment_set_repo = std::move(fulfillment_set_repo),
         provider_repo = std::move(provider_repo), inventory_item_repo = std::move(inventory_item_repo),
         inventory_level_repo = std::move(inventory_level_repo), sales_channel_repo = std::move(sales_channel_repo),
         address_repo = std::move(address_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto location = location_repo.find(id);
                
                if (!location) {
                    res.status(404).json(http::ErrorResponse::not_found("Stock location not found"));
                    return;
                }
                
                auto fulfillment_sets = fulfillment_set_repo.find_by({{"location_id", id}});
                auto providers = provider_repo.find_by({{"location_id", id}});
                auto inventory_items = inventory_item_repo.find_by({{"location_id", id}});
                auto inventory_levels = inventory_level_repo.find_by({{"location_id", id}});
                auto sales_channels = sales_channel_repo.find_by({{"location_id", id}});
                
                res.json({
                    {"id", location->get_id()},
                    {"name", location->get_name()},
                    {"address_id", location->get_address_id()},
                    {"fulfillment_sets", fulfillment_sets},
                    {"providers", providers},
                    {"inventory_items", inventory_items},
                    {"inventory_levels", inventory_levels},
                    {"sales_channels", sales_channels},
                    {"created_at", location->get_created_at()},
                    {"updated_at", location->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/stock-locations - Create stock location
    router.add_route("/admin/stock-locations", router::HttpMethod::POST,
        [location_repo = std::move(location_repo), address_repo = std::move(address_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                StockLocation location;
                location.add_id(generate_id());
                location.add_name(req.body_as<std::string>().value_or(""));
                
                // Parse address if provided
                std::string address_id = req.body_as<std::string>().value_or("");
                if (!address_id.empty()) {
                    location.add_address_id(address_id);
                }
                
                location = location_repo.save(std::move(location));
                
                res.status(201).json({{"stock_location", location}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/stock-locations/:id - Update stock location
    router.add_route("/admin/stock-locations/:id", router::HttpMethod::POST,
        [location_repo = std::move(location_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto location = location_repo.find(id);
                
                if (!location) {
                    res.status(404).json(http::ErrorResponse::not_found("Stock location not found"));
                    return;
                }
                
                if (auto name = req.body_as<std::string>()) {
                    location->set_name(std::move(*name));
                }
                
                location = location_repo.save(std::move(*location));
                
                res.json({{"stock_location", *location}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/stock-locations/:id - Delete stock location
    router.add_route("/admin/stock-locations/:id", router::HttpMethod::DELETE,
        [location_repo = std::move(location_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto location = location_repo.find(id);
                
                if (!location) {
                    res.status(404).json(http::ErrorResponse::not_found("Stock location not found"));
                    return;
                }
                
                location_repo.remove(*location);
                res.status(200).json({{"id", id}, {"object", "stock_location"}, {"deleted", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/stock-locations/:id/fulfillment-sets - Add fulfillment set
    router.add_route("/admin/stock-locations/:id/fulfillment-sets", router::HttpMethod::POST,
        [location_repo = std::move(location_repo), fulfillment_set_repo = std::move(fulfillment_set_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto location_id = req.get_param("id");
                auto location = location_repo.find(location_id);
                
                if (!location) {
                    res.status(404).json(http::ErrorResponse::not_found("Stock location not found"));
                    return;
                }
                
                std::string name = req.body_as<std::string>().value_or("");
                std::string type = req.body_as<std::string>().value_or("");
                
                FulfillmentSet fs;
                fs.add_id(generate_id());
                fs.add_name(name);
                fs.add_type(type);
                
                fs = fulfillment_set_repo.save(std::move(fs));
                
                res.status(201).json({{"fulfillment_set", fs}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/stock-locations/:id/fulfillment-providers - Add fulfillment provider
    router.add_route("/admin/stock-locations/:id/fulfillment-providers", router::HttpMethod::POST,
        [location_repo = std::move(location_repo), provider_repo = std::move(provider_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto location_id = req.get_param("id");
                auto location = location_repo.find(location_id);
                
                if (!location) {
                    res.status(404).json(http::ErrorResponse::not_found("Stock location not found"));
                    return;
                }
                
                std::string provider_id = req.body_as<std::string>().value_or("");
                
                // In real implementation, validate and associate provider
                res.json({{"fulfillment_provider", provider_id}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/stock-locations/:id/sales-channels - Add sales channel
    router.add_route("/admin/stock-locations/:id/sales-channels", router::HttpMethod::POST,
        [location_repo = std::move(location_repo), sales_channel_repo = std::move(sales_channel_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto location_id = req.get_param("id");
                auto location = location_repo.find(location_id);
                
                if (!location) {
                    res.status(404).json(http::ErrorResponse::not_found("Stock location not found"));
                    return;
                }
                
                std::string channel_id = req.body_as<std::string>().value_or("");
                
                // In real implementation, validate and associate sales channel
                res.json({{"sales_channel", channel_id}});
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