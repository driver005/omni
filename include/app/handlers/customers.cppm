module;
#include <ctime>
export module app_handlers:customers;

import std;
import interfaces;
import core_router;
import core_serde;
import core_database;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_store;
import app_http;
import app_crypto;
import app_metrics;

namespace app::handlers::customers {

// Local equivalents of the upstream Cargo features that select the customer
// surface (routes/app.rs:1536-1636): v1 `/customers` and v2 `/{prefix}/customers`.
inline constexpr bool kV1 = true;
inline constexpr bool kV2 = true;

// Upstream constants.
inline constexpr std::string_view kRedacted = "Redacted";                       // core/customers.rs:54
inline constexpr std::size_t kDefaultListApiLimit = 10;                         // consts.rs:42
inline constexpr std::size_t kDefaultCustomerListLimit = 10;                    // common_types CustomerListLimit::default()
inline constexpr std::size_t kMigrationMaxFileSize = 1024U * 1024U;             // routes/customers/migrate.rs:21
inline constexpr std::size_t kMigrationMaxRecords = 500;                        // routes/customers/migrate.rs:22
inline constexpr std::size_t kMigrationParallelism = 10;                        // routes/customers/migrate.rs:23

enum class ApiVersion : std::uint8_t { V1, V2 };

// ── Local in-process persistence for fields the shared CustomerRecord lacks ──
// (address, tax id, document details, global id, version, status, connector ids).
// These are reflected through serde::Serializable exactly like app::store records.

class CustomerExtensionRecord
{
public:
    void set_id(std::int64_t value) { m_id = value; }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_customer_id(std::string value) { m_customer_id = std::move(value); }
    [[nodiscard]] const std::string& get_customer_id() const noexcept { return m_customer_id; }
    void set_global_id(std::string value) { m_global_id = std::move(value); }
    [[nodiscard]] const std::string& get_global_id() const noexcept { return m_global_id; }
    void set_merchant_reference_id(std::string value) { m_merchant_reference_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_reference_id() const noexcept { return m_merchant_reference_id; }
    void set_version(std::string value) { m_version = std::move(value); }
    [[nodiscard]] const std::string& get_version() const noexcept { return m_version; }
    void set_phone_country_code(std::string value) { m_phone_country_code = std::move(value); }
    [[nodiscard]] const std::string& get_phone_country_code() const noexcept { return m_phone_country_code; }
    void set_tax_registration_id(std::string value) { m_tax_registration_id = std::move(value); }
    [[nodiscard]] const std::string& get_tax_registration_id() const noexcept { return m_tax_registration_id; }
    void set_document_type(std::string value) { m_document_type = std::move(value); }
    [[nodiscard]] const std::string& get_document_type() const noexcept { return m_document_type; }
    void set_document_number(std::string value) { m_document_number = std::move(value); }
    [[nodiscard]] const std::string& get_document_number() const noexcept { return m_document_number; }
    void set_connector_customer_ids(std::string value) { m_connector_customer_ids = std::move(value); }
    [[nodiscard]] const std::string& get_connector_customer_ids() const noexcept { return m_connector_customer_ids; }
    void set_default_payment_method_id(std::string value) { m_default_payment_method_id = std::move(value); }
    [[nodiscard]] const std::string& get_default_payment_method_id() const noexcept { return m_default_payment_method_id; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }

    std::int64_t m_id{};
    std::string m_customer_id{};
    std::string m_global_id{};
    std::string m_merchant_reference_id{};
    std::string m_version{};
    std::string m_phone_country_code{};
    std::string m_tax_registration_id{};
    std::string m_document_type{};
    std::string m_document_number{};
    std::string m_connector_customer_ids{};
    std::string m_default_payment_method_id{};
    std::string m_status{};
};

class CustomerAddressRecord
{
public:
    void set_id(std::int64_t value) { m_id = value; }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_address_id(std::string value) { m_address_id = std::move(value); }
    [[nodiscard]] const std::string& get_address_id() const noexcept { return m_address_id; }
    void set_customer_id(std::string value) { m_customer_id = std::move(value); }
    [[nodiscard]] const std::string& get_customer_id() const noexcept { return m_customer_id; }
    void set_address_kind(std::string value) { m_address_kind = std::move(value); }
    [[nodiscard]] const std::string& get_address_kind() const noexcept { return m_address_kind; }
    void set_city(std::string value) { m_city = std::move(value); }
    [[nodiscard]] const std::string& get_city() const noexcept { return m_city; }
    void set_country(std::string value) { m_country = std::move(value); }
    [[nodiscard]] const std::string& get_country() const noexcept { return m_country; }
    void set_line1(std::string value) { m_line1 = std::move(value); }
    [[nodiscard]] const std::string& get_line1() const noexcept { return m_line1; }
    void set_line2(std::string value) { m_line2 = std::move(value); }
    [[nodiscard]] const std::string& get_line2() const noexcept { return m_line2; }
    void set_line3(std::string value) { m_line3 = std::move(value); }
    [[nodiscard]] const std::string& get_line3() const noexcept { return m_line3; }
    void set_zip(std::string value) { m_zip = std::move(value); }
    [[nodiscard]] const std::string& get_zip() const noexcept { return m_zip; }
    void set_state(std::string value) { m_state = std::move(value); }
    [[nodiscard]] const std::string& get_state() const noexcept { return m_state; }
    void set_first_name(std::string value) { m_first_name = std::move(value); }
    [[nodiscard]] const std::string& get_first_name() const noexcept { return m_first_name; }
    void set_last_name(std::string value) { m_last_name = std::move(value); }
    [[nodiscard]] const std::string& get_last_name() const noexcept { return m_last_name; }
    void set_origin_zip(std::string value) { m_origin_zip = std::move(value); }
    [[nodiscard]] const std::string& get_origin_zip() const noexcept { return m_origin_zip; }

    std::int64_t m_id{};
    std::string m_address_id{};
    std::string m_customer_id{};
    std::string m_address_kind{};
    std::string m_city{};
    std::string m_country{};
    std::string m_line1{};
    std::string m_line2{};
    std::string m_line3{};
    std::string m_zip{};
    std::string m_state{};
    std::string m_first_name{};
    std::string m_last_name{};
    std::string m_origin_zip{};
};

struct AddressInput
{
    std::optional<std::string> city{};
    std::optional<std::string> country{};
    std::optional<std::string> line1{};
    std::optional<std::string> line2{};
    std::optional<std::string> line3{};
    std::optional<std::string> zip{};
    std::optional<std::string> state{};
    std::optional<std::string> first_name{};
    std::optional<std::string> last_name{};
    std::optional<std::string> origin_zip{};
};

struct DocumentInput
{
    std::optional<std::string> document_type{};
    std::optional<std::string> document_number{};
};

struct CustomerCreateInput
{
    std::optional<std::string> customer_id{};
    std::optional<std::string> merchant_reference_id{};
    std::optional<std::string> name{};
    std::optional<std::string> email{};
    std::optional<std::string> phone{};
    std::optional<std::string> description{};
    std::optional<std::string> phone_country_code{};
    std::optional<AddressInput> address{};
    std::optional<AddressInput> billing_address{};
    std::optional<AddressInput> shipping_address{};
    std::optional<DocumentInput> document_details{};
    std::optional<std::string> metadata_raw{};
    std::optional<std::string> tax_registration_id{};
    std::optional<std::string> default_payment_method_id{};
};

struct CustomerUpdateInput
{
    std::optional<std::string> name{};
    std::optional<std::string> email{};
    std::optional<std::string> phone{};
    std::optional<std::string> description{};
    std::optional<std::string> phone_country_code{};
    std::optional<AddressInput> address{};
    std::optional<AddressInput> billing_address{};
    std::optional<AddressInput> shipping_address{};
    std::optional<DocumentInput> document_details{};
    std::optional<std::string> metadata_raw{};
    std::optional<std::string> tax_registration_id{};
    std::optional<std::string> default_payment_method_id{};
};

/** @brief Typed aggregate: the shared record plus its local extension and addresses. */
struct CustomerAggregate
{
    ::app::store::customers::CustomerRecord record{};
    CustomerExtensionRecord extension{};
    std::vector<CustomerAddressRecord> addresses{};
};

/** @brief Mirrors the upstream `MigrationRow<State>` typestate machine (migrate.rs:111-306). */
struct MigrationRow
{
    enum class State : std::uint8_t { Parsed, Fetched, V1Verified, NeedsUpdate, Generated };
    std::size_t row_number{};
    std::string merchant_id{};
    std::string customer_id{};
    std::optional<std::string> old_id{};
    std::optional<std::string> version{};
    std::optional<std::string> new_id{};
    std::optional<std::string> update_status{};
    State state{State::Parsed};
};

/** @brief Minimal ordered JSON object writer; upstream field order is preserved exactly. */
class JsonObject
{
public:
    void add_string(std::string_view name, const std::optional<std::string>& value)
    {
        m_fields.emplace_back(
            std::string{name},
            value ? quote(*value) : std::string{"null"}
        );
    }
    void add_string(std::string_view name, std::string value)
    {
        m_fields.emplace_back(std::string{name}, quote(value));
    }
    void add_string(std::string_view name, std::string_view value)
    {
        m_fields.emplace_back(std::string{name}, quote(value));
    }
    void add_bool(std::string_view name, bool value)
    {
        m_fields.emplace_back(std::string{name}, value ? std::string{"true"} : std::string{"false"});
    }
    void add_number(std::string_view name, std::size_t value)
    {
        m_fields.emplace_back(std::string{name}, std::to_string(value));
    }
    void add_raw(std::string_view name, const std::optional<std::string>& value)
    {
        m_fields.emplace_back(
            std::string{name},
            value && !value->empty() ? *value : std::string{"null"}
        );
    }
    void add_object(std::string_view name, std::string_view body)
    {
        m_fields.emplace_back(std::string{name}, std::string{body});
    }
    [[nodiscard]] std::string str() const
    {
        std::string out{"{"};
        for (std::size_t i{0}; i < m_fields.size(); ++i) {
            if (i != 0) { out.push_back(','); }
            out += quote(m_fields[i].first);
            out.push_back(':');
            out += m_fields[i].second;
        }
        out.push_back('}');
        return out;
    }
private:
    [[nodiscard]] static std::string quote(std::string_view value)
    {
        return serde::Ser::encode_json(std::string{value});
    }
    std::vector<std::pair<std::string, std::string>> m_fields{};
};

/** @brief Read-only navigation of a parsed JSON value (serde::Value = rfl::Generic). */
class JsonValueView
{
public:
    JsonValueView() = default;
    explicit JsonValueView(serde::Value value) : m_value{std::move(value)} {}

