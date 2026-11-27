export module app_user:profile_acquirer_runtime_create;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_behavior;
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

import :profile_acquirer_input;
import :profile_acquirer_result;
import :profile_acquirer_runtime_common;

export namespace app::handlers::profile_acquirer {

class CreateProfileAcquirerRuntime
{
public:
    explicit CreateProfileAcquirerRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // routes/profile_acquirer.rs:11-50 -> core/profile_acquirer.rs:12-102.
    [[nodiscard]] core::behavior::Outcome<::app::dto::profile_acquirer::ProfileAcquirerResponse>
        create(const ::app::dto::profile_acquirer::ProfileAcquirerCreate& request,
        const ::core::behavior::CallContext& ctx) const
    {
        // core:20-26 — find_business_profile_by_profile_id -> ProfileNotFound.
        if (!m_support.profile_exists(ctx_string(ctx,"profile_id"))) {
            return std::unexpected{m_support.profile_not_found(ctx_string(ctx,"profile_id"))};
        }

        // core:28-38 — has_existing_default / is_default.
        auto bucket = m_support.load_bucket(ctx_string(ctx,"profile_id"));
        const bool has_existing_default =
            bucket.has_value() && bucket->default_acquirer_config.has_value();
        const bool is_default =
            !has_existing_default || request.get_is_default().value_or(false);

        // core:17 — generate_profile_acquirer_id_of_default_length().
        const std::string profile_acquirer_id =
            m_support.generate_profile_acquirer_id_of_default_length();

        // core:40-52 — build the incoming AcquirerConfig (country normalized).
        AcquirerConfig incoming = m_support.config_from_create(request);

        // core:54-60 — initialize the bucket with get_or_insert_with.
        if (!bucket) {
            bucket.emplace();
        }

        // core:62-65 — configs.insert(id, vec![incoming]) (replaces the bucket slot).
        bucket->configs[profile_acquirer_id] = std::vector<AcquirerConfig>{incoming};

        // core:67-69 — default pointer.
        if (is_default) {
            bucket->default_acquirer_config = profile_acquirer_id;
        }

        // core:71-79 — ProfileUpdate::AcquirerConfigBucketUpdate persistence.
        m_support.save_bucket(ctx_string(ctx,"profile_id"), *bucket);

        // core:81-92 — re-read and find the slot matching `network`.
        const auto stored = m_support.load_bucket(ctx_string(ctx,"profile_id"));
        const AcquirerConfig* updated = m_support.find_config_in_bucket(
            stored ? &*stored : nullptr,
            profile_acquirer_id,
            incoming.get_network()
        );
        if (updated == nullptr) {
            // core:91-92 .attach_printable("Failed to get updated acquirer config")
            return std::unexpected{m_support.internal_server_error()};
        }

        // core:94-99 — ProfileAcquirerResponse::from((id, &profile_id, Some(cfg), is_default)).
        auto response =
            m_support.build_response(profile_acquirer_id, ctx_string(ctx,"profile_id"), updated, is_default);

        m_context.metrics().record_flow("ProfileAcquirerCreate");
        m_context.metrics().counter("profile_acquirer_create").increment();

        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    ProfileAcquirerRuntimeSupport m_support;
};

} // namespace app::handlers::profile_acquirer
