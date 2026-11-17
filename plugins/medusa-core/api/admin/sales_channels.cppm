export module medusa_core.api.admin.sales_channels;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_sales_channel_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto channel_repo = db.repository<SalesChannel>();
    auto publishable_key_repo = db.repository<PublishableApiKey>();
    
    // GET /admin/sales-channels - List sales channels
    router.add_route("/admin/sales-channels", router::HttpMethod::GET,
        [channel_repo = std::move(channel_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto channels = channel_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<SalesChannel> paginated;
                auto start = std::min(offset, channels.size());
                auto end = std::min(start + limit, channels.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(channels[i]));
                }
                
                res.json({
                    {"sales_channels", paginated},
                    {"count", channels.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/sales-channels/:id - Get sales channel
    router.add_route("/admin/sales-channels/:id", router::HttpMethod::GET,
        [channel_repo = std::move(channel_repo), publishable_key_repo = std::move(publishable_key_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto channel = channel_repo.find(id);
                
                if (!channel) {
                    res.status(404).json(http::ErrorResponse::not_found("Sales channel not found"));
                    return;
                }
                
                auto keys = publishable_key_repo.find_by({{"sales_channel_id", id}});
                
                res.json({
                    {"id", channel->get_id()},
                    {"name", channel->get_name()},
                    {"description", channel->get_description()},
                    {"is_disabled", channel->get_is_disabled()},
                    {"publishable_api_keys", keys},
                    {"created_at", channel->get_created_at()},
                    {"updated_at", channel->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/sales-channels - Create sales channel
    router.add_route("/admin/sales-channels", router::HttpMethod::POST,
        [channel_repo = std::move(channel_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                SalesChannel channel;
                channel.add_id(generate_id());
                channel.add_name(req.body_as<std::string>().value_or(""));
                channel.add_description(req.body_as<std::string>().value_or(""));
                
                channel = channel_repo.save(std::move(channel));
                
                res.status(201).json({{"sales_channel", channel}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/sales-channels/:id - Update sales channel
    router.add_route("/admin/sales-channels/:id", router::HttpMethod::POST,
        [channel_repo = std::move(channel_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto channel = channel_repo.find(id);
                
                if (!channel) {
                    res.status(404).json(http::ErrorResponse::not_found("Sales channel not found"));
                    return;
                }
                
                if (auto name = req.body_as<std::string>()) {
                    channel->set_name(std::move(*name));
                }
                if (auto description = req.body_as<std::string>()) {
                    channel->set_description(std::move(*description));
                }
                if (auto is_disabled = req.body_as<bool>()) {
                    channel->set_is_disabled(*is_disabled);
                }
                
                channel = channel_repo.save(std::move(*channel));
                
                res.json({{"sales_channel", *channel}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/sales-channels/:id - Delete sales channel
    router.add_route("/admin/sales-channels/:id", router::HttpMethod::DELETE,
        [channel_repo = std::move(channel_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto channel = channel_repo.find(id);
                
                if (!channel) {
                    res.status(404).json(http::ErrorResponse::not_found("Sales channel not found"));
                    return;
                }
                
                channel_repo.remove(*channel);
                res.status(200).json({{"id", id}, {"object", "sales_channel"}, {"deleted", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/sales-channels/:id/api-keys - Create publishable API key
    router.add_route("/admin/sales-channels/:id/api-keys", router::HttpMethod::POST,
        [channel_repo = std::move(channel_repo), publishable_key_repo = std::move(publishable_key_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto channel = channel_repo.find(id);
                
                if (!channel) {
                    res.status(404).json(http::ErrorResponse::not_found("Sales channel not found"));
                    return;
                }
                
                PublishableApiKey key;
                key.add_id(generate_id());
                key.add_sales_channel_id(id);
                key.add_key("pk_" + generate_id());
                
                key = publishable_key_repo.save(std::move(key));
                
                res.status(201).json({{"publishable_api_key", key}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/sales-channels/:id/api-keys/:key_id - Revoke publishable API key
    router.add_route("/admin/sales-channels/:id/api-keys/:key_id", router::HttpMethod::DELETE,
        [channel_repo = std::move(channel_repo), publishable_key_repo = std::move(publishable_key_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto key_id = req.get_param("key_id");
                auto key = publishable_key_repo.find(key_id);
                
                if (!key) {
                    res.status(404).json(http::ErrorResponse::not_found("Publishable API key not found"));
                    return;
                }
                
                if (key->get_sales_channel_id() != req.get_param("id")) {
                    res.status(403).json(http::ErrorResponse::forbidden("Key does not belong to this sales channel"));
                    return;
                }
                
                key->set_revoked(true);
                key->set_revoked_at(std::chrono::system_clock::now());
                key = publishable_key_repo.save(std::move(*key));
                
                res.status(200).json({{"id", key_id}, {"object", "publishable_api_key"}, {"deleted", true}});
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