    [[nodiscard]] static std::optional<JsonValueView> parse(std::string_view text)
    {
        auto parsed = serde::Ser::decode_generic("application/json", text);
        if (!parsed) { return std::nullopt; }
        return JsonValueView{std::move(*parsed)};
    }

    [[nodiscard]] std::optional<std::string> string(std::string_view key) const
    {
        const auto field = find(key);
        if (!field) { return std::nullopt; }
        const auto* value = std::get_if<std::string>(&field->get());
        if (!value) { return std::nullopt; }
        return *value;
    }

    [[nodiscard]] std::optional<std::string> raw(std::string_view key) const
    {
        const auto field = find(key);
        if (!field) { return std::nullopt; }
        return serde::Ser::encode_json(*field);
    }

    [[nodiscard]] std::optional<JsonValueView> object(std::string_view key) const
    {
        const auto field = find(key);
        if (!field) { return std::nullopt; }
        if (!std::holds_alternative<serde::Value::Object>(field->get())) { return std::nullopt; }
        return JsonValueView{*field};
    }

private:
    [[nodiscard]] std::optional<serde::Value> find(std::string_view key) const
    {
        const auto* object = std::get_if<serde::Value::Object>(&m_value.get());
        if (!object) { return std::nullopt; }
        auto value = object->get(std::string{key});
        if (!value) { return std::nullopt; }
        return std::optional<serde::Value>{*value};
    }
    serde::Value m_value{};
};

} // namespace app::handlers::customers

template<>
struct serde::Serializable<::app::handlers::customers::CustomerExtensionRecord>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::customers::CustomerExtensionRecord;
        return std::tuple{
            serde::FieldDesc<"id", &T::get_id, &T::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"customer_id", &T::get_customer_id, &T::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"global_id", &T::get_global_id, &T::set_global_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_reference_id", &T::get_merchant_reference_id, &T::set_merchant_reference_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"version", &T::get_version, &T::set_version, serde::FieldOptions{}>{},
            serde::FieldDesc<"phone_country_code", &T::get_phone_country_code, &T::set_phone_country_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"tax_registration_id", &T::get_tax_registration_id, &T::set_tax_registration_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"document_type", &T::get_document_type, &T::set_document_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"document_number", &T::get_document_number, &T::set_document_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_customer_ids", &T::get_connector_customer_ids, &T::set_connector_customer_ids, serde::FieldOptions{}>{},
            serde::FieldDesc<"default_payment_method_id", &T::get_default_payment_method_id, &T::set_default_payment_method_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &T::get_status, &T::set_status, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "customer_extensions"; }
};

template<>
struct serde::Serializable<::app::handlers::customers::CustomerAddressRecord>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::customers::CustomerAddressRecord;
        return std::tuple{
            serde::FieldDesc<"id", &T::get_id, &T::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"address_id", &T::get_address_id, &T::set_address_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_id", &T::get_customer_id, &T::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"address_kind", &T::get_address_kind, &T::set_address_kind, serde::FieldOptions{}>{},
            serde::FieldDesc<"city", &T::get_city, &T::set_city, serde::FieldOptions{}>{},
            serde::FieldDesc<"country", &T::get_country, &T::set_country, serde::FieldOptions{}>{},
            serde::FieldDesc<"line1", &T::get_line1, &T::set_line1, serde::FieldOptions{}>{},
            serde::FieldDesc<"line2", &T::get_line2, &T::set_line2, serde::FieldOptions{}>{},
            serde::FieldDesc<"line3", &T::get_line3, &T::set_line3, serde::FieldOptions{}>{},
            serde::FieldDesc<"zip", &T::get_zip, &T::set_zip, serde::FieldOptions{}>{},
            serde::FieldDesc<"state", &T::get_state, &T::set_state, serde::FieldOptions{}>{},
            serde::FieldDesc<"first_name", &T::get_first_name, &T::set_first_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"last_name", &T::get_last_name, &T::set_last_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"origin_zip", &T::get_origin_zip, &T::set_origin_zip, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "customer_addresses"; }
};

export namespace app::handlers::customers {

class CustomersRouteHandlers
{
public:
    // ── Route entry points (v1 surface; routes/customers.rs + app.rs:1590-1636) ──
    void handle_customers_create(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) { create_customer(req, res, std::move(send), ApiVersion::V1); }
    void handle_customers_retrieve(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) { retrieve_customer(req, res, std::move(send), ApiVersion::V1); }
    void handle_customers_list(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) { list_customers(req, res, std::move(send), ApiVersion::V1); }
    void handle_customers_list_with_count(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) { list_customers_with_count(req, res, std::move(send), ApiVersion::V1); }
    void handle_customers_update(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) { update_customer(req, res, std::move(send), ApiVersion::V1); }
    void handle_customers_delete(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) { delete_customer(req, res, std::move(send), ApiVersion::V1); }
    void handle_get_customer_mandates(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) { get_customer_mandates(req, res, std::move(send)); }

    // ── Route entry points (v2 surface; app.rs:1538-1588) ──
    void handle_customers_create_v2(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) { create_customer(req, res, std::move(send), ApiVersion::V2); }
    void handle_customers_retrieve_v2(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) { retrieve_customer(req, res, std::move(send), ApiVersion::V2); }
    void handle_customers_retrieve_by_merchant_reference_id(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) { retrieve_customer_by_merchant_reference_id(req, res, std::move(send)); }
    void handle_customers_list_v2(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) { list_customers(req, res, std::move(send), ApiVersion::V2); }
    void handle_customers_list_with_count_v2(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) { list_customers_with_count(req, res, std::move(send), ApiVersion::V2); }
    void handle_customers_update_v2(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) { update_customer(req, res, std::move(send), ApiVersion::V2); }
    void handle_customers_delete_v2(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) { delete_customer(req, res, std::move(send), ApiVersion::V2); }
    void handle_migrate_global_id(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) { migrate_global_id(req, res, std::move(send)); }

    void install(core::router::RouterContext<>& router)
    {
        // Local in-process stores for the fields upstream keeps in dedicated tables.
        if (m_context) {
            m_context->database().register_table("customer_extensions");
            m_context->database().register_table("customer_addresses");
        }
        if constexpr (kV1) { install_v1_scope(router); }
        if constexpr (kV2) {
            std::string base{"/v2/customers"};
            if (m_context) {
                const auto prefix = m_context->config_store().get_value(
                    "micro_services", "payment_methods_prefix"
                );
                if (prefix && !prefix->empty()) {
                    base = "/" + *prefix + "/customers";
                }
            }
            if (base != "/customers") { install_v2_scope(router, base); }
        }
    }

    explicit CustomersRouteHandlers(::app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    // ───────────────────────────── Route mounting ─────────────────────────────

    void install_v1_scope(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // app.rs:1593-1632. Multi-segment resources are mounted before "/{customer_id}".
        (void)Router{router, "customers"}
            .add_route(Route{"list"}.get([this](auto& req, auto& res, auto send) {
                handle_customers_list(req, res, std::move(send)); }).summary("Hyperswitch customers_list"))
            .add_route(Route{"list_with_count"}.get([this](auto& req, auto& res, auto send) {
                handle_customers_list_with_count(req, res, std::move(send)); }).summary("Hyperswitch customers_list_with_count"))
            .add_router(Router{router, ":customer_id"}.add_route(Route{"mandates"}.get([this](auto& req, auto& res, auto send) {
                handle_get_customer_mandates(req, res, std::move(send)); }).summary("Hyperswitch get_customer_mandates")))
            .add_route(Route{""}.post([this](auto& req, auto& res, auto send) {
                handle_customers_create(req, res, std::move(send)); }).summary("Hyperswitch customers_create"))
            .add_route(Route{":customer_id"}.get([this](auto& req, auto& res, auto send) {
                handle_customers_retrieve(req, res, std::move(send)); }).summary("Hyperswitch customers_retrieve"))
            .add_route(Route{":customer_id"}.post([this](auto& req, auto& res, auto send) {
                handle_customers_update(req, res, std::move(send)); }).summary("Hyperswitch customers_update"))
            .add_route(Route{":customer_id"}.delt([this](auto& req, auto& res, auto send) {
                handle_customers_delete(req, res, std::move(send)); }).summary("Hyperswitch customers_delete"));
    }

    void install_v2_scope(core::router::RouterContext<>& router, std::string_view base)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // app.rs:1545-1586. The C++-only v1 `POST /{id}` alias and v1-only
        // `GET /{id}/mandates` are deliberately not mounted on the v2 scope.
        (void)Router{router, base}
            .add_route(Route{"list"}.get([this](auto& req, auto& res, auto send) {
                handle_customers_list_v2(req, res, std::move(send)); }).summary("Hyperswitch customers_list (v2)"))
            .add_route(Route{"list_with_count"}.get([this](auto& req, auto& res, auto send) {
                handle_customers_list_with_count_v2(req, res, std::move(send)); }).summary("Hyperswitch customers_list_with_count (v2)"))
            .add_router(Router{router, "migrate"}.add_route(Route{"global-id"}.post([this](auto& req, auto& res, auto send) {
                handle_migrate_global_id(req, res, std::move(send)); }).summary("Hyperswitch customers migrate_global_id")))
            .add_router(Router{router, "reference"}.add_route(Route{":merchant_reference_id"}.get([this](auto& req, auto& res, auto send) {
                handle_customers_retrieve_by_merchant_reference_id(req, res, std::move(send)); }).summary("Hyperswitch customers_retrieve_by_merchant_reference_id")))
            .add_route(Route{""}.post([this](auto& req, auto& res, auto send) {
                handle_customers_create_v2(req, res, std::move(send)); }).summary("Hyperswitch customers_create (v2)"))
            .add_route(Route{":id"}.get([this](auto& req, auto& res, auto send) {
                handle_customers_retrieve_v2(req, res, std::move(send)); }).summary("Hyperswitch customers_retrieve (v2)"))
            .add_route(Route{":id"}.put([this](auto& req, auto& res, auto send) {
                handle_customers_update_v2(req, res, std::move(send)); }).summary("Hyperswitch customers_update (v2)"))
            .add_route(Route{":id"}.delt([this](auto& req, auto& res, auto send) {
                handle_customers_delete_v2(req, res, std::move(send)); }).summary("Hyperswitch customers_delete (v2)"));
    }

