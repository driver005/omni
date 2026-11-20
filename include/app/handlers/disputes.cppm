module;
export module app_handlers:disputes;

import std;
import interfaces;
import core_router;
import core_serde;
import core_cache;
import core_error;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import app_http;
import app_metrics;
import app_store;
import app_connector;
import app_workflow;
import app_events;
import app_locking;
import app_services;

export namespace app::handlers::disputes {

/** @brief Upstream StatusOk wire shape (used by delete-evidence). */
class StatusOkWire
{
public:
    void set_status(std::string value) { status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return status; }
    std::string status{};
};

/**
 * @brief Local serializable shape for the multipart evidence upload body.
 * @note The local HTTP layer carries JSON, not multipart. This normalizes the
 * file metadata that upstream `files::files_create_core` derives from a part.
 */
class EvidenceUploadWire
{
public:
    void set_dispute_id(std::optional<std::string> value) { dispute_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_dispute_id() const noexcept { return dispute_id; }
    void set_file_id(std::optional<std::string> value) { file_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_file_id() const noexcept { return file_id; }
    void set_file_name(std::optional<std::string> value) { file_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_file_name() const noexcept { return file_name; }
    void set_evidence_type(std::optional<std::string> value) { evidence_type = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_evidence_type() const noexcept { return evidence_type; }
    std::optional<std::string> dispute_id{};
    std::optional<std::string> file_id{};
    std::optional<std::string> file_name{};
    std::optional<std::string> evidence_type{};
};

} // namespace app::handlers::disputes

template<>
struct serde::Serializable<app::handlers::disputes::StatusOkWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"status", &app::handlers::disputes::StatusOkWire::get_status, &app::handlers::disputes::StatusOkWire::set_status, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "status_ok_wire"; }
};

template<>
struct serde::Serializable<app::handlers::disputes::EvidenceUploadWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"dispute_id", &app::handlers::disputes::EvidenceUploadWire::get_dispute_id, &app::handlers::disputes::EvidenceUploadWire::set_dispute_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"file_id", &app::handlers::disputes::EvidenceUploadWire::get_file_id, &app::handlers::disputes::EvidenceUploadWire::set_file_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"file_name", &app::handlers::disputes::EvidenceUploadWire::get_file_name, &app::handlers::disputes::EvidenceUploadWire::set_file_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"evidence_type", &app::handlers::disputes::EvidenceUploadWire::get_evidence_type, &app::handlers::disputes::EvidenceUploadWire::set_evidence_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "evidence_upload_wire"; }
};

export namespace app::handlers::disputes {

class DisputesRouteHandlers
{
    using Error = core::error::AppError;
    using Kind = core::error::AppErrorKind;
    using Record = app::store::disputes::DisputeRecord;
    using Query = app::http::QueryParams;
    using Auth = app::auth::AuthContext;

    /** @brief Identity and profile scope resolved from auth plus (legacy) query params. */
    struct Scope
    {
        std::string merchant_id{};
        std::string profile_id{};
        std::string org_id{};
        bool profile_conflict{false};
    };

public:
    void handle_retrieve_dispute(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("retrieve_dispute");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto scope = scope_of(auth_context, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        const std::string dispute_id = path_segment(req);
        if (dispute_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing dispute id"));
            return;
        }

        app::model::disputes::DisputeRetrieveRequest request{};
        request.set_dispute_id(dispute_id);
        if (const auto force_sync = query.get("force_sync")) {
            request.set_force_sync(*force_sync == "true" || *force_sync == "1");
        }

        auto record = m_context->repository().find_dispute_by_string(dispute_id);
        if (!record) {
            m_context->metrics().counter("disputes_not_found").increment();
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::NOT_FOUND, "dispute not found"));
            return;
        }

        m_context->metrics().record_flow("dispute_retrieve");
        m_context->metrics().counter("disputes_retrieve_total").increment();

        // force_sync: Dsync substitute — short-lived dedupe marker + connector sync + webhook.
        if (request.get_force_sync().value_or(false)) {
            const std::string dedupe_key = std::string{"disputes.sync."} + dispute_id;
            if (!m_context->cache().get(dedupe_key).hit) {
                m_context->cache().put(
                    dedupe_key,
                    core::cache::to_cache_value(std::string{"pending"}),
                    core::cache::Ttl{}.seconds(60));
                call_connector("sync", dispute_id, scope, *record, serde::Ser::encode_json(request));
                notify_dispute(*record, "dispute.retrieve.force_sync");
                m_context->metrics().counter("disputes_force_sync_total").increment();
            } else {
                m_context->metrics().counter("disputes_force_sync_deduped").increment();
            }
        }

        auto text = serde::Ser::encode_json(to_response(*record));
        send_json(res, send, text, true);
    }

