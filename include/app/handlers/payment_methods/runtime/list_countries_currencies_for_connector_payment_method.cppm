export module app_handlers_payment_methods:runtime_list_countries_currencies_for_connector_payment_method;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::payment_methods {

class ListCountriesCurrenciesForConnectorPaymentMethodRuntime
{
public:
    explicit ListCountriesCurrenciesForConnectorPaymentMethodRuntime(
        app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::
        expected<ListCountriesCurrenciesForConnectorPaymentMethodResult, core::error::AppError>
        list_countries_currencies_for_connector_payment_method(
            const ListCountriesCurrenciesForConnectorPaymentMethodInput& input
        ) const
    {
        // Upstream cards.rs:7801-7856 — pm_filters[connector][payment_method_type] with a
        // "default" fallback; when no filter is configured the full Currency / CountryAlpha2
        // enum sets are returned. The config store is the local substitute for
        // state.conf.pm_filters.
        std::string filters_json{};
        if (const auto value = m_context.config_store().get_value("pm_filters", input.connector);
            value && !value->empty()) {
            filters_json = *value;
        } else if (
            const auto value = m_context.config_store().get_value("pm_filters", "default");
            value && !value->empty()
        ) {
            filters_json = *value;
        }
        std::vector<std::string> currency_codes{};
        std::vector<std::pair<std::string, std::string>> country_entries{};
        bool configured{false};
        if (!filters_json.empty()) {
            if (const auto doc = serde::Ser::decode_generic("application/json", filters_json)) {
                const interfaces::Value::Object* by_type{nullptr};
                if (const auto* root = std::get_if<interfaces::Value::Object>(&doc->get())) {
                    for (const auto& [name, field]: *root) {
                        if (name == input.payment_method_type || name == "default") {
                            if (const auto* object =
                                    std::get_if<interfaces::Value::Object>(&field.get())) {
                                by_type = object;
                                if (name == input.payment_method_type) {
                                    break;
                                }
                            }
                        }
                    }
                }
                if (by_type != nullptr) {
                    for (const auto& [name, field]: *by_type) {
                        if (const auto* array =
                                std::get_if<interfaces::Value::Array>(&field.get())) {
                            for (const auto& element: *array) {
                                if (const auto* text = std::get_if<std::string>(&element.get())) {
                                    if (name == "currency") {
                                        currency_codes.push_back(*text);
                                    } else if (name == "country") {
                                        country_entries.emplace_back(*text, *text);
                                    }
                                } else if (
                                    const auto* object =
                                        std::get_if<interfaces::Value::Object>(&element.get())
                                ) {
                                    std::string code{};
                                    std::string name_value{};
                                    for (const auto& [key, value]: *object) {
                                        if (const auto* text =
                                                std::get_if<std::string>(&value.get())) {
                                            if (key == "code") {
                                                code = *text;
                                            } else if (key == "name") {
                                                name_value = *text;
                                            }
                                        }
                                    }
                                    if (!code.empty()) {
                                        country_entries.emplace_back(
                                            code,
                                            name_value.empty() ? code : name_value
                                        );
                                    }
                                }
                            }
                        }
                    }
                    configured = !currency_codes.empty() || !country_entries.empty();
                }
            }
        }
        m_context.metrics().record_flow("list_countries_currencies_for_connector_payment_method");
        m_context.metrics()
            .counter("PAYMENT_METHOD_LIST_COUNTRIES_CURRENCIES_FOR_CONNECTOR_PAYMENT_METHOD")
            .increment();
        app::model::payment_methods::ListCountriesCurrenciesResponse response{};
        if (configured) {
            std::vector<app::model::payment_methods::CurrencyValue> currencies{};
            for (const auto& code: currency_codes) {
                app::model::payment_methods::CurrencyValue value{};
                value.set_value(code);
                currencies.push_back(std::move(value));
            }
            std::vector<app::model::payment_methods::CountryCodeWithName> countries{};
            for (const auto& [code, name]: country_entries) {
                app::model::payment_methods::CountryAlpha2Value alpha{};
                alpha.set_value(code);
                app::model::payment_methods::CountryValue country{};
                country.set_value(name);
                app::model::payment_methods::CountryCodeWithName item{};
                item.set_code(std::move(alpha));
                item.set_name(std::move(country));
                countries.push_back(std::move(item));
            }
            response.set_currencies(std::move(currencies));
            response.set_countries(std::move(countries));
        } else {
            response.set_currencies(app::model::payment_methods::AllCurrencyCodes{}());
            response.set_countries(app::model::payment_methods::AllCountryCodes{}());
        }
        return ListCountriesCurrenciesForConnectorPaymentMethodResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
