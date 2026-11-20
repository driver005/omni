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
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class CustomerIdValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class CustomerListLimitValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class DescriptionValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class DocumentKindValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class EmailValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class MerchantIdValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class OptionalEncryptableEmailValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class OptionalEncryptableNameValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class OptionalEncryptablePhoneValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class OptionalEncryptableSecretStringValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PrimitiveDateTimeValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class SecretSerdeValueValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class TimeRangeValue
{
public:
    void set_value(std::string value) { this->value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class CustomerDeleteResponse
{
public:
    void set_customer_id(CustomerIdValue value) { customer_id = std::move(value); }
    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept { return customer_id; }
    void set_customer_deleted(bool value) { customer_deleted = std::move(value); }
    [[nodiscard]] const bool& get_customer_deleted() const noexcept { return customer_deleted; }
    void set_address_deleted(bool value) { address_deleted = std::move(value); }
    [[nodiscard]] const bool& get_address_deleted() const noexcept { return address_deleted; }
    void set_payment_methods_deleted(bool value) { payment_methods_deleted = std::move(value); }
    [[nodiscard]] const bool& get_payment_methods_deleted() const noexcept { return payment_methods_deleted; }
    CustomerIdValue customer_id{};
    bool customer_deleted{};
    bool address_deleted{};
    bool payment_methods_deleted{};
};

class CustomerDocumentDetails
{
public:
    void set_document_type(DocumentKindValue value) { document_type = std::move(value); }
    [[nodiscard]] const DocumentKindValue& get_document_type() const noexcept { return document_type; }
    void set_document_number(std::string value) { document_number = std::move(value); }
    [[nodiscard]] const std::string& get_document_number() const noexcept { return document_number; }
    DocumentKindValue document_type{};
    std::string document_number{};
};

class CustomerListRequest
{
public:
    void set_offset(std::optional<std::uint32_t> value) { offset = std::move(value); }
    [[nodiscard]] const std::optional<std::uint32_t>& get_offset() const noexcept { return offset; }
    void set_limit(std::optional<std::uint16_t> value) { limit = std::move(value); }
    [[nodiscard]] const std::optional<std::uint16_t>& get_limit() const noexcept { return limit; }
    void set_customer_id(std::optional<CustomerIdValue> value) { customer_id = std::move(value); }
    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept { return customer_id; }
    std::optional<std::uint32_t> offset{};
    std::optional<std::uint16_t> limit{};
    std::optional<CustomerIdValue> customer_id{};
};

class CustomerListRequestWithConstraints
{
public:
    void set_offset(std::optional<std::uint32_t> value) { offset = std::move(value); }
    [[nodiscard]] const std::optional<std::uint32_t>& get_offset() const noexcept { return offset; }
    void set_limit(std::optional<CustomerListLimitValue> value) { limit = std::move(value); }
    [[nodiscard]] const std::optional<CustomerListLimitValue>& get_limit() const noexcept { return limit; }
    void set_customer_id(std::optional<CustomerIdValue> value) { customer_id = std::move(value); }
    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept { return customer_id; }
    void set_time_range(std::optional<TimeRangeValue> value) { time_range = std::move(value); }
    [[nodiscard]] const std::optional<TimeRangeValue>& get_time_range() const noexcept { return time_range; }
    std::optional<std::uint32_t> offset{};
    std::optional<CustomerListLimitValue> limit{};
    std::optional<CustomerIdValue> customer_id{};
    std::optional<TimeRangeValue> time_range{};
};

class CustomerRequest
{
public:
    void set_customer_id(std::optional<CustomerIdValue> value) { customer_id = std::move(value); }
    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept { return customer_id; }
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_name(std::optional<std::string> value) { name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_name() const noexcept { return name; }
    void set_email(std::optional<EmailValue> value) { email = std::move(value); }
    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept { return email; }
    void set_phone(std::optional<std::string> value) { phone = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_phone() const noexcept { return phone; }
    void set_description(std::optional<DescriptionValue> value) { description = std::move(value); }
    [[nodiscard]] const std::optional<DescriptionValue>& get_description() const noexcept { return description; }
    void set_phone_country_code(std::optional<std::string> value) { phone_country_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_phone_country_code() const noexcept { return phone_country_code; }
    void set_address(std::optional<AddressDetailsValue> value) { address = std::move(value); }
    [[nodiscard]] const std::optional<AddressDetailsValue>& get_address() const noexcept { return address; }
    void set_metadata(std::optional<SecretSerdeValueValue> value) { metadata = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept { return metadata; }
    void set_tax_registration_id(std::optional<std::string> value) { tax_registration_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_tax_registration_id() const noexcept { return tax_registration_id; }
    void set_document_details(std::optional<CustomerDocumentDetails> value) { document_details = std::move(value); }
    [[nodiscard]] const std::optional<CustomerDocumentDetails>& get_document_details() const noexcept { return document_details; }
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
    void set_customer_id(CustomerIdValue value) { customer_id = std::move(value); }
    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept { return customer_id; }
    void set_name(OptionalEncryptableNameValue value) { name = std::move(value); }
    [[nodiscard]] const OptionalEncryptableNameValue& get_name() const noexcept { return name; }
    void set_email(OptionalEncryptableEmailValue value) { email = std::move(value); }
    [[nodiscard]] const OptionalEncryptableEmailValue& get_email() const noexcept { return email; }
    void set_phone(OptionalEncryptablePhoneValue value) { phone = std::move(value); }
    [[nodiscard]] const OptionalEncryptablePhoneValue& get_phone() const noexcept { return phone; }
    void set_phone_country_code(std::optional<std::string> value) { phone_country_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_phone_country_code() const noexcept { return phone_country_code; }
    void set_description(std::optional<DescriptionValue> value) { description = std::move(value); }
    [[nodiscard]] const std::optional<DescriptionValue>& get_description() const noexcept { return description; }
    void set_address(std::optional<AddressDetailsValue> value) { address = std::move(value); }
    [[nodiscard]] const std::optional<AddressDetailsValue>& get_address() const noexcept { return address; }
    void set_created_at(PrimitiveDateTimeValue value) { created_at = std::move(value); }
    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept { return created_at; }
    void set_metadata(std::optional<SecretSerdeValueValue> value) { metadata = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept { return metadata; }
    void set_default_payment_method_id(std::optional<std::string> value) { default_payment_method_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_default_payment_method_id() const noexcept { return default_payment_method_id; }
    void set_tax_registration_id(OptionalEncryptableSecretStringValue value) { tax_registration_id = std::move(value); }
    [[nodiscard]] const OptionalEncryptableSecretStringValue& get_tax_registration_id() const noexcept { return tax_registration_id; }
    void set_document_details(std::optional<CustomerDocumentDetails> value) { document_details = std::move(value); }
    [[nodiscard]] const std::optional<CustomerDocumentDetails>& get_document_details() const noexcept { return document_details; }
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
    void set_merchant_id(MerchantIdValue value) { merchant_id = std::move(value); }
    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept { return merchant_id; }
    void set_name(std::optional<std::string> value) { name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_name() const noexcept { return name; }
    void set_email(std::optional<EmailValue> value) { email = std::move(value); }
    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept { return email; }
    void set_phone(std::optional<std::string> value) { phone = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_phone() const noexcept { return phone; }
    void set_description(std::optional<DescriptionValue> value) { description = std::move(value); }
    [[nodiscard]] const std::optional<DescriptionValue>& get_description() const noexcept { return description; }
    void set_phone_country_code(std::optional<std::string> value) { phone_country_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_phone_country_code() const noexcept { return phone_country_code; }
    void set_address(std::optional<AddressDetailsValue> value) { address = std::move(value); }
    [[nodiscard]] const std::optional<AddressDetailsValue>& get_address() const noexcept { return address; }
    void set_metadata(std::optional<SecretSerdeValueValue> value) { metadata = std::move(value); }
    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_metadata() const noexcept { return metadata; }
    void set_tax_registration_id(std::optional<std::string> value) { tax_registration_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_tax_registration_id() const noexcept { return tax_registration_id; }
    void set_document_details(std::optional<CustomerDocumentDetails> value) { document_details = std::move(value); }
    [[nodiscard]] const std::optional<CustomerDocumentDetails>& get_document_details() const noexcept { return document_details; }
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
    void set_data(std::vector<CustomerResponse> value) { data = std::move(value); }
    [[nodiscard]] const std::vector<CustomerResponse>& get_data() const noexcept { return data; }
    void set_total_count(std::size_t value) { total_count = std::move(value); }
    [[nodiscard]] const std::size_t& get_total_count() const noexcept { return total_count; }
    std::vector<CustomerResponse> data{};
    std::size_t total_count{};
};

class CustomerUpdateRequestInternal
{
public:
    void set_customer_id(CustomerIdValue value) { customer_id = std::move(value); }
    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept { return customer_id; }
    void set_request(CustomerUpdateRequest value) { request = std::move(value); }
    [[nodiscard]] const CustomerUpdateRequest& get_request() const noexcept { return request; }
    CustomerIdValue customer_id{};
    CustomerUpdateRequest request{};
};
} // namespace app::model::customers

template<>
struct serde::Serializable<app::model::customers::CustomerDocumentDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"document_type", &app::model::customers::CustomerDocumentDetails::get_document_type, &app::model::customers::CustomerDocumentDetails::set_document_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"document_number", &app::model::customers::CustomerDocumentDetails::get_document_number, &app::model::customers::CustomerDocumentDetails::set_document_number, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "customer_document_details"; }
};

template<>
struct serde::Serializable<app::model::customers::CustomerRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"customer_id", &app::model::customers::CustomerRequest::get_customer_id, &app::model::customers::CustomerRequest::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::model::customers::CustomerRequest::get_merchant_id, &app::model::customers::CustomerRequest::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &app::model::customers::CustomerRequest::get_name, &app::model::customers::CustomerRequest::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &app::model::customers::CustomerRequest::get_email, &app::model::customers::CustomerRequest::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"phone", &app::model::customers::CustomerRequest::get_phone, &app::model::customers::CustomerRequest::set_phone, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &app::model::customers::CustomerRequest::get_description, &app::model::customers::CustomerRequest::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"phone_country_code", &app::model::customers::CustomerRequest::get_phone_country_code, &app::model::customers::CustomerRequest::set_phone_country_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"address", &app::model::customers::CustomerRequest::get_address, &app::model::customers::CustomerRequest::set_address, serde::FieldOptions{}>{},
            serde::FieldDesc<"metadata", &app::model::customers::CustomerRequest::get_metadata, &app::model::customers::CustomerRequest::set_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"tax_registration_id", &app::model::customers::CustomerRequest::get_tax_registration_id, &app::model::customers::CustomerRequest::set_tax_registration_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"document_details", &app::model::customers::CustomerRequest::get_document_details, &app::model::customers::CustomerRequest::set_document_details, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "customer_request"; }
};

template<>
struct serde::Serializable<app::model::customers::CustomerListRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"offset", &app::model::customers::CustomerListRequest::get_offset, &app::model::customers::CustomerListRequest::set_offset, serde::FieldOptions{}>{},
            serde::FieldDesc<"limit", &app::model::customers::CustomerListRequest::get_limit, &app::model::customers::CustomerListRequest::set_limit, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_id", &app::model::customers::CustomerListRequest::get_customer_id, &app::model::customers::CustomerListRequest::set_customer_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "customer_list_request"; }
};

template<>
struct serde::Serializable<app::model::customers::CustomerListRequestWithConstraints>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"offset", &app::model::customers::CustomerListRequestWithConstraints::get_offset, &app::model::customers::CustomerListRequestWithConstraints::set_offset, serde::FieldOptions{}>{},
            serde::FieldDesc<"limit", &app::model::customers::CustomerListRequestWithConstraints::get_limit, &app::model::customers::CustomerListRequestWithConstraints::set_limit, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_id", &app::model::customers::CustomerListRequestWithConstraints::get_customer_id, &app::model::customers::CustomerListRequestWithConstraints::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"time_range", &app::model::customers::CustomerListRequestWithConstraints::get_time_range, &app::model::customers::CustomerListRequestWithConstraints::set_time_range, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "customer_list_request_with_constraints"; }
};

template<>
struct serde::Serializable<app::model::customers::CustomerResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"customer_id", &app::model::customers::CustomerResponse::get_customer_id, &app::model::customers::CustomerResponse::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &app::model::customers::CustomerResponse::get_name, &app::model::customers::CustomerResponse::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &app::model::customers::CustomerResponse::get_email, &app::model::customers::CustomerResponse::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"phone", &app::model::customers::CustomerResponse::get_phone, &app::model::customers::CustomerResponse::set_phone, serde::FieldOptions{}>{},
            serde::FieldDesc<"phone_country_code", &app::model::customers::CustomerResponse::get_phone_country_code, &app::model::customers::CustomerResponse::set_phone_country_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &app::model::customers::CustomerResponse::get_description, &app::model::customers::CustomerResponse::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"address", &app::model::customers::CustomerResponse::get_address, &app::model::customers::CustomerResponse::set_address, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::model::customers::CustomerResponse::get_created_at, &app::model::customers::CustomerResponse::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"metadata", &app::model::customers::CustomerResponse::get_metadata, &app::model::customers::CustomerResponse::set_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"default_payment_method_id", &app::model::customers::CustomerResponse::get_default_payment_method_id, &app::model::customers::CustomerResponse::set_default_payment_method_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"tax_registration_id", &app::model::customers::CustomerResponse::get_tax_registration_id, &app::model::customers::CustomerResponse::set_tax_registration_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"document_details", &app::model::customers::CustomerResponse::get_document_details, &app::model::customers::CustomerResponse::set_document_details, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "customer_response"; }
};

template<>
struct serde::Serializable<app::model::customers::CustomerDeleteResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"customer_id", &app::model::customers::CustomerDeleteResponse::get_customer_id, &app::model::customers::CustomerDeleteResponse::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_deleted", &app::model::customers::CustomerDeleteResponse::get_customer_deleted, &app::model::customers::CustomerDeleteResponse::set_customer_deleted, serde::FieldOptions{}>{},
            serde::FieldDesc<"address_deleted", &app::model::customers::CustomerDeleteResponse::get_address_deleted, &app::model::customers::CustomerDeleteResponse::set_address_deleted, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_methods_deleted", &app::model::customers::CustomerDeleteResponse::get_payment_methods_deleted, &app::model::customers::CustomerDeleteResponse::set_payment_methods_deleted, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "customer_delete_response"; }
};

template<>
struct serde::Serializable<app::model::customers::CustomerUpdateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"merchant_id", &app::model::customers::CustomerUpdateRequest::get_merchant_id, &app::model::customers::CustomerUpdateRequest::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &app::model::customers::CustomerUpdateRequest::get_name, &app::model::customers::CustomerUpdateRequest::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &app::model::customers::CustomerUpdateRequest::get_email, &app::model::customers::CustomerUpdateRequest::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"phone", &app::model::customers::CustomerUpdateRequest::get_phone, &app::model::customers::CustomerUpdateRequest::set_phone, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &app::model::customers::CustomerUpdateRequest::get_description, &app::model::customers::CustomerUpdateRequest::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"phone_country_code", &app::model::customers::CustomerUpdateRequest::get_phone_country_code, &app::model::customers::CustomerUpdateRequest::set_phone_country_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"address", &app::model::customers::CustomerUpdateRequest::get_address, &app::model::customers::CustomerUpdateRequest::set_address, serde::FieldOptions{}>{},
            serde::FieldDesc<"metadata", &app::model::customers::CustomerUpdateRequest::get_metadata, &app::model::customers::CustomerUpdateRequest::set_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"tax_registration_id", &app::model::customers::CustomerUpdateRequest::get_tax_registration_id, &app::model::customers::CustomerUpdateRequest::set_tax_registration_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"document_details", &app::model::customers::CustomerUpdateRequest::get_document_details, &app::model::customers::CustomerUpdateRequest::set_document_details, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "customer_update_request"; }
};

template<>
struct serde::Serializable<app::model::customers::CustomerUpdateRequestInternal>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"customer_id", &app::model::customers::CustomerUpdateRequestInternal::get_customer_id, &app::model::customers::CustomerUpdateRequestInternal::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"request", &app::model::customers::CustomerUpdateRequestInternal::get_request, &app::model::customers::CustomerUpdateRequestInternal::set_request, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "customer_update_request_internal"; }
};

template<>
struct serde::Serializable<app::model::customers::CustomerListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"data", &app::model::customers::CustomerListResponse::get_data, &app::model::customers::CustomerListResponse::set_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"total_count", &app::model::customers::CustomerListResponse::get_total_count, &app::model::customers::CustomerListResponse::set_total_count, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "customer_list_response"; }
};

#ifdef CONGELADO_TEST
namespace app::model::customers::tests {
using namespace boost::ut;
suite<"app_model_customers"> app_model_customers_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::model::customers::CustomerRequest>::fields())> > 0);
    };
};
}
#endif
