export module medusa_core.api.auth.routes;

import database.connection;
import database.types;
import medusa_core.database.entities;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::auth {

void register_auth_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto user_repo = db.repository<User>();
    auto api_key_repo = db.repository<ApiKey>();
    auto customer_repo = db.repository<Customer>();
    auto publishable_key_repo = db.repository<PublishableApiKey>();
    
    // POST /auth/token - Get access token (login)
    router.add_route("/auth/token", router::HttpMethod::POST,
        [user_repo = std::move(user_repo), api_key_repo = std::move(api_key_repo),
         customer_repo = std::move(customer_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                // Parse credentials from request body
                // In real implementation, parse JSON
                
                // This is a placeholder - real implementation would:
                // 1. Validate email/password or API key
                // 2. Generate JWT token
                // 3. Return access token and refresh token
                
                res.json({
                    {"access_token", "mock_access_token"},
                    {"refresh_token", "mock_refresh_token"},
                    {"expires_in", 3600},
                    {"token_type", "bearer"}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /auth/token/refresh - Refresh access token
    router.add_route("/auth/token/refresh", router::HttpMethod::POST,
        [](const http::Request& req, http::Response& res) {
            try {
                // Parse refresh token
                // Validate and generate new access token
                
                res.json({
                    {"access_token", "mock_new_access_token"},
                    {"refresh_token", "mock_new_refresh_token"},
                    {"expires_in", 3600},
                    {"token_type", "bearer"}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /auth/session - Create session (cookie-based auth)
    router.add_route("/auth/session", router::HttpMethod::POST,
        [user_repo = std::move(user_repo), customer_repo = std::move(customer_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                // Parse credentials
                // Create session cookie
                
                http::Response::Cookie cookie;
                cookie.m_name = "session_id";
                cookie.m_value = generate_id();
                cookie.m_http_only = true;
                cookie.m_secure = true;
                cookie.m_same_site = "lax";
                cookie.m_max_age = 86400; // 24 hours
                cookie.m_path = "/";
                
                res.set_cookie(cookie);
                res.json({
                    {"success", true}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /auth/session - Destroy session (logout)
    router.add_route("/auth/session", router::HttpMethod::DELETE,
        [](const http::Request& req, http::Response& res) {
            try {
                http::Response::Cookie cookie;
                cookie.m_name = "session_id";
                cookie.m_value = "";
                cookie.m_http_only = true;
                cookie.m_secure = true;
                cookie.m_same_site = "lax";
                cookie.m_max_age = 0;
                cookie.m_expires = std::chrono::system_clock::from_time_t(0);
                cookie.m_path = "/";
                
                res.set_cookie(cookie);
                res.json({{"success", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /auth/identity/customer - Customer registration
    router.add_route("/auth/identity/customer", router::HttpMethod::POST,
        [customer_repo = std::move(customer_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                // Parse registration data
                // Create customer
                
                entities::Customer customer;
                customer.add_id(generate_id());
                customer.add_email(req.body_as<std::string>().value_or(""));
                customer.add_has_account(true);
                
                customer = customer_repo.save(std::move(customer));
                
                // Generate auth token for new customer
                res.status(201).json({
                    {"customer", customer},
                    {"access_token", "mock_access_token"},
                    {"refresh_token", "mock_refresh_token"},
                    {"expires_in", 3600},
                    {"token_type", "bearer"}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /auth/identity/user - Admin user registration (invite only)
    router.add_route("/auth/identity/user", router::HttpMethod::POST,
        [user_repo = std::move(user_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                // Parse registration data
                // Create user
                
                entities::User user;
                user.add_id(generate_id());
                user.add_email(req.body_as<std::string>().value_or(""));
                
                user = user_repo.save(std::move(user));
                
                res.status(201).json({
                    {"user", user},
                    {"access_token", "mock_access_token"},
                    {"refresh_token", "mock_refresh_token"},
                    {"expires_in", 3600},
                    {"token_type", "bearer"}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /auth/password/forgot - Request password reset
    router.add_route("/auth/password/forgot", router::HttpMethod::POST,
        [user_repo = std::move(user_repo), customer_repo = std::move(customer_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                // Parse email
                // Send password reset email
                
                res.json({{"success", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /auth/password/reset - Reset password
    router.add_route("/auth/password/reset", router::HttpMethod::POST,
        [user_repo = std::move(user_repo), customer_repo = std::move(customer_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                // Parse token and new password
                // Reset password
                
                res.json({{"success", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /auth/publishable - Get publishable API key info
    router.add_route("/auth/publishable", router::HttpMethod::GET,
        [publishable_key_repo = std::move(publishable_key_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                // Get publishable key from header
                auto pub_key = req.get_header("x-publishable-api-key");
                
                if (pub_key.empty()) {
                    res.status(401).json(http::ErrorResponse::unauthorized("Publishable API key required"));
                    return;
                }
                
                // In real implementation, find and validate publishable key
                res.json({
                    {"publishable_key", pub_key},
                    {"sales_channel_id", "sc_default"}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /auth/user/providers - List auth providers for user actor
    router.add_route("/auth/user/providers", router::HttpMethod::GET,
        [](const http::Request& req, http::Response& res) {
            try {
                // In real implementation, list registered auth providers for user actor
                // For now, return mock emailpass provider
                res.json({
                    {"providers", std::vector<std::map<std::string, std::string>>{
                        {
                            {"id", "emailpass"},
                            {"identifier", "emailpass"},
                            {"display_name", "Email/Password Authentication"},
                            {"flow", "credentials"}
                        }
                    }}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /auth/customer/providers - List auth providers for customer actor
    router.add_route("/auth/customer/providers", router::HttpMethod::GET,
        [](const http::Request& req, http::Response& res) {
            try {
                // In real implementation, list registered auth providers for customer actor
                // For now, return mock emailpass provider
                res.json({
                    {"providers", std::vector<std::map<std::string, std::string>>{
                        {
                            {"id", "emailpass"},
                            {"identifier", "emailpass"},
                            {"display_name", "Email/Password Authentication"},
                            {"flow", "credentials"}
                        }
                    }}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /auth/emailpass/user - Provision user via emailpass
    router.add_route("/auth/emailpass/user", router::HttpMethod::POST,
        [user_repo = std::move(user_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                // Parse request body
                // In real implementation, validate and create user
                entities::User user;
                user.add_id(generate_id());
                user.add_email(req.body_as<std::string>().value_or(""));
                
                user = user_repo.save(std::move(user));
                
                res.status(201).json({
                    {"user", user},
                    {"access_token", "mock_access_token"},
                    {"refresh_token", "mock_refresh_token"},
                    {"expires_in", 3600},
                    {"token_type", "bearer"}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /auth/emailpass/customer - Provision customer via emailpass
    router.add_route("/auth/emailpass/customer", router::HttpMethod::POST,
        [customer_repo = std::move(customer_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                // Parse request body
                // In real implementation, validate and create customer
                entities::Customer customer;
                customer.add_id(generate_id());
                customer.add_email(req.body_as<std::string>().value_or(""));
                customer.add_has_account(true);
                
                customer = customer_repo.save(std::move(customer));
                
                res.status(201).json({
                    {"customer", customer},
                    {"access_token", "mock_access_token"},
                    {"refresh_token", "mock_refresh_token"},
                    {"expires_in", 3600},
                    {"token_type", "bearer"}
                });
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

} // namespace medusa_core::api::auth