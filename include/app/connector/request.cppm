module;
export module app_connector:request;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::connector {

/** @brief Plain request describing one payment operation, no network or crypto involved. */
class ConnectorRequest
{
public:
    ConnectorRequest() = default;

    void set_operation(std::string value)
    {
        m_operation = std::move(value);
    }

    [[nodiscard]] const std::string& get_operation() const noexcept
    {
        return m_operation;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_amount(std::int64_t value) noexcept
    {
        m_amount = value;
    }

    [[nodiscard]] std::int64_t get_amount() const noexcept
    {
        return m_amount;
    }

    void set_currency(std::string value)
    {
        m_currency = std::move(value);
    }

    [[nodiscard]] const std::string& get_currency() const noexcept
    {
        return m_currency;
    }

    void set_payment_method_type(std::string value)
    {
        m_payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_method_type() const noexcept
    {
        return m_payment_method_type;
    }

    void set_resource_id(std::string value)
    {
        m_resource_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_resource_id() const noexcept
    {
        return m_resource_id;
    }

    void set_raw_payload(std::string value)
    {
        m_raw_payload = std::move(value);
    }

    [[nodiscard]] const std::string& get_raw_payload() const noexcept
    {
        return m_raw_payload;
    }

private:
    std::string m_operation{};
    std::string m_merchant_id{};
    std::int64_t m_amount{0};
    std::string m_currency{};
    std::string m_payment_method_type{};
    std::string m_resource_id{};
    std::string m_raw_payload{};
};

/** @brief Instance functor building a ConnectorRequest one field at a time. */
class ConnectorRequestBuilder
{
public:
    ConnectorRequestBuilder& operation(std::string value)
    {
        m_request.set_operation(std::move(value));
        return *this;
    }

    ConnectorRequestBuilder& merchant_id(std::string value)
    {
        m_request.set_merchant_id(std::move(value));
        return *this;
    }

    ConnectorRequestBuilder& amount(std::int64_t value) noexcept
    {
        m_request.set_amount(value);
        return *this;
    }

    ConnectorRequestBuilder& currency(std::string value)
    {
        m_request.set_currency(std::move(value));
        return *this;
    }

    ConnectorRequestBuilder& payment_method_type(std::string value)
    {
        m_request.set_payment_method_type(std::move(value));
        return *this;
    }

    ConnectorRequestBuilder& resource_id(std::string value)
    {
        m_request.set_resource_id(std::move(value));
        return *this;
    }

    ConnectorRequestBuilder& raw_payload(std::string value)
    {
        m_request.set_raw_payload(std::move(value));
        return *this;
    }

    [[nodiscard]] ConnectorRequest operator()() const
    {
        return m_request;
    }

private:
    ConnectorRequest m_request{};
};

} // namespace app::connector

#ifdef CONGELADO_TEST
namespace app::connector::tests {
using namespace boost::ut;

suite<"app_connector_request"> request_suite = []
{
    "request defaults to empty fields and zero amount"_test = []
    {
        ConnectorRequest request{};
        expect(request.get_operation().empty());
        expect(request.get_merchant_id().empty());
        expect(request.get_amount() == 0);
        expect(request.get_currency().empty());
        expect(request.get_payment_method_type().empty());
        expect(request.get_resource_id().empty());
        expect(request.get_raw_payload().empty());
    };

    "builder functor materializes every field in one call"_test = []
    {
        ConnectorRequest request = ConnectorRequestBuilder{}
                                       .operation{"authorize"}
                                       .merchant_id{"merchant_1"}
                                       .amount{4'250}
                                       .currency{"USD"}
                                       .payment_method_type{"card"}
                                       .resource_id{"payment_1"}
                                       .raw_payload{"{\"source\":\"card\"}"}();

        expect(request.get_operation() == "authorize");
        expect(request.get_merchant_id() == "merchant_1");
        expect(request.get_amount() == 4'250);
        expect(request.get_currency() == "USD");
        expect(request.get_payment_method_type() == "card");
        expect(request.get_resource_id() == "payment_1");
        expect(request.get_raw_payload() == "{\"source\":\"card\"}");
    };

    "builder instances are independent"_test = []
    {
        ConnectorRequestBuilder first{};
        first.operation{"capture"};
        ConnectorRequestBuilder second{};

        expect(first().get_operation() == "capture");
        expect(second().get_operation().empty());
    };
};

} // namespace app::connector::tests
#endif
