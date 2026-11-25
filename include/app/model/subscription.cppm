module;
export module app_model:subscription;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::subscription {

enum class AccountReceivablesHandling
{
    NoAction,
    SchedulePaymentCollection,
    WriteOff
};

class AddressValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class AuthenticationTypeValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class CancelOption
{
    Immediately,
    EndOfTerm,
    SpecificDate
};

class CaptureMethodValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class ChargesHandling
{
    InvoiceImmediately,
    AddToUnbilledCharges
};

class ClientSecret
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class CreditOption
{
    None,
    Prorate,
    Full
};

class CurrencyValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CustomerAcceptanceValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
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
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class FutureUsageValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class IntentStatusValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class InvoiceIdValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class InvoiceStatusValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ListSubscriptionQuery
{
public:
    void set_limit(std::optional<std::int64_t> value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_limit() const noexcept
    {
        return limit;
    }

    void set_offset(std::optional<std::int64_t> value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_offset() const noexcept
    {
        return offset;
    }

    std::optional<std::int64_t> limit{};
    std::optional<std::int64_t> offset{};
};

class MerchantConnectorAccountIdValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
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
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MinorUnitValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class NextActionDataValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class PauseOption
{
    Immediately,
    EndOfTerm,
    SpecificDate
};

class PaymentExperienceValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentIdValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentMethodDataRequestValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentMethodTypeValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentMethodValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PaymentTypeValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class PeriodUnit
{
    Day,
    Week,
    Month,
    Year
};

class PrimitiveDateTimeValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ProfileIdValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class RecurringDetailsValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class RefundableCreditsHandling
{
    NoAction,
    ScheduleRefund
};

enum class ResumeOption
{
    Immediately,
    SpecificDate
};

class SubscriptionIdValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class SubscriptionItemType
{
    Plan,
    Addon
};

class SubscriptionStatusValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class UnpaidInvoicesHandling
{
    NoAction,
    SchedulePaymentCollection
};

class UpdateSubscriptionRequest
{
public:
    void set_plan_id(std::string value)
    {
        plan_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_plan_id() const noexcept
    {
        return plan_id;
    }

    void set_item_price_id(std::string value)
    {
        item_price_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_item_price_id() const noexcept
    {
        return item_price_id;
    }

    std::string plan_id{};
    std::string item_price_id{};
};

class UrlValue
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CancelSubscriptionResponse
{
public:
    void set_id(SubscriptionIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const SubscriptionIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_status(SubscriptionStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const SubscriptionStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_merchant_reference_id(std::optional<std::string> value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_customer_id(CustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_cancelled_at(std::optional<PrimitiveDateTimeValue> value)
    {
        cancelled_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_cancelled_at() const noexcept
    {
        return cancelled_at;
    }

    SubscriptionIdValue id{};
    SubscriptionStatusValue status{};
    std::optional<std::string> merchant_reference_id{};
    ProfileIdValue profile_id{};
    MerchantIdValue merchant_id{};
    CustomerIdValue customer_id{};
    std::optional<PrimitiveDateTimeValue> cancelled_at{};
};

class ConfirmPaymentsRequestData
{
public:
    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    void set_shipping(std::optional<AddressValue> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_payment_method(PaymentMethodValue value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_type(std::optional<PaymentMethodTypeValue> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodTypeValue>&
    get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method_data(std::optional<PaymentMethodDataRequestValue> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodDataRequestValue>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_customer_acceptance(std::optional<CustomerAcceptanceValue> value)
    {
        customer_acceptance = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerAcceptanceValue>&
    get_customer_acceptance() const noexcept
    {
        return customer_acceptance;
    }

    void set_payment_type(std::optional<PaymentTypeValue> value)
    {
        payment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentTypeValue>& get_payment_type() const noexcept
    {
        return payment_type;
    }

    void set_payment_token(std::optional<std::string> value)
    {
        payment_token = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_token() const noexcept
    {
        return payment_token;
    }

    std::optional<AddressValue> billing{};
    std::optional<AddressValue> shipping{};
    std::optional<ProfileIdValue> profile_id{};
    PaymentMethodValue payment_method{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<PaymentMethodDataRequestValue> payment_method_data{};
    std::optional<CustomerAcceptanceValue> customer_acceptance{};
    std::optional<PaymentTypeValue> payment_type{};
    std::optional<std::string> payment_token{};
};

class ConfirmSubscriptionPaymentDetails
{
public:
    void set_shipping(std::optional<AddressValue> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    void set_payment_method(PaymentMethodValue value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const PaymentMethodValue& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_type(std::optional<PaymentMethodTypeValue> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodTypeValue>&
    get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method_data(std::optional<PaymentMethodDataRequestValue> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodDataRequestValue>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_customer_acceptance(std::optional<CustomerAcceptanceValue> value)
    {
        customer_acceptance = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerAcceptanceValue>&
    get_customer_acceptance() const noexcept
    {
        return customer_acceptance;
    }

    void set_payment_type(std::optional<PaymentTypeValue> value)
    {
        payment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentTypeValue>& get_payment_type() const noexcept
    {
        return payment_type;
    }

    void set_payment_token(std::optional<std::string> value)
    {
        payment_token = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_token() const noexcept
    {
        return payment_token;
    }

    std::optional<AddressValue> shipping{};
    std::optional<AddressValue> billing{};
    PaymentMethodValue payment_method{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<PaymentMethodDataRequestValue> payment_method_data{};
    std::optional<CustomerAcceptanceValue> customer_acceptance{};
    std::optional<PaymentTypeValue> payment_type{};
    std::optional<std::string> payment_token{};
};

class CreateAndConfirmPaymentsRequestData
{
public:
    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_confirm(bool value)
    {
        confirm = std::move(value);
    }

    [[nodiscard]] const bool& get_confirm() const noexcept
    {
        return confirm;
    }

    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    void set_shipping(std::optional<AddressValue> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_setup_future_usage(std::optional<FutureUsageValue> value)
    {
        setup_future_usage = std::move(value);
    }

    [[nodiscard]] const std::optional<FutureUsageValue>& get_setup_future_usage() const noexcept
    {
        return setup_future_usage;
    }

    void set_return_url(std::optional<UrlValue> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_capture_method(std::optional<CaptureMethodValue> value)
    {
        capture_method = std::move(value);
    }

    [[nodiscard]] const std::optional<CaptureMethodValue>& get_capture_method() const noexcept
    {
        return capture_method;
    }

    void set_authentication_type(std::optional<AuthenticationTypeValue> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::optional<AuthenticationTypeValue>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    void set_payment_method(std::optional<PaymentMethodValue> value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodValue>& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_type(std::optional<PaymentMethodTypeValue> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodTypeValue>&
    get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method_data(std::optional<PaymentMethodDataRequestValue> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodDataRequestValue>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_customer_acceptance(std::optional<CustomerAcceptanceValue> value)
    {
        customer_acceptance = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerAcceptanceValue>&
    get_customer_acceptance() const noexcept
    {
        return customer_acceptance;
    }

    void set_payment_type(std::optional<PaymentTypeValue> value)
    {
        payment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentTypeValue>& get_payment_type() const noexcept
    {
        return payment_type;
    }

    void set_recurring_details(std::optional<RecurringDetailsValue> value)
    {
        recurring_details = std::move(value);
    }

    [[nodiscard]] const std::optional<RecurringDetailsValue>& get_recurring_details() const noexcept
    {
        return recurring_details;
    }

    void set_off_session(std::optional<bool> value)
    {
        off_session = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_off_session() const noexcept
    {
        return off_session;
    }

    MinorUnitValue amount{};
    CurrencyValue currency{};
    std::optional<CustomerIdValue> customer_id{};
    bool confirm{};
    std::optional<AddressValue> billing{};
    std::optional<AddressValue> shipping{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<FutureUsageValue> setup_future_usage{};
    std::optional<UrlValue> return_url{};
    std::optional<CaptureMethodValue> capture_method{};
    std::optional<AuthenticationTypeValue> authentication_type{};
    std::optional<PaymentMethodValue> payment_method{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<PaymentMethodDataRequestValue> payment_method_data{};
    std::optional<CustomerAcceptanceValue> customer_acceptance{};
    std::optional<PaymentTypeValue> payment_type{};
    std::optional<RecurringDetailsValue> recurring_details{};
    std::optional<bool> off_session{};
};

class CreateMitPaymentRequestData
{
public:
    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_confirm(bool value)
    {
        confirm = std::move(value);
    }

    [[nodiscard]] const bool& get_confirm() const noexcept
    {
        return confirm;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_recurring_details(std::optional<RecurringDetailsValue> value)
    {
        recurring_details = std::move(value);
    }

    [[nodiscard]] const std::optional<RecurringDetailsValue>& get_recurring_details() const noexcept
    {
        return recurring_details;
    }

    void set_off_session(std::optional<bool> value)
    {
        off_session = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_off_session() const noexcept
    {
        return off_session;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    MinorUnitValue amount{};
    CurrencyValue currency{};
    bool confirm{};
    std::optional<CustomerIdValue> customer_id{};
    std::optional<RecurringDetailsValue> recurring_details{};
    std::optional<bool> off_session{};
    std::optional<ProfileIdValue> profile_id{};
};

class CreatePaymentsRequestData
{
public:
    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    void set_shipping(std::optional<AddressValue> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_setup_future_usage(std::optional<FutureUsageValue> value)
    {
        setup_future_usage = std::move(value);
    }

    [[nodiscard]] const std::optional<FutureUsageValue>& get_setup_future_usage() const noexcept
    {
        return setup_future_usage;
    }

    void set_return_url(std::optional<UrlValue> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_capture_method(std::optional<CaptureMethodValue> value)
    {
        capture_method = std::move(value);
    }

    [[nodiscard]] const std::optional<CaptureMethodValue>& get_capture_method() const noexcept
    {
        return capture_method;
    }

    void set_authentication_type(std::optional<AuthenticationTypeValue> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::optional<AuthenticationTypeValue>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    MinorUnitValue amount{};
    CurrencyValue currency{};
    std::optional<CustomerIdValue> customer_id{};
    std::optional<AddressValue> billing{};
    std::optional<AddressValue> shipping{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<FutureUsageValue> setup_future_usage{};
    std::optional<UrlValue> return_url{};
    std::optional<CaptureMethodValue> capture_method{};
    std::optional<AuthenticationTypeValue> authentication_type{};
};

class CreateSubscriptionPaymentDetails
{
public:
    void set_return_url(UrlValue value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const UrlValue& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_setup_future_usage(std::optional<FutureUsageValue> value)
    {
        setup_future_usage = std::move(value);
    }

    [[nodiscard]] const std::optional<FutureUsageValue>& get_setup_future_usage() const noexcept
    {
        return setup_future_usage;
    }

    void set_capture_method(std::optional<CaptureMethodValue> value)
    {
        capture_method = std::move(value);
    }

    [[nodiscard]] const std::optional<CaptureMethodValue>& get_capture_method() const noexcept
    {
        return capture_method;
    }

    void set_authentication_type(std::optional<AuthenticationTypeValue> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::optional<AuthenticationTypeValue>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    void set_payment_type(std::optional<PaymentTypeValue> value)
    {
        payment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentTypeValue>& get_payment_type() const noexcept
    {
        return payment_type;
    }

    UrlValue return_url{};
    std::optional<FutureUsageValue> setup_future_usage{};
    std::optional<CaptureMethodValue> capture_method{};
    std::optional<AuthenticationTypeValue> authentication_type{};
    std::optional<PaymentTypeValue> payment_type{};
};

class EstimateSubscriptionQuery
{
public:
    void set_client_secret(std::optional<ClientSecret> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<ClientSecret>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_plan_id(std::optional<std::string> value)
    {
        plan_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_plan_id() const noexcept
    {
        return plan_id;
    }

    void set_item_price_id(std::string value)
    {
        item_price_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_item_price_id() const noexcept
    {
        return item_price_id;
    }

    void set_coupon_code(std::optional<std::string> value)
    {
        coupon_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_coupon_code() const noexcept
    {
        return coupon_code;
    }

    std::optional<ClientSecret> client_secret{};
    std::optional<std::string> plan_id{};
    std::string item_price_id{};
    std::optional<std::string> coupon_code{};
};

class GetSubscriptionItemsQuery
{
public:
    void set_client_secret(std::optional<ClientSecret> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<ClientSecret>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_limit(std::optional<std::uint32_t> value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_limit() const noexcept
    {
        return limit;
    }

    void set_offset(std::optional<std::uint32_t> value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const std::optional<std::uint32_t>& get_offset() const noexcept
    {
        return offset;
    }

    void set_item_type(SubscriptionItemType value)
    {
        item_type = value;
    }

    [[nodiscard]] const SubscriptionItemType& get_item_type() const noexcept
    {
        return item_type;
    }

    std::optional<ClientSecret> client_secret{};
    std::optional<std::uint32_t> limit{};
    std::optional<std::uint32_t> offset{};
    SubscriptionItemType item_type{};
};

class Invoice
{
public:
    void set_id(InvoiceIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const InvoiceIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_subscription_id(SubscriptionIdValue value)
    {
        subscription_id = std::move(value);
    }

    [[nodiscard]] const SubscriptionIdValue& get_subscription_id() const noexcept
    {
        return subscription_id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_merchant_connector_id(MerchantConnectorAccountIdValue value)
    {
        merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const MerchantConnectorAccountIdValue& get_merchant_connector_id() const noexcept
    {
        return merchant_connector_id;
    }

    void set_payment_intent_id(std::optional<PaymentIdValue> value)
    {
        payment_intent_id = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentIdValue>& get_payment_intent_id() const noexcept
    {
        return payment_intent_id;
    }

    void set_payment_method_id(std::optional<std::string> value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method_id() const noexcept
    {
        return payment_method_id;
    }

    void set_customer_id(CustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_status(InvoiceStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const InvoiceStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_billing_processor_invoice_id(std::optional<std::string> value)
    {
        billing_processor_invoice_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_billing_processor_invoice_id() const noexcept
    {
        return billing_processor_invoice_id;
    }

    InvoiceIdValue id{};
    SubscriptionIdValue subscription_id{};
    MerchantIdValue merchant_id{};
    ProfileIdValue profile_id{};
    MerchantConnectorAccountIdValue merchant_connector_id{};
    std::optional<PaymentIdValue> payment_intent_id{};
    std::optional<std::string> payment_method_id{};
    CustomerIdValue customer_id{};
    MinorUnitValue amount{};
    CurrencyValue currency{};
    InvoiceStatusValue status{};
    std::optional<std::string> billing_processor_invoice_id{};
};

class PauseSubscriptionRequest
{
public:
    void set_pause_option(std::optional<PauseOption> value)
    {
        pause_option = std::move(value);
    }

    [[nodiscard]] const std::optional<PauseOption>& get_pause_option() const noexcept
    {
        return pause_option;
    }

    void set_pause_at(std::optional<PrimitiveDateTimeValue> value)
    {
        pause_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_pause_at() const noexcept
    {
        return pause_at;
    }

    std::optional<PauseOption> pause_option{};
    std::optional<PrimitiveDateTimeValue> pause_at{};
};

class PauseSubscriptionResponse
{
public:
    void set_id(SubscriptionIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const SubscriptionIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_status(SubscriptionStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const SubscriptionStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_merchant_reference_id(std::optional<std::string> value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_customer_id(CustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_paused_at(std::optional<PrimitiveDateTimeValue> value)
    {
        paused_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_paused_at() const noexcept
    {
        return paused_at;
    }

    SubscriptionIdValue id{};
    SubscriptionStatusValue status{};
    std::optional<std::string> merchant_reference_id{};
    ProfileIdValue profile_id{};
    MerchantIdValue merchant_id{};
    CustomerIdValue customer_id{};
    std::optional<PrimitiveDateTimeValue> paused_at{};
};

class PaymentDetails
{
public:
    void set_payment_method(std::optional<PaymentMethodValue> value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodValue>& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_type(std::optional<PaymentMethodTypeValue> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodTypeValue>&
    get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method_data(std::optional<PaymentMethodDataRequestValue> value)
    {
        payment_method_data = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodDataRequestValue>&
    get_payment_method_data() const noexcept
    {
        return payment_method_data;
    }

    void set_setup_future_usage(std::optional<FutureUsageValue> value)
    {
        setup_future_usage = std::move(value);
    }

    [[nodiscard]] const std::optional<FutureUsageValue>& get_setup_future_usage() const noexcept
    {
        return setup_future_usage;
    }

    void set_customer_acceptance(std::optional<CustomerAcceptanceValue> value)
    {
        customer_acceptance = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerAcceptanceValue>&
    get_customer_acceptance() const noexcept
    {
        return customer_acceptance;
    }

    void set_return_url(std::optional<UrlValue> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_capture_method(std::optional<CaptureMethodValue> value)
    {
        capture_method = std::move(value);
    }

    [[nodiscard]] const std::optional<CaptureMethodValue>& get_capture_method() const noexcept
    {
        return capture_method;
    }

    void set_authentication_type(std::optional<AuthenticationTypeValue> value)
    {
        authentication_type = std::move(value);
    }

    [[nodiscard]] const std::optional<AuthenticationTypeValue>&
    get_authentication_type() const noexcept
    {
        return authentication_type;
    }

    void set_payment_type(std::optional<PaymentTypeValue> value)
    {
        payment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentTypeValue>& get_payment_type() const noexcept
    {
        return payment_type;
    }

    void set_payment_method_id(std::optional<std::string> value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method_id() const noexcept
    {
        return payment_method_id;
    }

    std::optional<PaymentMethodValue> payment_method{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<PaymentMethodDataRequestValue> payment_method_data{};
    std::optional<FutureUsageValue> setup_future_usage{};
    std::optional<CustomerAcceptanceValue> customer_acceptance{};
    std::optional<UrlValue> return_url{};
    std::optional<CaptureMethodValue> capture_method{};
    std::optional<AuthenticationTypeValue> authentication_type{};
    std::optional<PaymentTypeValue> payment_type{};
    std::optional<std::string> payment_method_id{};
};

class PaymentResponseData
{
public:
    void set_payment_id(PaymentIdValue value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const PaymentIdValue& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_status(IntentStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const IntentStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_connector(std::optional<std::string> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector() const noexcept
    {
        return connector;
    }

    void set_payment_method_id(std::optional<std::string> value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method_id() const noexcept
    {
        return payment_method_id;
    }

    void set_return_url(std::optional<UrlValue> value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::optional<UrlValue>& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_next_action(std::optional<NextActionDataValue> value)
    {
        next_action = std::move(value);
    }

    [[nodiscard]] const std::optional<NextActionDataValue>& get_next_action() const noexcept
    {
        return next_action;
    }

    void set_payment_experience(std::optional<PaymentExperienceValue> value)
    {
        payment_experience = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentExperienceValue>&
    get_payment_experience() const noexcept
    {
        return payment_experience;
    }

    void set_error_code(std::optional<std::string> value)
    {
        error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept
    {
        return error_code;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    void set_payment_method_type(std::optional<PaymentMethodTypeValue> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentMethodTypeValue>&
    get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_client_secret(std::optional<std::string> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    void set_shipping(std::optional<AddressValue> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_payment_type(std::optional<PaymentTypeValue> value)
    {
        payment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentTypeValue>& get_payment_type() const noexcept
    {
        return payment_type;
    }

    void set_payment_token(std::optional<std::string> value)
    {
        payment_token = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_token() const noexcept
    {
        return payment_token;
    }

    PaymentIdValue payment_id{};
    IntentStatusValue status{};
    MinorUnitValue amount{};
    CurrencyValue currency{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<std::string> connector{};
    std::optional<std::string> payment_method_id{};
    std::optional<UrlValue> return_url{};
    std::optional<NextActionDataValue> next_action{};
    std::optional<PaymentExperienceValue> payment_experience{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
    std::optional<PaymentMethodTypeValue> payment_method_type{};
    std::optional<std::string> client_secret{};
    std::optional<AddressValue> billing{};
    std::optional<AddressValue> shipping{};
    std::optional<PaymentTypeValue> payment_type{};
    std::optional<std::string> payment_token{};
};

class ResumeSubscriptionRequest
{
public:
    void set_resume_option(std::optional<ResumeOption> value)
    {
        resume_option = std::move(value);
    }

    [[nodiscard]] const std::optional<ResumeOption>& get_resume_option() const noexcept
    {
        return resume_option;
    }

    void set_resume_date(std::optional<PrimitiveDateTimeValue> value)
    {
        resume_date = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_resume_date() const noexcept
    {
        return resume_date;
    }

    void set_charges_handling(std::optional<ChargesHandling> value)
    {
        charges_handling = std::move(value);
    }

    [[nodiscard]] const std::optional<ChargesHandling>& get_charges_handling() const noexcept
    {
        return charges_handling;
    }

    void set_unpaid_invoices_handling(std::optional<UnpaidInvoicesHandling> value)
    {
        unpaid_invoices_handling = std::move(value);
    }

    [[nodiscard]] const std::optional<UnpaidInvoicesHandling>&
    get_unpaid_invoices_handling() const noexcept
    {
        return unpaid_invoices_handling;
    }

    std::optional<ResumeOption> resume_option{};
    std::optional<PrimitiveDateTimeValue> resume_date{};
    std::optional<ChargesHandling> charges_handling{};
    std::optional<UnpaidInvoicesHandling> unpaid_invoices_handling{};
};

class ResumeSubscriptionResponse
{
public:
    void set_id(SubscriptionIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const SubscriptionIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_status(SubscriptionStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const SubscriptionStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_merchant_reference_id(std::optional<std::string> value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_customer_id(CustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_next_billing_at(std::optional<PrimitiveDateTimeValue> value)
    {
        next_billing_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_next_billing_at() const noexcept
    {
        return next_billing_at;
    }

    SubscriptionIdValue id{};
    SubscriptionStatusValue status{};
    std::optional<std::string> merchant_reference_id{};
    ProfileIdValue profile_id{};
    MerchantIdValue merchant_id{};
    CustomerIdValue customer_id{};
    std::optional<PrimitiveDateTimeValue> next_billing_at{};
};

class SubscriptionItemPrices
{
public:
    void set_price_id(std::string value)
    {
        price_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_price_id() const noexcept
    {
        return price_id;
    }

    void set_item_id(std::optional<std::string> value)
    {
        item_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_item_id() const noexcept
    {
        return item_id;
    }

    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_interval(PeriodUnit value)
    {
        interval = value;
    }

    [[nodiscard]] const PeriodUnit& get_interval() const noexcept
    {
        return interval;
    }

    void set_interval_count(std::int64_t value)
    {
        interval_count = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_interval_count() const noexcept
    {
        return interval_count;
    }

    void set_trial_period(std::optional<std::int64_t> value)
    {
        trial_period = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int64_t>& get_trial_period() const noexcept
    {
        return trial_period;
    }

    void set_trial_period_unit(std::optional<PeriodUnit> value)
    {
        trial_period_unit = std::move(value);
    }

    [[nodiscard]] const std::optional<PeriodUnit>& get_trial_period_unit() const noexcept
    {
        return trial_period_unit;
    }

    std::string price_id{};
    std::optional<std::string> item_id{};
    MinorUnitValue amount{};
    CurrencyValue currency{};
    PeriodUnit interval{};
    std::int64_t interval_count{};
    std::optional<std::int64_t> trial_period{};
    std::optional<PeriodUnit> trial_period_unit{};
};

class SubscriptionLineItem
{
public:
    void set_item_id(std::string value)
    {
        item_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_item_id() const noexcept
    {
        return item_id;
    }

    void set_item_type(std::string value)
    {
        item_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_item_type() const noexcept
    {
        return item_type;
    }

    void set_description(std::string value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return description;
    }

    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_quantity(std::int64_t value)
    {
        quantity = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_quantity() const noexcept
    {
        return quantity;
    }

    std::string item_id{};
    std::string item_type{};
    std::string description{};
    MinorUnitValue amount{};
    CurrencyValue currency{};
    std::int64_t quantity{};
};

class ConfirmSubscriptionRequest
{
public:
    void set_client_secret(std::optional<ClientSecret> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<ClientSecret>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_payment_details(ConfirmSubscriptionPaymentDetails value)
    {
        payment_details = std::move(value);
    }

    [[nodiscard]] const ConfirmSubscriptionPaymentDetails& get_payment_details() const noexcept
    {
        return payment_details;
    }

    std::optional<ClientSecret> client_secret{};
    ConfirmSubscriptionPaymentDetails payment_details{};
};

class ConfirmSubscriptionResponse
{
public:
    void set_id(SubscriptionIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const SubscriptionIdValue& get_id() const noexcept
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

    void set_status(SubscriptionStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const SubscriptionStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_plan_id(std::optional<std::string> value)
    {
        plan_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_plan_id() const noexcept
    {
        return plan_id;
    }

    void set_item_price_id(std::optional<std::string> value)
    {
        item_price_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_item_price_id() const noexcept
    {
        return item_price_id;
    }

    void set_coupon(std::optional<std::string> value)
    {
        coupon = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_coupon() const noexcept
    {
        return coupon;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_payment(std::optional<PaymentResponseData> value)
    {
        payment = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentResponseData>& get_payment() const noexcept
    {
        return payment;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_invoice(std::optional<Invoice> value)
    {
        invoice = std::move(value);
    }

    [[nodiscard]] const std::optional<Invoice>& get_invoice() const noexcept
    {
        return invoice;
    }

    void set_billing_processor_subscription_id(std::optional<std::string> value)
    {
        billing_processor_subscription_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_billing_processor_subscription_id() const noexcept
    {
        return billing_processor_subscription_id;
    }

    SubscriptionIdValue id{};
    std::optional<std::string> merchant_reference_id{};
    SubscriptionStatusValue status{};
    std::optional<std::string> plan_id{};
    std::optional<std::string> item_price_id{};
    std::optional<std::string> coupon{};
    ProfileIdValue profile_id{};
    std::optional<PaymentResponseData> payment{};
    std::optional<CustomerIdValue> customer_id{};
    std::optional<Invoice> invoice{};
    std::optional<std::string> billing_processor_subscription_id{};
};

class CreateAndConfirmSubscriptionRequest
{
public:
    void set_plan_id(std::optional<std::string> value)
    {
        plan_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_plan_id() const noexcept
    {
        return plan_id;
    }

    void set_item_price_id(std::string value)
    {
        item_price_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_item_price_id() const noexcept
    {
        return item_price_id;
    }

    void set_coupon_code(std::optional<std::string> value)
    {
        coupon_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_coupon_code() const noexcept
    {
        return coupon_code;
    }

    void set_customer_id(CustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    void set_shipping(std::optional<AddressValue> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_shipping() const noexcept
    {
        return shipping;
    }

    void set_payment_details(PaymentDetails value)
    {
        payment_details = std::move(value);
    }

    [[nodiscard]] const PaymentDetails& get_payment_details() const noexcept
    {
        return payment_details;
    }

    void set_merchant_reference_id(std::optional<std::string> value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    std::optional<std::string> plan_id{};
    std::string item_price_id{};
    std::optional<std::string> coupon_code{};
    CustomerIdValue customer_id{};
    std::optional<AddressValue> billing{};
    std::optional<AddressValue> shipping{};
    PaymentDetails payment_details{};
    std::optional<std::string> merchant_reference_id{};
};

class CreateSubscriptionRequest
{
public:
    void set_merchant_reference_id(std::optional<std::string> value)
    {
        merchant_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_reference_id() const noexcept
    {
        return merchant_reference_id;
    }

    void set_item_price_id(std::string value)
    {
        item_price_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_item_price_id() const noexcept
    {
        return item_price_id;
    }

    void set_plan_id(std::optional<std::string> value)
    {
        plan_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_plan_id() const noexcept
    {
        return plan_id;
    }

    void set_coupon_code(std::optional<std::string> value)
    {
        coupon_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_coupon_code() const noexcept
    {
        return coupon_code;
    }

    void set_customer_id(CustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_payment_details(CreateSubscriptionPaymentDetails value)
    {
        payment_details = std::move(value);
    }

    [[nodiscard]] const CreateSubscriptionPaymentDetails& get_payment_details() const noexcept
    {
        return payment_details;
    }

    void set_billing(std::optional<AddressValue> value)
    {
        billing = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_billing() const noexcept
    {
        return billing;
    }

    void set_shipping(std::optional<AddressValue> value)
    {
        shipping = std::move(value);
    }

    [[nodiscard]] const std::optional<AddressValue>& get_shipping() const noexcept
    {
        return shipping;
    }

    std::optional<std::string> merchant_reference_id{};
    std::string item_price_id{};
    std::optional<std::string> plan_id{};
    std::optional<std::string> coupon_code{};
    CustomerIdValue customer_id{};
    CreateSubscriptionPaymentDetails payment_details{};
    std::optional<AddressValue> billing{};
    std::optional<AddressValue> shipping{};
};

class EstimateSubscriptionResponse
{
public:
    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_currency(CurrencyValue value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency() const noexcept
    {
        return currency;
    }

    void set_plan_id(std::optional<std::string> value)
    {
        plan_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_plan_id() const noexcept
    {
        return plan_id;
    }

    void set_item_price_id(std::optional<std::string> value)
    {
        item_price_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_item_price_id() const noexcept
    {
        return item_price_id;
    }

    void set_coupon_code(std::optional<std::string> value)
    {
        coupon_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_coupon_code() const noexcept
    {
        return coupon_code;
    }

    void set_customer_id(std::optional<CustomerIdValue> value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<CustomerIdValue>& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_line_items(std::vector<SubscriptionLineItem> value)
    {
        line_items = std::move(value);
    }

    [[nodiscard]] const std::vector<SubscriptionLineItem>& get_line_items() const noexcept
    {
        return line_items;
    }

    MinorUnitValue amount{};
    CurrencyValue currency{};
    std::optional<std::string> plan_id{};
    std::optional<std::string> item_price_id{};
    std::optional<std::string> coupon_code{};
    std::optional<CustomerIdValue> customer_id{};
    std::vector<SubscriptionLineItem> line_items{};
};

class GetSubscriptionItemsResponse
{
public:
    void set_item_id(std::string value)
    {
        item_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_item_id() const noexcept
    {
        return item_id;
    }

    void set_name(std::string value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return name;
    }

    void set_description(std::optional<std::string> value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept
    {
        return description;
    }

    void set_price_id(std::vector<SubscriptionItemPrices> value)
    {
        price_id = std::move(value);
    }

    [[nodiscard]] const std::vector<SubscriptionItemPrices>& get_price_id() const noexcept
    {
        return price_id;
    }

    std::string item_id{};
    std::string name{};
    std::optional<std::string> description{};
    std::vector<SubscriptionItemPrices> price_id{};
};

class SubscriptionResponse
{
public:
    void set_id(SubscriptionIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const SubscriptionIdValue& get_id() const noexcept
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

    void set_status(SubscriptionStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const SubscriptionStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_plan_id(std::optional<std::string> value)
    {
        plan_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_plan_id() const noexcept
    {
        return plan_id;
    }

    void set_item_price_id(std::optional<std::string> value)
    {
        item_price_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_item_price_id() const noexcept
    {
        return item_price_id;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_client_secret(std::optional<std::string> value)
    {
        client_secret = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_client_secret() const noexcept
    {
        return client_secret;
    }

    void set_merchant_id(MerchantIdValue value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const MerchantIdValue& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_coupon_code(std::optional<std::string> value)
    {
        coupon_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_coupon_code() const noexcept
    {
        return coupon_code;
    }

    void set_customer_id(CustomerIdValue value)
    {
        customer_id = std::move(value);
    }

    [[nodiscard]] const CustomerIdValue& get_customer_id() const noexcept
    {
        return customer_id;
    }

    void set_payment(std::optional<PaymentResponseData> value)
    {
        payment = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentResponseData>& get_payment() const noexcept
    {
        return payment;
    }

    void set_invoice(std::optional<Invoice> value)
    {
        invoice = std::move(value);
    }

    [[nodiscard]] const std::optional<Invoice>& get_invoice() const noexcept
    {
        return invoice;
    }

    SubscriptionIdValue id{};
    std::optional<std::string> merchant_reference_id{};
    SubscriptionStatusValue status{};
    std::optional<std::string> plan_id{};
    std::optional<std::string> item_price_id{};
    ProfileIdValue profile_id{};
    std::optional<std::string> client_secret{};
    MerchantIdValue merchant_id{};
    std::optional<std::string> coupon_code{};
    CustomerIdValue customer_id{};
    std::optional<PaymentResponseData> payment{};
    std::optional<Invoice> invoice{};
};

enum class UnbilledChargesOption
{
    Invoice,
    Delete
};

class CancelSubscriptionRequest
{
public:
    void set_cancel_option(std::optional<CancelOption> value)
    {
        cancel_option = std::move(value);
    }

    [[nodiscard]] const std::optional<CancelOption>& get_cancel_option() const noexcept
    {
        return cancel_option;
    }

    void set_cancel_at(std::optional<PrimitiveDateTimeValue> value)
    {
        cancel_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_cancel_at() const noexcept
    {
        return cancel_at;
    }

    void set_unbilled_charges_option(std::optional<UnbilledChargesOption> value)
    {
        unbilled_charges_option = std::move(value);
    }

    [[nodiscard]] const std::optional<UnbilledChargesOption>&
    get_unbilled_charges_option() const noexcept
    {
        return unbilled_charges_option;
    }

    void set_credit_option_for_current_term_charges(std::optional<CreditOption> value)
    {
        credit_option_for_current_term_charges = std::move(value);
    }

    [[nodiscard]] const std::optional<CreditOption>&
    get_credit_option_for_current_term_charges() const noexcept
    {
        return credit_option_for_current_term_charges;
    }

    void set_account_receivables_handling(std::optional<AccountReceivablesHandling> value)
    {
        account_receivables_handling = std::move(value);
    }

    [[nodiscard]] const std::optional<AccountReceivablesHandling>&
    get_account_receivables_handling() const noexcept
    {
        return account_receivables_handling;
    }

    void set_refundable_credits_handling(std::optional<RefundableCreditsHandling> value)
    {
        refundable_credits_handling = std::move(value);
    }

    [[nodiscard]] const std::optional<RefundableCreditsHandling>&
    get_refundable_credits_handling() const noexcept
    {
        return refundable_credits_handling;
    }

    void set_cancel_reason_code(std::optional<std::string> value)
    {
        cancel_reason_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_cancel_reason_code() const noexcept
    {
        return cancel_reason_code;
    }

    std::optional<CancelOption> cancel_option{};
    std::optional<PrimitiveDateTimeValue> cancel_at{};
    std::optional<UnbilledChargesOption> unbilled_charges_option{};
    std::optional<CreditOption> credit_option_for_current_term_charges{};
    std::optional<AccountReceivablesHandling> account_receivables_handling{};
    std::optional<RefundableCreditsHandling> refundable_credits_handling{};
    std::optional<std::string> cancel_reason_code{};
};
} // namespace app::model::subscription

template<>
struct serde::Serializable<app::model::subscription::CreateSubscriptionRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::subscription::CreateSubscriptionRequest::get_merchant_reference_id,
                &app::model::subscription::CreateSubscriptionRequest::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "item_price_id",
                &app::model::subscription::CreateSubscriptionRequest::get_item_price_id,
                &app::model::subscription::CreateSubscriptionRequest::set_item_price_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "plan_id",
                &app::model::subscription::CreateSubscriptionRequest::get_plan_id,
                &app::model::subscription::CreateSubscriptionRequest::set_plan_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "coupon_code",
                &app::model::subscription::CreateSubscriptionRequest::get_coupon_code,
                &app::model::subscription::CreateSubscriptionRequest::set_coupon_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::subscription::CreateSubscriptionRequest::get_customer_id,
                &app::model::subscription::CreateSubscriptionRequest::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_details",
                &app::model::subscription::CreateSubscriptionRequest::get_payment_details,
                &app::model::subscription::CreateSubscriptionRequest::set_payment_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::subscription::CreateSubscriptionRequest::get_billing,
                &app::model::subscription::CreateSubscriptionRequest::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::subscription::CreateSubscriptionRequest::get_shipping,
                &app::model::subscription::CreateSubscriptionRequest::set_shipping,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "create_subscription_request";
    }
};

template<>
struct serde::Serializable<app::model::subscription::SubscriptionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::subscription::SubscriptionResponse::get_id,
                &app::model::subscription::SubscriptionResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::subscription::SubscriptionResponse::get_merchant_reference_id,
                &app::model::subscription::SubscriptionResponse::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::subscription::SubscriptionResponse::get_status,
                &app::model::subscription::SubscriptionResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "plan_id",
                &app::model::subscription::SubscriptionResponse::get_plan_id,
                &app::model::subscription::SubscriptionResponse::set_plan_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "item_price_id",
                &app::model::subscription::SubscriptionResponse::get_item_price_id,
                &app::model::subscription::SubscriptionResponse::set_item_price_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::subscription::SubscriptionResponse::get_profile_id,
                &app::model::subscription::SubscriptionResponse::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::subscription::SubscriptionResponse::get_client_secret,
                &app::model::subscription::SubscriptionResponse::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::subscription::SubscriptionResponse::get_merchant_id,
                &app::model::subscription::SubscriptionResponse::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "coupon_code",
                &app::model::subscription::SubscriptionResponse::get_coupon_code,
                &app::model::subscription::SubscriptionResponse::set_coupon_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::subscription::SubscriptionResponse::get_customer_id,
                &app::model::subscription::SubscriptionResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment",
                &app::model::subscription::SubscriptionResponse::get_payment,
                &app::model::subscription::SubscriptionResponse::set_payment,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "invoice",
                &app::model::subscription::SubscriptionResponse::get_invoice,
                &app::model::subscription::SubscriptionResponse::set_invoice,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "subscription_response";
    }
};

template<>
struct serde::Serializable<app::model::subscription::GetSubscriptionItemsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "item_id",
                &app::model::subscription::GetSubscriptionItemsResponse::get_item_id,
                &app::model::subscription::GetSubscriptionItemsResponse::set_item_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::model::subscription::GetSubscriptionItemsResponse::get_name,
                &app::model::subscription::GetSubscriptionItemsResponse::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::subscription::GetSubscriptionItemsResponse::get_description,
                &app::model::subscription::GetSubscriptionItemsResponse::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "price_id",
                &app::model::subscription::GetSubscriptionItemsResponse::get_price_id,
                &app::model::subscription::GetSubscriptionItemsResponse::set_price_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_subscription_items_response";
    }
};

template<>
struct serde::Serializable<app::model::subscription::SubscriptionItemPrices>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "price_id",
                &app::model::subscription::SubscriptionItemPrices::get_price_id,
                &app::model::subscription::SubscriptionItemPrices::set_price_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "item_id",
                &app::model::subscription::SubscriptionItemPrices::get_item_id,
                &app::model::subscription::SubscriptionItemPrices::set_item_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::subscription::SubscriptionItemPrices::get_amount,
                &app::model::subscription::SubscriptionItemPrices::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::subscription::SubscriptionItemPrices::get_currency,
                &app::model::subscription::SubscriptionItemPrices::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "interval",
                &app::model::subscription::SubscriptionItemPrices::get_interval,
                &app::model::subscription::SubscriptionItemPrices::set_interval,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "interval_count",
                &app::model::subscription::SubscriptionItemPrices::get_interval_count,
                &app::model::subscription::SubscriptionItemPrices::set_interval_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "trial_period",
                &app::model::subscription::SubscriptionItemPrices::get_trial_period,
                &app::model::subscription::SubscriptionItemPrices::set_trial_period,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "trial_period_unit",
                &app::model::subscription::SubscriptionItemPrices::get_trial_period_unit,
                &app::model::subscription::SubscriptionItemPrices::set_trial_period_unit,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "subscription_item_prices";
    }
};

template<>
struct serde::Serializable<app::model::subscription::ClientSecret>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &app::model::subscription::ClientSecret::get_value,
            &app::model::subscription::ClientSecret::set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "client_secret";
    }
};

template<>
struct serde::Serializable<app::model::subscription::GetSubscriptionItemsQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "client_secret",
                &app::model::subscription::GetSubscriptionItemsQuery::get_client_secret,
                &app::model::subscription::GetSubscriptionItemsQuery::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::subscription::GetSubscriptionItemsQuery::get_limit,
                &app::model::subscription::GetSubscriptionItemsQuery::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::subscription::GetSubscriptionItemsQuery::get_offset,
                &app::model::subscription::GetSubscriptionItemsQuery::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "item_type",
                &app::model::subscription::GetSubscriptionItemsQuery::get_item_type,
                &app::model::subscription::GetSubscriptionItemsQuery::set_item_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_subscription_items_query";
    }
};

template<>
struct serde::Serializable<app::model::subscription::ConfirmSubscriptionPaymentDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "shipping",
                &app::model::subscription::ConfirmSubscriptionPaymentDetails::get_shipping,
                &app::model::subscription::ConfirmSubscriptionPaymentDetails::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::subscription::ConfirmSubscriptionPaymentDetails::get_billing,
                &app::model::subscription::ConfirmSubscriptionPaymentDetails::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::subscription::ConfirmSubscriptionPaymentDetails::get_payment_method,
                &app::model::subscription::ConfirmSubscriptionPaymentDetails::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::subscription::ConfirmSubscriptionPaymentDetails::
                    get_payment_method_type,
                &app::model::subscription::ConfirmSubscriptionPaymentDetails::
                    set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::subscription::ConfirmSubscriptionPaymentDetails::
                    get_payment_method_data,
                &app::model::subscription::ConfirmSubscriptionPaymentDetails::
                    set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_acceptance",
                &app::model::subscription::ConfirmSubscriptionPaymentDetails::
                    get_customer_acceptance,
                &app::model::subscription::ConfirmSubscriptionPaymentDetails::
                    set_customer_acceptance,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_type",
                &app::model::subscription::ConfirmSubscriptionPaymentDetails::get_payment_type,
                &app::model::subscription::ConfirmSubscriptionPaymentDetails::set_payment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_token",
                &app::model::subscription::ConfirmSubscriptionPaymentDetails::get_payment_token,
                &app::model::subscription::ConfirmSubscriptionPaymentDetails::set_payment_token,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "confirm_subscription_payment_details";
    }
};

template<>
struct serde::Serializable<app::model::subscription::CreateSubscriptionPaymentDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "return_url",
                &app::model::subscription::CreateSubscriptionPaymentDetails::get_return_url,
                &app::model::subscription::CreateSubscriptionPaymentDetails::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage",
                &app::model::subscription::CreateSubscriptionPaymentDetails::get_setup_future_usage,
                &app::model::subscription::CreateSubscriptionPaymentDetails::set_setup_future_usage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_method",
                &app::model::subscription::CreateSubscriptionPaymentDetails::get_capture_method,
                &app::model::subscription::CreateSubscriptionPaymentDetails::set_capture_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::subscription::CreateSubscriptionPaymentDetails::
                    get_authentication_type,
                &app::model::subscription::CreateSubscriptionPaymentDetails::
                    set_authentication_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_type",
                &app::model::subscription::CreateSubscriptionPaymentDetails::get_payment_type,
                &app::model::subscription::CreateSubscriptionPaymentDetails::set_payment_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "create_subscription_payment_details";
    }
};

template<>
struct serde::Serializable<app::model::subscription::PaymentDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method",
                &app::model::subscription::PaymentDetails::get_payment_method,
                &app::model::subscription::PaymentDetails::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::subscription::PaymentDetails::get_payment_method_type,
                &app::model::subscription::PaymentDetails::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::subscription::PaymentDetails::get_payment_method_data,
                &app::model::subscription::PaymentDetails::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage",
                &app::model::subscription::PaymentDetails::get_setup_future_usage,
                &app::model::subscription::PaymentDetails::set_setup_future_usage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_acceptance",
                &app::model::subscription::PaymentDetails::get_customer_acceptance,
                &app::model::subscription::PaymentDetails::set_customer_acceptance,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::subscription::PaymentDetails::get_return_url,
                &app::model::subscription::PaymentDetails::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_method",
                &app::model::subscription::PaymentDetails::get_capture_method,
                &app::model::subscription::PaymentDetails::set_capture_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::subscription::PaymentDetails::get_authentication_type,
                &app::model::subscription::PaymentDetails::set_authentication_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_type",
                &app::model::subscription::PaymentDetails::get_payment_type,
                &app::model::subscription::PaymentDetails::set_payment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_id",
                &app::model::subscription::PaymentDetails::get_payment_method_id,
                &app::model::subscription::PaymentDetails::set_payment_method_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_details";
    }
};

template<>
struct serde::Serializable<app::model::subscription::CreatePaymentsRequestData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount",
                &app::model::subscription::CreatePaymentsRequestData::get_amount,
                &app::model::subscription::CreatePaymentsRequestData::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::subscription::CreatePaymentsRequestData::get_currency,
                &app::model::subscription::CreatePaymentsRequestData::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::subscription::CreatePaymentsRequestData::get_customer_id,
                &app::model::subscription::CreatePaymentsRequestData::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::subscription::CreatePaymentsRequestData::get_billing,
                &app::model::subscription::CreatePaymentsRequestData::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::subscription::CreatePaymentsRequestData::get_shipping,
                &app::model::subscription::CreatePaymentsRequestData::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::subscription::CreatePaymentsRequestData::get_profile_id,
                &app::model::subscription::CreatePaymentsRequestData::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage",
                &app::model::subscription::CreatePaymentsRequestData::get_setup_future_usage,
                &app::model::subscription::CreatePaymentsRequestData::set_setup_future_usage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::subscription::CreatePaymentsRequestData::get_return_url,
                &app::model::subscription::CreatePaymentsRequestData::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_method",
                &app::model::subscription::CreatePaymentsRequestData::get_capture_method,
                &app::model::subscription::CreatePaymentsRequestData::set_capture_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::subscription::CreatePaymentsRequestData::get_authentication_type,
                &app::model::subscription::CreatePaymentsRequestData::set_authentication_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "create_payments_request_data";
    }
};

template<>
struct serde::Serializable<app::model::subscription::ConfirmPaymentsRequestData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "billing",
                &app::model::subscription::ConfirmPaymentsRequestData::get_billing,
                &app::model::subscription::ConfirmPaymentsRequestData::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::subscription::ConfirmPaymentsRequestData::get_shipping,
                &app::model::subscription::ConfirmPaymentsRequestData::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::subscription::ConfirmPaymentsRequestData::get_profile_id,
                &app::model::subscription::ConfirmPaymentsRequestData::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::subscription::ConfirmPaymentsRequestData::get_payment_method,
                &app::model::subscription::ConfirmPaymentsRequestData::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::subscription::ConfirmPaymentsRequestData::get_payment_method_type,
                &app::model::subscription::ConfirmPaymentsRequestData::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::subscription::ConfirmPaymentsRequestData::get_payment_method_data,
                &app::model::subscription::ConfirmPaymentsRequestData::set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_acceptance",
                &app::model::subscription::ConfirmPaymentsRequestData::get_customer_acceptance,
                &app::model::subscription::ConfirmPaymentsRequestData::set_customer_acceptance,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_type",
                &app::model::subscription::ConfirmPaymentsRequestData::get_payment_type,
                &app::model::subscription::ConfirmPaymentsRequestData::set_payment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_token",
                &app::model::subscription::ConfirmPaymentsRequestData::get_payment_token,
                &app::model::subscription::ConfirmPaymentsRequestData::set_payment_token,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "confirm_payments_request_data";
    }
};

template<>
struct serde::Serializable<app::model::subscription::CreateAndConfirmPaymentsRequestData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount",
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::get_amount,
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::get_currency,
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::get_customer_id,
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "confirm",
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::get_confirm,
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::set_confirm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::get_billing,
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::get_shipping,
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::get_profile_id,
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "setup_future_usage",
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::
                    get_setup_future_usage,
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::
                    set_setup_future_usage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::get_return_url,
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "capture_method",
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::get_capture_method,
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::set_capture_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_type",
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::
                    get_authentication_type,
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::
                    set_authentication_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::get_payment_method,
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::
                    get_payment_method_type,
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::
                    set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_data",
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::
                    get_payment_method_data,
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::
                    set_payment_method_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_acceptance",
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::
                    get_customer_acceptance,
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::
                    set_customer_acceptance,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_type",
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::get_payment_type,
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::set_payment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_details",
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::
                    get_recurring_details,
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::
                    set_recurring_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "off_session",
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::get_off_session,
                &app::model::subscription::CreateAndConfirmPaymentsRequestData::set_off_session,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "create_and_confirm_payments_request_data";
    }
};

template<>
struct serde::Serializable<app::model::subscription::PaymentResponseData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &app::model::subscription::PaymentResponseData::get_payment_id,
                &app::model::subscription::PaymentResponseData::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::subscription::PaymentResponseData::get_status,
                &app::model::subscription::PaymentResponseData::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::subscription::PaymentResponseData::get_amount,
                &app::model::subscription::PaymentResponseData::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::subscription::PaymentResponseData::get_currency,
                &app::model::subscription::PaymentResponseData::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::subscription::PaymentResponseData::get_profile_id,
                &app::model::subscription::PaymentResponseData::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::subscription::PaymentResponseData::get_connector,
                &app::model::subscription::PaymentResponseData::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_id",
                &app::model::subscription::PaymentResponseData::get_payment_method_id,
                &app::model::subscription::PaymentResponseData::set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::model::subscription::PaymentResponseData::get_return_url,
                &app::model::subscription::PaymentResponseData::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "next_action",
                &app::model::subscription::PaymentResponseData::get_next_action,
                &app::model::subscription::PaymentResponseData::set_next_action,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_experience",
                &app::model::subscription::PaymentResponseData::get_payment_experience,
                &app::model::subscription::PaymentResponseData::set_payment_experience,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &app::model::subscription::PaymentResponseData::get_error_code,
                &app::model::subscription::PaymentResponseData::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::subscription::PaymentResponseData::get_error_message,
                &app::model::subscription::PaymentResponseData::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::subscription::PaymentResponseData::get_payment_method_type,
                &app::model::subscription::PaymentResponseData::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_secret",
                &app::model::subscription::PaymentResponseData::get_client_secret,
                &app::model::subscription::PaymentResponseData::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::subscription::PaymentResponseData::get_billing,
                &app::model::subscription::PaymentResponseData::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::subscription::PaymentResponseData::get_shipping,
                &app::model::subscription::PaymentResponseData::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_type",
                &app::model::subscription::PaymentResponseData::get_payment_type,
                &app::model::subscription::PaymentResponseData::set_payment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_token",
                &app::model::subscription::PaymentResponseData::get_payment_token,
                &app::model::subscription::PaymentResponseData::set_payment_token,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_response_data";
    }
};

template<>
struct serde::Serializable<app::model::subscription::CreateMitPaymentRequestData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount",
                &app::model::subscription::CreateMitPaymentRequestData::get_amount,
                &app::model::subscription::CreateMitPaymentRequestData::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::subscription::CreateMitPaymentRequestData::get_currency,
                &app::model::subscription::CreateMitPaymentRequestData::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "confirm",
                &app::model::subscription::CreateMitPaymentRequestData::get_confirm,
                &app::model::subscription::CreateMitPaymentRequestData::set_confirm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::subscription::CreateMitPaymentRequestData::get_customer_id,
                &app::model::subscription::CreateMitPaymentRequestData::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_details",
                &app::model::subscription::CreateMitPaymentRequestData::get_recurring_details,
                &app::model::subscription::CreateMitPaymentRequestData::set_recurring_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "off_session",
                &app::model::subscription::CreateMitPaymentRequestData::get_off_session,
                &app::model::subscription::CreateMitPaymentRequestData::set_off_session,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::subscription::CreateMitPaymentRequestData::get_profile_id,
                &app::model::subscription::CreateMitPaymentRequestData::set_profile_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "create_mit_payment_request_data";
    }
};

template<>
struct serde::Serializable<app::model::subscription::Invoice>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::subscription::Invoice::get_id,
                &app::model::subscription::Invoice::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "subscription_id",
                &app::model::subscription::Invoice::get_subscription_id,
                &app::model::subscription::Invoice::set_subscription_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::subscription::Invoice::get_merchant_id,
                &app::model::subscription::Invoice::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::subscription::Invoice::get_profile_id,
                &app::model::subscription::Invoice::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::subscription::Invoice::get_merchant_connector_id,
                &app::model::subscription::Invoice::set_merchant_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_intent_id",
                &app::model::subscription::Invoice::get_payment_intent_id,
                &app::model::subscription::Invoice::set_payment_intent_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_id",
                &app::model::subscription::Invoice::get_payment_method_id,
                &app::model::subscription::Invoice::set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::subscription::Invoice::get_customer_id,
                &app::model::subscription::Invoice::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::subscription::Invoice::get_amount,
                &app::model::subscription::Invoice::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::subscription::Invoice::get_currency,
                &app::model::subscription::Invoice::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::subscription::Invoice::get_status,
                &app::model::subscription::Invoice::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_processor_invoice_id",
                &app::model::subscription::Invoice::get_billing_processor_invoice_id,
                &app::model::subscription::Invoice::set_billing_processor_invoice_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "invoice";
    }
};

template<>
struct serde::Serializable<app::model::subscription::ConfirmSubscriptionRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "client_secret",
                &app::model::subscription::ConfirmSubscriptionRequest::get_client_secret,
                &app::model::subscription::ConfirmSubscriptionRequest::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_details",
                &app::model::subscription::ConfirmSubscriptionRequest::get_payment_details,
                &app::model::subscription::ConfirmSubscriptionRequest::set_payment_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "confirm_subscription_request";
    }
};

template<>
struct serde::Serializable<app::model::subscription::CreateAndConfirmSubscriptionRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "plan_id",
                &app::model::subscription::CreateAndConfirmSubscriptionRequest::get_plan_id,
                &app::model::subscription::CreateAndConfirmSubscriptionRequest::set_plan_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "item_price_id",
                &app::model::subscription::CreateAndConfirmSubscriptionRequest::get_item_price_id,
                &app::model::subscription::CreateAndConfirmSubscriptionRequest::set_item_price_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "coupon_code",
                &app::model::subscription::CreateAndConfirmSubscriptionRequest::get_coupon_code,
                &app::model::subscription::CreateAndConfirmSubscriptionRequest::set_coupon_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::subscription::CreateAndConfirmSubscriptionRequest::get_customer_id,
                &app::model::subscription::CreateAndConfirmSubscriptionRequest::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing",
                &app::model::subscription::CreateAndConfirmSubscriptionRequest::get_billing,
                &app::model::subscription::CreateAndConfirmSubscriptionRequest::set_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping",
                &app::model::subscription::CreateAndConfirmSubscriptionRequest::get_shipping,
                &app::model::subscription::CreateAndConfirmSubscriptionRequest::set_shipping,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_details",
                &app::model::subscription::CreateAndConfirmSubscriptionRequest::get_payment_details,
                &app::model::subscription::CreateAndConfirmSubscriptionRequest::set_payment_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::subscription::CreateAndConfirmSubscriptionRequest::
                    get_merchant_reference_id,
                &app::model::subscription::CreateAndConfirmSubscriptionRequest::
                    set_merchant_reference_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "create_and_confirm_subscription_request";
    }
};

template<>
struct serde::Serializable<app::model::subscription::ConfirmSubscriptionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::subscription::ConfirmSubscriptionResponse::get_id,
                &app::model::subscription::ConfirmSubscriptionResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::subscription::ConfirmSubscriptionResponse::get_merchant_reference_id,
                &app::model::subscription::ConfirmSubscriptionResponse::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::subscription::ConfirmSubscriptionResponse::get_status,
                &app::model::subscription::ConfirmSubscriptionResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "plan_id",
                &app::model::subscription::ConfirmSubscriptionResponse::get_plan_id,
                &app::model::subscription::ConfirmSubscriptionResponse::set_plan_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "item_price_id",
                &app::model::subscription::ConfirmSubscriptionResponse::get_item_price_id,
                &app::model::subscription::ConfirmSubscriptionResponse::set_item_price_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "coupon",
                &app::model::subscription::ConfirmSubscriptionResponse::get_coupon,
                &app::model::subscription::ConfirmSubscriptionResponse::set_coupon,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::subscription::ConfirmSubscriptionResponse::get_profile_id,
                &app::model::subscription::ConfirmSubscriptionResponse::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment",
                &app::model::subscription::ConfirmSubscriptionResponse::get_payment,
                &app::model::subscription::ConfirmSubscriptionResponse::set_payment,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::subscription::ConfirmSubscriptionResponse::get_customer_id,
                &app::model::subscription::ConfirmSubscriptionResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "invoice",
                &app::model::subscription::ConfirmSubscriptionResponse::get_invoice,
                &app::model::subscription::ConfirmSubscriptionResponse::set_invoice,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_processor_subscription_id",
                &app::model::subscription::ConfirmSubscriptionResponse::
                    get_billing_processor_subscription_id,
                &app::model::subscription::ConfirmSubscriptionResponse::
                    set_billing_processor_subscription_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "confirm_subscription_response";
    }
};

template<>
struct serde::Serializable<app::model::subscription::UpdateSubscriptionRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "plan_id",
                &app::model::subscription::UpdateSubscriptionRequest::get_plan_id,
                &app::model::subscription::UpdateSubscriptionRequest::set_plan_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "item_price_id",
                &app::model::subscription::UpdateSubscriptionRequest::get_item_price_id,
                &app::model::subscription::UpdateSubscriptionRequest::set_item_price_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "update_subscription_request";
    }
};

template<>
struct serde::Serializable<app::model::subscription::EstimateSubscriptionQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "client_secret",
                &app::model::subscription::EstimateSubscriptionQuery::get_client_secret,
                &app::model::subscription::EstimateSubscriptionQuery::set_client_secret,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "plan_id",
                &app::model::subscription::EstimateSubscriptionQuery::get_plan_id,
                &app::model::subscription::EstimateSubscriptionQuery::set_plan_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "item_price_id",
                &app::model::subscription::EstimateSubscriptionQuery::get_item_price_id,
                &app::model::subscription::EstimateSubscriptionQuery::set_item_price_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "coupon_code",
                &app::model::subscription::EstimateSubscriptionQuery::get_coupon_code,
                &app::model::subscription::EstimateSubscriptionQuery::set_coupon_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "estimate_subscription_query";
    }
};

template<>
struct serde::Serializable<app::model::subscription::ListSubscriptionQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "limit",
                &app::model::subscription::ListSubscriptionQuery::get_limit,
                &app::model::subscription::ListSubscriptionQuery::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::subscription::ListSubscriptionQuery::get_offset,
                &app::model::subscription::ListSubscriptionQuery::set_offset,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "list_subscription_query";
    }
};

template<>
struct serde::Serializable<app::model::subscription::SubscriptionLineItem>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "item_id",
                &app::model::subscription::SubscriptionLineItem::get_item_id,
                &app::model::subscription::SubscriptionLineItem::set_item_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "item_type",
                &app::model::subscription::SubscriptionLineItem::get_item_type,
                &app::model::subscription::SubscriptionLineItem::set_item_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::subscription::SubscriptionLineItem::get_description,
                &app::model::subscription::SubscriptionLineItem::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::subscription::SubscriptionLineItem::get_amount,
                &app::model::subscription::SubscriptionLineItem::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::subscription::SubscriptionLineItem::get_currency,
                &app::model::subscription::SubscriptionLineItem::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "quantity",
                &app::model::subscription::SubscriptionLineItem::get_quantity,
                &app::model::subscription::SubscriptionLineItem::set_quantity,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "subscription_line_item";
    }
};

template<>
struct serde::Serializable<app::model::subscription::EstimateSubscriptionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount",
                &app::model::subscription::EstimateSubscriptionResponse::get_amount,
                &app::model::subscription::EstimateSubscriptionResponse::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::subscription::EstimateSubscriptionResponse::get_currency,
                &app::model::subscription::EstimateSubscriptionResponse::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "plan_id",
                &app::model::subscription::EstimateSubscriptionResponse::get_plan_id,
                &app::model::subscription::EstimateSubscriptionResponse::set_plan_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "item_price_id",
                &app::model::subscription::EstimateSubscriptionResponse::get_item_price_id,
                &app::model::subscription::EstimateSubscriptionResponse::set_item_price_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "coupon_code",
                &app::model::subscription::EstimateSubscriptionResponse::get_coupon_code,
                &app::model::subscription::EstimateSubscriptionResponse::set_coupon_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::subscription::EstimateSubscriptionResponse::get_customer_id,
                &app::model::subscription::EstimateSubscriptionResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "line_items",
                &app::model::subscription::EstimateSubscriptionResponse::get_line_items,
                &app::model::subscription::EstimateSubscriptionResponse::set_line_items,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "estimate_subscription_response";
    }
};

template<>
struct serde::Serializable<app::model::subscription::PauseSubscriptionRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "pause_option",
                &app::model::subscription::PauseSubscriptionRequest::get_pause_option,
                &app::model::subscription::PauseSubscriptionRequest::set_pause_option,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "pause_at",
                &app::model::subscription::PauseSubscriptionRequest::get_pause_at,
                &app::model::subscription::PauseSubscriptionRequest::set_pause_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "pause_subscription_request";
    }
};

template<>
struct serde::Serializable<app::model::subscription::PauseSubscriptionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::subscription::PauseSubscriptionResponse::get_id,
                &app::model::subscription::PauseSubscriptionResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::subscription::PauseSubscriptionResponse::get_status,
                &app::model::subscription::PauseSubscriptionResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::subscription::PauseSubscriptionResponse::get_merchant_reference_id,
                &app::model::subscription::PauseSubscriptionResponse::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::subscription::PauseSubscriptionResponse::get_profile_id,
                &app::model::subscription::PauseSubscriptionResponse::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::subscription::PauseSubscriptionResponse::get_merchant_id,
                &app::model::subscription::PauseSubscriptionResponse::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::subscription::PauseSubscriptionResponse::get_customer_id,
                &app::model::subscription::PauseSubscriptionResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "paused_at",
                &app::model::subscription::PauseSubscriptionResponse::get_paused_at,
                &app::model::subscription::PauseSubscriptionResponse::set_paused_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "pause_subscription_response";
    }
};

template<>
struct serde::Serializable<app::model::subscription::ResumeSubscriptionRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "resume_option",
                &app::model::subscription::ResumeSubscriptionRequest::get_resume_option,
                &app::model::subscription::ResumeSubscriptionRequest::set_resume_option,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "resume_date",
                &app::model::subscription::ResumeSubscriptionRequest::get_resume_date,
                &app::model::subscription::ResumeSubscriptionRequest::set_resume_date,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "charges_handling",
                &app::model::subscription::ResumeSubscriptionRequest::get_charges_handling,
                &app::model::subscription::ResumeSubscriptionRequest::set_charges_handling,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "unpaid_invoices_handling",
                &app::model::subscription::ResumeSubscriptionRequest::get_unpaid_invoices_handling,
                &app::model::subscription::ResumeSubscriptionRequest::set_unpaid_invoices_handling,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "resume_subscription_request";
    }
};

template<>
struct serde::Serializable<app::model::subscription::ResumeSubscriptionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::subscription::ResumeSubscriptionResponse::get_id,
                &app::model::subscription::ResumeSubscriptionResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::subscription::ResumeSubscriptionResponse::get_status,
                &app::model::subscription::ResumeSubscriptionResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::subscription::ResumeSubscriptionResponse::get_merchant_reference_id,
                &app::model::subscription::ResumeSubscriptionResponse::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::subscription::ResumeSubscriptionResponse::get_profile_id,
                &app::model::subscription::ResumeSubscriptionResponse::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::subscription::ResumeSubscriptionResponse::get_merchant_id,
                &app::model::subscription::ResumeSubscriptionResponse::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::subscription::ResumeSubscriptionResponse::get_customer_id,
                &app::model::subscription::ResumeSubscriptionResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "next_billing_at",
                &app::model::subscription::ResumeSubscriptionResponse::get_next_billing_at,
                &app::model::subscription::ResumeSubscriptionResponse::set_next_billing_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "resume_subscription_response";
    }
};

template<>
struct serde::Serializable<app::model::subscription::CancelSubscriptionRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "cancel_option",
                &app::model::subscription::CancelSubscriptionRequest::get_cancel_option,
                &app::model::subscription::CancelSubscriptionRequest::set_cancel_option,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cancel_at",
                &app::model::subscription::CancelSubscriptionRequest::get_cancel_at,
                &app::model::subscription::CancelSubscriptionRequest::set_cancel_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "unbilled_charges_option",
                &app::model::subscription::CancelSubscriptionRequest::get_unbilled_charges_option,
                &app::model::subscription::CancelSubscriptionRequest::set_unbilled_charges_option,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "credit_option_for_current_term_charges",
                &app::model::subscription::CancelSubscriptionRequest::
                    get_credit_option_for_current_term_charges,
                &app::model::subscription::CancelSubscriptionRequest::
                    set_credit_option_for_current_term_charges,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_receivables_handling",
                &app::model::subscription::CancelSubscriptionRequest::
                    get_account_receivables_handling,
                &app::model::subscription::CancelSubscriptionRequest::
                    set_account_receivables_handling,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refundable_credits_handling",
                &app::model::subscription::CancelSubscriptionRequest::
                    get_refundable_credits_handling,
                &app::model::subscription::CancelSubscriptionRequest::
                    set_refundable_credits_handling,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cancel_reason_code",
                &app::model::subscription::CancelSubscriptionRequest::get_cancel_reason_code,
                &app::model::subscription::CancelSubscriptionRequest::set_cancel_reason_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "cancel_subscription_request";
    }
};

template<>
struct serde::Serializable<app::model::subscription::CancelSubscriptionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::subscription::CancelSubscriptionResponse::get_id,
                &app::model::subscription::CancelSubscriptionResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::subscription::CancelSubscriptionResponse::get_status,
                &app::model::subscription::CancelSubscriptionResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_reference_id",
                &app::model::subscription::CancelSubscriptionResponse::get_merchant_reference_id,
                &app::model::subscription::CancelSubscriptionResponse::set_merchant_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::subscription::CancelSubscriptionResponse::get_profile_id,
                &app::model::subscription::CancelSubscriptionResponse::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::subscription::CancelSubscriptionResponse::get_merchant_id,
                &app::model::subscription::CancelSubscriptionResponse::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::model::subscription::CancelSubscriptionResponse::get_customer_id,
                &app::model::subscription::CancelSubscriptionResponse::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cancelled_at",
                &app::model::subscription::CancelSubscriptionResponse::get_cancelled_at,
                &app::model::subscription::CancelSubscriptionResponse::set_cancelled_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "cancel_subscription_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::model::subscription::tests {
using namespace boost::ut;
suite<"app_model_subscription"> app_model_subscription_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<
                         app::model::subscription::CreateSubscriptionRequest>::fields())> > 0
        );
    };
};
} // namespace app::model::subscription::tests
#endif
