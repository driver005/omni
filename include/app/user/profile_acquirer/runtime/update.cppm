export module app_user:profile_acquirer_runtime_update;

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

class ProfileAcquirerUpdateRuntime
{
public:
    explicit ProfileAcquirerUpdateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // routes/profile_acquirer.rs:52-97 -> core/profile_acquirer.rs:105-215.
    [[nodiscard]] core::behavior::Outcome<::app::dto::profile_acquirer::ProfileAcquirerResponse>
        update(const ::app::dto::profile_acquirer::ProfileAcquirerUpdate& request,
        const ::core::behavior::CallContext& ctx) const
    {
        // core:114-120 — find_business_profile_by_profile_id -> ProfileNotFound.
        if (!m_support.profile_exists(ctx_string(ctx,"profile_id"))) {
            return std::unexpected{m_support.profile_not_found(ctx_string(ctx,"profile_id"))};
        }

        // core:122-129 — acquirer_config_map must exist.
        auto bucket = m_support.load_bucket(ctx_string(ctx,"profile_id"));
        if (!bucket) {
            return std::unexpected{
                m_support.profile_acquirer_not_found(ctx_string(ctx,"profile_id"), ctx_string(ctx,"profile_acquirer_id"))
            };
        }

        // core:131 — default_bucket_id.
        const auto default_bucket_id = bucket->default_acquirer_config;

        // core:133-144 — verify the target bucket exists.
        if (bucket->configs.find(ctx_string(ctx,"profile_acquirer_id")) == bucket->configs.end()) {
            return std::unexpected{
                m_support.profile_acquirer_not_found(ctx_string(ctx,"profile_id"), ctx_string(ctx,"profile_acquirer_id"))
            };
        }

        // core:147-149 — is_default from request or current default pointer.
        const bool is_default = request.get_is_default().value_or(
            default_bucket_id.has_value() && *default_bucket_id == ctx_string(ctx,"profile_acquirer_id")
        );

        // core:151-156 — apply_default_bucket_change (mutates the map).
        const bool default_bucket_changed = m_support.apply_default_bucket_change(
            *bucket,
            ctx_string(ctx,"profile_acquirer_id"),
            default_bucket_id,
            request.get_is_default()
        );

        // core:158-180 — upsert the network slot, or allow is_default-only changes.
        if (request.get_network().has_value()) {
            if (const auto err = m_support.upsert_acquirer_config_in_bucket(
                    *bucket,
                    ctx_string(ctx,"profile_acquirer_id"),
                    request.get_network()->get_value(),
                    request,
                    default_bucket_changed
                )) {
                return std::unexpected{*err};
            }
        } else if (!default_bucket_changed) {
            return std::unexpected{m_support.invalid_request(
                "A `network` must be provided to update an acquirer configuration, unless you are "
                "only changing the `is_default` fallback status of this entire bucket."
            )};
        }

        // core:182-193 — persist the updated bucket.
        m_support.save_bucket(ctx_string(ctx,"profile_id"), *bucket);

        // core:195-205 — final config from the re-read profile; None for is_default-only.
        const AcquirerConfig* final_config = nullptr;
        if (request.get_network().has_value()) {
            const auto stored = m_support.load_bucket(ctx_string(ctx,"profile_id"));
            final_config = m_support.find_config_in_bucket(
                stored ? &*stored : nullptr,
                ctx_string(ctx,"profile_acquirer_id"),
                request.get_network()->get_value()
            );
        }

        // core:207-212 — ProfileAcquirerResponse::from((id, &profile_id, final_cfg, is_default)).
        auto response = m_support.build_response(
            ctx_string(ctx,"profile_acquirer_id"),
            ctx_string(ctx,"profile_id"),
            final_config,
            is_default
        );

        m_context.metrics().record_flow("ProfileAcquirerUpdate");
        m_context.metrics().counter("profile_acquirer_update").increment();

        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    ProfileAcquirerRuntimeSupport m_support;
};

} // namespace app::handlers::profile_acquirer
