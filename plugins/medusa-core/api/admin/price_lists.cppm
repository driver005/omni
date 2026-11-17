export module medusa_core.api.admin.price_lists;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_price_list_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto price_list_repo = db.repository<PriceList>();
    auto price_repo = db.repository<Price>();
    
    // GET /admin/price-lists - List price lists
    router.add_route("/admin/price-lists", router::HttpMethod::GET,
        [price_list_repo = std::move(price_list_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto price_lists = price_list_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<PriceList> paginated;
                auto start = std::min(offset, price_lists.size());
                auto end = std::min(start + limit, price_lists.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(price_lists[i]));
                }
                
                res.json({
                    {"price_lists", paginated},
                    {"count", price_lists.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/price-lists/:id - Get price list
    router.add_route("/admin/price-lists/:id", router::HttpMethod::GET,
        [price_list_repo = std::move(price_list_repo), price_repo = std::move(price_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto price_list = price_list_repo.find(id);
                
                if (!price_list) {
                    res.status(404).json(http::ErrorResponse::not_found("Price list not found"));
                    return;
                }
                
                auto prices = price_repo.find_by({{"price_list_id", id}});
                
                res.json({
                    {"id", price_list->get_id()},
                    {"title", price_list->get_title()},
                    {"type", price_list->get_type()},
                    {"status", price_list->get_status()},
                    {"prices", prices},
                    {"created_at", price_list->get_created_at()},
                    {"updated_at", price_list->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/price-lists - Create price list
    router.add_route("/admin/price-lists", router::HttpMethod::POST,
        [price_list_repo = std::move(price_list_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                PriceList price_list;
                price_list.add_id(generate_id());
                price_list.add_title(req.body_as<std::string>().value_or(""));
                price_list.add_type(req.body_as<std::string>().value_or("sale"));
                price_list.add_status(req.body_as<std::string>().value_or("active"));
                
                price_list = price_list_repo.save(std::move(price_list));
                
                res.status(201).json({{"price_list", price_list}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/price-lists/:id - Update price list
    router.add_route("/admin/price-lists/:id", router::HttpMethod::POST,
        [price_list_repo = std::move(price_list_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto price_list = price_list_repo.find(id);
                
                if (!price_list) {
                    res.status(404).json(http::ErrorResponse::not_found("Price list not found"));
                    return;
                }
                
                if (auto title = req.body_as<std::string>()) {
                    price_list->set_title(std::move(*title));
                }
                if (auto status = req.body_as<std::string>()) {
                    price_list->set_status(std::move(*status));
                }
                
                price_list = price_list_repo.save(std::move(*price_list));
                
                res.json({{"price_list", *price_list}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/price-lists/:id - Delete price list
    router.add_route("/admin/price-lists/:id", router::HttpMethod::DELETE,
        [price_list_repo = std::move(price_list_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto price_list = price_list_repo.find(id);
                
                if (!price_list) {
                    res.status(404).json(http::ErrorResponse::not_found("Price list not found"));
                    return;
                }
                
                price_list_repo.remove(*price_list);
                res.status(200).json({{"id", id}, {"object", "price_list"}, {"deleted", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/price-lists/:id/prices - Add price to price list
    router.add_route("/admin/price-lists/:id/prices", router::HttpMethod::POST,
        [price_list_repo = std::move(price_list_repo), price_repo = std::move(price_repo),
         variant_repo = std::move(variant_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto price_list_id = req.get_param("id");
                auto price_list = price_list_repo.find(price_list_id);
                
                if (!price_list) {
                    res.status(404).json(http::ErrorResponse::not_found("Price list not found"));
                    return;
                }
                
                std::string variant_id = req.body_as<std::string>().value_or("");
                std::string amount = req.body_as<std::string>().value_or("0");
                std::string currency_code = req.body_as<std::string>().value_or("usd");
                
                Price price;
                price.add_id(generate_id());
                price.add_price_list_id(price_list_id);
                price.add_variant_id(variant_id);
                price.add_amount(amount);
                price.add_currency_code(currency_code);
                
                price = price_repo.save(std::move(price));
                
                res.status(201).json({{"price", price}});
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