    // ─────────────────────── core::customers::create_customer ─────────────────
    void create_customer(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res,
        std::function<void()> send, ApiVersion version
    )
    {
        const bool jwt = is_jwt_auth(req);
        if (version == ApiVersion::V1) {
            if (const auto denied = authorize(req, ::app::auth::AppPermission::USER_WRITE,
                    jwt, false, false, false, true, true, true)) { send_error(res, send, *denied); return; }
            if (const auto denied = validate_legacy_endpoint_access(req)) { send_error(res, send, *denied); return; }
        } else {
            if (const auto denied = authorize(req, ::app::auth::AppPermission::USER_WRITE,
                    jwt, true, false, false, false, true, true)) { send_error(res, send, *denied); return; }
        }

        const std::string body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, upstream_error("InvalidRequestData", "empty request"));
            return;
        }
        const auto parsed = JsonValueView::parse(body);
        if (!parsed) {
            send_error(res, send, upstream_error("InvalidRequestData", "invalid JSON"));
            return;
        }
        const CustomerCreateInput input = parse_create_input(*parsed, version);
        if (const auto invalid = validate_document_details(input.document_details)) {
            send_error(res, send, *invalid); return;
        }
        if (const auto invalid = validate_phone_country_code(input.phone_country_code)) {
            send_error(res, send, *invalid); return;
        }
        if (version == ApiVersion::V2 && (!input.name || !input.email)) {
            send_error(res, send, upstream_error(
                "InvalidRequestData", "name and email are required"));
            return;
        }

        const std::string merchant_id = request_merchant_id(req);
        // MerchantReferenceIdForCustomer::verify_if_merchant_reference_not_present*
        // (core/customers.rs:89-104 / 472-569).
        const std::string reference = version == ApiVersion::V1
            ? input.customer_id.value_or(generate_merchant_reference_id())
            : input.merchant_reference_id.value_or(input.customer_id.value_or(
                  generate_merchant_reference_id()));
        if (reference.empty()) {
            send_error(res, send, upstream_error("InternalServerError", "missing customer reference"));
            return;
        }
        auto repo = m_context->repository();
        if (find_by_reference(repo, reference, merchant_id)) {
            send_error(res, send, upstream_error("CustomerAlreadyExists", "customer already exists"));
            return;
        }

        // CustomerCreateBridge::create_domain_model_from_request (core/customers.rs:152-421).
        CustomerAggregate aggregate{};
        const std::string global_id = generate_global_customer_id();
        aggregate.record.set_customer_id(version == ApiVersion::V1 ? reference : global_id);
        aggregate.record.set_merchant_id(merchant_id);
        aggregate.record.set_created_at(now_iso8601());
        if (input.name) { aggregate.record.set_name(encrypt_pii(*input.name)); }
        if (input.email) { aggregate.record.set_email(encrypt_pii(*input.email)); }
        if (input.phone) { aggregate.record.set_phone(encrypt_pii(*input.phone)); }
        if (input.description) { aggregate.record.set_description(*input.description); }
        if (input.metadata_raw) { aggregate.record.set_metadata(*input.metadata_raw); }

        aggregate.extension.set_global_id(global_id);
        aggregate.extension.set_merchant_reference_id(reference);
        aggregate.extension.set_version(version == ApiVersion::V1 ? "v1" : "v2");
        aggregate.extension.set_status("Active");
        if (input.phone_country_code) { aggregate.extension.set_phone_country_code(*input.phone_country_code); }
        if (input.tax_registration_id) {
            aggregate.extension.set_tax_registration_id(encrypt_pii(*input.tax_registration_id));
        }
        if (input.document_details) {
            aggregate.extension.set_document_type(input.document_details->document_type.value_or(""));
            aggregate.extension.set_document_number(
                encrypt_pii(input.document_details->document_number.value_or("")));
        }
        if (input.default_payment_method_id) {
            aggregate.extension.set_default_payment_method_id(*input.default_payment_method_id);
        }

        // AddressStructForDbEntry::encrypt_customer_address_and_set_to_db (core/customers.rs:423-470).
        if (version == ApiVersion::V1 && input.address) {
            aggregate.addresses.push_back(to_address_record(*input.address, "v1"));
        } else if (version == ApiVersion::V2) {
            if (input.billing_address) {
                aggregate.addresses.push_back(to_address_record(*input.billing_address, "billing"));
            }
            if (input.shipping_address) {
                aggregate.addresses.push_back(to_address_record(*input.shipping_address, "shipping"));
            }
        }
        persist_create(repo, aggregate);

