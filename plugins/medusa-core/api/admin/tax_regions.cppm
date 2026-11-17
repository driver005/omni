export module medusa_core.api.admin.tax_regions;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_tax_region_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto tax_region_repo = db.repository<TaxRegion>();
    auto tax_rate_repo = db.repository<TaxRate>();
    
    // GET /admin/tax-regions - List tax regions
    router.add_route("/admin/tax-regions", router::HttpMethod::GET,
        [tax_region_repo = std::move(tax_region_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto tax_regions = tax_region_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<TaxRegion> paginated;
                auto start = std::min(offset, tax_regions.size());
                auto end = std::min(start + limit, tax_regions.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(tax_regions[i]));
                }
                
                res.json({
                    {"tax_regions", paginated},
                    {"count", tax_regions.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/tax-regions/:id - Get tax region
    router.add_route("/admin/tax-regions/:id", router::HttpMethod::GET,
        [tax_region_repo = std::move(tax_region_repo), tax_rate_repo = std::move(tax_rate_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto tax_region = tax_region_repo.find(id);
                
                if (!tax_region) {
                    res.status(404).json(http::ErrorResponse::not_found("Tax region not found"));
                    return;
                }
                
                auto tax_rates = tax_rate_repo.find_by({{"tax_region_id", id}});
                
                res.json({
                    {"id", tax_region->get_id()},
                    {"country_code", tax_region->get_country_code()},
                    {"province_code", tax_region->get_province_code()},
                    {"tax_rates", tax_rates},
                    {"created_at", tax_region->get_created_at()},
                    {"updated_at", tax_region->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/tax-regions - Create tax region
    router.add_route("/admin/tax-regions", router::HttpMethod::POST,
        [tax_region_repo = std::move(tax_region_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                TaxRegion tax_region;
                tax_region.add_id(generate_id());
                tax_region.add_country_code(req.body_as<std::string>().value_or(""));
                
                tax_region = tax_region_repo.save(std::move(tax_region));
                
                res.status(201).json({{"tax_region", tax_region}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/tax-regions/:id - Update tax region
    router.add_route("/admin/tax-regions/:id", router::HttpMethod::POST,
        [tax_region_repo = std::move(tax_region_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto tax_region = tax_region_repo.find(id);
                
                if (!tax_region) {
                    res.status(404).json(http::ErrorResponse::not_found("Tax region not found"));
                    return;
                }
                
                if (auto country_code = req.body_as<std::string>()) {
                    tax_region->set_country_code(std::move(*country_code));
                }
                if (auto province_code = req.body_as<std::string>()) {
                    tax_region->set_province_code(std::move(*province_code));
                }
                
                tax_region = tax_region_repo.save(std::move(*tax_region));
                
                res.json({{"tax_region", *tax_region}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/tax-regions/:id - Delete tax region
    router.add_route("/admin/tax-regions/:id", router::HttpMethod::DELETE,
        [tax_region_repo = std::move(tax_region_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto tax_region = tax_region_repo.find(id);
                
                if (!tax_region) {
                    res.status(404).json(http::ErrorResponse::not_found("Tax region not found"));
                    return;
                }
                
                tax_region_repo.remove(*tax_region);
                res.status(200).json({{"id", id}, {"object", "tax_region"}, {"deleted", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/tax-regions/:id/tax-rates - Add tax rate
    router.add_route("/admin/tax-regions/:id/tax-rates", router::HttpMethod::POST,
        [tax_region_repo = std::move(tax_region_repo), tax_rate_repo = std::move(tax_rate_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto region_id = req.get_param("id");
                auto tax_region = tax_region_repo.find(region_id);
                
                if (!tax_region) {
                    res.status(404).json(http::ErrorResponse::not_found("Tax region not found"));
                    return;
                }
                
                TaxRate tax_rate;
                tax_rate.add_id(generate_id());
                tax_rate.add_rate(req.body_as<std::string>().value_or("0"));
                tax_rate.add_code(req.body_as<std::string>().value_or(""));
                tax_rate.add_name(req.body_as<std::string>().value_or(""));
                
                tax_rate = tax_rate_repo.save(std::move(tax_rate));
                
                res.status(201).json({{"tax_rate", tax_rate}});
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