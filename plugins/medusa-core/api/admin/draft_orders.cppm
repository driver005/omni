export module medusa_core.api.admin.draft_orders;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_draft_order_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto draft_order_repo = db.repository<DraftOrder>();
    auto order_repo = db.repository<Order>();
    auto customer_repo = db.repository<Customer>();
    auto region_repo = db.repository<Region>();
    auto line_item_repo = db.repository<LineItem>();
    auto shipping_method_repo = db.repository<ShippingMethod>();
    auto discount_repo = db.repository<Discount>();
    auto promotion_repo = db.repository<Promotion>();
    
    // GET /admin/draft-orders - List draft orders
    router.add_route("/admin/draft-orders", router::HttpMethod::GET,
        [draft_order_repo = std::move(draft_order_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto drafts = draft_order_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<DraftOrder> paginated;
                auto start = std::min(offset, drafts.size());
                auto end = std::min(start + limit, drafts.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(drafts[i]));
                }
                
                res.json({
                    {"draft_orders", paginated},
                    {"count", drafts.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/draft-orders/:id - Get draft order
    router.add_route("/admin/draft-orders/:id", router::HttpMethod::GET,
        [draft_order_repo = std::move(draft_order_repo), line_item_repo = std::move(line_item_repo),
         shipping_method_repo = std::move(shipping_method_repo), discount_repo = std::move(discount_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto draft = draft_order_repo.find(id);
                
                if (!draft) {
                    res.status(404).json(http::ErrorResponse::not_found("Draft order not found"));
                    return;
                }
                
                auto items = line_item_repo.find_by({{"draft_order_id", id}});
                auto methods = shipping_method_repo.find_by({{"draft_order_id", id}});
                auto discounts = discount_repo.find_by({{"draft_order_id", id}});
                
                res.json({
                    {"id", draft->get_id()},
                    {"customer_id", draft->get_customer_id()},
                    {"email", draft->get_email()},
                    {"status", draft->get_status()},
                    {"items", items},
                    {"shipping_methods", methods},
                    {"discounts", discounts},
                    {"created_at", draft->get_created_at()},
                    {"updated_at", draft->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/draft-orders - Create draft order
    router.add_route("/admin/draft-orders", router::HttpMethod::POST,
        [draft_order_repo = std::move(draft_order_repo), customer_repo = std::move(customer_repo),
         region_repo = std::move(region_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                DraftOrder draft;
                draft.add_id(generate_id());
                draft.add_customer_id(req.body_as<std::string>().value_or(""));
                draft.add_region_id(req.body_as<std::string>().value_or(""));
                draft.add_currency_code(req.body_as<std::string>().value_or("usd"));
                draft.add_status(std::string("open"));
                
                draft = draft_order_repo.save(std::move(draft));
                
                res.status(201).json({{"draft_order", draft}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/draft-orders/:id - Update draft order
    router.add_route("/admin/draft-orders/:id", router::HttpMethod::POST,
        [draft_order_repo = std::move(draft_order_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto draft = draft_order_repo.find(id);
                
                if (!draft) {
                    res.status(404).json(http::ErrorResponse::not_found("Draft order not found"));
                    return;
                }
                
                if (auto status = req.body_as<std::string>()) {
                    draft->set_status(std::move(*status));
                }
                
                draft = draft_order_repo.save(std::move(*draft));
                
                res.json({{"draft_order", *draft}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/draft-orders/:id/complete - Complete draft order (convert to order)
    router.add_route("/admin/draft-orders/:id/complete", router::HttpMethod::POST,
        [draft_order_repo = std::move(draft_order_repo), order_repo = std::move(order_repo),
         line_item_repo = std::move(line_item_repo), customer_repo = std::move(customer_repo),
         region_repo = std::move(region_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto draft = draft_order_repo.find(id);
                
                if (!draft) {
                    res.status(404).json(http::ErrorResponse::not_found("Draft order not found"));
                    return;
                }
                
                // Convert draft order to order
                Order order;
                order.add_id(generate_id());
                order.add_customer_id(draft->get_customer_id());
                order.add_region_id(draft->get_region_id());
                order.add_currency_code(draft->get_currency_code());
                order.add_status(std::string("pending"));
                order.add_payment_status(std::string("pending"));
                order.add_fulfillment_status(std::string("not_fulfilled"));
                order.add_total(std::string("0"));
                order.add_subtotal(std::string("0"));
                order.add_tax_total(std::string("0"));
                order.add_shipping_total(std::string("0"));
                order.add_discount_total(std::string("0"));
                
                order = order_repo.save(std::move(order));
                
                // Mark draft as completed
                draft->set_status(std::string("completed"));
                draft = draft_order_repo.save(std::move(*draft));
                
                res.json({{"draft_order", *draft}, {"order_id", order.get_id()}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/draft-orders/:id - Delete draft order
    router.add_route("/admin/draft-orders/:id", router::HttpMethod::DELETE,
        [draft_order_repo = std::move(draft_order_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto draft = draft_order_repo.find(id);
                
                if (!draft) {
                    res.status(404).json(http::ErrorResponse::not_found("Draft order not found"));
                    return;
                }
                
                draft_order_repo.remove(*draft);
                res.status(200).json({{"id", id}, {"object", "draft_order"}, {"deleted", true}});
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