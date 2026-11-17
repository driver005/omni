export module medusa_core.api.admin.regions;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_region_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto region_repo = db.repository<Region>();
    auto country_repo = db.repository<Country>();
    auto province_repo = db.repository<Province>();
    auto currency_repo = db.repository<Currency>();
    auto tax_rate_repo = db.repository<TaxRate>();
    auto tax_region_repo = db.repository<TaxRegion>();
    
    // GET /admin/regions - List regions
    router.add_route("/admin/regions", router::HttpMethod::GET,
        [region_repo = std::move(region_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto regions = region_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<Region> paginated;
                auto start = std::min(offset, regions.size());
                auto end = std::min(start + limit, regions.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(regions[i]));
                }
                
                res.json({
                    {"regions", paginated},
                    {"count", regions.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/regions/:id - Get region
    router.add_route("/admin/regions/:id", router::HttpMethod::GET,
        [region_repo = std::move(region_repo), country_repo = std::move(country_repo),
         province_repo = std::move(province_repo), currency_repo = std::move(currency_repo),
         tax_rate_repo = std::move(tax_rate_repo), tax_region_repo = std::move(tax_region_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto region = region_repo.find(id);
                
                if (!region) {
                    res.status(404).json(http::ErrorResponse::not_found("Region not found"));
                    return;
                }
                
                auto countries = country_repo.find_by({{"region_id", id}});
                auto provinces = province_repo.find_by({{"region_id", id}});
                auto tax_rates = tax_rate_repo.find_by({{"region_id", id}});
                auto tax_regions = tax_region_repo.find_by({{"region_id", id}});
                
                res.json({
                    {"id", region->get_id()},
                    {"name", region->get_name()},
                    {"currency_code", region->get_currency_code()},
                    {"tax_rate", region->get_tax_rate()},
                    {"automatic_taxes", region->get_automatic_taxes()},
                    {"countries", countries},
                    {"provinces", provinces},
                    {"tax_rates", tax_rates},
                    {"tax_regions", tax_regions},
                    {"created_at", region->get_created_at()},
                    {"updated_at", region->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/regions - Create region
    router.add_route("/admin/regions", router::HttpMethod::POST,
        [region_repo = std::move(region_repo), currency_repo = std::move(currency_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                Region region;
                region.add_id(generate_id());
                region.add_name(req.body_as<std::string>().value_or(""));
                region.add_currency_code(req.body_as<std::string>().value_or("usd"));
                region.add_tax_rate(req.body_as<std::string>().value_or("0"));
                region.add_automatic_taxes(true);
                
                region = region_repo.save(std::move(region));
                
                res.status(201).json({{"region", region}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/regions/:id - Update region
    router.add_route("/admin/regions/:id", router::HttpMethod::POST,
        [region_repo = std::move(region_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto region = region_repo.find(id);
                
                if (!region) {
                    res.status(404).json(http::ErrorResponse::not_found("Region not found"));
                    return;
                }
                
                if (auto name = req.body_as<std::string>()) {
                    region->set_name(std::move(*name));
                }
                if (auto tax_rate = req.body_as<std::string>()) {
                    region->set_tax_rate(std::move(*tax_rate));
                }
                if (auto automatic_taxes = req.body_as<bool>()) {
                    region->set_automatic_taxes(*automatic_taxes);
                }
                
                region = region_repo.save(std::move(*region));
                
                res.json({{"region", *region}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/regions/:id - Delete region
    router.add_route("/admin/regions/:id", router::HttpMethod::DELETE,
        [region_repo = std::move(region_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto region = region_repo.find(id);
                
                if (!region) {
                    res.status(404).json(http::ErrorResponse::not_found("Region not found"));
                    return;
                }
                
                region_repo.remove(*region);
                res.status(200).json({{"id", id}, {"object", "region"}, {"deleted", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/regions/:id/countries - Add country to region
    router.add_route("/admin/regions/:id/countries", router::HttpMethod::POST,
        [region_repo = std::move(region_repo), country_repo = std::move(country_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto region_id = req.get_param("id");
                auto region = region_repo.find(region_id);
                
                if (!region) {
                    res.status(404).json(http::ErrorResponse::not_found("Region not found"));
                    return;
                }
                
                std::string country_code = req.body_as<std::string>().value_or("");
                
                Country country;
                country.add_id(generate_id());
                country.add_iso_2(country_code);
                country.add_name(country_code); // Would need lookup in real implementation
                
                country = country_repo.save(std::move(country));
                
                res.status(201).json({{"country", country}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/regions/:id/countries/:country_code - Remove country from region
    router.add_route("/admin/regions/:id/countries/:country_code", router::HttpMethod::DELETE,
        [country_repo = std::move(country_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto country_code = req.get_param("country_code");
                auto countries = country_repo.find_by({{"iso_2", country_code}});
                
                if (countries.empty()) {
                    res.status(404).json(http::ErrorResponse::not_found("Country not found"));
                    return;
                }
                
                country_repo.remove(countries[0]);
                res.status(200).json({{"id", country_code}, {"object", "country"}, {"deleted", true}});
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