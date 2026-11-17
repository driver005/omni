export module medusa_core.api.store.customers;

import database.connection;
import database.types;
import medusa_core.database.entities;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::store {

void register_customer_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto customer_repo = db.repository<Customer>();
    auto address_repo = db.repository<Address>();
    
    // GET /store/customers/me - Get authenticated customer
    router.add_route("/store/customers/me", router::HttpMethod::GET,
        [customer_repo = std::move(customer_repo), address_repo = std::move(address_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto customer = customer_repo.find(customer_id);
                
                if (!customer) {
                    res.status(404).json(http::ErrorResponse::not_found("Customer not found"));
                    return;
                }
                
                auto addresses = address_repo.find_by({{"customer_id", customer_id}});
                
                res.json({
                    {"id", customer->get_id()},
                    {"email", customer->get_email()},
                    {"first_name", customer->get_first_name()},
                    {"last_name", customer->get_last_name()},
                    {"phone", customer->get_phone()},
                    {"has_account", customer->get_has_account()},
                    {"addresses", addresses},
                    {"created_at", customer->get_created_at()},
                    {"updated_at", customer->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // PUT /store/customers/me - Update customer
    router.add_route("/store/customers/me", router::HttpMethod::PUT,
        [customer_repo = std::move(customer_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto customer = customer_repo.find(customer_id);
                
                if (!customer) {
                    res.status(404).json(http::ErrorResponse::not_found("Customer not found"));
                    return;
                }
                
                // Update fields from request body
                // In real implementation, parse JSON and update allowed fields
                
                customer = customer_repo.save(std::move(*customer));
                
                res.json({{"customer", customer}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /store/customers/me/addresses - Get customer addresses
    router.add_route("/store/customers/me/addresses", router::HttpMethod::GET,
        [address_repo = std::move(address_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto addresses = address_repo.find_by({{"customer_id", customer_id}});
                
                res.json({{"addresses", addresses}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/customers/me/addresses - Create address
    router.add_route("/store/customers/me/addresses", router::HttpMethod::POST,
        [customer_repo = std::move(customer_repo), address_repo = std::move(address_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto customer = customer_repo.find(customer_id);
                
                if (!customer) {
                    res.status(404).json(http::ErrorResponse::not_found("Customer not found"));
                    return;
                }
                
                entities::Address address;
                address.add_id(generate_id());
                address.add_customer_id(customer_id);
                address.add_first_name(req.body_as<std::string>().value_or(""));
                address.add_address_1(req.body_as<std::string>().value_or(""));
                address.add_city(req.body_as<std::string>().value_or(""));
                address.add_country_code(req.body_as<std::string>().value_or("US"));
                
                address = address_repo.save(std::move(address));
                
                res.status(201).json({{"address", address}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // PUT /store/customers/me/addresses/:id - Update address
    router.add_route("/store/customers/me/addresses/:id", router::HttpMethod::PUT,
        [address_repo = std::move(address_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto address = address_repo.find(id);
                
                if (!address || address->get_customer_id() != customer_id) {
                    res.status(404).json(http::ErrorResponse::not_found("Address not found"));
                    return;
                }
                
                address = address_repo.save(std::move(*address));
                
                res.json({{"address", address}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /store/customers/me/addresses/:id - Delete address
    router.add_route("/store/customers/me/addresses/:id", router::HttpMethod::DELETE,
        [address_repo = std::move(address_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto address = address_repo.find(id);
                
                if (!address || address->get_customer_id() != customer_id) {
                    res.status(404).json(http::ErrorResponse::not_found("Address not found"));
                    return;
                }
                
                address_repo.remove(*address);
                
                res.status(200).json({{"id", id}, {"deleted", true}});
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

} // namespace medusa_core::api::store