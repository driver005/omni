export module medusa_core.api.store.categories;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::store {

void register_category_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto category_repo = db.repository<ProductCategory>();
    
    // GET /store/product-categories - List product categories
    router.add_route("/store/product-categories", router::HttpMethod::GET,
        [category_repo = std::move(category_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto categories = category_repo.find_by(filters);
                
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<ProductCategory> paginated;
                auto start = std::min(offset, categories.size());
                auto end = std::min(start + limit, categories.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(categories[i]));
                }
                
                res.json({
                    {"product_categories", paginated},
                    {"count", categories.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /store/product-categories/:id - Get category
    router.add_route("/store/product-categories/:id", router::HttpMethod::GET,
        [category_repo = std::move(category_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto category = category_repo.find(id);
                
                if (!category) {
                    res.status(404).json(http::ErrorResponse::not_found("Category not found"));
                    return;
                }
                
                res.json({
                    {"id", category->get_id()},
                    {"name", category->get_name()},
                    {"handle", category->get_handle()},
                    {"parent_id", category->get_parent_id()},
                    {"rank", category->get_rank()},
                    {"created_at", category->get_created_at()},
                    {"updated_at", category->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
}

} // namespace medusa_core::api::store