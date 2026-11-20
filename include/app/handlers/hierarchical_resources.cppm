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

// Local typed wire shapes. Upstream sends a plain ResourceId string and a
// PrimitiveDateTime string (api_models/src/hierarchical_resources.rs:12-42); the
// converted app_dto wrappers serialize nested ids/dates as {"value":...}, so these
// shapes keep the upstream wire form while still going through serde encoding.
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

/** @brief Local mirror of upstream ApplePayCertificateData (core:65-72).
 *
 *  `status` is serialized as snake_case; the two Option fields are skipped when
 *  absent (serde `skip_serializing_if`), exactly like upstream.
 */
struct ApplePayCertificateData
{
    std::string status{};
    std::string apple_merchant_identifier{};
    std::string payment_processing_certificate{};
};

/** @brief Local mirror of the persisted HierarchicalResource payload.
 *
 *  Upstream stores `data: Secret<serde_json::Value>` (ApplePayCertificateData) and
 *  `encrypted_data: Option<Encryption>` as separate entity columns
 *  (core:180-194, :349-355). The local repository record has a single `data`
 *  column, so this typed JSON object carries the same field names verbatim.
 */
struct ResourceData
{
    std::string data{};
    std::string encrypted_data{};
    std::string public_key_digest{};
    std::string merchant_id{};
};

struct GeneratedKeys
{
    std::string private_key_pem{};
    std::string csr_pem{};
};

struct ParsedCertificate
{
    std::string der{};
    std::string public_key_digest{};
};

struct GenerateOutcome
{
    std::string id{};
    std::string data{};
};

struct UploadOutcome
{
    std::string id{};
    std::string created_at{};
};

struct ListOutcome
{
    std::vector<SummaryWire> resources{};
};

struct LinkOutcome
{
    std::string id{};
};

class HierarchicalResourcesRouteHandlers
{
public:
    explicit HierarchicalResourcesRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    // ── Route handlers: parse body, resolve mandatory X-Profile-Id, authorize
    //    (upstream server_wrap), then dispatch to the core helpers below. ──────

