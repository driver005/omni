export module medusa_core.api.admin.inventory;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_inventory_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto inventory_item_repo = db.repository<InventoryItem>();
    auto inventory_level_repo = db.repository<InventoryLevel>();
    auto reservation_repo = db.repository<Reservation>();
    auto stock_location_repo = db.repository<StockLocation>();
    auto variant_repo = db.repository<ProductVariant>();
    auto product_repo = db.repository<Product>();
    
    // GET /admin/inventory-items - List inventory items
    router.add_route("/admin/inventory-items", router::HttpMethod::GET,
        [inventory_item_repo = std::move(inventory_item_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto items = inventory_item_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<InventoryItem> paginated;
                auto start = std::min(offset, items.size());
                auto end = std::min(start + limit, items.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(items[i]));
                }
                
                res.json({
                    {"inventory_items", paginated},
                    {"count", items.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/inventory-items/:id - Get inventory item
    router.add_route("/admin/inventory-items/:id", router::HttpMethod::GET,
        [inventory_item_repo = std::move(inventory_item_repo), inventory_level_repo = std::move(inventory_level_repo),
         variant_repo = std::move(variant_repo), product_repo = std::move(product_repo),
         stock_location_repo = db.repository<StockLocation>()]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto item = inventory_item_repo.find(id);
                
                if (!item) {
                    res.status(404).json(http::ErrorResponse::not_found("Inventory item not found"));
                    return;
                }
                
                auto levels = inventory_level_repo.find_by({{"inventory_item_id", id}});
                auto variant = variant_repo.find(item->get_variant_id());
                auto product = product_repo.find(item->get_product_id());
                
                res.json({
                    {"id", item->get_id()},
                    {"sku", item->get_sku()},
                    {"requires_shipping", item->get_requires_shipping()},
                    {"levels", levels},
                    {"variant", variant},
                    {"product", product},
                    {"created_at", item->get_created_at()},
                    {"updated_at", item->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/inventory-items - Create inventory item
    router.add_route("/admin/inventory-items", router::HttpMethod::POST,
        [inventory_item_repo = std::move(inventory_item_repo), variant_repo = std::move(variant_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                std::string variant_id = req.body_as<std::string>().value_or("");
                
                auto variant = variant_repo.find(variant_id);
                if (!variant) {
                    res.status(404).json(http::ErrorResponse::not_found("Variant not found"));
                    return;
                }
                
                InventoryItem item;
                item.add_id(generate_id());
                item.add_variant_id(variant_id);
                item.add_sku(req.body_as<std::string>().value_or(""));
                item.add_requires_shipping(true);
                
                item = inventory_item_repo.save(std::move(item));
                
                res.status(201).json({{"inventory_item", item}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/inventory-items/:id/location-levels - Add location level
    router.add_route("/admin/inventory-items/:id/location-levels", router::HttpMethod::POST,
        [inventory_item_repo = std::move(inventory_item_repo), inventory_level_repo = std::move(inventory_level_repo),
         stock_location_repo = std::move(stock_location_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto item = inventory_item_repo.find(id);
                
                if (!item) {
                    res.status(404).json(http::ErrorResponse::not_found("Inventory item not found"));
                    return;
                }
                
                std::string location_id = req.body_as<std::string>().value_or("");
                int64_t stocked_quantity = req.body_as<int64_t>().value_or(0);
                
                auto location = stock_location_repo.find(location_id);
                if (!location) {
                    res.status(404).json(http::ErrorResponse::not_found("Stock location not found"));
                    return;
                }
                
                InventoryLevel level;
                level.add_id(generate_id());
                level.add_inventory_item_id(id);
                level.add_location_id(location_id);
                level.add_stocked_quantity(stocked_quantity);
                
                level = inventory_level_repo.save(std::move(level));
                
                res.status(201).json({{"inventory_level", level}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/inventory-items/:id/location-levels/batch - Batch update location levels
    router.add_route("/admin/inventory-items/:id/location-levels/batch", router::HttpMethod::POST,
        [inventory_item_repo = std::move(inventory_item_repo), inventory_level_repo = std::move(inventory_level_repo),
         stock_location_repo = std::move(stock_location_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                // Batch update location levels
                // In real implementation, parse array of location levels
                
                res.json({{"updated", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/inventory-items/:id/location-levels/:location_id - Get location level
    router.add_route("/admin/inventory-items/:id/location-levels/:location_id", router::HttpMethod::GET,
        [inventory_level_repo = std::move(inventory_level_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto item_id = req.get_param("id");
                auto location_id = req.get_param("location_id");
                
                auto levels = inventory_level_repo.find_by({{"inventory_item_id", item_id}, {"location_id", location_id}});
                
                if (levels.empty()) {
                    res.status(404).json(http::ErrorResponse::not_found("Inventory level not found"));
                    return;
                }
                
                res.json({{"inventory_level", levels[0]}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/inventory - Get inventory levels (with filters)
    router.add_route("/admin/inventory", router::HttpMethod::GET,
        [inventory_level_repo = std::move(inventory_level_repo), inventory_item_repo = std::move(inventory_item_repo),
         variant_repo = std::move(variant_repo), product_repo = std::move(product_repo),
         stock_location_repo = std::move(stock_location_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto levels = inventory_level_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<InventoryLevel> paginated;
                auto start = std::min(offset, levels.size());
                auto end = std::min(start + limit, levels.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(levels[i]));
                }
                
                res.json({
                    {"inventory_levels", paginated},
                    {"count", levels.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/reservations - List reservations
    router.add_route("/admin/reservations", router::HttpMethod::GET,
        [reservation_repo = std::move(reservation_repo), line_item_repo = db.repository<LineItem>(),
         inventory_item_repo = std::move(inventory_item_repo), stock_location_repo = db.repository<StockLocation>()]
        (const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto reservations = reservation_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<Reservation> paginated;
                auto start = std::min(offset, reservations.size());
                auto end = std::min(start + limit, reservations.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(reservations[i]));
                }
                
                res.json({
                    {"reservations", paginated},
                    {"count", reservations.size()},
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