        send_json(res, send, encode_customer_response(aggregate, input, std::nullopt, version), true);
        m_context->metrics().record_flow("customers.create");
        m_context->metrics().counter("customers_create").increment();
    }

    // ───────────────────── core::customers::retrieve_customer ─────────────────
    void retrieve_customer(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res,
        std::function<void()> send, ApiVersion version
    )
    {
        const bool jwt = is_jwt_auth(req);
        if (version == ApiVersion::V1) {
            if (const auto denied = authorize(req, ::app::auth::AppPermission::USER_READ,
                    jwt, false, false, false, true, true, true)) { send_error(res, send, *denied); return; }
            if (const auto denied = validate_legacy_endpoint_access(req)) { send_error(res, send, *denied); return; }
        } else {
            if (const auto denied = authorize(req, ::app::auth::AppPermission::USER_READ,
                    jwt, true, true, true, false, true, true)) { send_error(res, send, *denied); return; }
        }
        const std::string id = path_segment(req);
        if (id.empty()) {
            send_error(res, send, upstream_error("InvalidRequestData", "missing customer id")); return;
        }
        auto repo = m_context->repository();
        const auto aggregate = version == ApiVersion::V1
            ? locate_customer(repo, req, id)
            : locate_global_customer(repo, id);
        if (!aggregate) {
            send_error(res, send, upstream_error("CustomerNotFound", "customer not found")); return;
        }
        send_json(res, send, encode_customer_response(*aggregate, std::nullopt, std::nullopt, version), true);
        m_context->metrics().record_flow("customers.retrieve");
        m_context->metrics().counter("customers_retrieve").increment();
    }

    // ── core::customers::retrieve_customer_by_merchant_reference_id (v2 only) ──
    void retrieve_customer_by_merchant_reference_id(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize(req, ::app::auth::AppPermission::USER_READ,
                false, true, false, false, false, true, true)) {
            send_error(res, send, *denied); return;
        }
        const std::string reference = path_segment(req);
        if (reference.empty()) {
            send_error(res, send, upstream_error("InvalidRequestData", "missing merchant reference id"));
            return;
        }
        auto repo = m_context->repository();
        const auto aggregate = find_by_reference(repo, reference, request_merchant_id(req));
        if (!aggregate) {
            send_error(res, send, upstream_error("CustomerNotFound", "customer not found")); return;
        }
        send_json(res, send, encode_customer_response(*aggregate, std::nullopt, std::nullopt, ApiVersion::V2), true);
        m_context->metrics().record_flow("customers.retrieve_by_merchant_reference_id");
        m_context->metrics().counter("customers_retrieve_by_merchant_reference_id").increment();
    }

    // ───────────────────── core::customers::list_customers ────────────────────
    void list_customers(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res,
        std::function<void()> send, ApiVersion version
    )
    {
        if (const auto denied = authorize(req, ::app::auth::AppPermission::USER_READ,
                is_jwt_auth(req), version == ApiVersion::V2, false, false, false, true, true)) {
            send_error(res, send, *denied); return;
        }
        const auto query = ::app::http::RequestQuery{}(req);
        std::optional<std::uint32_t> offset{};
        std::optional<std::uint16_t> limit{};
        std::optional<std::string> customer_id{};
        if (const auto value = query.get("offset")) { offset = parse_uint32(*value); }
        if (const auto value = query.get("limit")) { limit = parse_uint16(*value); }
        if (const auto value = query.get("customer_id")) { customer_id = *value; }

        const std::size_t off = offset.value_or(0);
        const std::size_t lim = limit ? static_cast<std::size_t>(*limit) : kDefaultListApiLimit;
        auto repo = m_context->repository();
        const std::string merchant_id = request_merchant_id(req);
        std::size_t position{0};
        std::string data{"["};
        bool first{true};
        for (const auto& record : repo.list_customers()) {
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) { continue; }
            if (customer_id && record.get_customer_id() != *customer_id) { continue; }
            if (position < off) { ++position; continue; }
            if (position - off >= lim) { break; }
            const auto aggregate = load_aggregate(record);
            if (!first) { data.push_back(','); }
            first = false;
            data += encode_customer_response(*aggregate, std::nullopt, std::nullopt, version);
            ++position;
        }
        data.push_back(']');
        send_json(res, send, data, true);
        m_context->metrics().record_flow("customers.list");
        m_context->metrics().counter("customers_list").increment();
    }

    // ──────────────── core::customers::list_customers_with_count ──────────────
    void list_customers_with_count(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res,
        std::function<void()> send, ApiVersion version
    )
    {
        if (const auto denied = authorize(req, ::app::auth::AppPermission::USER_READ,
                is_jwt_auth(req), version == ApiVersion::V2, false, false, false, true, true)) {
            send_error(res, send, *denied); return;
        }
        const auto query = ::app::http::RequestQuery{}(req);
        const std::size_t off = query.get("offset") ? parse_uint32(*query.get("offset")).value_or(0) : 0U;
        std::size_t lim = kDefaultCustomerListLimit;
        if (const auto value = query.get("limit")) { lim = parse_size(*value).value_or(lim); }
        std::optional<std::string> customer_id{};
        if (const auto value = query.get("customer_id")) { customer_id = *value; }

        // `#[serde(flatten)] time_range: Option<TimeRange>` — start_time/end_time
        // (aliases startTime/endTime) appear at the top level of the query.
        std::optional<app::model::customers::CustomerTimeRange> time_range{};
        auto start = query.get("start_time");
        if (!start) { start = query.get("startTime"); }
        if (start) {
            app::model::customers::CustomerTimeRange range{};
            range.set_start_time(*start);
            auto end = query.get("end_time");
            if (!end) { end = query.get("endTime"); }
            if (end) { range.set_end_time(*end); }
            time_range = std::move(range);
        }

        auto repo = m_context->repository();
        const std::string merchant_id = request_merchant_id(req);
        std::vector<CustomerAggregate> filtered{};
        for (const auto& record : repo.list_customers()) {
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) { continue; }
            if (customer_id && record.get_customer_id() != *customer_id) { continue; }
            if (time_range && !within_range(record.get_created_at(), *time_range)) { continue; }
            if (auto aggregate = load_aggregate(record)) { filtered.push_back(std::move(*aggregate)); }
        }
        const std::size_t total = filtered.size();
        JsonObject response{};
        std::size_t position{0};
        std::string data{"["};
        bool first{true};
        for (const auto& aggregate : filtered) {
            if (position < off) { ++position; continue; }
            if (position - off >= lim) { break; }
            if (!first) { data.push_back(','); }
            first = false;
            data += encode_customer_response(aggregate, std::nullopt, std::nullopt, version);
            ++position;
        }
        data.push_back(']');
        response.add_raw("data", data);
        response.add_number("total_count", total);
        send_json(res, send, response.str(), true);
        m_context->metrics().record_flow("customers.list_with_count");
        m_context->metrics().counter("customers_list_with_count").increment();
    }

    // ───────────────────── core::customers::update_customer ───────────────────
    void update_customer(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res,
        std::function<void()> send, ApiVersion version
    )
    {
        const bool jwt = is_jwt_auth(req);
        if (version == ApiVersion::V1) {
            if (const auto denied = authorize(req, ::app::auth::AppPermission::USER_WRITE,
                    jwt, false, false, false, true, true, true)) { send_error(res, send, *denied); return; }
            if (const auto denied = validate_legacy_endpoint_access(req)) { send_error(res, send, *denied); return; }
        } else {
            if (const auto denied = authorize(req, ::app::auth::AppPermission::USER_WRITE,
                    jwt, true, false, false, false, true, true)) { send_error(res, send, *denied); return; }
        }
        const std::string id = path_segment(req);
        if (id.empty()) {
            send_error(res, send, upstream_error("InvalidRequestData", "missing customer id")); return;
        }
        auto repo = m_context->repository();
        auto existing = version == ApiVersion::V1
            ? locate_customer(repo, req, id)
            : locate_global_customer(repo, id);
        if (!existing) {
            send_error(res, send, upstream_error("CustomerNotFound", "customer not found")); return;
        }

        const std::string body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, upstream_error("InvalidRequestData", "empty request")); return;
        }
        const auto parsed = JsonValueView::parse(body);
        if (!parsed) {
            send_error(res, send, upstream_error("InvalidRequestData", "invalid JSON")); return;
        }
        const CustomerUpdateInput input = parse_update_input(*parsed);
        if (const auto invalid = validate_document_details(input.document_details)) {
            send_error(res, send, *invalid); return;
        }
        if (const auto invalid = validate_phone_country_code(input.phone_country_code)) {
            send_error(res, send, *invalid); return;
        }

        // VerifyIdForUpdateCustomer::verify_id_and_get_customer_object (core/customers.rs:1385-1424).
        if (version == ApiVersion::V1 && !is_global_customer_id_format(id)
            && !existing->record.get_customer_id().empty()
            && existing->record.get_customer_id() != id) {
            send_error(res, send, upstream_error("InvalidRequestData", "customer id mismatch")); return;
        }

        // CustomerUpdateBridge::create_domain_model_from_request / CustomerUpdate::Update
        // (core/customers.rs:1426-1684) — only fields present in the request are applied.
        CustomerUpdateInput effective = input;
        if (input.name) { existing->record.set_name(encrypt_pii(*input.name)); }
        if (input.email) { existing->record.set_email(encrypt_pii(*input.email)); }
        if (input.phone) { existing->record.set_phone(encrypt_pii(*input.phone)); }
        if (input.description) { existing->record.set_description(*input.description); }
        if (input.metadata_raw) { existing->record.set_metadata(*input.metadata_raw); }
        if (input.phone_country_code) { existing->extension.set_phone_country_code(*input.phone_country_code); }
        if (input.tax_registration_id) {
            existing->extension.set_tax_registration_id(encrypt_pii(*input.tax_registration_id));
        }
        if (input.document_details) {
            existing->extension.set_document_type(input.document_details->document_type.value_or(""));
            existing->extension.set_document_number(
                encrypt_pii(input.document_details->document_number.value_or("")));
        }
        if (input.default_payment_method_id) {
            existing->extension.set_default_payment_method_id(*input.default_payment_method_id);
        }
        // AddressStructForDbUpdate::update_address_if_sent (core/customers.rs:1287-1365).
        if (version == ApiVersion::V1 && effective.address) {
            upsert_address(repo, *existing, *effective.address, "v1");
        } else if (version == ApiVersion::V2) {
            if (effective.billing_address) { upsert_address(repo, *existing, *effective.billing_address, "billing"); }
            if (effective.shipping_address) { upsert_address(repo, *existing, *effective.shipping_address, "shipping"); }
        }
        persist_update(repo, *existing);

        send_json(res, send, encode_customer_response(*existing, std::nullopt, input, version), true);
        m_context->metrics().record_flow("customers.update");
        m_context->metrics().counter("customers_update").increment();
    }

    // ── CustomerDeleteBridge::redact_customer_details_and_generate_response ────
    void delete_customer(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res,
        std::function<void()> send, ApiVersion version
    )
    {
        const bool jwt = is_jwt_auth(req);
        if (version == ApiVersion::V1) {
            if (const auto denied = authorize(req, ::app::auth::AppPermission::USER_WRITE,
                    jwt, false, false, false, true, true, true)) { send_error(res, send, *denied); return; }
            if (const auto denied = validate_legacy_endpoint_access(req)) { send_error(res, send, *denied); return; }
        } else {
            if (const auto denied = authorize(req, ::app::auth::AppPermission::USER_WRITE,
                    jwt, true, false, false, false, true, true)) { send_error(res, send, *denied); return; }
        }
        const std::string id = path_segment(req);
        if (id.empty()) {
            send_error(res, send, upstream_error("InvalidRequestData", "missing customer id")); return;
        }
        auto repo = m_context->repository();
        auto existing = version == ApiVersion::V1
            ? locate_customer(repo, req, id)
            : locate_global_customer(repo, id);
        if (!existing) {
            send_error(res, send, upstream_error("CustomerNotFound", "customer not found")); return;
        }
        const std::string merchant_id = request_merchant_id(req);
        if (!merchant_id.empty() && !existing->record.get_merchant_id().empty()
            && existing->record.get_merchant_id() != merchant_id) {
            send_error(res, send, upstream_error("AccessForbidden", "customer belongs to another merchant"));
            return;
        }

        // MandateActive rejection (core/customers.rs:834-840 / 1007-1016).
        for (const auto& mandate : repo.find_mandates_by_customer_id(existing->record.get_customer_id())) {
            if (mandate.get_status() == "ACTIVE" || mandate.get_status() == "Active") {
                send_error(res, send, upstream_error("MandateActive", "customer has an active mandate"));
                return;
            }
        }

        // Payment-method deletion (core/customers.rs:842-866 / 1018-1079).
        for (const auto& method : repo.list_payment_methods()) {
            if (method.get_customer_id() != existing->record.get_customer_id()) { continue; }
            if (!merchant_id.empty() && method.get_merchant_id() != merchant_id) { continue; }
            (void)repo.remove_payment_method(method.get_id());
        }

        // Encrypted `Redacted` values and DeleteStatus::Redacted (core/customers.rs:868-929 / 1081-1185).
        const std::string tombstone = encrypt_pii(std::string{kRedacted});
        existing->record.set_name(tombstone);
        existing->record.set_email(tombstone);
        existing->record.set_phone(tombstone);
        existing->record.set_description(std::string{kRedacted});
        existing->record.set_metadata("");
        existing->extension.set_tax_registration_id(tombstone);
        existing->extension.set_document_number("");
        existing->extension.set_document_type("");
        existing->extension.set_phone_country_code(std::string{kRedacted});
        existing->extension.set_status("Redacted");
        for (auto& address : existing->addresses) {
            address.set_line1(tombstone);
            address.set_line2(tombstone);
            address.set_line3(tombstone);
            address.set_zip(tombstone);
            address.set_state(tombstone);
            address.set_first_name(tombstone);
            address.set_last_name(tombstone);
            address.set_city(std::string{kRedacted});
            address.set_country(std::string{kRedacted});
            address.set_origin_zip(std::string{kRedacted});
        }
        persist_update(repo, *existing);

        JsonObject response{};
        if (version == ApiVersion::V1) {
            response.add_string("customer_id", existing->record.get_customer_id());
        } else {
            response.add_string("id", existing->extension.get_global_id().empty()
                ? existing->record.get_customer_id() : existing->extension.get_global_id());
            const std::string& reference = existing->extension.get_merchant_reference_id();
            response.add_string("merchant_reference_id", reference.empty()
                ? std::optional<std::string>{} : std::optional<std::string>{reference});
        }
        response.add_bool("customer_deleted", true);
        response.add_bool("address_deleted", true);
        response.add_bool("payment_methods_deleted", true);
        send_json(res, send, response.str(), true);

        m_context->metrics().record_flow("customers.redacted");
        m_context->metrics().counter("CUSTOMER_REDACTED").increment();
        m_context->metrics().record_flow("customers.delete");
        m_context->metrics().counter("customers_delete").increment();
    }

    // ───────── core::mandate::get_customer_mandates (routes/customers.rs:560) ───
    void get_customer_mandates(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // HeaderAuth(ApiKeyAuth{connected:false, platform:false}) or JWT
        // MerchantMandateRead{connected:false, platform:false} (customers.rs:562-572).
        if (const auto denied = authorize(req, ::app::auth::AppPermission::MANDATE_READ,
                is_jwt_auth(req), false, false, false, true, false, false)) {
            send_error(res, send, *denied); return;
        }
        const std::string id = path_segment(req, 1);
        if (id.empty()) {
            send_error(res, send, upstream_error("InvalidRequestData", "missing customer id")); return;
        }
        auto repo = m_context->repository();
        const auto customer = locate_customer(repo, req, id);
        if (!customer) {
            send_error(res, send, upstream_error("CustomerNotFound", "customer not found")); return;
        }
        const std::string merchant_id = request_merchant_id(req);
        std::string out{"["};
        bool first{true};
        for (const auto& record : repo.find_mandates_by_customer_id(customer->record.get_customer_id())) {
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) { continue; }
            JsonObject item{};
            item.add_string("mandate_id", record.get_mandate_id());
            item.add_string("status", record.get_status());
            item.add_string("payment_method_id", record.get_payment_method_id());
            item.add_string("payment_method", record.get_payment_method());
            if (!first) { out.push_back(','); }
            first = false;
            out += item.str();
        }
        out.push_back(']');
        send_json(res, send, out, true);
        m_context->metrics().record_flow("customers.get_customer_mandates");
        m_context->metrics().counter("customers_get_customer_mandates").increment();
    }

    // ── routes/customers/migrate.rs::migrate_customer_global_id ────────────────
    void migrate_global_id(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream `V2AdminApiAuth` — local equivalent is the platform-admin rule.
        const auto rule = ::app::auth::RouteAuthRule{
            ::app::auth::AuthScheme::PLATFORM_ADMIN, ::app::auth::AppPermission::ADMIN
        };
        if (const auto denied = m_context->authorizer().authorize_route(
                ::app::auth::AuthContextBuilder{}(req), rule)) { send_error(res, send, *denied); return; }

        const std::string body = req.body_as_string();
        const auto rows = parse_customer_global_id_migration_csv(body);
        if (!rows) { send_error(res, send, rows.error()); return; }

        auto repo = m_context->repository();
        std::vector<::app::model::customers::CustomerGlobalIdMigrationRowResult> results{};
        results.reserve(rows->size());
        for (const auto& row : *rows) {
            if (!std::holds_alternative<MigrationRow>(row)) {
                results.push_back(build_migration_result(std::get<::app::model::customers::CustomerGlobalIdMigrationRowResult>(row)));
            } else {
                results.push_back(process_fetched_row(repo, std::get<MigrationRow>(row)));
            }
        }
        const std::size_t total = results.size();
        std::size_t updated{0}, skipped{0};
        for (const auto& result : results) {
            const auto& status = result.get_status();
            if (status == "UpdatedNullId" || status == "UpdatedNonGlobalId") { ++updated; }
            if (status == "AlreadyGlobalId" || status == "SkippedNonV1") { ++skipped; }
        }
        JsonObject response{};
        response.add_number("total_rows", total);
        response.add_number("updated_count", updated);
        response.add_number("skipped_count", skipped);
        response.add_number("failed_count", total - updated - skipped);
        std::string data{"["};
        for (std::size_t i{0}; i < results.size(); ++i) {
            if (i != 0) { data.push_back(','); }
            data += encode_migration_row(results[i]);
        }
        data.push_back(']');
        response.add_raw("results", data);
        send_json(res, send, response.str(), true);
        m_context->metrics().record_flow("customers.migrate_global_id");
        m_context->metrics().counter("customers_migrate_global_id").increment();
    }

    // ───────────────────────── Local helpers ──────────────────────────────────

    [[nodiscard]] ::core::error::AppError error(
        ::core::error::AppErrorKind kind, std::string message
    ) const
    {
        return ::core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    [[nodiscard]] ::core::error::AppError upstream_error(
        std::string_view wire_name, std::string message
    ) const
    {
        using K = ::core::error::AppErrorKind;
        const K kind = wire_name == "CustomerAlreadyExists" ? K::ALREADY_EXISTS
            : wire_name == "CustomerNotFound" ? K::NOT_FOUND
            : wire_name == "MandateActive" ? K::PRECONDITION_FAILED
            : wire_name == "AccessForbidden" ? K::FORBIDDEN
            : wire_name == "InternalServerError" ? K::INTERNAL_ERROR
            : K::VALIDATION_FAILED;
        auto result = error(kind, std::move(message));
        result.set_kind_name(std::string{wire_name});
        return result;
    }

    /** @brief Upstream `ErrorResponse` wire name → (code, error_type, canonical message)
     *  (api_models/errors/types.rs:38-56, api_error_response.rs:453-455,586-588,607-609,643-645;
     *  core/errors/transformers.rs:30-43). */
    [[nodiscard]] std::optional<std::tuple<std::string_view, std::string_view, std::string>>
    upstream_wire(std::string_view name, const std::string& message) const
    {
        if (name == "InvalidRequestData") {
            return std::tuple<std::string_view, std::string_view, std::string>{std::string_view{"IR_06"}, std::string_view{"invalid_request"}, message};
        }
        if (name == "CustomerAlreadyExists") {
            return std::tuple<std::string_view, std::string_view, std::string>{std::string_view{"IR_12"}, std::string_view{"invalid_request"}, std::string{"Customer with the given `customer_id` already exists"}};
        }
        if (name == "CustomerNotFound") {
            return std::tuple<std::string_view, std::string_view, std::string>{std::string_view{"HE_02"}, std::string_view{"invalid_request"}, std::string{"Customer does not exist in our records"}};
        }
        if (name == "MandateActive") {
            return std::tuple<std::string_view, std::string_view, std::string>{std::string_view{"IR_10"}, std::string_view{"invalid_request"}, std::string{"Customer has active mandate/subsciption"}};
        }
        if (name == "AccessForbidden") {
            return std::tuple<std::string_view, std::string_view, std::string>{std::string_view{"IR_22"}, std::string_view{"invalid_request"}, "Access forbidden. Not authorized to access this resource " + message};
        }
        if (name == "InternalServerError") {
            return std::tuple<std::string_view, std::string_view, std::string>{std::string_view{"HE_00"}, std::string_view{"api"}, std::string{"Something went wrong"}};
        }
        return std::nullopt;
    }

    void send_error(
        interfaces::io::IResponse& res, std::function<void()> send,
        ::core::error::AppError value
    ) const
    {
        std::string text{};
        if (const auto wire = upstream_wire(value.get_kind_name(), value.get_message())) {
            JsonObject inner{};
            inner.add_string("type", std::get<1>(*wire));
            inner.add_string("message", std::get<2>(*wire));
            inner.add_string("code", std::get<0>(*wire));
            JsonObject outer{};
            outer.add_object("error", inner.str());
            text = outer.str();
        } else {
            text = serde::Ser::encode_json(value);
        }
        res.set_status(::core::error::AppErrorStatus{}(value.get_kind()));
        res.set_body(serde::Ser::serialize_raw("application/json", text));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res, std::function<void()> send,
        std::string_view text, bool ok
    ) const
    {
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(serde::Ser::serialize_raw("application/json", text));
        send();
    }

    [[nodiscard]] bool is_jwt_auth(interfaces::io::IRequest& req) const
    {
        const std::string_view header = req.find_header("authorization");
        constexpr std::string_view prefix{"Bearer "};
        return header.size() > prefix.size()
            && std::equal(prefix.begin(), prefix.end(), header.begin(),
                [](char lhs, char rhs) {
                    return std::tolower(static_cast<unsigned char>(lhs))
                        == std::tolower(static_cast<unsigned char>(rhs));
                });
    }

    [[nodiscard]] bool has_header(interfaces::io::IRequest& req, std::string_view name) const
    {
        return !req.find_header(name).empty();
    }

    /** @brief Upstream auth extractor selection (routes/customers.rs:39-50,82-93, et al). */
    [[nodiscard]] std::optional<::core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        ::app::auth::AppPermission permission,
        bool jwt,
        bool v2_api_key,
        bool sdk,
        bool client,
        bool header_api_key,
        bool allow_connected,
        bool allow_platform
    ) const
    {
        const auto context = ::app::auth::AuthContextBuilder{}(req);
        if (jwt && is_jwt_auth(req)) {
            return m_context->authorizer().authorize_route(
                context, ::app::auth::RouteAuthRule{
                    ::app::auth::AuthScheme::JWT, permission, allow_connected, allow_platform});
        }
        if (sdk && (has_header(req, "x-sdk-key") || has_header(req, "x-publishable-key"))) {
            return m_context->authorizer().authorize_route(
                context, ::app::auth::RouteAuthRule{
                    ::app::auth::AuthScheme::SDK, permission, allow_connected, allow_platform});
        }
        if (client && (has_header(req, "x-client-secret") || has_header(req, "client-secret"))) {
            return m_context->authorizer().authorize_route(
                context, ::app::auth::RouteAuthRule{
                    ::app::auth::AuthScheme::CLIENT_SECRET, permission, allow_connected, allow_platform});
        }
        if (v2_api_key && has_header(req, "x-api-key")) {
            return m_context->authorizer().authorize_route(
                context, ::app::auth::RouteAuthRule{
                    ::app::auth::AuthScheme::V2_API_KEY, permission, allow_connected, allow_platform});
        }
        if (header_api_key) {
            return m_context->authorizer().authorize_route(
                context, ::app::auth::RouteAuthRule{
                    ::app::auth::AuthScheme::API_KEY, permission, allow_connected, allow_platform});
        }
        // No matching extractor: deny with the first upstream extractor's requirement.
        const auto scheme = v2_api_key ? ::app::auth::AuthScheme::V2_API_KEY
                                       : ::app::auth::AuthScheme::API_KEY;
        return m_context->authorizer().authorize_route(
            context, ::app::auth::RouteAuthRule{scheme, permission, allow_connected, allow_platform});
    }

    /** @brief core::utils::validate_legacy_endpoint_access (core/utils.rs:167-193). */
    [[nodiscard]] std::optional<::core::error::AppError> validate_legacy_endpoint_access(
        interfaces::io::IRequest& req
    ) const
    {
        // The dashboard (JWT initiator) keeps legacy access; API-key traffic is blocked
        // once the payment-method modular feature is enabled for the merchant.
        const bool dashboard = is_jwt_auth(req);
        if (dashboard || !m_context) { return std::nullopt; }
        const auto enabled = m_context->config_store().get_value(
            "feature_config", "is_payment_method_modular_allowed"
        );
        if (enabled && (*enabled == "true" || *enabled == "1")) {
            return upstream_error("AccessForbidden", "Deprecated route");
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string request_merchant_id(interfaces::io::IRequest& req) const
    {
        return ::app::http::merchant_id_from_header{}(req);
    }

    [[nodiscard]] std::string path_segment(
        interfaces::io::IRequest& req, std::size_t from_end = 0
    ) const
    {
        const ::app::http::PathSegments segments{::app::http::RequestPath{}(req)};
        const std::size_t count{segments.count()};
        if (from_end >= count) { return {}; }
        const auto value = segments.at(count - 1 - from_end);
        return value ? std::string{*value} : std::string{};
    }

    [[nodiscard]] std::optional<std::uint32_t> parse_uint32(std::string_view text) const
    {
        std::uint32_t out{};
        const auto result = std::from_chars(text.data(), text.data() + text.size(), out);
        if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) { return std::nullopt; }
        return out;
    }
    [[nodiscard]] std::optional<std::uint16_t> parse_uint16(std::string_view text) const
    {
        std::uint16_t out{};
        const auto result = std::from_chars(text.data(), text.data() + text.size(), out);
        if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) { return std::nullopt; }
        return out;
    }
    [[nodiscard]] std::optional<std::size_t> parse_size(std::string_view text) const
    {
        std::size_t out{};
        const auto result = std::from_chars(text.data(), text.data() + text.size(), out);
        if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) { return std::nullopt; }
        return out;
    }

    [[nodiscard]] CustomerCreateInput parse_create_input(
        const JsonValueView& view, ApiVersion version
    ) const
    {
        CustomerCreateInput input{};
        input.customer_id = view.string(version == ApiVersion::V1 ? "customer_id" : "merchant_reference_id");
        input.merchant_reference_id = view.string("merchant_reference_id");
        input.name = view.string("name");
        input.email = view.string("email");
        input.phone = view.string("phone");
        input.description = view.string("description");
        input.phone_country_code = view.string("phone_country_code");
        input.tax_registration_id = view.string("tax_registration_id");
        input.default_payment_method_id = view.string("default_payment_method_id");
        input.metadata_raw = view.raw("metadata");
        if (const auto address = view.object("address")) { input.address = parse_address(*address); }
        if (const auto address = view.object("default_billing_address")) { input.billing_address = parse_address(*address); }
        if (const auto address = view.object("default_shipping_address")) { input.shipping_address = parse_address(*address); }
        if (const auto document = view.object("document_details")) {
            DocumentInput parsed{};
            parsed.document_type = document->string("document_type");
            parsed.document_number = document->string("document_number");
            input.document_details = std::move(parsed);
        }
        return input;
    }

    [[nodiscard]] CustomerUpdateInput parse_update_input(const JsonValueView& view) const
    {
        CustomerUpdateInput input{};
        input.name = view.string("name");
        input.email = view.string("email");
        input.phone = view.string("phone");
        input.description = view.string("description");
        input.phone_country_code = view.string("phone_country_code");
        input.tax_registration_id = view.string("tax_registration_id");
        input.default_payment_method_id = view.string("default_payment_method_id");
        input.metadata_raw = view.raw("metadata");
        if (const auto address = view.object("address")) { input.address = parse_address(*address); }
        if (const auto address = view.object("default_billing_address")) { input.billing_address = parse_address(*address); }
        if (const auto address = view.object("default_shipping_address")) { input.shipping_address = parse_address(*address); }
        if (const auto document = view.object("document_details")) {
            DocumentInput parsed{};
            parsed.document_type = document->string("document_type");
            parsed.document_number = document->string("document_number");
            input.document_details = std::move(parsed);
        }
        return input;
    }

    [[nodiscard]] AddressInput parse_address(const JsonValueView& view) const
    {
        AddressInput address{};
        address.city = view.string("city");
        address.country = view.string("country");
        address.line1 = view.string("line1");
        address.line2 = view.string("line2");
        address.line3 = view.string("line3");
        address.zip = view.string("zip");
        address.state = view.string("state");
        address.first_name = view.string("first_name");
        address.last_name = view.string("last_name");
        address.origin_zip = view.string("origin_zip");
        return address;
    }

    // DocumentKind::validate (common_types/customers.rs:65-122).
    [[nodiscard]] std::optional<::core::error::AppError> validate_document_details(
        const std::optional<DocumentInput>& document
    ) const
    {
        if (!document) { return std::nullopt; }
        const std::string& type = document->document_type.value_or("");
        const std::string& number = document->document_number.value_or("");
        if (type.empty() || number.empty() || number.size() > 255U) {
            return upstream_error("InvalidRequestData", "invalid document_details");
        }
        bool valid{true};
        if (type == "psn") {
            valid = number.size() == 12U
                && std::all_of(number.begin(), number.end(), [](char c) {
                       return c >= '0' && c <= '9'; });
        } else if (type == "cpf") {
            valid = valid_cpf(number);
        } else if (type == "cnpj") {
            valid = valid_cnpj(number);
        }
        if (!valid) {
            return upstream_error("InvalidRequestData", "invalid document_details");
        }
        return std::nullopt;
    }

    [[nodiscard]] bool valid_cpf(std::string_view value) const noexcept
    {
        if (value.size() != 11U) { return false; }
        std::array<int, 11> digits{};
        for (std::size_t i{0}; i < 11U; ++i) {
            if (value[i] < '0' || value[i] > '9') { return false; }
            digits[i] = value[i] - '0';
        }
        const auto check = [&digits](std::size_t length) {
            int sum{0};
            for (std::size_t i{0}; i < length; ++i) { sum += digits[i] * static_cast<int>(length + 1 - i); }
            int result = (sum * 10) % 11;
            return result == 10 ? 0 : result;
        };
        return check(9) == digits[9] && check(10) == digits[10];
    }

    [[nodiscard]] bool valid_cnpj(std::string_view value) const noexcept
    {
        if (value.size() != 14U) { return false; }
        std::array<int, 14> digits{};
        for (std::size_t i{0}; i < 14U; ++i) {
            if (value[i] < '0' || value[i] > '9') { return false; }
            digits[i] = value[i] - '0';
        }
        const auto check = [&digits](std::size_t length) {
            int sum{0};
            std::size_t weight{length - 7};
            for (std::size_t i{0}; i < length; ++i) {
                sum += digits[i] * static_cast<int>(weight);
                weight = weight == 2 ? 9 : weight - 1;
            }
            const int result = sum % 11;
            return result < 2 ? 0 : 11 - result;
        };
        return check(12) == digits[12] && check(13) == digits[13];
    }

    [[nodiscard]] std::optional<::core::error::AppError> validate_phone_country_code(
        const std::optional<std::string>& code
    ) const
    {
        if (!code) { return std::nullopt; }
        constexpr std::size_t max_length{8U}; // common_utils/consts.rs:161
        const bool valid = !code->empty() && code->size() <= max_length
            && std::all_of(code->begin() + (code->front() == '+' ? 1 : 0), code->end(),
                [](char c) { return c >= '0' && c <= '9'; });
        if (!valid) {
            return upstream_error("InvalidRequestData",
                std::format("phone_country_code must be a valid country calling code, got \"{}\"", *code));
        }
        return std::nullopt;
    }

    /** @brief hyperswitch_domain_models::customer::is_global_customer_id_format (customer.rs:478-492). */
    [[nodiscard]] bool is_global_customer_id_format(std::string_view id) const noexcept
    {
        const std::size_t first = id.find('_');
        if (first == std::string_view::npos || first == 0) { return false; }
        const std::size_t second = id.find('_', first + 1);
        if (second == std::string_view::npos || second == first + 1) { return false; }
        if (id.find('_', second + 1) != std::string_view::npos) { return false; }
        const auto cell = id.substr(0, first);
        if (!std::all_of(cell.begin(), cell.end(), [](char c) {
                return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'); })) { return false; }
        if (id.substr(first + 1, second - first - 1) != "cus") { return false; }
        const auto uuid = id.substr(second + 1);
        if (uuid.size() != 32U) { return false; }
        return std::all_of(uuid.begin(), uuid.end(), [](char c) {
            return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); });
    }

    [[nodiscard]] std::string cell_id() const
    {
        if (m_context) {
            const auto value = m_context->config_store().get_value("cell_information", "id");
            if (value && !value->empty()) { return *value; }
        }
        return "0a";
    }

    [[nodiscard]] std::string generate_global_customer_id()
    {
        std::string uuid = m_context->ids().uuid_v4();
        uuid.erase(std::remove(uuid.begin(), uuid.end(), '-'), uuid.end());
        return cell_id() + "_cus_" + uuid;
    }

    [[nodiscard]] std::string generate_merchant_reference_id()
    {
        return m_context->ids().payment_id("cus_");
    }

    /** @brief Deterministic local key-manager substitute (core/customers.rs:182-236 et al). */
    [[nodiscard]] std::string pii_key() const
    {
        if (m_context) {
            const auto secret = m_context->key_store().get("customer_pii", "v1");
            if (secret) { return secret->value(); }
        }
        return "customer-pii-local-substitute";
    }

    [[nodiscard]] std::string encrypt_pii(std::string_view value) const
    {
        if (value.empty()) { return {}; }
        const std::string cipher = ::app::crypto::AesWrap{}.encrypt(value, pii_key());
        std::string out{"enc1:"};
        constexpr std::string_view digits{"0123456789abcdef"};
        for (const char ch : cipher) {
            const auto byte = static_cast<unsigned char>(ch);
            out.push_back(digits[(byte >> 4U) & 0xFU]);
            out.push_back(digits[byte & 0xFU]);
        }
        return out;
    }

    [[nodiscard]] std::string decrypt_pii(std::string_view value) const
    {
        constexpr std::string_view marker{"enc1:"};
        if (!value.starts_with(marker)) { return std::string{value}; }
        const std::string_view encoded = value.substr(marker.size());
        std::string raw{};
        raw.reserve(encoded.size() / 2U);
        const auto nibble = [](char ch) -> int {
            if (ch >= '0' && ch <= '9') { return ch - '0'; }
            if (ch >= 'a' && ch <= 'f') { return ch - 'a' + 10; }
            if (ch >= 'A' && ch <= 'F') { return ch - 'A' + 10; }
            return -1;
        };
        for (std::size_t i{0}; i + 1 < encoded.size(); i += 2) {
            const int high = nibble(encoded[i]);
            const int low = nibble(encoded[i + 1]);
            if (high < 0 || low < 0) { return std::string{value}; }
            raw.push_back(static_cast<char>((high << 4) | low));
        }
        return ::app::crypto::AesWrap{}.decrypt(raw, pii_key());
    }

    [[nodiscard]] CustomerAddressRecord to_address_record(
        const AddressInput& input, std::string_view kind
    ) const
    {
        CustomerAddressRecord record{};
        record.set_address_id(m_context->ids().payment_id("addr_"));
        record.set_address_kind(std::string{kind});
        record.set_city(input.city.value_or(""));
        record.set_country(input.country.value_or(""));
        record.set_line1(encrypt_pii(input.line1.value_or("")));
        record.set_line2(encrypt_pii(input.line2.value_or("")));
        record.set_line3(encrypt_pii(input.line3.value_or("")));
        record.set_zip(encrypt_pii(input.zip.value_or("")));
        record.set_state(encrypt_pii(input.state.value_or("")));
        record.set_first_name(encrypt_pii(input.first_name.value_or("")));
        record.set_last_name(encrypt_pii(input.last_name.value_or("")));
        record.set_origin_zip(input.origin_zip.value_or(""));
        return record;
    }

    template<typename E>
    [[nodiscard]] std::vector<E> query_by(std::string_view column, std::string_view value) const
    {
        auto spec = m_context->database().query<E>().spec();
        spec.predicates.push_back(core::db::Predicate{
            .column = std::string{column},
            .comparison = core::db::Compare::EQ,
            .value = core::db::DbValue{std::string{value}}
        });
        return m_context->database().collect<E>(spec);
    }

    [[nodiscard]] std::vector<CustomerExtensionRecord> extensions_for(std::string_view customer_id) const
    {
        return query_by<CustomerExtensionRecord>("customer_id", customer_id);
    }
    [[nodiscard]] std::vector<CustomerAddressRecord> addresses_for(std::string_view customer_id) const
    {
        return query_by<CustomerAddressRecord>("customer_id", customer_id);
    }

    [[nodiscard]] std::optional<CustomerAggregate>
    load_aggregate(const ::app::store::customers::CustomerRecord& record) const
    {
        CustomerAggregate aggregate{};
        aggregate.record = record;
        const auto extensions = extensions_for(record.get_customer_id());
        if (!extensions.empty()) { aggregate.extension = extensions.front(); }
        else {
            aggregate.extension.set_customer_id(record.get_customer_id());
            aggregate.extension.set_version(record.get_customer_id().starts_with("cus_") ? "v1" : "v2");
        }
        aggregate.addresses = addresses_for(record.get_customer_id());
        return aggregate;
    }

    [[nodiscard]] std::optional<CustomerAggregate>
    find_by_reference(
        ::app::store::AppRepository& repository,
        std::string_view reference, std::string_view merchant_id
    ) const
    {
        if (auto found = repository.find_customer_by_string(reference)) {
            if (merchant_id.empty() || found->get_merchant_id() == merchant_id) {
                return load_aggregate(*found);
            }
        }
        for (const auto& extension : query_by<CustomerExtensionRecord>("merchant_reference_id", reference)) {
            if (auto found = repository.find_customer_by_string(extension.get_customer_id())) {
                if (merchant_id.empty() || found->get_merchant_id() == merchant_id) {
                    return load_aggregate(*found);
                }
            }
        }
        return std::nullopt;
    }

    /** @brief v1 lookup with the merchant-reference-as-customer-id fallback
     *  (core/customers.rs:581-599 / 989-1005). */
    [[nodiscard]] std::optional<CustomerAggregate> locate_customer(
        ::app::store::AppRepository& repository,
        interfaces::io::IRequest& req,
        std::string_view id
    ) const
    {
        const std::string merchant_id = request_merchant_id(req);
        if (auto found = repository.find_customer_by_string(id)) {
            if (merchant_id.empty() || found->get_merchant_id() == merchant_id) {
                return load_aggregate(*found);
            }
        }
        if (auto found = find_by_reference(repository, id, merchant_id)) { return found; }
        std::int64_t numeric{};
        const auto converted = std::from_chars(id.data(), id.data() + id.size(), numeric);
        if (converted.ec == std::errc{} && converted.ptr == id.data() + id.size()) {
            if (auto found = repository.find_customer(numeric)) { return load_aggregate(*found); }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<CustomerAggregate> locate_global_customer(
        ::app::store::AppRepository& repository, std::string_view id
    ) const
    {
        auto found = repository.find_customer_by_string(id);
        if (!found) { return std::nullopt; }
        return load_aggregate(*found);
    }

    void persist_create(
        ::app::store::AppRepository& repository, CustomerAggregate& aggregate
    )
    {
        const auto saved = repository.save_customer(aggregate.record);
        aggregate.record = saved;
        aggregate.extension.set_customer_id(saved.get_customer_id());
        if (aggregate.extension.get_global_id().empty()) {
            aggregate.extension.set_global_id(saved.get_customer_id());
        }
        (void)m_context->database().insert(aggregate.extension);
        for (auto& address : aggregate.addresses) {
            address.set_customer_id(saved.get_customer_id());
            (void)m_context->database().insert(address);
        }
    }

    void persist_update(
        ::app::store::AppRepository& repository, const CustomerAggregate& aggregate
    )
    {
        (void)repository.update_customer(aggregate.record);
        if (aggregate.extension.get_id() != 0) {
            (void)m_context->database().update_where_column(
                aggregate.extension, "id", core::db::Compare::EQ,
                core::db::DbValue{aggregate.extension.get_id()});
        } else {
            auto extension = aggregate.extension;
            extension.set_id(0);
            (void)m_context->database().insert(extension);
        }
        for (const auto& address : aggregate.addresses) {
            if (address.get_id() != 0) {
                (void)m_context->database().update_where_column(
                    address, "id", core::db::Compare::EQ, core::db::DbValue{address.get_id()});
            } else {
                auto copy = address;
                copy.set_id(0);
                (void)m_context->database().insert(copy);
            }
        }
    }

    void upsert_address(
        ::app::store::AppRepository&, const CustomerAggregate& aggregate,
        const AddressInput& input, std::string_view kind
    )
    {
        for (const auto& existing : addresses_for(aggregate.record.get_customer_id())) {
            if (existing.get_address_kind() != kind) { continue; }
            auto updated = to_address_record(input, kind);
            updated.set_id(existing.get_id());
            updated.set_address_id(existing.get_address_id());
            updated.set_customer_id(existing.get_customer_id());
            (void)m_context->database().update_where_column(
                updated, "id", core::db::Compare::EQ, core::db::DbValue{updated.get_id()});
            return;
        }
        auto created = to_address_record(input, kind);
        created.set_customer_id(aggregate.record.get_customer_id());
        (void)m_context->database().insert(created);
    }

    [[nodiscard]] std::string encode_address(const CustomerAddressRecord& address) const
    {
        JsonObject out{};
        out.add_string("city", address.get_city());
        out.add_string("country", address.get_country());
        out.add_string("line1", decrypt_pii(address.get_line1()));
        out.add_string("line2", decrypt_pii(address.get_line2()));
        out.add_string("line3", decrypt_pii(address.get_line3()));
        out.add_string("zip", decrypt_pii(address.get_zip()));
        out.add_string("state", decrypt_pii(address.get_state()));
        out.add_string("first_name", decrypt_pii(address.get_first_name()));
        out.add_string("last_name", decrypt_pii(address.get_last_name()));
        out.add_string("origin_zip", address.get_origin_zip());
        return out.str();
    }

    [[nodiscard]] std::string encode_document(const CustomerExtensionRecord& extension) const
    {
        if (extension.get_document_type().empty() && extension.get_document_number().empty()) {
            return "null";
        }
        JsonObject out{};
        out.add_string("document_type", extension.get_document_type());
        out.add_string("document_number", decrypt_pii(extension.get_document_number()));
        return out.str();
    }

    [[nodiscard]] std::string encode_customer_response(
        const CustomerAggregate& aggregate,
        const std::optional<CustomerCreateInput>& create,
        const std::optional<CustomerUpdateInput>& update,
        ApiVersion version
    ) const
    {
        JsonObject out{};
        const std::string& customer_id = aggregate.record.get_customer_id();
        const auto tax = aggregate.extension.get_tax_registration_id().empty()
            ? std::optional<std::string>{}
            : std::optional<std::string>{decrypt_pii(aggregate.extension.get_tax_registration_id())};
        const auto metadata = aggregate.record.get_metadata().empty()
            ? std::optional<std::string>{}
            : std::optional<std::string>{aggregate.record.get_metadata()};
        const auto country = aggregate.extension.get_phone_country_code().empty()
            ? std::optional<std::string>{}
            : std::optional<std::string>{aggregate.extension.get_phone_country_code()};
        const auto description = aggregate.record.get_description().empty()
            ? std::optional<std::string>{}
            : std::optional<std::string>{aggregate.record.get_description()};
        const auto pm = aggregate.extension.get_default_payment_method_id().empty()
            ? std::optional<std::string>{}
            : std::optional<std::string>{aggregate.extension.get_default_payment_method_id()};
        const auto name = aggregate.record.get_name().empty()
            ? std::optional<std::string>{} : std::optional<std::string>{decrypt_pii(aggregate.record.get_name())};
        const auto email = aggregate.record.get_email().empty()
            ? std::optional<std::string>{} : std::optional<std::string>{decrypt_pii(aggregate.record.get_email())};
        const auto phone = aggregate.record.get_phone().empty()
            ? std::optional<std::string>{} : std::optional<std::string>{decrypt_pii(aggregate.record.get_phone())};

        if (version == ApiVersion::V1) {
            out.add_string("customer_id", customer_id);
            out.add_string("name", name);
            out.add_string("email", email);
            out.add_string("phone", phone);
            out.add_string("phone_country_code", country);
            out.add_string("description", description);
            // v1 `address` comes from the request on write and from address_id on read.
            const auto address = find_address(aggregate, "v1");
            out.add_object("address", address ? encode_address(*address) : std::string{"null"});
            out.add_string("created_at", aggregate.record.get_created_at());
            out.add_raw("metadata", metadata);
            out.add_string("default_payment_method_id", pm);
            out.add_string("tax_registration_id", tax);
            out.add_raw("document_details", encode_document(aggregate.extension));
        } else {
            out.add_string("id", aggregate.extension.get_global_id().empty()
                ? customer_id : aggregate.extension.get_global_id());
            const auto& reference = aggregate.extension.get_merchant_reference_id();
            out.add_string("merchant_reference_id", reference.empty()
                ? std::optional<std::string>{} : std::optional<std::string>{reference});
            const auto connector = aggregate.extension.get_connector_customer_ids().empty()
                ? std::optional<std::string>{}
                : std::optional<std::string>{aggregate.extension.get_connector_customer_ids()};
            out.add_raw("connector_customer_ids", connector);
            out.add_string("name", name);
            out.add_string("email", email);
            out.add_string("phone", phone);
            out.add_string("phone_country_code", country);
            out.add_string("description", description);
            const auto billing = find_address(aggregate, "billing");
            const auto shipping = find_address(aggregate, "shipping");
            out.add_object("default_billing_address", billing ? encode_address(*billing) : std::string{"null"});
            out.add_object("default_shipping_address", shipping ? encode_address(*shipping) : std::string{"null"});
            out.add_string("created_at", aggregate.record.get_created_at());
            out.add_raw("metadata", metadata);
            out.add_string("default_payment_method_id", pm);
            out.add_string("tax_registration_id", tax);
            out.add_raw("document_details", encode_document(aggregate.extension));
        }
        // Values present only on the originating request override persisted fields,
        // mirroring `CustomerResponse::try_from((customer, address))`.
        if (create) {
            if (create->description) { out.add_string("description", *create->description); }
            if (create->phone_country_code) { out.add_string("phone_country_code", *create->phone_country_code); }
            if (create->tax_registration_id) { out.add_string("tax_registration_id", *create->tax_registration_id); }
        }
        if (update) {
            if (update->description) { out.add_string("description", *update->description); }
            if (update->phone_country_code) { out.add_string("phone_country_code", *update->phone_country_code); }
            if (update->tax_registration_id) { out.add_string("tax_registration_id", *update->tax_registration_id); }
        }
        return out.str();
    }

    [[nodiscard]] const CustomerAddressRecord* find_address(
        const CustomerAggregate& aggregate, std::string_view kind
    ) const
    {
        for (const auto& address : aggregate.addresses) {
            if (address.get_address_kind() == kind) { return &address; }
        }
        return nullptr;
    }

    [[nodiscard]] std::optional<std::pair<std::string, std::string>> parse_time_range(
        const app::model::customers::CustomerTimeRange& range
    ) const
    {
        if (range.get_start_time().empty()) { return std::nullopt; }
        return std::pair<std::string, std::string>{
            trim(range.get_start_time()),
            trim(range.get_end_time().value_or(""))};
    }

    [[nodiscard]] bool within_range(
        std::string_view created_at, const app::model::customers::CustomerTimeRange& range
    ) const
    {
        const auto bounds = parse_time_range(range);
        if (!bounds) { return true; }
        if (created_at < bounds->first) { return false; }
        if (!bounds->second.empty() && created_at > bounds->second) { return false; }
        return true;
    }

    [[nodiscard]] std::string trim(std::string_view value) const
    {
        std::size_t begin{0};
        std::size_t end{value.size()};
        while (begin < end && std::isspace(static_cast<unsigned char>(value[begin]))) { ++begin; }
        while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) { --end; }
        return std::string{value.substr(begin, end - begin)};
    }

    [[nodiscard]] std::string now_iso8601() const
    {
        const auto now = std::chrono::system_clock::now();
        const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        const std::time_t seconds = static_cast<std::time_t>(millis / 1000);
        std::tm parts{};
        gmtime_r(&seconds, &parts);
        std::ostringstream out{};
        out << std::put_time(&parts, "%Y-%m-%dT%H:%M:%S") << '.'
            << std::setw(3) << std::setfill('0') << (millis % 1000) << 'Z';
        return out.str();
    }

    // ────────────── v2 global-id migration (routes/customers/migrate.rs) ──────

    using MigrationOutcome = std::variant<
        MigrationRow, ::app::model::customers::CustomerGlobalIdMigrationRowResult>;

    [[nodiscard]] std::expected<std::vector<MigrationOutcome>, ::core::error::AppError>
    parse_customer_global_id_migration_csv(std::string_view body) const
    {
        if (body.size() > kMigrationMaxFileSize) {
            return std::unexpected{upstream_error("InvalidRequestData",
                std::format("CSV file exceeds maximum size of {} bytes", kMigrationMaxFileSize))};
        }
        std::vector<std::string> lines{};
        std::string current{};
        const auto finish = [&]() {
            if (!current.empty()) { lines.push_back(current); }
            current.clear();
        };
        for (const char ch : body) {
            if (ch == '\n') { finish(); }
            else if (ch != '\r') { current.push_back(ch); }
        }
        finish();
        if (lines.empty() || trim(lines.front()) != "merchant_id,customer_id") {
            return std::unexpected{upstream_error("InvalidRequestData",
                "CSV must include header: merchant_id,customer_id")};
        }
        std::vector<MigrationOutcome> rows{};
        for (std::size_t i{1}; i < lines.size(); ++i) {
            if (i - 1 >= kMigrationMaxRecords) {
                return std::unexpected{upstream_error("InvalidRequestData",
                    std::format("CSV file exceeds maximum record count of {}", kMigrationMaxRecords))};
            }
            const std::size_t comma = lines[i].find(',');
            if (comma == std::string::npos) {
                rows.push_back(invalid_csv_row_result(i + 1, "", "",
                    "CSV row must contain valid merchant_id and customer_id values"));
                continue;
            }
            MigrationRow row{};
            row.row_number = i + 1;
            row.merchant_id = trim(lines[i].substr(0, comma));
            row.customer_id = trim(lines[i].substr(comma + 1));
            if (row.merchant_id.empty() || row.customer_id.empty()
                || lines[i].find(',', comma + 1) != std::string::npos) {
                rows.push_back(invalid_csv_row_result(
                    row.row_number, row.merchant_id, row.customer_id,
                    "CSV row must contain valid merchant_id and customer_id values"));
                continue;
            }
            rows.push_back(std::move(row));
        }
        return rows;
    }

    [[nodiscard]] ::app::model::customers::CustomerGlobalIdMigrationRowResult
    invalid_csv_row_result(
        std::size_t row_number, std::string merchant_id, std::string customer_id,
        std::string message
    ) const
    {
        ::app::model::customers::CustomerGlobalIdMigrationRowResult result{};
        result.set_row_number(row_number);
        result.set_merchant_id(merchant_id.empty() ? std::nullopt : std::optional<std::string>{merchant_id});
        result.set_customer_id(customer_id.empty() ? std::nullopt : std::optional<std::string>{customer_id});
        result.set_status("InvalidCsvRow");
        result.set_message(std::move(message));
        return result;
    }

    [[nodiscard]] ::app::model::customers::CustomerGlobalIdMigrationRowResult
    migration_result(
        const MigrationRow& row, std::string status,
        std::optional<std::string> new_id, std::string message
    ) const
    {
        ::app::model::customers::CustomerGlobalIdMigrationRowResult result{};
        result.set_row_number(row.row_number);
        result.set_merchant_id(row.merchant_id);
        result.set_customer_id(row.customer_id);
        result.set_status(std::move(status));
        result.set_old_id(row.old_id);
        result.set_new_id(std::move(new_id));
        result.set_message(std::move(message));
        return result;
    }

    [[nodiscard]] ::app::model::customers::CustomerGlobalIdMigrationRowResult
    build_migration_result(
        const ::app::model::customers::CustomerGlobalIdMigrationRowResult& row
    ) const { return row; }

    [[nodiscard]] ::app::model::customers::CustomerGlobalIdMigrationRowResult
    process_fetched_row(
        ::app::store::AppRepository& repository, MigrationRow row
    )
    {
        auto aggregate = find_by_reference(repository, row.customer_id, row.merchant_id);
        if (!aggregate) {
            return migration_result(row, "NotFound", std::nullopt,
                "Customer not found for merchant_id and customer_id");
        }
        row.old_id = aggregate->extension.get_global_id().empty()
            ? std::optional<std::string>{} : std::optional<std::string>{aggregate->extension.get_global_id()};
        row.version = aggregate->extension.get_version();

        if (!row.version || *row.version != "v1") {
            return migration_result(row, "SkippedNonV1", std::nullopt,
                "Customer is not a v1 row; skipping migration");
        }
        if (row.old_id && is_global_customer_id_format(*row.old_id)) {
            return migration_result(row, "AlreadyGlobalId", std::nullopt,
                "Customer id is already in global id format");
        }
        const std::string status = row.old_id ? "UpdatedNonGlobalId" : "UpdatedNullId";
        const std::string new_id = generate_global_customer_id();
        aggregate->extension.set_global_id(new_id);
        aggregate->extension.set_version("v2");
        persist_update(repository, *aggregate);
        return migration_result(row, status, new_id, "Customer id updated successfully");
    }

    [[nodiscard]] std::string encode_migration_row(
        const ::app::model::customers::CustomerGlobalIdMigrationRowResult& row
    ) const
    {
        JsonObject out{};
        out.add_number("row_number", row.get_row_number());
        out.add_string("merchant_id", row.get_merchant_id());
        out.add_string("customer_id", row.get_customer_id());
        out.add_string("status", row.get_status());
        out.add_string("old_id", row.get_old_id());
        out.add_string("new_id", row.get_new_id());
        out.add_string("message", row.get_message());
        return out.str();
    }

    ::app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::customers