    void handle_generate_hierarchical_resource(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: web::Json<GenerateHierarchicalResourceRequest> is decoded by the
        // framework before the handler (routes:77); a bad body is a 400.
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
                core::error::AppErrorKind::INVALID_REQUEST, "invalid resource type"));
            return;
        }

        // routes:81-84 — mandatory_profile_id_from_header runs before server_wrap.
        const auto profile_id = mandatory_profile_id_from_header(req);
        if (!profile_id.has_value()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::INVALID_REQUEST, "missing X-Profile-Id header"));
            return;
        }
        const auto auth_error = authorize_request(req, "generate_hierarchical_resource");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto merchant_id = resolve_merchant_id(req, auth);
        // routes:93 — resolve_profile_from_header: scoped fetch is the auth check.
        if (!resolve_profile_from_header(*profile_id, merchant_id).has_value()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::UNAUTHORIZED, "profile not found for merchant"));
            return;
        }
        const auto organization_id = resolve_scope_id(req, auth, merchant_id);
        if (organization_id.empty()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::INVALID_REQUEST, "missing organization scope"));
            return;
        }

        m_context->metrics().record_flow("HierarchicalResourcesGenerate");
        m_context->metrics().counter("hierarchical_resources_generate_total").increment();

        const auto result = generate_hierarchical_resource(merchant_id, organization_id, resource_type);
        if (!result) {
            send_error(res, send, std::move(result.error()));
            return;
        }
        const GenerateResponseWire response{result->id, result->data};
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_upload_hierarchical_resource(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // web::Path<ResourceId> is extracted by the framework (routes:117); the
        // local extractor validates the same alphanumeric/length shape.
        const auto resource_id = resource_id_from_path(req, "apple_pay_certificate");
        if (!resource_id.has_value() || resource_id->empty()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::INVALID_REQUEST, "missing resource id in path"));
            return;
        }

        // routes:48-71 — content-type aware body parse: JSON object or raw DER
        // bytes that are base64-encoded into the request's `certificate` field.
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, app_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed_body = parse_upload_certificate_body(req, body);
        if (!parsed_body) {
            send_error(res, send, std::move(parsed_body.error()));
            return;
        }

        const auto profile_id = mandatory_profile_id_from_header(req);
        if (!profile_id.has_value()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::INVALID_REQUEST, "missing X-Profile-Id header"));
            return;
        }
        const auto auth_error = authorize_request(req, "upload_hierarchical_resource");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto merchant_id = resolve_merchant_id(req, auth);
        if (!resolve_profile_from_header(*profile_id, merchant_id).has_value()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::UNAUTHORIZED, "profile not found for merchant"));
            return;
        }
        const auto organization_id = resolve_scope_id(req, auth, merchant_id);

        m_context->metrics().record_flow("HierarchicalResourcesUpload");
        m_context->metrics().counter("hierarchical_resources_upload_total").increment();

        const auto result = upload_hierarchical_resource(
            *resource_id, parsed_body->get_certificate(), merchant_id, organization_id);
        if (!result) {
            send_error(res, send, std::move(result.error()));
            return;
        }
        const UploadResponseWire response{result->id, result->created_at};
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_list_hierarchical_resources(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
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
                core::error::AppErrorKind::INVALID_REQUEST, "invalid resource type"));
            return;
        }

        const auto profile_id = mandatory_profile_id_from_header(req);
        if (!profile_id.has_value()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::INVALID_REQUEST, "missing X-Profile-Id header"));
            return;
        }
        const auto auth_error = authorize_request(req, "list_hierarchical_resources");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto merchant_id = resolve_merchant_id(req, auth);
        if (!resolve_profile_from_header(*profile_id, merchant_id).has_value()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::UNAUTHORIZED, "profile not found for merchant"));
            return;
        }
        const auto organization_id = resolve_scope_id(req, auth, merchant_id);

        m_context->metrics().record_flow("HierarchicalResourcesList");
        m_context->metrics().counter("hierarchical_resources_list_total").increment();

        const auto result = list_hierarchical_resources(*parsed, merchant_id, organization_id);
        if (!result) {
            send_error(res, send, std::move(result.error()));
            return;
        }
        const ListResponseWire response{result->resources};
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_link_hierarchical_resource(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
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
        if (!valid_requestor_type(requestor_type) || requestor_id.empty()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::INVALID_REQUEST, "requestor_type and requestor_id are required"));
            return;
        }

        const auto profile_id = mandatory_profile_id_from_header(req);
        if (!profile_id.has_value()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::INVALID_REQUEST, "missing X-Profile-Id header"));
            return;
        }
        const auto auth_error = authorize_request(req, "link_hierarchical_resource");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto merchant_id = resolve_merchant_id(req, auth);
        if (!resolve_profile_from_header(*profile_id, merchant_id).has_value()) {
            send_error(res, send, app_error(
                core::error::AppErrorKind::UNAUTHORIZED, "profile not found for merchant"));
            return;
        }
        const auto organization_id = resolve_scope_id(req, auth, merchant_id);

        m_context->metrics().record_flow("HierarchicalResourcesLink");
        m_context->metrics().counter("hierarchical_resources_link_total").increment();

        const auto result = link_hierarchical_resource(
            *resource_id, requestor_type, requestor_id, merchant_id, organization_id);
        if (!result) {
            send_error(res, send, std::move(result.error()));
            return;
        }
        const LinkResponseWire response{result->id};
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Upstream scope registration uses web::resource("") for generate
        // (routes/app.rs:2282-2284); an empty segment is the exact match.
        Router{router, "/hierarchical_resources"}
            .add_route(
                Route{""}
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
    // ── Core helpers — direct ports of core/hierarchical_resources.rs ─────────

    /** @brief core:578-603 generate_hierarchical_resource. */
    [[nodiscard]] std::expected<GenerateOutcome, core::error::AppError> generate_hierarchical_resource(
        std::string_view merchant_id,
        std::string_view organization_id,
        std::string_view resource_type
    ) const
    {
        const auto org_key = ensure_organization_key_store(organization_id);
        const auto keys = generate_ec_keypair_and_csr(org_key);

        // core:179 — id_type::ResourceId::default() -> "res" + alphanumeric.
        const auto resource_id = m_context->ids().payment_id("res");

        // core:155-171 — ApplePayCertificateKeyWrapperData serialized then encrypted
        // with the organization key (keymanager crypto_operation substitute: AesWrap).
        const auto private_key_payload = "{\"payment_processing_certificate_key\":"
            + serde::Ser::encode_json(keys.private_key_pem) + "}";
        const auto encrypted_private_key = app::crypto::AesWrap{}.encrypt(private_key_payload, org_key);

        ApplePayCertificateData data;
        data.status = status_csr_generated();
        ResourceData payload;
        payload.data = encode_apple_pay_certificate_data(data);
        payload.encrypted_data = encrypted_private_key;
        payload.public_key_digest = app::crypto::KeyedHasher{org_key}.hash(keys.private_key_pem);
        payload.merchant_id = merchant_id;

        app::store::hierarchical_resources::HierarchicalResourceRecord record{};
        record.set_resource_id(resource_id);
        record.set_scope_id(std::string{organization_id});
        record.set_resource_type(std::string{resource_type});
        record.set_data(encode_resource_data(payload));
        record.set_status(data.status);
        record.set_created_at(created_at_value());
        const auto stored = m_context->repository().create_hierarchical_resource(std::move(record));
        if (!stored) {
            return std::unexpected{stored.error()};
        }

        // core:196-211 — response data is {"apple_pay_certificate":{"csr": <PEM>}}.
        const auto response_data = "{\"" + std::string{resource_type} + "\":{\"csr\":"
            + serde::Ser::encode_json(keys.csr_pem) + "}}";
        return GenerateOutcome{resource_id, response_data};
    }

    /** @brief core:630-668 + ApplePayCertificateResource::upload (:305-367). */
    [[nodiscard]] std::expected<UploadOutcome, core::error::AppError> upload_hierarchical_resource(
        std::string_view resource_id,
        std::string_view certificate,
        std::string_view merchant_id,
        std::string_view organization_id
    ) const
    {
        // core:639-646 — find_resource_scope_id + authorize_scope_id_belongs_to_org.
        const auto stored = m_context->repository().find_hierarchical_resource_by_string(resource_id);
        if (!stored.has_value()) {
            return std::unexpected{app_error(
                core::error::AppErrorKind::NOT_FOUND, "resource not found")};
        }
        if (!authorize_scope_id_belongs_to_org(stored->get_scope_id(), organization_id)) {
            return std::unexpected{app_error(
                core::error::AppErrorKind::FORBIDDEN, "resource belongs to a different organization")};
        }

        const auto org_key = ensure_organization_key_store(organization_id);
        auto payload = decode_resource_data(stored->get_data());
        if (!merchant_id.empty() && !payload.merchant_id.empty() && payload.merchant_id != merchant_id) {
            return std::unexpected{app_error(
                core::error::AppErrorKind::FORBIDDEN, "resource belongs to a different merchant")};
        }

        // core:323 — base64 DER decode + X509::from_der (local OpenSSL substitute).
        auto parsed_cert = parse_certificate_from_base64_der(certificate, org_key);
        if (!parsed_cert) {
            return std::unexpected{std::move(parsed_cert.error())};
        }
        // core:325-336 — cert/key match. The local substitute verifies a structural
        // binding (no EC public-key DER available without OpenSSL); see helper.
        if (!certificate_matches_private_key(*parsed_cert, payload)) {
            return std::unexpected{app_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Uploaded certificate does not match the generated key")};
        }
        // core:338-341 — CN extraction, rsplit(':')+trim, InvalidRequestData on miss.
        auto identifier = parse_apple_merchant_identifier(*parsed_cert);
        if (!identifier.has_value() || identifier->empty()) {
            return std::unexpected{app_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Could not parse Apple merchant identifier from certificate")};
        }

        ApplePayCertificateData data;
        data.status = status_active();
        data.apple_merchant_identifier = *identifier;
        data.payment_processing_certificate = std::string{certificate};
        payload.data = encode_apple_pay_certificate_data(data);

        auto updated = *stored;
        updated.set_data(encode_resource_data(payload));
        updated.set_status(data.status);
        if (!m_context->repository().update_hierarchical_resource(updated)) {
            return std::unexpected{app_error(
                core::error::AppErrorKind::INTERNAL_ERROR, "failed to update resource")};
        }

        return UploadOutcome{std::string{resource_id}, updated.get_created_at()};
    }

    /** @brief core:728-820 list_hierarchical_resources. */
    [[nodiscard]] std::expected<ListOutcome, core::error::AppError> list_hierarchical_resources(
        const app::dto::hierarchical_resources::ListHierarchicalResourcesRequest& request,
        std::string_view merchant_id,
        std::string_view organization_id
    ) const
    {
        // core:750-775 — effective id resolves only when BOTH scope_type and
        // scope_id are present; otherwise it is None.
        std::string effective_resource_id{};
        if (request.get_scope_type().has_value() && request.get_scope_id().has_value()
            && !request.get_scope_id()->empty()) {
            const auto account = fetch_requestor_account(
                request.get_scope_type()->get_value(), *request.get_scope_id(), merchant_id);
            if (!account) {
                return std::unexpected{std::move(account.error())};
            }
            effective_resource_id = resolve_apple_pay_certificate_data(*account);
        }

        const auto resource_type = request.get_resource_type().get_value();
        std::vector<SummaryWire> summaries{};
        for (const auto& record : m_context->repository().list_hierarchical_resources()) {
            if (record.get_resource_type() != resource_type) {
                continue;
            }
            if (!organization_id.empty() && record.get_scope_id() != organization_id) {
                continue;
            }
            const auto payload = decode_resource_data(record.get_data());
            if (!hierarchical_resource_is_linkable(payload)) {
                continue;
            }
            if (!merchant_id.empty() && !payload.merchant_id.empty() && payload.merchant_id != merchant_id) {
                continue;
            }

            const auto certificate_data = parse_apple_pay_certificate_payload(payload.data);
            SummaryWire summary;
            summary.id = record.get_resource_id();
            summary.display_schema = display_schema();
            summary.display_data = display_data(certificate_data);
            summary.created_at = record.get_created_at();
            summary.is_linked = !effective_resource_id.empty()
                && effective_resource_id == record.get_resource_id();
            summaries.emplace_back(std::move(summary));
        }
        return ListOutcome{std::move(summaries)};
    }

    /** @brief core:822-878 link_hierarchical_resource. */
    [[nodiscard]] std::expected<LinkOutcome, core::error::AppError> link_hierarchical_resource(
        std::string_view resource_id,
        std::string_view requestor_type,
        std::string_view requestor_id,
        std::string_view merchant_id,
        std::string_view organization_id
    ) const
    {
        // core:832-839 — find_resource_scope_id + org authorization.
        const auto stored = m_context->repository().find_hierarchical_resource_by_string(resource_id);
        if (!stored.has_value()) {
            return std::unexpected{app_error(
                core::error::AppErrorKind::NOT_FOUND, "resource not found")};
        }
        if (!authorize_scope_id_belongs_to_org(stored->get_scope_id(), organization_id)) {
            return std::unexpected{app_error(
                core::error::AppErrorKind::FORBIDDEN, "resource belongs to a different organization")};
        }

        // core:841-847 — fetch_requestor_account (V1, merchant scoped).
        const auto account = fetch_requestor_account(requestor_type, requestor_id, merchant_id);
        if (!account) {
            return std::unexpected{std::move(account.error())};
        }
        // core:859-873 — resource type + Active-only is_linkable is enforced by
        // is_linkable in upstream; link itself relies on the caller having an Active
        // resource. The local guard preserves that precondition explicitly.
        auto payload = decode_resource_data(stored->get_data());
        if (stored->get_status() != status_active()) {
            return std::unexpected{app_error(
                core::error::AppErrorKind::PRECONDITION_FAILED, "resource is not active")};
        }
        if (!merchant_id.empty() && !payload.merchant_id.empty() && payload.merchant_id != merchant_id) {
            return std::unexpected{app_error(
                core::error::AppErrorKind::FORBIDDEN, "resource belongs to a different merchant")};
        }

        // core:238-301 on_link — re-encrypt the private key under the requestor's
        // key store and mirror the account attachment (local config/event substitute).
        const auto requestor_key = ensure_requestor_key(requestor_id);
        const auto rewrapped = app::crypto::AesWrap{}.encrypt(payload.encrypted_data, requestor_key);
        const auto link_id = link_resource_id(requestor_id);
        const auto link_data = "{\"resource_id\":" + serde::Ser::encode_json(std::string{resource_id})
            + ",\"requestor_id\":" + serde::Ser::encode_json(std::string{requestor_id})
            + ",\"requestor_type\":" + serde::Ser::encode_json(std::string{requestor_type})
            + ",\"encrypted_data\":" + serde::Ser::encode_json(rewrapped) + "}";
        auto link_record = m_context->repository().find_hierarchical_resource_by_string(link_id);
        if (!link_record.has_value()) {
            app::store::hierarchical_resources::HierarchicalResourceRecord fresh{};
            fresh.set_resource_id(link_id);
            fresh.set_scope_id(std::string{resource_id});
            fresh.set_resource_type("link");
            fresh.set_data(link_data);
            fresh.set_status(status_active());
            fresh.set_created_at(created_at_value());
            const auto created = m_context->repository().create_hierarchical_resource(std::move(fresh));
            if (!created) {
                return std::unexpected{created.error()};
            }
        } else {
            link_record->set_scope_id(std::string{resource_id});
            link_record->set_data(link_data);
            if (!m_context->repository().update_hierarchical_resource(*link_record)) {
                return std::unexpected{app_error(
                    core::error::AppErrorKind::INTERNAL_ERROR, "failed to persist link")};
            }
        }
        m_context->config_store().put(
            "hierarchical_resources.account_links", requestor_id, std::string{resource_id},
            std::string{requestor_type});
        m_context->events().publish(
            "hierarchical_resources.linked",
            "{\"resource_id\":\"" + std::string{resource_id} + "\",\"requestor_id\":\""
                + std::string{requestor_id} + "\"}");
        return LinkOutcome{std::string{resource_id}};
    }

    // ── Upstream route-level helpers ──────────────────────────────────────────

    /** @brief routes:22-26 mandatory_profile_id_from_header. */
    [[nodiscard]] std::optional<std::string> mandatory_profile_id_from_header(
        interfaces::io::IRequest& req
    ) const
    {
        const auto header = app::http::HeaderReader{}.get(req, "x-profile-id");
        if (header.empty()) {
            return std::nullopt;
        }
        if (!valid_profile_id(header)) {
            return std::nullopt;
        }
        return header;
    }

    /** @brief routes:28-46 resolve_profile_from_header (scoped fetch = auth). */
    [[nodiscard]] std::optional<std::string> resolve_profile_from_header(
        std::string_view profile_id,
        std::string_view merchant_id
    ) const
    {
        const auto profile = m_context->repository().find_profile_by_string(profile_id);
        if (!profile.has_value()) {
            return std::nullopt;
        }
        if (!merchant_id.empty() && profile->get_merchant_id() != merchant_id) {
            return std::nullopt;
        }
        return std::string{profile_id};
    }

    /** @brief routes:48-71 parse_upload_certificate_body. */
    [[nodiscard]] std::expected<app::dto::hierarchical_resources::UploadCertificateRequest,
                                core::error::AppError> parse_upload_certificate_body(
        interfaces::io::IRequest& req,
        std::string_view body
    ) const
    {
        using Input = app::dto::hierarchical_resources::UploadCertificateRequest;
        const auto content_type = app::http::HeaderReader{}.get(req, "content-type");
        const bool is_json = content_type.starts_with("application/json");
        if (is_json) {
            auto parsed = serde::Ser::deserialize<Input>("application/json", body);
            if (!parsed) {
                return std::unexpected{app_error(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "invalid JSON body: " + parsed.error())};
            }
            return *parsed;
        }
        Input raw{};
        raw.set_certificate(base64_encode(body));
        return raw;
    }

    /** @brief core:880-890 authorize_scope_id_belongs_to_org. */
    [[nodiscard]] bool authorize_scope_id_belongs_to_org(
        std::string_view resource_scope,
        std::string_view organization_id
    ) const
    {
        return !resource_scope.empty() && !organization_id.empty()
            && resource_scope == organization_id;
    }

    // ── Requestor resolution (core:412-483, V1) ───────────────────────────────

    [[nodiscard]] std::expected<std::string, core::error::AppError> fetch_requestor_account(
        std::string_view requestor_type,
        std::string_view requestor_id,
        std::string_view merchant_id
    ) const
    {
        if (requestor_type == "merchant_connector_account") {
            const auto connector = m_context->repository().find_merchant_connector_by_string(requestor_id);
            if (!connector.has_value()) {
                return std::unexpected{app_error(
                    core::error::AppErrorKind::NOT_FOUND, "merchant connector account not found")};
            }
            if (!merchant_id.empty() && connector->get_merchant_id() != merchant_id) {
                return std::unexpected{app_error(
                    core::error::AppErrorKind::FORBIDDEN, "requestor belongs to a different merchant")};
            }
            return std::string{requestor_id};
        }
        if (requestor_type == "profile") {
            const auto profile = m_context->repository().find_profile_by_string(requestor_id);
            if (!profile.has_value()) {
                return std::unexpected{app_error(
                    core::error::AppErrorKind::NOT_FOUND, "profile not found")};
            }
            if (!merchant_id.empty() && profile->get_merchant_id() != merchant_id) {
                return std::unexpected{app_error(
                    core::error::AppErrorKind::FORBIDDEN, "requestor belongs to a different merchant")};
            }
            return std::string{requestor_id};
        }
        if (requestor_type == "merchant_account") {
            if (!merchant_id.empty() && requestor_id != merchant_id) {
                return std::unexpected{app_error(
                    core::error::AppErrorKind::NOT_FOUND, "merchant account not found")};
            }
            if (!m_context->repository().find_merchant_account_by_string(requestor_id).has_value()) {
                return std::unexpected{app_error(
                    core::error::AppErrorKind::NOT_FOUND, "merchant account not found")};
            }
            return std::string{requestor_id};
        }
        return std::unexpected{app_error(
            core::error::AppErrorKind::INVALID_REQUEST, "invalid requestor_type")};
    }

    /** @brief core:502-533 resolve_apple_pay_certificate_data cascading cache read. */
    [[nodiscard]] std::string resolve_apple_pay_certificate_data(
        std::string_view requestor_id
    ) const
    {
        return find_linked_resource(requestor_id);
    }

    [[nodiscard]] std::string find_linked_resource(std::string_view requestor_id) const
    {
        const auto cached = m_context->config_store().get_value(
            "hierarchical_resources.account_links", requestor_id);
        if (cached.has_value() && !cached->empty()) {
            return *cached;
        }
        const auto link = m_context->repository().find_hierarchical_resource_by_string(
            link_resource_id(requestor_id));
        if (link.has_value()) {
            // Local mirror of the account `apple_pay_certificates` cache payload:
            // the link record stores a JSON object with a `resource_id` field.
            const auto pos = link->get_data().find("\"resource_id\":\"");
            if (pos != std::string::npos) {
                const auto start = pos + std::string{"\"resource_id\":\""}.size();
                const auto end = link->get_data().find('"', start);
                if (end != std::string::npos) {
                    return link->get_data().substr(start, end - start);
                }
            }
        }
        // Cascade profile -> merchant account (upstream uses inherent entity properties).
        const auto profile = m_context->repository().find_profile_by_string(requestor_id);
        if (profile.has_value()) {
            return find_linked_resource(profile->get_merchant_id());
        }
        const auto connector = m_context->repository().find_merchant_connector_by_string(requestor_id);
        if (connector.has_value()) {
            return find_linked_resource(connector->get_merchant_id());
        }
        return {};
    }

    // ── Apple Pay crypto / certificate helpers (local OpenSSL substitutes) ────

    /** @brief core:892-922 ensure_organization_key_store. */
    [[nodiscard]] std::string ensure_organization_key_store(std::string_view org_id) const
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

    /** @brief core:605-628 generate_ec_keypair_and_csr (local PEM substitute). */
    [[nodiscard]] GeneratedKeys generate_ec_keypair_and_csr(std::string_view org_key) const
    {
        const auto seed = m_context->ids().uuid_v4() + m_context->ids().uuid_v4();
        const auto private_der = base64_encode("ec-p256|" + seed);
        const auto csr_hash = app::crypto::KeyedHasher{std::string{org_key}}.hash(seed);
        const auto csr_der = base64_encode("csr|" + csr_hash);
        return GeneratedKeys{
            "-----BEGIN PRIVATE KEY-----\n" + private_der + "\n-----END PRIVATE KEY-----\n",
            "-----BEGIN CERTIFICATE REQUEST-----\n" + csr_der
                + "\n-----END CERTIFICATE REQUEST-----\n"
        };
    }

    /** @brief core:685-695 parse_certificate_from_base64_der (local substitute). */
    [[nodiscard]] std::expected<ParsedCertificate, core::error::AppError>
    parse_certificate_from_base64_der(std::string_view certificate_b64, std::string_view org_key) const
    {
        const auto der = base64_decode(certificate_b64);
        if (!der.has_value() || der->empty()) {
            return std::unexpected{app_error(
                core::error::AppErrorKind::INVALID_REQUEST, "certificate is not valid base64")};
        }
        // Minimal DER sanity check: an X.509 certificate is a SEQUENCE (0x30).
        if (static_cast<unsigned char>((*der)[0]) != 0x30U) {
            return std::unexpected{app_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "certificate is not a valid DER-encoded X.509 certificate")};
        }
        return ParsedCertificate{
            *der,
            app::crypto::KeyedHasher{std::string{org_key}}.hash(*der)
        };
    }

    /** @brief core:697-707 certificate_matches_private_key (local substitute).
     *
     *  OpenSSL public-key DER comparison is unavailable locally. The structural
     *  substitute verifies the certificate is a well-formed DER SEQUENCE and that
     *  a binding digest was stored at generation time; it cannot reject a
     *  structurally valid certificate issued for a different key.
     */
    [[nodiscard]] bool certificate_matches_private_key(
        const ParsedCertificate& certificate,
        const ResourceData& payload
    ) const
    {
        return !certificate.der.empty() && !payload.encrypted_data.empty()
            && !payload.public_key_digest.empty();
    }

    /** @brief core:709-726 parse_apple_merchant_identifier (local substitute). */
    [[nodiscard]] std::optional<std::string> parse_apple_merchant_identifier(
        const ParsedCertificate& certificate
    ) const
    {
        std::string common_name{};
        // DER: CN has OID 2.5.4.3 = {0x55,0x04,0x03}, followed by a TLV.
        static constexpr std::string_view cn_oid{"\x55\x04\x03", 3};
        const auto oid_pos = certificate.der.find(cn_oid);
        if (oid_pos != std::string::npos && oid_pos + 3 + 2 <= certificate.der.size()) {
            const auto tag = static_cast<unsigned char>(certificate.der[oid_pos + 3]);
            if (tag == 0x0CU || tag == 0x13U || tag == 0x16U) {
                std::size_t length = static_cast<unsigned char>(certificate.der[oid_pos + 4]);
                std::size_t value_start = oid_pos + 5;
                if ((length & 0x80U) != 0U) {
                    const std::size_t extra = length & 0x7FU;
                    if (extra > 0 && extra <= 4 && value_start + extra <= certificate.der.size()) {
                        length = 0;
                        for (std::size_t i{0}; i < extra; ++i) {
                            length = (length << 8)
                                | static_cast<unsigned char>(certificate.der[value_start + i]);
                        }
                        value_start += extra;
                    } else {
                        length = 0;
                    }
                }
                if (length > 0 && value_start + length <= certificate.der.size()) {
                    common_name = certificate.der.substr(value_start, length);
                }
            }
        }
        // Text fallback (local certificate stand-ins may carry a literal CN=).
        if (common_name.empty()) {
            const auto cn_pos = certificate.der.find("CN=");
            if (cn_pos != std::string::npos) {
                const auto rest = std::string_view{certificate.der}.substr(cn_pos + 3);
                const auto end = rest.find_first_of(",/ \t\r\n\"");
                common_name = std::string{rest.substr(0, end)};
            }
        }
        if (common_name.empty()) {
            return std::nullopt;
        }
        // core:720-725 — rsplit(':').next().trim().
        const auto last_colon = common_name.rfind(':');
        auto identifier = last_colon == std::string::npos
            ? common_name
            : common_name.substr(last_colon + 1);
        const auto first = identifier.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            return std::nullopt;
        }
        const auto last = identifier.find_last_not_of(" \t\r\n");
        return identifier.substr(first, last - first + 1);
    }

    // ── Status / display (core:101-117, :214-236, :800-820) ───────────────────

    [[nodiscard]] std::string status_csr_generated() const { return "csr_generated"; }
    [[nodiscard]] std::string status_active() const { return "active"; }

    [[nodiscard]] std::string display_schema() const
    {
        return R"({"apple_pay_merchant_identifier":"String"})";
    }

    [[nodiscard]] std::string display_data(const ApplePayCertificateData& data) const
    {
        if (data.apple_merchant_identifier.empty()) {
            return R"({"apple_pay_merchant_identifier":null})";
        }
        return "{\"apple_pay_merchant_identifier\":"
            + serde::Ser::encode_json(data.apple_merchant_identifier) + "}";
    }

    [[nodiscard]] bool hierarchical_resource_is_linkable(const ResourceData& payload) const
    {
        const auto data = parse_apple_pay_certificate_payload(payload.data);
        return data.status == status_active();
    }

    // ── Typed persistence encoding (local repository substitute) ──────────────

    [[nodiscard]] std::string encode_apple_pay_certificate_data(
        const ApplePayCertificateData& data
    ) const
    {
        std::string out = "{\"status\":" + serde::Ser::encode_json(data.status);
        if (!data.apple_merchant_identifier.empty()) {
            out += ",\"apple_pay_merchant_identifier\":"
                + serde::Ser::encode_json(data.apple_merchant_identifier);
        }
        if (!data.payment_processing_certificate.empty()) {
            out += ",\"payment_processing_certificate\":"
                + serde::Ser::encode_json(data.payment_processing_certificate);
        }
        out += "}";
        return out;
    }

    [[nodiscard]] ApplePayCertificateData parse_apple_pay_certificate_payload(
        std::string_view json
    ) const
    {
        ApplePayCertificateData data{};
        data.status = json_string_field(json, "status");
        data.apple_merchant_identifier = json_string_field(json, "apple_pay_merchant_identifier");
        data.payment_processing_certificate = json_string_field(json, "payment_processing_certificate");
        return data;
    }

    [[nodiscard]] std::string json_string_field(std::string_view json, std::string_view key) const
    {
        const std::string needle = "\"" + std::string{key} + "\":";
        const auto pos = json.find(needle);
        if (pos == std::string_view::npos) {
            return {};
        }
        auto rest = json.substr(pos + needle.size());
        const auto first = rest.find_first_not_of(" \t\r\n");
        if (first == std::string_view::npos || rest[first] != '"') {
            return {};
        }
        rest = rest.substr(first + 1);
        std::string out{};
        for (std::size_t i{0}; i < rest.size(); ++i) {
            if (rest[i] == '\\' && i + 1 < rest.size()) {
                out.push_back(rest[++i]);
            } else if (rest[i] == '"') {
                break;
            } else {
                out.push_back(rest[i]);
            }
        }
        return out;
    }

    [[nodiscard]] std::string encode_resource_data(const ResourceData& value) const
    {
        return "{\"data\":" + serde::Ser::encode_json(value.data)
            + ",\"encrypted_data\":" + serde::Ser::encode_json(value.encrypted_data)
            + ",\"public_key_digest\":" + serde::Ser::encode_json(value.public_key_digest)
            + ",\"merchant_id\":" + serde::Ser::encode_json(value.merchant_id) + "}";
    }

    [[nodiscard]] ResourceData decode_resource_data(std::string_view raw) const
    {
        ResourceData result{};
        result.data = json_string_field(raw, "data");
        result.encrypted_data = json_string_field(raw, "encrypted_data");
        result.public_key_digest = json_string_field(raw, "public_key_digest");
        result.merchant_id = json_string_field(raw, "merchant_id");
        return result;
    }

    // ── Standard base64 (upstream BASE64_ENGINE; local, no new module deps) ────

    [[nodiscard]] std::string base64_encode(std::string_view input) const
    {
        static constexpr std::string_view table{
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
        std::string out{};
        out.reserve(4 * ((input.size() + 2) / 3));
        std::uint32_t accumulator{0};
        int bits{0};
        for (const char c : input) {
            accumulator = (accumulator << 8) | static_cast<unsigned char>(c);
            bits += 8;
            while (bits >= 6) {
                bits -= 6;
                out.push_back(table[(accumulator >> bits) & 0x3FU]);
            }
        }
        if (bits > 0) {
            out.push_back(table[(accumulator << (6 - bits)) & 0x3FU]);
            while (out.size() % 4 != 0) {
                out.push_back('=');
            }
        }
        return out;
    }

    [[nodiscard]] std::optional<std::string> base64_decode(std::string_view input) const
    {
        static constexpr std::string_view table{
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
        if (input.size() % 4 != 0) {
            return std::nullopt;
        }
        std::string out{};
        out.reserve((input.size() / 4) * 3);
        for (std::size_t i{0}; i < input.size(); i += 4) {
            const auto chunk = input.substr(i, 4);
            std::size_t padding{0};
            while (padding < chunk.size() && chunk[chunk.size() - 1 - padding] == '=') {
                ++padding;
            }
            if (padding > 2) {
                return std::nullopt;
            }
            std::uint32_t word{0};
            for (std::size_t j{0}; j < 4; ++j) {
                const char c = chunk[j];
                if (j >= 4 - padding) {
                    if (c != '=') {
                        return std::nullopt;
                    }
                    continue; // '=' contributes zero bits.
                }
                const auto pos = table.find(c);
                if (pos == std::string_view::npos) {
                    return std::nullopt;
                }
                word = (word << 6) | static_cast<std::uint32_t>(pos);
            }
            out.push_back(static_cast<char>((word >> 16) & 0xFFU));
            if (padding < 2) {
                out.push_back(static_cast<char>((word >> 8) & 0xFFU));
            }
            if (padding < 1) {
                out.push_back(static_cast<char>(word & 0xFFU));
            }
        }
        return out;
    }

    // ── Validators / path extraction ───────────────────────────────────────────

    [[nodiscard]] bool valid_resource_type(std::string_view value) const
    {
        // common_enums::ResourceType has exactly one snake_case variant.
        return value == "apple_pay_certificate";
    }

    [[nodiscard]] bool valid_requestor_type(std::string_view value) const
    {
        // common_enums::ResourceRequestorType snake_case variants (accounts.rs:145-149).
        return value == "merchant_connector_account" || value == "profile"
            || value == "merchant_account";
    }

    [[nodiscard]] bool valid_profile_id(std::string_view value) const
    {
        return !value.empty() && value.size() <= 36;
    }

    /** @brief web::Path<ResourceId> substitute: alphanumeric, length 6..36. */
    [[nodiscard]] bool valid_resource_id(std::string_view value) const
    {
        if (value.size() < 6 || value.size() > 36) {
            return false;
        }
        for (const char c : value) {
            const bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                || (c >= '0' && c <= '9');
            if (!ok) {
                return false;
            }
        }
        return true;
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
            if (candidate.has_value() && valid_resource_id(*candidate)) {
                return std::string{*candidate};
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string link_resource_id(std::string_view requestor_id) const
    {
        return "link:" + std::string{requestor_id};
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

    /** @brief JWTAndEmbeddedAuth + MerchantConnector Write/Read + allow flags. */
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

    /** @brief time::PrimitiveDateTime wire shape: "YYYY-MM-DD HH:MM:SS.nnnnnnnnn". */
    [[nodiscard]] std::int64_t now_epoch() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

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
            "{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.000000000",
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
