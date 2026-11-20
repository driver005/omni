module;
export module app_handlers:hierarchical_resources;

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
import app_keystore;
import app_services;
import app_events;

export namespace app::handlers::hierarchical_resources {

// Local typed wire shapes. The app_dto wrappers serialize nested ids and dates as
// {"value":...}; upstream sends a plain ResourceId string and a PrimitiveDateTime
// string. These shapes keep the upstream wire form while still going through
// serde::Ser::encode_json (no raw body echo).
class GenerateResponseWire
{
public:
    std::string id{};
    std::string data{};
};

class UploadResponseWire
{
public:
    std::string id{};
    std::string created_at{};
};

class SummaryWire
{
public:
    std::string id{};
    std::string display_schema{};
    std::string display_data{};
    std::string created_at{};
    bool is_linked{};
};

class ListResponseWire
{
public:
    std::vector<SummaryWire> resources{};
};

class LinkResponseWire
{
public:
    std::string id{};
};

class HierarchicalResourcesRouteHandlers
{
public:
    explicit HierarchicalResourcesRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    void handle_generate_hierarchical_resource(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "generate_hierarchical_resource");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, app_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::hierarchical_resources::GenerateHierarchicalResourceRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, app_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto resource_type = parsed->get_resource_type().get_value();
        if (!valid_resource_type(resource_type)) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::VALIDATION_FAILED, "invalid resource type"));
            return;
        }

        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto merchant_id = resolve_merchant_id(req, auth);
        if (!authorized_profile_id(req, auth, merchant_id).has_value()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::UNAUTHORIZED, "profile not found for merchant"));
            return;
        }
        const auto scope_id = resolve_scope_id(req, auth, merchant_id);
        if (scope_id.empty()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::INVALID_REQUEST, "missing organization scope"));
            return;
        }

        m_context->metrics().record_flow("HierarchicalResourcesGenerate");
        m_context->metrics().counter("hierarchical_resources_generate_total").increment();

        // Upstream: ensure_organization_key_store -> EC keypair + CSR -> private key
        // encrypted under the org key -> insert_linked_resource. The local stand-ins
        // are the repository-backed KeyStore (AES-256 purpose label), a deterministic
        // uuid-derived key/CSR pair and the HierarchicalResource repository record.
        const auto resource_id = m_context->ids().payment_id("rs_");
        const auto org_key = ensure_org_key(scope_id);
        const app::crypto::SecretString private_key{
            m_context->ids().uuid_v4() + m_context->ids().uuid_v4()
        };
        const auto encrypted_private_key = app::crypto::AesWrap{}.encrypt(private_key.value(), org_key);
        const auto csr = "csr_" + app::crypto::KeyedHasher{org_key}.hash(resource_id);

        ResourceData payload;
        payload.merchant_id = merchant_id;
        payload.csr = csr;
        payload.encrypted_private_key = encrypted_private_key;

        app::store::hierarchical_resources::HierarchicalResourceRecord record{};
        record.set_resource_id(resource_id);
        record.set_scope_id(scope_id);
        record.set_resource_type(resource_type);
        record.set_data(encode_resource_data(payload));
        record.set_status(status_csr_generated());
        record.set_created_at(created_at_value());
        const auto stored = m_context->repository().create_hierarchical_resource(std::move(record));
        if (!stored) {
            send_error(res, send, stored.error());
            return;
        }

        const auto data = "{\"" + resource_type + "\":{\"csr\":\"" + csr + "\"}}";
        const GenerateResponseWire response{resource_id, data};
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_upload_hierarchical_resource(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "upload_hierarchical_resource");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto resource_id = resource_id_from_path(req, "apple_pay_certificate");
        if (!resource_id.has_value() || resource_id->empty()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::INVALID_REQUEST, "missing resource id in path"));
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, app_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::hierarchical_resources::UploadCertificateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, app_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        if (parsed->get_certificate().empty()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::INVALID_REQUEST, "certificate is required"));
            return;
        }

        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto merchant_id = resolve_merchant_id(req, auth);
        if (!authorized_profile_id(req, auth, merchant_id).has_value()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::UNAUTHORIZED, "profile not found for merchant"));
            return;
        }
        const auto scope_id = resolve_scope_id(req, auth, merchant_id);

        m_context->metrics().record_flow("HierarchicalResourcesUpload");
        m_context->metrics().counter("hierarchical_resources_upload_total").increment();

        const auto stored = m_context->repository().find_hierarchical_resource_by_string(*resource_id);
        if (!stored.has_value()) {
            send_error(res, send, app_error(core::error::AppErrorKind::NOT_FOUND, "resource not found"));
            return;
        }
        // authorize_scope_id_belongs_to_org: compare the persisted scope against the
        // authoritative organization resolved from the caller's merchant account.
        if (!scope_belongs_to_org(stored->get_scope_id(), scope_id)) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::FORBIDDEN, "resource belongs to a different organization"));
            return;
        }

        auto payload = decode_resource_data(stored->get_data());
        if (!merchant_id.empty() && !payload.merchant_id.empty() && payload.merchant_id != merchant_id) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::FORBIDDEN, "resource belongs to a different merchant"));
            return;
        }

        // Upstream: base64-DER decode, X509::from_der, cert/key match, CN parse,
        // status -> Active. DER/X.509 decoding needs an external crypto library, so
        // the local substitute records a keyed digest and extracts an Apple merchant
        // identifier from a CN= segment when one is present in the supplied text.
        const auto org_key = ensure_org_key(stored->get_scope_id());
        payload.certificate_digest = app::crypto::KeyedHasher{org_key}.hash(parsed->get_certificate());
        payload.apple_merchant_identifier = extract_apple_merchant_id(parsed->get_certificate());

        auto updated = *stored;
        updated.set_data(encode_resource_data(payload));
        updated.set_status(status_active());
        if (!m_context->repository().update_hierarchical_resource(updated)) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::INTERNAL_ERROR, "failed to update resource"));
            return;
        }

        const UploadResponseWire response{*resource_id, updated.get_created_at()};
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_list_hierarchical_resources(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "list_hierarchical_resources");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, app_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::hierarchical_resources::ListHierarchicalResourcesRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, app_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto resource_type = parsed->get_resource_type().get_value();
        if (!valid_resource_type(resource_type)) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::VALIDATION_FAILED, "invalid resource type"));
            return;
        }

        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto merchant_id = resolve_merchant_id(req, auth);
        if (!authorized_profile_id(req, auth, merchant_id).has_value()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::UNAUTHORIZED, "profile not found for merchant"));
            return;
        }
        const auto scope_id = resolve_scope_id(req, auth, merchant_id);

        m_context->metrics().record_flow("HierarchicalResourcesList");
        m_context->metrics().counter("hierarchical_resources_list_total").increment();

        // resolve_apple_pay_certificate_data cascade: effective id comes from the
        // requestor scope, falling back through profile -> merchant account.
        const auto effective_resource_id = resolve_effective_resource_id(req, *parsed, auth);

        std::vector<SummaryWire> summaries;
        for (const auto& record : m_context->repository().list_hierarchical_resources()) {
            if (record.get_resource_type() != resource_type) {
                continue;
            }
            if (!scope_id.empty() && !record.get_scope_id().empty() && record.get_scope_id() != scope_id) {
                continue;
            }
            if (record.get_status() != status_active()) {
                continue;
            }
            const auto payload = decode_resource_data(record.get_data());
            if (!merchant_id.empty() && !payload.merchant_id.empty() && payload.merchant_id != merchant_id) {
                continue;
            }

            SummaryWire summary;
            summary.id = record.get_resource_id();
            summary.display_schema = R"({"apple_pay_merchant_identifier":"String"})";
            summary.display_data = payload.apple_merchant_identifier.empty()
                ? std::string{R"({"apple_pay_merchant_identifier":null})"}
                : "{\"apple_pay_merchant_identifier\":\"" + payload.apple_merchant_identifier + "\"}";
            summary.created_at = record.get_created_at();
            summary.is_linked = !effective_resource_id.empty()
                && effective_resource_id == record.get_resource_id();
            summaries.emplace_back(std::move(summary));
        }

        const ListResponseWire response{std::move(summaries)};
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_link_hierarchical_resource(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "link_hierarchical_resource");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto resource_id = resource_id_from_path(req, "link");
        if (!resource_id.has_value() || resource_id->empty()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::INVALID_REQUEST, "missing resource id in path"));
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, app_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::hierarchical_resources::LinkHierarchicalResourceRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, app_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        const auto requestor_type = parsed->get_requestor_type().get_value();
        const auto& requestor_id = parsed->get_requestor_id();
        if (requestor_type.empty() || requestor_id.empty()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::INVALID_REQUEST, "requestor_type and requestor_id are required"));
            return;
        }

        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto merchant_id = resolve_merchant_id(req, auth);
        if (!authorized_profile_id(req, auth, merchant_id).has_value()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::UNAUTHORIZED, "profile not found for merchant"));
            return;
        }
        const auto scope_id = resolve_scope_id(req, auth, merchant_id);

        m_context->metrics().record_flow("HierarchicalResourcesLink");
        m_context->metrics().counter("hierarchical_resources_link_total").increment();

        const auto stored = m_context->repository().find_hierarchical_resource_by_string(*resource_id);
        if (!stored.has_value()) {
            send_error(res, send, app_error(core::error::AppErrorKind::NOT_FOUND, "resource not found"));
            return;
        }
        if (!scope_belongs_to_org(stored->get_scope_id(), scope_id)) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::FORBIDDEN, "resource belongs to a different organization"));
            return;
        }
        if (stored->get_status() != status_active()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::PRECONDITION_FAILED, "resource is not active"));
            return;
        }
        const auto payload = decode_resource_data(stored->get_data());
        if (!merchant_id.empty() && !payload.merchant_id.empty() && payload.merchant_id != merchant_id) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::FORBIDDEN, "resource belongs to a different merchant"));
            return;
        }

        // fetch_requestor_account: strict merchant-scoped resolution with
        // to_not_found_response style mapping. Profiles now have a repository entity.
        if (requestor_type == "merchant_connector_account" || requestor_type == "merchant_connector") {
            const auto connector = m_context->repository().find_merchant_connector_by_string(requestor_id);
            if (!connector.has_value()) {
                send_error(res, send, app_error(
                    core::error::AppErrorKind::NOT_FOUND, "merchant connector account not found"));
                return;
            }
            if (!merchant_id.empty() && connector->get_merchant_id() != merchant_id) {
                send_error(res, send, app_error(
                    core::error::AppErrorKind::FORBIDDEN, "requestor belongs to a different merchant"));
                return;
            }
        } else if (requestor_type == "merchant_account") {
            const auto account = m_context->repository().find_merchant_account_by_string(requestor_id);
            if (!account.has_value()) {
                send_error(res, send, app_error(
                    core::error::AppErrorKind::NOT_FOUND, "merchant account not found"));
                return;
            }
            if (!merchant_id.empty() && requestor_id != merchant_id) {
                send_error(res, send, app_error(
                    core::error::AppErrorKind::FORBIDDEN, "requestor belongs to a different merchant"));
                return;
            }
        } else if (requestor_type == "profile") {
            const auto profile = m_context->repository().find_profile_by_string(requestor_id);
            if (!profile.has_value()) {
                send_error(res, send, app_error(
                    core::error::AppErrorKind::NOT_FOUND, "profile not found"));
                return;
            }
            if (!merchant_id.empty() && profile->get_merchant_id() != merchant_id) {
                send_error(res, send, app_error(
                    core::error::AppErrorKind::FORBIDDEN, "requestor belongs to a different merchant"));
                return;
            }
        } else {
            send_error(res, send, app_error(
                core::error::AppErrorKind::INVALID_REQUEST, "unsupported requestor_type"));
            return;
        }

        // on_link: re-encrypt the stored private key under a requestor-scoped key from
        // the repository-backed key store, then persist the link as a resource record
        // and mirror the account attachment through config_store (with audit log).
        const auto requestor_key = ensure_requestor_key(requestor_id);
        const auto rewrapped = app::crypto::AesWrap{}.encrypt(payload.encrypted_private_key, requestor_key);

        const auto link_id = link_resource_id(requestor_type, requestor_id);
        auto link_record = m_context->repository().find_hierarchical_resource_by_string(link_id);
        const auto link_data = requestor_id + "|" + *resource_id + "|" + requestor_type + "|" + rewrapped;
        if (!link_record.has_value()) {
            app::store::hierarchical_resources::HierarchicalResourceRecord fresh{};
            fresh.set_resource_id(link_id);
            fresh.set_scope_id(*resource_id);
            fresh.set_resource_type("link");
            fresh.set_data(link_data);
            fresh.set_status(status_active());
            fresh.set_created_at(created_at_value());
            const auto created = m_context->repository().create_hierarchical_resource(std::move(fresh));
            if (!created) {
                send_error(res, send, created.error());
                return;
            }
            link_record = *created;
        } else {
            link_record->set_scope_id(*resource_id);
            link_record->set_data(link_data);
            if (!m_context->repository().update_hierarchical_resource(*link_record)) {
                send_error(res, send, app_error(
                    core::error::AppErrorKind::INTERNAL_ERROR, "failed to persist link"));
                return;
            }
        }

        m_context->config_store().put(
            "hierarchical_resources.account_links", requestor_id, *resource_id, requestor_type);
        m_context->events().publish(
            "hierarchical_resources.linked",
            "{\"resource_id\":\"" + *resource_id + "\",\"requestor_id\":\"" + requestor_id + "\"}");

        const LinkResponseWire response{*resource_id};
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/hierarchical_resources"}
            .add_route(
                Route{"/"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_generate_hierarchical_resource(req, res, std::move(send));
                    })
                    .summary("Hyperswitch generate_hierarchical_resource")
            )
            .add_route(
                Route{"/list"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_hierarchical_resources(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_hierarchical_resources")
            )
            .add_route(
                Route{"/apple_pay_certificate/:resource_id"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_upload_hierarchical_resource(req, res, std::move(send));
                    })
                    .summary("Hyperswitch upload_hierarchical_resource")
            )
            .add_route(
                Route{"/:resource_id/link"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_link_hierarchical_resource(req, res, std::move(send));
                    })
                    .summary("Hyperswitch link_hierarchical_resource")
            );
    }