    void handle_fetch_disputes(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("fetch_disputes");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto scope = scope_of(auth_context, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        const std::string connector_id = path_segment(req, 1);
        if (connector_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing connector id"));
            return;
        }

        // Upstream parses fetch_from/fetch_till (RFC3339); here they are opaque
        // timestamp strings carried into the connector payload and schedule time.
        const auto body = query_json(query, "connector_id", connector_id, {
            "limit", "offset", "dispute_status", "dispute_stage", "reason",
            "connector", "received_time", "force_sync", "fetch_from", "fetch_till"
        });
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }
        (void)parsed;

        m_context->metrics().record_flow("dispute_fetch");
        m_context->metrics().counter("disputes_fetch_total").increment();

        // MCA lookup: connector_id may be a merchant connector id; resolve its name.
        std::string connector_name = connector_id;
        for (const auto& mca : m_context->repository().list_merchant_connectors()) {
            if (mca.get_merchant_connector_id() == connector_id ||
                mca.get_connector_name() == connector_id) {
                connector_name = mca.get_connector_name();
                break;
            }
        }

        // Payment-attempt lookup + schedule-time resolution are represented by a
        // process-tracker task carrying the requested date window.
        const std::string fetch_from = query.get("fetch_from").value_or(std::string{});
        const std::string fetch_till = query.get("fetch_till").value_or(std::string{});
        const std::string run_at = fetch_till.empty() ? std::string{"0"} : fetch_till;
        const auto tracker_id = m_context->process_trackers().schedule(
            "dispute_sync",
            std::format(
                "{{\"connector\":\"{}\",\"fetch_from\":\"{}\",\"fetch_till\":\"{}\"}}",
                connector_name, fetch_from, fetch_till),
            run_at);
        m_context->metrics().counter("disputes_fetch_scheduled").increment();

        Record probe{};
        probe.set_dispute_id(connector_id);
        probe.set_connector(connector_name);
        probe.set_merchant_id(scope.merchant_id);
        probe.set_profile_id(scope.profile_id);
        probe.set_created_at(now_string());
        call_connector("fetch_disputes", connector_id, scope, probe, body);

        std::vector<app::model::disputes::DisputeResponse> result;
        for (const auto& record : m_context->repository().list_disputes()) {
            if (record.get_connector() == connector_name &&
                (scope.profile_id.empty() || record.get_profile_id() == scope.profile_id)) {
                result.push_back(to_response(record));
            }
        }

        m_context->process_trackers().mark_terminal(tracker_id, "completed");
        auto text = serde::Ser::encode_json(result);
        send_json(res, send, text, true);
    }

    void handle_retrieve_disputes_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("retrieve_disputes_list");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto scope = scope_of(auth_context, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        const auto constraints = list_constraints_from(query, scope);
        m_context->metrics().record_flow("dispute_list");
        m_context->metrics().counter("disputes_list_total").increment();

        const auto tracker_id = schedule_list_task("dispute_list", scope);
        const auto records = filter_disputes(constraints, scope, false);
        m_context->process_trackers().mark_terminal(tracker_id, "completed");

        std::vector<app::model::disputes::DisputeResponse> response{};
        response.reserve(records.size());
        for (const auto& record : records) {
            response.push_back(to_response(record));
        }
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_retrieve_disputes_list_for_platform(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("retrieve_disputes_list_for_platform");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        auto scope = scope_of(auth_context, query, req);
        if (const auto invalid = scope_error(scope, true)) { send_error(res, send, *invalid); return; }

        const auto constraints = list_constraints_from(query, scope);
        if (const auto processor = query.get("processor_merchant_id")) {
            scope.merchant_id = *processor;
        }
        m_context->metrics().record_flow("dispute_platform_list");
        m_context->metrics().counter("disputes_platform_list_total").increment();

        const auto tracker_id = schedule_list_task("dispute_platform_list", scope);
        const auto records = filter_disputes(constraints, scope, true);
        m_context->process_trackers().mark_terminal(tracker_id, "completed");

        app::model::disputes::PlatformDisputeListResponse response{};
        response.set_count(records.size());
        response.set_total_count(static_cast<std::int64_t>(records.size()));
        std::vector<app::model::disputes::PlatformDisputeListItem> items{};
        items.reserve(records.size());
        for (const auto& record : records) {
            items.push_back(to_platform_item(record));
        }
        response.set_data(std::move(items));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_get_platform_disputes_filters(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_platform_disputes_filters");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto scope = scope_of(auth_context, query, req);
        if (const auto invalid = scope_error(scope, true)) { send_error(res, send, *invalid); return; }

        m_context->metrics().record_flow("dispute_platform_filters");
        m_context->metrics().counter("disputes_platform_filters_total").increment();

        auto text = serde::Ser::encode_json(platform_filters_from(scope, true));
        send_json(res, send, text, true);
    }

    void handle_retrieve_disputes_list_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("retrieve_disputes_list_profile");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto scope = scope_of(auth_context, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }
        if (scope.profile_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::VALIDATION_FAILED, "missing profile id"));
            return;
        }

        const auto constraints = list_constraints_from(query, scope);
        m_context->metrics().record_flow("dispute_profile_list");
        m_context->metrics().counter("disputes_profile_list_total").increment();

        const auto tracker_id = schedule_list_task("dispute_profile_list", scope);
        const auto records = filter_disputes(constraints, scope, false);
        m_context->process_trackers().mark_terminal(tracker_id, "completed");

        std::vector<app::model::disputes::DisputeResponse> response{};
        response.reserve(records.size());
        for (const auto& record : records) {
            response.push_back(to_response(record));
        }
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_get_disputes_filters(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_disputes_filters");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto scope = scope_of(auth_context, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        m_context->metrics().record_flow("dispute_filters");
        m_context->metrics().counter("disputes_filters_total").increment();

        auto text = serde::Ser::encode_json(filters_from(scope, false));
        send_json(res, send, text, true);
    }

