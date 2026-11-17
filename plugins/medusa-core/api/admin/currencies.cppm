export module medusa_core.api.admin.currencies;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_currency_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto currency_repo = db.repository<Currency>();
    
    // GET /admin/currencies - List currencies
    router.add_route("/admin/currencies", router::HttpMethod::GET,
        [currency_repo = std::move(currency_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto currencies = currency_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<Currency> paginated;
                auto start = std::min(offset, currencies.size());
                auto end = std::min(start + limit, currencies.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(currencies[i]));
                }
                
                res.json({
                    {"currencies", paginated},
                    {"count", currencies.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/currencies/:code - Get currency
    router.add_route("/admin/currencies/:code", router::HttpMethod::GET,
        [currency_repo = std::move(currency_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto code = req.get_param("code");
                auto currency = currency_repo.find(code);
                
                if (!currency) {
                    res.status(404).json(http::ErrorResponse::not_found("Currency not found"));
                    return;
                }
                
                res.json({
                    {"code", currency->get_code()},
                    {"name", currency->get_name()},
                    {"symbol", currency->get_symbol()},
                    {"symbol_native", currency->get_symbol_native()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/currencies - Create currency
    router.add_route("/admin/currencies", router::HttpMethod::POST,
        [currency_repo = std::move(currency_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                Currency currency;
                currency.add_code(req.body_as<std::string>().value_or(""));
                currency.add_name(req.body_as<std::string>().value_or(""));
                currency.add_symbol(req.body_as<std::string>().value_or(""));
                currency.add_symbol_native(req.body_as<std::string>().value_or(""));
                
                currency = currency_repo.save(std::move(currency));
                
                res.status(201).json({{"currency", currency}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/currencies/:code - Update currency
    router.add_route("/admin/currencies/:code", router::HttpMethod::POST,
        [currency_repo = std::move(currency_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto code = req.get_param("code");
                auto currency = currency_repo.find(code);
                
                if (!currency) {
                    res.status(404).json(http::ErrorResponse::not_found("Currency not found"));
                    return;
                }
                
                if (auto name = req.body_as<std::string>()) {
                    currency->set_name(std::move(*name));
                }
                if (auto symbol = req.body_as<std::string>()) {
                    currency->set_symbol(std::move(*symbol));
                }
                if (auto symbol_native = req.body_as<std::string>()) {
                    currency->set_symbol_native(std::move(*symbol_native));
                }
                
                currency = currency_repo.save(std::move(*currency));
                
                res.json({{"currency", *currency}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/currencies/:code - Delete currency
    router.add_route("/admin/currencies/:code", router::HttpMethod::DELETE,
        [currency_repo = std::move(currency_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto code = req.get_param("code");
                auto currency = currency_repo.find(code);
                
                if (!currency) {
                    res.status(404).json(http::ErrorResponse::not_found("Currency not found"));
                    return;
                }
                
                currency_repo.remove(*currency);
                res.status(200).json({{"code", code}, {"object", "currency"}, {"deleted", true}});
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