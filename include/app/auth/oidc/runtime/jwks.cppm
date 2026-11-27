export module app_auth:oidc_runtime_jwks;

import std;
import core_behavior;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_context_core;
import app_http;
import app_crypto;
import app_metrics;
import app_context_core;
import app_keystore;
import app_services;

import :oidc_input;
import :oidc_result;
import :oidc_runtime_common;

export namespace app::handlers::oidc {

class OidcJwksRuntime
{
public:
    explicit OidcJwksRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::oidc::JwksResponse>
    jwks(const core::behavior::CallContext& /*ctx*/) const
    {
        m_context.logger().record_flow("jwks_endpoint");
        m_context.logger().counter("oidc.jwks_endpoint").increment();
        m_context.logger().counter("oidc.api_event.Oidc").increment();

        m_support.ensure_oidc_config();

        using Response = ::app::dto::oidc::JwksResponse;
        const auto key = std::string{"oidc.jwks_endpoint:document"};
        const auto cached = m_context.cache().get(key);
        Response document{};
        if (cached.hit) {
            // Cache stores encoded JSON as an internal storage format; decode it back
            // into the typed DTO so the response is always written from typed data.
            auto decoded = serde::Ser::deserialize<Response>(
                "application/json",
                cached.value_or<std::string>("")
            );
            if (decoded) {
                document = std::move(*decoded);
            }
        } else {
            std::vector<::app::dto::oidc::Jwk> keys{};
            for (const auto& configured: m_support.configured_oidc_keys()) {
                const auto components =
                    oidc_support::extract_rsa_public_key_components(configured.private_key);
                if (!components) {
                    continue;
                }
                ::app::dto::oidc::Jwk jwk{};
                jwk.set_kty(::app::dto::oidc::KeyType::Rsa);
                jwk.set_kid(configured.kid);
                jwk.set_key_use(::app::dto::oidc::KeyUse::Sig);
                jwk.set_alg(::app::dto::oidc::SigningAlgorithm::Rs256);
                jwk.set_n(components->first);
                jwk.set_e(components->second);
                keys.push_back(std::move(jwk));
            }
            document.set_keys(std::move(keys));
            m_context.cache().put(
                key,
                core::cache::to_cache_value(serde::Ser::encode_json(document))
            );
        }
        return std::move(document);
    }

private:
    ::app::context::AppContext& m_context;
    OidcRuntimeSupport m_support;
};

} // namespace app::handlers::oidc
