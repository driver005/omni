module;
export module app_model:customers;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::customers {

class AddressDetailsValue
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CustomerIdValue
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CustomerListLimitValue
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class DescriptionValue
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class DocumentKindValue
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class EmailValue
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MerchantIdValue
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class OptionalEncryptableEmailValue
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class OptionalEncryptableNameValue
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class OptionalEncryptablePhoneValue
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class OptionalEncryptableSecretStringValue
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PrimitiveDateTimeValue
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class SecretSerdeValueValue
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class TimeRangeValue
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CustomerDeleteResponse
{
public:
    void set_customer_id(CustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_customer_deleted(bool value)
    {
        customer_deleted = std::move(value);
    }

    [[nodiscard]] const bool& get_customer_deleted() const noexcept
    {
        return customer_deleted;
    }

    void set_address_deleted(bool value)
    {
        address_deleted = std::move(value);
    }

    [[nodiscard]] const bool& get_address_deleted() const noexcept
    {
        return address_deleted;
    }

    void set_payment_methods_deleted(bool value)
    {
        payment_methods_deleted = std::move(value);
    }

    [[nodiscard]] const bool& get_payment_methods_deleted() const noexcept
    {
        return payment_methods_deleted;
    }

    CustomerIdValue customer_id{};
    bool customer_deleted{};
    bool address_deleted{};
    bool payment_methods_deleted{};
};

class CustomerDocumentDetails
{
public:
    void set_document_type(DocumentKindValue value)
    {
        document_type = std::move(value);
    }

    [[nodiscard]] const DocumentKindValue& get_document_type() const noexcept
    {
        return document_type;
    }

    void set_document_number(std::string value)
    {
        document_number = std::move(value);
    }

    [[nodiscard]] const std::string& get_document_number() const noexcept
    {
        return document_number;
    }

    DocumentKindValue document_type{};
    std::string document_number{};
};

class CustomerListRequest
{
public:
    void set_offset(std::optional<std::uint32_t> value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_offset() const noexcept
    {
        return offset;
    }

    void set_limit(std::optional<std::uint16_t> value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint16_t>& get_limit() const noexcept
    {
        return limit;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    std::optional<std::uint32_t> offset{};
    std::optional<std::uint16_t> limit{};
    std::optional<CustomerIdValue> customer_id{};
};

class CustomerListRequestWithConstraints
{
public:
    void set_offset(std::optional<std::uint32_t> value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_offset() const noexcept
    {
        return offset;
    }

    void set_limit(std::optional<CustomerListLimitValue> value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerListLimitValue>& get_limit() const noexcept
    {
        return limit;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_time_range(std::optional<TimeRangeValue> value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const std::optional<TimeRangeValue>& get_time_range() const noexcept
    {
        return time_range;
    }

    std::optional<std::uint32_t> offset{};
    std::optional<CustomerListLimitValue> limit{};
    std::optional<CustomerIdValue> customer_id{};
    std::optional<TimeRangeValue> time_range{};
};

class CustomerRequest
{
public:
    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_name(std::optional<std::string> value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_name() const noexcept
    {
        return name;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    void set_phone(std::optional<std::string> value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone() const noexcept
    {
        return phone;
    }

    void set_description(std::optional<DescriptionValue> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<DescriptionValue>& get_description() const noexcept
    {
        return description;
    }

    void set_phone_country_code(std::optional<std::string> value)
    {
        phone_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone_country_code() const noexcept
    {
        return phone_country_code;
    }

    void set_address(std::optional<AddressDetailsValue> value)
    {
        address = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressDetailsValue>& get_address() const noexcept
    {
        return address;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_tax_registration_id(std::optional<std::string> value)
    {
        tax_registration_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_tax_registration_id() const noexcept
    {
        return tax_registration_id;
    }

    void set_document_details(std::optional<CustomerDocumentDetails> value)
    {
        document_details = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDocumentDetails>&
    get_document_details() const noexcept
    {
        return document_details;
    }

    std::optional<CustomerIdValue> customer_id{};
    MerchantIdValue merchant_id{};
    std::optional<std::string> name{};
    std::optional<EmailValue> email{};
    std::optional<std::string> phone{};
    std::optional<DescriptionValue> description{};
    std::optional<std::string> phone_country_code{};
    std::optional<AddressDetailsValue> address{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<std::string> tax_registration_id{};
    std::optional<CustomerDocumentDetails> document_details{};
};

class CustomerResponse
{
public:
    void set_customer_id(CustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_name(OptionalEncryptableNameValue value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const OptionalEncryptableNameValue& get_name() const noexcept
    {
        return name;
    }

    void set_email(OptionalEncryptableEmailValue value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const OptionalEncryptableEmailValue& get_email() const noexcept
    {
        return email;
    }

    void set_phone(OptionalEncryptablePhoneValue value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const OptionalEncryptablePhoneValue& get_phone() const noexcept
    {
        return phone;
    }

    void set_phone_country_code(std::optional<std::string> value)
    {
        phone_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone_country_code() const noexcept
    {
        return phone_country_code;
    }

    void set_description(std::optional<DescriptionValue> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<DescriptionValue>& get_description() const noexcept
    {
        return description;
    }

    void set_address(std::optional<AddressDetailsValue> value)
    {
        address = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressDetailsValue>& get_address() const noexcept
    {
        return address;
    }

    void set_created_at(PrimitiveDateTimeValue value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_default_payment_method_id(std::optional<std::string> value)
    {
        default_payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_default_payment_method_id() const noexcept
    {
        return default_payment_method_id;
    }

    void set_tax_registration_id(OptionalEncryptableSecretStringValue value)
    {
        tax_registration_id = std::move(value);
    }

    [[nodiscard]] const OptionalEncryptableSecretStringValue&
    get_tax_registration_id() const noexcept
    {
        return tax_registration_id;
    }

    void set_document_details(std::optional<CustomerDocumentDetails> value)
    {
        document_details = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDocumentDetails>&
    get_document_details() const noexcept
    {
        return document_details;
    }

    CustomerIdValue customer_id{};
    OptionalEncryptableNameValue name{};
    OptionalEncryptableEmailValue email{};
    OptionalEncryptablePhoneValue phone{};
    std::optional<std::string> phone_country_code{};
    std::optional<DescriptionValue> description{};
    std::optional<AddressDetailsValue> address{};
    PrimitiveDateTimeValue created_at{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<std::string> default_payment_method_id{};
    OptionalEncryptableSecretStringValue tax_registration_id{};
    std::optional<CustomerDocumentDetails> document_details{};
};

class CustomerUpdateRequest
{
public:
    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_name(std::optional<std::string> value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_name() const noexcept
    {
        return name;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    void set_phone(std::optional<std::string> value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone() const noexcept
    {
        return phone;
    }

    void set_description(std::optional<DescriptionValue> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<DescriptionValue>& get_description() const noexcept
    {
        return description;
    }

    void set_phone_country_code(std::optional<std::string> value)
    {
        phone_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone_country_code() const noexcept
    {
        return phone_country_code;
    }

    void set_address(std::optional<AddressDetailsValue> value)
    {
        address = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressDetailsValue>& get_address() const noexcept
    {
        return address;
    }

    void set_metadata(std::optional<SecretSerdeValueValue> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_tax_registration_id(std::optional<std::string> value)
    {
        tax_registration_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_tax_registration_id() const noexcept
    {
        return tax_registration_id;
    }

    void set_document_details(std::optional<CustomerDocumentDetails> value)
    {
        document_details = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDocumentDetails>&
    get_document_details() const noexcept
    {
        return document_details;
    }

    MerchantIdValue merchant_id{};
    std::optional<std::string> name{};
    std::optional<EmailValue> email{};
    std::optional<std::string> phone{};
    std::optional<DescriptionValue> description{};
    std::optional<std::string> phone_country_code{};
    std::optional<AddressDetailsValue> address{};
    std::optional<SecretSerdeValueValue> metadata{};
    std::optional<std::string> tax_registration_id{};
    std::optional<CustomerDocumentDetails> document_details{};
};

class CustomerListResponse
{
public:
    void set_data(std::vector<CustomerResponse> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::vector<CustomerResponse>& get_data() const noexcept
    {
        return data;
    }

    void set_total_count(std::size_t value)
    {
        total_count = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_total_count() const noexcept
    {
        return total_count;
    }

    std::vector<CustomerResponse> data{};
    std::size_t total_count{};
};

class CustomerUpdateRequestInternal
{
public:
    void set_customer_id(CustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_request(CustomerUpdateRequest value)
    {
        request = std::move(value);
    }

    [[nodiscard]] const CustomerUpdateRequest& get_request() const noexcept
    {
        return request;
    }

    CustomerIdValue customer_id{};
    CustomerUpdateRequest request{};
};

// ─────────────────────────────────────────────────────────────────────────────
// Upstream-faithful wire DTOs.
//
// These mirror `api_models::customers` (docs/hyperswitch/crates/api_models/src/customers.rs)
// and `api_models::payments::AddressDetails` (payments.rs:6837-6879). Scalars are plain
// strings so `rfl::json::write` emits upstream JSON shapes; the legacy `*Value` wrapper
// types above would emit `{"value": ...}` and are therefore not used on the wire.
// `serde::RawJson` is used for arbitrary JSON objects (metadata / connector ids).
// ─────────────────────────────────────────────────────────────────────────────

class CustomerDocumentDetailsWire
{
public:
    void set_document_type(std::string value)
    {
        document_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_document_type() const noexcept
    {
        return document_type;
    }

    void set_document_number(std::string value)
    {
        document_number = std::move(value);
    }

    [[nodiscard]] const std::string& get_document_number() const noexcept
    {
        return document_number;
    }

    std::string document_type{};
    std::string document_number{};
};

/** @brief Upstream `api_models::payments::AddressDetails` (payments.rs:6837-6879). */
class CustomerAddressDetails
{
public:
    void set_city(std::optional<std::string> value)
    {
        city = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_city() const noexcept
    {
        return city;
    }

    void set_country(std::optional<std::string> value)
    {
        country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_country() const noexcept
    {
        return country;
    }

    void set_line1(std::optional<std::string> value)
    {
        line1 = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_line1() const noexcept
    {
        return line1;
    }

    void set_line2(std::optional<std::string> value)
    {
        line2 = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_line2() const noexcept
    {
        return line2;
    }

    void set_line3(std::optional<std::string> value)
    {
        line3 = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_line3() const noexcept
    {
        return line3;
    }

    void set_zip(std::optional<std::string> value)
    {
        zip = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_zip() const noexcept
    {
        return zip;
    }

    void set_state(std::optional<std::string> value)
    {
        state = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_state() const noexcept
    {
        return state;
    }

    void set_first_name(std::optional<std::string> value)
    {
        first_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_first_name() const noexcept
    {
        return first_name;
    }

    void set_last_name(std::optional<std::string> value)
    {
        last_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_last_name() const noexcept
    {
        return last_name;
    }

    void set_origin_zip(std::optional<std::string> value)
    {
        origin_zip = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_origin_zip() const noexcept
    {
        return origin_zip;
    }

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

/** @brief Upstream `common_utils::types::TimeRange` (types.rs:813-822); query keys are
 *  `start_time`/`end_time` with `startTime`/`endTime` aliases. */
class CustomerTimeRange
{
public:
    void set_start_time(std::string value)
    {
        start_time = std::move(value);
    }

    [[nodiscard]] const std::string& get_start_time() const noexcept
    {
        return start_time;
    }

    void set_end_time(std::optional<std::string> value)
    {
        end_time = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_end_time() const noexcept
    {
        return end_time;
    }

    std::string start_time{};
    std::optional<std::string> end_time{};
};

/** @brief Upstream `api_models::customers::CustomerRequest` (customers.rs:23-70 / 129-165). */
class CustomerCreateRequestWire
{
public:
    void set_customer_id(std::optional<std::string> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_merchant_reference_id(std::optional<std::string> value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    void set_name(std::optional<std::string> value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_name() const noexcept
    {
        return name;
    }

    void set_email(std::optional<std::string> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_email() const noexcept
    {
        return email;
    }

    void set_phone(std::optional<std::string> value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone() const noexcept
    {
        return phone;
    }

    void set_description(std::optional<std::string> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept
    {
        return description;
    }

    void set_phone_country_code(std::optional<std::string> value)
    {
        phone_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone_country_code() const noexcept
    {
        return phone_country_code;
    }

    void set_address(std::optional<CustomerAddressDetails> value)
    {
        address = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerAddressDetails>& get_address() const noexcept
    {
        return address;
    }

    void set_default_billing_address(std::optional<CustomerAddressDetails> value)
    {
        default_billing_address = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerAddressDetails>&
    get_default_billing_address() const noexcept
    {
        return default_billing_address;
    }

    void set_default_shipping_address(std::optional<CustomerAddressDetails> value)
    {
        default_shipping_address = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerAddressDetails>&
    get_default_shipping_address() const noexcept
    {
        return default_shipping_address;
    }

    void set_metadata(std::optional<serde::RawJson> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::RawJson>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_tax_registration_id(std::optional<std::string> value)
    {
        tax_registration_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_tax_registration_id() const noexcept
    {
        return tax_registration_id;
    }

    void set_document_details(std::optional<CustomerDocumentDetailsWire> value)
    {
        document_details = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDocumentDetailsWire>&
    get_document_details() const noexcept
    {
        return document_details;
    }

    void set_default_payment_method_id(std::optional<std::string> value)
    {
        default_payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_default_payment_method_id() const noexcept
    {
        return default_payment_method_id;
    }

    void set_merchant_id(std::optional<std::string> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    std::optional<std::string> customer_id{};
    std::optional<std::string> merchant_reference_id{};
    std::optional<std::string> name{};
    std::optional<std::string> email{};
    std::optional<std::string> phone{};
    std::optional<std::string> description{};
    std::optional<std::string> phone_country_code{};
    std::optional<CustomerAddressDetails> address{};
    std::optional<CustomerAddressDetails> default_billing_address{};
    std::optional<CustomerAddressDetails> default_shipping_address{};
    std::optional<serde::RawJson> metadata{};
    std::optional<std::string> tax_registration_id{};
    std::optional<CustomerDocumentDetailsWire> document_details{};
    std::optional<std::string> default_payment_method_id{};
    std::optional<std::string> merchant_id{};
};

/** @brief Upstream `api_models::customers::CustomerUpdateRequest` (customers.rs:366-456). */
class CustomerUpdateRequestWire
{
public:
    void set_name(std::optional<std::string> value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_name() const noexcept
    {
        return name;
    }

    void set_email(std::optional<std::string> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_email() const noexcept
    {
        return email;
    }

    void set_phone(std::optional<std::string> value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone() const noexcept
    {
        return phone;
    }

    void set_description(std::optional<std::string> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept
    {
        return description;
    }

    void set_phone_country_code(std::optional<std::string> value)
    {
        phone_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone_country_code() const noexcept
    {
        return phone_country_code;
    }

    void set_address(std::optional<CustomerAddressDetails> value)
    {
        address = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerAddressDetails>& get_address() const noexcept
    {
        return address;
    }

    void set_default_billing_address(std::optional<CustomerAddressDetails> value)
    {
        default_billing_address = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerAddressDetails>&
    get_default_billing_address() const noexcept
    {
        return default_billing_address;
    }

    void set_default_shipping_address(std::optional<CustomerAddressDetails> value)
    {
        default_shipping_address = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerAddressDetails>&
    get_default_shipping_address() const noexcept
    {
        return default_shipping_address;
    }

    void set_metadata(std::optional<serde::RawJson> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::RawJson>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_tax_registration_id(std::optional<std::string> value)
    {
        tax_registration_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_tax_registration_id() const noexcept
    {
        return tax_registration_id;
    }

    void set_document_details(std::optional<CustomerDocumentDetailsWire> value)
    {
        document_details = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDocumentDetailsWire>&
    get_document_details() const noexcept
    {
        return document_details;
    }

    void set_default_payment_method_id(std::optional<std::string> value)
    {
        default_payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_default_payment_method_id() const noexcept
    {
        return default_payment_method_id;
    }

    void set_merchant_id(std::optional<std::string> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    std::optional<std::string> name{};
    std::optional<std::string> email{};
    std::optional<std::string> phone{};
    std::optional<std::string> description{};
    std::optional<std::string> phone_country_code{};
    std::optional<CustomerAddressDetails> address{};
    std::optional<CustomerAddressDetails> default_billing_address{};
    std::optional<CustomerAddressDetails> default_shipping_address{};
    std::optional<serde::RawJson> metadata{};
    std::optional<std::string> tax_registration_id{};
    std::optional<CustomerDocumentDetailsWire> document_details{};
    std::optional<std::string> default_payment_method_id{};
    std::optional<std::string> merchant_id{};
};

/** @brief Upstream `customers::CustomerListRequest` (customers.rs:74-84). */
class CustomerListRequestWire
{
public:
    void set_offset(std::optional<std::uint32_t> value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_offset() const noexcept
    {
        return offset;
    }

    void set_limit(std::optional<std::uint16_t> value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint16_t>& get_limit() const noexcept
    {
        return limit;
    }

    void set_customer_id(std::optional<std::string> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    std::optional<std::uint32_t> offset{};
    std::optional<std::uint16_t> limit{};
    std::optional<std::string> customer_id{};
};

/** @brief Upstream `customers::CustomerListRequestWithConstraints` (customers.rs:87-99). */
class CustomerListConstraintsRequestWire
{
public:
    void set_offset(std::optional<std::uint32_t> value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_offset() const noexcept
    {
        return offset;
    }

    void set_limit(std::optional<std::string> value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_limit() const noexcept
    {
        return limit;
    }

    void set_customer_id(std::optional<std::string> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_time_range(std::optional<CustomerTimeRange> value)
    {
        time_range = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerTimeRange>& get_time_range() const noexcept
    {
        return time_range;
    }

    std::optional<std::uint32_t> offset{};
    std::optional<std::string> limit{};
    std::optional<std::string> customer_id{};
    std::optional<CustomerTimeRange> time_range{};
};

/** @brief Upstream v1 `customers::CustomerResponse` (customers.rs:189-241). */
class CustomerResponseV1Wire
{
public:
    void set_customer_id(std::string value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_name(std::optional<std::string> value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_name() const noexcept
    {
        return name;
    }

    void set_email(std::optional<std::string> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_email() const noexcept
    {
        return email;
    }

    void set_phone(std::optional<std::string> value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone() const noexcept
    {
        return phone;
    }

    void set_phone_country_code(std::optional<std::string> value)
    {
        phone_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone_country_code() const noexcept
    {
        return phone_country_code;
    }

    void set_description(std::optional<std::string> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept
    {
        return description;
    }

    void set_address(std::optional<CustomerAddressDetails> value)
    {
        address = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerAddressDetails>& get_address() const noexcept
    {
        return address;
    }

    void set_created_at(std::string value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_metadata(std::optional<serde::RawJson> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::RawJson>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_default_payment_method_id(std::optional<std::string> value)
    {
        default_payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_default_payment_method_id() const noexcept
    {
        return default_payment_method_id;
    }

    void set_tax_registration_id(std::optional<std::string> value)
    {
        tax_registration_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_tax_registration_id() const noexcept
    {
        return tax_registration_id;
    }

    void set_document_details(std::optional<CustomerDocumentDetailsWire> value)
    {
        document_details = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDocumentDetailsWire>&
    get_document_details() const noexcept
    {
        return document_details;
    }

    std::string customer_id{};
    std::optional<std::string> name{};
    std::optional<std::string> email{};
    std::optional<std::string> phone{};
    std::optional<std::string> phone_country_code{};
    std::optional<std::string> description{};
    std::optional<CustomerAddressDetails> address{};
    std::string created_at{};
    std::optional<serde::RawJson> metadata{};
    std::optional<std::string> default_payment_method_id{};
    std::optional<std::string> tax_registration_id{};
    std::optional<CustomerDocumentDetailsWire> document_details{};
};

/** @brief Upstream v2 `customers::CustomerResponse` (customers.rs:252-306). */
class CustomerResponseV2Wire
{
public:
    void set_id(std::string value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return id;
    }

    void set_merchant_reference_id(std::optional<std::string> value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    void set_connector_customer_ids(std::optional<serde::RawJson> value)
    {
        connector_customer_ids = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::RawJson>& get_connector_customer_ids() const noexcept
    {
        return connector_customer_ids;
    }

    void set_name(std::optional<std::string> value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_name() const noexcept
    {
        return name;
    }

    void set_email(std::optional<std::string> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_email() const noexcept
    {
        return email;
    }

    void set_phone(std::optional<std::string> value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone() const noexcept
    {
        return phone;
    }

    void set_phone_country_code(std::optional<std::string> value)
    {
        phone_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_phone_country_code() const noexcept
    {
        return phone_country_code;
    }

    void set_description(std::optional<std::string> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept
    {
        return description;
    }

    void set_default_billing_address(std::optional<CustomerAddressDetails> value)
    {
        default_billing_address = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerAddressDetails>&
    get_default_billing_address() const noexcept
    {
        return default_billing_address;
    }

    void set_default_shipping_address(std::optional<CustomerAddressDetails> value)
    {
        default_shipping_address = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerAddressDetails>&
    get_default_shipping_address() const noexcept
    {
        return default_shipping_address;
    }

    void set_created_at(std::string value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_metadata(std::optional<serde::RawJson> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::RawJson>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_default_payment_method_id(std::optional<std::string> value)
    {
        default_payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_default_payment_method_id() const noexcept
    {
        return default_payment_method_id;
    }

    void set_tax_registration_id(std::optional<std::string> value)
    {
        tax_registration_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_tax_registration_id() const noexcept
    {
        return tax_registration_id;
    }

    void set_document_details(std::optional<CustomerDocumentDetailsWire> value)
    {
        document_details = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerDocumentDetailsWire>&
    get_document_details() const noexcept
    {
        return document_details;
    }

    std::string id{};
    std::optional<std::string> merchant_reference_id{};
    std::optional<serde::RawJson> connector_customer_ids{};
    std::optional<std::string> name{};
    std::optional<std::string> email{};
    std::optional<std::string> phone{};
    std::optional<std::string> phone_country_code{};
    std::optional<std::string> description{};
    std::optional<CustomerAddressDetails> default_billing_address{};
    std::optional<CustomerAddressDetails> default_shipping_address{};
    std::string created_at{};
    std::optional<serde::RawJson> metadata{};
    std::optional<std::string> default_payment_method_id{};
    std::optional<std::string> tax_registration_id{};
    std::optional<CustomerDocumentDetailsWire> document_details{};
};

/** @brief Upstream v1 `CustomerDeleteResponse` (customers.rs:318-335). */
class CustomerDeleteResponseV1Wire
{
public:
    void set_customer_id(std::string value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_customer_deleted(bool value)
    {
        customer_deleted = value;
    }

    [[nodiscard]] bool get_customer_deleted() const noexcept
    {
        return customer_deleted;
    }

    void set_address_deleted(bool value)
    {
        address_deleted = value;
    }

    [[nodiscard]] bool get_address_deleted() const noexcept
    {
        return address_deleted;
    }

    void set_payment_methods_deleted(bool value)
    {
        payment_methods_deleted = value;
    }

    [[nodiscard]] bool get_payment_methods_deleted() const noexcept
    {
        return payment_methods_deleted;
    }

    std::string customer_id{};
    bool customer_deleted{};
    bool address_deleted{};
    bool payment_methods_deleted{};
};

/** @brief Upstream v2 `CustomerDeleteResponse` (customers.rs:339-360). */
class CustomerDeleteResponseV2Wire
{
public:
    void set_id(std::string value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return id;
    }

    void set_merchant_reference_id(std::optional<std::string> value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    void set_customer_deleted(bool value)
    {
        customer_deleted = value;
    }

    [[nodiscard]] bool get_customer_deleted() const noexcept
    {
        return customer_deleted;
    }

    void set_address_deleted(bool value)
    {
        address_deleted = value;
    }

    [[nodiscard]] bool get_address_deleted() const noexcept
    {
        return address_deleted;
    }

    void set_payment_methods_deleted(bool value)
    {
        payment_methods_deleted = value;
    }

    [[nodiscard]] bool get_payment_methods_deleted() const noexcept
    {
        return payment_methods_deleted;
    }

    std::string id{};
    std::optional<std::string> merchant_reference_id{};
    bool customer_deleted{};
    bool address_deleted{};
    bool payment_methods_deleted{};
};

class CustomerListResponseV1Wire
{
public:
    void set_data(std::vector<CustomerResponseV1Wire> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::vector<CustomerResponseV1Wire>& get_data() const noexcept
    {
        return data;
    }

    void set_total_count(std::size_t value)
    {
        total_count = value;
    }

    [[nodiscard]] std::size_t get_total_count() const noexcept
    {
        return total_count;
    }

    std::vector<CustomerResponseV1Wire> data{};
    std::size_t total_count{};
};

class CustomerListResponseV2Wire
{
public:
    void set_data(std::vector<CustomerResponseV2Wire> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::vector<CustomerResponseV2Wire>& get_data() const noexcept
    {
        return data;
    }

    void set_total_count(std::size_t value)
    {
        total_count = value;
    }

    [[nodiscard]] std::size_t get_total_count() const noexcept
    {
        return total_count;
    }

    std::vector<CustomerResponseV2Wire> data{};
    std::size_t total_count{};
};

/** @brief Upstream `api_models::customers::migrate` DTOs (migrate.rs:5-37). */
class CustomerGlobalIdMigrationRowResult
{
public:
    void set_row_number(std::size_t value)
    {
        row_number = value;
    }

    [[nodiscard]] std::size_t get_row_number() const noexcept
    {
        return row_number;
    }

    void set_merchant_id(std::optional<std::string> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_customer_id(std::optional<std::string> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_status(std::string value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return status;
    }

    void set_old_id(std::optional<std::string> value)
    {
        old_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_old_id() const noexcept
    {
        return old_id;
    }

    void set_new_id(std::optional<std::string> value)
    {
        new_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_new_id() const noexcept
    {
        return new_id;
    }

    void set_message(std::string value)
    {
        message = std::move(value);
    }

    [[nodiscard]] const std::string& get_message() const noexcept
    {
        return message;
    }

    std::size_t row_number{};
    std::optional<std::string> merchant_id{};
    std::optional<std::string> customer_id{};
    std::string status{};
    std::optional<std::string> old_id{};
    std::optional<std::string> new_id{};
    std::string message{};
};

class CustomerGlobalIdMigrationResponse
{
public:
    void set_total_rows(std::size_t value)
    {
        total_rows = value;
    }

    [[nodiscard]] std::size_t get_total_rows() const noexcept
    {
        return total_rows;
    }

    void set_updated_count(std::size_t value)
    {
        updated_count = value;
    }

    [[nodiscard]] std::size_t get_updated_count() const noexcept
    {
        return updated_count;
    }

    void set_skipped_count(std::size_t value)
    {
        skipped_count = value;
    }

    [[nodiscard]] std::size_t get_skipped_count() const noexcept
    {
        return skipped_count;
    }

    void set_failed_count(std::size_t value)
    {
        failed_count = value;
    }

    [[nodiscard]] std::size_t get_failed_count() const noexcept
    {
        return failed_count;
    }

    void set_results(std::vector<CustomerGlobalIdMigrationRowResult> value)
    {
        results = std::move(value);
    }

    [[nodiscard]] const std::vector<CustomerGlobalIdMigrationRowResult>&
    get_results() const noexcept
    {
        return results;
    }

    std::size_t total_rows{};
    std::size_t updated_count{};
    std::size_t skipped_count{};
    std::size_t failed_count{};
    std::vector<CustomerGlobalIdMigrationRowResult> results{};
};

} // namespace app::model::customers

template<>
struct serde::Serializable<app::model::customers::CustomerDocumentDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "document_type",
                &app::model::customers::CustomerDocumentDetails::get_document_type,
                &app::model::customers::CustomerDocumentDetails::set_document_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "document_number",
                &app::model::customers::CustomerDocumentDetails::get_document_number,
                &app::model::customers::CustomerDocumentDetails::set_document_number,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_document_details";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "customer_id",
                &app::model::customers::CustomerRequest::get_customer_id,
                &app::model::customers::CustomerRequest::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::customers::CustomerRequest::get_merchant_id,
                &app::model::customers::CustomerRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::model::customers::CustomerRequest::get_name,
                &app::model::customers::CustomerRequest::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::customers::CustomerRequest::get_email,
                &app::model::customers::CustomerRequest::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::model::customers::CustomerRequest::get_phone,
                &app::model::customers::CustomerRequest::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::customers::CustomerRequest::get_description,
                &app::model::customers::CustomerRequest::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone_country_code",
                &app::model::customers::CustomerRequest::get_phone_country_code,
                &app::model::customers::CustomerRequest::set_phone_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "address",
                &app::model::customers::CustomerRequest::get_address,
                &app::model::customers::CustomerRequest::set_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::customers::CustomerRequest::get_metadata,
                &app::model::customers::CustomerRequest::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_registration_id",
                &app::model::customers::CustomerRequest::get_tax_registration_id,
                &app::model::customers::CustomerRequest::set_tax_registration_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "document_details",
                &app::model::customers::CustomerRequest::get_document_details,
                &app::model::customers::CustomerRequest::set_document_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_request";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerListRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "offset",
                &app::model::customers::CustomerListRequest::get_offset,
                &app::model::customers::CustomerListRequest::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::customers::CustomerListRequest::get_limit,
                &app::model::customers::CustomerListRequest::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::customers::CustomerListRequest::get_customer_id,
                &app::model::customers::CustomerListRequest::set_customer_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_list_request";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerListRequestWithConstraints>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "offset",
                &app::model::customers::CustomerListRequestWithConstraints::get_offset,
                &app::model::customers::CustomerListRequestWithConstraints::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::customers::CustomerListRequestWithConstraints::get_limit,
                &app::model::customers::CustomerListRequestWithConstraints::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::customers::CustomerListRequestWithConstraints::get_customer_id,
                &app::model::customers::CustomerListRequestWithConstraints::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "time_range",
                &app::model::customers::CustomerListRequestWithConstraints::get_time_range,
                &app::model::customers::CustomerListRequestWithConstraints::set_time_range,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_list_request_with_constraints";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "customer_id",
                &app::model::customers::CustomerResponse::get_customer_id,
                &app::model::customers::CustomerResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::model::customers::CustomerResponse::get_name,
                &app::model::customers::CustomerResponse::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::customers::CustomerResponse::get_email,
                &app::model::customers::CustomerResponse::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::model::customers::CustomerResponse::get_phone,
                &app::model::customers::CustomerResponse::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone_country_code",
                &app::model::customers::CustomerResponse::get_phone_country_code,
                &app::model::customers::CustomerResponse::set_phone_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::customers::CustomerResponse::get_description,
                &app::model::customers::CustomerResponse::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "address",
                &app::model::customers::CustomerResponse::get_address,
                &app::model::customers::CustomerResponse::set_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::model::customers::CustomerResponse::get_created_at,
                &app::model::customers::CustomerResponse::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::customers::CustomerResponse::get_metadata,
                &app::model::customers::CustomerResponse::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "default_payment_method_id",
                &app::model::customers::CustomerResponse::get_default_payment_method_id,
                &app::model::customers::CustomerResponse::set_default_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_registration_id",
                &app::model::customers::CustomerResponse::get_tax_registration_id,
                &app::model::customers::CustomerResponse::set_tax_registration_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "document_details",
                &app::model::customers::CustomerResponse::get_document_details,
                &app::model::customers::CustomerResponse::set_document_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_response";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerDeleteResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "customer_id",
                &app::model::customers::CustomerDeleteResponse::get_customer_id,
                &app::model::customers::CustomerDeleteResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_deleted",
                &app::model::customers::CustomerDeleteResponse::get_customer_deleted,
                &app::model::customers::CustomerDeleteResponse::set_customer_deleted,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "address_deleted",
                &app::model::customers::CustomerDeleteResponse::get_address_deleted,
                &app::model::customers::CustomerDeleteResponse::set_address_deleted,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_methods_deleted",
                &app::model::customers::CustomerDeleteResponse::get_payment_methods_deleted,
                &app::model::customers::CustomerDeleteResponse::set_payment_methods_deleted,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_delete_response";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerUpdateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::customers::CustomerUpdateRequest::get_merchant_id,
                &app::model::customers::CustomerUpdateRequest::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::model::customers::CustomerUpdateRequest::get_name,
                &app::model::customers::CustomerUpdateRequest::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::customers::CustomerUpdateRequest::get_email,
                &app::model::customers::CustomerUpdateRequest::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::model::customers::CustomerUpdateRequest::get_phone,
                &app::model::customers::CustomerUpdateRequest::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::customers::CustomerUpdateRequest::get_description,
                &app::model::customers::CustomerUpdateRequest::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone_country_code",
                &app::model::customers::CustomerUpdateRequest::get_phone_country_code,
                &app::model::customers::CustomerUpdateRequest::set_phone_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "address",
                &app::model::customers::CustomerUpdateRequest::get_address,
                &app::model::customers::CustomerUpdateRequest::set_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::customers::CustomerUpdateRequest::get_metadata,
                &app::model::customers::CustomerUpdateRequest::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_registration_id",
                &app::model::customers::CustomerUpdateRequest::get_tax_registration_id,
                &app::model::customers::CustomerUpdateRequest::set_tax_registration_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "document_details",
                &app::model::customers::CustomerUpdateRequest::get_document_details,
                &app::model::customers::CustomerUpdateRequest::set_document_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_update_request";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerUpdateRequestInternal>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "customer_id",
                &app::model::customers::CustomerUpdateRequestInternal::get_customer_id,
                &app::model::customers::CustomerUpdateRequestInternal::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "request",
                &app::model::customers::CustomerUpdateRequestInternal::get_request,
                &app::model::customers::CustomerUpdateRequestInternal::set_request,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_update_request_internal";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "data",
                &app::model::customers::CustomerListResponse::get_data,
                &app::model::customers::CustomerListResponse::set_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_count",
                &app::model::customers::CustomerListResponse::get_total_count,
                &app::model::customers::CustomerListResponse::set_total_count,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_list_response";
    }
};

// ── serde for the upstream-faithful wire DTOs ──────────────────────────────────

template<>
struct serde::Serializable<app::model::customers::CustomerDocumentDetailsWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "document_type",
                &app::model::customers::CustomerDocumentDetailsWire::get_document_type,
                &app::model::customers::CustomerDocumentDetailsWire::set_document_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "document_number",
                &app::model::customers::CustomerDocumentDetailsWire::get_document_number,
                &app::model::customers::CustomerDocumentDetailsWire::set_document_number,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_document_details_wire";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerAddressDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "city",
                &app::model::customers::CustomerAddressDetails::get_city,
                &app::model::customers::CustomerAddressDetails::set_city,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "country",
                &app::model::customers::CustomerAddressDetails::get_country,
                &app::model::customers::CustomerAddressDetails::set_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "line1",
                &app::model::customers::CustomerAddressDetails::get_line1,
                &app::model::customers::CustomerAddressDetails::set_line1,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "line2",
                &app::model::customers::CustomerAddressDetails::get_line2,
                &app::model::customers::CustomerAddressDetails::set_line2,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "line3",
                &app::model::customers::CustomerAddressDetails::get_line3,
                &app::model::customers::CustomerAddressDetails::set_line3,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "zip",
                &app::model::customers::CustomerAddressDetails::get_zip,
                &app::model::customers::CustomerAddressDetails::set_zip,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "state",
                &app::model::customers::CustomerAddressDetails::get_state,
                &app::model::customers::CustomerAddressDetails::set_state,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "first_name",
                &app::model::customers::CustomerAddressDetails::get_first_name,
                &app::model::customers::CustomerAddressDetails::set_first_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_name",
                &app::model::customers::CustomerAddressDetails::get_last_name,
                &app::model::customers::CustomerAddressDetails::set_last_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "origin_zip",
                &app::model::customers::CustomerAddressDetails::get_origin_zip,
                &app::model::customers::CustomerAddressDetails::set_origin_zip,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_address_details";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerTimeRange>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "start_time",
                &app::model::customers::CustomerTimeRange::get_start_time,
                &app::model::customers::CustomerTimeRange::set_start_time,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "end_time",
                &app::model::customers::CustomerTimeRange::get_end_time,
                &app::model::customers::CustomerTimeRange::set_end_time,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_time_range";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerCreateRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "customer_id",
                &app::model::customers::CustomerCreateRequestWire::get_customer_id,
                &app::model::customers::CustomerCreateRequestWire::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::customers::CustomerCreateRequestWire::get_merchant_reference_id,
                &app::model::customers::CustomerCreateRequestWire::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::model::customers::CustomerCreateRequestWire::get_name,
                &app::model::customers::CustomerCreateRequestWire::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::customers::CustomerCreateRequestWire::get_email,
                &app::model::customers::CustomerCreateRequestWire::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::model::customers::CustomerCreateRequestWire::get_phone,
                &app::model::customers::CustomerCreateRequestWire::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::customers::CustomerCreateRequestWire::get_description,
                &app::model::customers::CustomerCreateRequestWire::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone_country_code",
                &app::model::customers::CustomerCreateRequestWire::get_phone_country_code,
                &app::model::customers::CustomerCreateRequestWire::set_phone_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "address",
                &app::model::customers::CustomerCreateRequestWire::get_address,
                &app::model::customers::CustomerCreateRequestWire::set_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "default_billing_address",
                &app::model::customers::CustomerCreateRequestWire::get_default_billing_address,
                &app::model::customers::CustomerCreateRequestWire::set_default_billing_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "default_shipping_address",
                &app::model::customers::CustomerCreateRequestWire::get_default_shipping_address,
                &app::model::customers::CustomerCreateRequestWire::set_default_shipping_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::customers::CustomerCreateRequestWire::get_metadata,
                &app::model::customers::CustomerCreateRequestWire::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_registration_id",
                &app::model::customers::CustomerCreateRequestWire::get_tax_registration_id,
                &app::model::customers::CustomerCreateRequestWire::set_tax_registration_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "document_details",
                &app::model::customers::CustomerCreateRequestWire::get_document_details,
                &app::model::customers::CustomerCreateRequestWire::set_document_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "default_payment_method_id",
                &app::model::customers::CustomerCreateRequestWire::get_default_payment_method_id,
                &app::model::customers::CustomerCreateRequestWire::set_default_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::customers::CustomerCreateRequestWire::get_merchant_id,
                &app::model::customers::CustomerCreateRequestWire::set_merchant_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_create_request_wire";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerUpdateRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &app::model::customers::CustomerUpdateRequestWire::get_name,
                &app::model::customers::CustomerUpdateRequestWire::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::customers::CustomerUpdateRequestWire::get_email,
                &app::model::customers::CustomerUpdateRequestWire::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::model::customers::CustomerUpdateRequestWire::get_phone,
                &app::model::customers::CustomerUpdateRequestWire::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::customers::CustomerUpdateRequestWire::get_description,
                &app::model::customers::CustomerUpdateRequestWire::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone_country_code",
                &app::model::customers::CustomerUpdateRequestWire::get_phone_country_code,
                &app::model::customers::CustomerUpdateRequestWire::set_phone_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "address",
                &app::model::customers::CustomerUpdateRequestWire::get_address,
                &app::model::customers::CustomerUpdateRequestWire::set_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "default_billing_address",
                &app::model::customers::CustomerUpdateRequestWire::get_default_billing_address,
                &app::model::customers::CustomerUpdateRequestWire::set_default_billing_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "default_shipping_address",
                &app::model::customers::CustomerUpdateRequestWire::get_default_shipping_address,
                &app::model::customers::CustomerUpdateRequestWire::set_default_shipping_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::customers::CustomerUpdateRequestWire::get_metadata,
                &app::model::customers::CustomerUpdateRequestWire::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_registration_id",
                &app::model::customers::CustomerUpdateRequestWire::get_tax_registration_id,
                &app::model::customers::CustomerUpdateRequestWire::set_tax_registration_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "document_details",
                &app::model::customers::CustomerUpdateRequestWire::get_document_details,
                &app::model::customers::CustomerUpdateRequestWire::set_document_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "default_payment_method_id",
                &app::model::customers::CustomerUpdateRequestWire::get_default_payment_method_id,
                &app::model::customers::CustomerUpdateRequestWire::set_default_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::customers::CustomerUpdateRequestWire::get_merchant_id,
                &app::model::customers::CustomerUpdateRequestWire::set_merchant_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_update_request_wire";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerListRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "offset",
                &app::model::customers::CustomerListRequestWire::get_offset,
                &app::model::customers::CustomerListRequestWire::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::customers::CustomerListRequestWire::get_limit,
                &app::model::customers::CustomerListRequestWire::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::customers::CustomerListRequestWire::get_customer_id,
                &app::model::customers::CustomerListRequestWire::set_customer_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_list_request_wire";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerListConstraintsRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "offset",
                &app::model::customers::CustomerListConstraintsRequestWire::get_offset,
                &app::model::customers::CustomerListConstraintsRequestWire::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::customers::CustomerListConstraintsRequestWire::get_limit,
                &app::model::customers::CustomerListConstraintsRequestWire::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::customers::CustomerListConstraintsRequestWire::get_customer_id,
                &app::model::customers::CustomerListConstraintsRequestWire::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "time_range",
                &app::model::customers::CustomerListConstraintsRequestWire::get_time_range,
                &app::model::customers::CustomerListConstraintsRequestWire::set_time_range,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_list_constraints_request_wire";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerResponseV1Wire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "customer_id",
                &app::model::customers::CustomerResponseV1Wire::get_customer_id,
                &app::model::customers::CustomerResponseV1Wire::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::model::customers::CustomerResponseV1Wire::get_name,
                &app::model::customers::CustomerResponseV1Wire::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::customers::CustomerResponseV1Wire::get_email,
                &app::model::customers::CustomerResponseV1Wire::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::model::customers::CustomerResponseV1Wire::get_phone,
                &app::model::customers::CustomerResponseV1Wire::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone_country_code",
                &app::model::customers::CustomerResponseV1Wire::get_phone_country_code,
                &app::model::customers::CustomerResponseV1Wire::set_phone_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::customers::CustomerResponseV1Wire::get_description,
                &app::model::customers::CustomerResponseV1Wire::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "address",
                &app::model::customers::CustomerResponseV1Wire::get_address,
                &app::model::customers::CustomerResponseV1Wire::set_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::model::customers::CustomerResponseV1Wire::get_created_at,
                &app::model::customers::CustomerResponseV1Wire::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::customers::CustomerResponseV1Wire::get_metadata,
                &app::model::customers::CustomerResponseV1Wire::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "default_payment_method_id",
                &app::model::customers::CustomerResponseV1Wire::get_default_payment_method_id,
                &app::model::customers::CustomerResponseV1Wire::set_default_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_registration_id",
                &app::model::customers::CustomerResponseV1Wire::get_tax_registration_id,
                &app::model::customers::CustomerResponseV1Wire::set_tax_registration_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "document_details",
                &app::model::customers::CustomerResponseV1Wire::get_document_details,
                &app::model::customers::CustomerResponseV1Wire::set_document_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_response_v1_wire";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerResponseV2Wire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::customers::CustomerResponseV2Wire::get_id,
                &app::model::customers::CustomerResponseV2Wire::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::customers::CustomerResponseV2Wire::get_merchant_reference_id,
                &app::model::customers::CustomerResponseV2Wire::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_customer_ids",
                &app::model::customers::CustomerResponseV2Wire::get_connector_customer_ids,
                &app::model::customers::CustomerResponseV2Wire::set_connector_customer_ids,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::model::customers::CustomerResponseV2Wire::get_name,
                &app::model::customers::CustomerResponseV2Wire::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::model::customers::CustomerResponseV2Wire::get_email,
                &app::model::customers::CustomerResponseV2Wire::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::model::customers::CustomerResponseV2Wire::get_phone,
                &app::model::customers::CustomerResponseV2Wire::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone_country_code",
                &app::model::customers::CustomerResponseV2Wire::get_phone_country_code,
                &app::model::customers::CustomerResponseV2Wire::set_phone_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::customers::CustomerResponseV2Wire::get_description,
                &app::model::customers::CustomerResponseV2Wire::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "default_billing_address",
                &app::model::customers::CustomerResponseV2Wire::get_default_billing_address,
                &app::model::customers::CustomerResponseV2Wire::set_default_billing_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "default_shipping_address",
                &app::model::customers::CustomerResponseV2Wire::get_default_shipping_address,
                &app::model::customers::CustomerResponseV2Wire::set_default_shipping_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::model::customers::CustomerResponseV2Wire::get_created_at,
                &app::model::customers::CustomerResponseV2Wire::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::customers::CustomerResponseV2Wire::get_metadata,
                &app::model::customers::CustomerResponseV2Wire::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "default_payment_method_id",
                &app::model::customers::CustomerResponseV2Wire::get_default_payment_method_id,
                &app::model::customers::CustomerResponseV2Wire::set_default_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_registration_id",
                &app::model::customers::CustomerResponseV2Wire::get_tax_registration_id,
                &app::model::customers::CustomerResponseV2Wire::set_tax_registration_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "document_details",
                &app::model::customers::CustomerResponseV2Wire::get_document_details,
                &app::model::customers::CustomerResponseV2Wire::set_document_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_response_v2_wire";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerDeleteResponseV1Wire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "customer_id",
                &app::model::customers::CustomerDeleteResponseV1Wire::get_customer_id,
                &app::model::customers::CustomerDeleteResponseV1Wire::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_deleted",
                &app::model::customers::CustomerDeleteResponseV1Wire::get_customer_deleted,
                &app::model::customers::CustomerDeleteResponseV1Wire::set_customer_deleted,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "address_deleted",
                &app::model::customers::CustomerDeleteResponseV1Wire::get_address_deleted,
                &app::model::customers::CustomerDeleteResponseV1Wire::set_address_deleted,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_methods_deleted",
                &app::model::customers::CustomerDeleteResponseV1Wire::get_payment_methods_deleted,
                &app::model::customers::CustomerDeleteResponseV1Wire::set_payment_methods_deleted,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_delete_response_v1_wire";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerDeleteResponseV2Wire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::customers::CustomerDeleteResponseV2Wire::get_id,
                &app::model::customers::CustomerDeleteResponseV2Wire::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::customers::CustomerDeleteResponseV2Wire::get_merchant_reference_id,
                &app::model::customers::CustomerDeleteResponseV2Wire::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_deleted",
                &app::model::customers::CustomerDeleteResponseV2Wire::get_customer_deleted,
                &app::model::customers::CustomerDeleteResponseV2Wire::set_customer_deleted,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "address_deleted",
                &app::model::customers::CustomerDeleteResponseV2Wire::get_address_deleted,
                &app::model::customers::CustomerDeleteResponseV2Wire::set_address_deleted,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_methods_deleted",
                &app::model::customers::CustomerDeleteResponseV2Wire::get_payment_methods_deleted,
                &app::model::customers::CustomerDeleteResponseV2Wire::set_payment_methods_deleted,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_delete_response_v2_wire";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerListResponseV1Wire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "data",
                &app::model::customers::CustomerListResponseV1Wire::get_data,
                &app::model::customers::CustomerListResponseV1Wire::set_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_count",
                &app::model::customers::CustomerListResponseV1Wire::get_total_count,
                &app::model::customers::CustomerListResponseV1Wire::set_total_count,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_list_response_v1_wire";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerListResponseV2Wire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "data",
                &app::model::customers::CustomerListResponseV2Wire::get_data,
                &app::model::customers::CustomerListResponseV2Wire::set_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_count",
                &app::model::customers::CustomerListResponseV2Wire::get_total_count,
                &app::model::customers::CustomerListResponseV2Wire::set_total_count,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_list_response_v2_wire";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerGlobalIdMigrationRowResult>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "row_number",
                &app::model::customers::CustomerGlobalIdMigrationRowResult::get_row_number,
                &app::model::customers::CustomerGlobalIdMigrationRowResult::set_row_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::customers::CustomerGlobalIdMigrationRowResult::get_merchant_id,
                &app::model::customers::CustomerGlobalIdMigrationRowResult::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::customers::CustomerGlobalIdMigrationRowResult::get_customer_id,
                &app::model::customers::CustomerGlobalIdMigrationRowResult::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::customers::CustomerGlobalIdMigrationRowResult::get_status,
                &app::model::customers::CustomerGlobalIdMigrationRowResult::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "old_id",
                &app::model::customers::CustomerGlobalIdMigrationRowResult::get_old_id,
                &app::model::customers::CustomerGlobalIdMigrationRowResult::set_old_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "new_id",
                &app::model::customers::CustomerGlobalIdMigrationRowResult::get_new_id,
                &app::model::customers::CustomerGlobalIdMigrationRowResult::set_new_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "message",
                &app::model::customers::CustomerGlobalIdMigrationRowResult::get_message,
                &app::model::customers::CustomerGlobalIdMigrationRowResult::set_message,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_global_id_migration_row";
    }
};

template<>
struct serde::Serializable<app::model::customers::CustomerGlobalIdMigrationResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "total_rows",
                &app::model::customers::CustomerGlobalIdMigrationResponse::get_total_rows,
                &app::model::customers::CustomerGlobalIdMigrationResponse::set_total_rows,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "updated_count",
                &app::model::customers::CustomerGlobalIdMigrationResponse::get_updated_count,
                &app::model::customers::CustomerGlobalIdMigrationResponse::set_updated_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "skipped_count",
                &app::model::customers::CustomerGlobalIdMigrationResponse::get_skipped_count,
                &app::model::customers::CustomerGlobalIdMigrationResponse::set_skipped_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "failed_count",
                &app::model::customers::CustomerGlobalIdMigrationResponse::get_failed_count,
                &app::model::customers::CustomerGlobalIdMigrationResponse::set_failed_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "results",
                &app::model::customers::CustomerGlobalIdMigrationResponse::get_results,
                &app::model::customers::CustomerGlobalIdMigrationResponse::set_results,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "customer_global_id_migration_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::model::customers::tests {
using namespace boost::ut;
suite<"app_model_customers"> app_model_customers_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<app::model::customers::CustomerRequest>::fields())> > 0
        );
    };
};
} // namespace app::model::customers::tests
#endif
