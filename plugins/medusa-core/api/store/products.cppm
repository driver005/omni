export module medusa_core.api.store.products;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::store {

void register_product_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto product_repo = db.repository<Product>();
    auto variant_repo = db.repository<ProductVariant>();
    auto image_repo = db.repository<Image>();
    auto option_repo = db.repository<Option>();
    auto tag_repo = db.repository<ProductTag>();
    auto type_repo = db.repository<ProductType>();
    auto collection_repo = db.repository<Collection>();
    auto category_repo = db.repository<ProductCategory>();
    auto shipping_profile_repo = db.repository<ShippingProfile>();
    auto price_repo = db.repository<Price>();
    auto price_list_repo = db.repository<PriceList>();
    
    // GET /store/products - List products (public, only published)
    router.add_route("/store/products", router::HttpMethod::GET,
        [product_repo = std::move(product_repo), variant_repo = std::move(variant_repo),
         image_repo = std::move(image_repo), option_repo = std::move(option_repo),
         tag_repo = std::move(tag_repo), type_repo = std::move(type_repo),
         collection_repo = std::move(collection_repo), category_repo = std::move(category_repo),
         price_repo = std::move(price_repo), price_list_repo = std::move(price_list_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                // Parse filters
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                filters["status"] = std::string("published");
                
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
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
    
    // GET /store/products/:handle - Get product by handle
    router.add_route("/store/products/:handle", router::HttpMethod::GET,
        [product_repo = std::move(product_repo), variant_repo = std::move(variant_repo),
         image_repo = std::move(image_repo), option_repo = std::move(option_repo),
         tag_repo = std::move(tag_repo), type_repo = std::move(type_repo),
         collection_repo = std::move(collection_repo), category_repo = std::move(category_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto handle = req.get_param("handle");
                auto products = product_repo.find_by({{"handle", handle}, {"status", std::string("published")}});
                
                if (products.empty()) {
                    res.status(404).json(http::ErrorResponse::not_found("Product not found"));
                    return;
                }
                
                const auto& product = products[0];
                
                // Fetch related entities
                auto variants = variant_repo.find_by({{"product_id", product.get_id()}});
                
                res.json({
                    {"id", product.get_id()},
                    {"title", product.get_title()},
                    {"description", product.get_description()},
                    {"handle", product.get_handle()},
                    {"thumbnail", product.get_thumbnail()},
                    {"variants", variants},
                    {"created_at", product.get_created_at()},
                    {"updated_at", product.get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /store/products/:id - Get product by ID
    router.add_route("/store/products/:id", router::HttpMethod::GET,
        [product_repo = std::move(product_repo), variant_repo = std::move(variant_repo),
         image_repo = std::move(image_repo), option_repo = std::move(option_repo),
         tag_repo = std::move(tag_repo), type_repo = std::move(type_repo),
         collection_repo = std::move(collection_repo), category_repo = std::move(category_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto products = product_repo.find_by({{"id", id}, {"status", std::string("published")}});
                
                if (products.empty()) {
                    res.status(404).json(http::ErrorResponse::not_found("Product not found"));
                    return;
                }
                
                const auto& product = products[0];
                
                // Fetch related entities
                auto variants = variant_repo.find_by({{"product_id", product.get_id()}});
                
                res.json({
                    {"id", product.get_id()},
                    {"title", product.get_title()},
                    {"description", product.get_description()},
                    {"handle", product.get_handle()},
                    {"thumbnail", product.get_thumbnail()},
                    {"variants", variants},
                    {"created_at", product.get_created_at()},
                    {"updated_at", product.get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
}

} // namespace medusa_core::api::store