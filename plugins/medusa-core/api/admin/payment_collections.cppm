export module medusa_core.api.admin.payment_collections;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_payment_collection_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto payment_collection_repo = db.repository<PaymentCollection>();
    auto payment_session_repo = db.repository<PaymentSession>();
    auto payment_repo = db.repository<Payment>();
    auto refund_repo = db.repository<Refund>();
    
    // GET /admin/payment-collections - List payment collections
    router.add_route("/admin/payment-collections", router::HttpMethod::GET,
        [payment_collection_repo = std::move(payment_collection_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto collections = payment_collection_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<PaymentCollection> paginated;
                auto start = std::min(offset, collections.size());
                auto end = std::min(start + limit, collections.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(collections[i]));
                }
                
                res.json({
                    {"payment_collections", paginated},
                    {"count", collections.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/payment-collections/:id - Get payment collection
    router.add_route("/admin/payment-collections/:id", router::HttpMethod::GET,
        [payment_collection_repo = std::move(payment_collection_repo), 
         payment_session_repo = std::move(payment_session_repo),
         payment_repo = std::move(payment_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto collection = payment_collection_repo.find(id);
                
                if (!collection) {
                    res.status(404).json(http::ErrorResponse::not_found("Payment collection not found"));
                    return;
                }
                
                auto sessions = payment_session_repo.find_by({{"payment_collection_id", id}});
                auto payments = payment_repo.find_by({{"payment_collection_id", id}});
                
                res.json({
                    {"id", collection->get_id()},
                    {"region_id", collection->get_region_id()},
                    {"currency_code", collection->get_currency_code()},
                    {"status", collection->get_status()},
                    {"payment_sessions", sessions},
                    {"payments", payments},
                    {"created_at", collection->get_created_at()},
                    {"updated_at", collection->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/payment-collections - Create payment collection
    router.add_route("/admin/payment-collections", router::HttpMethod::POST,
        [payment_collection_repo = std::move(payment_collection_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                PaymentCollection collection;
                collection.add_id(generate_id());
                collection.add_region_id(req.body_as<std::string>().value_or(""));
                collection.add_currency_code(req.body_as<std::string>().value_or("usd"));
                collection.add_status(std::string("pending"));
                
                collection = payment_collection_repo.save(std::move(collection));
                
                res.status(201).json({{"payment_collection", collection}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/payment-collections/:id - Update payment collection
    router.add_route("/admin/payment-collections/:id", router::HttpMethod::POST,
        [payment_collection_repo = std::move(payment_collection_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto collection = payment_collection_repo.find(id);
                
                if (!collection) {
                    res.status(404).json(http::ErrorResponse::not_found("Payment collection not found"));
                    return;
                }
                
                if (auto status = req.body_as<std::string>()) {
                    collection->set_status(std::move(*status));
                }
                
                collection = payment_collection_repo.save(std::move(*collection));
                
                res.json({{"payment_collection", *collection}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/payment-collections/:id/authorize - Authorize payment collection
    router.add_route("/admin/payment-collections/:id/authorize", router::HttpMethod::POST,
        [payment_collection_repo = std::move(payment_collection_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto collection = payment_collection_repo.find(id);
                
                if (!collection) {
                    res.status(404).json(http::ErrorResponse::not_found("Payment collection not found"));
                    return;
                }
                
                collection->set_status(std::string("authorized"));
                collection = payment_collection_repo.save(std::move(*collection));
                
                res.json({{"payment_collection", *collection}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/payment-collections/:id - Delete payment collection
    router.add_route("/admin/payment-collections/:id", router::HttpMethod::DELETE,
        [payment_collection_repo = std::move(payment_collection_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto collection = payment_collection_repo.find(id);
                
                if (!collection) {
                    res.status(404).json(http::ErrorResponse::not_found("Payment collection not found"));
                    return;
                }
                
                payment_collection_repo.remove(*collection);
                res.status(200).json({{"id", id}, {"object", "payment_collection"}, {"deleted", true}});
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