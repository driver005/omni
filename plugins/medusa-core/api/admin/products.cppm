export module medusa_core.api.admin.products;

import database.connection;
import database.types;
import medusa_core.database.entities;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_product_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto product_repo = db.repository<Product>();
    auto variant_repo = db.repository<ProductVariant>();
    auto image_repo = db.repository<Image>(); // Need to add Image entity
    auto option_repo = db.repository<Option>();
    auto tag_repo = db.repository<ProductTag>();
    auto type_repo = db.repository<ProductType>();
    auto collection_repo = db.repository<Collection>();
    auto category_repo = db.repository<ProductCategory>();
    auto shipping_profile_repo = db.repository<ShippingProfile>();
    
    // GET /admin/products - List products with full filtering
    router.add_route("/admin/products", router::HttpMethod::GET,
        [product_repo = std::move(product_repo), variant_repo = std::move(variant_repo),
         image_repo = std::move(image_repo), option_repo = std::move(option_repo),
         tag_repo = std::move(tag_repo), type_repo = std::move(type_repo),
         collection_repo = std::move(collection_repo), category_repo = std::move(category_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                // Parse filters
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields" && key != "with_deleted") {
                        // Handle special filter syntax like status[]=published,proposed
                        if (key.ends_with("[]")) {
                            std::string filter_key = key.substr(0, key.size() - 2);
                            std::string values_str = values.front();
                            // For simplicity, just use first value
                            filters[filter_key] = values_str;
                        } else {
                            filters[key] = values.front();
                        }
                    }
                }
                
                // Handle deleted_at filter
                if (auto deleted_at = req.get_query("deleted_at")) {
                    filters["deleted_at"] = *deleted_at;
                }
                
                // Handle with_deleted
                bool with_deleted = false;
                if (auto wd = req.get_query("with_deleted")) {
                    with_deleted = *wd == "true";
                }
                if (!with_deleted) {
                    filters["deleted_at"] = ""; // Will be handled in query
                }
                
                auto products = product_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                // Apply ordering
                if (auto order = req.get_query("order")) {
                    // In real implementation, sort based on order parameter
                }
                
                // Apply limit/offset
                std::vector<Product> paginated;
                auto start = std::min(offset, products.size());
                auto end = std::min(start + limit, products.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(products[i]));
                }
                
                // Expand relations if requested
                if (auto fields = req.get_query("fields")) {
                    // In real implementation, expand requested fields
                }
                
                res.json({
                    {"products", paginated},
                    {"count", products.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/products/:id - Get product with full relations
    router.add_route("/admin/products/:id", router::HttpMethod::GET,
        [product_repo = std::move(product_repo), variant_repo = std::move(variant_repo),
         image_repo = std::move(image_repo), option_repo = std::move(option_repo),
         tag_repo = std::move(tag_repo), type_repo = std::move(type_repo),
         collection_repo = std::move(collection_repo), category_repo = std::move(category_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto product = product_repo.find(id);
                
                if (!product) {
                    res.status(404).json(http::ErrorResponse::not_found("Product not found"));
                    return;
                }
                
                // Fetch related entities
                auto variants = variant_repo.find_by({{"product_id", id}});
                // auto images = image_repo.find_by({{"product_id", id}});
                // auto options = option_repo.find_by({{"product_id", id}});
                // auto tags = tag_repo.find_by({{"product_id", id}});
                
                // Build response
                res.json({
                    {"id", product->get_id()},
                    {"title", product->get_title()},
                    {"subtitle", product->get_subtitle()},
                    {"description", product->get_description()},
                    {"handle", product->get_handle()},
                    {"is_giftcard", false},
                    {"status", product->get_status()},
                    {"thumbnail", product->get_thumbnail()},
                    {"weight", product->get_weight()},
                    {"length", 0},
                    {"height", 0},
                    {"width", 0},
                    {"hs_code", ""},
                    {"origin_country", ""},
                    {"mid_code", ""},
                    {"material", ""},
                    {"collection_id", ""},
                    {"type_id", ""},
                    {"discountable", true},
                    {"external_id", ""},
                    {"metadata", std::map<std::string, std::string>{}},
                    {"variants", variants},
                    {"images", std::vector<int>{}},
                    {"options", std::vector<int>{}},
                    {"tags", std::vector<int>{}},
                    {"type", std::monostate{}},
                    {"collection", std::monostate{}},
                    {"created_at", product->get_created_at()},
                    {"updated_at", product->get_updated_at()},
                    {"deleted_at", product->get_deleted_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/products - Create product
    router.add_route("/admin/products", router::HttpMethod::POST,
        [product_repo = std::move(product_repo), variant_repo = std::move(variant_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                // Parse request body
                // In real implementation, parse JSON from req.body()
                
                entities::Product product;
                product.add_id(generate_id());
                product.add_title(req.body_as<std::string>().value_or("New Product"));
                product.add_status("draft");
                
                product = product_repo.save(std::move(product));
                
                res.status(201).json({
                    {"product", product}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // PUT /admin/products/:id - Update product
    router.add_route("/admin/products/:id", router::HttpMethod::PUT,
        [product_repo = std::move(product_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto product = product_repo.find(id);
                
                if (!product) {
                    res.status(404).json(http::ErrorResponse::not_found("Product not found"));
                    return;
                }
                
                // Update fields from request body
                // In real implementation, parse JSON and update fields
                
                product = product_repo.save(std::move(*product));
                
                res.json({
                    {"product", product}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/products/:id - Delete product
    router.add_route("/admin/products/:id", router::HttpMethod::DELETE,
        [product_repo = std::move(product_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto product = product_repo.find(id);
                
                if (!product) {
                    res.status(404).json(http::ErrorResponse::not_found("Product not found"));
                    return;
                }
                
                product->set_deleted_at(std::chrono::system_clock::now());
                product = product_repo.save(std::move(*product));
                
                res.status(200).json({
                    {"id", id},
                    {"deleted", true}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/products/:id/variants - Get product variants
    router.add_route("/admin/products/:id/variants", router::HttpMethod::GET,
        [variant_repo = std::move(variant_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto product_id = req.get_param("id");
                auto variants = variant_repo.find_by({{"product_id", product_id}});
                
                // Apply filters
                if (auto sku = req.get_query("sku")) {
                    // Filter variants by SKU
                }
                if (auto manage_inventory = req.get_query("manage_inventory")) {
                    // Filter by manage_inventory
                }
                
                res.json({{"variants", variants}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/products/:id/variants - Create variant
    router.add_route("/admin/products/:id/variants", router::HttpMethod::POST,
        [product_repo = std::move(product_repo), variant_repo = std::move(variant_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto product_id = req.get_param("id");
                auto product = product_repo.find(product_id);
                
                if (!product) {
                    res.status(404).json(http::ErrorResponse::not_found("Product not found"));
                    return;
                }
                
                entities::ProductVariant variant;
                variant.add_id(generate_id());
                variant.add_product_id(product_id);
                variant.add_title(req.body_as<std::string>().value_or("New Variant"));
                variant.add_price("0");
                
                variant = variant_repo.save(std::move(variant));
                
                res.status(201).json({{"variant", variant}});
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