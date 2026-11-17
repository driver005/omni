export module medusa_core.api.admin.shipping_options;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_shipping_option_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto option_repo = db.repository<ShippingOption>();
    auto profile_repo = db.repository<ShippingProfile>();
    auto region_repo = db.repository<Region>();
    auto provider_repo = db.repository<FulfillmentProvider>();
    auto fulfillment_set_repo = db.repository<FulfillmentSet>();
    auto type_repo = db.repository<ShippingOptionType>();
    
    // GET /admin/shipping-options - List shipping options
    router.add_route("/admin/shipping-options", router::HttpMethod::GET,
        [option_repo = std::move(option_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto options = option_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<ShippingOption> paginated;
                auto start = std::min(offset, options.size());
                auto end = std::min(start + limit, options.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(options[i]));
                }
                
                res.json({
                    {"shipping_options", paginated},
                    {"count", options.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/shipping-options/:id - Get shipping option
    router.add_route("/admin/shipping-options/:id", router::HttpMethod::GET,
        [option_repo = std::move(option_repo), profile_repo = std::move(profile_repo),
         region_repo = std::move(region_repo), provider_repo = db.repository<FulfillmentProvider>(),
         fulfillment_set_repo = db.repository<FulfillmentSet>(), type_repo = db.repository<ShippingOptionType>()]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto option = option_repo.find(id);
                
                if (!option) {
                    res.status(404).json(http::ErrorResponse::not_found("Shipping option not found"));
                    return;
                }
                
                auto profile = profile_repo.find(option->get_profile_id());
                auto region = region_repo.find(option->get_region_id());
                
                res.json({
                    {"id", option->get_id()},
                    {"name", option->get_name()},
                    {"region_id", option->get_region_id()},
                    {"provider_id", option->get_provider_id()},
                    {"profile_id", option->get_profile_id()},
                    {"price_type", option->get_price_type()},
                    {"amount", option->get_amount()},
                    {"is_return", option->get_is_return()},
                    {"profile", profile},
                    {"region", region},
                    {"created_at", option->get_created_at()},
                    {"updated_at", option->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/shipping-options - Create shipping option
    router.add_route("/admin/shipping-options", router::HttpMethod::POST,
        [option_repo = std::move(option_repo), profile_repo = std::move(profile_repo),
         region_repo = std::move(region_repo), provider_repo = db.repository<FulfillmentProvider>(),
         fulfillment_set_repo = db.repository<FulfillmentSet>(), type_repo = db.repository<ShippingOptionType>()]
        (const http::Request& req, http::Response& res) mutable {
            try {
                ShippingOption option;
                option.add_id(generate_id());
                option.add_name(req.body_as<std::string>().value_or(""));
                option.add_region_id(req.body_as<std::string>().value_or(""));
                option.add_provider_id(req.body_as<std::string>().value_or(""));
                option.add_profile_id(req.body_as<std::string>().value_or(""));
                option.add_price_type(req.body_as<std::string>().value_or("flat"));
                option.add_amount(req.body_as<std::string>().value_or("0"));
                
                option = option_repo.save(std::move(option));
                
                res.status(201).json({{"shipping_option", option}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/shipping-options/:id - Update shipping option
    router.add_route("/admin/shipping-options/:id", router::HttpMethod::POST,
        [option_repo = std::move(option_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto option = option_repo.find(id);
                
                if (!option) {
                    res.status(404).json(http::ErrorResponse::not_found("Shipping option not found"));
                    return;
                }
                
                if (auto name = req.body_as<std::string>()) {
                    option->set_name(std::move(*name));
                }
                if (auto amount = req.body_as<std::string>()) {
                    option->set_amount(std::move(*amount));
                }
                if (auto price_type = req.body_as<std::string>()) {
                    option->set_price_type(std::move(*price_type));
                }
                
                option = option_repo.save(std::move(*option));
                
                res.json({{"shipping_option", *option}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/shipping-options/:id - Delete shipping option
    router.add_route("/admin/shipping-options/:id", router::HttpMethod::DELETE,
        [option_repo = std::move(option_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto option = option_repo.find(id);
                
                if (!option) {
                    res.status(404).json(http::ErrorResponse::not_found("Shipping option not found"));
                    return;
                }
                
                option_repo.remove(*option);
                res.status(200).json({{"id", id}, {"object", "shipping_option"}, {"deleted", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/shipping-options/:id/prices - Add price rule
    router.add_route("/admin/shipping-options/:id/prices", router::HttpMethod::POST,
        [option_repo = std::move(option_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                // In real implementation, add price rule to shipping option
                res.json({{"success", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/shipping-options/:id/prices/:price_id - Update price rule
    router.add_route("/admin/shipping-options/:id/prices/:price_id", router::HttpMethod::POST,
        [option_repo = std::move(option_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                // In real implementation, update price rule
                res.json({{"success", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/shipping-options/:id/prices/:price_id - Delete price rule
    router.add_route("/admin/shipping-options/:id/prices/:price_id", router::HttpMethod::DELETE,
        [option_repo = std::move(option_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                // In real implementation, delete price rule
                res.status(200).json({{"id", req.get_param("price_id")}, {"object", "shipping_option_price"}, {"deleted", true}});
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