export module medusa_core.api.store.carts_complete;

import std;
import database.connection;
import database.types;
import medusa_core.database.entities;
import medusa_core.database.entities_extended;
import router.router;
import http.types;

export namespace medusa_core::api::store {

struct Cart {
    std::string m_id{};
    std::string m_customer_id{};
    std::string m_email{};
    std::string m_region_id{};
    std::string m_currency_code{"usd"};
    std::string m_status{"open"};
    std::string m_sales_channel_id{};
    std::string m_shipping_address_id{};
    std::string m_billing_address_id{};
    std::vector<LineItem> m_items{};
    std::vector<ShippingMethod> m_shipping_methods{};
    std::vector<Discount> m_discounts{};
    std::vector<CreditLine> m_credit_lines{};
    std::string m_item_subtotal{"0"};
    std::string m_item_tax_total{"0"};
    std::string m_item_total{"0"};
    std::string m_shipping_subtotal{"0"};
    std::string m_shipping_tax_total{"0"};
    std::string m_shipping_total{"0"};
    std::string m_tax_total{"0"};
    std::string m_discount_total{"0"};
    std::string m_subtotal{"0"};
    std::string m_total{"0"};
    std::string m_promo_total{"0"};
    std::string m_original_item_subtotal{"0"};
    std::string m_original_item_tax_total{"0"};
    std::string m_original_item_total{"0"};
    std::string m_original_shipping_subtotal{"0"};
    std::string m_original_shipping_tax_total{"0"};
    std::string m_original_shipping_total{"0"};
    std::string m_original_tax_total{"0"};
    std::string m_original_total{"0"};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_completed_at{};
    std::map<std::string, std::string> m_metadata{};
    std::string m_context{};
    std::string m_idempotency_key{};
    
    Cart() noexcept = default;
    Cart(std::string&& id) noexcept : m_id(std::move(id)) {}
    
