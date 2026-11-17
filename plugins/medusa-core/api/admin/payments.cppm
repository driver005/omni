export module medusa_core.api.admin.payments;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_payment_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto payment_repo = db.repository<Payment>();
    auto payment_collection_repo = db.repository<PaymentCollection>();
    auto payment_session_repo = db.repository<PaymentSession>();
    auto refund_repo = db.repository<Refund>();
    
    // GET /admin/payments - List payments
    router.add_route("/admin/payments", router::HttpMethod::GET,
        [payment_repo = std::move(payment_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto payments = payment_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<Payment> paginated;
                auto start = std::min(offset, payments.size());
                auto end = std::min(start + limit, payments.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(payments[i]));
                }
                
                res.json({
                    {"payments", paginated},
                    {"count", payments.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/payments/:id - Get payment
    router.add_route("/admin/payments/:id", router::HttpMethod::GET,
        [payment_repo = std::move(payment_repo), refund_repo = std::move(refund_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto payment = payment_repo.find(id);
                
                if (!payment) {
                    res.status(404).json(http::ErrorResponse::not_found("Payment not found"));
                    return;
                }
                
                auto refunds = refund_repo.find_by({{"payment_id", id}});
                
                res.json({
                    {"id", payment->get_id()},
                    {"payment_collection_id", payment->get_payment_collection_id()},
                    {"provider_id", payment->get_provider_id()},
                    {"amount", payment->get_amount()},
                    {"currency_code", payment->get_currency_code()},
                    {"status", payment->get_status()},
                    {"refunds", refunds},
                    {"created_at", payment->get_created_at()},
                    {"updated_at", payment->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/payments - Create payment
    router.add_route("/admin/payments", router::HttpMethod::POST,
        [payment_repo = std::move(payment_repo), payment_collection_repo = std::move(payment_collection_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                std::string payment_collection_id = req.body_as<std::string>().value_or("");
                
                auto collection = payment_collection_repo.find(payment_collection_id);
                if (!collection) {
                    res.status(404).json(http::ErrorResponse::not_found("Payment collection not found"));
                    return;
                }
                
                Payment payment;
                payment.add_id(generate_id());
                payment.add_payment_collection_id(payment_collection_id);
                payment.add_provider_id(req.body_as<std::string>().value_or(""));
                payment.add_amount(req.body_as<std::string>().value_or("0"));
                payment.add_currency_code(req.body_as<std::string>().value_or("usd"));
                payment.add_status(std::string("pending"));
                
                payment = payment_repo.save(std::move(payment));
                
                res.status(201).json({{"payment", payment}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/payments/:id/capture - Capture payment
    router.add_route("/admin/payments/:id/capture", router::HttpMethod::POST,
        [payment_repo = std::move(payment_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto payment = payment_repo.find(id);
                
                if (!payment) {
                    res.status(404).json(http::ErrorResponse::not_found("Payment not found"));
                    return;
                }
                
                if (payment->get_status() != "authorized") {
                    res.status(400).json(http::ErrorResponse::invalid_data("Payment not authorized"));
                    return;
                }
                
                payment->set_status(std::string("captured"));
                payment = payment_repo.save(std::move(*payment));
                
                res.json({{"payment", *payment}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/payments/:id/refund - Refund payment
    router.add_route("/admin/payments/:id/refund", router::HttpMethod::POST,
        [payment_repo = std::move(payment_repo), refund_repo = std::move(refund_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto payment = payment_repo.find(id);
                
                if (!payment) {
                    res.status(404).json(http::ErrorResponse::not_found("Payment not found"));
                    return;
                }
                
                std::string amount = req.body_as<std::string>().value_or(payment->get_amount());
                std::string reason = req.body_as<std::string>().value_or("");
                
                Refund refund;
                refund.add_id(generate_id());
                refund.add_payment_id(id);
                refund.add_amount(amount);
                refund.add_currency_code(payment->get_currency_code());
                refund.add_reason(reason);
                
                refund = refund_repo.save(std::move(refund));
                
                res.status(201).json({{"refund", refund}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/payments/:id/refunds - List refunds for payment
    router.add_route("/admin/payments/:id/refunds", router::HttpMethod::GET,
        [refund_repo = std::move(refund_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                
                auto refunds = refund_repo.find_by({{"payment_id", id}});
                
                res.json({{"refunds", refunds}});
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