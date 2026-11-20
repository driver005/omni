module;
export module app_dto:verify_connector;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::verify_connector {

class ConnectorAuthTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ConnectorValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class VerifyConnectorRequest
{
public:
    void set_connector_name(ConnectorValue value) { connector_name = std::move(value); }
    [[nodiscard]] const ConnectorValue& get_connector_name() const noexcept { return connector_name; }
    void set_connector_account_details(ConnectorAuthTypeValue value) { connector_account_details = std::move(value); }
    [[nodiscard]] const ConnectorAuthTypeValue& get_connector_account_details() const noexcept { return connector_account_details; }
    ConnectorValue connector_name{};
    ConnectorAuthTypeValue connector_account_details{};
};
} // namespace app::dto::verify_connector

template<>
struct serde::Serializable<app::dto::verify_connector::VerifyConnectorRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector_name", &app::dto::verify_connector::VerifyConnectorRequest::get_connector_name, &app::dto::verify_connector::VerifyConnectorRequest::set_connector_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_account_details", &app::dto::verify_connector::VerifyConnectorRequest::get_connector_account_details, &app::dto::verify_connector::VerifyConnectorRequest::set_connector_account_details, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "verify_connector_request"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::verify_connector::tests {
using namespace boost::ut;
suite<"app_dto_verify_connector"> app_dto_verify_connector_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::verify_connector::VerifyConnectorRequest>::fields())> > 0);
    };
};
}
#endif
