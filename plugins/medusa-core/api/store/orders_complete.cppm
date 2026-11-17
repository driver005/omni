export module medusa_core.api.store.orders_complete;

import std;
import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;

export namespace medusa_core::api::store {

struct Order {
    std::string m_id{};
    std::string m_customer_id{};
    std::string m_email{};
    std::string m_region_id{};
    std::string m_currency_code{};
    std::string m_status{"pending"};
    std::string m_payment_status{"pending"};
    std::string m_fulfillment_status{"not_fulfilled"};
    std::string m_display_id{};
    std::string m_cart_id{};
    std::vector<LineItem> m_items{};
    std::vector<ShippingMethod> m_shipping_methods{};
    std::vector<PaymentCollection> m_payment_collections{};
    std::vector<Return> m_returns{};
    std::vector<Claim> m_claims{};
    std::vector<Exchange> m_exchanges{};
    Address m_shipping_address{};
    Address m_billing_address{};
    std::string m_subtotal{"0"};
    std::string m_tax_total{"0"};
    std::string m_shipping_total{"0"};
    std::string m_discount_total{"0"};
    std::string m_total{"0"};
    std::string m_refunded_total{"0"};
    std::map<std::string, std::string> m_metadata{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_completed_at{};
    std::optional<std::chrono::system_clock::time_point> m_canceled_at{};
    
    Order() noexcept = default;
    Order(std::string&& id) noexcept : m_id(std::move(id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_customer_id() noexcept { return m_customer_id; }
    const std::string& get_customer_id() const noexcept { return m_customer_id; }
    std::string& get_email() noexcept { return m_email; }
    const std::string& get_email() const noexcept { return m_email; }
    std::string& get_region_id() noexcept { return m_region_id; }
    const std::string& get_region_id() const noexcept { return m_region_id; }
    std::string& get_currency_code() noexcept { return m_currency_code; }
    const std::string& get_currency_code() const noexcept { return m_currency_code; }
    std::string& get_status() noexcept { return m_status; }
    const std::string& get_status() const noexcept { return m_status; }
    std::string& get_payment_status() noexcept { return m_payment_status; }
    const std::string& get_payment_status() const noexcept { return m_payment_status; }
    std::string& get_fulfillment_status() noexcept { return m_fulfillment_status; }
    const std::string& get_fulfillment_status() const noexcept { return m_fulfillment_status; }
    std::string& get_display_id() noexcept { return m_display_id; }
    const std::string& get_display_id() const noexcept { return m_display_id; }
    std::string& get_cart_id() noexcept { return m_cart_id; }
    const std::string& get_cart_id() const noexcept { return m_cart_id; }
    std::vector<LineItem>& get_items() noexcept { return m_items; }
    const std::vector<LineItem>& get_items() const noexcept { return m_items; }
    std::vector<ShippingMethod>& get_shipping_methods() noexcept { return m_shipping_methods; }
    const std::vector<ShippingMethod>& get_shipping_methods() const noexcept { return m_shipping_methods; }
    Address& get_shipping_address() noexcept { return m_shipping_address; }
    const Address& get_shipping_address() const noexcept { return m_shipping_address; }
    Address& get_billing_address() noexcept { return m_billing_address; }
    const Address& get_billing_address() const noexcept { return m_billing_address; }
    std::string& get_subtotal() noexcept { return m_subtotal; }
    const std::string& get_subtotal() const noexcept { return m_subtotal; }
    std::string& get_tax_total() noexcept { return m_tax_total; }
    const std::string& get_tax_total() const noexcept { return m_tax_total; }
    std::string& get_shipping_total() noexcept { return m_shipping_total; }
    const std::string& get_shipping_total() const noexcept { return m_shipping_total; }
    std::string& get_discount_total() noexcept { return m_discount_total; }
    const std::string& get_discount_total() const noexcept { return m_discount_total; }
    std::string& get_total() noexcept { return m_total; }
    const std::string& get_total() const noexcept { return m_total; }
    std::string& get_refunded_total() noexcept { return m_refunded_total; }
    const std::string& get_refunded_total() const noexcept { return m_refunded_total; }
    std::map<std::string, std::string>& get_metadata() noexcept { return m_metadata; }
    const std::map<std::string, std::string>& get_metadata() const noexcept { return m_metadata; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_completed_at() noexcept { return m_completed_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_completed_at() const noexcept { return m_completed_at; }
    std::optional<std::chrono::system_clock::time_point>& get_canceled_at() noexcept { return m_canceled_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_canceled_at() const noexcept { return m_canceled_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_customer_id(std::string&& id) noexcept { m_customer_id = std::move(id); }
    void set_email(std::string&& email) noexcept { m_email = std::move(email); }
    void set_region_id(std::string&& id) noexcept { m_region_id = std::move(id); }
    void set_currency_code(std::string&& code) noexcept { m_currency_code = std::move(code); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_payment_status(std::string&& status) noexcept { m_payment_status = std::move(status); }
    void set_fulfillment_status(std::string&& status) noexcept { m_fulfillment_status = std::move(status); }
    void set_display_id(std::string&& id) noexcept { m_display_id = std::move(id); }
    void set_cart_id(std::string&& id) noexcept { m_cart_id = std::move(id); }
    void set_items(std::vector<LineItem>&& items) noexcept { m_items = std::move(items); }
    void set_shipping_methods(std::vector<ShippingMethod>&& methods) noexcept { m_shipping_methods = std::move(methods); }
    void set_shipping_address(Address&& addr) noexcept { m_shipping_address = std::move(addr); }
    void set_billing_address(Address&& addr) noexcept { m_billing_address = std::move(addr); }
    void set_subtotal(std::string&& val) noexcept { m_subtotal = std::move(val); }
    void set_tax_total(std::string&& val) noexcept { m_tax_total = std::move(val); }
    void set_shipping_total(std::string&& val) noexcept { m_shipping_total = std::move(val); }
    void set_discount_total(std::string&& val) noexcept { m_discount_total = std::move(val); }
    void set_total(std::string&& val) noexcept { m_total = std::move(val); }
    void set_refunded_total(std::string&& val) noexcept { m_refunded_total = std::move(val); }
    void set_metadata(std::map<std::string, std::string>&& meta) noexcept { m_metadata = std::move(meta); }
    void set_created_at(std::chrono::system_clock::time_point&& t) noexcept { m_created_at = std::move(t); }
    void set_updated_at(std::chrono::system_clock::time_point&& t) noexcept { m_updated_at = std::move(t); }
    void set_completed_at(std::optional<std::chrono::system_clock::time_point>&& t) noexcept { m_completed_at = std::move(t); }
    void set_canceled_at(std::optional<std::chrono::system_clock::time_point>&& t) noexcept { m_canceled_at = std::move(t); }
};

struct Return {
    std::string m_id{};
    std::string m_order_id{};
    std::string m_status{"requested"};
    std::string m_reason{};
    std::vector<ReturnItem> m_items{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Return() noexcept = default;
    Return(std::string&& id, std::string&& order_id) noexcept : m_id(std::move(id)), m_order_id(std::move(order_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_order_id() noexcept { return m_order_id; }
    const std::string& get_order_id() const noexcept { return m_order_id; }
    std::string& get_status() noexcept { return m_status; }
    const std::string& get_status() const noexcept { return m_status; }
    std::string& get_reason() noexcept { return m_reason; }
    const std::string& get_reason() const noexcept { return m_reason; }
    std::vector<ReturnItem>& get_items() noexcept { return m_items; }
    const std::vector<ReturnItem>& get_items() const noexcept { return m_items; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_order_id(std::string&& id) noexcept { m_order_id = std::move(id); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_reason(std::string&& reason) noexcept { m_reason = std::move(reason); }
    void set_items(std::vector<ReturnItem>&& items) noexcept { m_items = std::move(items); }
    void set_created_at(std::chrono::system_clock::time_point&& t) noexcept { m_created_at = std::move(t); }
    void set_updated_at(std::chrono::system_clock::time_point&& t) noexcept { m_updated_at = std::move(t); }
};

struct ReturnItem {
    std::string m_id{};
    std::string m_return_id{};
    std::string m_line_item_id{};
    int64_t m_quantity{1};
    std::string m_reason{};
    std::string m_note{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    ReturnItem() noexcept = default;
    ReturnItem(std::string&& id, std::string&& return_id, std::string&& line_item_id) noexcept
        : m_id(std::move(id)), m_return_id(std::move(return_id)), m_line_item_id(std::move(line_item_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_return_id() noexcept { return m_return_id; }
    const std::string& get_return_id() const noexcept { return m_return_id; }
    std::string& get_line_item_id() noexcept { return m_line_item_id; }
    const std::string& get_line_item_id() const noexcept { return m_line_item_id; }
    int64_t& get_quantity() noexcept { return m_quantity; }
    const int64_t& get_quantity() const noexcept { return m_quantity; }
    std::string& get_reason() noexcept { return m_reason; }
    const std::string& get_reason() const noexcept { return m_reason; }
    std::string& get_note() noexcept { return m_note; }
    const std::string& get_note() const noexcept { return m_note; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_return_id(std::string&& id) noexcept { m_return_id = std::move(id); }
    void set_line_item_id(std::string&& id) noexcept { m_line_item_id = std::move(id); }
    void set_quantity(int64_t qty) noexcept { m_quantity = qty; }
    void set_reason(std::string&& reason) noexcept { m_reason = std::move(reason); }
    void set_note(std::string&& note) noexcept { m_note = std::move(note); }
    void set_created_at(std::chrono::system_clock::time_point&& t) noexcept { m_created_at = std::move(t); }
    void set_updated_at(std::chrono::system_clock::time_point&& t) noexcept { m_updated_at = std::move(t); }
};

struct Claim {
    std::string m_id{};
    std::string m_order_id{};
    std::string m_type{"refund"};
    std::string m_status{"pending"};
    std::vector<ClaimItem> m_items{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Claim() noexcept = default;
    Claim(std::string&& id, std::string&& order_id) noexcept : m_id(std::move(id)), m_order_id(std::move(order_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_order_id() noexcept { return m_order_id; }
    const std::string& get_order_id() const noexcept { return m_order_id; }
    std::string& get_type() noexcept { return m_type; }
    const std::string& get_type() const noexcept { return m_type; }
    std::string& get_status() noexcept { return m_status; }
    const std::string& get_status() const noexcept { return m_status; }
    std::vector<ClaimItem>& get_items() noexcept { return m_items; }
    const std::vector<ClaimItem>& get_items() const noexcept { return m_items; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_order_id(std::string&& id) noexcept { m_order_id = std::move(id); }
    void set_type(std::string&& type) noexcept { m_type = std::move(type); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_items(std::vector<ClaimItem>&& items) noexcept { m_items = std::move(items); }
    void set_created_at(std::chrono::system_clock::time_point&& t) noexcept { m_created_at = std::move(t); }
    void set_updated_at(std::chrono::system_clock::time_point&& t) noexcept { m_updated_at = std::move(t); }
};

struct ClaimItem {
    std::string m_id{};
    std::string m_claim_id{};
    std::string m_line_item_id{};
    int64_t m_quantity{1};
    std::string m_reason{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    ClaimItem() noexcept = default;
    ClaimItem(std::string&& id, std::string&& claim_id, std::string&& line_item_id) noexcept
        : m_id(std::move(id)), m_claim_id(std::move(claim_id)), m_line_item_id(std::move(line_item_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_claim_id() noexcept { return m_claim_id; }
    const std::string& get_claim_id() const noexcept { return m_claim_id; }
    std::string& get_line_item_id() noexcept { return m_line_item_id; }
    const std::string& get_line_item_id() const noexcept { return m_line_item_id; }
    int64_t& get_quantity() noexcept { return m_quantity; }
    const int64_t& get_quantity() const noexcept { return m_quantity; }
    std::string& get_reason() noexcept { return m_reason; }
    const std::string& get_reason() const noexcept { return m_reason; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_claim_id(std::string&& id) noexcept { m_claim_id = std::move(id); }
    void set_line_item_id(std::string&& id) noexcept { m_line_item_id = std::move(id); }
    void set_quantity(int64_t qty) noexcept { m_quantity = qty; }
    void set_reason(std::string&& reason) noexcept { m_reason = std::move(reason); }
    void set_created_at(std::chrono::system_clock::time_point&& t) noexcept { m_created_at = std::move(t); }
    void set_updated_at(std::chrono::system_clock::time_point&& t) noexcept { m_updated_at = std::move(t); }
};

struct Exchange {
    std::string m_id{};
    std::string m_order_id{};
    std::string m_status{"pending"};
    std::vector<ExchangeItem> m_items{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Exchange() noexcept = default;
    Exchange(std::string&& id, std::string&& order_id) noexcept : m_id(std::move(id)), m_order_id(std::move(order_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_order_id() noexcept { return m_order_id; }
    const std::string& get_order_id() const noexcept { return m_order_id; }
    std::string& get_status() noexcept { return m_status; }
    const std::string& get_status() const noexcept { return m_status; }
    std::vector<ExchangeItem>& get_items() noexcept { return m_items; }
    const std::vector<ExchangeItem>& get_items() const noexcept { return m_items; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_order_id(std::string&& id) noexcept { m_order_id = std::move(id); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_items(std::vector<ExchangeItem>&& items) noexcept { m_items = std::move(items); }
    void set_created_at(std::chrono::system_clock::time_point&& t) noexcept { m_created_at = std::move(t); }
    void set_updated_at(std::chrono::system_clock::time_point&& t) noexcept { m_updated_at = std::move(t); }
};

struct ExchangeItem {
    std::string m_id{};
    std::string m_exchange_id{};
    std::string m_line_item_id{};
    int64_t m_quantity{1};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    ExchangeItem() noexcept = default;
    ExchangeItem(std::string&& id, std::string&& exchange_id, std::string&& line_item_id) noexcept
        : m_id(std::move(id)), m_exchange_id(std::move(exchange_id)), m_line_item_id(std::move(line_item_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_exchange_id() noexcept { return m_exchange_id; }
    const std::string& get_exchange_id() const noexcept { return m_exchange_id; }
    std::string& get_line_item_id() noexcept { return m_line_item_id; }
    const std::string& get_line_item_id() const noexcept { return m_line_item_id; }
    int64_t& get_quantity() noexcept { return m_quantity; }
    const int64_t& get_quantity() const noexcept { return m_quantity; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_exchange_id(std::string&& id) noexcept { m_exchange_id = std::move(id); }
    void set_line_item_id(std::string&& id) noexcept { m_line_item_id = std::move(id); }
    void set_quantity(int64_t qty) noexcept { m_quantity = qty; }
    void set_created_at(std::chrono::system_clock::time_point&& t) noexcept { m_created_at = std::move(t); }
    void set_updated_at(std::chrono::system_clock::time_point&& t) noexcept { m_updated_at = std::move(t); }
};

struct PaymentCollection {
    std::string m_id{};
    std::string m_region_id{};
    std::string m_currency_code{};
    std::string m_status{"pending"};
    std::vector<PaymentSession> m_payment_sessions{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    PaymentCollection() noexcept = default;
    PaymentCollection(std::string&& id) noexcept : m_id(std::move(id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_region_id() noexcept { return m_region_id; }
    const std::string& get_region_id() const noexcept { return m_region_id; }
    std::string& get_currency_code() noexcept { return m_currency_code; }
    const std::string& get_currency_code() const noexcept { return m_currency_code; }
    std::string& get_status() noexcept { return m_status; }
    const std::string& get_status() const noexcept { return m_status; }
    std::vector<PaymentSession>& get_payment_sessions() noexcept { return m_payment_sessions; }
    const std::vector<PaymentSession>& get_payment_sessions() const noexcept { return m_payment_sessions; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_region_id(std::string&& id) noexcept { m_region_id = std::move(id); }
    void set_currency_code(std::string&& code) noexcept { m_currency_code = std::move(code); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_payment_sessions(std::vector<PaymentSession>&& sessions) noexcept { m_payment_sessions = std::move(sessions); }
    void set_created_at(std::chrono::system_clock::time_point&& t) noexcept { m_created_at = std::move(t); }
    void set_updated_at(std::chrono::system_clock::time_point&& t) noexcept { m_updated_at = std::move(t); }
};

struct PaymentSession {
    std::string m_id{};
    std::string m_payment_collection_id{};
    std::string m_provider_id{};
    std::string m_status{"pending"};
    std::map<std::string, std::string> m_data{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    PaymentSession() noexcept = default;
    PaymentSession(std::string&& id, std::string&& pc_id, std::string&& provider) noexcept
        : m_id(std::move(id)), m_payment_collection_id(std::move(pc_id)), m_provider_id(std::move(provider)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_payment_collection_id() noexcept { return m_payment_collection_id; }
    const std::string& get_payment_collection_id() const noexcept { return m_payment_collection_id; }
    std::string& get_provider_id() noexcept { return m_provider_id; }
    const std::string& get_provider_id() const noexcept { return m_provider_id; }
    std::string& get_status() noexcept { return m_status; }
    const std::string& get_status() const noexcept { return m_status; }
    std::map<std::string, std::string>& get_data() noexcept { return m_data; }
    const std::map<std::string, std::string>& get_data() const noexcept { return m_data; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_payment_collection_id(std::string&& id) noexcept { m_payment_collection_id = std::move(id); }
    void set_provider_id(std::string&& id) noexcept { m_provider_id = std::move(id); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_data(std::map<std::string, std::string>&& data) noexcept { m_data = std::move(data); }
    void set_created_at(std::chrono::system_clock::time_point&& t) noexcept { m_created_at = std::move(t); }
    void set_updated_at(std::chrono::system_clock::time_point&& t) noexcept { m_updated_at = std::move(t); }
};

// In-memory repositories for store orders
class OrderRepository {
    std::map<std::string, Order> m_orders{};
    
public:
    std::optional<Order> find(const std::string& id) const noexcept {
        auto it = m_orders.find(id);
        if (it != m_orders.end()) return it->second;
        return std::nullopt;
    }
    
    std::vector<Order> find_by(const std::map<std::string, std::variant<int64_t, double, bool, std::string>>& criteria) const noexcept {
        std::vector<Order> results;
        for (const auto& [id, order] : m_orders) {
            bool match = true;
            for (const auto& [key, value] : criteria) {
                if (key == "customer_id" && order.get_customer_id() != std::get<std::string>(value)) {
                    match = false; break;
                }
            }
            if (match) results.push_back(order);
        }
        return results;
    }
    
    Order save(Order order) noexcept {
        m_orders[order.get_id()] = order;
        return order;
    }
    
    void remove(const std::string& id) noexcept {
        m_orders.erase(id);
    }
};

class ReturnRepository {
    std::map<std::string, Return> m_returns{};
    
public:
    std::optional<Return> find(const std::string& id) const noexcept {
        auto it = m_returns.find(id);
        if (it != m_returns.end()) return it->second;
        return std::nullopt;
    }
    
    Return save(Return ret) noexcept {
        m_returns[ret.get_id()] = ret;
        return ret;
    }
};

class ClaimRepository {
    std::map<std::string, Claim> m_claims{};
    
public:
    std::optional<Claim> find(const std::string& id) const noexcept {
        auto it = m_claims.find(id);
        if (it != m_claims.end()) return it->second;
        return std::nullopt;
    }
    
    Claim save(Claim claim) noexcept {
        m_claims[claim.get_id()] = claim;
        return claim;
    }
};

class ExchangeRepository {
    std::map<std::string, Exchange> m_exchanges{};
    
public:
    std::optional<Exchange> find(const std::string& id) const noexcept {
        auto it = m_exchanges.find(id);
        if (it != m_exchanges.end()) return it->second;
        return std::nullopt;
    }
    
    Exchange save(Exchange ex) noexcept {
        m_exchanges[ex.get_id()] = ex;
        return ex;
    }
};

void register_complete_order_routes(router::Router& router, database::Database& db) {
    OrderRepository order_repo;
    ReturnRepository return_repo;
    ClaimRepository claim_repo;
    ExchangeRepository exchange_repo;
    auto customer_repo = db.repository<entities::Customer>();
    auto line_item_repo = db.repository<entities::LineItem>();
    auto variant_repo = db.repository<entities::ProductVariant>();
    
    // GET /store/orders - List customer's orders
    router.add_route("/store/orders", router::HttpMethod::GET,
        [order_repo = std::move(order_repo), customer_repo = std::move(customer_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                
                std::map<std::string, std::variant<int64_t, double, bool, std::string>> filters;
                filters["customer_id"] = customer_id;
                
                // Add status filter if provided
                for (const auto& [key, values] : req.query()) {
                    if (!values.empty() && key != "limit" && key != "offset" && key != "order" && key != "fields") {
                        filters[key] = values.front();
                    }
                }
                
                auto orders = order_repo.find_by(filters);
                
                // Apply pagination
                size_t limit = 50;
                size_t offset = 0;
                if (auto l = req.get_query("limit")) {
                    limit = std::stoull(*l);
                }
                if (auto o = req.get_query("offset")) {
                    offset = std::stoull(*o);
                }
                
                std::vector<Order> paginated;
                auto start = std::min(offset, orders.size());
                auto end = std::min(start + limit, orders.size());
                for (size_t i = start; i < end; ++i) {
                    paginated.push_back(std::move(orders[i]));
                }
                
                // Filter out pivot fields (customer.orders)
                // In real implementation, handle fields query param
                
                res.json({
                    {"orders", paginated},
                    {"count", orders.size()},
                    {"offset", offset},
                    {"limit", limit}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /store/orders/:id - Get order
    router.add_route("/store/orders/:id", router::HttpMethod::GET,
        [order_repo = std::move(order_repo), line_item_repo = std::move(line_item_repo),
         variant_repo = std::move(variant_repo), customer_repo = std::move(customer_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto order = order_repo.find(id);
                
                if (!order) {
                    res.status(404).json(http::ErrorResponse::not_found("Order not found"));
                    return;
                }
                
                // Verify ownership
                if (order->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized to view this order"));
                    return;
                }
                
                // Fetch related entities
                auto items = line_item_repo.find_by({{"order_id", id}});
                
                res.json({
                    {"order", *order}
                });
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/orders/:id/cancel - Cancel order
    router.add_route("/store/orders/:id/cancel", router::HttpMethod::POST,
        [order_repo = std::move(order_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto order = order_repo.find(id);
                
                if (!order) {
                    res.status(404).json(http::ErrorResponse::not_found("Order not found"));
                    return;
                }
                
                if (order->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                if (order->get_status() == "cancelled") {
                    res.status(400).json(http::ErrorResponse::invalid_data("Order already cancelled"));
                    return;
                }
                
                order->set_status(std::string("cancelled"));
                order->set_canceled_at(std::chrono::system_clock::now());
                order = order_repo.save(std::move(*order));
                
                res.json({{"order", *order}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /store/orders/:id/returns - Get order returns
    router.add_route("/store/orders/:id/returns", router::HttpMethod::GET,
        [order_repo = std::move(order_repo), return_repo = std::move(return_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto order = order_repo.find(id);
                
                if (!order) {
                    res.status(404).json(http::ErrorResponse::not_found("Order not found"));
                    return;
                }
                
                if (order->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                // In real implementation, fetch returns for order
                res.json({{"returns", std::vector<Return>{}}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/orders/:id/returns - Create return
    router.add_route("/store/orders/:id/returns", router::HttpMethod::POST,
        [order_repo = std::move(order_repo), return_repo = std::move(return_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto order = order_repo.find(id);
                
                if (!order) {
                    res.status(404).json(http::ErrorResponse::not_found("Order not found"));
                    return;
                }
                
                if (order->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                // Parse return items
                // In real implementation, validate items belong to order
                
                Return ret(generate_id(), id);
                ret.set_status(std::string("requested"));
                ret.set_reason(req.body_as<std::string>().value_or(""));
                ret.set_created_at(std::chrono::system_clock::now());
                ret.set_updated_at(std::chrono::system_clock::now());
                
                ret = return_repo.save(std::move(ret));
                
                res.status(201).json({{"return", ret}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /store/orders/:id/claims - Get order claims
    router.add_route("/store/orders/:id/claims", router::HttpMethod::GET,
        [order_repo = std::move(order_repo), claim_repo = std::move(claim_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto order = order_repo.find(id);
                
                if (!order) {
                    res.status(404).json(http::ErrorResponse::not_found("Order not found"));
                    return;
                }
                
                if (order->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                res.json({{"claims", std::vector<Claim>{}}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/orders/:id/claims - Create claim
    router.add_route("/store/orders/:id/claims", router::HttpMethod::POST,
        [order_repo = std::move(order_repo), claim_repo = std::move(claim_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto order = order_repo.find(id);
                
                if (!order) {
                    res.status(404).json(http::ErrorResponse::not_found("Order not found"));
                    return;
                }
                
                if (order->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                Claim claim(generate_id(), id);
                claim.set_type(req.body_as<std::string>().value_or("refund"));
                claim.set_status(std::string("pending"));
                claim.set_created_at(std::chrono::system_clock::now());
                claim.set_updated_at(std::chrono::system_clock::now());
                
                claim = claim_repo.save(std::move(claim));
                
                res.status(201).json({{"claim", claim}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /store/orders/:id/exchanges - Get order exchanges
    router.add_route("/store/orders/:id/exchanges", router::HttpMethod::GET,
        [order_repo = std::move(order_repo), exchange_repo = std::move(exchange_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto order = order_repo.find(id);
                
                if (!order) {
                    res.status(404).json(http::ErrorResponse::not_found("Order not found"));
                    return;
                }
                
                if (order->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                res.json({{"exchanges", std::vector<Exchange>{}}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/orders/:id/exchanges - Create exchange
    router.add_route("/store/orders/:id/exchanges", router::HttpMethod::POST,
        [order_repo = std::move(order_repo), exchange_repo = std::move(exchange_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto order = order_repo.find(id);
                
                if (!order) {
                    res.status(404).json(http::ErrorResponse::not_found("Order not found"));
                    return;
                }
                
                if (order->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                Exchange ex(generate_id(), id);
                ex.set_status(std::string("pending"));
                ex.set_created_at(std::chrono::system_clock::now());
                ex.set_updated_at(std::chrono::system_clock::now());
                
                ex = exchange_repo.save(std::move(ex));
                
                res.status(201).json({{"exchange", ex}});
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