module;
export module app_handlers:blocklist;

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
import app_workflow;
import app_events;
import app_services;

// Upstream reference (SHA 7736d55adaf235f7af74e133ae408573b1826823):
//   routes/blocklist.rs:1-632           - the 11 HTTP route wrappers (server_wrap + auth_type).
//   core/blocklist.rs:1-160             - dispatch into the five submodules.
//   core/blocklist/utils.rs             - insert/delete/list/count/lookup/guard domain logic.
//   core/blocklist/batch.rs             - CSV validation, job creation, job status/listing.
//   core/blocklist/clone.rs             - profile ownership checks and clone job creation.
//   core/blocklist/export.rs            - export job creation and download signing.
//   core/blocklist/transformers.rs      - storage -> API response mapping.
//   api_models/src/blocklist.rs         - all wire DTOs.
// Every upstream function is ported below as a local helper. External infrastructure
// (vault fingerprinting, S3 file storage, process-tracker workers, dedicated job tables)
// is replaced by in-process facilities in AppContext (instruction: local substitutes
// allowed for external infra); observable wire shapes, status codes, validation,
// transaction namespaces, profile scoping and error messages mirror upstream exactly.

export namespace app::handlers::blocklist {

class BlocklistRouteHandlers
{
public:
    explicit BlocklistRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    // routes/blocklist.rs:30-68 -> core/blocklist.rs:16-26 ->
    // utils::insert_entry_into_blocklist (utils.rs:316-433).
    void handle_add_entry_to_blocklist(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "add_entry_to_blocklist")) {
            send_error(res, send, *denied);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        auto parsed = parse_entry_body(body);
        if (parsed.failure) {
            send_error(res, send, *parsed.failure);
            return;
        }

        const auto merchant = merchant_id_of(req);
        if (const auto bad = require_merchant(merchant)) {
            send_error(res, send, *bad);
            return;
        }
        const auto profile = resolve_required_profile(req, merchant);
        if (profile.failure) {
            send_error(res, send, *profile.failure);
            return;
        }

        // utils.rs:377-430 (fingerprint) and :448-504 (BIN duplicate check).
        if (find_by_identity(merchant, profile.id, parsed.data, parsed.transaction_type)) {
            const auto message = parsed.kind == "payment_method"
                                     ? std::string{"data associated with the given fingerprint is already blocked"}
                                     : std::string{"provided bin is already blocked"};
            // Upstream PreconditionFailed maps to HTTP 400 (api_error_response.rs:626-628).
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, message));
            return;
        }

        const auto created = now_iso8601();
        save_entry(make_entry(parsed.data, merchant, profile.id, parsed.kind,
                              parsed.transaction_type, created));

        // transformers.rs:17-27 (ForeignFrom<storage::Blocklist>).
        app::model::blocklist::BlocklistResponse response;
        response.set_fingerprint_id(parsed.data);
        response.set_data_kind(wrap<app::model::blocklist::BlocklistDataKindValue>(parsed.kind));
        response.set_transaction_type(
            wrap<app::model::blocklist::BlocklistTransactionTypeValue>(parsed.transaction_type)
        );
        response.set_created_at(wrap<app::model::blocklist::PrimitiveDateTimeValue>(created));
        if (!profile.id.empty()) {
            response.set_profile_id(wrap<app::model::blocklist::ProfileIdValue>(profile.id));
        }
        emit("blocklist.entry.added", response);
        track("add_entry_to_blocklist");
        // server_wrap -> ApplicationResponse::Json -> HTTP 200.
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // routes/blocklist.rs:89-127 -> core/blocklist.rs:28-38 ->
    // utils::delete_entry_from_blocklist (utils.rs:25-74, 506-525).
    void handle_remove_entry_from_blocklist(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "remove_entry_from_blocklist")) {
            send_error(res, send, *denied);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        auto parsed = parse_entry_body(body);
        if (parsed.failure) {
            send_error(res, send, *parsed.failure);
            return;
        }

        const auto merchant = merchant_id_of(req);
        if (const auto bad = require_merchant(merchant)) {
            send_error(res, send, *bad);
            return;
        }
        const auto profile = resolve_required_profile(req, merchant);
        if (profile.failure) {
            send_error(res, send, *profile.failure);
            return;
        }

        const auto existing = find_by_identity(merchant, profile.id, parsed.data,
                                               parsed.transaction_type);
        if (!existing) {
            const auto message = parsed.kind == "payment_method"
                                     ? std::string{"no blocklist record for the given fingerprint id was found"}
                                     : std::string{"could not find a blocklist entry for the given bin"};
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, message));
            return;
        }

        auto response = response_of(*existing);
        remove_entry(existing->get_id());
        emit("blocklist.entry.removed", response);
        track("remove_entry_from_blocklist");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // routes/blocklist.rs:148-186 -> core/blocklist.rs:40-54 ->
    // utils::list_blocklist_entries_for_merchant (utils.rs:120-162).
    void handle_list_blocked_payment_methods(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "list_blocked_payment_methods")) {
            send_error(res, send, *denied);
            return;
        }

        const auto merchant = merchant_id_of(req);
        if (const auto bad = require_merchant(merchant)) {
            send_error(res, send, *bad);
            return;
        }
        // list is merchant-wide when no profile resolves (routes.rs:162-163).
        const auto profile = explicit_profile_of(req);
        const auto query = app::http::RequestQuery{}(req);

        auto data_kind = query.get("data_kind");
        if (!data_kind) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                        "missing required param: data_kind"));
            return;
        }
        const auto kind = query_kind(*data_kind);
        if (!kind) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                        "unknown variant for data_kind"));
            return;
        }
        std::string transaction_type{"payment"};
        if (const auto raw = query.get("transaction_type")) {
            const auto parsed = parse_transaction_type(*raw);
            if (!parsed) {
                send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                            "unknown variant for transaction_type"));
                return;
            }
            transaction_type = *parsed;
        }
        const auto limit = query_number<std::uint16_t>(query, "limit").value_or(10);
        const auto offset = query_number<std::uint16_t>(query, "offset").value_or(0);

        std::vector<EntryRecord> selected;
        for (const auto& record : list_entries()) {
            if (!matches_scope(record, merchant, profile, *kind, transaction_type)) {
                continue;
            }
            selected.push_back(record);
        }
        sort_by_created_desc(selected);
        const std::size_t total = selected.size();
        auto page = paginate(std::move(selected), offset, limit);

        std::vector<app::model::blocklist::BlocklistResponse> data;
        data.reserve(page.size());
        for (const auto& record : page) {
            data.push_back(response_of(record));
        }

        app::model::blocklist::ListBlocklistResponse response;
        response.set_count(data.size());
        response.set_total_count(total);
        response.set_data(std::move(data));
        emit("blocklist.list", response);
        track("list_blocked_payment_methods");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // routes/blocklist.rs:207-242 -> core/blocklist.rs:56-65 ->
    // utils::get_blocklist_count (utils.rs:192-269).
    void handle_get_blocklist_count(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "get_blocklist_count")) {
            send_error(res, send, *denied);
            return;
        }

        const auto merchant = merchant_id_of(req);
        if (const auto bad = require_merchant(merchant)) {
            send_error(res, send, *bad);
            return;
        }
        const auto profile = resolve_required_profile(req, merchant);
        if (profile.failure) {
            send_error(res, send, *profile.failure);
            return;
        }
        const auto query = app::http::RequestQuery{}(req);

        auto raw_kind = query.get("data_kind");
        if (!raw_kind) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                        "missing required param: data_kind"));
            return;
        }
        const auto kind = query_kind(*raw_kind);
        if (!kind) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                        "unknown variant for data_kind"));
            return;
        }
        std::string transaction_type; // empty = both, including legacy NULL (api_models:109-112).
        if (const auto raw = query.get("transaction_type")) {
            const auto parsed = parse_transaction_type(*raw);
            if (!parsed) {
                send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                            "unknown variant for transaction_type"));
                return;
            }
            transaction_type = *parsed;
        }

        std::size_t total{0};
        std::map<std::string, std::size_t> by_length;
        for (const auto& record : list_entries()) {
            if (!matches_scope(record, merchant, profile.id, *kind, transaction_type)) {
                continue;
            }
            ++total;
            // utils.rs:231-260 groups by length(fingerprint_id); fingerprints (fixed width)
            // have no breakdown, so counts_by_length is None for payment_method.
            if (*kind != "payment_method") {
                ++by_length[std::to_string(record.get_fingerprint().size())];
            }
        }

        app::model::blocklist::BlocklistCountResponse response;
        response.set_data_kind(wrap<app::model::blocklist::BlocklistDataKindValue>(*kind));
        response.set_total_count(total);
        if (*kind != "payment_method" && !by_length.empty()) {
            response.set_counts_by_length(std::move(by_length));
        }
        emit("blocklist.count", response);
        track("get_blocklist_count");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // routes/blocklist.rs:263-298 -> core/blocklist.rs:67-76 ->
    // utils::lookup_blocklist_entry (utils.rs:271-314).
    void handle_lookup_blocklist_entry(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "lookup_blocklist_entry")) {
            send_error(res, send, *denied);
            return;
        }

        const auto merchant = merchant_id_of(req);
        if (const auto bad = require_merchant(merchant)) {
            send_error(res, send, *bad);
            return;
        }
        const auto profile = resolve_required_profile(req, merchant);
        if (profile.failure) {
            send_error(res, send, *profile.failure);
            return;
        }
        const auto query = app::http::RequestQuery{}(req);

        auto data = query.get("data");
        if (!data || data->empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                        "Missing required param: data"));
            return;
        }
        // LengthString<20,1> enforced at deserialization (common_utils/types.rs:1066-1071).
        if (data->size() > 20) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                        "data contains invalid data. Expected format is a string of 1 to 20 characters"));
            return;
        }
        std::string transaction_type;
        if (const auto raw = query.get("transaction_type")) {
            const auto parsed = parse_transaction_type(*raw);
            if (!parsed) {
                send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                            "unknown variant for transaction_type"));
                return;
            }
            transaction_type = *parsed;
        }

        std::vector<std::string> distinct;
        for (const auto& record : list_entries()) {
            if (!matches_merchant(record, merchant) || !matches_profile(record, profile.id)) {
                continue;
            }
            if (record.get_fingerprint() != *data) {
                continue;
            }
            if (!transaction_type.empty() && !tt_matches(record, transaction_type)) {
                continue;
            }
            const auto tt = legacy_transaction_type(record);
            if (std::find(distinct.begin(), distinct.end(), tt) == distinct.end()) {
                distinct.push_back(tt);
            }
        }

        std::vector<app::model::blocklist::BlocklistTransactionTypeValue> types;
        types.reserve(distinct.size());
        for (const auto& tt : distinct) {
            types.push_back(wrap<app::model::blocklist::BlocklistTransactionTypeValue>(tt));
        }
        app::model::blocklist::BlocklistLookupResponse response;
        response.set_data(*data);
        response.set_blocked(!types.empty());
        response.set_transaction_types(std::move(types));
        emit("blocklist.lookup", response);
        track("lookup_blocklist_entry");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // routes/blocklist.rs:314-343 -> core/blocklist.rs:78-86 ->
    // utils::toggle_blocklist_guard_for_merchant (utils.rs:76-118). Merchant-wide, no profile.
    void handle_toggle_blocklist_guard(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "toggle_blocklist_guard")) {
            send_error(res, send, *denied);
            return;
        }

        const auto query = app::http::RequestQuery{}(req);
        auto raw = query.get("status");
        if (!raw) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                        "missing required param: status"));
            return;
        }
        bool status{false};
        if (*raw == "true") {
            status = true;
        } else if (*raw != "false") {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                        "invalid bool value for status"));
            return;
        }

        const auto merchant = merchant_id_of(req);
        if (const auto bad = require_merchant(merchant)) {
            send_error(res, send, *bad);
            return;
        }
        // common_utils/id_type/merchant.rs:126-128: guard_blocklist_for_{merchant}.
        const auto key = "guard_blocklist_for_" + merchant;
        m_context->config_store().put("configs", key, status ? "true" : "false");

        app::model::blocklist::ToggleBlocklistResponse response;
        response.set_blocklist_guard_status(status ? "enabled" : "disabled");
        emit("blocklist.guard.toggled", response);
        track("toggle_blocklist_guard");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // routes/blocklist.rs:383-430 -> core/blocklist.rs:88-106 ->
    // batch::initiate_batch_blocklist_upload (batch.rs:348-510).
    void handle_upload_batch_blocklist(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "upload_batch_blocklist")) {
            send_error(res, send, *denied);
            return;
        }

        const auto query = app::http::RequestQuery{}(req);
        std::string override_type;
        if (const auto raw = query.get("transaction_type")) {
            const auto parsed = parse_transaction_type(*raw);
            if (!parsed) {
                send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                            "unknown variant for transaction_type"));
                return;
            }
            override_type = *parsed;
        }

        const auto merchant = merchant_id_of(req);
        if (const auto bad = require_merchant(merchant)) {
            send_error(res, send, *bad);
            return;
        }
        const auto profile = resolve_required_profile(req, merchant);
        if (profile.failure) {
            send_error(res, send, *profile.failure);
            return;
        }

        const auto body = req.body_as_string();
        auto upload = multipart_file(req, body); // extract the `file` field (MultipartBytes).
        // routes/blocklist.rs:351: #[multipart(limit = "5MiB")].
        if (upload.content.size() > 5U * 1024U * 1024U) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                        "file exceeds 5MiB limit"));
            return;
        }

        auto parsed = parse_blocklist_csv(upload.content, override_type);
        if (parsed.failure) {
            send_error(res, send, *parsed.failure);
            return;
        }

        const auto job_id = m_context->ids().payment_id("blkbatch");
        const auto stamp = now_iso8601();
        // in-process substitute for batch.rs:512-551 process_chunk (the worker is not present).
        std::uint32_t succeeded{0};
        for (const auto& row : parsed.rows) {
            const auto key = identity_key(merchant, profile.id, row.data, row.transaction_type);
            if (find_entry(key)) {
                ++succeeded;
                continue;
            }
            save_entry(make_entry(row.data, merchant, profile.id, row.kind,
                                  row.transaction_type, stamp));
            ++succeeded;
        }

        upsert_job(job_id, merchant, profile.id, "upload", "initiated",
                   static_cast<std::uint32_t>(parsed.rows.size()), override_type,
                   upload.file_name, {}, false, std::nullopt, succeeded, 0, {});
        // Worker completion: batch.rs:437-450 job row starts Initiated; the in-process
        // substitute updates it to Completed once the synchronous insert has run.
        upsert_job(job_id, merchant, profile.id, "upload", "completed",
                   static_cast<std::uint32_t>(parsed.rows.size()), override_type,
                   upload.file_name, {}, false, std::nullopt, succeeded, 0, {});
        schedule_job("BATCH_BLOCKLIST_UPLOAD", job_id, merchant, profile.id, parsed.rows.size());

        app::model::blocklist::BatchBlocklistUploadResponse response;
        response.set_job_id(job_id);
        response.set_total_rows(static_cast<std::uint32_t>(parsed.rows.size()));
        response.set_status(wrap<app::model::blocklist::BatchBlocklistJobStatusValue>("initiated"));
        emit("blocklist.batch.upload", response);
        track("upload_batch_blocklist");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // routes/blocklist.rs:446-477 -> core/blocklist.rs:108-122 ->
    // batch::get_batch_blocklist_job_status (batch.rs:553-590).
    void handle_get_batch_blocklist_job_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "get_batch_blocklist_job_status")) {
            send_error(res, send, *denied);
            return;
        }

        const auto job_id = job_id_of(req);
        if (job_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                        "missing job id"));
            return;
        }
        const auto stored = find_job(job_key(job_id));
        if (!stored) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND,
                                        "Batch blocklist job `" + job_id + "` not found"));
            return;
        }
        const auto merchant = merchant_id_of(req);
        if (const auto bad = require_merchant(merchant)) {
            send_error(res, send, *bad);
            return;
        }
        const auto profile = explicit_profile_of(req);
        if (!matches_merchant(*stored, merchant) || !matches_profile(*stored, profile)) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND,
                                        "Batch blocklist job `" + job_id + "` not found"));
            return;
        }

        auto response = job_status_of(*stored);
        // batch.rs:581-589: the only place a download link is minted.
        if (is_export_downloadable(response)) {
            const auto expires = now_iso8601(900);
            response.set_download_url(signed_download_url(response.get_file_name()
                                                              .value_or(std::string{}), expires));
            response.set_download_url_expires_at(
                wrap<app::model::blocklist::PrimitiveDateTimeValue>(expires)
            );
        }
        emit("blocklist.job.status", response);
        track("get_batch_blocklist_job_status");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // routes/blocklist.rs:498-528 -> core/blocklist.rs:124-138 ->
    // batch::list_batch_blocklist_jobs (batch.rs:657-697).
    void handle_list_batch_blocklist_jobs(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "list_batch_blocklist_jobs")) {
            send_error(res, send, *denied);
            return;
        }

        const auto merchant = merchant_id_of(req);
        if (const auto bad = require_merchant(merchant)) {
            send_error(res, send, *bad);
            return;
        }
        const auto profile = explicit_profile_of(req);
        const auto query = app::http::RequestQuery{}(req);
        const auto limit = query_number<std::uint32_t>(query, "limit").value_or(10);
        const auto offset = query_number<std::uint32_t>(query, "offset").value_or(0);
        std::string job_type;
        if (const auto raw = query.get("job_type")) {
            if (!known_job_type(*raw)) {
                send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                            "unknown variant for job_type"));
                return;
            }
            job_type = *raw;
        }

        std::vector<JobRecord> selected;
        for (auto& record : list_jobs("blocklist.job:")) {
            if (!matches_merchant(record, merchant) || !matches_profile(record, profile)) {
                continue;
            }
            if (!job_type.empty() && record.get_name() != job_type) {
                continue;
            }
            selected.push_back(std::move(record));
        }
        const std::size_t total = selected.size();
        auto page = paginate(std::move(selected), offset, limit);

        std::vector<app::model::blocklist::BatchBlocklistJobStatusResponse> data;
        data.reserve(page.size());
        for (const auto& record : page) {
            auto job = job_status_of(record);
            // api_models:228: listing never mints a download_url per row.
            job.set_download_url(std::nullopt);
            job.set_download_url_expires_at(std::nullopt);
            data.push_back(std::move(job));
        }
        app::model::blocklist::ListBatchBlocklistJobsResponse response;
        response.set_count(data.size());
        response.set_total_count(total);
        response.set_data(std::move(data));
        emit("blocklist.jobs.list", response);
        track("list_batch_blocklist_jobs");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // routes/blocklist.rs:549-584 -> core/blocklist.rs:140-149 ->
    // export::initiate_blocklist_export (export.rs:73-162).
    void handle_create_blocklist_export(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "create_blocklist_export")) {
            send_error(res, send, *denied);
            return;
        }

        std::string transaction_type;
        const auto query = app::http::RequestQuery{}(req);
        if (const auto raw = query.get("transaction_type")) {
            const auto parsed = parse_transaction_type(*raw);
            if (!parsed) {
                send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                            "unknown variant for transaction_type"));
                return;
            }
            transaction_type = *parsed;
        }

        const auto merchant = merchant_id_of(req);
        if (const auto bad = require_merchant(merchant)) {
            send_error(res, send, *bad);
            return;
        }
        const auto profile = resolve_required_profile(req, merchant);
        if (profile.failure) {
            send_error(res, send, *profile.failure);
            return;
        }

        const auto export_id = m_context->ids().payment_id("blkexp");
        const auto stamp = now_iso8601();
        // export.rs:32-42: blocklist_export_YYYYMMDD_HHMMSS.csv.
        const auto file_name = "blocklist_export_" + stamp.substr(0, 4) + stamp.substr(5, 2) +
                               stamp.substr(8, 2) + "_" + stamp.substr(11, 2) +
                               stamp.substr(14, 2) + stamp.substr(17, 2) + ".csv";
        const auto csv = export_csv(merchant, profile.id, transaction_type);

        ::app::store::files::FileRecord file;
        file.set_file_id(export_id);
        file.set_merchant_id(merchant);
        file.set_profile_id(profile.id);
        file.set_file_name(file_name);
        file.set_file_type("text/csv");
        file.set_file_size(static_cast<std::int64_t>(csv.size()));
        file.set_purpose("blocklist_export");
        file.set_provider_file_id(file_name);
        file.set_upload_provider("local");
        file.set_status("available");
        file.set_created_at(stamp);
        m_context->repository().save_file(std::move(file));

        upsert_job(export_id, merchant, profile.id, "export", "initiated", 0, transaction_type,
                   file_name, {}, false, std::nullopt, 0, 0, {}, {}, {});
        // Worker completion substitute: export.rs:164-174 downloadable/expires_at.
        const auto rows = std::count(csv.begin(), csv.end(), '\n') > 0
                              ? static_cast<std::uint32_t>(std::count(csv.begin(), csv.end(), '\n') - 1)
                              : 0U;
        upsert_job(export_id, merchant, profile.id, "export", "completed", rows, transaction_type,
                   file_name, {}, true, std::nullopt, 0, 0, {},
                   now_iso8601(86400), now_iso8601(900));
        schedule_job("BLOCKLIST_EXPORT", export_id, merchant, profile.id, rows);

        app::model::blocklist::BlocklistExportResponse response;
        response.set_export_id(export_id);
        response.set_status(wrap<app::model::blocklist::BatchBlocklistJobStatusValue>("initiated"));
        response.set_file_name(file_name);
        emit("blocklist.export.created", response);
        track("create_blocklist_export");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // routes/blocklist.rs:602-632 -> core/blocklist.rs:151-160 ->
    // clone::clone_blocklist_entries (clone.rs:68-109, 114-225).
    void handle_clone_blocklist_entries(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "clone_blocklist_entries")) {
            send_error(res, send, *denied);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::blocklist::CloneBlocklistEntriesRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto merchant = merchant_id_of(req);
        if (const auto bad = require_merchant(merchant)) {
            send_error(res, send, *bad);
            return;
        }
        // clone.rs:76-80: source is auth.profile, no default-profile fallback here.
        const auto source = explicit_profile_of(req);
        if (source.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                        "Missing required param: profile_id"));
            return;
        }

        std::vector<app::model::blocklist::ProfileIdValue> targets;
        for (const auto& target : typed->get_target_profile_ids()) {
            const auto id = target.get_value();
            if (std::find_if(targets.begin(), targets.end(), [&id](const auto& item) {
                    return item.get_value() == id;
                }) != targets.end()) {
                continue; // HashSet dedup.
            }
            targets.push_back(target);
        }
        // api_models:314-329 CloneBlocklistEntriesRequest::validate.
        if (targets.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::VALIDATION_FAILED,
                                        "target_profile_ids must contain at least one profile"));
            return;
        }
        if (std::any_of(targets.begin(), targets.end(), [&source](const auto& item) {
                return item.get_value() == source;
            })) {
            send_error(res, send, error(core::error::AppErrorKind::VALIDATION_FAILED,
                                        "target_profile_ids must not contain the source profile"));
            return;
        }
        // clone.rs:26-63 resolve_clone_targets: any foreign profile fails the whole request.
        if (!profile_is_owned(source, merchant)) {
            send_error(res, send, profile_not_found(source));
            return;
        }
        for (const auto& target : targets) {
            if (!profile_is_owned(target.get_value(), merchant)) {
                send_error(res, send, profile_not_found(target.get_value()));
                return;
            }
        }

        const auto job_id = m_context->ids().payment_id("blkclone");
        const auto stamp = now_iso8601();
        // clone.rs:126-136: the enqueued job starts with Initiated/0 per target.
        app::model::blocklist::ProfileCloneJobMetadata init_metadata;
        std::vector<app::model::blocklist::ProfileCloneTargetMetadata> init_targets;
        init_targets.reserve(targets.size());
        for (const auto& target : targets) {
            app::model::blocklist::ProfileCloneTargetMetadata item;
            item.set_profile_id(target);
            item.set_status(wrap<app::model::blocklist::BatchBlocklistJobStatusValue>("initiated"));
            item.set_processed_rows(0);
            init_targets.push_back(std::move(item));
        }
        init_metadata.set_targets(std::move(init_targets));
        upsert_job(job_id, merchant, source, "profile_clone", "initiated", 0,
                   value_of(typed->get_transaction_type()), {}, {}, false, std::move(init_metadata),
                   0, 0, {});

        app::model::blocklist::ProfileCloneJobMetadata metadata;
        std::vector<app::model::blocklist::ProfileCloneTargetMetadata> clone_targets;
        clone_targets.reserve(targets.size());
        std::size_t cloned{0};
        for (const auto& target : targets) {
            const auto target_id = target.get_value();
            std::uint32_t copied{0};
            // Worker substitute: clone.rs:230-246 to_target_entry (one target at a time).
            for (const auto& record : list_entries()) {
                if (!matches_merchant(record, merchant) || record.get_profile_id() != source) {
                    continue;
                }
                const auto key = identity_key(merchant, target_id, record.get_fingerprint(),
                                              legacy_transaction_type(record));
                if (!find_entry(key)) {
                    save_entry(make_entry(record.get_fingerprint(), merchant, target_id,
                                          record.get_data_kind(), legacy_transaction_type(record),
                                          stamp));
                }
                ++copied;
            }
            cloned += copied;
            app::model::blocklist::ProfileCloneTargetMetadata item;
            item.set_profile_id(target);
            item.set_status(wrap<app::model::blocklist::BatchBlocklistJobStatusValue>("completed"));
            item.set_processed_rows(copied);
            clone_targets.push_back(std::move(item));
        }
        metadata.set_targets(std::move(clone_targets));

        upsert_job(job_id, merchant, source, "profile_clone", "completed", 0,
                   value_of(typed->get_transaction_type()), {}, {}, false,
                   std::move(metadata), static_cast<std::uint32_t>(cloned), 0, {});
        schedule_job("BLOCKLIST_PROFILE_CLONE", job_id, merchant, source, cloned);

        app::model::blocklist::CloneBlocklistEntriesResponse response;
        response.set_source_profile_id(wrap<app::model::blocklist::ProfileIdValue>(source));
        response.set_job_id(job_id);
        response.set_status(wrap<app::model::blocklist::BatchBlocklistJobStatusValue>("initiated"));
        response.set_target_profile_ids(targets);
        emit("blocklist.clone", response);
        track("clone_blocklist_entries");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        // Deterministic in-process substitute for the compile-time
        // `all(feature = "olap", feature = "v1")` gate at routes/app.rs:2053.
        if (const auto enabled = m_context->config_store().get_value("route_features", "blocklist");
            enabled.has_value() && *enabled == "false") {
            return;
        }

        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/blocklist"}
            .add_route(
                Route{"/"}
                    .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                std::function<void()> send) {
                        handle_list_blocked_payment_methods(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_blocked_payment_methods")
            )
            .add_route(
                Route{"/"}
                    .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                 std::function<void()> send) {
                        handle_add_entry_to_blocklist(req, res, std::move(send));
                    })
                    .summary("Hyperswitch add_entry_to_blocklist")
            )
            .add_route(
                Route{"/"}
                    .delt([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                 std::function<void()> send) {
                        handle_remove_entry_from_blocklist(req, res, std::move(send));
                    })
                    .summary("Hyperswitch remove_entry_from_blocklist")
            )
            .add_route(
                Route{"/toggle"}
                    .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                 std::function<void()> send) {
                        handle_toggle_blocklist_guard(req, res, std::move(send));
                    })
                    .summary("Hyperswitch toggle_blocklist_guard")
            )
            .add_route(
                Route{"/count"}
                    .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                std::function<void()> send) {
                        handle_get_blocklist_count(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_blocklist_count")
            )
            .add_route(
                Route{"/lookup"}
                    .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                std::function<void()> send) {
                        handle_lookup_blocklist_entry(req, res, std::move(send));
                    })
                    .summary("Hyperswitch lookup_blocklist_entry")
            )
            .add_route(
                Route{"/batch"}
                    .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                 std::function<void()> send) {
                        handle_upload_batch_blocklist(req, res, std::move(send));
                    })
                    .summary("Hyperswitch upload_batch_blocklist")
            )
            .add_route(
                Route{"/batch"}
                    .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                std::function<void()> send) {
                        handle_list_batch_blocklist_jobs(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_batch_blocklist_jobs")
            )
            .add_route(
                // core router supports only `:param`; upstream OpenAPI spells `{job_id}`.
                Route{"/batch/:job_id"}
                    .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                std::function<void()> send) {
                        handle_get_batch_blocklist_job_status(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_batch_blocklist_job_status")
            )
            .add_route(
                Route{"/export"}
                    .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                 std::function<void()> send) {
                        handle_create_blocklist_export(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_blocklist_export")
            )
            .add_route(
                Route{"/clone"}
                    .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                 std::function<void()> send) {
                        handle_clone_blocklist_entries(req, res, std::move(send));
                    })
                    .summary("Hyperswitch clone_blocklist_entries")
            );
    }

