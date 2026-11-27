module;
#include <ctime>
export module app_user:customers_runtime_common;

import std;
import interfaces;
import core_serde;
import core_database;
import core_error;
import app_context;
import app_store;
import app_model;
import app_crypto;
import app_metrics;

import :customers_input;

export namespace app::handlers::customers {

// ── Local in-process persistence for fields the shared CustomerRecord lacks ──

class CustomerExtensionRecord
{
public:
    void set_id(std::int64_t value)
    {
        m_id = value;
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_customer_id(std::string value)
    {
        m_customer_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_customer_id() const noexcept
    {
        return m_customer_id;
    }

    void set_global_id(std::string value)
    {
        m_global_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_global_id() const noexcept
    {
        return m_global_id;
    }

    void set_merchant_reference_id(std::string value)
    {
        m_merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_reference_id() const noexcept
    {
        return m_merchant_reference_id;
    }

    void set_version(std::string value)
    {
        m_version = std::move(value);
    }

    [[nodiscard]] const std::string& get_version() const noexcept
    {
        return m_version;
    }

    void set_phone_country_code(std::string value)
    {
        m_phone_country_code = std::move(value);
    }

    [[nodiscard]] const std::string& get_phone_country_code() const noexcept
    {
        return m_phone_country_code;
    }

    void set_tax_registration_id(std::string value)
    {
        m_tax_registration_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_tax_registration_id() const noexcept
    {
        return m_tax_registration_id;
    }

    void set_document_type(std::string value)
    {
        m_document_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_document_type() const noexcept
    {
        return m_document_type;
    }

    void set_document_number(std::string value)
    {
        m_document_number = std::move(value);
    }

    [[nodiscard]] const std::string& get_document_number() const noexcept
    {
        return m_document_number;
    }

    void set_connector_customer_ids(std::string value)
    {
        m_connector_customer_ids = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_customer_ids() const noexcept
    {
        return m_connector_customer_ids;
    }

    void set_default_payment_method_id(std::string value)
    {
        m_default_payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_default_payment_method_id() const noexcept
    {
        return m_default_payment_method_id;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

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
    void set_id(std::int64_t value)
    {
        m_id = value;
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_address_id(std::string value)
    {
        m_address_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_address_id() const noexcept
    {
        return m_address_id;
    }

    void set_customer_id(std::string value)
    {
        m_customer_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_customer_id() const noexcept
    {
        return m_customer_id;
    }

    void set_address_kind(std::string value)
    {
        m_address_kind = std::move(value);
    }

    [[nodiscard]] const std::string& get_address_kind() const noexcept
    {
        return m_address_kind;
    }

    void set_city(std::string value)
    {
        m_city = std::move(value);
    }

    [[nodiscard]] const std::string& get_city() const noexcept
    {
        return m_city;
    }

    void set_country(std::string value)
    {
        m_country = std::move(value);
    }

    [[nodiscard]] const std::string& get_country() const noexcept
    {
        return m_country;
    }

    void set_line1(std::string value)
    {
        m_line1 = std::move(value);
    }

    [[nodiscard]] const std::string& get_line1() const noexcept
    {
        return m_line1;
    }

    void set_line2(std::string value)
    {
        m_line2 = std::move(value);
    }

    [[nodiscard]] const std::string& get_line2() const noexcept
    {
        return m_line2;
    }

    void set_line3(std::string value)
    {
        m_line3 = std::move(value);
    }

    [[nodiscard]] const std::string& get_line3() const noexcept
    {
        return m_line3;
    }

    void set_zip(std::string value)
    {
        m_zip = std::move(value);
    }

    [[nodiscard]] const std::string& get_zip() const noexcept
    {
        return m_zip;
    }

    void set_state(std::string value)
    {
        m_state = std::move(value);
    }

    [[nodiscard]] const std::string& get_state() const noexcept
    {
        return m_state;
    }

    void set_first_name(std::string value)
    {
        m_first_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_first_name() const noexcept
    {
        return m_first_name;
    }

    void set_last_name(std::string value)
    {
        m_last_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_last_name() const noexcept
    {
        return m_last_name;
    }

    void set_origin_zip(std::string value)
    {
        m_origin_zip = std::move(value);
    }

    [[nodiscard]] const std::string& get_origin_zip() const noexcept
    {
        return m_origin_zip;
    }

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

/** @brief Typed aggregate: the shared record plus its local extension and addresses. */
struct CustomerAggregate
{
    ::app::store::customers::CustomerRecord record{};
    CustomerExtensionRecord extension{};
    std::vector<CustomerAddressRecord> addresses{};
};

} // namespace app::handlers::customers

template<>
struct serde::Serializable<::app::handlers::customers::CustomerExtensionRecord>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::customers::CustomerExtensionRecord;
        return std::tuple{
            serde::FieldDesc<
                "id",
                &T::get_id,
                &T::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "customer_id",
                &T::get_customer_id,
                &T::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "global_id",
                &T::get_global_id,
                &T::set_global_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &T::get_merchant_reference_id,
                &T::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"version", &T::get_version, &T::set_version, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone_country_code",
                &T::get_phone_country_code,
                &T::set_phone_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_registration_id",
                &T::get_tax_registration_id,
                &T::set_tax_registration_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "document_type",
                &T::get_document_type,
                &T::set_document_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "document_number",
                &T::get_document_number,
                &T::set_document_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_customer_ids",
                &T::get_connector_customer_ids,
                &T::set_connector_customer_ids,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "default_payment_method_id",
                &T::get_default_payment_method_id,
                &T::set_default_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &T::get_status, &T::set_status, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_extensions";
    }
};

template<>
struct serde::Serializable<::app::handlers::customers::CustomerAddressRecord>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::customers::CustomerAddressRecord;
        return std::tuple{
            serde::FieldDesc<
                "id",
                &T::get_id,
                &T::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "address_id",
                &T::get_address_id,
                &T::set_address_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &T::get_customer_id,
                &T::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "address_kind",
                &T::get_address_kind,
                &T::set_address_kind,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"city", &T::get_city, &T::set_city, serde::FieldOptions{}>{},
            serde::FieldDesc<"country", &T::get_country, &T::set_country, serde::FieldOptions{}>{},
            serde::FieldDesc<"line1", &T::get_line1, &T::set_line1, serde::FieldOptions{}>{},
            serde::FieldDesc<"line2", &T::get_line2, &T::set_line2, serde::FieldOptions{}>{},
            serde::FieldDesc<"line3", &T::get_line3, &T::set_line3, serde::FieldOptions{}>{},
            serde::FieldDesc<"zip", &T::get_zip, &T::set_zip, serde::FieldOptions{}>{},
            serde::FieldDesc<"state", &T::get_state, &T::set_state, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "first_name",
                &T::get_first_name,
                &T::set_first_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_name",
                &T::get_last_name,
                &T::set_last_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "origin_zip",
                &T::get_origin_zip,
                &T::set_origin_zip,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_addresses";
    }
};

export namespace app::handlers::customers {

class CustomersRuntimeSupport
{
public:
    explicit CustomersRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] ::core::error::AppError
    error(::core::error::AppErrorKind kind, std::string message) const
    {
        return ::core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    [[nodiscard]] ::core::error::AppError
    upstream_error(std::string_view wire_name, std::string message) const
    {
        using K = ::core::error::AppErrorKind;
        const K kind = wire_name == "CustomerAlreadyExists" ? K::ALREADY_EXISTS
                       : wire_name == "CustomerNotFound"    ? K::NOT_FOUND
                       : wire_name == "MandateActive"       ? K::PRECONDITION_FAILED
                       : wire_name == "AccessForbidden"     ? K::FORBIDDEN
                       : wire_name == "InternalServerError" ? K::INTERNAL_ERROR
                                                            : K::VALIDATION_FAILED;
        // Upstream code lives in `kind_name` (the serialized `error` field),
        // canonical human-readable text in `message`.
        std::string code{wire_name};
        std::string canonical{message};
        const CustomersInputSupport support{m_context};
        if (const auto wire = support.upstream_wire(wire_name, message)) {
            code = std::string{std::get<0>(*wire)};
            canonical = std::get<2>(*wire);
        }
        auto result = error(kind, std::move(canonical));
        result.set_kind_name(std::move(code));
        return result;
    }

    // ───────────────────────── ID / crypto helpers ─────────────────────────

    [[nodiscard]] std::string cell_id() const
    {
        const auto value = m_context.config_store().get_value("cell_information", "id");
        if (value && !value->empty()) {
            return *value;
        }
        return "0a";
    }

    [[nodiscard]] std::string generate_global_customer_id() const
    {
        std::string uuid = m_context.ids().uuid_v4();
        uuid.erase(std::remove(uuid.begin(), uuid.end(), '-'), uuid.end());
        return cell_id() + "_cus_" + uuid;
    }

    [[nodiscard]] std::string generate_merchant_reference_id() const
    {
        return m_context.ids().payment_id("cus_");
    }

    /** @brief Deterministic local key-manager substitute (core/customers.rs:182-236 et al). */
    [[nodiscard]] std::string pii_key() const
    {
        const auto secret = m_context.key_store().get("customer_pii", "v1");
        if (secret) {
            return secret->value();
        }
        return "customer-pii-local-substitute";
    }

    [[nodiscard]] std::string encrypt_pii(std::string_view value) const
    {
        if (value.empty()) {
            return {};
        }
        const std::string cipher = ::app::crypto::AesWrap{}.encrypt(value, pii_key());
        std::string out{"enc1:"};
        constexpr std::string_view digits{"0123456789abcdef"};
        for (const char ch: cipher) {
            const auto byte = static_cast<unsigned char>(ch);
            out.push_back(digits[(byte >> 4U) & 0xFU]);
            out.push_back(digits[byte & 0xFU]);
        }
        return out;
    }

    [[nodiscard]] std::string decrypt_pii(std::string_view value) const
    {
        constexpr std::string_view marker{"enc1:"};
        if (!value.starts_with(marker)) {
            return std::string{value};
        }
        const std::string_view encoded = value.substr(marker.size());
        std::string raw{};
        raw.reserve(encoded.size() / 2U);
        const auto nibble = [](char ch) -> int
        {
            if (ch >= '0' && ch <= '9') {
                return ch - '0';
            }
            if (ch >= 'a' && ch <= 'f') {
                return ch - 'a' + 10;
            }
            if (ch >= 'A' && ch <= 'F') {
                return ch - 'A' + 10;
            }
            return -1;
        };
        for (std::size_t i{0}; i + 1 < encoded.size(); i += 2) {
            const int high = nibble(encoded[i]);
            const int low = nibble(encoded[i + 1]);
            if (high < 0 || low < 0) {
                return std::string{value};
            }
            raw.push_back(static_cast<char>((high << 4) | low));
        }
        return ::app::crypto::AesWrap{}.decrypt(raw, pii_key());
    }

    [[nodiscard]] std::string now_iso8601() const
    {
        const auto now = std::chrono::system_clock::now();
        const auto millis =
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        const std::time_t seconds = static_cast<std::time_t>(millis / 1'000);
        std::tm parts{};
        gmtime_r(&seconds, &parts);
        std::ostringstream out{};
        out << std::put_time(&parts, "%Y-%m-%dT%H:%M:%S") << '.' << std::setw(3)
            << std::setfill('0') << (millis % 1'000) << 'Z';
        return out.str();
    }

    /** @brief hyperswitch_domain_models::customer::is_global_customer_id_format. */
    [[nodiscard]] bool is_global_customer_id_format(std::string_view id) const noexcept
    {
        const std::size_t first = id.find('_');
        if (first == std::string_view::npos || first == 0) {
            return false;
        }
        const std::size_t second = id.find('_', first + 1);
        if (second == std::string_view::npos || second == first + 1) {
            return false;
        }
        if (id.find('_', second + 1) != std::string_view::npos) {
            return false;
        }
        const auto cell = id.substr(0, first);
        if (!std::all_of(
                cell.begin(),
                cell.end(),
                [](char c)
                {
                    return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
                }
            )) {
            return false;
        }
        if (id.substr(first + 1, second - first - 1) != "cus") {
            return false;
        }
        const auto uuid = id.substr(second + 1);
        if (uuid.size() != 32U) {
            return false;
        }
        return std::all_of(
            uuid.begin(),
            uuid.end(),
            [](char c)
            {
                return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
            }
        );
    }

    // ───────────────────────── Persistence helpers ─────────────────────────

    template<typename E>
    [[nodiscard]] std::vector<E> query_by(std::string_view column, std::string_view value) const
    {
        auto spec = m_context.database().query<E>().spec();
        spec.predicates.push_back(
            core::db::Predicate{
                .column = std::string{column},
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{value}}
            }
        );
        return m_context.database().collect<E>(spec);
    }

    [[nodiscard]] std::vector<CustomerExtensionRecord>
    extensions_for(std::string_view customer_id) const
    {
        return query_by<CustomerExtensionRecord>("customer_id", customer_id);
    }

    [[nodiscard]] std::vector<CustomerAddressRecord>
    addresses_for(std::string_view customer_id) const
    {
        return query_by<CustomerAddressRecord>("customer_id", customer_id);
    }

    [[nodiscard]] std::optional<CustomerAggregate>
    load_aggregate(const ::app::store::customers::CustomerRecord& record) const
    {
        CustomerAggregate aggregate{};
        aggregate.record = record;
        const auto extensions = extensions_for(record.get_customer_id());
        if (!extensions.empty()) {
            aggregate.extension = extensions.front();
        } else {
            aggregate.extension.set_customer_id(record.get_customer_id());
            aggregate.extension.set_version(
                record.get_customer_id().starts_with("cus_") ? "v1" : "v2"
            );
        }
        aggregate.addresses = addresses_for(record.get_customer_id());
        return aggregate;
    }

    [[nodiscard]] std::optional<CustomerAggregate> find_by_reference(
        ::app::store::AppRepository& repository,
        std::string_view reference,
        std::string_view merchant_id
    ) const
    {
        if (auto found = repository.find_customer_by_string(reference)) {
            if (merchant_id.empty() || found->get_merchant_id() == merchant_id) {
                return load_aggregate(*found);
            }
        }
        for (const auto& extension:
             query_by<CustomerExtensionRecord>("merchant_reference_id", reference)) {
            if (auto found = repository.find_customer_by_string(extension.get_customer_id())) {
                if (merchant_id.empty() || found->get_merchant_id() == merchant_id) {
                    return load_aggregate(*found);
                }
            }
        }
        return std::nullopt;
    }

    /** @brief v1 lookup with the merchant-reference-as-customer-id fallback. */
    [[nodiscard]] std::optional<CustomerAggregate> locate_customer(
        ::app::store::AppRepository& repository,
        std::string_view merchant_id,
        std::string_view id
    ) const
    {
        if (auto found = repository.find_customer_by_string(id)) {
            if (merchant_id.empty() || found->get_merchant_id() == merchant_id) {
                return load_aggregate(*found);
            }
        }
        if (auto found = find_by_reference(repository, id, merchant_id)) {
            return found;
        }
        std::int64_t numeric{};
        const auto converted = std::from_chars(id.data(), id.data() + id.size(), numeric);
        if (converted.ec == std::errc{} && converted.ptr == id.data() + id.size()) {
            if (auto found = repository.find_customer(numeric)) {
                return load_aggregate(*found);
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<CustomerAggregate>
    locate_global_customer(::app::store::AppRepository& repository, std::string_view id) const
    {
        auto found = repository.find_customer_by_string(id);
        if (!found) {
            return std::nullopt;
        }
        return load_aggregate(*found);
    }

    [[nodiscard]] CustomerAddressRecord
    to_address_record(const CustomerAddressDto& input, std::string_view kind) const
    {
        CustomerAddressRecord record{};
        record.set_address_id(m_context.ids().payment_id("addr_"));
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

    void persist_create(::app::store::AppRepository& repository, CustomerAggregate& aggregate) const
    {
        const auto saved = repository.save_customer(aggregate.record);
        aggregate.record = saved;
        aggregate.extension.set_customer_id(saved.get_customer_id());
        if (aggregate.extension.get_global_id().empty()) {
            aggregate.extension.set_global_id(saved.get_customer_id());
        }
        (void)m_context.database().insert(aggregate.extension);
        for (auto& address: aggregate.addresses) {
            address.set_customer_id(saved.get_customer_id());
            (void)m_context.database().insert(address);
        }
    }

    void persist_update(
        ::app::store::AppRepository& repository,
        const CustomerAggregate& aggregate
    ) const
    {
        (void)repository.update_customer(aggregate.record);
        if (aggregate.extension.get_id() != 0) {
            (void)m_context.database().update_where_column(
                aggregate.extension,
                "id",
                core::db::Compare::EQ,
                core::db::DbValue{aggregate.extension.get_id()}
            );
        } else {
            auto extension = aggregate.extension;
            extension.set_id(0);
            (void)m_context.database().insert(extension);
        }
        for (const auto& address: aggregate.addresses) {
            if (address.get_id() != 0) {
                (void)m_context.database().update_where_column(
                    address,
                    "id",
                    core::db::Compare::EQ,
                    core::db::DbValue{address.get_id()}
                );
            } else {
                auto copy = address;
                copy.set_id(0);
                (void)m_context.database().insert(copy);
            }
        }
    }

    void upsert_address(
        ::app::store::AppRepository&,
        const CustomerAggregate& aggregate,
        const CustomerAddressDto& input,
        std::string_view kind
    ) const
    {
        for (const auto& existing: addresses_for(aggregate.record.get_customer_id())) {
            if (existing.get_address_kind() != kind) {
                continue;
            }
            auto updated = to_address_record(input, kind);
            updated.set_id(existing.get_id());
            updated.set_address_id(existing.get_address_id());
            updated.set_customer_id(existing.get_customer_id());
            (void)m_context.database().update_where_column(
                updated,
                "id",
                core::db::Compare::EQ,
                core::db::DbValue{updated.get_id()}
            );
            return;
        }
        auto created = to_address_record(input, kind);
        created.set_customer_id(aggregate.record.get_customer_id());
        (void)m_context.database().insert(created);
    }

    // ───────────────────────── Response documents ─────────────────────────

    /** @brief Parses stored JSON text into a document for embedding in a response. */
    [[nodiscard]] serde::Value json_document(std::string_view text) const
    {
        const auto parsed = serde::Ser::decode_generic("application/json", text);
        return parsed ? std::move(*parsed) : serde::Value{};
    }

    [[nodiscard]] serde::Value address_document(const CustomerAddressRecord& address) const
    {
        serde::Value::Object out{};
        out["city"] = serde::Value{address.get_city()};
        out["country"] = serde::Value{address.get_country()};
        out["line1"] = serde::Value{decrypt_pii(address.get_line1())};
        out["line2"] = serde::Value{decrypt_pii(address.get_line2())};
        out["line3"] = serde::Value{decrypt_pii(address.get_line3())};
        out["zip"] = serde::Value{decrypt_pii(address.get_zip())};
        out["state"] = serde::Value{decrypt_pii(address.get_state())};
        out["first_name"] = serde::Value{decrypt_pii(address.get_first_name())};
        out["last_name"] = serde::Value{decrypt_pii(address.get_last_name())};
        out["origin_zip"] = serde::Value{address.get_origin_zip()};
        return serde::Value{std::move(out)};
    }

    [[nodiscard]] serde::Value
    document_details_document(const CustomerExtensionRecord& extension) const
    {
        if (extension.get_document_type().empty() && extension.get_document_number().empty()) {
            return serde::Value{};
        }
        serde::Value::Object out{};
        out["document_type"] = serde::Value{extension.get_document_type()};
        out["document_number"] = serde::Value{decrypt_pii(extension.get_document_number())};
        return serde::Value{std::move(out)};
    }

    [[nodiscard]] const CustomerAddressRecord*
    find_address(const CustomerAggregate& aggregate, std::string_view kind) const
    {
        for (const auto& address: aggregate.addresses) {
            if (address.get_address_kind() == kind) {
                return &address;
            }
        }
        return nullptr;
    }

    [[nodiscard]] serde::Value customer_document(
        const CustomerAggregate& aggregate,
        const std::optional<CustomerCreateDto>& create,
        const std::optional<CustomerUpdateDto>& update,
        ApiVersion version
    ) const
    {
        serde::Value::Object out{};
        const std::string& customer_id = aggregate.record.get_customer_id();
        const auto tax = aggregate.extension.get_tax_registration_id().empty()
                             ? std::optional<std::string>{}
                             : std::optional<std::string>{
                                   decrypt_pii(aggregate.extension.get_tax_registration_id())
                               };
        const auto metadata = aggregate.record.get_metadata().empty()
                                  ? std::optional<std::string>{}
                                  : std::optional<std::string>{aggregate.record.get_metadata()};
        const auto country =
            aggregate.extension.get_phone_country_code().empty()
                ? std::optional<std::string>{}
                : std::optional<std::string>{aggregate.extension.get_phone_country_code()};
        const auto description =
            aggregate.record.get_description().empty()
                ? std::optional<std::string>{}
                : std::optional<std::string>{aggregate.record.get_description()};
        const auto pm =
            aggregate.extension.get_default_payment_method_id().empty()
                ? std::optional<std::string>{}
                : std::optional<std::string>{aggregate.extension.get_default_payment_method_id()};
        const auto name =
            aggregate.record.get_name().empty()
                ? std::optional<std::string>{}
                : std::optional<std::string>{decrypt_pii(aggregate.record.get_name())};
        const auto email =
            aggregate.record.get_email().empty()
                ? std::optional<std::string>{}
                : std::optional<std::string>{decrypt_pii(aggregate.record.get_email())};
        const auto phone =
            aggregate.record.get_phone().empty()
                ? std::optional<std::string>{}
                : std::optional<std::string>{decrypt_pii(aggregate.record.get_phone())};

        if (version == ApiVersion::V1) {
            out["customer_id"] = serde::Value{customer_id};
            out["name"] = name ? serde::Value{*name} : serde::Value{};
            out["email"] = email ? serde::Value{*email} : serde::Value{};
            out["phone"] = phone ? serde::Value{*phone} : serde::Value{};
            out["phone_country_code"] = country ? serde::Value{*country} : serde::Value{};
            out["description"] = description ? serde::Value{*description} : serde::Value{};
            const auto address = find_address(aggregate, "v1");
            out["address"] = address ? address_document(*address) : serde::Value{};
            out["created_at"] = serde::Value{aggregate.record.get_created_at()};
            out["metadata"] = metadata ? json_document(*metadata) : serde::Value{};
            out["default_payment_method_id"] = pm ? serde::Value{*pm} : serde::Value{};
            out["tax_registration_id"] = tax ? serde::Value{*tax} : serde::Value{};
            out["document_details"] = document_details_document(aggregate.extension);
        } else {
            out["id"] = serde::Value{
                aggregate.extension.get_global_id().empty() ? customer_id
                                                            : aggregate.extension.get_global_id()
            };
            const auto& reference = aggregate.extension.get_merchant_reference_id();
            out["merchant_reference_id"] =
                reference.empty() ? serde::Value{} : serde::Value{reference};
            const auto connector =
                aggregate.extension.get_connector_customer_ids().empty()
                    ? std::optional<std::string>{}
                    : std::optional<std::string>{aggregate.extension.get_connector_customer_ids()};
            out["connector_customer_ids"] = connector ? json_document(*connector) : serde::Value{};
            out["name"] = name ? serde::Value{*name} : serde::Value{};
            out["email"] = email ? serde::Value{*email} : serde::Value{};
            out["phone"] = phone ? serde::Value{*phone} : serde::Value{};
            out["phone_country_code"] = country ? serde::Value{*country} : serde::Value{};
            out["description"] = description ? serde::Value{*description} : serde::Value{};
            const auto billing = find_address(aggregate, "billing");
            const auto shipping = find_address(aggregate, "shipping");
            out["default_billing_address"] = billing ? address_document(*billing) : serde::Value{};
            out["default_shipping_address"] =
                shipping ? address_document(*shipping) : serde::Value{};
            out["created_at"] = serde::Value{aggregate.record.get_created_at()};
            out["metadata"] = metadata ? json_document(*metadata) : serde::Value{};
            out["default_payment_method_id"] = pm ? serde::Value{*pm} : serde::Value{};
            out["tax_registration_id"] = tax ? serde::Value{*tax} : serde::Value{};
            out["document_details"] = document_details_document(aggregate.extension);
        }
        // Upstream create/update responses restate these fields; overwrite in
        // place so field order is preserved and no duplicate keys are emitted.
        if (create) {
            if (create->description) {
                out["description"] = serde::Value{*create->description};
            }
            if (create->phone_country_code) {
                out["phone_country_code"] = serde::Value{*create->phone_country_code};
            }
            if (create->tax_registration_id) {
                out["tax_registration_id"] = serde::Value{*create->tax_registration_id};
            }
        }
        if (update) {
            if (update->description) {
                out["description"] = serde::Value{*update->description};
            }
            if (update->phone_country_code) {
                out["phone_country_code"] = serde::Value{*update->phone_country_code};
            }
            if (update->tax_registration_id) {
                out["tax_registration_id"] = serde::Value{*update->tax_registration_id};
            }
        }
        return serde::Value{std::move(out)};
    }

    // ───────────────────────── Time-range helpers ─────────────────────────

    [[nodiscard]] std::string trim(std::string_view value) const
    {
        std::size_t begin{0};
        std::size_t end{value.size()};
        while (begin < end && std::isspace(static_cast<unsigned char>(value[begin]))) {
            ++begin;
        }
        while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
            --end;
        }
        return std::string{value.substr(begin, end - begin)};
    }

    [[nodiscard]] std::optional<std::pair<std::string, std::string>>
    parse_time_range(const ::app::model::customers::CustomerTimeRange& range) const
    {
        if (range.get_start_time().empty()) {
            return std::nullopt;
        }
        return std::pair<std::string, std::string>{
            trim(range.get_start_time()),
            trim(range.get_end_time().value_or(""))
        };
    }

    [[nodiscard]] bool within_range(
        std::string_view created_at,
        const ::app::model::customers::CustomerTimeRange& range
    ) const
    {
        const auto bounds = parse_time_range(range);
        if (!bounds) {
            return true;
        }
        if (created_at < bounds->first) {
            return false;
        }
        if (!bounds->second.empty() && created_at > bounds->second) {
            return false;
        }
        return true;
    }

    // ───────────────────────── Migration row helpers ─────────────────────────

    [[nodiscard]] ::app::model::customers::CustomerGlobalIdMigrationRowResult migration_result(
        const MigrationRow& row,
        std::string status,
        std::optional<std::string> new_id,
        std::string message
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
    ) const
    {
        return row;
    }

    [[nodiscard]] ::app::model::customers::CustomerGlobalIdMigrationRowResult
    process_fetched_row(::app::store::AppRepository& repository, MigrationRow row) const
    {
        auto aggregate = find_by_reference(repository, row.customer_id, row.merchant_id);
        if (!aggregate) {
            return migration_result(
                row,
                "NotFound",
                std::nullopt,
                "Customer not found for merchant_id and customer_id"
            );
        }
        row.old_id = aggregate->extension.get_global_id().empty()
                         ? std::optional<std::string>{}
                         : std::optional<std::string>{aggregate->extension.get_global_id()};
        row.version = aggregate->extension.get_version();

        if (!row.version || *row.version != "v1") {
            return migration_result(
                row,
                "SkippedNonV1",
                std::nullopt,
                "Customer is not a v1 row; skipping migration"
            );
        }
        if (row.old_id && is_global_customer_id_format(*row.old_id)) {
            return migration_result(
                row,
                "AlreadyGlobalId",
                std::nullopt,
                "Customer id is already in global id format"
            );
        }
        const std::string status = row.old_id ? "UpdatedNonGlobalId" : "UpdatedNullId";
        const std::string new_id = generate_global_customer_id();
        aggregate->extension.set_global_id(new_id);
        aggregate->extension.set_version("v2");
        persist_update(repository, *aggregate);
        return migration_result(row, status, new_id, "Customer id updated successfully");
    }

    [[nodiscard]] serde::Value migration_row_document(
        const ::app::model::customers::CustomerGlobalIdMigrationRowResult& row
    ) const
    {
        serde::Value::Object out{};
        out["row_number"] = serde::Value{static_cast<std::int64_t>(row.get_row_number())};
        out["merchant_id"] =
            row.get_merchant_id() ? serde::Value{*row.get_merchant_id()} : serde::Value{};
        out["customer_id"] =
            row.get_customer_id() ? serde::Value{*row.get_customer_id()} : serde::Value{};
        out["status"] = serde::Value{row.get_status()};
        out["old_id"] = row.get_old_id() ? serde::Value{*row.get_old_id()} : serde::Value{};
        out["new_id"] = row.get_new_id() ? serde::Value{*row.get_new_id()} : serde::Value{};
        out["message"] = serde::Value{row.get_message()};
        return serde::Value{std::move(out)};
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::customers
