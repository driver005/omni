export module app_user:profiles_runtime_create;

import std;
import core_behavior;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_metrics;

import :profiles_input;
import :profiles_result;
import :profiles_runtime_common;

export namespace app::handlers::profiles {

// core/admin.rs:4061-4119 — create_profile.
class ProfileCreateRuntime
{
public:
    explicit ProfileCreateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<ProfileWireResponse>
    create(const ::app::dto::admin::ProfileCreate& request,
        const ::core::behavior::CallContext& ctx) const
    {
        auto repository = m_context.repository();
        auto account = repository.find_merchant_account_by_string(ctx_string(ctx,"merchant_id"));
        const std::string account_type =
            account ? account->get_merchant_account_type() : std::string{};

        // core/admin.rs:4066-4070.
        if (const auto error = m_support.validate_external_vault_config_for_merchant_account_type(
                account_type,
                request.get_is_external_vault_enabled(),
                request.get_external_vault_connector_details().has_value()
            )) {
            return std::unexpected(*error);
        }

        // core/admin.rs:4072-4084 — create_domain_model_from_request.
        auto domain = m_support.create_domain_model_from_request(ctx_string(ctx,"merchant_id"), request);
        if (!domain) {
            return std::unexpected(domain.error());
        }
        const std::string profile_id = domain->get_profile_id();

        // core/admin.rs:4088-4098 — insert_business_profile + duplicate mapping
        // (GenericDuplicateError -> ALREADY_EXISTS / HTTP 409).
        if (repository.find_profile_by_string(profile_id)) {
            return std::unexpected(m_support.make_error(
                core::error::AppErrorKind::ALREADY_EXISTS,
                std::format("Business Profile with the profile_id {} already exists", profile_id)
            ));
        }
        if (const auto insert_error = m_support.insert_business_profile(
                profile_id,
                ctx_string(ctx,"merchant_id"),
                domain->get_profile_name(),
                *domain
            )) {
            return std::unexpected(*insert_error);
        }

        // core/admin.rs:4100-4112 (v1) — unset an existing default profile.
        if (!ctx_bool(ctx,"v2_route") && account && !account->get_default_profile().empty() &&
            account->get_default_profile() != profile_id) {
            account->set_default_profile({});
            (void)repository.update_merchant_account(*account);
        }

        const auto payload = serde::Ser::encode_json(*domain);
        m_support.emit_event("create", payload);
        m_context.metrics().record_flow("profiles.create");
        m_context.metrics().record_flow("Flow::ProfileCreate");
        m_context.metrics().counter("profiles_create").increment();
        return std::move(*domain);
    }

private:
    ::app::context::AppContext& m_context;
    ProfilesRuntimeSupport m_support;
};

} // namespace app::handlers::profiles
