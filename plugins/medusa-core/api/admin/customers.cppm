export module medusa_core.api.admin.customers;

import database.connection;
import database.types;
import medusa_core.database.entities;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_customer_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto customer_repo = db.repository<Customer>();
    auto address_repo = db.repository<Address>();
    auto group_repo = db.repository<CustomerGroup>();
    
    // GET /admin/customers - List customers
    router.add_route("/admin/customers", router::HttpMethod::GET,
        [customer_repo = std::move(customer_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto customers = customer_repo.find_by(filters);
                
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<Customer> paginated;
                auto start = std::min(offset, customers.size());
                auto end = std::min(start + limit, customers.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(customers[i]));
                }
                
                res.json({
                    {"customers", paginated},
                    {"count", customers.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/customers/:id - Get customer
    router.add_route("/admin/customers/:id", router::HttpMethod::GET,
        [customer_repo = std::move(customer_repo), address_repo = std::move(address_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto customer = customer_repo.find(id);
                
                if (!customer) {
                    res.status(404).json(http::ErrorResponse::not_found("Customer not found"));
                    return;
                }
                
                auto addresses = address_repo.find_by({{"customer_id", id}});
                
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
                    {"deleted_at", customer->get_deleted_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/customers - Create customer
    router.add_route("/admin/customers", router::HttpMethod::POST,
        [customer_repo = std::move(customer_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                entities::Customer customer;
                customer.add_id(generate_id());
                customer.add_email(req.body_as<std::string>().value_or(""));
                customer.add_has_account(true);
                
                customer = customer_repo.save(std::move(customer));
                
                res.status(201).json({{"customer", customer}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // PUT /admin/customers/:id - Update customer
    router.add_route("/admin/customers/:id", router::HttpMethod::PUT,
        [customer_repo = std::move(customer_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto customer = customer_repo.find(id);
                
                if (!customer) {
                    res.status(404).json(http::ErrorResponse::not_found("Customer not found"));
                    return;
                }
                
                customer = customer_repo.save(std::move(*customer));
                
                res.json({{"customer", customer}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/customers/:id - Delete customer
    router.add_route("/admin/customers/:id", router::HttpMethod::DELETE,
        [customer_repo = std::move(customer_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto customer = customer_repo.find(id);
                
                if (!customer) {
                    res.status(404).json(http::ErrorResponse::not_found("Customer not found"));
                    return;
                }
                
                customer->set_deleted_at(std::chrono::system_clock::now());
                customer = customer_repo.save(std::move(*customer));
                
                res.status(200).json({{"id", id}, {"deleted", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/customers/:id/addresses - Get customer addresses
    router.add_route("/admin/customers/:id/addresses", router::HttpMethod::GET,
        [address_repo = std::move(address_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto customer_id = req.get_param("id");
                auto addresses = address_repo.find_by({{"customer_id", customer_id}});
                res.json({{"addresses", addresses}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/customers/:id/addresses - Create address
    router.add_route("/admin/customers/:id/addresses", router::HttpMethod::POST,
        [customer_repo = std::move(customer_repo), address_repo = std::move(address_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto customer_id = req.get_param("id");
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