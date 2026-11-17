export module medusa_core.api.routes;

import medusa_core.api.admin.orders;
import medusa_core.api.admin.products;
import medusa_core.api.admin.customers;
import medusa_core.api.admin.return_reasons;
import medusa_core.api.admin.payment_collections;
import medusa_core.api.admin.draft_orders;
import medusa_core.api.admin.order_edits;
import medusa_core.api.admin.promotions;
import medusa_core.api.admin.price_lists;
import medusa_core.api.admin.regions;
import medusa_core.api.admin.customer_groups;
import medusa_core.api.admin.shipping_profiles;
import medusa_core.api.admin.stock_locations;
import medusa_core.api.admin.fulfillment;
import medusa_core.api.admin.inventory;
import medusa_core.api.admin.sales_channels;
import medusa_core.api.admin.users;
import medusa_core.api.admin.product_variants;
import medusa_core.api.admin.shipping_options;
import medusa_core.api.admin.payments;
import medusa_core.api.admin.tax_regions;
import medusa_core.api.admin.currencies;
import medusa_core.api.store.orders;
import medusa_core.api.store.orders_complete;
import medusa_core.api.store.customers;
import medusa_core.api.store.carts_complete;
import medusa_core.api.store.products;
import medusa_core.api.store.categories;
import medusa_core.api.store.claims;
import medusa_core.api.store.returns;
import medusa_core.api.store.exchanges;
import medusa_core.api.auth.routes;
import router.router;
import database.connection;
import database.types;

export namespace medusa_core::api {

void register_admin_routes(router::Router& router, database::Database& db) {
    admin::register_order_routes(router, db);
    admin::register_product_routes(router, db);
    admin::register_customer_routes(router, db);
    admin::register_return_reason_routes(router, db);
    admin::register_payment_collection_routes(router, db);
    admin::register_draft_order_routes(router, db);
    admin::register_order_edit_routes(router, db);
    admin::register_promotion_routes(router, db);
    admin::register_price_list_routes(router, db);
    admin::register_region_routes(router, db);
    admin::register_customer_group_routes(router, db);
    admin::register_shipping_profile_routes(router, db);
    admin::register_stock_location_routes(router, db);
    admin::register_fulfillment_routes(router, db);
    admin::register_inventory_routes(router, db);
    admin::register_sales_channel_routes(router, db);
    admin::register_user_routes(router, db);
    admin::register_product_variant_routes(router, db);
    admin::register_shipping_option_routes(router, db);
    admin::register_payment_routes(router, db);
    admin::register_tax_region_routes(router, db);
    admin::register_currency_routes(router, db);
    // Add more admin routes as needed
}

void register_store_routes(router::Router& router, database::Database& db) {
    store::register_order_routes(router, db);
    store::register_complete_order_routes(router, db);
    store::register_customer_routes(router, db);
    store::register_complete_cart_routes(router, db);
    store::register_product_routes(router, db);
    store::register_category_routes(router, db);
    store::register_claim_routes(router, db);
    store::register_return_routes(router, db);
    store::register_exchange_routes(router, db);
    // Add more store routes as needed
}

void register_auth_routes(router::Router& router, database::Database& db) {
    auth::register_auth_routes(router, db);
}

void register_all_routes(router::Router& router, database::Database& db) {
    // Register middleware first
    router.use(router::middleware::logger());
    router.use(router::middleware::request_id());
    router.use(router::middleware::body_parser());
    router.use(router::middleware::cors({
        "http://localhost:9000",
        "http://localhost:7000",
        "http://localhost:3000"
    }));
    
    // Admin routes with authentication
    auto admin_router = router.group("/admin");
    admin_router->use(router::middleware::authenticate("user", {"bearer", "session", "api-key"}));
    register_admin_routes(*admin_router, db);
    
    // Store routes with publishable key
    auto store_router = router.group("/store");
    store_router->use(router::middleware::authenticate("customer", {"bearer", "session"}));
    register_store_routes(*store_router, db);
    
    // Auth routes (no auth required)
    auto auth_router = router.group("/auth");
    register_auth_routes(*auth_router, db);
    
    // Health check
    router.add_route("/health", router::HttpMethod::GET,
        [](const http::Request& req, http::Response& res) {
            res.json({{"status", "ok"}});
        });
}

} // namespace medusa_core::api