export module medusa_core.database.entities_extended;

import std;
import database.types;
import reflection.entity_traits;

export namespace medusa_core::entities {

// Image entity
struct Image final {
    std::string m_id{};
    std::string m_product_id{};
    std::string m_url{};
    int m_rank{0};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Image() noexcept = default;
    Image(std::string&& id, std::string&& product_id, std::string&& url) noexcept
        : m_id(std::move(id)), m_product_id(std::move(product_id)), m_url(std::move(url)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_product_id() noexcept { return m_product_id; }
    const std::string& get_product_id() const noexcept { return m_product_id; }
    std::string& get_url() noexcept { return m_url; }
    const std::string& get_url() const noexcept { return m_url; }
    int& get_rank() noexcept { return m_rank; }
    const int& get_rank() const noexcept { return m_rank; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_product_id(std::string&& product_id) noexcept { m_product_id = std::move(product_id); }
    void set_url(std::string&& url) noexcept { m_url = std::move(url); }
    void set_rank(int rank) noexcept { m_rank = rank; }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "Image"; }
    const char* table_name() const noexcept { return "image"; }
};

// Type aliases for compatibility
using ProductImage = Image;
using ProductOption = Option;
using ProductTag = ProductTag;
using ProductType = ProductType;

// Option entity
struct Option final {
    std::string m_id{};
    std::string m_product_id{};
    std::string m_title{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Option() noexcept = default;
    Option(std::string&& id, std::string&& product_id, std::string&& title) noexcept
        : m_id(std::move(id)), m_product_id(std::move(product_id)), m_title(std::move(title)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_product_id() noexcept { return m_product_id; }
    const std::string& get_product_id() const noexcept { return m_product_id; }
    std::string& get_title() noexcept { return m_title; }
    const std::string& get_title() const noexcept { return m_title; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_product_id(std::string&& product_id) noexcept { m_product_id = std::move(product_id); }
    void set_title(std::string&& title) noexcept { m_title = std::move(title); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "Option"; }
    const char* table_name() const noexcept { return "option"; }
};

// ProductTag entity
struct ProductTag final {
    std::string m_id{};
    std::string m_value{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    ProductTag() noexcept = default;
    ProductTag(std::string&& id, std::string&& value) noexcept
        : m_id(std::move(id)), m_value(std::move(value)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_value() noexcept { return m_value; }
    const std::string& get_value() const noexcept { return m_value; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_value(std::string&& value) noexcept { m_value = std::move(value); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "ProductTag"; }
    const char* table_name() const noexcept { return "product_tag"; }
};

// ProductType entity
struct ProductType final {
    std::string m_id{};
    std::string m_value{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    ProductType() noexcept = default;
    ProductType(std::string&& id, std::string&& value) noexcept
        : m_id(std::move(id)), m_value(std::move(value)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_value() noexcept { return m_value; }
    const std::string& get_value() const noexcept { return m_value; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_value(std::string&& value) noexcept { m_value = std::move(value); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "ProductType"; }
    const char* table_name() const noexcept { return "product_type"; }
};

// Collection entity
struct Collection final {
    std::string m_id{};
    std::string m_title{};
    std::string m_handle{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    Collection() noexcept = default;
    Collection(std::string&& id, std::string&& title) noexcept
        : m_id(std::move(id)), m_title(std::move(title)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_title() noexcept { return m_title; }
    const std::string& get_title() const noexcept { return m_title; }
    std::string& get_handle() noexcept { return m_handle; }
    const std::string& get_handle() const noexcept { return m_handle; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_title(std::string&& title) noexcept { m_title = std::move(title); }
    void set_handle(std::string&& handle) noexcept { m_handle = std::move(handle); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    const char* entity_name() const noexcept { return "Collection"; }
    const char* table_name() const noexcept { return "collection"; }
};

// ProductCategory entity
struct ProductCategory final {
    std::string m_id{};
    std::string m_name{};
    std::string m_handle{};
    std::string m_parent_id{};
    int m_rank{0};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    ProductCategory() noexcept = default;
    ProductCategory(std::string&& id, std::string&& name) noexcept
        : m_id(std::move(id)), m_name(std::move(name)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_name() noexcept { return m_name; }
    const std::string& get_name() const noexcept { return m_name; }
    std::string& get_handle() noexcept { return m_handle; }
    const std::string& get_handle() const noexcept { return m_handle; }
    std::string& get_parent_id() noexcept { return m_parent_id; }
    const std::string& get_parent_id() const noexcept { return m_parent_id; }
    int& get_rank() noexcept { return m_rank; }
    const int& get_rank() const noexcept { return m_rank; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_name(std::string&& name) noexcept { m_name = std::move(name); }
    void set_handle(std::string&& handle) noexcept { m_handle = std::move(handle); }
    void set_parent_id(std::string&& parent_id) noexcept { m_parent_id = std::move(parent_id); }
    void set_rank(int rank) noexcept { m_rank = rank; }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    const char* entity_name() const noexcept { return "ProductCategory"; }
    const char* table_name() const noexcept { return "product_category"; }
};

// CustomerGroup entity
struct CustomerGroup final {
    std::string m_id{};
    std::string m_name{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    CustomerGroup() noexcept = default;
    CustomerGroup(std::string&& id, std::string&& name) noexcept
        : m_id(std::move(id)), m_name(std::move(name)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_name() noexcept { return m_name; }
    const std::string& get_name() const noexcept { return m_name; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_name(std::string&& name) noexcept { m_name = std::move(name); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    const char* entity_name() const noexcept { return "CustomerGroup"; }
    const char* table_name() const noexcept { return "customer_group"; }
};

// ReturnReason entity
struct ReturnReason final {
    std::string m_id{};
    std::string m_value{};
    std::string m_label{};
    std::string m_description{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    ReturnReason() noexcept = default;
    ReturnReason(std::string&& id, std::string&& value, std::string&& label) noexcept
        : m_id(std::move(id)), m_value(std::move(value)), m_label(std::move(label)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_value() noexcept { return m_value; }
    const std::string& get_value() const noexcept { return m_value; }
    std::string& get_label() noexcept { return m_label; }
    const std::string& get_label() const noexcept { return m_label; }
    std::string& get_description() noexcept { return m_description; }
    const std::string& get_description() const noexcept { return m_description; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_value(std::string&& value) noexcept { m_value = std::move(value); }
    void set_label(std::string&& label) noexcept { m_label = std::move(label); }
    void set_description(std::string&& description) noexcept { m_description = std::move(description); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    const char* entity_name() const noexcept { return "ReturnReason"; }
    const char* table_name() const noexcept { return "return_reason"; }
};

// Return entity
struct Return final {
    std::string m_id{};
    std::string m_order_id{};
    std::string m_customer_id{};
    std::string m_status{"pending"};
    std::string m_reason{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Return() noexcept = default;
    Return(std::string&& id, std::string&& order_id, std::string&& customer_id) noexcept
        : m_id(std::move(id)), m_order_id(std::move(order_id)), m_customer_id(std::move(customer_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_order_id() noexcept { return m_order_id; }
    const std::string& get_order_id() const noexcept { return m_order_id; }
    std::string& get_customer_id() noexcept { return m_customer_id; }
    const std::string& get_customer_id() const noexcept { return m_customer_id; }
    std::string& get_status() noexcept { return m_status; }
    const std::string& get_status() const noexcept { return m_status; }
    std::string& get_reason() noexcept { return m_reason; }
    const std::string& get_reason() const noexcept { return m_reason; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_order_id(std::string&& order_id) noexcept { m_order_id = std::move(order_id); }
    void set_customer_id(std::string&& customer_id) noexcept { m_customer_id = std::move(customer_id); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_reason(std::string&& reason) noexcept { m_reason = std::move(reason); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "Return"; }
    const char* table_name() const noexcept { return "return"; }
};

// Claim entity
struct Claim final {
    std::string m_id{};
    std::string m_order_id{};
    std::string m_customer_id{};
    std::string m_status{"pending"};
    std::string m_type{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Claim() noexcept = default;
    Claim(std::string&& id, std::string&& order_id, std::string&& customer_id) noexcept
        : m_id(std::move(id)), m_order_id(std::move(order_id)), m_customer_id(std::move(customer_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_order_id() noexcept { return m_order_id; }
    const std::string& get_order_id() const noexcept { return m_order_id; }
    std::string& get_customer_id() noexcept { return m_customer_id; }
    const std::string& get_customer_id() const noexcept { return m_customer_id; }
    std::string& get_status() noexcept { return m_status; }
    const std::string& get_status() const noexcept { return m_status; }
    std::string& get_type() noexcept { return m_type; }
    const std::string& get_type() const noexcept { return m_type; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_order_id(std::string&& order_id) noexcept { m_order_id = std::move(order_id); }
    void set_customer_id(std::string&& customer_id) noexcept { m_customer_id = std::move(customer_id); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_type(std::string&& type) noexcept { m_type = std::move(type); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "Claim"; }
    const char* table_name() const noexcept { return "claim"; }
};

// Exchange entity
struct Exchange final {
    std::string m_id{};
    std::string m_order_id{};
    std::string m_customer_id{};
    std::string m_status{"pending"};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Exchange() noexcept = default;
    Exchange(std::string&& id, std::string&& order_id, std::string&& customer_id) noexcept
        : m_id(std::move(id)), m_order_id(std::move(order_id)), m_customer_id(std::move(customer_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_order_id() noexcept { return m_order_id; }
    const std::string& get_order_id() const noexcept { return m_order_id; }
    std::string& get_customer_id() noexcept { return m_customer_id; }
    const std::string& get_customer_id() const noexcept { return m_customer_id; }
    std::string& get_status() noexcept { return m_status; }
    const std::string& get_status() const noexcept { return m_status; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_order_id(std::string&& order_id) noexcept { m_order_id = std::move(order_id); }
    void set_customer_id(std::string&& customer_id) noexcept { m_customer_id = std::move(customer_id); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "Exchange"; }
    const char* table_name() const noexcept { return "exchange"; }
};

// Country entity
struct Country final {
    std::string m_id{};
    std::string m_iso_2{};
    std::string m_iso_3{};
    std::string m_num_code{};
    std::string m_name{};
    std::string m_display_name{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Country() noexcept = default;
    Country(std::string&& id, std::string&& iso_2, std::string&& name) noexcept
        : m_id(std::move(id)), m_iso_2(std::move(iso_2)), m_name(std::move(name)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_iso_2() noexcept { return m_iso_2; }
    const std::string& get_iso_2() const noexcept { return m_iso_2; }
    std::string& get_iso_3() noexcept { return m_iso_3; }
    const std::string& get_iso_3() const noexcept { return m_iso_3; }
    std::string& get_num_code() noexcept { return m_num_code; }
    const std::string& get_num_code() const noexcept { return m_num_code; }
    std::string& get_name() noexcept { return m_name; }
    const std::string& get_name() const noexcept { return m_name; }
    std::string& get_display_name() noexcept { return m_display_name; }
    const std::string& get_display_name() const noexcept { return m_display_name; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_iso_2(std::string&& iso_2) noexcept { m_iso_2 = std::move(iso_2); }
    void set_iso_3(std::string&& iso_3) noexcept { m_iso_3 = std::move(iso_3); }
    void set_num_code(std::string&& num_code) noexcept { m_num_code = std::move(num_code); }
    void set_name(std::string&& name) noexcept { m_name = std::move(name); }
    void set_display_name(std::string&& display_name) noexcept { m_display_name = std::move(display_name); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "Country"; }
    const char* table_name() const noexcept { return "country"; }
};

// Province entity
struct Province final {
    std::string m_id{};
    std::string m_country_id{};
    std::string m_name{};
    std::string m_code{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Province() noexcept = default;
    Province(std::string&& id, std::string&& country_id, std::string&& name) noexcept
        : m_id(std::move(id)), m_country_id(std::move(country_id)), m_name(std::move(name)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_country_id() noexcept { return m_country_id; }
    const std::string& get_country_id() const noexcept { return m_country_id; }
    std::string& get_name() noexcept { return m_name; }
    const std::string& get_name() const noexcept { return m_name; }
    std::string& get_code() noexcept { return m_code; }
    const std::string& get_code() const noexcept { return m_code; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_country_id(std::string&& country_id) noexcept { m_country_id = std::move(country_id); }
    void set_name(std::string&& name) noexcept { m_name = std::move(name); }
    void set_code(std::string&& code) noexcept { m_code = std::move(code); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "Province"; }
    const char* table_name() const noexcept { return "province"; }
};

// InventoryItem entity
struct InventoryItem final {
    std::string m_id{};
    std::string m_sku{};
    bool m_requires_shipping{true};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    InventoryItem() noexcept = default;
    InventoryItem(std::string&& id, std::string&& sku) noexcept
        : m_id(std::move(id)), m_sku(std::move(sku)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_sku() noexcept { return m_sku; }
    const std::string& get_sku() const noexcept { return m_sku; }
    bool& get_requires_shipping() noexcept { return m_requires_shipping; }
    const bool& get_requires_shipping() const noexcept { return m_requires_shipping; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_sku(std::string&& sku) noexcept { m_sku = std::move(sku); }
    void set_requires_shipping(bool requires_shipping) noexcept { m_requires_shipping = requires_shipping; }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    const char* entity_name() const noexcept { return "InventoryItem"; }
    const char* table_name() const noexcept { return "inventory_item"; }
};

// InventoryLevel entity
struct InventoryLevel final {
    std::string m_id{};
    std::string m_inventory_item_id{};
    std::string m_location_id{};
    int64_t m_stocked_quantity{0};
    int64_t m_reserved_quantity{0};
    int64_t m_incoming_quantity{0};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    InventoryLevel() noexcept = default;
    InventoryLevel(std::string&& id, std::string&& inventory_item_id, std::string&& location_id) noexcept
        : m_id(std::move(id)), m_inventory_item_id(std::move(inventory_item_id)), m_location_id(std::move(location_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_inventory_item_id() noexcept { return m_inventory_item_id; }
    const std::string& get_inventory_item_id() const noexcept { return m_inventory_item_id; }
    std::string& get_location_id() noexcept { return m_location_id; }
    const std::string& get_location_id() const noexcept { return m_location_id; }
    int64_t& get_stocked_quantity() noexcept { return m_stocked_quantity; }
    const int64_t& get_stocked_quantity() const noexcept { return m_stocked_quantity; }
    int64_t& get_reserved_quantity() noexcept { return m_reserved_quantity; }
    const int64_t& get_reserved_quantity() const noexcept { return m_reserved_quantity; }
    int64_t& get_incoming_quantity() noexcept { return m_incoming_quantity; }
    const int64_t& get_incoming_quantity() const noexcept { return m_incoming_quantity; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_inventory_item_id(std::string&& inventory_item_id) noexcept { m_inventory_item_id = std::move(inventory_item_id); }
    void set_location_id(std::string&& location_id) noexcept { m_location_id = std::move(location_id); }
    void set_stocked_quantity(int64_t stocked_quantity) noexcept { m_stocked_quantity = stocked_quantity; }
    void set_reserved_quantity(int64_t reserved_quantity) noexcept { m_reserved_quantity = reserved_quantity; }
    void set_incoming_quantity(int64_t incoming_quantity) noexcept { m_incoming_quantity = incoming_quantity; }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "InventoryLevel"; }
    const char* table_name() const noexcept { return "inventory_level"; }
};

// Reservation entity
struct Reservation final {
    std::string m_id{};
    std::string m_inventory_item_id{};
    std::string m_location_id{};
    std::string m_line_item_id{};
    int64_t m_quantity{0};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Reservation() noexcept = default;
    Reservation(std::string&& id, std::string&& inventory_item_id, std::string&& location_id, std::string&& line_item_id) noexcept
        : m_id(std::move(id)), m_inventory_item_id(std::move(inventory_item_id)), m_location_id(std::move(location_id)), m_line_item_id(std::move(line_item_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_inventory_item_id() noexcept { return m_inventory_item_id; }
    const std::string& get_inventory_item_id() const noexcept { return m_inventory_item_id; }
    std::string& get_location_id() noexcept { return m_location_id; }
    const std::string& get_location_id() const noexcept { return m_location_id; }
    std::string& get_line_item_id() noexcept { return m_line_item_id; }
    const std::string& get_line_item_id() const noexcept { return m_line_item_id; }
    int64_t& get_quantity() noexcept { return m_quantity; }
    const int64_t& get_quantity() const noexcept { return m_quantity; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_inventory_item_id(std::string&& inventory_item_id) noexcept { m_inventory_item_id = std::move(inventory_item_id); }
    void set_location_id(std::string&& location_id) noexcept { m_location_id = std::move(location_id); }
    void set_line_item_id(std::string&& line_item_id) noexcept { m_line_item_id = std::move(line_item_id); }
    void set_quantity(int64_t quantity) noexcept { m_quantity = quantity; }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "Reservation"; }
    const char* table_name() const noexcept { return "reservation"; }
};

// StockLocation entity
struct StockLocation final {
    std::string m_id{};
    std::string m_name{};
    std::string m_address_id{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    StockLocation() noexcept = default;
    StockLocation(std::string&& id, std::string&& name) noexcept
        : m_id(std::move(id)), m_name(std::move(name)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_name() noexcept { return m_name; }
    const std::string& get_name() const noexcept { return m_name; }
    std::string& get_address_id() noexcept { return m_address_id; }
    const std::string& get_address_id() const noexcept { return m_address_id; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_name(std::string&& name) noexcept { m_name = std::move(name); }
    void set_address_id(std::string&& address_id) noexcept { m_address_id = std::move(address_id); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    const char* entity_name() const noexcept { return "StockLocation"; }
    const char* table_name() const noexcept { return "stock_location"; }
};

// Fulfillment entity
struct Fulfillment final {
    std::string m_id{};
    std::string m_order_id{};
    std::string m_status{"pending"};
    std::string m_tracking_number{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Fulfillment() noexcept = default;
    Fulfillment(std::string&& id, std::string&& order_id) noexcept
        : m_id(std::move(id)), m_order_id(std::move(order_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_order_id() noexcept { return m_order_id; }
    const std::string& get_order_id() const noexcept { return m_order_id; }
    std::string& get_status() noexcept { return m_status; }
    const std::string& get_status() const noexcept { return m_status; }
    std::string& get_tracking_number() noexcept { return m_tracking_number; }
    const std::string& get_tracking_number() const noexcept { return m_tracking_number; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_order_id(std::string&& order_id) noexcept { m_order_id = std::move(order_id); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_tracking_number(std::string&& tracking_number) noexcept { m_tracking_number = std::move(tracking_number); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "Fulfillment"; }
    const char* table_name() const noexcept { return "fulfillment"; }
};

// FulfillmentItem entity
struct FulfillmentItem final {
    std::string m_id{};
    std::string m_fulfillment_id{};
    std::string m_line_item_id{};
    int64_t m_quantity{0};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    FulfillmentItem() noexcept = default;
    FulfillmentItem(std::string&& id, std::string&& fulfillment_id, std::string&& line_item_id) noexcept
        : m_id(std::move(id)), m_fulfillment_id(std::move(fulfillment_id)), m_line_item_id(std::move(line_item_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_fulfillment_id() noexcept { return m_fulfillment_id; }
    const std::string& get_fulfillment_id() const noexcept { return m_fulfillment_id; }
    std::string& get_line_item_id() noexcept { return m_line_item_id; }
    const std::string& get_line_item_id() const noexcept { return m_line_item_id; }
    int64_t& get_quantity() noexcept { return m_quantity; }
    const int64_t& get_quantity() const noexcept { return m_quantity; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_fulfillment_id(std::string&& fulfillment_id) noexcept { m_fulfillment_id = std::move(fulfillment_id); }
    void set_line_item_id(std::string&& line_item_id) noexcept { m_line_item_id = std::move(line_item_id); }
    void set_quantity(int64_t quantity) noexcept { m_quantity = quantity; }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "FulfillmentItem"; }
    const char* table_name() const noexcept { return "fulfillment_item"; }
};

// ShippingProfile entity
struct ShippingProfile final {
    std::string m_id{};
    std::string m_name{};
    std::string m_type{"default"};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    ShippingProfile() noexcept = default;
    ShippingProfile(std::string&& id, std::string&& name) noexcept
        : m_id(std::move(id)), m_name(std::move(name)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_name() noexcept { return m_name; }
    const std::string& get_name() const noexcept { return m_name; }
    std::string& get_type() noexcept { return m_type; }
    const std::string& get_type() const noexcept { return m_type; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_name(std::string&& name) noexcept { m_name = std::move(name); }
    void set_type(std::string&& type) noexcept { m_type = std::move(type); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    const char* entity_name() const noexcept { return "ShippingProfile"; }
    const char* table_name() const noexcept { return "shipping_profile"; }
};

// ShippingOption entity
struct ShippingOption final {
    std::string m_id{};
    std::string m_name{};
    std::string m_region_id{};
    std::string m_provider_id{};
    std::string m_profile_id{};
    std::string m_price_type{"flat"};
    std::string m_amount{"0"};
    bool m_is_return{false};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    ShippingOption() noexcept = default;
    ShippingOption(std::string&& id, std::string&& name, std::string&& region_id) noexcept
        : m_id(std::move(id)), m_name(std::move(name)), m_region_id(std::move(region_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_name() noexcept { return m_name; }
    const std::string& get_name() const noexcept { return m_name; }
    std::string& get_region_id() noexcept { return m_region_id; }
    const std::string& get_region_id() const noexcept { return m_region_id; }
    std::string& get_provider_id() noexcept { return m_provider_id; }
    const std::string& get_provider_id() const noexcept { return m_provider_id; }
    std::string& get_profile_id() noexcept { return m_profile_id; }
    const std::string& get_profile_id() const noexcept { return m_profile_id; }
    std::string& get_price_type() noexcept { return m_price_type; }
    const std::string& get_price_type() const noexcept { return m_price_type; }
    std::string& get_amount() noexcept { return m_amount; }
    const std::string& get_amount() const noexcept { return m_amount; }
    bool& get_is_return() noexcept { return m_is_return; }
    const bool& get_is_return() const noexcept { return m_is_return; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_name(std::string&& name) noexcept { m_name = std::move(name); }
    void set_region_id(std::string&& region_id) noexcept { m_region_id = std::move(region_id); }
    void set_provider_id(std::string&& provider_id) noexcept { m_provider_id = std::move(provider_id); }
    void set_profile_id(std::string&& profile_id) noexcept { m_profile_id = std::move(profile_id); }
    void set_price_type(std::string&& price_type) noexcept { m_price_type = std::move(price_type); }
    void set_amount(std::string&& amount) noexcept { m_amount = std::move(amount); }
    void set_is_return(bool is_return) noexcept { m_is_return = is_return; }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    const char* entity_name() const noexcept { return "ShippingOption"; }
    const char* table_name() const noexcept { return "shipping_option"; }
};

// PaymentCollection entity
struct PaymentCollection final {
    std::string m_id{};
    std::string m_region_id{};
    std::string m_currency_code{};
    std::string m_status{"pending"};
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
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_region_id(std::string&& region_id) noexcept { m_region_id = std::move(region_id); }
    void set_currency_code(std::string&& currency_code) noexcept { m_currency_code = std::move(currency_code); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "PaymentCollection"; }
    const char* table_name() const noexcept { return "payment_collection"; }
};

// PaymentSession entity
struct PaymentSession final {
    std::string m_id{};
    std::string m_payment_collection_id{};
    std::string m_provider_id{};
    std::string m_status{"pending"};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    PaymentSession() noexcept = default;
    PaymentSession(std::string&& id, std::string&& payment_collection_id, std::string&& provider_id) noexcept
        : m_id(std::move(id)), m_payment_collection_id(std::move(payment_collection_id)), m_provider_id(std::move(provider_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_payment_collection_id() noexcept { return m_payment_collection_id; }
    const std::string& get_payment_collection_id() const noexcept { return m_payment_collection_id; }
    std::string& get_provider_id() noexcept { return m_provider_id; }
    const std::string& get_provider_id() const noexcept { return m_provider_id; }
    std::string& get_status() noexcept { return m_status; }
    const std::string& get_status() const noexcept { return m_status; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_payment_collection_id(std::string&& payment_collection_id) noexcept { m_payment_collection_id = std::move(payment_collection_id); }
    void set_provider_id(std::string&& provider_id) noexcept { m_provider_id = std::move(provider_id); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "PaymentSession"; }
    const char* table_name() const noexcept { return "payment_session"; }
};

// Payment entity
struct Payment final {
    std::string m_id{};
    std::string m_payment_collection_id{};
    std::string m_provider_id{};
    std::string m_amount{"0"};
    std::string m_currency_code{};
    std::string m_status{"pending"};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Payment() noexcept = default;
    Payment(std::string&& id, std::string&& payment_collection_id, std::string&& provider_id) noexcept
        : m_id(std::move(id)), m_payment_collection_id(std::move(payment_collection_id)), m_provider_id(std::move(provider_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_payment_collection_id() noexcept { return m_payment_collection_id; }
    const std::string& get_payment_collection_id() const noexcept { return m_payment_collection_id; }
    std::string& get_provider_id() noexcept { return m_provider_id; }
    const std::string& get_provider_id() const noexcept { return m_provider_id; }
    std::string& get_amount() noexcept { return m_amount; }
    const std::string& get_amount() const noexcept { return m_amount; }
    std::string& get_currency_code() noexcept { return m_currency_code; }
    const std::string& get_currency_code() const noexcept { return m_currency_code; }
    std::string& get_status() noexcept { return m_status; }
    const std::string& get_status() const noexcept { return m_status; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_payment_collection_id(std::string&& payment_collection_id) noexcept { m_payment_collection_id = std::move(payment_collection_id); }
    void set_provider_id(std::string&& provider_id) noexcept { m_provider_id = std::move(provider_id); }
    void set_amount(std::string&& amount) noexcept { m_amount = std::move(amount); }
    void set_currency_code(std::string&& currency_code) noexcept { m_currency_code = std::move(currency_code); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "Payment"; }
    const char* table_name() const noexcept { return "payment"; }
};

// Refund entity
struct Refund final {
    std::string m_id{};
    std::string m_payment_id{};
    std::string m_amount{"0"};
    std::string m_currency_code{};
    std::string m_reason{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Refund() noexcept = default;
    Refund(std::string&& id, std::string&& payment_id) noexcept
        : m_id(std::move(id)), m_payment_id(std::move(payment_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_payment_id() noexcept { return m_payment_id; }
    const std::string& get_payment_id() const noexcept { return m_payment_id; }
    std::string& get_amount() noexcept { return m_amount; }
    const std::string& get_amount() const noexcept { return m_amount; }
    std::string& get_currency_code() noexcept { return m_currency_code; }
    const std::string& get_currency_code() const noexcept { return m_currency_code; }
    std::string& get_reason() noexcept { return m_reason; }
    const std::string& get_reason() const noexcept { return m_reason; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_payment_id(std::string&& payment_id) noexcept { m_payment_id = std::move(payment_id); }
    void set_amount(std::string&& amount) noexcept { m_amount = std::move(amount); }
    void set_currency_code(std::string&& currency_code) noexcept { m_currency_code = std::move(currency_code); }
    void set_reason(std::string&& reason) noexcept { m_reason = std::move(reason); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "Refund"; }
    const char* table_name() const noexcept { return "refund"; }
};

// Promotion entity
struct Promotion final {
    std::string m_id{};
    std::string m_code{};
    std::string m_type{"percentage"};
    std::string m_value{"0"};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    Promotion() noexcept = default;
    Promotion(std::string&& id, std::string&& code) noexcept
        : m_id(std::move(id)), m_code(std::move(code)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_code() noexcept { return m_code; }
    const std::string& get_code() const noexcept { return m_code; }
    std::string& get_type() noexcept { return m_type; }
    const std::string& get_type() const noexcept { return m_type; }
    std::string& get_value() noexcept { return m_value; }
    const std::string& get_value() const noexcept { return m_value; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_code(std::string&& code) noexcept { m_code = std::move(code); }
    void set_type(std::string&& type) noexcept { m_type = std::move(type); }
    void set_value(std::string&& value) noexcept { m_value = std::move(value); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    const char* entity_name() const noexcept { return "Promotion"; }
    const char* table_name() const noexcept { return "promotion"; }
};

// Discount entity
struct Discount final {
    std::string m_id{};
    std::string m_code{};
    std::string m_type{"percentage"};
    std::string m_value{"0"};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    Discount() noexcept = default;
    Discount(std::string&& id, std::string&& code) noexcept
        : m_id(std::move(id)), m_code(std::move(code)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_code() noexcept { return m_code; }
    const std::string& get_code() const noexcept { return m_code; }
    std::string& get_type() noexcept { return m_type; }
    const std::string& get_type() const noexcept { return m_type; }
    std::string& get_value() noexcept { return m_value; }
    const std::string& get_value() const noexcept { return m_value; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_code(std::string&& code) noexcept { m_code = std::move(code); }
    void set_type(std::string&& type) noexcept { m_type = std::move(type); }
    void set_value(std::string&& value) noexcept { m_value = std::move(value); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    const char* entity_name() const noexcept { return "Discount"; }
    const char* table_name() const noexcept { return "discount"; }
};

// Campaign entity
struct Campaign final {
    std::string m_id{};
    std::string m_name{};
    std::chrono::system_clock::time_point m_starts_at{};
    std::chrono::system_clock::time_point m_ends_at{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    Campaign() noexcept = default;
    Campaign(std::string&& id, std::string&& name) noexcept
        : m_id(std::move(id)), m_name(std::move(name)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_name() noexcept { return m_name; }
    const std::string& get_name() const noexcept { return m_name; }
    std::chrono::system_clock::time_point& get_starts_at() noexcept { return m_starts_at; }
    const std::chrono::system_clock::time_point& get_starts_at() const noexcept { return m_starts_at; }
    std::chrono::system_clock::time_point& get_ends_at() noexcept { return m_ends_at; }
    const std::chrono::system_clock::time_point& get_ends_at() const noexcept { return m_ends_at; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_name(std::string&& name) noexcept { m_name = std::move(name); }
    void set_starts_at(std::chrono::system_clock::time_point&& starts_at) noexcept { m_starts_at = std::move(starts_at); }
    void set_ends_at(std::chrono::system_clock::time_point&& ends_at) noexcept { m_ends_at = std::move(ends_at); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    const char* entity_name() const noexcept { return "Campaign"; }
    const char* table_name() const noexcept { return "campaign"; }
};

// Setting entity
struct Setting final {
    std::string m_id{};
    std::string m_key{};
    std::string m_value{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Setting() noexcept = default;
    Setting(std::string&& id, std::string&& key, std::string&& value) noexcept
        : m_id(std::move(id)), m_key(std::move(key)), m_value(std::move(value)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_key() noexcept { return m_key; }
    const std::string& get_key() const noexcept { return m_key; }
    std::string& get_value() noexcept { return m_value; }
    const std::string& get_value() const noexcept { return m_value; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_key(std::string&& key) noexcept { m_key = std::move(key); }
    void set_value(std::string&& value) noexcept { m_value = std::move(value); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "Setting"; }
    const char* table_name() const noexcept { return "setting"; }
};

// Notification entity
struct Notification final {
    std::string m_id{};
    std::string m_to{};
    std::string m_subject{};
    std::string m_body{};
    std::string m_status{"pending"};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Notification() noexcept = default;
    Notification(std::string&& id, std::string&& to, std::string&& subject, std::string&& body) noexcept
        : m_id(std::move(id)), m_to(std::move(to)), m_subject(std::move(subject)), m_body(std::move(body)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_to() noexcept { return m_to; }
    const std::string& get_to() const noexcept { return m_to; }
    std::string& get_subject() noexcept { return m_subject; }
    const std::string& get_subject() const noexcept { return m_subject; }
    std::string& get_body() noexcept { return m_body; }
    const std::string& get_body() const noexcept { return m_body; }
    std::string& get_status() noexcept { return m_status; }
    const std::string& get_status() const noexcept { return m_status; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_to(std::string&& to) noexcept { m_to = std::move(to); }
    void set_subject(std::string&& subject) noexcept { m_subject = std::move(subject); }
    void set_body(std::string&& body) noexcept { m_body = std::move(body); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "Notification"; }
    const char* table_name() const noexcept { return "notification"; }
};

// File entity
struct File final {
    std::string m_id{};
    std::string m_url{};
    std::string m_mime_type{};
    int64_t m_size{0};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    File() noexcept = default;
    File(std::string&& id, std::string&& url) noexcept
        : m_id(std::move(id)), m_url(std::move(url)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_url() noexcept { return m_url; }
    const std::string& get_url() const noexcept { return m_url; }
    std::string& get_mime_type() noexcept { return m_mime_type; }
    const std::string& get_mime_type() const noexcept { return m_mime_type; }
    int64_t& get_size() noexcept { return m_size; }
    const int64_t& get_size() const noexcept { return m_size; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_url(std::string&& url) noexcept { m_url = std::move(url); }
    void set_mime_type(std::string&& mime_type) noexcept { m_mime_type = std::move(mime_type); }
    void set_size(int64_t size) noexcept { m_size = size; }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "File"; }
    const char* table_name() const noexcept { return "file"; }
};

// WorkflowExecution entity
struct WorkflowExecution final {
    std::string m_id{};
    std::string m_workflow_id{};
    std::string m_status{"pending"};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    WorkflowExecution() noexcept = default;
    WorkflowExecution(std::string&& id, std::string&& workflow_id) noexcept
        : m_id(std::move(id)), m_workflow_id(std::move(workflow_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_workflow_id() noexcept { return m_workflow_id; }
    const std::string& get_workflow_id() const noexcept { return m_workflow_id; }
    std::string& get_status() noexcept { return m_status; }
    const std::string& get_status() const noexcept { return m_status; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_workflow_id(std::string&& workflow_id) noexcept { m_workflow_id = std::move(workflow_id); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "WorkflowExecution"; }
    const char* table_name() const noexcept { return "workflow_execution"; }
};

// Event entity
struct Event final {
    std::string m_id{};
    std::string m_name{};
    std::string m_payload{};
    std::chrono::system_clock::time_point m_created_at{};
    
    Event() noexcept = default;
    Event(std::string&& id, std::string&& name, std::string&& payload) noexcept
        : m_id(std::move(id)), m_name(std::move(name)), m_payload(std::move(payload)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_name() noexcept { return m_name; }
    const std::string& get_name() const noexcept { return m_name; }
    std::string& get_payload() noexcept { return m_payload; }
    const std::string& get_payload() const noexcept { return m_payload; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_name(std::string&& name) noexcept { m_name = std::move(name); }
    void set_payload(std::string&& payload) noexcept { m_payload = std::move(payload); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    
    const char* entity_name() const noexcept { return "Event"; }
    const char* table_name() const noexcept { return "event"; }
};

// Lock entity
struct Lock final {
    std::string m_id{};
    std::string m_resource{};
    std::string m_owner{};
    std::chrono::system_clock::time_point m_expires_at{};
    std::chrono::system_clock::time_point m_created_at{};
    
    Lock() noexcept = default;
    Lock(std::string&& id, std::string&& resource, std::string&& owner) noexcept
        : m_id(std::move(id)), m_resource(std::move(resource)), m_owner(std::move(owner)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_resource() noexcept { return m_resource; }
    const std::string& get_resource() const noexcept { return m_resource; }
    std::string& get_owner() noexcept { return m_owner; }
    const std::string& get_owner() const noexcept { return m_owner; }
    std::chrono::system_clock::time_point& get_expires_at() noexcept { return m_expires_at; }
    const std::chrono::system_clock::time_point& get_expires_at() const noexcept { return m_expires_at; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_resource(std::string&& resource) noexcept { m_resource = std::move(resource); }
    void set_owner(std::string&& owner) noexcept { m_owner = std::move(owner); }
    void set_expires_at(std::chrono::system_clock::time_point&& expires_at) noexcept { m_expires_at = std::move(expires_at); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    
    const char* entity_name() const noexcept { return "Lock"; }
    const char* table_name() const noexcept { return "lock"; }
};

// SearchIndex entity
struct SearchIndex final {
    std::string m_id{};
    std::string m_entity_type{};
    std::string m_entity_id{};
    std::string m_data{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    SearchIndex() noexcept = default;
    SearchIndex(std::string&& id, std::string&& entity_type, std::string&& entity_id) noexcept
        : m_id(std::move(id)), m_entity_type(std::move(entity_type)), m_entity_id(std::move(entity_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_entity_type() noexcept { return m_entity_type; }
    const std::string& get_entity_type() const noexcept { return m_entity_type; }
    std::string& get_entity_id() noexcept { return m_entity_id; }
    const std::string& get_entity_id() const noexcept { return m_entity_id; }
    std::string& get_data() noexcept { return m_data; }
    const std::string& get_data() const noexcept { return m_data; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_entity_type(std::string&& entity_type) noexcept { m_entity_type = std::move(entity_type); }
    void set_entity_id(std::string&& entity_id) noexcept { m_entity_id = std::move(entity_id); }
    void set_data(std::string&& data) noexcept { m_data = std::move(data); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "SearchIndex"; }
    const char* table_name() const noexcept { return "search_index"; }
};

// TaxRate entity
struct TaxRate final {
    std::string m_id{};
    std::string m_rate{"0"};
    std::string m_code{};
    std::string m_name{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    TaxRate() noexcept = default;
    TaxRate(std::string&& id, std::string&& code, std::string&& name) noexcept
        : m_id(std::move(id)), m_code(std::move(code)), m_name(std::move(name)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_rate() noexcept { return m_rate; }
    const std::string& get_rate() const noexcept { return m_rate; }
    std::string& get_code() noexcept { return m_code; }
    const std::string& get_code() const noexcept { return m_code; }
    std::string& get_name() noexcept { return m_name; }
    const std::string& get_name() const noexcept { return m_name; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_rate(std::string&& rate) noexcept { m_rate = std::move(rate); }
    void set_code(std::string&& code) noexcept { m_code = std::move(code); }
    void set_name(std::string&& name) noexcept { m_name = std::move(name); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    const char* entity_name() const noexcept { return "TaxRate"; }
    const char* table_name() const noexcept { return "tax_rate"; }
};

// TaxRegion entity
struct TaxRegion final {
    std::string m_id{};
    std::string m_country_code{};
    std::string m_province_code{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    TaxRegion() noexcept = default;
    TaxRegion(std::string&& id, std::string&& country_code) noexcept
        : m_id(std::move(id)), m_country_code(std::move(country_code)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_country_code() noexcept { return m_country_code; }
    const std::string& get_country_code() const noexcept { return m_country_code; }
    std::string& get_province_code() noexcept { return m_province_code; }
    const std::string& get_province_code() const noexcept { return m_province_code; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_country_code(std::string&& country_code) noexcept { m_country_code = std::move(country_code); }
    void set_province_code(std::string&& province_code) noexcept { m_province_code = std::move(province_code); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    const char* entity_name() const noexcept { return "TaxRegion"; }
    const char* table_name() const noexcept { return "tax_region"; }
};

// PriceList entity
struct PriceList final {
    std::string m_id{};
    std::string m_title{};
    std::string m_type{"sale"};
    std::string m_status{"active"};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    PriceList() noexcept = default;
    PriceList(std::string&& id, std::string&& title) noexcept
        : m_id(std::move(id)), m_title(std::move(title)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_title() noexcept { return m_title; }
    const std::string& get_title() const noexcept { return m_title; }
    std::string& get_type() noexcept { return m_type; }
    const std::string& get_type() const noexcept { return m_type; }
    std::string& get_status() noexcept { return m_status; }
    const std::string& get_status() const noexcept { return m_status; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_title(std::string&& title) noexcept { m_title = std::move(title); }
    void set_type(std::string&& type) noexcept { m_type = std::move(type); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    const char* entity_name() const noexcept { return "PriceList"; }
    const char* table_name() const noexcept { return "price_list"; }
};

// Price entity
struct Price final {
    std::string m_id{};
    std::string m_price_list_id{};
    std::string m_variant_id{};
    std::string m_amount{"0"};
    std::string m_currency_code{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Price() noexcept = default;
    Price(std::string&& id, std::string&& price_list_id, std::string&& variant_id) noexcept
        : m_id(std::move(id)), m_price_list_id(std::move(price_list_id)), m_variant_id(std::move(variant_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_price_list_id() noexcept { return m_price_list_id; }
    const std::string& get_price_list_id() const noexcept { return m_price_list_id; }
    std::string& get_variant_id() noexcept { return m_variant_id; }
    const std::string& get_variant_id() const noexcept { return m_variant_id; }
    std::string& get_amount() noexcept { return m_amount; }
    const std::string& get_amount() const noexcept { return m_amount; }
    std::string& get_currency_code() noexcept { return m_currency_code; }
    const std::string& get_currency_code() const noexcept { return m_currency_code; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_price_list_id(std::string&& price_list_id) noexcept { m_price_list_id = std::move(price_list_id); }
    void set_variant_id(std::string&& variant_id) noexcept { m_variant_id = std::move(variant_id); }
    void set_amount(std::string&& amount) noexcept { m_amount = std::move(amount); }
    void set_currency_code(std::string&& currency_code) noexcept { m_currency_code = std::move(currency_code); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "Price"; }
    const char* table_name() const noexcept { return "price"; }
};

// ReturnItem entity
struct ReturnItem final {
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
    void set_return_id(std::string&& return_id) noexcept { m_return_id = std::move(return_id); }
    void set_line_item_id(std::string&& line_item_id) noexcept { m_line_item_id = std::move(line_item_id); }
    void set_quantity(int64_t quantity) noexcept { m_quantity = quantity; }
    void set_reason(std::string&& reason) noexcept { m_reason = std::move(reason); }
    void set_note(std::string&& note) noexcept { m_note = std::move(note); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "ReturnItem"; }
    const char* table_name() const noexcept { return "return_item"; }
};

// ExchangeItem entity
struct ExchangeItem final {
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
    void set_exchange_id(std::string&& exchange_id) noexcept { m_exchange_id = std::move(exchange_id); }
    void set_line_item_id(std::string&& line_item_id) noexcept { m_line_item_id = std::move(line_item_id); }
    void set_quantity(int64_t quantity) noexcept { m_quantity = quantity; }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "ExchangeItem"; }
    const char* table_name() const noexcept { return "exchange_item"; }
};

// ClaimItem entity
struct ClaimItem final {
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
    void set_claim_id(std::string&& claim_id) noexcept { m_claim_id = std::move(claim_id); }
    void set_line_item_id(std::string&& line_item_id) noexcept { m_line_item_id = std::move(line_item_id); }
    void set_quantity(int64_t quantity) noexcept { m_quantity = quantity; }
    void set_reason(std::string&& reason) noexcept { m_reason = std::move(reason); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    const char* entity_name() const noexcept { return "ClaimItem"; }
    const char* table_name() const noexcept { return "claim_item"; }
};

// FulfillmentSet entity
struct FulfillmentSet final {
    std::string m_id{};
    std::string m_name{};
    std::string m_type{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    FulfillmentSet() noexcept = default;
    FulfillmentSet(std::string&& id, std::string&& name, std::string&& type) noexcept
        : m_id(std::move(id)), m_name(std::move(name)), m_type(std::move(type)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_name() noexcept { return m_name; }
    const std::string& get_name() const noexcept { return m_name; }
    std::string& get_type() noexcept { return m_type; }
    const std::string& get_type() const noexcept { return m_type; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_name(std::string&& name) noexcept { m_name = std::move(name); }
    void set_type(std::string&& type) noexcept { m_type = std::move(type); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    const char* entity_name() const noexcept { return "FulfillmentSet"; }
    const char* table_name() const noexcept { return "fulfillment_set"; }
};

// FulfillmentProvider entity
struct FulfillmentProvider final {
    std::string m_id{};
    std::string m_name{};
    std::string m_type{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    FulfillmentProvider() noexcept = default;
    FulfillmentProvider(std::string&& id, std::string&& name, std::string&& type) noexcept
        : m_id(std::move(id)), m_name(std::move(name)), m_type(std::move(type)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_name() noexcept { return m_name; }
    const std::string& get_name() const noexcept { return m_name; }
    std::string& get_type() noexcept { return m_type; }
    const std::string& get_type() const noexcept { return m_type; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_name(std::string&& name) noexcept { m_name = std::move(name); }
    void set_type(std::string&& type) noexcept { m_type = std::move(type); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    const char* entity_name() const noexcept { return "FulfillmentProvider"; }
    const char* table_name() const noexcept { return "fulfillment_provider"; }
};

// ShippingOptionType entity
struct ShippingOptionType final {
    std::string m_id{};
    std::string m_label{};
    std::string m_description{};
    std::string m_code{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    ShippingOptionType() noexcept = default;
    ShippingOptionType(std::string&& id, std::string&& label, std::string&& description, std::string&& code) noexcept
        : m_id(std::move(id)), m_label(std::move(label)), m_description(std::move(description)), m_code(std::move(code)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_label() noexcept { return m_label; }
    const std::string& get_label() const noexcept { return m_label; }
    std::string& get_description() noexcept { return m_description; }
    const std::string& get_description() const noexcept { return m_description; }
    std::string& get_code() noexcept { return m_code; }
    const std::string& get_code() const noexcept { return m_code; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_label(std::string&& label) noexcept { m_label = std::move(label); }
    void set_description(std::string&& description) noexcept { m_description = std::move(description); }
    void set_code(std::string&& code) noexcept { m_code = std::move(code); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    const char* entity_name() const noexcept { return "ShippingOptionType"; }
    const char* table_name() const noexcept { return "shipping_option_type"; }
};

// DraftOrder entity
struct DraftOrder final {
    std::string m_id{};
    std::string m_customer_id{};
    std::string m_email{};
    std::string m_region_id{};
    std::string m_currency_code{};
    std::string m_status{"open"};
    std::vector<LineItem> m_items{};
    std::vector<ShippingMethod> m_shipping_methods{};
    std::vector<Discount> m_discounts{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_completed_at{};
    std::map<std::string, std::string> m_metadata{};
    
    DraftOrder() noexcept = default;
    DraftOrder(std::string&& id) noexcept : m_id(std::move(id)) {}
    
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
    std::vector<LineItem>& get_items() noexcept { return m_items; }
    const std::vector<LineItem>& get_items() const noexcept { return m_items; }
    std::vector<ShippingMethod>& get_shipping_methods() noexcept { return m_shipping_methods; }
    const std::vector<ShippingMethod>& get_shipping_methods() const noexcept { return m_shipping_methods; }
    std::vector<Discount>& get_discounts() noexcept { return m_discounts; }
    const std::vector<Discount>& get_discounts() const noexcept { return m_discounts; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_completed_at() noexcept { return m_completed_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_completed_at() const noexcept { return m_completed_at; }
    std::map<std::string, std::string>& get_metadata() noexcept { return m_metadata; }
    const std::map<std::string, std::string>& get_metadata() const noexcept { return m_metadata; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_customer_id(std::string&& customer_id) noexcept { m_customer_id = std::move(customer_id); }
    void set_email(std::string&& email) noexcept { m_email = std::move(email); }
    void set_region_id(std::string&& region_id) noexcept { m_region_id = std::move(region_id); }
    void set_currency_code(std::string&& currency_code) noexcept { m_currency_code = std::move(currency_code); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_items(std::vector<LineItem>&& items) noexcept { m_items = std::move(items); }
    void set_shipping_methods(std::vector<ShippingMethod>&& methods) noexcept { m_shipping_methods = std::move(methods); }
    void set_discounts(std::vector<Discount>&& discounts) noexcept { m_discounts = std::move(discounts); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_completed_at(std::optional<std::chrono::system_clock::time_point>&& completed_at) noexcept { m_completed_at = std::move(completed_at); }
    void set_metadata(std::map<std::string, std::string>&& meta) noexcept { m_metadata = std::move(meta); }
    
    const char* entity_name() const noexcept { return "DraftOrder"; }
    const char* table_name() const noexcept { return "draft_order"; }
};

// OrderEdit entity
struct OrderEdit final {
    std::string m_id{};
    std::string m_order_id{};
    std::string m_status{"pending"};
    std::vector<LineItem> m_items{};
    std::vector<ShippingMethod> m_shipping_methods{};
    std::vector<Discount> m_discounts{};
    std::vector<PaymentCollection> m_payment_collections{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_completed_at{};
    std::optional<std::chrono::system_clock::time_point> m_canceled_at{};
    std::map<std::string, std::string> m_metadata{};
    
    OrderEdit() noexcept = default;
    OrderEdit(std::string&& id, std::string&& order_id) noexcept
        : m_id(std::move(id)), m_order_id(std::move(order_id)) {}
    
    std::string& get_id() noexcept { return m_id; }
    const std::string& get_id() const noexcept { return m_id; }
    std::string& get_order_id() noexcept { return m_order_id; }
    const std::string& get_order_id() const noexcept { return m_order_id; }
    std::string& get_status() noexcept { return m_status; }
    const std::string& get_status() const noexcept { return m_status; }
    std::vector<LineItem>& get_items() noexcept { return m_items; }
    const std::vector<LineItem>& get_items() const noexcept { return m_items; }
    std::vector<ShippingMethod>& get_shipping_methods() noexcept { return m_shipping_methods; }
    const std::vector<ShippingMethod>& get_shipping_methods() const noexcept { return m_shipping_methods; }
    std::vector<Discount>& get_discounts() noexcept { return m_discounts; }
    const std::vector<Discount>& get_discounts() const noexcept { return m_discounts; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_completed_at() noexcept { return m_completed_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_completed_at() const noexcept { return m_completed_at; }
    std::optional<std::chrono::system_clock::time_point>& get_canceled_at() noexcept { return m_canceled_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_canceled_at() const noexcept { return m_canceled_at; }
    std::map<std::string, std::string>& get_metadata() noexcept { return m_metadata; }
    const std::map<std::string, std::string>& get_metadata() const noexcept { return m_metadata; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_order_id(std::string&& order_id) noexcept { m_order_id = std::move(order_id); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_items(std::vector<LineItem>&& items) noexcept { m_items = std::move(items); }
    void set_shipping_methods(std::vector<ShippingMethod>&& methods) noexcept { m_shipping_methods = std::move(methods); }
    void set_discounts(std::vector<Discount>&& discounts) noexcept { m_discounts = std::move(discounts); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_completed_at(std::optional<std::chrono::system_clock::time_point>&& completed_at) noexcept { m_completed_at = std::move(completed_at); }
    void set_canceled_at(std::optional<std::chrono::system_clock::time_point>&& canceled_at) noexcept { m_canceled_at = std::move(canceled_at); }
    void set_metadata(std::map<std::string, std::string>&& meta) noexcept { m_metadata = std::move(meta); }
    
    const char* entity_name() const noexcept { return "OrderEdit"; }
    const char* table_name() const noexcept { return "order_edit"; }
};

} // namespace medusa_core::entities