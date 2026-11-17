export module medusa_core.api.admin.shipping_profiles;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_shipping_profile_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto profile_repo = db.repository<ShippingProfile>();
    auto option_repo = db.repository<ShippingOption>();
    auto fulfillment_set_repo = db.repository<FulfillmentSet>();
    auto stock_location_repo = db.repository<StockLocation>();
    auto provider_repo = db.repository<FulfillmentProvider>();
    
    // GET /admin/shipping-profiles - List shipping profiles
    router.add_route("/admin/shipping-profiles", router::HttpMethod::GET,
        [profile_repo = std::move(profile_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto profiles = profile_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<ShippingProfile> paginated;
                auto start = std::min(offset, profiles.size());
                auto end = std::min(start + limit, profiles.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(profiles[i]));
                }
                
                res.json({
                    {"shipping_profiles", paginated},
                    {"count", profiles.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/shipping-profiles/:id - Get shipping profile
    router.add_route("/admin/shipping-profiles/:id", router::HttpMethod::GET,
        [profile_repo = std::move(profile_repo), option_repo = std::move(option_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto profile = profile_repo.find(id);
                
                if (!profile) {
                    res.status(404).json(http::ErrorResponse::not_found("Shipping profile not found"));
                    return;
                }
                
                auto options = option_repo.find_by({{"profile_id", id}});
                
                res.json({
                    {"id", profile->get_id()},
                    {"name", profile->get_name()},
                    {"type", profile->get_type()},
                    {"options", options},
                    {"created_at", profile->get_created_at()},
                    {"updated_at", profile->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/shipping-profiles - Create shipping profile
    router.add_route("/admin/shipping-profiles", router::HttpMethod::POST,
        [profile_repo = std::move(profile_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                ShippingProfile profile;
                profile.add_id(generate_id());
                profile.add_name(req.body_as<std::string>().value_or(""));
                profile.add_type(req.body_as<std::string>().value_or("default"));
                
                profile = profile_repo.save(std::move(profile));
                
                res.status(201).json({{"shipping_profile", profile}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/shipping-profiles/:id - Update shipping profile
    router.add_route("/admin/shipping-profiles/:id", router::HttpMethod::POST,
        [profile_repo = std::move(profile_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto profile = profile_repo.find(id);
                
                if (!profile) {
                    res.status(404).json(http::ErrorResponse::not_found("Shipping profile not found"));
                    return;
                }
                
                if (auto name = req.body_as<std::string>()) {
                    profile->set_name(std::move(*name));
                }
                if (auto type = req.body_as<std::string>()) {
                    profile->set_type(std::move(*type));
                }
                
                profile = profile_repo.save(std::move(*profile));
                
                res.json({{"shipping_profile", *profile}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/shipping-profiles/:id - Delete shipping profile
    router.add_route("/admin/shipping-profiles/:id", router::HttpMethod::DELETE,
        [profile_repo = std::move(profile_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto profile = profile_repo.find(id);
                
                if (!profile) {
                    res.status(404).json(http::ErrorResponse::not_found("Shipping profile not found"));
                    return;
                }
                
                profile_repo.remove(*profile);
                res.status(200).json({{"id", id}, {"object", "shipping_profile"}, {"deleted", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/shipping-profiles/:id/options - Add shipping option to profile
    router.add_route("/admin/shipping-profiles/:id/options", router::HttpMethod::POST,
        [profile_repo = std::move(profile_repo), option_repo = std::move(option_repo),
         fulfillment_set_repo = std::move(fulfillment_set_repo), stock_location_repo = std::move(stock_location_repo),
         provider_repo = std::move(provider_repo), shipping_option_type_repo = db.repository<ShippingOptionType>()]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto profile_id = req.get_param("id");
                auto profile = profile_repo.find(profile_id);
                
                if (!profile) {
                    res.status(404).json(http::ErrorResponse::not_found("Shipping profile not found"));
                    return;
                }
                
                // Parse request body
                std::string name = req.body_as<std::string>().value_or("");
                std::string region_id = req.body_as<std::string>().value_or("");
                std::string provider_id = req.body_as<std::string>().value_or("");
                std::string price_type = req.body_as<std::string>().value_or("flat");
                std::string amount = req.body_as<std::string>().value_or("0");
                
                ShippingOption option;
                option.add_id(generate_id());
                option.add_name(name);
                option.add_region_id(region_id);
                option.add_provider_id(provider_id);
                option.add_profile_id(profile_id);
                option.add_price_type(price_type);
                option.add_amount(amount);
                
                option = option_repo.save(std::move(option));
                
                res.status(201).json({{"shipping_option", option}});
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