    void handle_get_disputes_filters_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_disputes_filters_profile");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto scope = scope_of(auth_context, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }
        if (scope.profile_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::VALIDATION_FAILED, "missing profile id"));
            return;
        }

        m_context->metrics().record_flow("dispute_profile_filters");
        m_context->metrics().counter("disputes_profile_filters_total").increment();

        auto text = serde::Ser::encode_json(filters_from(scope, true));
        send_json(res, send, text, true);
    }

    void handle_accept_dispute(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("accept_dispute");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto scope = scope_of(auth_context, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        const std::string dispute_id = path_segment(req);
        if (dispute_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing dispute id"));
            return;
        }
        auto record = m_context->repository().find_dispute_by_string(dispute_id);
        if (!record) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::NOT_FOUND, "dispute not found"));
            return;
        }
        if (const auto guard = accept_guard(*record)) { send_error(res, send, *guard); return; }

        m_context->metrics().record_flow("dispute_accept");
        m_context->metrics().counter("disputes_accept_total").increment();

        const std::string lock_key = std::string{"dispute:accept:"} + dispute_id;
        const auto lock = m_context->locks().acquire(lock_key, std::int64_t{30}, "dispute_accept");
        if (!lock) {
            m_context->metrics().counter("disputes_accept_lock_contended").increment();
            send_error(res, send, core::error::AppErrorBuilder{}(
                Kind::DUPLICATE_REQUEST, "dispute accept already in progress"));
            return;
        }

        call_connector("accept_dispute", dispute_id, scope, *record,
                       query_json(query, "dispute_id", dispute_id, {}));
        m_context->repository().update_dispute(*record);
        notify_dispute(*record, "dispute.accepted");
        m_context->locks().release(lock_key);

        auto text = serde::Ser::encode_json(to_response(*record));
        send_json(res, send, text, true);
    }

    void handle_submit_dispute_evidence(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("submit_dispute_evidence");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto scope = scope_of(auth_context, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::disputes::SubmitEvidenceRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }
        const std::string dispute_id = parsed->get_dispute_id();
        if (dispute_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing dispute id"));
            return;
        }
        auto record = m_context->repository().find_dispute_by_string(dispute_id);
        if (!record) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::NOT_FOUND, "dispute not found"));
            return;
        }
        if (const auto guard = evidence_guard(*record)) { send_error(res, send, *guard); return; }

        m_context->metrics().record_flow("dispute_submit_evidence");
        m_context->metrics().counter("disputes_submit_evidence_total").increment();

        const std::string lock_key = std::string{"dispute:evidence:"} + dispute_id;
        const auto lock = m_context->locks().acquire(lock_key, std::int64_t{30}, "dispute_submit_evidence");
        if (!lock) {
            m_context->metrics().counter("disputes_submit_evidence_lock_contended").increment();
            send_error(res, send, core::error::AppErrorBuilder{}(
                Kind::DUPLICATE_REQUEST, "dispute evidence update already in progress"));
            return;
        }

        const bool defend = query.get("defend").value_or("false") == "true";
        m_context->config_store().put("dispute_evidence_payload", dispute_id, body);
        const std::string raw = std::format("{{\"dispute_id\":\"{}\",\"defend\":{}}}",
                                            dispute_id, defend ? "true" : "false");
        call_connector("submit_evidence", dispute_id, scope, *record, raw);
        m_context->repository().update_dispute(*record);
        notify_dispute(*record, "dispute.evidence_submitted");
        m_context->locks().release(lock_key);

        auto text = serde::Ser::encode_json(to_response(*record));
        send_json(res, send, text, true);
    }

    void handle_attach_dispute_evidence(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("attach_dispute_evidence");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto scope = scope_of(auth_context, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::deserialize<EvidenceUploadWire>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }
        std::string dispute_id = parsed->get_dispute_id().value_or(std::string{});
        if (dispute_id.empty()) {
            dispute_id = query.get("dispute_id").value_or(std::string{});
        }
        if (dispute_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing dispute id"));
            return;
        }
        auto record = m_context->repository().find_dispute_by_string(dispute_id);
        if (!record) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::NOT_FOUND, "dispute not found"));
            return;
        }

        m_context->metrics().record_flow("dispute_attach_evidence");
        m_context->metrics().counter("disputes_attach_evidence_total").increment();

        const std::string lock_key = std::string{"dispute:attach:"} + dispute_id;
        const auto lock = m_context->locks().acquire(lock_key, std::int64_t{30}, "dispute_attach_evidence");
        if (!lock) {
            m_context->metrics().counter("disputes_attach_evidence_lock_contended").increment();
            send_error(res, send, core::error::AppErrorBuilder{}(
                Kind::DUPLICATE_REQUEST, "dispute evidence upload already in progress"));
            return;
        }

        // files_create_core substitute: persist file metadata, then update evidence.
        std::string file_id = parsed->get_file_id().value_or(std::string{});
        if (file_id.empty()) {
            file_id = m_context->ids().payment_id("file_");
        }
        const std::string file_name = parsed->get_file_name().value_or(std::string{});
        app::store::files::FileRecord file{};
        file.set_file_id(file_id);
        file.set_merchant_id(record->get_merchant_id().empty() ? scope.merchant_id : record->get_merchant_id());
        file.set_profile_id(record->get_profile_id().empty() ? scope.profile_id : record->get_profile_id());
        file.set_file_name(file_name.empty() ? file_id : file_name);
        file.set_file_type("application/octet-stream");
        file.set_purpose("dispute_evidence");
        file.set_status("ACTIVE");
        file.set_created_at(now_string());
        m_context->repository().save_file(file);

        auto blocks = stored_evidence(dispute_id);
        app::model::disputes::DisputeEvidenceBlock block{};
        block.set_evidence_type(evidence_type_from(parsed->get_evidence_type().value_or(std::string{})));
        app::model::disputes::FileMetadataResponseValue metadata{};
        metadata.set_value(file_id + "|" + file.get_file_name());
        block.set_file_metadata_response(metadata);
        blocks.push_back(block);
        m_context->config_store().put("dispute_evidence", dispute_id, encode_evidence(blocks));
        m_context->repository().update_dispute(*record);
        m_context->locks().release(lock_key);

        app::dto::files::CreateFileResponse response{};
        response.set_file_id(file_id);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_retrieve_dispute_evidence(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("retrieve_dispute_evidence");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto scope = scope_of(auth_context, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        const std::string dispute_id = path_segment(req);
        if (dispute_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing dispute id"));
            return;
        }
        if (!m_context->repository().find_dispute_by_string(dispute_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::NOT_FOUND, "dispute not found"));
            return;
        }

        m_context->metrics().record_flow("dispute_retrieve_evidence");
        m_context->metrics().counter("disputes_retrieve_evidence_total").increment();

        auto text = serde::Ser::encode_json(stored_evidence(dispute_id));
        send_json(res, send, text, true);
    }

    void handle_delete_dispute_evidence(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("delete_dispute_evidence");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto scope = scope_of(auth_context, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::disputes::DeleteEvidenceRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }
        const std::string dispute_id = parsed->get_dispute_id();
        if (dispute_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing dispute id"));
            return;
        }
        auto record = m_context->repository().find_dispute_by_string(dispute_id);
        if (!record) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::NOT_FOUND, "dispute not found"));
            return;
        }

        m_context->metrics().record_flow("dispute_delete_evidence");
        m_context->metrics().counter("disputes_delete_evidence_total").increment();

        const std::string lock_key = std::string{"dispute:delete:"} + dispute_id;
        const auto lock = m_context->locks().acquire(lock_key, std::int64_t{30}, "dispute_delete_evidence");
        if (!lock) {
            m_context->metrics().counter("disputes_delete_evidence_lock_contended").increment();
            send_error(res, send, core::error::AppErrorBuilder{}(
                Kind::DUPLICATE_REQUEST, "dispute evidence delete already in progress"));
            return;
        }

        const std::string removed = evidence_type_name(parsed->get_evidence_type());
        auto blocks = stored_evidence(dispute_id);
        std::vector<app::model::disputes::DisputeEvidenceBlock> kept{};
        for (const auto& block : blocks) {
            if (evidence_type_name(block.get_evidence_type()) != removed) {
                kept.push_back(block);
            }
        }
        m_context->config_store().put("dispute_evidence", dispute_id, encode_evidence(kept));
        m_context->repository().update_dispute(*record);
        notify_dispute(*record, "dispute.evidence_deleted");
        m_context->locks().release(lock_key);

        StatusOkWire response{};
        response.set_status("success");
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_get_disputes_aggregate(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_disputes_aggregate");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto scope = scope_of(auth_context, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        m_context->metrics().record_flow("dispute_aggregate");
        m_context->metrics().counter("disputes_aggregate_total").increment();

        auto text = serde::Ser::encode_json(aggregate_from(records_in_time_range(query, scope, false)));
        send_json(res, send, text, true);
    }

    void handle_get_disputes_aggregate_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_disputes_aggregate_profile");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto scope = scope_of(auth_context, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }
        if (scope.profile_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::VALIDATION_FAILED, "missing profile id"));
            return;
        }

        m_context->metrics().record_flow("dispute_profile_aggregate");
        m_context->metrics().counter("disputes_profile_aggregate_total").increment();

        auto text = serde::Ser::encode_json(aggregate_from(records_in_time_range(query, scope, true)));
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/disputes"}
            .add_route(
                Route{"/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_disputes_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_disputes_list")
            )
            .add_route(
                Route{"/profile/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_disputes_list_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_disputes_list_profile")
            )
            .add_route(
                Route{"/filter"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_disputes_filters(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_disputes_filters")
            )
            .add_route(
                Route{"/profile/filter"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_disputes_filters_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_disputes_filters_profile")
            )
            .add_route(
                Route{"/platform/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_disputes_list_for_platform(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_disputes_list_for_platform")
            )
            .add_route(
                Route{"/platform/filter"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_platform_disputes_filters(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_platform_disputes_filters")
            )
            .add_route(
                Route{"/accept/:dispute_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_accept_dispute(req, res, std::move(send));
                    })
                    .summary("Hyperswitch accept_dispute")
            )
            .add_route(
                Route{"/aggregate"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_disputes_aggregate(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_disputes_aggregate")
            )
            .add_route(
                Route{"/profile/aggregate"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_disputes_aggregate_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_disputes_aggregate_profile")
            )
            .add_route(
                Route{"/evidence"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_submit_dispute_evidence(req, res, std::move(send));
                    })
                    .summary("Hyperswitch submit_dispute_evidence")
            )
            .add_route(
                Route{"/evidence"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_attach_dispute_evidence(req, res, std::move(send));
                    })
                    .summary("Hyperswitch attach_dispute_evidence")
            )
            .add_route(
                Route{"/evidence"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_delete_dispute_evidence(req, res, std::move(send));
                    })
                    .summary("Hyperswitch delete_dispute_evidence")
            )
            .add_route(
                Route{"/evidence/:dispute_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_dispute_evidence(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_dispute_evidence")
            )
            .add_route(
                Route{"/:dispute_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_dispute(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_dispute")
            )
            .add_route(
                Route{"/:connector_id/fetch"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_fetch_disputes(req, res, std::move(send));
                    })
                    .summary("Hyperswitch fetch_disputes")
            );
    }

    explicit DisputesRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    // ── Identity / scope ────────────────────────────────────────────────────────

    [[nodiscard]] Scope scope_of(const Auth& auth, const Query& query, interfaces::io::IRequest& req) const
    {
        Scope scope{};
        scope.merchant_id = auth.merchant_id();
        if (scope.merchant_id.empty()) {
            scope.merchant_id = app::http::merchant_id_from_header{}(req);
        }
        scope.org_id = auth.org_id();
        if (scope.org_id.empty()) {
            scope.org_id = app::http::org_id_from_header{}(req);
        }
        scope.profile_id = auth.profile_id();
        const auto query_profile = query.get("profile_id").value_or(std::string{});
        if (!query_profile.empty()) {
            if (scope.profile_id.empty()) {
                scope.profile_id = query_profile;
            } else if (scope.profile_id != query_profile) {
                scope.profile_conflict = true;
                m_context->metrics().counter("disputes_profile_scope_conflict").increment();
            }
        }
        return scope;
    }

    [[nodiscard]] std::optional<Error> scope_error(const Scope& scope, bool require_platform) const
    {
        if (scope.profile_conflict) {
            return core::error::AppErrorBuilder{}(Kind::VALIDATION_FAILED, "profile id does not match auth scope");
        }
        // Local substitute for upstream is_platform_account(): a platform operation
        // must carry an organization id from the auth layer.
        if (require_platform && scope.org_id.empty()) {
            m_context->metrics().counter("disputes_platform_guard_denied").increment();
            return core::error::AppErrorBuilder{}(Kind::FORBIDDEN, "platform account required");
        }
        return std::nullopt;
    }

    // ── Guards and state transitions ───────────────────────────────────────────

    [[nodiscard]] std::optional<Error> accept_guard(const Record& record) const
    {
        return terminal_guard(record, "accept");
    }

    [[nodiscard]] std::optional<Error> evidence_guard(const Record& record) const
    {
        return terminal_guard(record, "evidence");
    }

    [[nodiscard]] std::optional<Error> terminal_guard(const Record& record, std::string_view operation) const
    {
        const std::string& status = record.get_status();
        if (status == "Accepted" || status == "Won" || status == "Lost") {
            m_context->metrics().counter("disputes_terminal_guard_denied").increment();
            return core::error::AppErrorBuilder{}(
                Kind::PRECONDITION_FAILED,
                std::string{"dispute is terminal; cannot "} + std::string{operation});
        }
        return std::nullopt;
    }

    // ── Connector / process tracker / webhook side effects ─────────────────────

    void call_connector(
        std::string_view operation,
        std::string_view dispute_id,
        const Scope& scope,
        Record& record,
        std::string_view raw_payload
    ) const
    {
        app::connector::ConnectorRequest request{};
        request.set_operation(std::string{operation});
        request.set_merchant_id(scope.merchant_id.empty() ? record.get_merchant_id() : scope.merchant_id);
        request.set_resource_id(std::string{dispute_id});
        request.set_raw_payload(std::string{raw_payload});
        if (!record.get_currency().empty()) {
            request.set_currency(record.get_currency());
        }
        if (record.get_amount() > 0) {
            request.set_amount(record.get_amount());
        }

        auto connector = m_context->connector_router().select_for(request);
        if (!connector) {
            connector = m_context->connector_router().select(request.get_merchant_id());
        }
        if (!connector) {
            m_context->metrics().record_flow("dispute.connector.none");
            m_context->metrics().counter("disputes_connector_unavailable").increment();
            return;
        }

        const std::string name{connector->name()};
        record.set_connector(name);
        if (m_context->config_store().is_killed(name)) {
            m_context->metrics().counter("disputes_connector_killed").increment();
        }
        if (m_context->config_store().is_shadow_killed(name)) {
            m_context->metrics().counter("disputes_connector_shadow_killed").increment();
        }

        const std::int64_t tracker_id = m_context->process_trackers().schedule(
            std::string{"dispute_"} + std::string{operation},
            std::format("{{\"dispute_id\":\"{}\",\"connector\":\"{}\"}}", dispute_id, name),
            "0");

        app::connector::ConnectorResponse response{};
        if (operation == "sync") {
            response = connector->sync(request);
        } else if (operation == "fetch_disputes") {
            response = connector->fetch_disputes(request);
        } else if (operation == "accept_dispute") {
            response = connector->accept_dispute(request);
        } else if (operation == "submit_evidence") {
            response = connector->submit_evidence(request);
        } else {
            response = connector->verify(request);
        }

        if (!response.get_status().empty()) {
            if (operation == "accept_dispute") {
                record.set_status("Accepted");
            } else if (operation == "submit_evidence") {
                record.set_status("NeedResponse");
            }
            if (!response.get_reference_id().empty()) {
                m_context->config_store().put(
                    "dispute_connector_ref", std::string{dispute_id}, response.get_reference_id());
            }
            m_context->config_store().put(
                "dispute_connector_status", std::string{dispute_id}, response.get_status());
        } else {
            m_context->metrics().counter("disputes_connector_no_status").increment();
        }

        m_context->process_trackers().mark_terminal(tracker_id, record.get_status());
        m_context->metrics().record_flow(std::string{"dispute.connector."} + std::string{operation});
        m_context->metrics().counter(std::string{"disputes_connector_"} + std::string{operation}).increment();
    }

    void notify_dispute(const Record& record, std::string_view flow) const
    {
        const std::string payload = std::format(
            "{{\"dispute_id\":\"{}\",\"payment_id\":\"{}\",\"merchant_id\":\"{}\",\"status\":\"{}\",\"flow\":\"{}\"}}",
            record.get_dispute_id(), record.get_payment_id(), record.get_merchant_id(),
            record.get_status(), flow);
        const std::string topic = std::string{"dispute."} + record.get_status();
        const std::string event_id = m_context->events().publish(topic, payload);
        const std::string endpoint = m_context->config_store()
            .get_value("dispute_webhook_endpoint", record.get_merchant_id())
            .value_or(std::string{"local://dispute-webhook/"} + record.get_merchant_id());
        m_context->webhook_delivery().enqueue(event_id, endpoint);
        m_context->metrics().counter("disputes_webhook_notified").increment();
    }

    [[nodiscard]] std::int64_t schedule_list_task(std::string_view name, const Scope& scope) const
    {
        const std::int64_t tracker_id = m_context->process_trackers().schedule(
            std::string{name},
            std::format("{{\"merchant_id\":\"{}\",\"profile_id\":\"{}\"}}",
                        scope.merchant_id, scope.profile_id),
            "0");
        m_context->metrics().counter("disputes_list_task_scheduled").increment();
        return tracker_id;
    }

    // ── DTO conversion ─────────────────────────────────────────────────────────

    [[nodiscard]] app::model::disputes::DisputeResponse to_response(const Record& record) const
    {
        using namespace app::model::disputes;
        DisputeResponse response{};
        response.set_dispute_id(record.get_dispute_id());

        PaymentIdValue payment{};
        payment.set_value(record.get_payment_id());
        response.set_payment_id(payment);

        response.set_attempt_id(attempt_id_for(record.get_payment_id()));

        StringMinorUnitValue amount{};
        amount.set_value(std::to_string(record.get_amount()));
        response.set_amount(amount);

        CurrencyValue currency{};
        currency.set_value(record.get_currency());
        response.set_currency(currency);

        DisputeStageValue stage{};
        stage.set_value(record.get_stage());
        response.set_dispute_stage(stage);

        DisputeStatusValue status{};
        status.set_value(record.get_status());
        response.set_dispute_status(status);

        response.set_connector(record.get_connector());
        response.set_connector_status(
            m_context->config_store()
                .get_value("dispute_connector_status", record.get_dispute_id())
                .value_or(std::string{}));
        response.set_connector_dispute_id(
            m_context->config_store()
                .get_value("dispute_connector_ref", record.get_dispute_id())
                .value_or(std::string{}));

        PrimitiveDateTimeValue created{};
        created.set_value(record.get_created_at());
        response.set_created_at(created);

        if (!record.get_profile_id().empty()) {
            ProfileIdValue profile{};
            profile.set_value(record.get_profile_id());
            response.set_profile_id(profile);
        }
        response.set_is_already_refunded(is_already_refunded(record.get_payment_id()));
        return response;
    }

    [[nodiscard]] app::model::disputes::PlatformDisputeListItem to_platform_item(const Record& record) const
    {
        using namespace app::model::disputes;
        PlatformDisputeListItem item{};
        item.set_dispute_id(record.get_dispute_id());

        PaymentIdValue payment{};
        payment.set_value(record.get_payment_id());
        item.set_payment_id(payment);

        MerchantIdValue merchant{};
        merchant.set_value(record.get_merchant_id());
        item.set_merchant_id(merchant);

        StringMinorUnitValue amount{};
        amount.set_value(std::to_string(record.get_amount()));
        item.set_amount(amount);

        CurrencyValue currency{};
        currency.set_value(record.get_currency());
        item.set_currency(currency);

        DisputeStageValue stage{};
        stage.set_value(record.get_stage());
        item.set_dispute_stage(stage);

        DisputeStatusValue status{};
        status.set_value(record.get_status());
        item.set_dispute_status(status);

        item.set_connector(record.get_connector());
        item.set_attempt_id(attempt_id_for(record.get_payment_id()));

        PrimitiveDateTimeValue created{};
        created.set_value(record.get_created_at());
        item.set_created_at(created);
        return item;
    }

    [[nodiscard]] std::string attempt_id_for(std::string_view payment_id) const
    {
        if (payment_id.empty()) {
            return {};
        }
        for (const auto& attempt : m_context->repository().list_payment_attempts()) {
            if (attempt.get_payment_id() == payment_id) {
                return attempt.get_attempt_id();
            }
        }
        return {};
    }

    [[nodiscard]] bool is_already_refunded(std::string_view payment_id) const
    {
        return !m_context->repository().find_refunds_by_payment_id(payment_id).empty();
    }

    // ── List constraints / filtering / filters / aggregates ────────────────────

    [[nodiscard]] app::model::disputes::DisputeListGetConstraints list_constraints_from(
        const Query& query,
        const Scope& scope
    ) const
    {
        using namespace app::model::disputes;
        DisputeListGetConstraints constraints{};

        PageSizeValue limit{};
        limit.set_value(query.get("limit").value_or(std::string{"10"}));
        constraints.set_limit(limit);

        PageOffsetValue offset{};
        offset.set_value(query.get("offset").value_or(std::string{"0"}));
        constraints.set_offset(offset);

        if (const auto value = query.get("dispute_id")) {
            constraints.set_dispute_id(*value);
        }
        if (const auto value = query.get("payment_id")) {
            PaymentIdValue payment{};
            payment.set_value(*value);
            constraints.set_payment_id(payment);
        }
        if (const auto value = query.get("reason")) {
            constraints.set_reason(*value);
        }
        if (const auto value = query.get("dispute_status")) {
            std::vector<DisputeStatusValue> statuses{};
            DisputeStatusValue status{};
            status.set_value(*value);
            statuses.push_back(status);
            constraints.set_dispute_status(statuses);
        }
        if (const auto value = query.get("dispute_stage")) {
            std::vector<DisputeStageValue> stages{};
            DisputeStageValue stage{};
            stage.set_value(*value);
            stages.push_back(stage);
            constraints.set_dispute_stage(stages);
        }
        if (const auto value = query.get("connector")) {
            constraints.set_connector(std::vector<std::string>{*value});
        }
        if (!scope.profile_id.empty()) {
            ProfileIdValue profile{};
            profile.set_value(scope.profile_id);
            constraints.set_profile_id(profile);
        }
        if (const auto from = query.get("start_time")) {
            TimeRangeValue range{};
            range.set_value(*from);
            constraints.set_time_range(range);
        }
        return constraints;
    }

    [[nodiscard]] std::vector<Record> filter_disputes(
        const app::model::disputes::DisputeListGetConstraints& constraints,
        const Scope& scope,
        bool platform
    ) const
    {
        std::vector<Record> matched{};
        const std::size_t offset = to_size(constraints.get_offset().get_value(), 0);
        const std::size_t limit = to_size(constraints.get_limit().get_value(), 10);

        for (const auto& record : m_context->repository().list_disputes()) {
            if (platform) {
                if (scope.merchant_id.empty() || record.get_merchant_id() != scope.merchant_id) {
                    continue;
                }
            } else if (!scope.merchant_id.empty() && !record.get_merchant_id().empty() &&
                       record.get_merchant_id() != scope.merchant_id) {
                continue;
            }
            if (!scope.profile_id.empty() && !record.get_profile_id().empty() &&
                record.get_profile_id() != scope.profile_id) {
                continue;
            }
            if (constraints.get_dispute_id() && !constraints.get_dispute_id()->empty() &&
                record.get_dispute_id() != *constraints.get_dispute_id()) {
                continue;
            }
            if (constraints.get_payment_id() && !constraints.get_payment_id()->get_value().empty() &&
                record.get_payment_id() != constraints.get_payment_id()->get_value()) {
                continue;
            }
            if (constraints.get_reason() && !constraints.get_reason()->empty() &&
                record.get_reason() != *constraints.get_reason()) {
                continue;
            }
            if (constraints.get_dispute_status() && !constraints.get_dispute_status()->empty()) {
                bool hit{false};
                for (const auto& value : *constraints.get_dispute_status()) {
                    if (record.get_status() == value.get_value()) { hit = true; break; }
                }
                if (!hit) { continue; }
            }
            if (constraints.get_dispute_stage() && !constraints.get_dispute_stage()->empty()) {
                bool hit{false};
                for (const auto& value : *constraints.get_dispute_stage()) {
                    if (record.get_stage() == value.get_value()) { hit = true; break; }
                }
                if (!hit) { continue; }
            }
            if (constraints.get_connector() && !constraints.get_connector()->empty()) {
                bool hit{false};
                for (const auto& value : *constraints.get_connector()) {
                    if (record.get_connector() == value) { hit = true; break; }
                }
                if (!hit) { continue; }
            }
            matched.push_back(record);
        }

        std::vector<Record> paged{};
        if (offset < matched.size()) {
            const std::size_t available = matched.size() - offset;
            paged.reserve(std::min(limit, available));
            for (std::size_t i = offset; i < matched.size() && paged.size() < limit; ++i) {
                paged.push_back(matched[i]);
            }
        }
        return paged;
    }

    [[nodiscard]] std::vector<Record> records_in_time_range(
        const Query& query,
        const Scope& scope,
        bool profile_scoped
    ) const
    {
        const auto start = query.get("start_time").value_or(std::string{});
        const auto end = query.get("end_time").value_or(std::string{});
        std::vector<Record> result{};
        for (const auto& record : m_context->repository().list_disputes()) {
            if (!scope.merchant_id.empty() && !record.get_merchant_id().empty() &&
                record.get_merchant_id() != scope.merchant_id) {
                continue;
            }
            if (profile_scoped && !scope.profile_id.empty() &&
                !record.get_profile_id().empty() && record.get_profile_id() != scope.profile_id) {
                continue;
            }
            if (!start.empty() && record.get_created_at() < start) { continue; }
            if (!end.empty() && record.get_created_at() > end) { continue; }
            result.push_back(record);
        }
        return result;
    }

    [[nodiscard]] app::model::disputes::DisputesAggregateResponse aggregate_from(
        const std::vector<Record>& records
    ) const
    {
        app::model::disputes::DisputesAggregateResponse response{};
        std::unordered_map<std::string, std::int64_t> counts{};
        for (const auto& record : records) {
            ++counts[record.get_status()];
        }
        response.set_status_with_count(counts);
        return response;
    }

    [[nodiscard]] app::model::disputes::DisputeListFilters filters_from(
        const Scope& scope,
        bool profile_scoped
    ) const
    {
        using namespace app::model::disputes;
        DisputeListFilters filters{};
        std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>> connectors{};
        std::vector<CurrencyValue> currencies{};
        std::vector<DisputeStatusValue> statuses{};
        std::vector<DisputeStageValue> stages{};

        for (const auto& mca : m_context->repository().list_merchant_connectors()) {
            if (profile_scoped && !scope.profile_id.empty() && !mca.get_profile_id().empty() &&
                mca.get_profile_id() != scope.profile_id) {
                continue;
            }
            MerchantConnectorInfoValue value{};
            value.set_value(mca.get_connector_name());
            connectors[mca.get_connector_name()].push_back(value);
        }
        collect_enum_values(scope, profile_scoped, currencies, statuses, stages);

        filters.set_connector(connectors);
        filters.set_currency(currencies);
        filters.set_dispute_status(statuses);
        filters.set_dispute_stage(stages);
        return filters;
    }

    [[nodiscard]] app::model::disputes::PlatformDisputeListFilters platform_filters_from(
        const Scope& scope,
        bool platform
    ) const
    {
        using namespace app::model::disputes;
        PlatformDisputeListFilters filters{};
        std::unordered_map<std::string, std::vector<MerchantConnectorInfoValue>> connectors{};
        std::vector<CurrencyValue> currencies{};
        std::vector<DisputeStatusValue> statuses{};
        std::vector<DisputeStageValue> stages{};

        for (const auto& mca : m_context->repository().list_merchant_connectors()) {
            if (platform && !scope.merchant_id.empty() && !mca.get_merchant_id().empty() &&
                mca.get_merchant_id() != scope.merchant_id) {
                continue;
            }
            MerchantConnectorInfoValue value{};
            value.set_value(mca.get_connector_name());
            connectors[mca.get_connector_name()].push_back(value);
        }
        collect_enum_values(scope, platform, currencies, statuses, stages);

        filters.set_connector(connectors);
        filters.set_currency(currencies);
        filters.set_dispute_status(statuses);
        filters.set_dispute_stage(stages);
        return filters;
    }

    void collect_enum_values(
        const Scope& scope,
        bool scoped,
        std::vector<app::model::disputes::CurrencyValue>& currencies,
        std::vector<app::model::disputes::DisputeStatusValue>& statuses,
        std::vector<app::model::disputes::DisputeStageValue>& stages
    ) const
    {
        using namespace app::model::disputes;
        std::vector<std::string> seen_currency{};
        std::vector<std::string> seen_status{};
        std::vector<std::string> seen_stage{};
        for (const auto& record : m_context->repository().list_disputes()) {
            if (scoped && !scope.profile_id.empty() && !record.get_profile_id().empty() &&
                record.get_profile_id() != scope.profile_id) {
                continue;
            }
            const auto add = [](std::vector<std::string>& seen, const std::string& value) {
                if (value.empty()) { return false; }
                for (const auto& existing : seen) { if (existing == value) { return false; } }
                seen.push_back(value);
                return true;
            };
            if (add(seen_currency, record.get_currency())) {
                CurrencyValue value{}; value.set_value(record.get_currency()); currencies.push_back(value);
            }
            if (add(seen_status, record.get_status())) {
                DisputeStatusValue value{}; value.set_value(record.get_status()); statuses.push_back(value);
            }
            if (add(seen_stage, record.get_stage())) {
                DisputeStageValue value{}; value.set_value(record.get_stage()); stages.push_back(value);
            }
        }
    }

    // ── Evidence encoding ──────────────────────────────────────────────────────

    [[nodiscard]] std::vector<app::model::disputes::DisputeEvidenceBlock> stored_evidence(
        std::string_view dispute_id
    ) const
    {
        std::vector<app::model::disputes::DisputeEvidenceBlock> result{};
        const auto stored = m_context->config_store().get_value("dispute_evidence", dispute_id);
        if (!stored || stored->empty()) {
            return result;
        }
        for (const auto& part : split(*stored, ';')) {
            const auto fields = split(part, '|');
            if (fields.size() < 2) { continue; }
            app::model::disputes::DisputeEvidenceBlock block{};
            block.set_evidence_type(evidence_type_from(fields[0]));
            app::model::disputes::FileMetadataResponseValue metadata{};
            metadata.set_value(fields[1] + (fields.size() > 2 ? "|" + fields[2] : std::string{}));
            block.set_file_metadata_response(metadata);
            result.push_back(block);
        }
        return result;
    }

    [[nodiscard]] std::string encode_evidence(
        const std::vector<app::model::disputes::DisputeEvidenceBlock>& blocks
    ) const
    {
        std::string out{};
        for (const auto& block : blocks) {
            if (!out.empty()) { out.push_back(';'); }
            out += evidence_type_name(block.get_evidence_type());
            out.push_back('|');
            out += block.get_file_metadata_response().get_value();
        }
        return out;
    }

    [[nodiscard]] std::string evidence_type_name(app::model::disputes::EvidenceType type) const
    {
        using T = app::model::disputes::EvidenceType;
        switch (type) {
        case T::CancellationPolicy: return "cancellation_policy";
        case T::CustomerCommunication: return "customer_communication";
        case T::CustomerSignature: return "customer_signature";
        case T::Receipt: return "receipt";
        case T::RefundPolicy: return "refund_policy";
        case T::ServiceDocumentation: return "service_documentation";
        case T::ShippingDocumentation: return "shipping_documentation";
        case T::InvoiceShowingDistinctTransactions: return "invoice_showing_distinct_transactions";
        case T::RecurringTransactionAgreement: return "recurring_transaction_agreement";
        case T::UncategorizedFile: return "uncategorized_file";
        }
        return "uncategorized_file";
    }

    [[nodiscard]] app::model::disputes::EvidenceType evidence_type_from(std::string_view name) const
    {
        using T = app::model::disputes::EvidenceType;
        if (name == "cancellation_policy") return T::CancellationPolicy;
        if (name == "customer_communication") return T::CustomerCommunication;
        if (name == "customer_signature") return T::CustomerSignature;
        if (name == "receipt") return T::Receipt;
        if (name == "refund_policy") return T::RefundPolicy;
        if (name == "service_documentation") return T::ServiceDocumentation;
        if (name == "shipping_documentation") return T::ShippingDocumentation;
        if (name == "invoice_showing_distinct_transactions") return T::InvoiceShowingDistinctTransactions;
        if (name == "recurring_transaction_agreement") return T::RecurringTransactionAgreement;
        return T::UncategorizedFile;
    }

    // ── Small parsing helpers ──────────────────────────────────────────────────

    [[nodiscard]] std::vector<std::string> split(std::string_view text, char delimiter) const
    {
        std::vector<std::string> parts{};
        std::size_t start{0};
        while (start <= text.size()) {
            const std::size_t at = text.find(delimiter, start);
            const std::size_t end = at == std::string_view::npos ? text.size() : at;
            parts.emplace_back(text.substr(start, end - start));
            if (at == std::string_view::npos) { break; }
            start = at + 1;
        }
        return parts;
    }

    [[nodiscard]] std::size_t to_size(std::string_view text, std::size_t fallback) const
    {
        if (text.empty()) { return fallback; }
        std::size_t result{0};
        for (const char ch : text) {
            if (ch < '0' || ch > '9') { return fallback; }
            result = result * 10 + static_cast<std::size_t>(ch - '0');
        }
        return result;
    }

    [[nodiscard]] std::string now_string() const
    {
        return std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count());
    }

    /** @brief Path segment counted from the end; 0 = last, 1 = second-to-last. */
    [[nodiscard]] std::string path_segment(
        interfaces::io::IRequest& req,
        std::size_t from_end = 0
    ) const
    {
        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        const std::size_t count{segments.count()};
        if (from_end >= count) {
            return {};
        }
        const auto value = segments.at(count - 1 - from_end);
        return value ? std::string{*value} : std::string{};
    }

    /** @brief Build a JSON object from an id segment plus known query keys. */
    [[nodiscard]] std::string query_json(
        const Query& query,
        std::string_view id_key,
        std::string_view id_value,
        std::initializer_list<std::string_view> keys
    ) const
    {
        const auto escape = [](std::string_view text) {
            std::string out{};
            out.reserve(text.size());
            for (const char ch : text) {
                switch (ch) {
                case '"': out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default: out.push_back(ch); break;
                }
            }
            return out;
        };

        std::string out{"{"};
        bool first{true};
        const auto add = [&out, &first, &escape](std::string_view key, std::string_view value) {
            if (!first) {
                out.push_back(',');
            }
            first = false;
            out.push_back('"');
            out += escape(key);
            out += "\":\"";
            out += escape(value);
            out.push_back('"');
        };

        if (!id_key.empty()) {
            add(id_key, id_value);
        }
        for (const auto key : keys) {
            if (const auto value = query.get(key)) {
                add(key, *value);
            }
        }
        out.push_back('}');
        return out;
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

} // namespace app::handlers::disputes
