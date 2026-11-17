export module medusa_core.api.admin.product_variants;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_product_variant_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto variant_repo = db.repository<ProductVariant>();
    auto product_repo = db.repository<Product>();
    auto inventory_item_repo = db.repository<InventoryItem>();
    auto inventory_level_repo = db.repository<InventoryLevel>();
    auto stock_location_repo = db.repository<StockLocation>();
    
    // GET /admin/products/:product_id/variants - List variants for product
    router.add_route("/admin/products/:product_id/variants", router::HttpMethod::GET,
        [variant_repo = std::move(variant_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto product_id = req.get_param("product_id");
                
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                filters["product_id"] = product_id;
                
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto variants = variant_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<ProductVariant> paginated;
                auto start = std::min(offset, variants.size());
                auto end = std::min(start + limit, variants.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(variants[i]));
                }
                
                res.json({
                    {"variants", paginated},
                    {"count", variants.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/products/:product_id/variants/:id - Get variant
    router.add_route("/admin/products/:product_id/variants/:id", router::HttpMethod::GET,
        [variant_repo = std::move(variant_repo), inventory_item_repo = std::move(inventory_item_repo),
         inventory_level_repo = std::move(inventory_level_repo), stock_location_repo = std::move(stock_location_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto variant = variant_repo.find(id);
                
                if (!variant) {
                    res.status(404).json(http::ErrorResponse::not_found("Product variant not found"));
                    return;
                }
                
                auto inventory_item = inventory_item_repo.find(variant->get_inventory_item_id());
                auto inventory_levels = inventory_level_repo.find_by({{"inventory_item_id", variant->get_inventory_item_id()}});
                
                res.json({
                    {"id", variant->get_id()},
                    {"product_id", variant->get_product_id()},
                    {"title", variant->get_title()},
                    {"sku", variant->get_sku()},
                    {"barcode", variant->get_barcode()},
                    {"price", variant->get_price()},
                    {"inventory_quantity", variant->get_inventory_quantity()},
                    {"allow_backorder", variant->get_allow_backorder()},
                    {"manage_inventory", variant->get_manage_inventory()},
                    {"inventory_item", inventory_item},
                    {"inventory_levels", inventory_levels},
                    {"created_at", variant->get_created_at()},
                    {"updated_at", variant->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/products/:product_id/variants - Create variant
    router.add_route("/admin/products/:product_id/variants", router::HttpMethod::POST,
        [variant_repo = std::move(variant_repo), product_repo = std::move(product_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto product_id = req.get_param("product_id");
                auto product = product_repo.find(product_id);
                
                if (!product) {
                    res.status(404).json(http::ErrorResponse::not_found("Product not found"));
                    return;
                }
                
                ProductVariant variant;
                variant.add_id(generate_id());
                variant.add_product_id(product_id);
                variant.add_title(req.body_as<std::string>().value_or(""));
                variant.add_price(req.body_as<std::string>().value_or("0"));
                
                variant = variant_repo.save(std::move(variant));
                
                res.status(201).json({{"variant", variant}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/products/:product_id/variants/:id - Update variant
    router.add_route("/admin/products/:product_id/variants/:id", router::HttpMethod::POST,
        [variant_repo = std::move(variant_repo), inventory_item_repo = std::move(inventory_item_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto variant = variant_repo.find(id);
                
                if (!variant) {
                    res.status(404).json(http::ErrorResponse::not_found("Product variant not found"));
                    return;
                }
                
                if (auto title = req.body_as<std::string>()) {
                    variant->set_title(std::move(*title));
                }
                if (auto price = req.body_as<std::string>()) {
                    variant->set_price(std::move(*price));
                }
                if (auto inventory_quantity = req.body_as<int64_t>()) {
                    variant->set_inventory_quantity(*inventory_quantity);
                }
                if (auto allow_backorder = req.body_as<bool>()) {
                    variant->set_allow_backorder(*allow_backorder);
                }
                if (auto manage_inventory = req.body_as<bool>()) {
                    variant->set_manage_inventory(*manage_inventory);
                }
                
                variant = variant_repo.save(std::move(*variant));
                
                res.json({{"variant", *variant}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/products/:product_id/variants/:id - Delete variant
    router.add_route("/admin/products/:product_id/variants/:id", router::HttpMethod::DELETE,
        [variant_repo = std::move(variant_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto variant = variant_repo.find(id);
                
                if (!variant) {
                    res.status(404).json(http::ErrorResponse::not_found("Product variant not found"));
                    return;
                }
                
                variant_repo.remove(*variant);
                res.status(200).json({{"id", id}, {"object", "product_variant"}, {"deleted", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/products/:product_id/variants/:id/inventory - Update inventory
    router.add_route("/admin/products/:product_id/variants/:id/inventory", router::HttpMethod::POST,
        [variant_repo = std::move(variant_repo), inventory_item_repo = std::move(inventory_item_repo),
         inventory_level_repo = db.repository<InventoryLevel>(), stock_location_repo = db.repository<StockLocation>()]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto variant = variant_repo.find(id);
                
                if (!variant) {
                    res.status(404).json(http::ErrorResponse::not_found("Product variant not found"));
                    return;
                }
                
                int64_t inventory_quantity = req.body_as<int64_t>().value_or(0);
                variant->set_inventory_quantity(inventory_quantity);
                variant = variant_repo.save(std::move(*variant));
                
                res.json({{"variant", *variant}});
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