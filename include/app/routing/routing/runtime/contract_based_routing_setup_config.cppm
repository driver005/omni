export module app_routing:routing_runtime_contract_based_routing_setup_config;

import std;
import core_error;
import core_serde;
import app_context;
import core_behavior;

import :routing_result;
import :routing_runtime_common;

export namespace app::handlers::routing {

class ContractBasedRoutingSetupConfigRuntime
{
public:
    explicit ContractBasedRoutingSetupConfigRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> contract_based_routing_setup_config(
        const std::optional<serde::Value>& body,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        const auto profile = ctx.profile_id;
        const auto feature = ctx_enable(ctx);
        if (feature == "none") {
            // Disable path (core:2772-2785): drop the contract algorithm ref.
            m_support.put_dynamic_ref(profile, R"({"contract_based_routing":null})");
            m_support.track("contract_based_routing_setup_config");
            return ResponseDocument::empty(); // StatusOk document
        }
        if (!body) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Failed to get ContractBasedRoutingConfig from request"
            )};
        }
        if (m_support.dynamic_ref_has_feature(profile, "contract_based_routing", feature)) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Contract Routing with specified features is already enabled"
            )};
        }
        if (const auto validation_error =
                m_support.profile_validation_error(ctx_auth_profile(ctx), profile)) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error)
            };
        }
        const auto algorithm = m_support.new_routing_id();
        auto record = m_support.config_record(
            ctx.merchant_id,
            algorithm,
            "Contract based dynamic routing algorithm",
            "",
            "contract_based",
            profile,
            serde::Ser::encode_json(*body)
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
        return ResponseDocument::from(m_support.dictionary_record(
            algorithm,
            profile,
            "Contract based dynamic routing algorithm",
            "",
            "contract_based",
            "payment"
        ));
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