    // Getters
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
    std::string& get_sales_channel_id() noexcept { return m_sales_channel_id; }
    const std::string& get_sales_channel_id() const noexcept { return m_sales_channel_id; }
    std::vector<LineItem>& get_items() noexcept { return m_items; }
    const std::vector<LineItem>& get_items() const noexcept { return m_items; }
    std::vector<ShippingMethod>& get_shipping_methods() noexcept { return m_shipping_methods; }
    const std::vector<ShippingMethod>& get_shipping_methods() const noexcept { return m_shipping_methods; }
    std::vector<Discount>& get_discounts() noexcept { return m_discounts; }
    const std::vector<Discount>& get_discounts() const noexcept { return m_discounts; }
    std::vector<CreditLine>& get_credit_lines() noexcept { return m_credit_lines; }
    const std::vector<CreditLine>& get_credit_lines() const noexcept { return m_credit_lines; }
    std::map<std::string, std::string>& get_metadata() noexcept { return m_metadata; }
    const std::map<std::string, std::string>& get_metadata() const noexcept { return m_metadata; }
    std::string& get_context() noexcept { return m_context; }
    const std::string& get_context() const noexcept { return m_context; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    // Setters
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_customer_id(std::string&& id) noexcept { m_customer_id = std::move(id); }
    void set_email(std::string&& email) noexcept { m_email = std::move(email); }
    void set_region_id(std::string&& id) noexcept { m_region_id = std::move(id); }
    void set_currency_code(std::string&& code) noexcept { m_currency_code = std::move(code); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_sales_channel_id(std::string&& id) noexcept { m_sales_channel_id = std::move(id); }
    void set_items(std::vector<LineItem>&& items) noexcept { m_items = std::move(items); }
    void set_shipping_methods(std::vector<ShippingMethod>&& methods) noexcept { m_shipping_methods = std::move(methods); }
    void set_discounts(std::vector<Discount>&& discounts) noexcept { m_discounts = std::move(discounts); }
    void set_credit_lines(std::vector<CreditLine>&& lines) noexcept { m_credit_lines = std::move(lines); }
    void set_created_at(std::chrono::system_clock::time_point&& t) noexcept { m_created_at = std::move(t); }
    void set_updated_at(std::chrono::system_clock::time_point&& t) noexcept { m_updated_at = std::move(t); }
    void set_metadata(std::map<std::string, std::string>&& meta) noexcept { m_metadata = std::move(meta); }
    void set_context(std::string&& ctx) noexcept { m_context = std::move(ctx); }
};

struct LineItem {
    std::string m_id{};
    std::string m_cart_id{};
    std::string m_variant_id{};
    std::string m_title{};
    int64_t m_quantity{1};
    std::string m_unit_price{"0"};
    std::string m_compare_at_unit_price{};
    std::string m_total{"0"};
    std::string m_subtotal{"0"};
    std::string m_tax_total{"0"};
    std::vector<TaxLine> m_tax_lines{};
    std::vector<Adjustment> m_adjustments{};
    bool m_is_tax_inclusive{false};
    std::map<std::string, std::string> m_metadata{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    LineItem() noexcept = default;
    LineItem(std::string&& id, std::string&& cart_id, std::string&& variant_id, std::string&& title) noexcept
        : m_id(std::move(id)), m_cart_id(std::move(cart_id)), m_variant_id(std::move(variant_id)), m_title(std::move(title)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_cart_id() noexcept { return m_cart_id; }
    const std::string& get_cart_id() const noexcept { return m_cart_id; }
    std::string& get_variant_id() noexcept { return m_variant_id; }
    const std::string& get_variant_id() const noexcept { return m_variant_id; }
    std::string& get_title() noexcept { return m_title; }
    const std::string& get_title() const noexcept { return m_title; }
    int64_t& get_quantity() noexcept { return m_quantity; }
    const int64_t& get_quantity() const noexcept { return m_quantity; }
    std::string& get_unit_price() noexcept { return m_unit_price; }
    const std::string& get_unit_price() const noexcept { return m_unit_price; }
    std::string& get_compare_at_unit_price() noexcept { return m_compare_at_unit_price; }
    const std::string& get_compare_at_unit_price() const noexcept { return m_compare_at_unit_price; }
    std::string& get_total() noexcept { return m_total; }
    const std::string& get_total() const noexcept { return m_total; }
    std::string& get_subtotal() noexcept { return m_subtotal; }
    const std::string& get_subtotal() const noexcept { return m_subtotal; }
    std::string& get_tax_total() noexcept { return m_tax_total; }
    const std::string& get_tax_total() const noexcept { return m_tax_total; }
    std::vector<TaxLine>& get_tax_lines() noexcept { return m_tax_lines; }
    const std::vector<TaxLine>& get_tax_lines() const noexcept { return m_tax_lines; }
    std::vector<Adjustment>& get_adjustments() noexcept { return m_adjustments; }
    const std::vector<Adjustment>& get_adjustments() const noexcept { return m_adjustments; }
    bool& get_is_tax_inclusive() noexcept { return m_is_tax_inclusive; }
    const bool& get_is_tax_inclusive() const noexcept { return m_is_tax_inclusive; }
    std::map<std::string, std::string>& get_metadata() noexcept { return m_metadata; }
    const std::map<std::string, std::string>& get_metadata() const noexcept { return m_metadata; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_cart_id(std::string&& id) noexcept { m_cart_id = std::move(id); }
    void set_variant_id(std::string&& id) noexcept { m_variant_id = std::move(id); }
    void set_title(std::string&& title) noexcept { m_title = std::move(title); }
    void set_quantity(int64_t qty) noexcept { m_quantity = qty; }
    void set_unit_price(std::string&& price) noexcept { m_unit_price = std::move(price); }
    void set_compare_at_unit_price(std::string&& price) noexcept { m_compare_at_unit_price = std::move(price); }
    void set_total(std::string&& total) noexcept { m_total = std::move(total); }
    void set_subtotal(std::string&& subtotal) noexcept { m_subtotal = std::move(subtotal); }
    void set_tax_total(std::string&& tax) noexcept { m_tax_total = std::move(tax); }
    void set_tax_lines(std::vector<TaxLine>&& lines) noexcept { m_tax_lines = std::move(lines); }
    void set_adjustments(std::vector<Adjustment>&& adj) noexcept { m_adjustments = std::move(adj); }
    void set_is_tax_inclusive(bool inclusive) noexcept { m_is_tax_inclusive = inclusive; }
    void set_metadata(std::map<std::string, std::string>&& meta) noexcept { m_metadata = std::move(meta); }
    void set_created_at(std::chrono::system_clock::time_point&& t) noexcept { m_created_at = std::move(t); }
    void set_updated_at(std::chrono::system_clock::time_point&& t) noexcept { m_updated_at = std::move(t); }
};

struct ShippingMethod {
    std::string m_id{};
    std::string m_cart_id{};
    std::string m_shipping_option_id{};
    std::string m_name{};
    std::string m_amount{"0"};
    std::string m_tax_total{"0"};
    std::vector<TaxLine> m_tax_lines{};
    std::map<std::string, std::string> m_data{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    ShippingMethod() noexcept = default;
    ShippingMethod(std::string&& id, std::string&& cart_id, std::string&& option_id, std::string&& name) noexcept
        : m_id(std::move(id)), m_cart_id(std::move(cart_id)), m_shipping_option_id(std::move(option_id)), m_name(std::move(name)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_cart_id() noexcept { return m_cart_id; }
    const std::string& get_cart_id() const noexcept { return m_cart_id; }
    std::string& get_shipping_option_id() noexcept { return m_shipping_option_id; }
    const std::string& get_shipping_option_id() const noexcept { return m_shipping_option_id; }
    std::string& get_name() noexcept { return m_name; }
    const std::string& get_name() const noexcept { return m_name; }
    std::string& get_amount() noexcept { return m_amount; }
    const std::string& get_amount() const noexcept { return m_amount; }
    std::string& get_tax_total() noexcept { return m_tax_total; }
    const std::string& get_tax_total() const noexcept { return m_tax_total; }
    std::vector<TaxLine>& get_tax_lines() noexcept { return m_tax_lines; }
    const std::vector<TaxLine>& get_tax_lines() const noexcept { return m_tax_lines; }
    std::map<std::string, std::string>& get_data() noexcept { return m_data; }
    const std::map<std::string, std::string>& get_data() const noexcept { return m_data; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_cart_id(std::string&& id) noexcept { m_cart_id = std::move(id); }
    void set_shipping_option_id(std::string&& id) noexcept { m_shipping_option_id = std::move(id); }
    void set_name(std::string&& name) noexcept { m_name = std::move(name); }
    void set_amount(std::string&& amount) noexcept { m_amount = std::move(amount); }
    void set_tax_total(std::string&& tax) noexcept { m_tax_total = std::move(tax); }
    void set_tax_lines(std::vector<TaxLine>&& lines) noexcept { m_tax_lines = std::move(lines); }
    void set_data(std::map<std::string, std::string>&& data) noexcept { m_data = std::move(data); }
    void set_created_at(std::chrono::system_clock::time_point&& t) noexcept { m_created_at = std::move(t); }
    void set_updated_at(std::chrono::system_clock::time_point&& t) noexcept { m_updated_at = std::move(t); }
};

struct TaxLine {
    std::string m_id{};
    std::string m_description{};
    std::string m_code{};
    std::string m_rate{"0"};
    std::string m_provider_id{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    TaxLine() noexcept = default;
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_description() noexcept { return m_description; }
    const std::string& get_description() const noexcept { return m_description; }
    std::string& get_code() noexcept { return m_code; }
    const std::string& get_code() const noexcept { return m_code; }
    std::string& get_rate() noexcept { return m_rate; }
    const std::string& get_rate() const noexcept { return m_rate; }
    std::string& get_provider_id() noexcept { return m_provider_id; }
    const std::string& get_provider_id() const noexcept { return m_provider_id; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
};

struct Adjustment {
    std::string m_id{};
    std::string m_amount{"0"};
    std::string m_description{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Adjustment() noexcept = default;
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_amount() noexcept { return m_amount; }
    const std::string& get_amount() const noexcept { return m_amount; }
    std::string& get_description() noexcept { return m_description; }
    const std::string& get_description() const noexcept { return m_description; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
};

struct Discount {
    std::string m_id{};
    std::string m_code{};
    std::string m_type{};
    std::string m_amount{"0"};
    bool m_is_dynamic{false};
    std::string m_rule_id{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Discount() noexcept = default;
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_code() noexcept { return m_code; }
    const std::string& get_code() const noexcept { return m_code; }
    std::string& get_type() noexcept { return m_type; }
    const std::string& get_type() const noexcept { return m_type; }
    std::string& get_amount() noexcept { return m_amount; }
    const std::string& get_amount() const noexcept { return m_amount; }
    bool& get_is_dynamic() noexcept { return m_is_dynamic; }
    const bool& get_is_dynamic() const noexcept { return m_is_dynamic; }
    std::string& get_rule_id() noexcept { return m_rule_id; }
    const std::string& get_rule_id() const noexcept { return m_rule_id; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
};

struct CreditLine {
    std::string m_id{};
    std::string m_amount{"0"};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    CreditLine() noexcept = default;
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_amount() noexcept { return m_amount; }
    const std::string& get_amount() const noexcept { return m_amount; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
};

// In-memory repository for carts
class CartRepository {
    std::map<std::string, Cart> m_carts{};
    std::map<std::string, std::vector<LineItem>> m_line_items{};
    std::map<std::string, std::vector<ShippingMethod>> m_shipping_methods{};
    std::map<std::string, std::vector<Discount>> m_discounts{};
    std::map<std::string, std::vector<CreditLine>> m_credit_lines{};
    
public:
    std::optional<Cart> find(const std::string& id) const noexcept {
        auto it = m_carts.find(id);
        if (it != m_carts.end()) return it->second;
        return std::nullopt;
    }
    
    std::optional<Cart> find_by_customer(const std::string& customer_id) const noexcept {
        for (const auto& [id, cart] : m_carts) {
            if (cart.get_customer_id() == customer_id && cart.get_status() == "open") {
                return cart;
            }
        }
        return std::nullopt;
    }
    
    Cart save(Cart cart) noexcept {
        m_carts[cart.get_id()] = cart;
        return cart;
    }
    
    void remove(const std::string& id) noexcept {
        m_carts.erase(id);
        m_line_items.erase(id);
        m_shipping_methods.erase(id);
        m_discounts.erase(id);
        m_credit_lines.erase(id);
    }
    
    std::vector<LineItem> get_line_items(const std::string& cart_id) const noexcept {
        auto it = m_line_items.find(cart_id);
        if (it != m_line_items.end()) return it->second;
        return {};
    }
    
    void set_line_items(const std::string& cart_id, std::vector<LineItem>&& items) noexcept {
        m_line_items[cart_id] = std::move(items);
    }
    
    std::vector<ShippingMethod> get_shipping_methods(const std::string& cart_id) const noexcept {
        auto it = m_shipping_methods.find(cart_id);
        if (it != m_shipping_methods.end()) return it->second;
        return {};
    }
    
    void set_shipping_methods(const std::string& cart_id, std::vector<ShippingMethod>&& methods) noexcept {
        m_shipping_methods[cart_id] = std::move(methods);
    }
    
    std::vector<Discount> get_discounts(const std::string& cart_id) const noexcept {
        auto it = m_discounts.find(cart_id);
        if (it != m_discounts.end()) return it->second;
        return {};
    }
    
    void set_discounts(const std::string& cart_id, std::vector<Discount>&& discounts) noexcept {
        m_discounts[cart_id] = std::move(discounts);
    }
    
    std::vector<CreditLine> get_credit_lines(const std::string& cart_id) const noexcept {
        auto it = m_credit_lines.find(cart_id);
        if (it != m_credit_lines.end()) return it->second;
        return {};
    }
    
    void set_credit_lines(const std::string& cart_id, std::vector<CreditLine>&& lines) noexcept {
        m_credit_lines[cart_id] = std::move(lines);
    }
};

// Calculate totals for a cart
void calculate_cart_totals(Cart& cart, const database::Database& db) {
    // In real implementation, this would:
    // 1. Calculate item subtotals with quantity-based pricing
    // 2. Apply price lists (sale, customer group)
    // 3. Calculate taxes based on region/address
    // 4. Apply promotions/discounts
    // 5. Calculate shipping
    // 6. Apply gift cards/credit lines
    
    // For now, simplified calculation
    std::string item_subtotal = "0";
    std::string item_tax_total = "0";
    std::string item_total = "0";
    
    for (const auto& item : cart.get_items()) {
        // Use unit_price * quantity
        int64_t unit_price = std::stoll(item.get_unit_price());
        int64_t qty = item.get_quantity();
        int64_t line_total = unit_price * qty;
        item_subtotal = std::to_string(std::stoll(item_subtotal) + line_total);
        item.set_total(std::to_string(line_total));
    }
    
    cart.set_total(item_subtotal);
    cart.set_subtotal(item_subtotal);
    cart.set_item_subtotal(item_subtotal);
    cart.set_item_total(item_subtotal);
    cart.set_original_item_subtotal(item_subtotal);
    cart.set_original_item_total(item_subtotal);
    cart.set_original_total(item_subtotal);
    cart.set_subtotal(item_subtotal);
    cart.set_original_subtotal(item_subtotal);
    cart.set_tax_total("0");
    cart.set_original_tax_total("0");
    cart.set_discount_total("0");
    cart.set_shipping_total("0");
    cart.set_original_shipping_total("0");
    cart.set_original_shipping_subtotal("0");
    cart.set_original_shipping_tax_total("0");
    cart.set_item_tax_total("0");
    cart.set_original_item_tax_total("0");
    cart.set_promo_total("0");
    cart.set_original_promo_total("0");
}

void register_complete_cart_routes(router::Router& router, database::Database& db) {
    CartRepository cart_repo;
    auto variant_repo = db.repository<entities::ProductVariant>();
    auto product_repo = db.repository<entities::Product>();
    auto region_repo = db.repository<entities::Region>();
    auto sales_channel_repo = db.repository<entities::SalesChannel>();
    auto price_list_repo = db.repository<entities::PriceList>();
    auto price_repo = db.repository<entities::Price>();
    auto customer_group_repo = db.repository<entities::CustomerGroup>();
    auto customer_repo = db.repository<entities::Customer>();
    auto shipping_option_repo = db.repository<entities::ShippingOption>();
    auto shipping_profile_repo = db.repository<entities::ShippingProfile>();
    auto fulfillment_set_repo = db.repository<entities::FulfillmentSet>();
    auto stock_location_repo = db.repository<entities::StockLocation>();
    auto promotion_repo = db.repository<entities::Promotion>();
    
    // GET /store/carts - Get current customer's cart
    router.add_route("/store/carts", router::HttpMethod::GET,
        [cart_repo = std::move(cart_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto cart = cart_repo.find_by_customer(customer_id);
                
                if (!cart) {
                    // Return empty cart or 404
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                // Attach line items
                cart->set_items(cart_repo.get_line_items(cart->get_id()));
                cart->set_shipping_methods(cart_repo.get_shipping_methods(cart->get_id()));
                cart->set_discounts(cart_repo.get_discounts(cart->get_id()));
                cart->set_credit_lines(cart_repo.get_credit_lines(cart->get_id()));
                
                res.json({{"cart", *cart}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /store/carts/:id - Get cart by ID
    router.add_route("/store/carts/:id", router::HttpMethod::GET,
        [cart_repo = std::move(cart_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto cart = cart_repo.find(id);
                
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                // Verify ownership or allow if admin
                if (!cart->get_customer_id().empty() && cart->get_customer_id() != customer_id) {
                    // Check if it's a guest cart with email
                    if (cart->get_email() != customer_id) {
                        res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                        return;
                    }
                }
                
                // Attach related data
                cart->set_items(cart_repo.get_line_items(cart->get_id()));
                cart->set_shipping_methods(cart_repo.get_shipping_methods(cart->get_id()));
                cart->set_discounts(cart_repo.get_discounts(cart->get_id()));
                cart->set_credit_lines(cart_repo.get_credit_lines(cart->get_id()));
                
                res.json({{"cart", *cart}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/carts - Create cart
    router.add_route("/store/carts", router::HttpMethod::POST,
        [cart_repo = std::move(cart_repo), region_repo = std::move(region_repo), 
         sales_channel_repo = std::move(sales_channel_repo), price_list_repo = std::move(price_list_repo),
         price_repo = std::move(price_repo), customer_group_repo = std::move(customer_group_repo),
         customer_repo = std::move(customer_repo), variant_repo = std::move(variant_repo),
         product_repo = std::move(product_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                std::string customer_id;
                std::string email;
                std::string currency_code = "usd";
                std::string region_id;
                std::string sales_channel_id;
                std::vector<std::map<std::string, std::string>> items;
                
                // Parse request body
                // In real implementation, parse JSON from req.body()
                // For now, using placeholder
                
                // If authenticated customer, use their ID
                if (req.auth_context()) {
                    customer_id = req.auth_context()->actor_id();
                    
                    // Try to find existing open cart
                    auto existing = cart_repo.find_by_customer(customer_id);
                    if (existing) {
                        res.json({{"cart", *existing}});
                        return;
                    }
                }
                
                Cart cart(generate_id());
                cart.set_customer_id(std::move(customer_id));
                cart.set_email(std::move(email));
                cart.set_currency_code(std::move(currency_code));
                cart.set_region_id(std::move(region_id));
                cart.set_sales_channel_id(std::move(sales_channel_id));
                cart.set_status(std::string("open"));
                cart.set_created_at(std::chrono::system_clock::now());
                cart.set_updated_at(std::chrono::system_clock::now());
                
                // Process line items
                std::vector<LineItem> line_items;
                for (const auto& item : items) {
                    std::string variant_id = item.at("variant_id");
                    int64_t quantity = std::stoll(item.at("quantity"));
                    
                    auto variant = variant_repo.find(variant_id);
                    if (!variant) continue;
                    
                    auto product = product_repo.find(variant->get_product_id());
                    
                    LineItem line_item(generate_id(), cart.get_id(), variant_id, variant->get_title());
                    line_item.set_quantity(quantity);
                    line_item.set_unit_price(variant->get_price());
                    line_item.set_total(std::to_string(std::stoll(variant->get_price()) * quantity));
                    line_item.set_subtotal(line_item.get_total());
                    line_item.set_is_tax_inclusive(true);
                    line_item.set_created_at(std::chrono::system_clock::now());
                    line_item.set_updated_at(std::chrono::system_clock::now());
                    
                    // Calculate tax lines (placeholder)
                    TaxLine tax_line;
                    tax_line.set_id(generate_id());
                    tax_line.set_description("CA Default Rate");
                    tax_line.set_code("CADEFAULT");
                    tax_line.set_rate("5");
                    tax_line.set_provider_id("system");
                    line_item.get_tax_lines().push_back(std::move(tax_line));
                    
                    line_items.push_back(std::move(line_item));
                }
                
                cart.set_items(std::move(line_items));
                calculate_cart_totals(cart, db);
                
                cart = cart_repo.save(std::move(cart));
                cart_repo.set_line_items(cart.get_id(), cart.get_items());
                
                res.status(201).json({{"cart", cart}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/carts/:id - Update cart
    router.add_route("/store/carts/:id", router::HttpMethod::POST,
        [cart_repo = std::move(cart_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto cart = cart_repo.find(id);
                
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                if (!cart->get_customer_id().empty() && cart->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                // Update fields from request body
                // In real implementation, parse JSON and update
                
                cart->set_updated_at(std::chrono::system_clock::now());
                cart = cart_repo.save(std::move(*cart));
                
                res.json({{"cart", *cart}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /store/carts/:id - Delete cart
    router.add_route("/store/carts/:id", router::HttpMethod::DELETE,
        [cart_repo = std::move(cart_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto cart = cart_repo.find(id);
                
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                if (cart->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                cart_repo.remove(id);
                res.status(200).json({{"id", id}, {"deleted", true}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/carts/:id/line-items - Add line item
    router.add_route("/store/carts/:id/line-items", router::HttpMethod::POST,
        [cart_repo = std::move(cart_repo), variant_repo = std::move(variant_repo),
         product_repo = std::move(product_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto cart = cart_repo.find(id);
                
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                if (!cart->get_customer_id().empty() && cart->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                // Parse variant_id, quantity, metadata
                std::string variant_id = req.body_as<std::string>().value_or("");
                int64_t quantity = 1;
                std::map<std::string, std::string> metadata;
                
                auto variant = variant_repo.find(variant_id);
                if (!variant) {
                    res.status(404).json(http::ErrorResponse::not_found("Variant not found"));
                    return;
                }
                
                auto product = product_repo.find(variant->get_product_id());
                
                LineItem line_item(generate_id(), id, variant_id, variant->get_title());
                line_item.set_quantity(quantity);
                line_item.set_unit_price(variant->get_price());
                line_item.set_total(std::to_string(std::stoll(variant->get_price()) * quantity));
                line_item.set_subtotal(line_item.get_total());
                line_item.set_is_tax_inclusive(true);
                line_item.set_metadata(std::move(metadata));
                line_item.set_created_at(std::chrono::system_clock::now());
                line_item.set_updated_at(std::chrono::system_clock::now());
                
                // Check if variant already in cart - merge quantities
                auto& items = cart->get_items();
                bool found = false;
                for (auto& item : items) {
                    if (item.get_variant_id() == variant_id) {
                        item.set_quantity(item.get_quantity() + quantity);
                        item.set_total(std::to_string(std::stoll(item.get_unit_price()) * item.get_quantity()));
                        found = true;
                        break;
                    }
                }
                
                if (!found) {
                    items.push_back(std::move(line_item));
                }
                
                cart->set_updated_at(std::chrono::system_clock::now());
                calculate_cart_totals(*cart, db);
                cart = cart_repo.save(std::move(*cart));
                cart_repo.set_line_items(cart->get_id(), cart->get_items());
                
                res.status(201).json({{"cart", *cart}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // PUT /store/carts/:id/line-items/:item_id - Update line item
    router.add_route("/store/carts/:id/line-items/:item_id", router::HttpMethod::PUT,
        [cart_repo = std::move(cart_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                auto item_id = req.get_param("item_id");
                
                auto cart = cart_repo.find(id);
                
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                if (!cart->get_customer_id().empty() && cart->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                auto& items = cart->get_items();
                bool found = false;
                for (auto& item : items) {
                    if (item.get_id() == item_id) {
                        int64_t quantity = req.body_as<int64_t>().value_or(1);
                        item.set_quantity(quantity);
                        item.set_total(std::to_string(std::stoll(item.get_unit_price()) * quantity));
                        item.set_updated_at(std::chrono::system_clock::now());
                        found = true;
                        break;
                    }
                }
                
                if (!found) {
                    res.status(404).json(http::ErrorResponse::not_found("Line item not found"));
                    return;
                }
                
                cart->set_updated_at(std::chrono::system_clock::now());
                calculate_cart_totals(*cart, db);
                cart = cart_repo.save(std::move(*cart));
                cart_repo.set_line_items(cart->get_id(), cart->get_items());
                
                res.json({{"cart", *cart}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /store/carts/:id/line-items/:item_id - Remove line item
    router.add_route("/store/carts/:id/line-items/:item_id", router::HttpMethod::DELETE,
        [cart_repo = std::move(cart_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                auto item_id = req.get_param("item_id");
                
                auto cart = cart_repo.find(id);
                
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                if (!cart->get_customer_id().empty() && cart->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                auto& items = cart->get_items();
                items.erase(
                    std::remove_if(items.begin(), items.end(),
                        [&item_id](const LineItem& item) { return item.get_id() == item_id; }),
                    items.end()
                );
                
                cart->set_updated_at(std::chrono::system_clock::now());
                calculate_cart_totals(*cart, db);
                cart = cart_repo.save(std::move(*cart));
                cart_repo.set_line_items(cart->get_id(), cart->get_items());
                
                res.status(200).json({{"cart", *cart}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/carts/:id/line-items/:item_id/metadata - Update line item metadata
    router.add_route("/store/carts/:id/line-items/:item_id/metadata", router::HttpMethod::POST,
        [cart_repo = std::move(cart_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto id = req.get_param("id");
                auto item_id = req.get_param("item_id");
                
                auto cart = cart_repo.find(id);
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                auto& items = cart->get_items();
                for (auto& item : items) {
                    if (item.get_id() == item_id) {
                        std::map<std::string, std::string> metadata = req.body_as<std::map<std::string, std::string>>().value_or({});
                        item.set_metadata(std::move(metadata));
                        item.set_updated_at(std::chrono::system_clock::now());
                        break;
                    }
                }
                
                cart->set_updated_at(std::chrono::system_clock::now());
                cart = cart_repo.save(std::move(*cart));
                cart_repo.set_line_items(cart->get_id(), cart->get_items());
                
                res.json({{"cart", *cart}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/carts/:id/payment-sessions - Initialize payment session
    router.add_route("/store/carts/:id/payment-sessions", router::HttpMethod::POST,
        [cart_repo = std::move(cart_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                auto id = req.get_param("id");
                auto cart = cart_repo.find(id);
                
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                // In real implementation, initialize payment session
                res.json({{"payment_sessions", std::vector<std::string>{}}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/carts/:id/shipping-methods - Add shipping method
    router.add_route("/store/carts/:id/shipping-methods", router::HttpMethod::POST,
        [cart_repo = std::move(cart_repo), shipping_option_repo = std::move(shipping_option_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto cart = cart_repo.find(id);
                
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                if (!cart->get_customer_id().empty() && cart->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                // Parse shipping_option_id
                std::string shipping_option_id = req.body_as<std::string>().value_or("");
                
                auto shipping_option = shipping_option_repo.find(shipping_option_id);
                if (!shipping_option) {
                    res.status(404).json(http::ErrorResponse::not_found("Shipping option not found"));
                    return;
                }
                
                ShippingMethod method(generate_id(), id, shipping_option_id, shipping_option->get_name());
                method.set_amount(shipping_option->get_amount());
                method.set_created_at(std::chrono::system_clock::now());
                method.set_updated_at(std::chrono::system_clock::now());
                
                auto methods = cart->get_shipping_methods();
                methods.push_back(std::move(method));
                cart->set_shipping_methods(std::move(methods));
                
                cart->set_updated_at(std::chrono::system_clock::now());
                calculate_cart_totals(*cart, db);
                cart = cart_repo.save(std::move(*cart));
                cart_repo.set_shipping_methods(cart->get_id(), cart->get_shipping_methods());
                
                res.json({{"cart", *cart}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /store/carts/:id/shipping-methods/:method_id - Remove shipping method
    router.add_route("/store/carts/:id/shipping-methods/:method_id", router::HttpMethod::DELETE,
        [cart_repo = std::move(cart_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                auto method_id = req.get_param("method_id");
                
                auto cart = cart_repo.find(id);
                
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                if (!cart->get_customer_id().empty() && cart->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                auto& methods = cart->get_shipping_methods();
                methods.erase(
                    std::remove_if(methods.begin(), methods.end(),
                        [&method_id](const ShippingMethod& m) { return m.get_id() == method_id; }),
                    methods.end()
                );
                
                cart->set_shipping_methods(std::move(methods));
                cart->set_updated_at(std::chrono::system_clock::now());
                calculate_cart_totals(*cart, db);
                cart = cart_repo.save(std::move(*cart));
                cart_repo.set_shipping_methods(cart->get_id(), cart->get_shipping_methods());
                
                res.json({{"cart", *cart}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/carts/:id/promotions - Add promotion
    router.add_route("/store/carts/:id/promotions", router::HttpMethod::POST,
        [cart_repo = std::move(cart_repo), promotion_repo = std::move(promotion_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto id = req.get_param("id");
                auto cart = cart_repo.find(id);
                
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                std::string code = req.body_as<std::string>().value_or("");
                
                // Find promotion by code
                // In real implementation, validate and apply promotion
                
                Discount discount;
                discount.set_id(generate_id());
                discount.set_code(code);
                discount.set_type("promotion");
                discount.set_amount("0");
                discount.set_created_at(std::chrono::system_clock::now());
                
                auto discounts = cart->get_discounts();
                discounts.push_back(std::move(discount));
                cart->set_discounts(std::move(discounts));
                
                cart->set_updated_at(std::chrono::system_clock::now());
                calculate_cart_totals(*cart, db);
                cart = cart_repo.save(std::move(*cart));
                cart_repo.set_discounts(cart->get_id(), cart->get_discounts());
                
                res.json({{"cart", *cart}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // DELETE /store/carts/:id/promotions/:code - Remove promotion
    router.add_route("/store/carts/:id/promotions/:code", router::HttpMethod::DELETE,
        [cart_repo = std::move(cart_repo)](const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto id = req.get_param("id");
                auto code = req.get_param("code");
                
                auto cart = cart_repo.find(id);
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                auto& discounts = cart->get_discounts();
                discounts.erase(
                    std::remove_if(discounts.begin(), discounts.end(),
                        [&code](const Discount& d) { return d.get_code() == code; }),
                    discounts.end()
                );
                
                cart->set_discounts(std::move(discounts));
                cart->set_updated_at(std::chrono::system_clock::now());
                calculate_cart_totals(*cart, db);
                cart = cart_repo.save(std::move(*cart));
                cart_repo.set_discounts(cart->get_id(), cart->get_discounts());
                
                res.json({{"cart", *cart}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // POST /store/carts/:id/complete - Complete cart (convert to order)
    router.add_route("/store/carts/:id/complete", router::HttpMethod::POST,
        [cart_repo = std::move(cart_repo), line_item_repo = db.repository<entities::LineItem>(),
         order_repo = db.repository<entities::Order>(), customer_repo = db.repository<entities::Customer>(),
         variant_repo = std::move(variant_repo), product_repo = std::move(product_repo)]
        (const http::Request& req, http::Response& res) mutable {
            try {
                if (!req.auth_context()) {
                    res.status(401).json(http::ErrorResponse::unauthorized());
                    return;
                }
                
                auto customer_id = req.auth_context()->actor_id();
                auto id = req.get_param("id");
                
                auto cart = cart_repo.find(id);
                
                if (!cart) {
                    res.status(404).json(http::ErrorResponse::not_found("Cart not found"));
                    return;
                }
                
                if (cart->get_customer_id() != customer_id) {
                    res.status(403).json(http::ErrorResponse::forbidden("Not authorized"));
                    return;
                }
                
                // Convert cart to order
                entities::Order order;
                order.add_id(generate_id());
                order.add_customer_id(cart->get_customer_id());
                order.add_region_id(cart->get_region_id());
                order.add_currency_code(cart->get_currency_code());
                order.add_status("completed");
                order.add_payment_status("captured");
                order.add_fulfillment_status("not_fulfilled");
                order.add_total(cart->get_total());
                order.add_subtotal(cart->get_subtotal());
                order.add_tax_total(cart->get_tax_total());
                order.add_shipping_total(cart->get_shipping_total());
                order.add_discount_total(cart->get_discount_total());
                order.set_created_at(std::chrono::system_clock::now());
                order.set_updated_at(std::chrono::system_clock::now());
                
                order = order_repo.save(std::move(order));
                
                // Create line items for order
                for (const auto& item : cart->get_items()) {
                    entities::LineItem line_item;
                    line_item.add_id(generate_id());
                    line_item.add_order_id(order.get_id());
                    line_item.add_variant_id(item.get_variant_id());
                    line_item.add_title(item.get_title());
                    line_item.add_quantity(item.get_quantity());
                    line_item.add_unit_price(item.get_unit_price());
                    line_item.add_total(item.get_total());
                    line_item_repo.save(std::move(line_item));
                }
                
                // Mark cart as completed
                cart->set_status(std::string("completed"));
                cart->set_completed_at(std::chrono::system_clock::now());
                cart = cart_repo.save(std::move(*cart));
                cart_repo.set_line_items(cart->get_id(), cart->get_items());
                
                res.json({{"cart", *cart}, {"order_id", order.get_id()}});
            } catch (const std::exception& e) {
                res.status(500).json(http::ErrorResponse::internal(e.what()));
            }
        });
    
    // GET /store/carts/:id/payment-collection - Get payment collection
    router.add_route("/store/carts/:id/payment-collection", router::HttpMethod::GET,
        [](const http::Request& req, http::Response& res) {
            res.json({{"payment_collection", std::monostate{}}});
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