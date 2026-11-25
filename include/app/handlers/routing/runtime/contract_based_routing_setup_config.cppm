export module app_handlers_routing:runtime_contract_based_routing_setup_config;

import std;
import core_error;
import core_serde;
import app_context;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::routing {

class ContractBasedRoutingSetupConfigRuntime
{
public:
    explicit ContractBasedRoutingSetupConfigRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ContractBasedRoutingSetupConfigResult, core::error::AppError>
    contract_based_routing_setup_config(const RoutingInput& input) const
    {
        const auto profile = input.profile_id;
        const auto feature = input.enable;
        if (feature == "none") {
            // Disable path (core:2772-2785): drop the contract algorithm ref.
            m_support.put_dynamic_ref(profile, R"({"contract_based_routing":null})");
            m_support.track("contract_based_routing_setup_config");
            return ContractBasedRoutingSetupConfigResult{
                ResponseDocument::empty()
            }; // StatusOk document
        }
        if (input.body.empty()) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Failed to get ContractBasedRoutingConfig from request"
            )};
        }
        if (!serde::Ser::decode_generic("application/json", input.body)) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")
            };
        }
        if (m_support.dynamic_ref_has_feature(profile, "contract_based_routing", feature)) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Contract Routing with specified features is already enabled"
            )};
        }
        if (const auto validation_error =
                m_support.profile_validation_error(input.auth_profile, profile)) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error)
            };
        }
        const auto algorithm = m_support.new_routing_id();
        auto record = m_support.config_record(
            input.merchant_id,
            algorithm,
            "Contract based dynamic routing algorithm",
            "",
            "contract_based",
            profile,
            input.body
        );
        record.set_algorithm_id(algorithm);
        m_support.upsert_config(std::move(record));
        m_support.put_dynamic_ref(
            profile,
            std::format(
                R"({{"contract_based_routing":{{"algorithm_id_with_timestamp":{{"algorithm_id":"{}","timestamp":{}}},"enabled_feature":"{}"}}}})",
                algorithm,
                m_support.now_unix(),
                feature
            )
        );
        m_support.track("contract_based_routing_setup_config");
        return ContractBasedRoutingSetupConfigResult{
            ResponseDocument::from(m_support.dictionary_record(
                algorithm,
                profile,
                "Contract based dynamic routing algorithm",
                "",
                "contract_based",
                "payment"
            ))
        };
    }

private:
    app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
