export module medusa_core.api.admin.customer_groups;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_customer_group_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto group_repo = db.repository<CustomerGroup>();
    auto customer_repo = db.repository<Customer>();
    
    // GET /admin/customer-groups - List customer groups
    router.add_route("/admin/customer-groups", router::HttpMethod::GET,
        [group_repo = std::move(group_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto groups = group_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<CustomerGroup> paginated;
                auto start = std::min(offset, groups.size());
                auto end = std::min(start + limit, groups.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(groups[i]));
                }
                
                res.json({
                    {"customer_groups", paginated},
                    {"count", groups.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/customer-groups/:id - Get customer group
    router.add_route("/admin/customer-groups/:id", router::HttpMethod::GET,
        [group_repo = std::move(group_repo), customer_repo = std::move(customer_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto group = group_repo.find(id);
                
                if (!group) {
                    res.status(404).json(http::ErrorResponse::not_found("Customer group not found"));
                    return;
                }
                
                auto customers = customer_repo.find_by({{"group_id", id}});
                
                res.json({
                    {"id", group->get_id()},
                    {"name", group->get_name()},
                    {"customers", customers},
                    {"created_at", group->get_created_at()},
                    {"updated_at", group->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/customer-groups - Create customer group
    router.add_route("/admin/customer-groups", router::HttpMethod::POST,
        [group_repo = std::move(group_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                CustomerGroup group;
                group.add_id(generate_id());
                group.add_name(req.body_as<std::string>().value_or(""));
                
                group = group_repo.save(std::move(group));
                
                res.status(201).json({{"customer_group", group}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/customer-groups/:id - Update customer group
    router.add_route("/admin/customer-groups/:id", router::HttpMethod::POST,
        [group_repo = std::move(group_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto group = group_repo.find(id);
                
                if (!group) {
                    res.status(404).json(http::ErrorResponse::not_found("Customer group not found"));
                    return;
                }
                
                if (auto name = req.body_as<std::string>()) {
                    group->set_name(std::move(*name));
                }
                
                group = group_repo.save(std::move(*group));
                
                res.json({{"customer_group", *group}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/customer-groups/:id - Delete customer group
    router.add_route("/admin/customer-groups/:id", router::HttpMethod::DELETE,
        [group_repo = std::move(group_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto group = group_repo.find(id);
                
                if (!group) {
                    res.status(404).json(http::ErrorResponse::not_found("Customer group not found"));
                    return;
                }
                
                group_repo.remove(*group);
                res.status(200).json({{"id", id}, {"object", "customer_group"}, {"deleted", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/customer-groups/:id/customers - Add customers to group
    router.add_route("/admin/customer-groups/:id/customers", router::HttpMethod::POST,
        [group_repo = std::move(group_repo), customer_repo = std::move(customer_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto group = group_repo.find(id);
                
                if (!group) {
                    res.status(404).json(http::ErrorResponse::not_found("Customer group not found"));
                    return;
                }
                
                // Parse customer IDs from request body
                // In real implementation, parse JSON array of customer IDs
                
                res.status(200).json({{"customer_group", *group}});
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