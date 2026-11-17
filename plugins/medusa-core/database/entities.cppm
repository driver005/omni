export module medusa_core.database.entities;

import std;
import database.types;
import reflection.entity_traits;

export namespace medusa_core::entities {

struct Product final {
    using IdType = std::string;
    
    // Member initialization
    std::string m_id{};
    std::string m_title{};
    std::string m_description{};
    std::string m_handle{};
    std::string m_status{"draft"};
    int64_t m_weight{0};
    std::string m_thumbnail{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    // 1. Default constructor
    Product() noexcept = default;
    
    // 2. Plain constructor (required fields only)
    Product(std::string&& id, std::string&& title, std::string&& description) noexcept
        : m_id(std::move(id))
        , m_title(std::move(title))
        , m_description(std::move(description))
    {}
    
    // 3. Full constructor (all fields as rvalues moved in)
    Product(
        std::string&& id,
        std::string&& title,
        std::string&& description,
        std::string&& handle,
        std::string&& status,
        int64_t&& weight,
        std::string&& thumbnail,
        std::chrono::system_clock::time_point&& created_at,
        std::chrono::system_clock::time_point&& updated_at,
        std::optional<std::chrono::system_clock::time_point>&& deleted_at
    ) noexcept
        : m_id(std::move(id))
        , m_title(std::move(title))
        , m_description(std::move(description))
        , m_handle(std::move(handle))
        , m_status(std::move(status))
        , m_weight(std::move(weight))
        , m_thumbnail(std::move(thumbnail))
        , m_created_at(std::move(created_at))
        , m_updated_at(std::move(updated_at))
        , m_deleted_at(std::move(deleted_at))
    {}
    
    // 4. Adder functions (fluent interface)
    Product& add_id(std::string&& id) noexcept { m_id = std::move(id); return *this; }
    Product& add_title(std::string&& title) noexcept { m_title = std::move(title); return *this; }
    Product& add_description(std::string&& description) noexcept { m_description = std::move(description); return *this; }
    Product& add_handle(std::string&& handle) noexcept { m_handle = std::move(handle); return *this; }
    Product& add_status(std::string&& status) noexcept { m_status = std::move(status); return *this; }
    Product& add_weight(int64_t&& weight) noexcept { m_weight = std::move(weight); return *this; }
    Product& add_thumbnail(std::string&& thumbnail) noexcept { m_thumbnail = std::move(thumbnail); return *this; }
    Product& add_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); return *this; }
    Product& add_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); return *this; }
    Product& add_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); return *this; }
    
    // 5. Setters (rvalue reference)
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_title(std::string&& title) noexcept { m_title = std::move(title); }
    void set_description(std::string&& description) noexcept { m_description = std::move(description); }
    void set_handle(std::string&& handle) noexcept { m_handle = std::move(handle); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_weight(int64_t&& weight) noexcept { m_weight = std::move(weight); }
    void set_thumbnail(std::string&& thumbnail) noexcept { m_thumbnail = std::move(thumbnail); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    // 6. Getters (non-const)
    std::string& get_id() noexcept { return m_id; }
    std::string& get_title() noexcept { return m_title; }
    std::string& get_description() noexcept { return m_description; }
    std::string& get_handle() noexcept { return m_handle; }
    std::string& get_status() noexcept { return m_status; }
    int64_t& get_weight() noexcept { return m_weight; }
    std::string& get_thumbnail() noexcept { return m_thumbnail; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    
    // 7. Const getters
    const std::string& get_id() const noexcept { return m_id; }
    const std::string& get_title() const noexcept { return m_title; }
    const std::string& get_description() const noexcept { return m_description; }
    const std::string& get_handle() const noexcept { return m_handle; }
    const std::string& get_status() const noexcept { return m_status; }
    const int64_t& get_weight() const noexcept { return m_weight; }
    const std::string& get_thumbnail() const noexcept { return m_thumbnail; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    // Entity interface
    const char* entity_name() const noexcept { return "Product"; }
    const char* table_name() const noexcept { return "product"; }
    
    using TableMeta = reflection::Table<"product", "public",
        reflection::Column<"id", database::ColumnType::UUID, true, false, false>,
        reflection::Column<"title", database::ColumnType::TEXT>,
        reflection::Column<"description", database::ColumnType::TEXT>,
        reflection::Column<"handle", database::ColumnType::TEXT, false, true, false>,
        reflection::ColumnWithDefault<"status", database::ColumnType::TEXT, false, true, false, "draft">,
        reflection::ColumnWithDefault<"weight", database::ColumnType::BIGINT, false, true, false, "0">,
        reflection::Column<"thumbnail", database::ColumnType::TEXT, false, true, false>,
        reflection::ColumnWithDefault<"created_at", database::ColumnType::TIMESTAMPTZ, false, true, false, "NOW()">,
        reflection::ColumnWithDefault<"updated_at", database::ColumnType::TIMESTAMPTZ, false, true, false, "NOW()">,
        reflection::Column<"deleted_at", database::ColumnType::TIMESTAMPTZ, false, true, false>,
        reflection::Index<"idx_product_handle", "handle">,
        reflection::Index<"idx_product_status", "status">,
        reflection::UniqueIndex<"uq_product_handle", "handle">
    >;
};

struct ProductVariant final {
    std::string m_id{};
    std::string m_product_id{};
    std::string m_title{};
    std::string m_sku{};
    std::string m_barcode{};
    std::string m_price{"0"};
    int64_t m_inventory_quantity{0};
    bool m_allow_backorder{false};
    bool m_manage_inventory{true};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    ProductVariant() noexcept = default;
    
    ProductVariant(std::string&& id, std::string&& product_id, std::string&& title) noexcept
        : m_id(std::move(id))
        , m_product_id(std::move(product_id))
        , m_title(std::move(title))
    {}
    
    ProductVariant(
        std::string&& id,
        std::string&& product_id,
        std::string&& title,
        std::string&& sku,
        std::string&& barcode,
        std::string&& price,
        int64_t&& inventory_quantity,
        bool&& allow_backorder,
        bool&& manage_inventory,
        std::chrono::system_clock::time_point&& created_at,
        std::chrono::system_clock::time_point&& updated_at,
        std::optional<std::chrono::system_clock::time_point>&& deleted_at
    ) noexcept
        : m_id(std::move(id))
        , m_product_id(std::move(product_id))
        , m_title(std::move(title))
        , m_sku(std::move(sku))
        , m_barcode(std::move(barcode))
        , m_price(std::move(price))
        , m_inventory_quantity(std::move(inventory_quantity))
        , m_allow_backorder(std::move(allow_backorder))
        , m_manage_inventory(std::move(manage_inventory))
        , m_created_at(std::move(created_at))
        , m_updated_at(std::move(updated_at))
        , m_deleted_at(std::move(deleted_at))
    {}
    
    ProductVariant& add_id(std::string&& id) noexcept { m_id = std::move(id); return *this; }
    ProductVariant& add_product_id(std::string&& product_id) noexcept { m_product_id = std::move(product_id); return *this; }
    ProductVariant& add_title(std::string&& title) noexcept { m_title = std::move(title); return *this; }
    ProductVariant& add_sku(std::string&& sku) noexcept { m_sku = std::move(sku); return *this; }
    ProductVariant& add_barcode(std::string&& barcode) noexcept { m_barcode = std::move(barcode); return *this; }
    ProductVariant& add_price(std::string&& price) noexcept { m_price = std::move(price); return *this; }
    ProductVariant& add_inventory_quantity(int64_t&& inventory_quantity) noexcept { m_inventory_quantity = std::move(inventory_quantity); return *this; }
    ProductVariant& add_allow_backorder(bool&& allow_backorder) noexcept { m_allow_backorder = std::move(allow_backorder); return *this; }
    ProductVariant& add_manage_inventory(bool&& manage_inventory) noexcept { m_manage_inventory = std::move(manage_inventory); return *this; }
    ProductVariant& add_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); return *this; }
    ProductVariant& add_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); return *this; }
    ProductVariant& add_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); return *this; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_product_id(std::string&& product_id) noexcept { m_product_id = std::move(product_id); }
    void set_title(std::string&& title) noexcept { m_title = std::move(title); }
    void set_sku(std::string&& sku) noexcept { m_sku = std::move(sku); }
    void set_barcode(std::string&& barcode) noexcept { m_barcode = std::move(barcode); }
    void set_price(std::string&& price) noexcept { m_price = std::move(price); }
    void set_inventory_quantity(int64_t&& inventory_quantity) noexcept { m_inventory_quantity = std::move(inventory_quantity); }
    void set_allow_backorder(bool&& allow_backorder) noexcept { m_allow_backorder = std::move(allow_backorder); }
    void set_manage_inventory(bool&& manage_inventory) noexcept { m_manage_inventory = std::move(manage_inventory); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    std::string& get_id() noexcept { return m_id; }
    std::string& get_product_id() noexcept { return m_product_id; }
    std::string& get_title() noexcept { return m_title; }
    std::string& get_sku() noexcept { return m_sku; }
    std::string& get_barcode() noexcept { return m_barcode; }
    std::string& get_price() noexcept { return m_price; }
    int64_t& get_inventory_quantity() noexcept { return m_inventory_quantity; }
    bool& get_allow_backorder() noexcept { return m_allow_backorder; }
    bool& get_manage_inventory() noexcept { return m_manage_inventory; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    
    const std::string& get_id() const noexcept { return m_id; }
    const std::string& get_product_id() const noexcept { return m_product_id; }
    const std::string& get_title() const noexcept { return m_title; }
    const std::string& get_sku() const noexcept { return m_sku; }
    const std::string& get_barcode() const noexcept { return m_barcode; }
    const std::string& get_price() const noexcept { return m_price; }
    const int64_t& get_inventory_quantity() const noexcept { return m_inventory_quantity; }
    const bool& get_allow_backorder() const noexcept { return m_allow_backorder; }
    const bool& get_manage_inventory() const noexcept { return m_manage_inventory; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    const char* entity_name() const noexcept { return "ProductVariant"; }
    const char* table_name() const noexcept { return "product_variant"; }
    
    using TableMeta = reflection::Table<"product_variant", "public",
        reflection::Column<"id", database::ColumnType::UUID, true, false, false>,
        reflection::ColumnWithFK<"product_id", database::ColumnType::UUID, false, true, false, "product", "id">,
        reflection::Column<"title", database::ColumnType::TEXT>,
        reflection::Column<"sku", database::ColumnType::TEXT, false, true, false>,
        reflection::Column<"barcode", database::ColumnType::TEXT, false, true, false>,
        reflection::ColumnWithDefault<"price", database::ColumnType::NUMERIC, false, true, false, "0">,
        reflection::ColumnWithDefault<"inventory_quantity", database::ColumnType::BIGINT, false, true, false, "0">,
        reflection::ColumnWithDefault<"allow_backorder", database::ColumnType::BOOLEAN, false, true, false, "false">,
        reflection::ColumnWithDefault<"manage_inventory", database::ColumnType::BOOLEAN, false, true, false, "true">,
        reflection::ColumnWithDefault<"created_at", database::ColumnType::TIMESTAMPTZ, false, true, false, "NOW()">,
        reflection::ColumnWithDefault<"updated_at", database::ColumnType::TIMESTAMPTZ, false, true, false, "NOW()">,
        reflection::Column<"deleted_at", database::ColumnType::TIMESTAMPTZ, false, true, false>,
        reflection::Index<"idx_variant_product_id", "product_id">,
        reflection::UniqueIndex<"uq_variant_sku", "sku">
    >;
};

struct Customer final {
    std::string m_id{};
    std::string m_email{};
    std::string m_first_name{};
    std::string m_last_name{};
    std::string m_phone{};
    bool m_has_account{false};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    Customer() noexcept = default;
    
    Customer(std::string&& id, std::string&& email) noexcept
        : m_id(std::move(id))
        , m_email(std::move(email))
    {}
    
    Customer(
        std::string&& id,
        std::string&& email,
        std::string&& first_name,
        std::string&& last_name,
        std::string&& phone,
        bool&& has_account,
        std::chrono::system_clock::time_point&& created_at,
        std::chrono::system_clock::time_point&& updated_at,
        std::optional<std::chrono::system_clock::time_point>&& deleted_at
    ) noexcept
        : m_id(std::move(id))
        , m_email(std::move(email))
        , m_first_name(std::move(first_name))
        , m_last_name(std::move(last_name))
        , m_phone(std::move(phone))
        , m_has_account(std::move(has_account))
        , m_created_at(std::move(created_at))
        , m_updated_at(std::move(updated_at))
        , m_deleted_at(std::move(deleted_at))
    {}
    
    Customer& add_id(std::string&& id) noexcept { m_id = std::move(id); return *this; }
    Customer& add_email(std::string&& email) noexcept { m_email = std::move(email); return *this; }
    Customer& add_first_name(std::string&& first_name) noexcept { m_first_name = std::move(first_name); return *this; }
    Customer& add_last_name(std::string&& last_name) noexcept { m_last_name = std::move(last_name); return *this; }
    Customer& add_phone(std::string&& phone) noexcept { m_phone = std::move(phone); return *this; }
    Customer& add_has_account(bool&& has_account) noexcept { m_has_account = std::move(has_account); return *this; }
    Customer& add_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); return *this; }
    Customer& add_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); return *this; }
    Customer& add_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); return *this; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_email(std::string&& email) noexcept { m_email = std::move(email); }
    void set_first_name(std::string&& first_name) noexcept { m_first_name = std::move(first_name); }
    void set_last_name(std::string&& last_name) noexcept { m_last_name = std::move(last_name); }
    void set_phone(std::string&& phone) noexcept { m_phone = std::move(phone); }
    void set_has_account(bool&& has_account) noexcept { m_has_account = std::move(has_account); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    
    std::string& get_id() noexcept { return m_id; }
    std::string& get_email() noexcept { return m_email; }
    std::string& get_first_name() noexcept { return m_first_name; }
    std::string& get_last_name() noexcept { return m_last_name; }
    std::string& get_phone() noexcept { return m_phone; }
    bool& get_has_account() noexcept { return m_has_account; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    
    const std::string& get_id() const noexcept { return m_id; }
    const std::string& get_email() const noexcept { return m_email; }
    const std::string& get_first_name() const noexcept { return m_first_name; }
    const std::string& get_last_name() const noexcept { return m_last_name; }
    const std::string& get_phone() const noexcept { return m_phone; }
    const bool& get_has_account() const noexcept { return m_has_account; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    
    const char* entity_name() const noexcept { return "Customer"; }
    const char* table_name() const noexcept { return "customer"; }
    
    using TableMeta = reflection::Table<"customer", "public",
        reflection::Column<"id", database::ColumnType::UUID, true, false, false>,
        reflection::Column<"email", database::ColumnType::TEXT>,
        reflection::Column<"first_name", database::ColumnType::TEXT, false, true, false>,
        reflection::Column<"last_name", database::ColumnType::TEXT, false, true, false>,
        reflection::Column<"phone", database::ColumnType::TEXT, false, true, false>,
        reflection::ColumnWithDefault<"has_account", database::ColumnType::BOOLEAN, false, true, false, "false">,
        reflection::ColumnWithDefault<"created_at", database::ColumnType::TIMESTAMPTZ, false, true, false, "NOW()">,
        reflection::ColumnWithDefault<"updated_at", database::ColumnType::TIMESTAMPTZ, false, true, false, "NOW()">,
        reflection::Column<"deleted_at", database::ColumnType::TIMESTAMPTZ, false, true, false>,
        reflection::UniqueIndex<"uq_customer_email", "email">,
        reflection::Index<"idx_customer_phone", "phone">
    >;
};

struct Order final {
    std::string m_id{};
    std::string m_customer_id{};
    std::string m_region_id{};
    std::string m_currency_code{};
    std::string m_status{"pending"};
    std::string m_payment_status{"pending"};
    std::string m_fulfillment_status{"not_fulfilled"};
    std::string m_total{"0"};
    std::string m_subtotal{"0"};
    std::string m_tax_total{"0"};
    std::string m_shipping_total{"0"};
    std::string m_discount_total{"0"};
    std::string m_email{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    std::optional<std::chrono::system_clock::time_point> m_completed_at{};
    
    Order() noexcept = default;
    
    Order(std::string&& id, std::string&& customer_id, std::string&& region_id, std::string&& currency_code) noexcept
        : m_id(std::move(id))
        , m_customer_id(std::move(customer_id))
        , m_region_id(std::move(region_id))
        , m_currency_code(std::move(currency_code))
    {}
    
    Order(
        std::string&& id,
        std::string&& customer_id,
        std::string&& region_id,
        std::string&& currency_code,
        std::string&& status,
        std::string&& payment_status,
        std::string&& fulfillment_status,
        std::string&& total,
        std::string&& subtotal,
        std::string&& tax_total,
        std::string&& shipping_total,
        std::string&& discount_total,
        std::string&& email,
        std::chrono::system_clock::time_point&& created_at,
        std::chrono::system_clock::time_point&& updated_at,
        std::optional<std::chrono::system_clock::time_point>&& deleted_at,
        std::optional<std::chrono::system_clock::time_point>&& completed_at
    ) noexcept
        : m_id(std::move(id))
        , m_customer_id(std::move(customer_id))
        , m_region_id(std::move(region_id))
        , m_currency_code(std::move(currency_code))
        , m_status(std::move(status))
        , m_payment_status(std::move(payment_status))
        , m_fulfillment_status(std::move(fulfillment_status))
        , m_total(std::move(total))
        , m_subtotal(std::move(subtotal))
        , m_tax_total(std::move(tax_total))
        , m_shipping_total(std::move(shipping_total))
        , m_discount_total(std::move(discount_total))
        , m_email(std::move(email))
        , m_created_at(std::move(created_at))
        , m_updated_at(std::move(updated_at))
        , m_deleted_at(std::move(deleted_at))
        , m_completed_at(std::move(completed_at))
    {}
    
    Order& add_id(std::string&& id) noexcept { m_id = std::move(id); return *this; }
    Order& add_customer_id(std::string&& customer_id) noexcept { m_customer_id = std::move(customer_id); return *this; }
    Order& add_region_id(std::string&& region_id) noexcept { m_region_id = std::move(region_id); return *this; }
    Order& add_currency_code(std::string&& currency_code) noexcept { m_currency_code = std::move(currency_code); return *this; }
    Order& add_status(std::string&& status) noexcept { m_status = std::move(status); return *this; }
    Order& add_payment_status(std::string&& payment_status) noexcept { m_payment_status = std::move(payment_status); return *this; }
    Order& add_fulfillment_status(std::string&& fulfillment_status) noexcept { m_fulfillment_status = std::move(fulfillment_status); return *this; }
    Order& add_total(std::string&& total) noexcept { m_total = std::move(total); return *this; }
    Order& add_subtotal(std::string&& subtotal) noexcept { m_subtotal = std::move(subtotal); return *this; }
    Order& add_tax_total(std::string&& tax_total) noexcept { m_tax_total = std::move(tax_total); return *this; }
    Order& add_shipping_total(std::string&& shipping_total) noexcept { m_shipping_total = std::move(shipping_total); return *this; }
    Order& add_discount_total(std::string&& discount_total) noexcept { m_discount_total = std::move(discount_total); return *this; }
    Order& add_email(std::string&& email) noexcept { m_email = std::move(email); return *this; }
    Order& add_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); return *this; }
    Order& add_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); return *this; }
    Order& add_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); return *this; }
    Order& add_completed_at(std::optional<std::chrono::system_clock::time_point>&& completed_at) noexcept { m_completed_at = std::move(completed_at); return *this; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_customer_id(std::string&& customer_id) noexcept { m_customer_id = std::move(customer_id); }
    void set_region_id(std::string&& region_id) noexcept { m_region_id = std::move(region_id); }
    void set_currency_code(std::string&& currency_code) noexcept { m_currency_code = std::move(currency_code); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_payment_status(std::string&& payment_status) noexcept { m_payment_status = std::move(payment_status); }
    void set_fulfillment_status(std::string&& fulfillment_status) noexcept { m_fulfillment_status = std::move(fulfillment_status); }
    void set_total(std::string&& total) noexcept { m_total = std::move(total); }
    void set_subtotal(std::string&& subtotal) noexcept { m_subtotal = std::move(subtotal); }
    void set_tax_total(std::string&& tax_total) noexcept { m_tax_total = std::move(tax_total); }
    void set_shipping_total(std::string&& shipping_total) noexcept { m_shipping_total = std::move(shipping_total); }
    void set_discount_total(std::string&& discount_total) noexcept { m_discount_total = std::move(discount_total); }
    void set_email(std::string&& email) noexcept { m_email = std::move(email); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_deleted_at(std::optional<std::chrono::system_clock::time_point>&& deleted_at) noexcept { m_deleted_at = std::move(deleted_at); }
    void set_completed_at(std::optional<std::chrono::system_clock::time_point>&& completed_at) noexcept { m_completed_at = std::move(completed_at); }
    
    std::string& get_id() noexcept { return m_id; }
    std::string& get_customer_id() noexcept { return m_customer_id; }
    std::string& get_region_id() noexcept { return m_region_id; }
    std::string& get_currency_code() noexcept { return m_currency_code; }
    std::string& get_status() noexcept { return m_status; }
    std::string& get_payment_status() noexcept { return m_payment_status; }
    std::string& get_fulfillment_status() noexcept { return m_fulfillment_status; }
    std::string& get_total() noexcept { return m_total; }
    std::string& get_subtotal() noexcept { return m_subtotal; }
    std::string& get_tax_total() noexcept { return m_tax_total; }
    std::string& get_shipping_total() noexcept { return m_shipping_total; }
    std::string& get_discount_total() noexcept { return m_discount_total; }
    std::string& get_email() noexcept { return m_email; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    std::optional<std::chrono::system_clock::time_point>& get_deleted_at() noexcept { return m_deleted_at; }
    std::optional<std::chrono::system_clock::time_point>& get_completed_at() noexcept { return m_completed_at; }
    
    const std::string& get_id() const noexcept { return m_id; }
    const std::string& get_customer_id() const noexcept { return m_customer_id; }
    const std::string& get_region_id() const noexcept { return m_region_id; }
    const std::string& get_currency_code() const noexcept { return m_currency_code; }
    const std::string& get_status() const noexcept { return m_status; }
    const std::string& get_payment_status() const noexcept { return m_payment_status; }
    const std::string& get_fulfillment_status() const noexcept { return m_fulfillment_status; }
    const std::string& get_total() const noexcept { return m_total; }
    const std::string& get_subtotal() const noexcept { return m_subtotal; }
    const std::string& get_tax_total() const noexcept { return m_tax_total; }
    const std::string& get_shipping_total() const noexcept { return m_shipping_total; }
    const std::string& get_discount_total() const noexcept { return m_discount_total; }
    const std::string& get_email() const noexcept { return m_email; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_deleted_at() const noexcept { return m_deleted_at; }
    const std::optional<std::chrono::system_clock::time_point>& get_completed_at() const noexcept { return m_completed_at; }
    
    const char* entity_name() const noexcept { return "Order"; }
    const char* table_name() const noexcept { return "order"; }
    
    using TableMeta = reflection::Table<"order", "public",
        reflection::Column<"id", database::ColumnType::UUID, true, false, false>,
        reflection::ColumnWithFK<"customer_id", database::ColumnType::UUID, false, true, false, "customer", "id">,
        reflection::ColumnWithFK<"region_id", database::ColumnType::UUID, false, true, false, "region", "id">,
        reflection::ColumnWithFK<"currency_code", database::ColumnType::CHAR, false, true, false, "currency", "code">,
        reflection::ColumnWithDefault<"status", database::ColumnType::TEXT, false, true, false, "pending">,
        reflection::ColumnWithDefault<"payment_status", database::ColumnType::TEXT, false, true, false, "pending">,
        reflection::ColumnWithDefault<"fulfillment_status", database::ColumnType::TEXT, false, true, false, "not_fulfilled">,
        reflection::ColumnWithDefault<"total", database::ColumnType::NUMERIC, false, true, false, "0">,
        reflection::ColumnWithDefault<"subtotal", database::ColumnType::NUMERIC, false, true, false, "0">,
        reflection::ColumnWithDefault<"tax_total", database::ColumnType::NUMERIC, false, true, false, "0">,
        reflection::ColumnWithDefault<"shipping_total", database::ColumnType::NUMERIC, false, true, false, "0">,
        reflection::ColumnWithDefault<"discount_total", database::ColumnType::NUMERIC, false, true, false, "0">,
        reflection::Column<"email", database::ColumnType::TEXT, false, true, false>,
        reflection::ColumnWithDefault<"created_at", database::ColumnType::TIMESTAMPTZ, false, true, false, "NOW()">,
        reflection::ColumnWithDefault<"updated_at", database::ColumnType::TIMESTAMPTZ, false, true, false, "NOW()">,
        reflection::Column<"deleted_at", database::ColumnType::TIMESTAMPTZ, false, true, false>,
        reflection::Column<"completed_at", database::ColumnType::TIMESTAMPTZ, false, true, false>,
        reflection::Index<"idx_order_customer_id", "customer_id">,
        reflection::Index<"idx_order_status", "status">,
        reflection::Index<"idx_order_created_at", "created_at">
    >;
};

struct LineItem final {
    std::string m_id{};
    std::string m_order_id{};
    std::string m_variant_id{};
    std::string m_title{};
    int64_t m_quantity{1};
    std::string m_unit_price{"0"};
    std::string m_total{"0"};
    std::optional<std::string> m_metadata{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    LineItem() noexcept = default;
    
    LineItem(std::string&& id, std::string&& order_id, std::string&& variant_id, std::string&& title) noexcept
        : m_id(std::move(id))
        , m_order_id(std::move(order_id))
        , m_variant_id(std::move(variant_id))
        , m_title(std::move(title))
    {}
    
    LineItem(
        std::string&& id,
        std::string&& order_id,
        std::string&& variant_id,
        std::string&& title,
        int64_t&& quantity,
        std::string&& unit_price,
        std::string&& total,
        std::optional<std::string>&& metadata,
        std::chrono::system_clock::time_point&& created_at,
        std::chrono::system_clock::time_point&& updated_at
    ) noexcept
        : m_id(std::move(id))
        , m_order_id(std::move(order_id))
        , m_variant_id(std::move(variant_id))
        , m_title(std::move(title))
        , m_quantity(std::move(quantity))
        , m_unit_price(std::move(unit_price))
        , m_total(std::move(total))
        , m_metadata(std::move(metadata))
        , m_created_at(std::move(created_at))
        , m_updated_at(std::move(updated_at))
    {}
    
    LineItem& add_id(std::string&& id) noexcept { m_id = std::move(id); return *this; }
    LineItem& add_order_id(std::string&& order_id) noexcept { m_order_id = std::move(order_id); return *this; }
    LineItem& add_variant_id(std::string&& variant_id) noexcept { m_variant_id = std::move(variant_id); return *this; }
    LineItem& add_title(std::string&& title) noexcept { m_title = std::move(title); return *this; }
    LineItem& add_quantity(int64_t&& quantity) noexcept { m_quantity = std::move(quantity); return *this; }
    LineItem& add_unit_price(std::string&& unit_price) noexcept { m_unit_price = std::move(unit_price); return *this; }
    LineItem& add_total(std::string&& total) noexcept { m_total = std::move(total); return *this; }
    LineItem& add_metadata(std::optional<std::string>&& metadata) noexcept { m_metadata = std::move(metadata); return *this; }
    LineItem& add_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); return *this; }
    LineItem& add_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); return *this; }
    
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_order_id(std::string&& order_id) noexcept { m_order_id = std::move(order_id); }
    void set_variant_id(std::string&& variant_id) noexcept { m_variant_id = std::move(variant_id); }
    void set_title(std::string&& title) noexcept { m_title = std::move(title); }
    void set_quantity(int64_t&& quantity) noexcept { m_quantity = std::move(quantity); }
    void set_unit_price(std::string&& unit_price) noexcept { m_unit_price = std::move(unit_price); }
    void set_total(std::string&& total) noexcept { m_total = std::move(total); }
    void set_metadata(std::optional<std::string>&& metadata) noexcept { m_metadata = std::move(metadata); }
    void set_created_at(std::chrono::system_clock::time_point&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::chrono::system_clock::time_point&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    
    std::string& get_id() noexcept { return m_id; }
    std::string& get_order_id() noexcept { return m_order_id; }
    std::string& get_variant_id() noexcept { return m_variant_id; }
    std::string& get_title() noexcept { return m_title; }
    int64_t& get_quantity() noexcept { return m_quantity; }
    std::string& get_unit_price() noexcept { return m_unit_price; }
    std::string& get_total() noexcept { return m_total; }
    std::optional<std::string>& get_metadata() noexcept { return m_metadata; }
    std::chrono::system_clock::time_point& get_created_at() noexcept { return m_created_at; }
    std::chrono::system_clock::time_point& get_updated_at() noexcept { return m_updated_at; }
    
    const std::string& get_id() const noexcept { return m_id; }
    const std::string& get_order_id() const noexcept { return m_order_id; }
    const std::string& get_variant_id() const noexcept { return m_variant_id; }
    const std::string& get_title() const noexcept { return m_title; }
    const int64_t& get_quantity() const noexcept { return m_quantity; }
    const std::string& get_unit_price() const noexcept { return m_unit_price; }
    const std::string& get_total() const noexcept { return m_total; }
    const std::optional<std::string>& get_metadata() const noexcept { return m_metadata; }
    const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    
    const char* entity_name() const noexcept { return "LineItem"; }
    const char* table_name() const noexcept { return "line_item"; }
    
    using TableMeta = reflection::Table<"line_item", "public",
        reflection::Column<"id", database::ColumnType::UUID, true, false, false>,
        reflection::ColumnWithFK<"order_id", database::ColumnType::UUID, false, true, false, "order", "id">,
        reflection::ColumnWithFK<"variant_id", database::ColumnType::UUID, false, true, false, "product_variant", "id">,
        reflection::Column<"title", database::ColumnType::TEXT>,
        reflection::ColumnWithDefault<"quantity", database::ColumnType::BIGINT, false, true, false, "1">,
        reflection::ColumnWithDefault<"unit_price", database::ColumnType::NUMERIC, false, true, false, "0">,
        reflection::ColumnWithDefault<"total", database::ColumnType::NUMERIC, false, true, false, "0">,
        reflection::Column<"metadata", database::ColumnType::JSONB, false, true, false>,
        reflection::ColumnWithDefault<"created_at", database::ColumnType::TIMESTAMPTZ, false, true, false, "NOW()">,
        reflection::ColumnWithDefault<"updated_at", database::ColumnType::TIMESTAMPTZ, false, true, false, "NOW()">,
        reflection::Index<"idx_line_item_order_id", "order_id">,
        reflection::Index<"idx_line_item_variant_id", "variant_id">
    >;
};

// Address, Region, Currency, SalesChannel, PublishableApiKey, User, ApiKey follow same pattern...
// Abbreviated for brevity - same structure applies

struct Address final {
    std::string m_id{};
    std::string m_customer_id{};
    std::string m_first_name{};
    std::string m_last_name{};
    std::string m_company{};
    std::string m_address_1{};
    std::string m_address_2{};
    std::string m_city{};
    std::string m_province{};
    std::string m_postal_code{};
    std::string m_country_code{};
    std::string m_phone{};
    bool m_is_default_billing{false};
    bool m_is_default_shipping{false};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    
    Address() noexcept = default;
    Address(std::string&& id, std::string&& customer_id, std::string&& first_name, std::string&& address_1, std::string&& city, std::string&& country_code) noexcept
        : m_id(std::move(id)), m_customer_id(std::move(customer_id)), m_first_name(std::move(first_name)), m_address_1(std::move(address_1)), m_city(std::move(city)), m_country_code(std::move(country_code)) {}
    
    // Adder, setter, getter pattern same as above...
    const char* entity_name() const noexcept { return "Address"; }
    const char* table_name() const noexcept { return "address"; }
};

struct Region final {
    std::string m_id{};
    std::string m_name{};
    std::string m_currency_code{};
    std::string m_tax_rate{"0"};
    bool m_automatic_taxes{true};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    Region() noexcept = default;
    Region(std::string&& id, std::string&& name, std::string&& currency_code) noexcept
        : m_id(std::move(id)), m_name(std::move(name)), m_currency_code(std::move(currency_code)) {}
    
    const char* entity_name() const noexcept { return "Region"; }
    const char* table_name() const noexcept { return "region"; }
};

struct Currency final {
    std::string m_code{};
    std::string m_name{};
    std::string m_symbol{};
    std::string m_symbol_native{};
    
    Currency() noexcept = default;
    Currency(std::string&& code, std::string&& name, std::string&& symbol) noexcept
        : m_code(std::move(code)), m_name(std::move(name)), m_symbol(std::move(symbol)) {}
    
    const char* entity_name() const noexcept { return "Currency"; }
    const char* table_name() const noexcept { return "currency"; }
};

struct SalesChannel final {
    std::string m_id{};
    std::string m_name{};
    std::string m_description{};
    bool m_is_disabled{false};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    SalesChannel() noexcept = default;
    SalesChannel(std::string&& id, std::string&& name) noexcept
        : m_id(std::move(id)), m_name(std::move(name)) {}
    
    const char* entity_name() const noexcept { return "SalesChannel"; }
    const char* table_name() const noexcept { return "sales_channel"; }
};

struct PublishableApiKey final {
    std::string m_id{};
    std::string m_sales_channel_id{};
    std::string m_key{};
    bool m_revoked{false};
    std::chrono::system_clock::time_point m_created_at{};
    std::optional<std::chrono::system_clock::time_point> m_revoked_at{};
    
    PublishableApiKey() noexcept = default;
    PublishableApiKey(std::string&& id, std::string&& sales_channel_id, std::string&& key) noexcept
        : m_id(std::move(id)), m_sales_channel_id(std::move(sales_channel_id)), m_key(std::move(key)) {}
    
    const char* entity_name() const noexcept { return "PublishableApiKey"; }
    const char* table_name() const noexcept { return "publishable_api_key"; }
};

struct User final {
    std::string m_id{};
    std::string m_email{};
    std::string m_first_name{};
    std::string m_last_name{};
    std::string m_password_hash{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::optional<std::chrono::system_clock::time_point> m_deleted_at{};
    
    User() noexcept = default;
    User(std::string&& id, std::string&& email) noexcept
        : m_id(std::move(id)), m_email(std::move(email)) {}
    
    const char* entity_name() const noexcept { return "User"; }
    const char* table_name() const noexcept { return "user"; }
};

struct ApiKey final {
    std::string m_id{};
    std::string m_user_id{};
    std::string m_title{};
    std::string m_key_hash{};
    std::optional<std::chrono::system_clock::time_point> m_revoked_at{};
    std::chrono::system_clock::time_point m_created_at{};
    
    ApiKey() noexcept = default;
    ApiKey(std::string&& id, std::string&& user_id, std::string&& title, std::string&& key_hash) noexcept
        : m_id(std::move(id)), m_user_id(std::move(user_id)), m_title(std::move(title)), m_key_hash(std::move(key_hash)) {}
    
    const char* entity_name() const noexcept { return "ApiKey"; }
    const char* table_name() const noexcept { return "api_key"; }
};

} // namespace medusa_core::entities