export module medusa_core.api.admin.order_edits;

import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;
import std;

export namespace medusa_core::api::admin {

void register_order_edit_routes(router::Router& router, database::Database& db) {
    using namespace entities;
    
    auto order_edit_repo = db.repository<OrderEdit>();
    auto order_repo = db.repository<Order>();
    auto line_item_repo = db.repository<LineItem>();
    auto shipping_method_repo = db.repository<ShippingMethod>();
    auto discount_repo = db.repository<Discount>();
    auto payment_collection_repo = db.repository<PaymentCollection>();
    
    // GET /admin/order-edits - List order edits
    router.add_route("/admin/order-edits", router::HttpMethod::GET,
        [order_edit_repo = std::move(order_edit_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto edits = order_edit_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<OrderEdit> paginated;
                auto start = std::min(offset, edits.size());
                auto end = std::min(start + limit, edits.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(edits[i]));
                }
                
                res.json({
                    {"order_edits", paginated},
                    {"count", edits.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /admin/order-edits/:id - Get order edit
    router.add_route("/admin/order-edits/:id", router::HttpMethod::GET,
        [order_edit_repo = std::move(order_edit_repo), order_repo = std::move(order_repo),
         line_item_repo = std::move(line_item_repo), shipping_method_repo = std::move(shipping_method_repo),
         discount_repo = std::move(discount_repo), payment_collection_repo = std::move(payment_collection_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto edit = order_edit_repo.find(id);
                
                if (!edit) {
                    res.status(404).json(http::ErrorResponse::not_found("Order edit not found"));
                    return;
                }
                
                // Fetch related entities
                auto order = order_repo.find(edit->get_order_id());
                auto items = line_item_repo.find_by({{"order_edit_id", id}});
                auto methods = shipping_method_repo.find_by({{"order_edit_id", id}});
                auto discounts = discount_repo.find_by({{"order_edit_id", id}});
                auto payment_collections = payment_collection_repo.find_by({{"order_edit_id", id}});
                
                res.json({
                    {"id", edit->get_id()},
                    {"order_id", edit->get_order_id()},
                    {"order", order},
                    {"status", edit->get_status()},
                    {"items", items},
                    {"shipping_methods", methods},
                    {"discounts", discounts},
                    {"payment_collections", payment_collections},
                    {"created_at", edit->get_created_at()},
                    {"updated_at", edit->get_updated_at()},
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/order-edits - Create order edit
    router.add_route("/admin/order-edits", router::HttpMethod::POST,
        [order_edit_repo = std::move(order_edit_repo), order_repo = std::move(order_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                OrderEdit edit;
                edit.add_id(generate_id());
                edit.add_order_id(req.body_as<std::string>().value_or(""));
                edit.add_status(std::string("pending"));
                
                edit = order_edit_repo.save(std::move(edit));
                
                res.status(201).json({{"order_edit", edit}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/order-edits/:id - Update order edit
    router.add_route("/admin/order-edits/:id", router::HttpMethod::POST,
        [order_edit_repo = std::move(order_edit_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto edit = order_edit_repo.find(id);
                
                if (!edit) {
                    res.status(404).json(http::ErrorResponse::not_found("Order edit not found"));
                    return;
                }
                
                if (auto status = req.body_as<std::string>()) {
                    edit->set_status(std::move(*status));
                }
                
                edit = order_edit_repo.save(std::move(*edit));
                
                res.json({{"order_edit", *edit}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/order-edits/:id/confirm - Confirm order edit
    router.add_route("/admin/order-edits/:id/confirm", router::HttpMethod::POST,
        [order_edit_repo = std::move(order_edit_repo), order_repo = std::move(order_repo),
         line_item_repo = std::move(line_item_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto edit = order_edit_repo.find(id);
                
                if (!edit) {
                    res.status(404).json(http::ErrorResponse::not_found("Order edit not found"));
                    return;
                }
                
                if (edit->get_status() != "pending") {
                    res.status(400).json(http::ErrorResponse::invalid_data("Order edit already confirmed"));
                    return;
                }
                
                edit->set_status(std::string("confirmed"));
                edit = order_edit_repo.save(std::move(*edit));
                
                // Apply changes to order
                // In real implementation, apply the edits to the order
                
                res.json({{"order_edit", *edit}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/order-edits/:id/cancel - Cancel order edit
    router.add_route("/admin/order-edits/:id/cancel", router::HttpMethod::POST,
        [order_edit_repo = std::move(order_edit_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto edit = order_edit_repo.find(id);
                
                if (!edit) {
                    res.status(404).json(http::ErrorResponse::not_found("Order edit not found"));
                    return;
                }
                
                if (edit->get_status() != "pending") {
                    res.status(400).json(http::ErrorResponse::invalid_data("Order edit cannot be cancelled"));
                    return;
                }
                
                edit->set_status(std::string("cancelled"));
                edit = order_edit_repo.save(std::move(*edit));
                
                res.json({{"order_edit", *edit}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /admin/order-edits/:id/items - Add item to order edit
    router.add_route("/admin/order-edits/:id/items", router::HttpMethod::POST,
        [order_edit_repo = std::move(order_edit_repo), line_item_repo = std::move(line_item_repo),
         variant_repo = std::move(variant_repo), product_repo = std::move(product_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                auto edit_id = req.get_param("id");
                auto edit = order_edit_repo.find(edit_id);
                
                if (!edit) {
                    res.status(404).json(http::ErrorResponse::not_found("Order edit not found"));
                    return;
                }
                
                // Parse variant_id, quantity from request body
                std::string variant_id = req.body_as<std::string>().value_or("");
                int64_t quantity = 1;
                
                auto variant = variant_repo.find(variant_id);
                if (!variant) {
                    res.status(404).json(http::ErrorResponse::not_found("Variant not found"));
                    return;
                }
                
                auto product = product_repo.find(variant->get_product_id());
                
                LineItem item;
                item.add_id(generate_id());
                item.add_order_edit_id(edit->get_id()); // Need to add this field
                item.add_variant_id(variant_id);
                item.add_title(variant->get_title());
                item.add_quantity(quantity);
                item.add_unit_price(variant->get_price());
                item.add_total(std::to_string(std::stoll(variant->get_price()) * quantity));
                
                res.status(201).json({{"item", item}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /admin/order-edits/:id - Delete order edit
    router.add_route("/admin/order-edits/:id", router::HttpMethod::DELETE,
        [order_edit_repo = std::move(order_edit_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto edit = order_edit_repo.find(id);
                
                if (!edit) {
                    res.status(404).json(http::ErrorResponse::not_found("Order edit not found"));
                    return;
                }
                
                order_edit_repo.remove(*edit);
                res.status(200).json({{"id", id}, {"object", "order_edit"}, {"deleted", true}});
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