private:
    using EntryRecord = ::app::store::blocklist_entries::BlocklistEntryRecord;
    using JobRecord = std::remove_cvref_t<
        decltype(*std::declval<app::context::AppContext&>()
                     .repository()
                     .find_routing_config_by_string(std::string_view{}))>;

    // Upstream api_models:10-20 wire `type` tokens -> internal BlocklistDataKind values.
    // `fingerprint` is stored as `payment_method` internally (common_enums/enums.rs:834-840).
    struct EntryWire
    {
        std::string kind{};
        std::string data{};
        std::string transaction_type{};
        std::optional<core::error::AppError> failure{};
    };

    struct CsvRow
    {
        std::string kind{};
        std::string data{};
        std::string transaction_type{};
    };

    struct CsvParse
    {
        std::vector<CsvRow> rows{};
        std::optional<core::error::AppError> failure{};
    };

    struct MultipartUpload
    {
        std::string_view content{};
        std::string file_name{};
    };

    struct ProfileResolution
    {
        std::string id{};
        std::optional<core::error::AppError> failure{};
    };

    template<typename T>
    [[nodiscard]] T wrap(std::string value) const
    {
        T result{};
        result.set_value(std::move(value));
        return result;
    }

    [[nodiscard]] core::error::AppError error(
        core::error::AppErrorKind kind,
        std::string message
    ) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    template<typename T>
    [[nodiscard]] std::optional<T> query_number(
        const app::http::QueryParams& query,
        std::string_view key
    ) const
    {
        const auto raw = query.get(key);
        if (!raw) {
            return std::nullopt;
        }
        T value{};
        const auto [end, ec] = std::from_chars(raw->data(), raw->data() + raw->size(), value);
        (void)end;
        if (ec != std::errc{}) {
            return std::nullopt;
        }
        return value;
    }

    [[nodiscard]] std::optional<core::error::AppError> require_merchant(
        std::string_view merchant
    ) const
    {
        if (merchant.empty()) {
            return error(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant id");
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string merchant_id_of(interfaces::io::IRequest& req) const
    {
        return app::http::merchant_id_from_header{}(req);
    }

    /** @brief auth.profile: JWT profile or X-Profile-Id header (auth/context.cppm:156-158). */
    [[nodiscard]] std::string explicit_profile_of(interfaces::io::IRequest& req) const
    {
        return app::auth::AuthContextBuilder{}(req).profile_id();
    }

    // core/utils.rs:2414-2450 get_profile_from_business_details.
    [[nodiscard]] ProfileResolution resolve_required_profile(
        interfaces::io::IRequest& req,
        std::string_view merchant
    ) const
    {
        const auto explicit_id = explicit_profile_of(req);
        if (!explicit_id.empty()) {
            const auto found = m_context->repository().find_profile_by_string(explicit_id);
            if (!found || (found->get_merchant_id() != merchant)) {
                return {explicit_id, profile_not_found(explicit_id)};
            }
            return {explicit_id, std::nullopt};
        }
        const auto account = m_context->repository().find_merchant_account_by_string(merchant);
        const auto default_id = account ? account->get_default_profile() : std::string{};
        if (!default_id.empty()) {
            const auto found = m_context->repository().find_profile_by_string(default_id);
            if (!found || (found->get_merchant_id() != merchant)) {
                return {default_id, profile_not_found(default_id)};
            }
            return {default_id, std::nullopt};
        }
        return {{}, error(core::error::AppErrorKind::INVALID_REQUEST,
                          "Missing required param: profile_id or business_country, business_label")};
    }

    [[nodiscard]] core::error::AppError profile_not_found(std::string_view id) const
    {
        return error(core::error::AppErrorKind::NOT_FOUND,
                     "Business profile with the given id " + std::string{id} + " does not exist");
    }

    [[nodiscard]] bool profile_is_owned(std::string_view profile_id, std::string_view merchant) const
    {
        if (profile_id.empty() || merchant.empty()) {
            return false;
        }
        const auto found = m_context->repository().find_profile_by_string(profile_id);
        return found.has_value() && found->get_merchant_id() == merchant;
    }

    [[nodiscard]] std::string job_id_of(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        for (std::size_t i = 0; i + 1 < segments.count(); ++i) {
            if (segments.at(i).value() == "batch") {
                return std::string{segments.at(i + 1).value()};
            }
        }
        return {};
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize_operation(
        interfaces::io::IRequest& req,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        const auto context = app::auth::AuthContextBuilder{}(req);
        return m_context->authorizer().authorize_route(context, rule);
    }

    // ---- wire parsing/validation -------------------------------------------------

    [[nodiscard]] std::optional<std::string> kind_from_type(std::string_view type) const
    {
        if (type == "fingerprint") {
            return std::string{"payment_method"};
        }
        if (type == "card_bin" || type == "extended_card_bin" || type == "generic_card_bin") {
            return std::string{type};
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::string> query_kind(std::string_view raw) const
    {
        if (raw == "payment_method") {
            return std::string{"payment_method"};
        }
        if (raw == "card_bin" || raw == "extended_card_bin" || raw == "generic_card_bin") {
            return std::string{raw};
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string type_from_kind(std::string_view kind) const
    {
        return kind == "payment_method" ? std::string{"fingerprint"} : std::string{kind};
    }

    [[nodiscard]] std::optional<std::string> parse_transaction_type(std::string_view raw) const
    {
        if (raw == "payment" || raw == "payout") {
            return std::string{raw};
        }
        return std::nullopt;
    }

    // api_models:36-43 BlocklistEntryRequest (flattened tagged enum + transaction_type).
    [[nodiscard]] EntryWire parse_entry_body(std::string_view body) const
    {
        EntryWire wire;
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!generic) {
            wire.failure = error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            return wire;
        }
        const auto* root = std::get_if<interfaces::Value::Object>(&generic->get());
        if (root == nullptr) {
            wire.failure = error(core::error::AppErrorKind::INVALID_REQUEST,
                                 "invalid blocklist entry request");
            return wire;
        }
        const auto type_field = root->get("type");
        if (!type_field) {
            wire.failure = error(core::error::AppErrorKind::INVALID_REQUEST,
                                 "missing field `type`");
            return wire;
        }
        const auto* type_text = std::get_if<std::string>(&type_field->get());
        if (type_text == nullptr) {
            wire.failure = error(core::error::AppErrorKind::INVALID_REQUEST,
                                 "invalid field `type`");
            return wire;
        }
        const auto kind = kind_from_type(*type_text);
        if (!kind) {
            wire.failure = error(core::error::AppErrorKind::INVALID_REQUEST,
                                 "unknown variant `" + *type_text +
                                 "`, expected one of `card_bin`, `fingerprint`, "
                                 "`extended_card_bin`, `generic_card_bin`");
            return wire;
        }
        const auto data_field = root->get("data");
        if (!data_field) {
            wire.failure = error(core::error::AppErrorKind::INVALID_REQUEST,
                                 "missing field `data`");
            return wire;
        }
        const auto* data_text = std::get_if<std::string>(&data_field->get());
        if (data_text == nullptr) {
            wire.failure = error(core::error::AppErrorKind::INVALID_REQUEST,
                                 "invalid field `data`");
            return wire;
        }
        wire.kind = *kind;
        wire.data = *data_text;
        wire.transaction_type = "payment"; // serde(default) at api_models:40.
        if (const auto tt_field = root->get("transaction_type")) {
            const auto* tt_text = std::get_if<std::string>(&tt_field->get());
            if (tt_text == nullptr) {
                wire.failure = error(core::error::AppErrorKind::INVALID_REQUEST,
                                     "invalid transaction_type");
                return wire;
            }
            const auto parsed = parse_transaction_type(*tt_text);
            if (!parsed) {
                wire.failure = error(core::error::AppErrorKind::INVALID_REQUEST,
                                     "unknown variant `" + *tt_text +
                                     "`, expected `payment` or `payout`");
                return wire;
            }
            wire.transaction_type = *parsed;
        }
        // utils.rs:164-190 validate_bin (fingerprints are opaque: no digit format).
        wire.failure = validate_bin(wire.data, wire.kind);
        return wire;
    }

    [[nodiscard]] std::optional<core::error::AppError> validate_bin(
        std::string_view data,
        std::string_view kind
    ) const
    {
        std::string expected;
        std::size_t length{0};
        if (kind == "card_bin") {
            expected = "a 6 digit number";
            length = 6;
        } else if (kind == "extended_card_bin") {
            expected = "an 8 digit number";
            length = 8;
        } else if (kind == "generic_card_bin") {
            expected = "a 6 to 10 digit number";
        } else {
            return std::nullopt; // payment_method / fingerprint: opaque.
        }
        const bool digits = std::all_of(
            data.begin(), data.end(), [](unsigned char ch) { return std::isdigit(ch) != 0; }
        );
        const bool valid_length = kind == "generic_card_bin"
                                      ? (data.size() >= 6 && data.size() <= 10)
                                      : data.size() == length;
        if (!digits || !valid_length) {
            return error(core::error::AppErrorKind::VALIDATION_FAILED,
                         "data contains invalid data. Expected format is " + expected);
        }
        return std::nullopt;
    }

    // ---- multipart/CSV (batch.rs:82-232, 347-510) --------------------------------

    [[nodiscard]] MultipartUpload multipart_file(
        interfaces::io::IRequest& req,
        std::string_view body
    ) const
    {
        const auto content_type = app::http::HeaderReader{}.get(req, "content-type");
        if (!content_type.starts_with("multipart/form-data")) {
            return {body, {}};
        }
        const auto marker = content_type.find("boundary=");
        if (marker == std::string::npos) {
            return {body, {}};
        }
        auto boundary = content_type.substr(marker + 9);
        if (!boundary.empty() && boundary.front() == '"') {
            boundary = boundary.substr(1);
            if (!boundary.empty() && boundary.back() == '"') {
                boundary = boundary.substr(0, boundary.size() - 1);
            }
        }
        const auto delimiter = std::string{"--"} + boundary;
        const auto header_end = body.find("\r\n\r\n");
        if (header_end == std::string_view::npos) {
            return {body, {}};
        }
        // Per-part headers carry name="file" and filename="...".
        std::string file_name;
        const auto part_headers = body.substr(0, header_end);
        const auto name_pos = part_headers.find("filename=");
        if (name_pos != std::string_view::npos) {
            auto quoted = part_headers.find('"', name_pos);
            if (quoted != std::string_view::npos) {
                const auto end = part_headers.find('"', ++quoted);
                if (end != std::string_view::npos) {
                    file_name = std::string{part_headers.substr(quoted, end - quoted)};
                }
            }
        }
        const auto start = header_end + 4;
        const auto tail = body.find(delimiter, start);
        return {tail == std::string_view::npos ? body.substr(start)
                                               : body.substr(start, tail - start),
                file_name};
    }

    [[nodiscard]] CsvParse parse_blocklist_csv(
        std::string_view csv,
        std::string_view override_type
    ) const
    {
        CsvParse parsed;
        std::vector<std::string_view> lines;
        std::size_t cursor{0};
        while (cursor <= csv.size()) {
            const auto newline = csv.find('\n', cursor);
            const auto end = newline == std::string_view::npos ? csv.size() : newline;
            auto line = csv.substr(cursor, end - cursor);
            if (!line.empty() && line.back() == '\r') {
                line = line.substr(0, line.size() - 1);
            }
            lines.push_back(line);
            if (newline == std::string_view::npos) {
                break;
            }
            cursor = newline + 1;
        }
        if (lines.empty()) {
            parsed.failure = error(core::error::AppErrorKind::VALIDATION_FAILED,
                                   "CSV must contain at least one valid data row");
            return parsed;
        }
        const auto header = split_csv_line(lines.front());
        std::size_t type_col{0};
        std::size_t data_col{1};
        std::size_t metadata_col{2};
        std::optional<std::size_t> tt_col{};
        for (std::size_t i = 0; i < header.size(); ++i) {
            const auto name = trim(header[i]);
            if (name == "type") {
                type_col = i;
            } else if (name == "data") {
                data_col = i;
            } else if (name == "metadata") {
                metadata_col = i;
            } else if (name == "transaction_type") {
                tt_col = i;
            }
        }
        const bool has_type = std::any_of(header.begin(), header.end(), [this](std::string_view v) {
            return trim(v) == "type";
        });
        const bool has_data = std::any_of(header.begin(), header.end(), [this](std::string_view v) {
            return trim(v) == "data";
        });
        if (!has_type || !has_data) {
            parsed.failure = csv_row_error(0, "card_bin", "", "missing `type` or `data` column");
            return parsed;
        }
        if (lines.size() - 1 > 100000) {
            parsed.failure = error(core::error::AppErrorKind::VALIDATION_FAILED,
                                   "CSV exceeds maximum allowed rows (100000); got " +
                                       std::to_string(lines.size() - 1));
            return parsed;
        }
        if (lines.size() == 1) {
            parsed.failure = error(core::error::AppErrorKind::VALIDATION_FAILED,
                                   "CSV must contain at least one valid data row");
            return parsed;
        }
        (void)metadata_col;
        for (std::size_t i = 1; i < lines.size(); ++i) {
            if (lines[i].empty()) {
                continue;
            }
            const auto cells = split_csv_line(lines[i]);
            const auto kind_token = trim(cells.size() > type_col ? cells[type_col] : "");
            const auto kind = kind_from_type(kind_token);
            if (!kind) {
                parsed.failure = csv_row_error(
                    i - 1, "card_bin", std::string{cells.size() > data_col ? cells[data_col] : ""},
                    "unknown type `" + std::string{kind_token} +
                        "`; expected generic_card_bin, card_bin, extended_card_bin, or fingerprint"
                );
                return parsed;
            }
            const auto data = std::string{trim(cells.size() > data_col ? cells[data_col] : "")};
            if (data.empty()) {
                parsed.failure = csv_row_error(i - 1, *kind, "", "data field must not be empty");
                return parsed;
            }
            if (const auto bad = validate_bin(data, *kind)) {
                const auto reason = *kind == "card_bin"
                                        ? std::string{"card_bin must be exactly 6 digits"}
                                        : *kind == "extended_card_bin"
                                              ? std::string{"extended_card_bin must be exactly 8 digits"}
                                              : std::string{"generic_card_bin must be a 6 to 10 digit number"};
                parsed.failure = csv_row_error(i - 1, *kind, data, reason);
                return parsed;
            }
            const auto metadata = cells.size() > metadata_col
                                      ? trim(cells[metadata_col]) : std::string_view{};
            if (!metadata.empty() && !valid_metadata(metadata)) {
                parsed.failure = csv_row_error(
                    i - 1, *kind, data,
                    "metadata must be in key=value format, separated by semicolons "
                    "(e.g. reason=fraud;source=manual)"
                );
                return parsed;
            }
            std::string transaction_type = "payment";
            if (!override_type.empty()) {
                transaction_type = override_type;
            } else if (tt_col && cells.size() > *tt_col) {
                const auto raw = trim(cells[*tt_col]);
                if (!raw.empty()) {
                    const auto parsed_tt = parse_transaction_type(raw);
                    if (!parsed_tt) {
                        parsed.failure = csv_row_error(
                            i - 1, *kind, data,
                            "unknown variant `" + std::string{raw} + "`, expected `payment` or `payout`"
                        );
                        return parsed;
                    }
                    transaction_type = *parsed_tt;
                }
            }
            parsed.rows.push_back({*kind, data, transaction_type});
        }
        return parsed;
    }

    [[nodiscard]] core::error::AppError csv_row_error(
        std::size_t row_index,
        std::string_view kind,
        std::string data,
        std::string reason
    ) const
    {
        app::model::blocklist::BlocklistRowError row;
        row.set_row_index(row_index);
        row.set_data_kind(wrap<app::model::blocklist::BlocklistDataKindValue>(std::string{kind}));
        row.set_data(std::move(data));
        row.set_reason(std::move(reason));
        // batch.rs:314-324 serializes BlocklistRowError into InvalidRequestData.message.
        return error(core::error::AppErrorKind::VALIDATION_FAILED, serde::Ser::encode_json(row));
    }

    [[nodiscard]] bool valid_metadata(std::string_view raw) const
    {
        std::size_t cursor{0};
        while (cursor <= raw.size()) {
            const auto semi = raw.find(';', cursor);
            const auto pair = raw.substr(cursor, (semi == std::string_view::npos ? raw.size() : semi) - cursor);
            const auto equals = pair.find('=');
            if (equals == std::string_view::npos || trim(pair.substr(0, equals)).empty()) {
                return false;
            }
            if (semi == std::string_view::npos) {
                break;
            }
            cursor = semi + 1;
        }
        return true;
    }

    [[nodiscard]] std::vector<std::string_view> split_csv_line(std::string_view line) const
    {
        std::vector<std::string_view> parts;
        std::size_t start{0};
        while (true) {
            const auto comma = line.find(',', start);
            if (comma == std::string_view::npos) {
                parts.push_back(line.substr(start));
                break;
            }
            parts.push_back(line.substr(start, comma - start));
            start = comma + 1;
        }
        return parts;
    }

    [[nodiscard]] std::string_view trim(std::string_view raw) const
    {
        while (!raw.empty() && std::isspace(static_cast<unsigned char>(raw.front()))) {
            raw.remove_prefix(1);
        }
        while (!raw.empty() && std::isspace(static_cast<unsigned char>(raw.back()))) {
            raw.remove_suffix(1);
        }
        return raw;
    }

    // ---- storage / filtering ----------------------------------------------------

    [[nodiscard]] std::string identity_key(
        std::string_view merchant,
        std::string_view profile,
        std::string_view fingerprint,
        std::string_view transaction_type
    ) const
    {
        return std::format("blocklist:{}:{}:{}:{}", merchant, profile, fingerprint, transaction_type);
    }

    [[nodiscard]] std::string job_key(std::string_view job_id) const
    {
        return "blocklist.job:" + std::string{job_id};
    }

    // common_enums BlocklistDataKind: payment_method is the fingerprint kind.
    [[nodiscard]] bool equivalent_kind(std::string_view wanted, std::string_view stored) const
    {
        if (wanted == "generic_card_bin") {
            return stored == "generic_card_bin" || stored == "card_bin" ||
                   stored == "extended_card_bin";
        }
        return stored == wanted;
    }

    [[nodiscard]] bool matches_merchant(const EntryRecord& record, std::string_view merchant) const
    {
        return merchant.empty() || record.get_merchant_id().empty() ||
               record.get_merchant_id() == merchant;
    }

    [[nodiscard]] bool matches_merchant(const JobRecord& record, std::string_view merchant) const
    {
        return merchant.empty() || record.get_merchant_id().empty() ||
               record.get_merchant_id() == merchant;
    }

    // Legacy NULL profile rows are merchant-wide: profile_id = p OR profile_id IS NULL.
    [[nodiscard]] bool matches_profile(const EntryRecord& record, std::string_view profile) const
    {
        return profile.empty() || record.get_profile_id().empty() ||
               record.get_profile_id() == profile;
    }

    [[nodiscard]] bool matches_profile(const JobRecord& record, std::string_view profile) const
    {
        return profile.empty() || record.get_profile_id().empty() ||
               record.get_profile_id() == profile;
    }

    // Legacy NULL transaction namespaces are payment, never payout.
    [[nodiscard]] bool tt_matches(const EntryRecord& record, std::string_view transaction_type) const
    {
        if (transaction_type.empty()) {
            return true;
        }
        const auto stored = legacy_transaction_type(record);
        if (transaction_type == "payment") {
            return stored == "payment";
        }
        return stored == transaction_type;
    }

    [[nodiscard]] std::string legacy_transaction_type(const EntryRecord& record) const
    {
        return record.get_transaction_type().empty() ? std::string{"payment"}
                                                     : record.get_transaction_type();
    }

    [[nodiscard]] bool matches_scope(
        const EntryRecord& record,
        std::string_view merchant,
        std::string_view profile,
        std::string_view data_kind,
        std::string_view transaction_type
    ) const
    {
        return matches_merchant(record, merchant) && matches_profile(record, profile) &&
               equivalent_kind(data_kind, record.get_data_kind()) &&
               tt_matches(record, transaction_type);
    }

    [[nodiscard]] EntryRecord make_entry(
        std::string_view fingerprint,
        std::string_view merchant,
        std::string_view profile,
        std::string_view data_kind,
        std::string_view transaction_type,
        std::string_view created
    ) const
    {
        EntryRecord record{};
        record.set_entry_id(identity_key(merchant, profile, fingerprint, transaction_type));
        record.set_merchant_id(std::string{merchant});
        record.set_profile_id(std::string{profile});
        record.set_data_kind(std::string{data_kind});
        record.set_transaction_type(std::string{transaction_type});
        record.set_fingerprint(std::string{fingerprint});
        record.set_created_at(std::string{created});
        return record;
    }

    [[nodiscard]] std::optional<EntryRecord> find_entry(std::string_view key) const
    {
        return m_context->repository().find_blocklist_entry_by_string(key);
    }

    [[nodiscard]] std::optional<EntryRecord> find_by_identity(
        std::string_view merchant,
        std::string_view profile,
        std::string_view fingerprint,
        std::string_view transaction_type
    ) const
    {
        for (const auto& record : list_entries()) {
            if (!matches_merchant(record, merchant) || !matches_profile(record, profile)) {
                continue;
            }
            if (record.get_fingerprint() != fingerprint || !tt_matches(record, transaction_type)) {
                continue;
            }
            return record;
        }
        return std::nullopt;
    }

    void save_entry(EntryRecord record) const
    {
        if (const auto existing = find_entry(record.get_entry_id())) {
            record.set_id(existing->get_id());
            m_context->repository().update_blocklist_entry(record);
        } else {
            m_context->repository().save_blocklist_entry(std::move(record));
        }
    }

    bool remove_entry(std::int64_t id) const
    {
        return m_context->repository().remove_blocklist_entry(id);
    }

    [[nodiscard]] std::vector<EntryRecord> list_entries() const
    {
        return m_context->repository().list_blocklist_entries();
    }

    void sort_by_created_desc(std::vector<EntryRecord>& records) const
    {
        std::sort(records.begin(), records.end(), [](const EntryRecord& lhs, const EntryRecord& rhs) {
            return lhs.get_created_at() > rhs.get_created_at();
        });
    }

    [[nodiscard]] app::model::blocklist::BlocklistResponse response_of(
        const EntryRecord& record
    ) const
    {
        app::model::blocklist::BlocklistResponse response;
        response.set_fingerprint_id(record.get_fingerprint());
        response.set_data_kind(wrap<app::model::blocklist::BlocklistDataKindValue>(record.get_data_kind()));
        response.set_transaction_type(
            wrap<app::model::blocklist::BlocklistTransactionTypeValue>(legacy_transaction_type(record))
        );
        response.set_created_at(
            wrap<app::model::blocklist::PrimitiveDateTimeValue>(record.get_created_at())
        );
        if (!record.get_profile_id().empty()) {
            response.set_profile_id(wrap<app::model::blocklist::ProfileIdValue>(record.get_profile_id()));
        }
        return response;
    }

    // ---- jobs --------------------------------------------------------------------

    [[nodiscard]] JobRecord config_record(
        std::string_view config_id,
        std::string_view merchant,
        std::string_view profile,
        std::string_view name,
        std::string_view description,
        std::string_view algorithm,
        std::string_view config_json
    ) const
    {
        JobRecord record{};
        record.set_config_id(std::string{config_id});
        record.set_merchant_id(std::string{merchant});
        record.set_profile_id(std::string{profile});
        record.set_name(std::string{name});
        record.set_description(std::string{description});
        record.set_algorithm(std::string{algorithm});
        record.set_config_json(std::string{config_json});
        return record;
    }

    [[nodiscard]] std::optional<JobRecord> find_job(std::string_view config_id) const
    {
        return m_context->repository().find_routing_config_by_string(config_id);
    }

    void upsert_job_record(JobRecord record) const
    {
        auto repository = m_context->repository();
        if (const auto existing = repository.find_routing_config_by_string(record.get_config_id())) {
            record.set_id(existing->get_id());
            repository.update_routing_config(record);
        } else {
            repository.save_routing_config(std::move(record));
        }
    }

    [[nodiscard]] std::vector<JobRecord> list_jobs(std::string_view prefix) const
    {
        std::vector<JobRecord> selected;
        for (auto& record : m_context->repository().list_routing_configs()) {
            if (record.get_config_id().starts_with(prefix)) {
                selected.push_back(std::move(record));
            }
        }
        return selected;
    }

    [[nodiscard]] bool known_job_type(std::string_view value) const
    {
        return value == "upload" || value == "export" || value == "profile_clone";
    }

    void upsert_job(
        std::string_view job_id,
        std::string_view merchant,
        std::string_view profile,
        std::string_view job_type,
        std::string_view status,
        std::uint32_t total_rows,
        std::string_view transaction_type,
        std::string_view file_name,
        std::string_view download_token,
        bool downloadable,
        std::optional<app::model::blocklist::ProfileCloneJobMetadata> metadata = std::nullopt,
        std::uint32_t succeeded_rows = 0,
        std::uint32_t failed_rows = 0,
        std::string_view error_message = {},
        std::string_view expires_at = {},
        std::string_view download_url_expires_at = {}
    ) const
    {
        app::model::blocklist::BatchBlocklistJobStatusResponse job;
        job.set_job_id(std::string{job_id});
        job.set_merchant_id(std::string{merchant});
        if (!profile.empty()) {
            job.set_profile_id(wrap<app::model::blocklist::ProfileIdValue>(std::string{profile}));
        }
        job.set_job_type(wrap<app::model::blocklist::BatchBlocklistJobTypeValue>(std::string{job_type}));
        job.set_status(wrap<app::model::blocklist::BatchBlocklistJobStatusValue>(std::string{status}));
        job.set_total_rows(total_rows);
        job.set_succeeded_rows(succeeded_rows);
        job.set_failed_rows(failed_rows);
        job.set_created_at(wrap<app::model::blocklist::PrimitiveDateTimeValue>(now_iso8601()));
        job.set_updated_at(wrap<app::model::blocklist::PrimitiveDateTimeValue>(now_iso8601()));
        job.set_downloadable(downloadable);
        if (!file_name.empty()) {
            job.set_file_name(std::string{file_name});
        }
        if (!download_token.empty()) {
            job.set_download_url(std::string{download_token});
        }
        if (!error_message.empty()) {
            job.set_error_message(std::string{error_message});
        }
        if (!expires_at.empty()) {
            job.set_expires_at(wrap<app::model::blocklist::PrimitiveDateTimeValue>(std::string{expires_at}));
        }
        if (!download_url_expires_at.empty()) {
            job.set_download_url_expires_at(
                wrap<app::model::blocklist::PrimitiveDateTimeValue>(std::string{download_url_expires_at})
            );
        }
        if (metadata) {
            job.set_metadata(std::move(metadata));
        }
        auto record = config_record(job_key(job_id), merchant, profile, job_type, status,
                                    "tt=" + std::string{transaction_type},
                                    serde::Ser::encode_json(job));
        record.set_created_at(now_iso8601());
        upsert_job_record(std::move(record));
    }

    [[nodiscard]] app::model::blocklist::BatchBlocklistJobStatusResponse job_status_of(
        const JobRecord& stored
    ) const
    {
        app::model::blocklist::BatchBlocklistJobStatusResponse response;
        if (auto parsed = serde::Ser::deserialize<app::model::blocklist::BatchBlocklistJobStatusResponse>(
                "application/json", stored.get_config_json())) {
            response = std::move(*parsed);
        } else {
            response.set_job_id(stored.get_config_id().substr(std::string{"blocklist.job:"}.size()));
            response.set_merchant_id(stored.get_merchant_id());
            response.set_job_type(
                wrap<app::model::blocklist::BatchBlocklistJobTypeValue>(stored.get_name())
            );
            response.set_status(
                wrap<app::model::blocklist::BatchBlocklistJobStatusValue>(stored.get_description())
            );
        }
        if (!response.get_profile_id() && !stored.get_profile_id().empty()) {
            response.set_profile_id(
                wrap<app::model::blocklist::ProfileIdValue>(stored.get_profile_id())
            );
        }
        return response;
    }

    // export.rs:164-174 is_export_downloadable.
    [[nodiscard]] bool is_export_downloadable(
        const app::model::blocklist::BatchBlocklistJobStatusResponse& job
    ) const
    {
        return job.get_job_type().get_value() == "export" &&
               job.get_status().get_value() == "completed" && job.get_file_name().has_value() &&
               job.get_downloadable();
    }

    [[nodiscard]] std::string signed_download_url(
        std::string_view file_name,
        std::string_view expires
    ) const
    {
        // In-process substitute for S3 presigning (export.rs:177-213).
        const auto token = app::crypto::JwtSigner{"blocklist.export"}.sign(
            std::string{file_name} + ":" + std::string{expires}
        );
        return std::format("http://localhost/blocklist/export/{}?token={}&expires={}",
                           file_name, token, expires);
    }

    void schedule_job(
        std::string_view name,
        std::string_view job_id,
        std::string_view merchant,
        std::string_view profile,
        std::size_t rows
    ) const
    {
        const auto payload = std::format(
            R"({{"job_id":"{}","merchant_id":"{}","profile_id":"{}","rows":{}}})",
            job_id, merchant, profile, rows
        );
        m_context->process_trackers().schedule(std::string{name}, payload, "0");
    }

    // ---- export CSV --------------------------------------------------------------

    // batch.rs:45-46 CSV_HEADER + export.rs:64-70 rows_to_csv_bytes.
    [[nodiscard]] std::string export_csv(
        std::string_view merchant,
        std::string_view profile,
        std::string_view transaction_type
    ) const
    {
        std::string csv{"type,data,metadata,transaction_type\n"};
        for (const auto& record : list_entries()) {
            if (!matches_scope(record, merchant, profile, "payment_method", {}) &&
                !matches_scope(record, merchant, profile, "card_bin", {}) &&
                !matches_scope(record, merchant, profile, "extended_card_bin", {}) &&
                !matches_scope(record, merchant, profile, "generic_card_bin", {})) {
                continue;
            }
            if (!transaction_type.empty() && !tt_matches(record, transaction_type)) {
                continue;
            }
            csv += std::format("{},{},,\"{}\"\n", type_from_kind(record.get_data_kind()),
                               record.get_fingerprint(), legacy_transaction_type(record));
        }
        return csv;
    }

    // ---- time / response helpers -------------------------------------------------

    // common_utils/custom_serde.rs:16-18: YYYY-MM-DDTHH:MM:SS.mmmZ.
    [[nodiscard]] std::string now_iso8601(std::int64_t offset_seconds = 0) const
    {
        const auto now = std::chrono::system_clock::now() + std::chrono::seconds{offset_seconds};
        const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                                now.time_since_epoch()
                            )
                                .count();
        const auto seconds = millis / 1000;
        const auto days = seconds / 86400;
        const auto sod = seconds % 86400;
        const auto [year, month, day] = civil_from_days(days);
        return std::format("{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}.{:03d}Z",
                           year, month, day,
                           sod / 3600, (sod % 3600) / 60, sod % 60,
                           static_cast<int>(millis % 1000));
    }

    [[nodiscard]] std::tuple<int, int, int> civil_from_days(std::int64_t z) const
    {
        z += 719468;
        const auto era = (z >= 0 ? z : z - 146096) / 146097;
        const auto doe = static_cast<unsigned long>(z - era * 146097);
        const auto yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
        auto year = static_cast<int>(yoe) + static_cast<int>(era) * 400;
        const auto doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        const auto mp = (5 * doy + 2) / 153;
        const auto day = static_cast<int>(doy - (153 * mp + 2) / 5 + 1);
        const auto month = static_cast<int>(mp + (mp < 10 ? 3 : -9));
        year += (month <= 2) ? 1 : 0;
        return {year, month, day};
    }

    [[nodiscard]] std::string value_of(
        const std::optional<app::model::blocklist::BlocklistTransactionTypeValue>& value
    ) const
    {
        return value ? value->get_value() : std::string{};
    }

    template<typename T>
    [[nodiscard]] std::vector<T> paginate(
        std::vector<T> items,
        std::size_t offset,
        std::size_t limit
    ) const
    {
        if (offset >= items.size()) {
            return {};
        }
        auto begin = items.begin() + static_cast<std::ptrdiff_t>(offset);
        if (limit == 0 || limit >= static_cast<std::size_t>(items.end() - begin)) {
            return std::vector<T>{begin, items.end()};
        }
        return std::vector<T>{begin, begin + static_cast<std::ptrdiff_t>(limit)};
    }

    template<typename T>
    void emit(std::string_view topic, const T& payload) const
    {
        m_context->events().publish(std::string{topic}, serde::Ser::encode_json(payload));
    }

    void track(std::string_view flow) const
    {
        const auto metric = std::string{"blocklist."} + std::string{flow};
        m_context->metrics().counter(metric).increment();
        m_context->metrics().record_flow(metric);
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError failure
    ) const
    {
        auto text = serde::Ser::encode_json(failure);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(failure.get_kind()));
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

} // namespace app::handlers::blocklist