private:
    /** @brief Registry rule for the operation, with upstream profile/allow overlays. */
    [[nodiscard]] std::optional<core::error::AppError> authorize_request(
        interfaces::io::IRequest& req,
        std::string_view operation
    ) const
    {
        const auto base = m_context->auth_rules().rule_for(operation);
        const auto rule = app::auth::RouteAuthRuleBuilder{}
                              .scheme(base.scheme())
                              .permission(base.permission())
                              .allow_connected(true)
                              .allow_platform(true)
                              .id_source("x-profile-id")
                              .build();
        return m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
    }

    /** @brief Upstream fetches the X-Profile-Id profile scoped to the JWT merchant. */
    [[nodiscard]] std::optional<std::string> authorized_profile_id(
        interfaces::io::IRequest& req,
        const app::auth::AuthContext& auth,
        std::string_view merchant_id
    ) const
    {
        const auto header = app::http::HeaderReader{}.get(req, "x-profile-id");
        const std::string profile_id = header.empty() ? auth.profile_id() : header;
        if (profile_id.empty()) {
            return std::nullopt;
        }
        const auto profile = m_context->repository().find_profile_by_string(profile_id);
        if (!profile.has_value()) {
            return std::nullopt;
        }
        if (!merchant_id.empty() && profile->get_merchant_id() != merchant_id) {
            return std::nullopt;
        }
        return profile_id;
    }

    [[nodiscard]] std::string resolve_merchant_id(
        interfaces::io::IRequest& req,
        const app::auth::AuthContext& auth
    ) const
    {
        const auto header = app::http::merchant_id_from_header{}(req);
        return header.empty() ? auth.merchant_id() : header;
    }

    /** @brief Authoritative organization of the caller's merchant account. */
    [[nodiscard]] std::string resolve_scope_id(
        interfaces::io::IRequest& req,
        const app::auth::AuthContext& auth,
        std::string_view merchant_id
    ) const
    {
        if (!merchant_id.empty()) {
            const auto account = m_context->repository().find_merchant_account_by_string(merchant_id);
            if (account.has_value() && !account->get_organization_id().empty()) {
                return account->get_organization_id();
            }
        }
        const auto header_org = app::http::org_id_from_header{}(req);
        if (!header_org.empty()) {
            const auto organization = m_context->repository().find_organization_by_string(header_org);
            if (organization.has_value() && !organization->get_organization_id().empty()) {
                return organization->get_organization_id();
            }
            return header_org;
        }
        const auto header_org_long = app::http::HeaderReader{}.get(req, "x-organization-id");
        if (!header_org_long.empty()) {
            return header_org_long;
        }
        return auth.org_id();
    }

    [[nodiscard]] bool scope_belongs_to_org(
        std::string_view resource_scope,
        std::string_view authoritative_org
    ) const
    {
        if (resource_scope.empty()) {
            return true;
        }
        if (authoritative_org.empty()) {
            return false;
        }
        return resource_scope == authoritative_org;
    }

    /** @brief Extract :resource_id next to the route's fixed sibling segment. */
    [[nodiscard]] std::optional<std::string> resource_id_from_path(
        interfaces::io::IRequest& req,
        std::string_view marker
    ) const
    {
        const auto segments = app::http::RequestPath{}(req);
        for (std::size_t i{0}; i < segments.count(); ++i) {
            if (segments.at(i).value_or(std::string_view{}) != marker) {
                continue;
            }
            const std::size_t target = marker == "apple_pay_certificate"
                ? i + 1
                : (i == 0 ? 0 : i - 1);
            if (target >= segments.count()) {
                break;
            }
            const auto candidate = segments.at(target);
            if (candidate.has_value() && !candidate->empty()
                && *candidate != "link" && *candidate != "apple_pay_certificate") {
                return std::string{*candidate};
            }
        }
        return std::nullopt;
    }

    // ── Repository-backed key store (org master key / requestor re-wrap key) ──

    [[nodiscard]] std::string ensure_org_key(std::string_view org_id) const
    {
        constexpr std::string_view purpose{"hierarchical_resources.org"};
        const auto existing = m_context->key_store().get(purpose, org_id);
        if (existing.has_value()) {
            return existing->value();
        }
        const app::crypto::SecretString master{
            m_context->ids().uuid_v4() + m_context->ids().uuid_v4()
        };
        m_context->key_store().put(purpose, org_id, "AES-256", master);
        return master.value();
    }

    [[nodiscard]] std::string ensure_requestor_key(std::string_view requestor_id) const
    {
        constexpr std::string_view purpose{"hierarchical_resources.requestor"};
        const auto existing = m_context->key_store().get(purpose, requestor_id);
        if (existing.has_value()) {
            return existing->value();
        }
        const app::crypto::SecretString key{
            m_context->ids().uuid_v4() + m_context->ids().uuid_v4()
        };
        m_context->key_store().put(purpose, requestor_id, "AES-256", key);
        return key.value();
    }

    // ── Resource payload encoding (repository `data` column) ──

    struct ResourceData
    {
        std::string merchant_id{};
        std::string csr{};
        std::string encrypted_private_key{};
        std::string certificate_digest{};
        std::string apple_merchant_identifier{};
    };

    [[nodiscard]] std::string encode_resource_data(const ResourceData& value) const
    {
        return value.merchant_id + "|" + value.csr + "|" + value.encrypted_private_key + "|"
            + value.certificate_digest + "|" + value.apple_merchant_identifier;
    }

    [[nodiscard]] ResourceData decode_resource_data(std::string_view raw) const
    {
        const auto fields = split(raw, '|');
        ResourceData result;
        if (fields.size() > 0) { result.merchant_id = fields[0]; }
        if (fields.size() > 1) { result.csr = fields[1]; }
        if (fields.size() > 2) { result.encrypted_private_key = fields[2]; }
        if (fields.size() > 3) { result.certificate_digest = fields[3]; }
        if (fields.size() > 4) { result.apple_merchant_identifier = fields[4]; }
        return result;
    }

    [[nodiscard]] std::string extract_apple_merchant_id(std::string_view certificate) const
    {
        const auto pos = certificate.find("CN=");
        if (pos == std::string_view::npos) {
            return {};
        }
        const auto rest = certificate.substr(pos + 3);
        const auto end = rest.find_first_of(",/ \t\r\n\"");
        return std::string{rest.substr(0, end)};
    }

    // ── Link resolution cascade (requestor -> its account -> effective resource) ──

    [[nodiscard]] std::string link_resource_id(
        std::string_view requestor_type,
        std::string_view requestor_id
    ) const
    {
        return "link:" + std::string{requestor_type} + ":" + std::string{requestor_id};
    }

    [[nodiscard]] std::string linked_resource_from(
        const app::store::hierarchical_resources::HierarchicalResourceRecord& record
    ) const
    {
        const auto fields = split(record.get_data(), '|');
        return fields.size() > 1 ? fields[1] : std::string{};
    }

    [[nodiscard]] std::string find_linked_resource(
        std::string_view requestor_type,
        std::string_view requestor_id
    ) const
    {
        const auto link = m_context->repository().find_hierarchical_resource_by_string(
            link_resource_id(requestor_type, requestor_id));
        if (link.has_value()) {
            return linked_resource_from(*link);
        }
        if (requestor_type == "profile") {
            const auto profile = m_context->repository().find_profile_by_string(requestor_id);
            if (profile.has_value()) {
                return find_linked_resource("merchant_account", profile->get_merchant_id());
            }
        } else if (requestor_type == "merchant_connector_account" || requestor_type == "merchant_connector") {
            const auto connector = m_context->repository().find_merchant_connector_by_string(requestor_id);
            if (connector.has_value()) {
                return find_linked_resource("merchant_account", connector->get_merchant_id());
            }
        }
        return {};
    }

    [[nodiscard]] std::string resolve_effective_resource_id(
        interfaces::io::IRequest& req,
        const app::dto::hierarchical_resources::ListHierarchicalResourcesRequest& request,
        const app::auth::AuthContext& auth
    ) const
    {
        if (request.get_scope_id().has_value() && !request.get_scope_id()->empty()) {
            const auto type = request.get_scope_type().has_value()
                ? request.get_scope_type()->get_value()
                : std::string{"profile"};
            return find_linked_resource(type, *request.get_scope_id());
        }
        const auto header = app::http::HeaderReader{}.get(req, "x-profile-id");
        const auto profile_id = header.empty() ? auth.profile_id() : header;
        if (profile_id.empty()) {
            return {};
        }
        return find_linked_resource("profile", profile_id);
    }

    // ── Small validators / formatters ──

    [[nodiscard]] bool valid_resource_type(std::string_view value) const
    {
        if (value.empty()) {
            return false;
        }
        for (const char c : value) {
            const bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                || (c >= '0' && c <= '9') || c == '_' || c == '-';
            if (!ok) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] std::string status_csr_generated() const
    {
        return "csr_generated";
    }

    [[nodiscard]] std::string status_active() const
    {
        return "active";
    }

    [[nodiscard]] std::int64_t now_epoch() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    /** @brief Civil date from days since 1970-01-01; deterministic UTC formatting. */
    [[nodiscard]] std::string iso_utc(std::int64_t epoch_seconds) const
    {
        constexpr std::int64_t seconds_per_day{86400};
        std::int64_t days = epoch_seconds / seconds_per_day;
        std::int64_t seconds_of_day = epoch_seconds % seconds_per_day;
        if (seconds_of_day < 0) {
            seconds_of_day += seconds_per_day;
            --days;
        }
        days += 719468;
        const std::int64_t era = (days >= 0 ? days : days - 146096) / 146097;
        const std::int64_t doe = days - era * 146097;
        const std::int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
        std::int64_t year = yoe + era * 400;
        const std::int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        const std::int64_t mp = (5 * doy + 2) / 153;
        const std::int64_t day = doy - (153 * mp + 2) / 5 + 1;
        const std::int64_t month = mp < 10 ? mp + 3 : mp - 9;
        year += (month <= 2) ? 1 : 0;
        return std::format(
            "{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}Z",
            year, month, day,
            seconds_of_day / 3600,
            (seconds_of_day % 3600) / 60,
            seconds_of_day % 60
        );
    }

    [[nodiscard]] std::string created_at_value() const
    {
        return iso_utc(now_epoch());
    }

    [[nodiscard]] std::vector<std::string> split(std::string_view value, char delimiter) const
    {
        std::vector<std::string> parts;
        std::size_t start{0};
        while (start <= value.size()) {
            const auto pos = value.find(delimiter, start);
            parts.emplace_back(value.substr(start, pos == std::string_view::npos
                ? std::string_view::npos : pos - start));
            if (pos == std::string_view::npos) {
                break;
            }
            start = pos + 1;
        }
        return parts;
    }

    [[nodiscard]] core::error::AppError app_error(
        core::error::AppErrorKind kind,
        std::string message
    ) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::hierarchical_resources
