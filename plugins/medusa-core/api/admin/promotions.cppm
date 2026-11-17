export module medusa_core.api.admin.promotions;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_promotion_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto promotion_repo = db.repository<Promotion>();
    auto discount_repo = db.repository<Discount>();
    auto campaign_repo = db.repository<Campaign>();
    
    // GET /admin/promotions - List promotions
    router.add_route("/admin/promotions", router::HttpMethod::GET,
        [promotion_repo = std::move(promotion_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto promotions = promotion_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<Promotion> paginated;
                auto start = std::min(offset, promotions.size());
                auto end = std::min(start + limit, promotions.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(promotions[i]));
                }
                
                res.json({
                    {"promotions", paginated},
                    {"count", promotions.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/promotions/:id - Get promotion
    router.add_route("/admin/promotions/:id", router::HttpMethod::GET,
        [promotion_repo = std::move(promotion_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto promotion = promotion_repo.find(id);
                
                if (!promotion) {
                    res.status(404).json(http::ErrorResponse::not_found("Promotion not found"));
                    return;
                }
                
                res.json({
                    {"id", promotion->get_id()},
                    {"code", promotion->get_code()},
                    {"type", promotion->get_type()},
                    {"value", promotion->get_value()},
                    {"status", promotion->get_status()},
                    {"created_at", promotion->get_created_at()},
                    {"updated_at", promotion->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/promotions - Create promotion
    router.add_route("/admin/promotions", router::HttpMethod::POST,
        [promotion_repo = std::move(promotion_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                Promotion promotion;
                promotion.add_id(generate_id());
                promotion.add_code(req.body_as<std::string>().value_or(""));
                promotion.add_type(req.body_as<std::string>().value_or("percentage"));
                promotion.add_value(req.body_as<std::string>().value_or("0"));
                promotion.add_status(std::string("active"));
                
                promotion = promotion_repo.save(std::move(promotion));
                
                res.status(201).json({{"promotion", promotion}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/promotions/:id - Update promotion
    router.add_route("/admin/promotions/:id", router::HttpMethod::POST,
        [promotion_repo = std::move(promotion_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto promotion = promotion_repo.find(id);
                
                if (!promotion) {
                    res.status(404).json(http::ErrorResponse::not_found("Promotion not found"));
                    return;
                }
                
                if (auto code = req.body_as<std::string>()) {
                    promotion->set_code(std::move(*code));
                }
                if (auto value = req.body_as<std::string>()) {
                    promotion->set_value(std::move(*value));
                }
                if (auto status = req.body_as<std::string>()) {
                    promotion->set_status(std::move(*status));
                }
                
                promotion = promotion_repo.save(std::move(*promotion));
                
                res.json({{"promotion", *promotion}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/promotions/:id - Delete promotion
    router.add_route("/admin/promotions/:id", router::HttpMethod::DELETE,
        [promotion_repo = std::move(promotion_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto promotion = promotion_repo.find(id);
                
                if (!promotion) {
                    res.status(404).json(http::ErrorResponse::not_found("Promotion not found"));
                    return;
                }
                
                promotion_repo.remove(*promotion);
                res.status(200).json({{"id", id}, {"object", "promotion"}, {"deleted", true}});
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