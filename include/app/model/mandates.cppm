module;
export module app_model:mandates;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::mandates {

class CardNetworkValue
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

class CardNumberValue
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

class CardSegmentTypeValue
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

class CurrencyValue
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

class CustomerAcceptanceValue
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

class FundingSourceValue
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

class MandateId
{
public:
    void set_mandate_id(std::string value)
    {
        mandate_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_mandate_id() const noexcept
    {
        return mandate_id;
    }

    std::string mandate_id{};
};

class MandateStatusValue
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

class MerchantConnectorAccountIdValue
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

class MinorUnitValue
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

class NetworkTokenValue
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

class PaymentIdValue
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

class ProcessorPaymentTokenDetails
{
public:
    void set_card_exp_month(std::optional<std::string> value)
    {
        card_exp_month = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_exp_month() const noexcept
    {
        return card_exp_month;
    }

    void set_card_exp_year(std::optional<std::string> value)
    {
        card_exp_year = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_exp_year() const noexcept
    {
        return card_exp_year;
    }

    void set_network_transaction_id(std::optional<std::string> value)
    {
        network_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_transaction_id() const noexcept
    {
        return network_transaction_id;
    }

    void set_transaction_link_id(std::optional<std::string> value)
    {
        transaction_link_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_transaction_link_id() const noexcept
    {
        return transaction_link_id;
    }

    std::optional<std::string> card_exp_month{};
    std::optional<std::string> card_exp_year{};
    std::optional<std::string> network_transaction_id{};
    std::optional<std::string> transaction_link_id{};
};

class ProfileIdValue
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

class CardWithLimitedData
{
public:
    void set_card_number(CardNumberValue value)
    {
        card_number = std::move(value);
    }

    [[nodiscard]] const CardNumberValue& get_card_number() const noexcept
    {
        return card_number;
    }

    void set_card_exp_month(std::optional<std::string> value)
    {
        card_exp_month = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_exp_month() const noexcept
    {
        return card_exp_month;
    }

    void set_card_exp_year(std::optional<std::string> value)
    {
        card_exp_year = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_exp_year() const noexcept
    {
        return card_exp_year;
    }

    void set_card_holder_name(std::optional<std::string> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_eci(std::optional<std::string> value)
    {
        eci = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_eci() const noexcept
    {
        return eci;
    }

    void set_network_transaction_id(std::optional<std::string> value)
    {
        network_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_network_transaction_id() const noexcept
    {
        return network_transaction_id;
    }

    void set_transaction_link_id(std::optional<std::string> value)
    {
        transaction_link_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_transaction_link_id() const noexcept
    {
        return transaction_link_id;
    }

    CardNumberValue card_number{};
    std::optional<std::string> card_exp_month{};
    std::optional<std::string> card_exp_year{};
    std::optional<std::string> card_holder_name{};
    std::optional<std::string> eci{};
    std::optional<std::string> network_transaction_id{};
    std::optional<std::string> transaction_link_id{};
};

class MandateCardDetails
{
public:
    void set_last4_digits(std::optional<std::string> value)
    {
        last4_digits = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_last4_digits() const noexcept
    {
        return last4_digits;
    }

    void set_card_exp_month(std::optional<std::string> value)
    {
        card_exp_month = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_exp_month() const noexcept
    {
        return card_exp_month;
    }

    void set_card_exp_year(std::optional<std::string> value)
    {
        card_exp_year = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_exp_year() const noexcept
    {
        return card_exp_year;
    }

    void set_card_holder_name(std::optional<std::string> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_card_token(std::optional<std::string> value)
    {
        card_token = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_token() const noexcept
    {
        return card_token;
    }

    void set_scheme(std::optional<std::string> value)
    {
        scheme = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_scheme() const noexcept
    {
        return scheme;
    }

    void set_issuer_country(std::optional<std::string> value)
    {
        issuer_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_country() const noexcept
    {
        return issuer_country;
    }

    void set_card_fingerprint(std::optional<std::string> value)
    {
        card_fingerprint = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_fingerprint() const noexcept
    {
        return card_fingerprint;
    }

    void set_card_isin(std::optional<std::string> value)
    {
        card_isin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_isin() const noexcept
    {
        return card_isin;
    }

    void set_card_issuer(std::optional<std::string> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept
    {
        return card_issuer;
    }

    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_type(std::optional<std::string> value)
    {
        card_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_type() const noexcept
    {
        return card_type;
    }

    void set_nick_name(std::optional<std::string> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nick_name() const noexcept
    {
        return nick_name;
    }

    std::optional<std::string> last4_digits{};
    std::optional<std::string> card_exp_month{};
    std::optional<std::string> card_exp_year{};
    std::optional<std::string> card_holder_name{};
    std::optional<std::string> card_token{};
    std::optional<std::string> scheme{};
    std::optional<std::string> issuer_country{};
    std::optional<std::string> card_fingerprint{};
    std::optional<std::string> card_isin{};
    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> nick_name{};
};

class MandateListConstraints
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

    void set_mandate_status(std::optional<MandateStatusValue> value)
    {
        mandate_status = std::move(value);
    }

    [[nodiscard]] const std::optional<MandateStatusValue>& get_mandate_status() const noexcept
    {
        return mandate_status;
    }

    void set_connector(std::optional<std::string> value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector() const noexcept
    {
        return connector;
    }

    void set_created_time(std::optional<PrimitiveDateTimeValue> value)
    {
        created_time = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created_time() const noexcept
    {
        return created_time;
    }

    void set_created_time_lt(std::optional<PrimitiveDateTimeValue> value)
    {
        created_time_lt = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created_time_lt() const noexcept
    {
        return created_time_lt;
    }

    void set_created_time_gt(std::optional<PrimitiveDateTimeValue> value)
    {
        created_time_gt = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created_time_gt() const noexcept
    {
        return created_time_gt;
    }

    void set_created_time_lte(std::optional<PrimitiveDateTimeValue> value)
    {
        created_time_lte = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created_time_lte() const noexcept
    {
        return created_time_lte;
    }

    void set_created_time_gte(std::optional<PrimitiveDateTimeValue> value)
    {
        created_time_gte = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_created_time_gte() const noexcept
    {
        return created_time_gte;
    }

    std::optional<std::int64_t> limit{};
    std::optional<std::int64_t> offset{};
    std::optional<MandateStatusValue> mandate_status{};
    std::optional<std::string> connector{};
    std::optional<PrimitiveDateTimeValue> created_time{};
    std::optional<PrimitiveDateTimeValue> created_time_lt{};
    std::optional<PrimitiveDateTimeValue> created_time_gt{};
    std::optional<PrimitiveDateTimeValue> created_time_lte{};
    std::optional<PrimitiveDateTimeValue> created_time_gte{};
};

class MandateRevokedResponse
{
public:
    void set_mandate_id(std::string value)
    {
        mandate_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_mandate_id() const noexcept
    {
        return mandate_id;
    }

    void set_status(MandateStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const MandateStatusValue& get_status() const noexcept
    {
        return status;
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

    std::string mandate_id{};
    MandateStatusValue status{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};

    // ── Exact v1 wire accessors (serde FieldDesc targets) ─────────────────────────
    // The upstream DTO (api_models/mandates.rs:17-33) serialises the newtype status
    // as a bare scalar and always emits error_code/error_message (Option -> null).
    // reflect-cpp would otherwise encode the status wrapper as {"value":...} and
    // drop the empty Options; these accessors keep the typed fields above while
    // giving the DTO an exact, serde-supported upstream wire shape.
    [[nodiscard]] const std::string& wire_status() const noexcept
    {
        return status.get_value();
    }

    [[nodiscard]] serde::RawJson wire_error_code() const
    {
        return serde::RawJson{error_code ? serde_quote(*error_code) : "null"};
    }

    [[nodiscard]] serde::RawJson wire_error_message() const
    {
        return serde::RawJson{error_message ? serde_quote(*error_message) : "null"};
    }

    void set_wire_status(std::string) {}

    void set_wire_error_code(serde::RawJson) {}

    void set_wire_error_message(serde::RawJson) {}

    /** @brief Minimal JSON string quoting for the (always-null on this route) error fields. */
    [[nodiscard]] static std::string serde_quote(std::string_view value)
    {
        std::string out{"\""};
        for (const char c: value) {
            if (c == '"' || c == '\\') {
                out.push_back('\\');
            }
            out.push_back(c);
        }
        out.push_back('"');
        return out;
    }
};

class NetworkTokenDetails
{
public:
    void set_network_token(NetworkTokenValue value)
    {
        network_token = std::move(value);
    }

    [[nodiscard]] const NetworkTokenValue& get_network_token() const noexcept
    {
        return network_token;
    }

    void set_token_exp_month(std::string value)
    {
        token_exp_month = std::move(value);
    }

    [[nodiscard]] const std::string& get_token_exp_month() const noexcept
    {
        return token_exp_month;
    }

    void set_token_exp_year(std::string value)
    {
        token_exp_year = std::move(value);
    }

    [[nodiscard]] const std::string& get_token_exp_year() const noexcept
    {
        return token_exp_year;
    }

    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_type(std::optional<std::string> value)
    {
        card_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_type() const noexcept
    {
        return card_type;
    }

    void set_card_issuing_country(std::optional<std::string> value)
    {
        card_issuing_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country() const noexcept
    {
        return card_issuing_country;
    }

    void set_bank_code(std::optional<std::string> value)
    {
        bank_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_code() const noexcept
    {
        return bank_code;
    }

    void set_card_holder_name(std::optional<std::string> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_card_issuer(std::optional<std::string> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept
    {
        return card_issuer;
    }

    void set_nick_name(std::optional<std::string> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nick_name() const noexcept
    {
        return nick_name;
    }

    void set_eci(std::optional<std::string> value)
    {
        eci = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_eci() const noexcept
    {
        return eci;
    }

    NetworkTokenValue network_token{};
    std::string token_exp_month{};
    std::string token_exp_year{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> bank_code{};
    std::optional<std::string> card_holder_name{};
    std::optional<std::string> card_issuer{};
    std::optional<std::string> nick_name{};
    std::optional<std::string> eci{};
};

class NetworkTransactionIdAndCardDetails
{
public:
    void set_card_number(CardNumberValue value)
    {
        card_number = std::move(value);
    }

    [[nodiscard]] const CardNumberValue& get_card_number() const noexcept
    {
        return card_number;
    }

    void set_card_exp_month(std::string value)
    {
        card_exp_month = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_exp_month() const noexcept
    {
        return card_exp_month;
    }

    void set_card_exp_year(std::string value)
    {
        card_exp_year = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_exp_year() const noexcept
    {
        return card_exp_year;
    }

    void set_card_holder_name(std::optional<std::string> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_card_issuer(std::optional<std::string> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept
    {
        return card_issuer;
    }

    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_type(std::optional<std::string> value)
    {
        card_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_type() const noexcept
    {
        return card_type;
    }

    void set_card_subtype(std::optional<std::string> value)
    {
        card_subtype = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_subtype() const noexcept
    {
        return card_subtype;
    }

    void set_card_segment_type(std::optional<CardSegmentTypeValue> value)
    {
        card_segment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<CardSegmentTypeValue>& get_card_segment_type() const noexcept
    {
        return card_segment_type;
    }

    void set_funding_source(std::optional<FundingSourceValue> value)
    {
        funding_source = std::move(value);
    }

    [[nodiscard]] const std::optional<FundingSourceValue>& get_funding_source() const noexcept
    {
        return funding_source;
    }

    void set_card_issuing_country(std::optional<std::string> value)
    {
        card_issuing_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country() const noexcept
    {
        return card_issuing_country;
    }

    void set_card_issuing_country_code(std::optional<std::string> value)
    {
        card_issuing_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country_code() const noexcept
    {
        return card_issuing_country_code;
    }

    void set_bank_code(std::optional<std::string> value)
    {
        bank_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_code() const noexcept
    {
        return bank_code;
    }

    void set_nick_name(std::optional<std::string> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nick_name() const noexcept
    {
        return nick_name;
    }

    void set_network_transaction_id(std::string value)
    {
        network_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_network_transaction_id() const noexcept
    {
        return network_transaction_id;
    }

    void set_transaction_link_id(std::optional<std::string> value)
    {
        transaction_link_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_transaction_link_id() const noexcept
    {
        return transaction_link_id;
    }

    CardNumberValue card_number{};
    std::string card_exp_month{};
    std::string card_exp_year{};
    std::optional<std::string> card_holder_name{};
    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_subtype{};
    std::optional<CardSegmentTypeValue> card_segment_type{};
    std::optional<FundingSourceValue> funding_source{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> card_issuing_country_code{};
    std::optional<std::string> bank_code{};
    std::optional<std::string> nick_name{};
    std::string network_transaction_id{};
    std::optional<std::string> transaction_link_id{};
};

class NetworkTransactionIdAndNetworkTokenDetails
{
public:
    void set_network_token(NetworkTokenValue value)
    {
        network_token = std::move(value);
    }

    [[nodiscard]] const NetworkTokenValue& get_network_token() const noexcept
    {
        return network_token;
    }

    void set_token_exp_month(std::string value)
    {
        token_exp_month = std::move(value);
    }

    [[nodiscard]] const std::string& get_token_exp_month() const noexcept
    {
        return token_exp_month;
    }

    void set_token_exp_year(std::string value)
    {
        token_exp_year = std::move(value);
    }

    [[nodiscard]] const std::string& get_token_exp_year() const noexcept
    {
        return token_exp_year;
    }

    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_type(std::optional<std::string> value)
    {
        card_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_type() const noexcept
    {
        return card_type;
    }

    void set_card_issuing_country(std::optional<std::string> value)
    {
        card_issuing_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country() const noexcept
    {
        return card_issuing_country;
    }

    void set_bank_code(std::optional<std::string> value)
    {
        bank_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_code() const noexcept
    {
        return bank_code;
    }

    void set_card_holder_name(std::string value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_card_issuer(std::optional<std::string> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept
    {
        return card_issuer;
    }

    void set_nick_name(std::optional<std::string> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nick_name() const noexcept
    {
        return nick_name;
    }

    void set_eci(std::optional<std::string> value)
    {
        eci = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_eci() const noexcept
    {
        return eci;
    }

    void set_network_transaction_id(std::string value)
    {
        network_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_network_transaction_id() const noexcept
    {
        return network_transaction_id;
    }

    void set_transaction_link_id(std::optional<std::string> value)
    {
        transaction_link_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_transaction_link_id() const noexcept
    {
        return transaction_link_id;
    }

    NetworkTokenValue network_token{};
    std::string token_exp_month{};
    std::string token_exp_year{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> bank_code{};
    std::string card_holder_name{};
    std::optional<std::string> card_issuer{};
    std::optional<std::string> nick_name{};
    std::optional<std::string> eci{};
    std::string network_transaction_id{};
    std::optional<std::string> transaction_link_id{};
};

class ProcessorPaymentToken
{
public:
    void set_processor_payment_token(std::string value)
    {
        processor_payment_token = std::move(value);
    }

    [[nodiscard]] const std::string& get_processor_payment_token() const noexcept
    {
        return processor_payment_token;
    }

    void set_merchant_connector_id(std::optional<MerchantConnectorAccountIdValue> value)
    {
        merchant_connector_id = std::move(value);
    }

    [[nodiscard]] const std::optional<MerchantConnectorAccountIdValue>&
    get_merchant_connector_id() const noexcept
    {
        return merchant_connector_id;
    }

    void set_processor_payment_token_details(std::optional<ProcessorPaymentTokenDetails> value)
    {
        processor_payment_token_details = std::move(value);
    }

    [[nodiscard]] const std::optional<ProcessorPaymentTokenDetails>&
    get_processor_payment_token_details() const noexcept
    {
        return processor_payment_token_details;
    }

    std::string processor_payment_token{};
    std::optional<MerchantConnectorAccountIdValue> merchant_connector_id{};
    std::optional<ProcessorPaymentTokenDetails> processor_payment_token_details{};
};

class VaultCardData
{
public:
    void set_card_number(std::string value)
    {
        card_number = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_number() const noexcept
    {
        return card_number;
    }

    void set_card_exp_month(std::string value)
    {
        card_exp_month = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_exp_month() const noexcept
    {
        return card_exp_month;
    }

    void set_card_exp_year(std::string value)
    {
        card_exp_year = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_exp_year() const noexcept
    {
        return card_exp_year;
    }

    void set_card_holder_name(std::optional<std::string> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_card_issuer(std::optional<std::string> value)
    {
        card_issuer = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept
    {
        return card_issuer;
    }

    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_type(std::optional<std::string> value)
    {
        card_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_type() const noexcept
    {
        return card_type;
    }

    void set_card_issuing_country(std::optional<std::string> value)
    {
        card_issuing_country = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country() const noexcept
    {
        return card_issuing_country;
    }

    void set_bank_code(std::optional<std::string> value)
    {
        bank_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_code() const noexcept
    {
        return bank_code;
    }

    void set_nick_name(std::optional<std::string> value)
    {
        nick_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_nick_name() const noexcept
    {
        return nick_name;
    }

    void set_bin_number(std::optional<std::string> value)
    {
        bin_number = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bin_number() const noexcept
    {
        return bin_number;
    }

    void set_last_four(std::optional<std::string> value)
    {
        last_four = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_last_four() const noexcept
    {
        return last_four;
    }

    std::string card_number{};
    std::string card_exp_month{};
    std::string card_exp_year{};
    std::optional<std::string> card_holder_name{};
    std::optional<std::string> card_issuer{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_type{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::string> bank_code{};
    std::optional<std::string> nick_name{};
    std::optional<std::string> bin_number{};
    std::optional<std::string> last_four{};
};

class MandateResponse
{
public:
    void set_mandate_id(std::string value)
    {
        mandate_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_mandate_id() const noexcept
    {
        return mandate_id;
    }

    void set_payment_id(std::optional<PaymentIdValue> value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const std::optional<PaymentIdValue>& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_amount(std::optional<MinorUnitValue> value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_amount() const noexcept
    {
        return amount;
    }

    void set_currency(std::optional<CurrencyValue> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::optional<CurrencyValue>& get_currency() const noexcept
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

    void set_mandate_metadata(std::optional<SecretSerdeValueValue> value)
    {
        mandate_metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretSerdeValueValue>& get_mandate_metadata() const noexcept
    {
        return mandate_metadata;
    }

    void set_status(MandateStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const MandateStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_payment_method_id(std::string value)
    {
        payment_method_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_method_id() const noexcept
    {
        return payment_method_id;
    }

    void set_payment_method(std::string value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_type(std::optional<std::string> value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_card(std::optional<MandateCardDetails> value)
    {
        card = std::move(value);
    }

    [[nodiscard]] const std::optional<MandateCardDetails>& get_card() const noexcept
    {
        return card;
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

    std::string mandate_id{};
    std::optional<PaymentIdValue> payment_id{};
    std::optional<MinorUnitValue> amount{};
    std::optional<CurrencyValue> currency{};
    std::optional<ProfileIdValue> profile_id{};
    std::optional<SecretSerdeValueValue> mandate_metadata{};
    MandateStatusValue status{};
    std::string payment_method_id{};
    std::string payment_method{};
    std::optional<std::string> payment_method_type{};
    std::optional<MandateCardDetails> card{};
    std::optional<CustomerAcceptanceValue> customer_acceptance{};

    // ── Exact v1 wire accessors (serde FieldDesc targets) ─────────────────────────
    // `from_db_mandate` (router/src/types/api/mandates.rs:39-135) maps newtype
    // wrappers to bare scalars, omits the always-None amount/currency/profile_id/
    // mandate_metadata and the absent payment_id, and always emits card + the
    // customer_acceptance object. reflect-cpp would encode the wrappers as
    // {"value":...}; these accessors give the typed DTO an exact wire shape while
    // serde's empty-optional omission covers the upstream skip_serializing_if.
    [[nodiscard]] std::optional<std::string> wire_payment_id() const
    {
        return payment_id ? std::optional<std::string>{payment_id->get_value()} : std::nullopt;
    }

    [[nodiscard]] const std::string& wire_status() const noexcept
    {
        return status.get_value();
    }

    [[nodiscard]] serde::RawJson wire_card() const
    {
        return serde::RawJson{card_wire};
    }

    [[nodiscard]] serde::RawJson wire_customer_acceptance() const
    {
        return serde::RawJson{acceptance_wire};
    }

    void set_wire_payment_id(std::optional<std::string>) {}

    void set_wire_status(std::string) {}

    void set_wire_card(serde::RawJson) {}

    void set_wire_customer_acceptance(serde::RawJson) {}

    void set_card_wire(std::string value)
    {
        card_wire = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_wire() const noexcept
    {
        return card_wire;
    }

    void set_acceptance_wire(std::string value)
    {
        acceptance_wire = std::move(value);
    }

    [[nodiscard]] const std::string& get_acceptance_wire() const noexcept
    {
        return acceptance_wire;
    }

    // Pre-rendered upstream JSON fragments for the two always-emitted object fields.
    std::string card_wire{"null"};
    std::string acceptance_wire{"{\"acceptance_type\":\"offline\",\"accepted_at\":null,"
                                "\"online\":{\"ip_address\":null,\"user_agent\":\"\"}}"};
};

class NetworkTokenWithVaultCardFallback
{
public:
    void set_network_token(NetworkTokenDetails value)
    {
        network_token = std::move(value);
    }

    [[nodiscard]] const NetworkTokenDetails& get_network_token() const noexcept
    {
        return network_token;
    }

    void set_network_transaction_id(std::string value)
    {
        network_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_network_transaction_id() const noexcept
    {
        return network_transaction_id;
    }

    void set_transaction_link_id(std::optional<std::string> value)
    {
        transaction_link_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_transaction_link_id() const noexcept
    {
        return transaction_link_id;
    }

    void set_vault_card_data(VaultCardData value)
    {
        vault_card_data = std::move(value);
    }

    [[nodiscard]] const VaultCardData& get_vault_card_data() const noexcept
    {
        return vault_card_data;
    }

    NetworkTokenDetails network_token{};
    std::string network_transaction_id{};
    std::optional<std::string> transaction_link_id{};
    VaultCardData vault_card_data{};
};

class NetworkTransactionIdAndVaultCardDetails
{
public:
    void set_vault_card_data(VaultCardData value)
    {
        vault_card_data = std::move(value);
    }

    [[nodiscard]] const VaultCardData& get_vault_card_data() const noexcept
    {
        return vault_card_data;
    }

    void set_network_transaction_id(std::string value)
    {
        network_transaction_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_network_transaction_id() const noexcept
    {
        return network_transaction_id;
    }

    void set_transaction_link_id(std::optional<std::string> value)
    {
        transaction_link_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_transaction_link_id() const noexcept
    {
        return transaction_link_id;
    }

    VaultCardData vault_card_data{};
    std::string network_transaction_id{};
    std::optional<std::string> transaction_link_id{};
};

enum class RecurringDetails
{
    MandateId,
    PaymentMethodId,
    ProcessorPaymentToken,
    NetworkTransactionIdAndCardDetails,
    NetworkTransactionIdAndNetworkTokenDetails,
    NetworkTransactionIdAndDecryptedWalletTokenDetails,
    NetworkTransactionIdAndVaultCardDetails,
    CardWithLimitedData,
    NetworkTokenWithVaultCardFallback,
};
} // namespace app::model::mandates

template<>
struct serde::Serializable<app::model::mandates::MandateId>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "mandate_id",
            &app::model::mandates::MandateId::get_mandate_id,
            &app::model::mandates::MandateId::set_mandate_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "mandate_id";
    }
};

template<>
struct serde::Serializable<app::model::mandates::MandateRevokedResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "mandate_id",
                &app::model::mandates::MandateRevokedResponse::get_mandate_id,
                &app::model::mandates::MandateRevokedResponse::set_mandate_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::mandates::MandateRevokedResponse::wire_status,
                &app::model::mandates::MandateRevokedResponse::set_wire_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &app::model::mandates::MandateRevokedResponse::wire_error_code,
                &app::model::mandates::MandateRevokedResponse::set_wire_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::mandates::MandateRevokedResponse::wire_error_message,
                &app::model::mandates::MandateRevokedResponse::set_wire_error_message,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "mandate_revoked_response";
    }
};

template<>
struct serde::Serializable<app::model::mandates::MandateCardDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "last4_digits",
                &app::model::mandates::MandateCardDetails::get_last4_digits,
                &app::model::mandates::MandateCardDetails::set_last4_digits,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::mandates::MandateCardDetails::get_card_exp_month,
                &app::model::mandates::MandateCardDetails::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::mandates::MandateCardDetails::get_card_exp_year,
                &app::model::mandates::MandateCardDetails::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::mandates::MandateCardDetails::get_card_holder_name,
                &app::model::mandates::MandateCardDetails::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_token",
                &app::model::mandates::MandateCardDetails::get_card_token,
                &app::model::mandates::MandateCardDetails::set_card_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "scheme",
                &app::model::mandates::MandateCardDetails::get_scheme,
                &app::model::mandates::MandateCardDetails::set_scheme,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_country",
                &app::model::mandates::MandateCardDetails::get_issuer_country,
                &app::model::mandates::MandateCardDetails::set_issuer_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_fingerprint",
                &app::model::mandates::MandateCardDetails::get_card_fingerprint,
                &app::model::mandates::MandateCardDetails::set_card_fingerprint,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_isin",
                &app::model::mandates::MandateCardDetails::get_card_isin,
                &app::model::mandates::MandateCardDetails::set_card_isin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::mandates::MandateCardDetails::get_card_issuer,
                &app::model::mandates::MandateCardDetails::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::mandates::MandateCardDetails::get_card_network,
                &app::model::mandates::MandateCardDetails::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::mandates::MandateCardDetails::get_card_type,
                &app::model::mandates::MandateCardDetails::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::mandates::MandateCardDetails::get_nick_name,
                &app::model::mandates::MandateCardDetails::set_nick_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "mandate_card_details";
    }
};

template<>
struct serde::Serializable<app::model::mandates::MandateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "mandate_id",
                &app::model::mandates::MandateResponse::get_mandate_id,
                &app::model::mandates::MandateResponse::set_mandate_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_id",
                &app::model::mandates::MandateResponse::wire_payment_id,
                &app::model::mandates::MandateResponse::set_wire_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::mandates::MandateResponse::get_amount,
                &app::model::mandates::MandateResponse::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::mandates::MandateResponse::get_currency,
                &app::model::mandates::MandateResponse::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::mandates::MandateResponse::get_profile_id,
                &app::model::mandates::MandateResponse::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mandate_metadata",
                &app::model::mandates::MandateResponse::get_mandate_metadata,
                &app::model::mandates::MandateResponse::set_mandate_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::mandates::MandateResponse::wire_status,
                &app::model::mandates::MandateResponse::set_wire_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_id",
                &app::model::mandates::MandateResponse::get_payment_method_id,
                &app::model::mandates::MandateResponse::set_payment_method_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::mandates::MandateResponse::get_payment_method,
                &app::model::mandates::MandateResponse::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::model::mandates::MandateResponse::get_payment_method_type,
                &app::model::mandates::MandateResponse::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card",
                &app::model::mandates::MandateResponse::wire_card,
                &app::model::mandates::MandateResponse::set_wire_card,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_acceptance",
                &app::model::mandates::MandateResponse::wire_customer_acceptance,
                &app::model::mandates::MandateResponse::set_wire_customer_acceptance,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "mandate_response";
    }
};

template<>
struct serde::Serializable<app::model::mandates::MandateListConstraints>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "limit",
                &app::model::mandates::MandateListConstraints::get_limit,
                &app::model::mandates::MandateListConstraints::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::mandates::MandateListConstraints::get_offset,
                &app::model::mandates::MandateListConstraints::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mandate_status",
                &app::model::mandates::MandateListConstraints::get_mandate_status,
                &app::model::mandates::MandateListConstraints::set_mandate_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::mandates::MandateListConstraints::get_connector,
                &app::model::mandates::MandateListConstraints::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_time",
                &app::model::mandates::MandateListConstraints::get_created_time,
                &app::model::mandates::MandateListConstraints::set_created_time,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_time.lt",
                &app::model::mandates::MandateListConstraints::get_created_time_lt,
                &app::model::mandates::MandateListConstraints::set_created_time_lt,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_time.gt",
                &app::model::mandates::MandateListConstraints::get_created_time_gt,
                &app::model::mandates::MandateListConstraints::set_created_time_gt,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_time.lte",
                &app::model::mandates::MandateListConstraints::get_created_time_lte,
                &app::model::mandates::MandateListConstraints::set_created_time_lte,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_time.gte",
                &app::model::mandates::MandateListConstraints::get_created_time_gte,
                &app::model::mandates::MandateListConstraints::set_created_time_gte,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "mandate_list_constraints";
    }
};

template<>
struct serde::Serializable<app::model::mandates::ProcessorPaymentTokenDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_exp_month",
                &app::model::mandates::ProcessorPaymentTokenDetails::get_card_exp_month,
                &app::model::mandates::ProcessorPaymentTokenDetails::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::mandates::ProcessorPaymentTokenDetails::get_card_exp_year,
                &app::model::mandates::ProcessorPaymentTokenDetails::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id",
                &app::model::mandates::ProcessorPaymentTokenDetails::get_network_transaction_id,
                &app::model::mandates::ProcessorPaymentTokenDetails::set_network_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_link_id",
                &app::model::mandates::ProcessorPaymentTokenDetails::get_transaction_link_id,
                &app::model::mandates::ProcessorPaymentTokenDetails::set_transaction_link_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "processor_payment_token_details";
    }
};

template<>
struct serde::Serializable<app::model::mandates::ProcessorPaymentToken>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "processor_payment_token",
                &app::model::mandates::ProcessorPaymentToken::get_processor_payment_token,
                &app::model::mandates::ProcessorPaymentToken::set_processor_payment_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_id",
                &app::model::mandates::ProcessorPaymentToken::get_merchant_connector_id,
                &app::model::mandates::ProcessorPaymentToken::set_merchant_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "processor_payment_token_details",
                &app::model::mandates::ProcessorPaymentToken::get_processor_payment_token_details,
                &app::model::mandates::ProcessorPaymentToken::set_processor_payment_token_details,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "processor_payment_token";
    }
};

template<>
struct serde::Serializable<app::model::mandates::NetworkTransactionIdAndCardDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_number",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_number,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_exp_month,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_exp_year,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_holder_name,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_issuer,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_network,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_type,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_subtype,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_segment_type",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_segment_type,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_segment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "funding_source",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_funding_source,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_funding_source,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_card_issuing_country,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country_code",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::
                    get_card_issuing_country_code,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::
                    set_card_issuing_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_bank_code,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_nick_name,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::
                    get_network_transaction_id,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::
                    set_network_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_link_id",
                &app::model::mandates::NetworkTransactionIdAndCardDetails::get_transaction_link_id,
                &app::model::mandates::NetworkTransactionIdAndCardDetails::set_transaction_link_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "network_transaction_id_and_card_details";
    }
};

template<>
struct serde::Serializable<app::model::mandates::CardWithLimitedData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_number",
                &app::model::mandates::CardWithLimitedData::get_card_number,
                &app::model::mandates::CardWithLimitedData::set_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::mandates::CardWithLimitedData::get_card_exp_month,
                &app::model::mandates::CardWithLimitedData::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::mandates::CardWithLimitedData::get_card_exp_year,
                &app::model::mandates::CardWithLimitedData::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::mandates::CardWithLimitedData::get_card_holder_name,
                &app::model::mandates::CardWithLimitedData::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "eci",
                &app::model::mandates::CardWithLimitedData::get_eci,
                &app::model::mandates::CardWithLimitedData::set_eci,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id",
                &app::model::mandates::CardWithLimitedData::get_network_transaction_id,
                &app::model::mandates::CardWithLimitedData::set_network_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_link_id",
                &app::model::mandates::CardWithLimitedData::get_transaction_link_id,
                &app::model::mandates::CardWithLimitedData::set_transaction_link_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_with_limited_data";
    }
};

template<>
struct serde::Serializable<app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "network_token",
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::
                    get_network_token,
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::
                    set_network_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_exp_month",
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::
                    get_token_exp_month,
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::
                    set_token_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_exp_year",
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::
                    get_token_exp_year,
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::
                    set_token_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::get_card_network,
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::get_card_type,
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::
                    get_card_issuing_country,
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::
                    set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::get_bank_code,
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::
                    get_card_holder_name,
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::
                    set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::get_card_issuer,
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::get_nick_name,
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "eci",
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::get_eci,
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::set_eci,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id",
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::
                    get_network_transaction_id,
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::
                    set_network_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_link_id",
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::
                    get_transaction_link_id,
                &app::model::mandates::NetworkTransactionIdAndNetworkTokenDetails::
                    set_transaction_link_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "network_transaction_id_and_network_token_details";
    }
};

template<>
struct serde::Serializable<app::model::mandates::NetworkTokenDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "network_token",
                &app::model::mandates::NetworkTokenDetails::get_network_token,
                &app::model::mandates::NetworkTokenDetails::set_network_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_exp_month",
                &app::model::mandates::NetworkTokenDetails::get_token_exp_month,
                &app::model::mandates::NetworkTokenDetails::set_token_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "token_exp_year",
                &app::model::mandates::NetworkTokenDetails::get_token_exp_year,
                &app::model::mandates::NetworkTokenDetails::set_token_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::mandates::NetworkTokenDetails::get_card_network,
                &app::model::mandates::NetworkTokenDetails::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::mandates::NetworkTokenDetails::get_card_type,
                &app::model::mandates::NetworkTokenDetails::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::mandates::NetworkTokenDetails::get_card_issuing_country,
                &app::model::mandates::NetworkTokenDetails::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::mandates::NetworkTokenDetails::get_bank_code,
                &app::model::mandates::NetworkTokenDetails::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::mandates::NetworkTokenDetails::get_card_holder_name,
                &app::model::mandates::NetworkTokenDetails::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::mandates::NetworkTokenDetails::get_card_issuer,
                &app::model::mandates::NetworkTokenDetails::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::mandates::NetworkTokenDetails::get_nick_name,
                &app::model::mandates::NetworkTokenDetails::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "eci",
                &app::model::mandates::NetworkTokenDetails::get_eci,
                &app::model::mandates::NetworkTokenDetails::set_eci,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "network_token_details";
    }
};

template<>
struct serde::Serializable<app::model::mandates::VaultCardData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_number",
                &app::model::mandates::VaultCardData::get_card_number,
                &app::model::mandates::VaultCardData::set_card_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::mandates::VaultCardData::get_card_exp_month,
                &app::model::mandates::VaultCardData::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::mandates::VaultCardData::get_card_exp_year,
                &app::model::mandates::VaultCardData::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::mandates::VaultCardData::get_card_holder_name,
                &app::model::mandates::VaultCardData::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &app::model::mandates::VaultCardData::get_card_issuer,
                &app::model::mandates::VaultCardData::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::mandates::VaultCardData::get_card_network,
                &app::model::mandates::VaultCardData::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::mandates::VaultCardData::get_card_type,
                &app::model::mandates::VaultCardData::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuing_country",
                &app::model::mandates::VaultCardData::get_card_issuing_country,
                &app::model::mandates::VaultCardData::set_card_issuing_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::mandates::VaultCardData::get_bank_code,
                &app::model::mandates::VaultCardData::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "nick_name",
                &app::model::mandates::VaultCardData::get_nick_name,
                &app::model::mandates::VaultCardData::set_nick_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bin_number",
                &app::model::mandates::VaultCardData::get_bin_number,
                &app::model::mandates::VaultCardData::set_bin_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_four",
                &app::model::mandates::VaultCardData::get_last_four,
                &app::model::mandates::VaultCardData::set_last_four,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "vault_card_data";
    }
};

template<>
struct serde::Serializable<app::model::mandates::NetworkTransactionIdAndVaultCardDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "vault_card_data",
                &app::model::mandates::NetworkTransactionIdAndVaultCardDetails::get_vault_card_data,
                &app::model::mandates::NetworkTransactionIdAndVaultCardDetails::set_vault_card_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id",
                &app::model::mandates::NetworkTransactionIdAndVaultCardDetails::
                    get_network_transaction_id,
                &app::model::mandates::NetworkTransactionIdAndVaultCardDetails::
                    set_network_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_link_id",
                &app::model::mandates::NetworkTransactionIdAndVaultCardDetails::
                    get_transaction_link_id,
                &app::model::mandates::NetworkTransactionIdAndVaultCardDetails::
                    set_transaction_link_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "network_transaction_id_and_vault_card_details";
    }
};

template<>
struct serde::Serializable<app::model::mandates::NetworkTokenWithVaultCardFallback>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "network_token",
                &app::model::mandates::NetworkTokenWithVaultCardFallback::get_network_token,
                &app::model::mandates::NetworkTokenWithVaultCardFallback::set_network_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_transaction_id",
                &app::model::mandates::NetworkTokenWithVaultCardFallback::
                    get_network_transaction_id,
                &app::model::mandates::NetworkTokenWithVaultCardFallback::
                    set_network_transaction_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_link_id",
                &app::model::mandates::NetworkTokenWithVaultCardFallback::get_transaction_link_id,
                &app::model::mandates::NetworkTokenWithVaultCardFallback::set_transaction_link_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "vault_card_data",
                &app::model::mandates::NetworkTokenWithVaultCardFallback::get_vault_card_data,
                &app::model::mandates::NetworkTokenWithVaultCardFallback::set_vault_card_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "network_token_with_vault_card_fallback";
    }
};

#ifdef CONGELADO_TEST
namespace app::model::mandates::tests {
using namespace boost::ut;
suite<"app_model_mandates"> app_model_mandates_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<app::model::mandates::MandateId>::fields())> > 0
        );
    };
    "typed mandate DTOs encode the exact upstream v1 wire shape"_test = []
    {
        using namespace app::model::mandates;
        MandateRevokedResponse revoked{};
        revoked.set_mandate_id("man_1");
        MandateStatusValue status{};
        status.set_value("revoked");
        revoked.set_status(std::move(status));
        expect(
            serde::Ser::encode_json(revoked) ==
            R"({"mandate_id":"man_1","status":"revoked","error_code":null,"error_message":null})"
        );

        MandateResponse full{};
        full.set_mandate_id("man_2");
        PaymentIdValue pid{};
        pid.set_value("pay_1");
        full.set_payment_id(std::move(pid));
        MandateStatusValue active{};
        active.set_value("active");
        full.set_status(std::move(active));
        full.set_payment_method_id("pm_1");
        full.set_payment_method("card");
        expect(
            serde::Ser::encode_json(full) ==
            R"({"mandate_id":"man_2","payment_id":"pay_1","status":"active",)"
            R"("payment_method_id":"pm_1","payment_method":"card","card":null,)"
            R"("customer_acceptance":{"acceptance_type":"offline","accepted_at":null,)"
            R"("online":{"ip_address":null,"user_agent":""}}}})"
        );
    };
};
} // namespace app::model::mandates::tests
#endif
