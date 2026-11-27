export module app_routing:routing_runtime_routing_entry;

import std;
import core_error;
import core_serde;
import core_cache;
import app_context;
import app_model;
import app_crypto;
import core_behavior;

import :routing_result;
import :routing_runtime_common;

export namespace app::handlers::routing {

class RoutingEntryRuntime
{
public:
    explicit RoutingEntryRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    routing_entry(const ::core::behavior::CallContext& ctx) const
    {
        if (ctx.profile_id.empty()) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "profile_id")
            };
        }
        if (const auto validation_error =
                m_support.profile_validation_error(ctx_auth_profile(ctx), ctx.profile_id)) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error)
            };
        }

        // Dimensions::with_* + is_decision_engine_routing_effective.
        const auto cutover = m_support.is_cutover(ctx.merchant_id, ctx.profile_id);
        ::app::model::routing::RoutingEntryResponse response;
        response.set_is_cutover(cutover);

        // Mint a one-time code only for (cutover, target). DE network calls are substituted
        // by a deterministic local code + token (external-infra substitute).
        const auto target = ctx_query(ctx, "target");
        if (cutover && target) {
            const auto merchant = ctx.merchant_id;
            const auto org = ctx.org_id;
            const auto profile = ctx.profile_id;
            const auto key_id = m_context.ids().key_id();
            const auto digest = ::app::crypto::KeyedHasher{"de-sso"}.hash(key_id);
            std::uint64_t step{};
            std::from_chars(digest.data(), digest.data() + 8, step, 16);
            const auto code =
                ::app::crypto::TotpGenerator{}.generate("de-sso:" + merchant + ":" + profile, step);
            const auto payload = std::format(
                R"({{"merchant":"{}","org":"{}","profile":"{}","target":"{}","code":"{}"}})",
                merchant,
                org,
                profile,
                *target,
                code
            );
            const auto token = ::app::crypto::JwtSigner{"routing-de-sso"}.sign(payload);
            const auto cache_key = "routing.entry:" + code;
            m_context.cache().put(cache_key, core::cache::to_cache_value(token));
            const auto cached = m_context.cache().get(cache_key).value_or(std::string{});
            if (cached.empty()) {
                return std::unexpected{
                    m_support.error(core::error::AppErrorKind::INTERNAL, "SSO token not cached")
                };
            }
            response.set_redirect_url(
                m_support.dashboard_url() + "/" + m_support.dashboard_path(*target) +
                "?code=" + code + "&token=" + cached
            );
        }
        m_support.track("routing_entry");
        return ResponseDocument::from(response);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
