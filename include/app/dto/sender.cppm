module;
export module app_dto:sender;

import core_serde;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::sender {

class AddressDetailsValue : public serde::Serializer<AddressDetailsValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class EmailValue : public serde::Serializer<EmailValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MaskedEmailValue : public serde::Serializer<MaskedEmailValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MaskedPhoneNumberValue : public serde::Serializer<MaskedPhoneNumberValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PhoneDetailsValue : public serde::Serializer<PhoneDetailsValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class SecretDateValue : public serde::Serializer<SecretDateValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class SecretStringValue : public serde::Serializer<SecretStringValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class SenderDetails
{
    Individual,
    Corporate,
};

enum class SenderDetailsResponse
{
    Individual,
    Corporate,
};

enum class SourceOfFunds
{
    Credit,
    Debit,
    Prepaid,
    DepositAccount,
    MobileMoneyAccount,
    Cash,
};

class CorporateSenderDetails : public serde::Serializer<CorporateSenderDetails>
{
public:
    void set_company_name(SecretStringValue value)
    {
        company_name = std::move(value);
    }

    [[nodiscard]] const SecretStringValue& get_company_name() const noexcept
    {
        return company_name;
    }

    void set_address(std::optional<AddressDetailsValue> value)
    {
        address = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressDetailsValue>& get_address() const noexcept
    {
        return address;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    void set_phone(std::optional<PhoneDetailsValue> value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const std::optional<PhoneDetailsValue>& get_phone() const noexcept
    {
        return phone;
    }

    void set_reference_id(std::optional<SecretStringValue> value)
    {
        reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretStringValue>& get_reference_id() const noexcept
    {
        return reference_id;
    }

    void set_source_of_funds(std::optional<SourceOfFunds> value)
    {
        source_of_funds = std::move(value);
    }

    [[nodiscard]] const std::optional<SourceOfFunds>& get_source_of_funds() const noexcept
    {
        return source_of_funds;
    }

    SecretStringValue company_name{};
    std::optional<AddressDetailsValue> address{};
    std::optional<EmailValue> email{};
    std::optional<PhoneDetailsValue> phone{};
    std::optional<SecretStringValue> reference_id{};
    std::optional<SourceOfFunds> source_of_funds{};
};

class IndividualSenderDetails : public serde::Serializer<IndividualSenderDetails>
{
public:
    void set_first_name(std::optional<SecretStringValue> value)
    {
        first_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretStringValue>& get_first_name() const noexcept
    {
        return first_name;
    }

    void set_last_name(std::optional<SecretStringValue> value)
    {
        last_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretStringValue>& get_last_name() const noexcept
    {
        return last_name;
    }

    void set_date_of_birth(std::optional<SecretDateValue> value)
    {
        date_of_birth = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretDateValue>& get_date_of_birth() const noexcept
    {
        return date_of_birth;
    }

    void set_address(std::optional<AddressDetailsValue> value)
    {
        address = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressDetailsValue>& get_address() const noexcept
    {
        return address;
    }

    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    void set_phone(std::optional<PhoneDetailsValue> value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const std::optional<PhoneDetailsValue>& get_phone() const noexcept
    {
        return phone;
    }

    void set_reference_id(std::optional<SecretStringValue> value)
    {
        reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretStringValue>& get_reference_id() const noexcept
    {
        return reference_id;
    }

    void set_source_of_funds(std::optional<SourceOfFunds> value)
    {
        source_of_funds = std::move(value);
    }

    [[nodiscard]] const std::optional<SourceOfFunds>& get_source_of_funds() const noexcept
    {
        return source_of_funds;
    }

    std::optional<SecretStringValue> first_name{};
    std::optional<SecretStringValue> last_name{};
    std::optional<SecretDateValue> date_of_birth{};
    std::optional<AddressDetailsValue> address{};
    std::optional<EmailValue> email{};
    std::optional<PhoneDetailsValue> phone{};
    std::optional<SecretStringValue> reference_id{};
    std::optional<SourceOfFunds> source_of_funds{};
};

class MaskedSenderPhoneDetails : public serde::Serializer<MaskedSenderPhoneDetails>
{
public:
    void set_number(std::optional<MaskedPhoneNumberValue> value)
    {
        number = std::move(value);
    }

    [[nodiscard]] const std::optional<MaskedPhoneNumberValue>& get_number() const noexcept
    {
        return number;
    }

    void set_country_code(std::optional<std::string> value)
    {
        country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_country_code() const noexcept
    {
        return country_code;
    }

    std::optional<MaskedPhoneNumberValue> number{};
    std::optional<std::string> country_code{};
};

class CorporateSenderDetailsResponse : public serde::Serializer<CorporateSenderDetailsResponse>
{
public:
    void set_company_name(SecretStringValue value)
    {
        company_name = std::move(value);
    }

    [[nodiscard]] const SecretStringValue& get_company_name() const noexcept
    {
        return company_name;
    }

    void set_address(std::optional<AddressDetailsValue> value)
    {
        address = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressDetailsValue>& get_address() const noexcept
    {
        return address;
    }

    void set_phone(std::optional<MaskedSenderPhoneDetails> value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const std::optional<MaskedSenderPhoneDetails>& get_phone() const noexcept
    {
        return phone;
    }

    void set_email(std::optional<MaskedEmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<MaskedEmailValue>& get_email() const noexcept
    {
        return email;
    }

    SecretStringValue company_name{};
    std::optional<AddressDetailsValue> address{};
    std::optional<MaskedSenderPhoneDetails> phone{};
    std::optional<MaskedEmailValue> email{};
};

class IndividualSenderDetailsResponse : public serde::Serializer<IndividualSenderDetailsResponse>
{
public:
    void set_first_name(std::optional<SecretStringValue> value)
    {
        first_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretStringValue>& get_first_name() const noexcept
    {
        return first_name;
    }

    void set_last_name(std::optional<SecretStringValue> value)
    {
        last_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretStringValue>& get_last_name() const noexcept
    {
        return last_name;
    }

    void set_address(std::optional<AddressDetailsValue> value)
    {
        address = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressDetailsValue>& get_address() const noexcept
    {
        return address;
    }

    void set_phone(std::optional<MaskedSenderPhoneDetails> value)
    {
        phone = std::move(value);
    }

    [[nodiscard]] const std::optional<MaskedSenderPhoneDetails>& get_phone() const noexcept
    {
        return phone;
    }

    void set_email(std::optional<MaskedEmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<MaskedEmailValue>& get_email() const noexcept
    {
        return email;
    }

    std::optional<SecretStringValue> first_name{};
    std::optional<SecretStringValue> last_name{};
    std::optional<AddressDetailsValue> address{};
    std::optional<MaskedSenderPhoneDetails> phone{};
    std::optional<MaskedEmailValue> email{};
};
} // namespace app::dto::sender

template<>
struct serde::Serializable<app::dto::sender::IndividualSenderDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "first_name",
                &app::dto::sender::IndividualSenderDetails::get_first_name,
                &app::dto::sender::IndividualSenderDetails::set_first_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_name",
                &app::dto::sender::IndividualSenderDetails::get_last_name,
                &app::dto::sender::IndividualSenderDetails::set_last_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "date_of_birth",
                &app::dto::sender::IndividualSenderDetails::get_date_of_birth,
                &app::dto::sender::IndividualSenderDetails::set_date_of_birth,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "address",
                &app::dto::sender::IndividualSenderDetails::get_address,
                &app::dto::sender::IndividualSenderDetails::set_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::dto::sender::IndividualSenderDetails::get_email,
                &app::dto::sender::IndividualSenderDetails::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::dto::sender::IndividualSenderDetails::get_phone,
                &app::dto::sender::IndividualSenderDetails::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reference_id",
                &app::dto::sender::IndividualSenderDetails::get_reference_id,
                &app::dto::sender::IndividualSenderDetails::set_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "source_of_funds",
                &app::dto::sender::IndividualSenderDetails::get_source_of_funds,
                &app::dto::sender::IndividualSenderDetails::set_source_of_funds,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "individual_sender_details";
    }
};

template<>
struct serde::Serializable<app::dto::sender::CorporateSenderDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "company_name",
                &app::dto::sender::CorporateSenderDetails::get_company_name,
                &app::dto::sender::CorporateSenderDetails::set_company_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "address",
                &app::dto::sender::CorporateSenderDetails::get_address,
                &app::dto::sender::CorporateSenderDetails::set_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::dto::sender::CorporateSenderDetails::get_email,
                &app::dto::sender::CorporateSenderDetails::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::dto::sender::CorporateSenderDetails::get_phone,
                &app::dto::sender::CorporateSenderDetails::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reference_id",
                &app::dto::sender::CorporateSenderDetails::get_reference_id,
                &app::dto::sender::CorporateSenderDetails::set_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "source_of_funds",
                &app::dto::sender::CorporateSenderDetails::get_source_of_funds,
                &app::dto::sender::CorporateSenderDetails::set_source_of_funds,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "corporate_sender_details";
    }
};

template<>
struct serde::Serializable<app::dto::sender::MaskedSenderPhoneDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "number",
                &app::dto::sender::MaskedSenderPhoneDetails::get_number,
                &app::dto::sender::MaskedSenderPhoneDetails::set_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "country_code",
                &app::dto::sender::MaskedSenderPhoneDetails::get_country_code,
                &app::dto::sender::MaskedSenderPhoneDetails::set_country_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "masked_sender_phone_details";
    }
};

template<>
struct serde::Serializable<app::dto::sender::IndividualSenderDetailsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "first_name",
                &app::dto::sender::IndividualSenderDetailsResponse::get_first_name,
                &app::dto::sender::IndividualSenderDetailsResponse::set_first_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_name",
                &app::dto::sender::IndividualSenderDetailsResponse::get_last_name,
                &app::dto::sender::IndividualSenderDetailsResponse::set_last_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "address",
                &app::dto::sender::IndividualSenderDetailsResponse::get_address,
                &app::dto::sender::IndividualSenderDetailsResponse::set_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::dto::sender::IndividualSenderDetailsResponse::get_phone,
                &app::dto::sender::IndividualSenderDetailsResponse::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::dto::sender::IndividualSenderDetailsResponse::get_email,
                &app::dto::sender::IndividualSenderDetailsResponse::set_email,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "individual_sender_details_response";
    }
};

template<>
struct serde::Serializable<app::dto::sender::CorporateSenderDetailsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "company_name",
                &app::dto::sender::CorporateSenderDetailsResponse::get_company_name,
                &app::dto::sender::CorporateSenderDetailsResponse::set_company_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "address",
                &app::dto::sender::CorporateSenderDetailsResponse::get_address,
                &app::dto::sender::CorporateSenderDetailsResponse::set_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "phone",
                &app::dto::sender::CorporateSenderDetailsResponse::get_phone,
                &app::dto::sender::CorporateSenderDetailsResponse::set_phone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &app::dto::sender::CorporateSenderDetailsResponse::get_email,
                &app::dto::sender::CorporateSenderDetailsResponse::set_email,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "corporate_sender_details_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::sender::tests {
using namespace boost::ut;
suite<"app_dto_sender"> app_dto_sender_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<decltype(serde::Serializable<
                                       app::dto::sender::IndividualSenderDetails>::fields())> > 0
        );
    };
};
} // namespace app::dto::sender::tests
#endif
