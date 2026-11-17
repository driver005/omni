export module medusa_core.api.admin.users;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_user_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto user_repo = db.repository<User>();
    auto api_key_repo = db.repository<ApiKey>();
    auto role_repo = db.repository<Role>(); // Need to add Role entity
    
    // GET /admin/users - List users
    router.add_route("/admin/users", router::HttpMethod::GET,
        [user_repo = std::move(user_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto users = user_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<User> paginated;
                auto start = std::min(offset, users.size());
                auto end = std::min(start + limit, users.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(users[i]));
                }
                
                res.json({
                    {"users", paginated},
                    {"count", users.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/users/:id - Get user
    router.add_route("/admin/users/:id", router::HttpMethod::GET,
        [user_repo = std::move(user_repo), api_key_repo = std::move(api_key_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto user = user_repo.find(id);
                
                if (!user) {
                    res.status(404).json(http::ErrorResponse::not_found("User not found"));
                    return;
                }
                
                auto api_keys = api_key_repo.find_by({{"user_id", id}});
                
                res.json({
                    {"id", user->get_id()},
                    {"email", user->get_email()},
                    {"first_name", user->get_first_name()},
                    {"last_name", user->get_last_name()},
                    {"api_keys", api_keys},
                    {"created_at", user->get_created_at()},
                    {"updated_at", user->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/users - Create user
    router.add_route("/admin/users", router::HttpMethod::POST,
        [user_repo = std::move(user_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                User user;
                user.add_id(generate_id());
                user.add_email(req.body_as<std::string>().value_or(""));
                user.add_first_name(req.body_as<std::string>().value_or(""));
                user.add_last_name(req.body_as<std::string>().value_or(""));
                user.add_password_hash(req.body_as<std::string>().value_or(""));
                
                user = user_repo.save(std::move(user));
                
                res.status(201).json({{"user", user}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/users/:id - Update user
    router.add_route("/admin/users/:id", router::HttpMethod::POST,
        [user_repo = std::move(user_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto user = user_repo.find(id);
                
                if (!user) {
                    res.status(404).json(http::ErrorResponse::not_found("User not found"));
                    return;
                }
                
                if (auto email = req.body_as<std::string>()) {
                    user->set_email(std::move(*email));
                }
                if (auto first_name = req.body_as<std::string>()) {
                    user->set_first_name(std::move(*first_name));
                }
                if (auto last_name = req.body_as<std::string>()) {
                    user->set_last_name(std::move(*last_name));
                }
                if (auto password_hash = req.body_as<std::string>()) {
                    user->set_password_hash(std::move(*password_hash));
                }
                
                user = user_repo.save(std::move(*user));
                
                res.json({{"user", *user}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/users/:id - Delete user
    router.add_route("/admin/users/:id", router::HttpMethod::DELETE,
        [user_repo = std::move(user_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto user = user_repo.find(id);
                
                if (!user) {
                    res.status(404).json(http::ErrorResponse::not_found("User not found"));
                    return;
                }
                
                user_repo.remove(*user);
                res.status(200).json({{"id", id}, {"object", "user"}, {"deleted", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/users/:id/roles - Add role to user
    router.add_route("/admin/users/:id/roles", router::HttpMethod::POST,
        [user_repo = std::move(user_repo), role_repo = std::move(role_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                // In real implementation, add role to user
                res.json({{"success", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/users/:id/roles/:role_id - Remove role from user
    router.add_route("/admin/users/:id/roles/:role_id", router::HttpMethod::DELETE,
        [user_repo = std::move(user_repo), role_repo = std::move(role_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                // In real implementation, remove role from user
                res.status(200).json({{"id", req.get_param("role_id")}, {"object", "role"}, {"deleted", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/users/:id/reset-password - Reset user password
    router.add_route("/admin/users/:id/reset-password", router::HttpMethod::POST,
        [user_repo = std::move(user_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto user = user_repo.find(id);
                
                if (!user) {
                    res.status(404).json(http::ErrorResponse::not_found("User not found"));
                    return;
                }
                
                // In real implementation, generate reset token and send email
                res.json({{"success", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/users/:id/api-keys - Create API key
    router.add_route("/admin/users/:id/api-keys", router::HttpMethod::POST,
        [user_repo = std::move(user_repo), api_key_repo = std::move(api_key_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto user = user_repo.find(id);
                
                if (!user) {
                    res.status(404).json(http::ErrorResponse::not_found("User not found"));
                    return;
                }
                
                ApiKey api_key;
                api_key.add_id(generate_id());
                api_key.add_user_id(id);
                api_key.add_title(req.body_as<std::string>().value_or(""));
                api_key.add_key_hash("hashed_" + generate_id());
                
                api_key = api_key_repo.save(std::move(api_key));
                
                res.status(201).json({{"api_key", api_key}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/users/:id/api-keys/:key_id - Revoke API key
    router.add_route("/admin/users/:id/api-keys/:key_id", router::HttpMethod::DELETE,
        [user_repo = std::move(user_repo), api_key_repo = std::move(api_key_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto key_id = req.get_param("key_id");
                auto api_key = api_key_repo.find(key_id);
                
                if (!api_key) {
                    res.status(404).json(http::ErrorResponse::not_found("API key not found"));
                    return;
                }
                
                if (api_key->get_user_id() != req.get_param("id")) {
                    res.status(403).json(http::ErrorResponse::forbidden("API key does not belong to this user"));
                    return;
                }
                
                api_key->set_revoked_at(std::chrono::system_clock::now());
                api_key = api_key_repo.save(std::move(*api_key));
                
                res.status(200).json({{"id", key_id}, {"object", "api_key"}, {"deleted", true}});
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