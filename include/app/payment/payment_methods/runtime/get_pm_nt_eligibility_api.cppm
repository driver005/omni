export module app_payment:payment_methods_runtime_get_pm_nt_eligibility_api;

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

import :payment_methods_input;
import :payment_methods_result;
import :payment_methods_runtime_common;

export namespace app::handlers::payment_methods {

class GetPmNtEligibilityApiRuntime
{
public:
    explicit GetPmNtEligibilityApiRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<
        ::app::model::payment_methods::GetNetworkTokenEiligibilityResponse,
        core::error::AppError>
    get_pm_nt_eligibility_api(const GetPmNtEligibilityApiInput& input) const
    {
        // Upstream core/payment_methods.rs:1777-1781 — NotSupported when network
        // tokenization is disabled for the profile. Local wire-name substitute.
        if (const auto nt_enabled = m_context.config_store().get_value(
                "payment_methods",
                "network_tokenization_enabled"
            );
            nt_enabled && (*nt_enabled == "false" || *nt_enabled == "disabled")) {
            return std::unexpected{m_support.local_error(
                "NOT_SUPPORTED",
                "Network tokenization is not enabled for this profile"
            )};
        }
        const auto connector_response =
            m_support.call_connector(input.org_id, "eligibility", input.card_bin, input.card_bin);
        m_context.metrics().record_flow("get_pm_nt_eligibility_api");
        m_context.metrics().counter("PAYMENT_METHOD_GET_PM_NT_ELIGIBILITY_API").increment();
        bool eligible = false;
        const std::string raw = connector_response.get_raw_json();
        if (const auto document = serde::Ser::decode_generic("application/json", raw)) {
            if (const auto* object = std::get_if<interfaces::Value::Object>(&document->get())) {
                for (const auto& [name, field]: *object) {
                    if (name == "tokenize_support") {
                        if (const auto* flag = std::get_if<bool>(&field.get())) {
                            eligible = *flag;
                        }
                        break;
                    }
                }
            }
        }
        if (raw.empty()) {
            eligible = !connector_response.get_status().empty();
        }
        ::app::model::payment_methods::GetNetworkTokenEiligibilityResponse response{};
        response.set_eligible_for_network_tokenization(eligible);
        return ::app::model::payment_methods::GetNetworkTokenEiligibilityResponse{
            std::move(response)
        };
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
