export module medusa_core.api.admin.return_reasons;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_return_reason_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto return_reason_repo = db.repository<ReturnReason>();
    
    // GET /admin/return-reasons - List return reasons
    router.add_route("/admin/return-reasons", router::HttpMethod::GET,
        [return_reason_repo = std::move(return_reason_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto reasons = return_reason_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<ReturnReason> paginated;
                auto start = std::min(offset, reasons.size());
                auto end = std::min(start + limit, reasons.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(reasons[i]));
                }
                
                res.json({
                    {"return_reasons", paginated},
                    {"count", reasons.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/return-reasons/:id - Get return reason
    router.add_route("/admin/return-reasons/:id", router::HttpMethod::GET,
        [return_reason_repo = std::move(return_reason_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto reason = return_reason_repo.find(id);
                
                if (!reason) {
                    res.status(404).json(http::ErrorResponse::not_found("Return reason not found"));
                    return;
                }
                
                res.json({
                    {"id", reason->get_id()},
                    {"value", reason->get_value()},
                    {"label", reason->get_label()},
                    {"description", reason->get_description()},
                    {"created_at", reason->get_created_at()},
                    {"updated_at", reason->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/return-reasons - Create return reason
    router.add_route("/admin/return-reasons", router::HttpMethod::POST,
        [return_reason_repo = std::move(return_reason_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                ReturnReason reason;
                reason.add_id(generate_id());
                reason.add_value(req.body_as<std::string>().value_or(""));
                reason.add_label(req.body_as<std::string>().value_or(""));
                reason.add_description(req.body_as<std::string>().value_or(""));
                
                reason = return_reason_repo.save(std::move(reason));
                
                res.status(201).json({{"return_reason", reason}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/return-reasons/:id - Update return reason
    router.add_route("/admin/return-reasons/:id", router::HttpMethod::POST,
        [return_reason_repo = std::move(return_reason_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto reason = return_reason_repo.find(id);
                
                if (!reason) {
                    res.status(404).json(http::ErrorResponse::not_found("Return reason not found"));
                    return;
                }
                
                if (auto value = req.body_as<std::string>()) {
                    reason->set_value(std::move(*value));
                }
                if (auto label = req.body_as<std::string>()) {
                    reason->set_label(std::move(*label));
                }
                if (auto description = req.body_as<std::string>()) {
                    reason->set_description(std::move(*description));
                }
                
                reason = return_reason_repo.save(std::move(*reason));
                
                res.json({{"return_reason", *reason}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/return-reasons/:id - Delete return reason
    router.add_route("/admin/return-reasons/:id", router::HttpMethod::DELETE,
        [return_reason_repo = std::move(return_reason_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto reason = return_reason_repo.find(id);
                
                if (!reason) {
                    res.status(404).json(http::ErrorResponse::not_found("Return reason not found"));
                    return;
                }
                
                return_reason_repo.remove(*reason);
                res.status(200).json({{"id", id}, {"object", "return_reason"}, {"deleted", true}});
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