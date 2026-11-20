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

export namespace app::handlers::blocklist {

class BlocklistRouteHandlers
{
public:
    explicit BlocklistRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

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

        using Input = app::model::blocklist::BlocklistEntryRequest;
        if (!serde::Ser::deserialize<Input>("application/json", body)) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto wire = entry_from_body(body);
        if (auto invalid = validate_entry(wire)) {
            send_error(res, send, *invalid);
            return;
        }
        const auto kind = wire.kind;
        const auto transaction_type = normalize_transaction_type(wire.transaction_type);
        const auto merchant = merchant_id_of(req);
        const auto profile = resolve_profile(req);
        if (const auto denied = profile_error(req, profile)) {
            send_error(res, send, *denied);
            return;
        }

        const auto fingerprint = fingerprint_for(kind, wire.data, merchant);
        const auto key = entry_key(merchant, profile, fingerprint, transaction_type);
        if (find_entry(key)) {
            send_error(res, send, error(core::error::AppErrorKind::ALREADY_EXISTS, "data associated with the given fingerprint is already blocked"));
            return;
        }

        const auto created = now_stamp();
        save_entry(make_entry(key, merchant, profile, kind, transaction_type, fingerprint, wire.data.size(), created));

        app::model::blocklist::BlocklistResponse response;
        response.set_fingerprint_id(fingerprint);
        response.set_data_kind(wrap<app::model::blocklist::BlocklistDataKindValue>(kind));
        response.set_transaction_type(wrap<app::model::blocklist::BlocklistTransactionTypeValue>(transaction_type));
        response.set_created_at(wrap<app::model::blocklist::PrimitiveDateTimeValue>(created));
        if (!profile.empty()) {
            response.set_profile_id(wrap<app::model::blocklist::ProfileIdValue>(profile));
        }
        emit("blocklist.entry.added", response);
        track("add_entry_to_blocklist");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

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

        using Input = app::model::blocklist::BlocklistEntryRequest;
        if (!serde::Ser::deserialize<Input>("application/json", body)) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto wire = entry_from_body(body);
        if (auto invalid = validate_entry(wire)) {
            send_error(res, send, *invalid);
            return;
        }
        const auto transaction_type = normalize_transaction_type(wire.transaction_type);
        const auto merchant = merchant_id_of(req);
        const auto profile = resolve_profile(req);
        if (const auto denied = profile_error(req, profile)) {
            send_error(res, send, *denied);
            return;
        }
        const auto fingerprint = fingerprint_for(wire.kind, wire.data, merchant);
        const auto key = entry_key(merchant, profile, fingerprint, transaction_type);
        const auto existing = find_entry(key);
        if (!existing) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "no blocklist record for the given fingerprint was found"));
            return;
        }

        app::model::blocklist::BlocklistResponse response;
        response.set_fingerprint_id(fingerprint);
        response.set_data_kind(wrap<app::model::blocklist::BlocklistDataKindValue>(wire.kind));
        response.set_transaction_type(wrap<app::model::blocklist::BlocklistTransactionTypeValue>(transaction_type));
        response.set_created_at(wrap<app::model::blocklist::PrimitiveDateTimeValue>(existing->get_created_at()));
        if (!profile.empty()) {
            response.set_profile_id(wrap<app::model::blocklist::ProfileIdValue>(profile));
        }

        remove_entry(key);
        emit("blocklist.entry.removed", response);
        track("remove_entry_from_blocklist");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

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
        const auto profile = resolve_profile(req);
        if (const auto denied = profile_error(req, profile)) {
            send_error(res, send, *denied);
            return;
        }
        const auto query = app::http::RequestQuery{}(req);

        app::model::blocklist::ListBlocklistQuery filter;
        if (const auto data_kind = query.get("data_kind")) {
            filter.set_data_kind(wrap<app::model::blocklist::BlocklistDataKindValue>(normalize_data_kind(*data_kind, true)));
        }
        if (const auto transaction_type = query.get("transaction_type")) {
            filter.set_transaction_type(wrap<app::model::blocklist::BlocklistTransactionTypeValue>(normalize_transaction_type(*transaction_type)));
        }
        filter.set_limit(query_number<std::uint16_t>(query, "limit").value_or(10));
        filter.set_offset(query_number<std::uint16_t>(query, "offset").value_or(0));

        const auto data_kind = filter.get_data_kind().get_value();
        const auto transaction_type = filter.get_transaction_type().get_value();
        std::vector<app::model::blocklist::BlocklistResponse> selected;
        if (auto invalid = validate_filter_kind(data_kind)) {
            send_error(res, send, *invalid);
            return;
        }
        for (const auto& record : list_entries()) {
            if (!matches_scope(record, merchant, profile, data_kind, transaction_type)) {
                continue;
            }
            selected.push_back(response_of(record));
        }
        const std::size_t total = selected.size();
        selected = paginate(
            std::move(selected),
            static_cast<std::size_t>(filter.get_offset()),
            static_cast<std::size_t>(filter.get_limit())
        );

        app::model::blocklist::ListBlocklistResponse response;
        response.set_count(selected.size());
        response.set_total_count(total);
        response.set_data(std::move(selected));
        emit("blocklist.list", response);
        track("list_blocked_payment_methods");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

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
        const auto profile = resolve_profile(req);
        if (const auto denied = profile_error(req, profile)) {
            send_error(res, send, *denied);
            return;
        }
        const auto query = app::http::RequestQuery{}(req);

        app::model::blocklist::BlocklistCountQuery filter;
        const auto data_kind = normalize_data_kind(query.get("data_kind").value_or("fingerprint"));
        if (auto invalid = validate_filter_kind(data_kind)) {
            send_error(res, send, *invalid);
            return;
        }
        filter.set_data_kind(wrap<app::model::blocklist::BlocklistDataKindValue>(data_kind));
        std::string transaction_type;
        if (const auto raw = query.get("transaction_type")) {
            transaction_type = normalize_transaction_type(*raw);
            filter.set_transaction_type(wrap<app::model::blocklist::BlocklistTransactionTypeValue>(transaction_type));
        }

        std::unordered_map<std::string, std::size_t> by_length;
        std::size_t total{0};
        for (const auto& record : list_entries()) {
            if (!matches_scope(record, merchant, profile, data_kind, transaction_type)) {
                continue;
            }
            ++total;
            ++by_length[record.get_status()];
        }

        app::model::blocklist::BlocklistCountResponse response;
        response.set_data_kind(filter.get_data_kind());
        response.set_total_count(total);
        if (!by_length.empty()) {
            response.set_counts_by_length(std::move(by_length));
        }
        emit("blocklist.count", response);
        track("get_blocklist_count");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

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
        const auto profile = resolve_profile(req);
        if (const auto denied = profile_error(req, profile)) {
            send_error(res, send, *denied);
            return;
        }
        const auto query = app::http::RequestQuery{}(req);

        app::model::blocklist::BlocklistLookupQuery filter;
        if (const auto data = query.get("data")) {
            filter.set_data(wrap<app::model::blocklist::BlocklistLookupDataValue>(*data));
        }
        if (const auto transaction_type = query.get("transaction_type")) {
            filter.set_transaction_type(wrap<app::model::blocklist::BlocklistTransactionTypeValue>(normalize_transaction_type(*transaction_type)));
        }
        const auto data = filter.get_data().get_value();
        if (data.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "missing lookup data"));
            return;
        }
        if (data.size() > 20) {
            send_error(res, send, error(core::error::AppErrorKind::VALIDATION_FAILED, "lookup data exceeds 20 characters"));
            return;
        }

        const auto wanted = value_of(filter.get_transaction_type());
        const auto candidates = candidate_fingerprints(data, merchant);
        std::vector<std::string> distinct;
        for (const auto& record : list_entries()) {
            if (!merchant.empty() && record.get_merchant_id() != merchant) {
                continue;
            }
            if (!profile.empty() && !record.get_profile_id().empty() && record.get_profile_id() != profile) {
                continue;
            }
            if (!candidates.contains(record.get_fingerprint())) {
                continue;
            }
            const auto tt = legacy_transaction_type(record);
            if (!wanted.empty() && tt != wanted) {
                continue;
            }
            distinct.push_back(tt);
        }
        std::sort(distinct.begin(), distinct.end());
        distinct.erase(std::unique(distinct.begin(), distinct.end()), distinct.end());

        std::vector<app::model::blocklist::BlocklistTransactionTypeValue> types;
        types.reserve(distinct.size());
        for (const auto& tt : distinct) {
            types.push_back(wrap<app::model::blocklist::BlocklistTransactionTypeValue>(tt));
        }

        app::model::blocklist::BlocklistLookupResponse response;
        response.set_data(data);
        response.set_blocked(!types.empty());
        response.set_transaction_types(std::move(types));
        emit("blocklist.lookup", response);
        track("lookup_blocklist_entry");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

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
        auto status = query_bool(query, "status");
        if (!query.get("status") && !req.body_as_string().empty()) {
            if (auto typed = serde::Ser::deserialize<app::model::blocklist::ToggleBlocklistQuery>(
                    "application/json",
                    req.body_as_string()
                )) {
                status = typed->get_status();
            }
        }

        const auto merchant = merchant_id_of(req);
        const auto profile = resolve_profile(req);
        if (const auto denied = profile_error(req, profile)) {
            send_error(res, send, *denied);
            return;
        }
        const auto text = status ? "enabled" : "disabled";
        // Guard state is a real config-store record so a payment-time reader can consult it.
        m_context->config_store().put("blocklist_guard", guard_key(merchant, profile), text);
        m_context->config_store().put("blocklist_guard", "merchant:" + merchant, text);

        app::model::blocklist::ToggleBlocklistResponse response;
        response.set_blocklist_guard_status(text);
        emit("blocklist.guard.toggled", response);
        track("toggle_blocklist_guard");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

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
        if (const auto transaction_type = query.get("transaction_type")) {
            override_type = normalize_transaction_type(*transaction_type);
        }

        const auto merchant = merchant_id_of(req);
        const auto profile = resolve_profile(req);
        if (const auto denied = profile_error(req, profile)) {
            send_error(res, send, *denied);
            return;
        }

        constexpr std::size_t kMaxUploadBytes = 5U * 1024U * 1024U;
        const auto body = req.body_as_string();
        if (body.size() > kMaxUploadBytes) {
            send_error(res, send, error(core::error::AppErrorKind::VALIDATION_FAILED, "batch upload exceeds 5 MiB"));
            return;
        }
        auto parsed = parse_blocklist_csv(csv_payload(req, body), override_type);
        if (parsed.error) {
            send_error(res, send, *parsed.error);
            return;
        }

        const auto job_id = m_context->ids().payment_id("job_");
        const auto stamp = now_stamp();
        for (const auto& row : parsed.rows) {
            if (!row.valid) {
                continue;
            }
            const auto fingerprint = fingerprint_for(row.kind, row.data, merchant);
            const auto key = entry_key(merchant, profile, fingerprint, row.transaction_type);
            if (!find_entry(key)) {
                save_entry(make_entry(key, merchant, profile, row.kind, row.transaction_type, fingerprint, row.data.size(), stamp));
            }
        }

        upsert_job(
            job_id,
            merchant,
            profile,
            "batch_upload",
            "completed",
            static_cast<std::uint32_t>(parsed.total),
            override_type,
            {},
            {},
            false,
            std::nullopt,
            static_cast<std::uint32_t>(parsed.succeeded),
            static_cast<std::uint32_t>(parsed.failed),
            parsed.first_error
        );
        schedule_job("blocklist_batch_upload", job_id, merchant, profile, parsed.total);

        app::model::blocklist::BatchBlocklistUploadResponse response;
        response.set_job_id(job_id);
        response.set_total_rows(static_cast<std::uint32_t>(parsed.total));
        response.set_status(wrap<app::model::blocklist::BatchBlocklistJobStatusValue>("completed"));
        emit("blocklist.batch.upload", response);
        track("upload_batch_blocklist");
        send_json_status(res, send, serde::Ser::encode_json(response), interfaces::io::types::Status::ACCEPTED);
    }

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
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing job id"));
            return;
        }
        const auto stored = find_job(job_key(job_id));
        if (!stored) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "batch blocklist job not found"));
            return;
        }

        app::model::blocklist::BatchBlocklistJobStatusResponse response;
        if (auto parsed = serde::Ser::deserialize<app::model::blocklist::BatchBlocklistJobStatusResponse>(
                "application/json",
                stored->get_config_json()
            )) {
            response = std::move(*parsed);
        } else {
            response.set_job_id(stored->get_config_id().substr(std::string{"blocklist.job:"}.size()));
            response.set_merchant_id(stored->get_merchant_id());
            response.set_job_type(wrap<app::model::blocklist::BatchBlocklistJobTypeValue>(stored->get_name()));
            response.set_status(wrap<app::model::blocklist::BatchBlocklistJobStatusValue>(stored->get_description()));
        }
        if (!response.get_profile_id() && !stored->get_profile_id().empty()) {
            response.set_profile_id(wrap<app::model::blocklist::ProfileIdValue>(stored->get_profile_id()));
        }
        emit("blocklist.job.status", response);
        track("get_batch_blocklist_job_status");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

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
        const auto profile = resolve_profile(req);
        const auto query = app::http::RequestQuery{}(req);

        app::model::blocklist::ListBatchBlocklistJobsQuery filter;
        if (const auto limit = query.get("limit")) {
            filter.set_limit(wrap<app::model::blocklist::PageSizeValue>(*limit));
        }
        if (const auto offset = query.get("offset")) {
            filter.set_offset(wrap<app::model::blocklist::PageOffsetValue>(*offset));
        }
        if (const auto job_type = query.get("job_type")) {
            filter.set_job_type(wrap<app::model::blocklist::BatchBlocklistJobTypeValue>(*job_type));
        }

        auto records = list_jobs("blocklist.job:");
        std::vector<app::model::blocklist::BatchBlocklistJobStatusResponse> jobs;
        for (const auto& record : records) {
            if (!merchant.empty() && record.get_merchant_id() != merchant) {
                continue;
            }
            if (!profile.empty() && !record.get_profile_id().empty() && record.get_profile_id() != profile) {
                continue;
            }
            if (const auto wanted = value_of(filter.get_job_type());
                !wanted.empty() && record.get_name() != wanted) {
                continue;
            }
            app::model::blocklist::BatchBlocklistJobStatusResponse job;
            if (auto parsed = serde::Ser::deserialize<app::model::blocklist::BatchBlocklistJobStatusResponse>(
                    "application/json",
                    record.get_config_json()
                )) {
                job = std::move(*parsed);
            } else {
                job.set_job_id(record.get_config_id().substr(std::string{"blocklist.job:"}.size()));
                job.set_merchant_id(record.get_merchant_id());
                job.set_job_type(wrap<app::model::blocklist::BatchBlocklistJobTypeValue>(record.get_name()));
                job.set_status(wrap<app::model::blocklist::BatchBlocklistJobStatusValue>(record.get_description()));
            }
            jobs.push_back(std::move(job));
        }
        const std::size_t total = jobs.size();
        jobs = paginate(
            std::move(jobs),
            number_from(filter.get_offset().get_value()),
            number_from(filter.get_limit().get_value())
        );

        app::model::blocklist::ListBatchBlocklistJobsResponse response;
        response.set_count(jobs.size());
        response.set_total_count(total);
        response.set_data(std::move(jobs));
        emit("blocklist.jobs.list", response);
        track("list_batch_blocklist_jobs");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

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

        const auto query = app::http::RequestQuery{}(req);
        std::string transaction_type;
        if (const auto raw = query.get("transaction_type")) {
            transaction_type = normalize_transaction_type(*raw);
        }

        const auto merchant = merchant_id_of(req);
        const auto profile = resolve_profile(req);
        if (const auto denied = profile_error(req, profile)) {
            send_error(res, send, *denied);
            return;
        }

        const auto export_id = m_context->ids().payment_id("exp_");
        const auto file_name = "blocklist-" + export_id + ".csv";
        const auto csv = export_csv(merchant, profile, transaction_type);
        const auto download_token = app::crypto::JwtSigner{"blocklist.export"}.sign(export_id + ":" + merchant);

        ::app::store::files::FileRecord file;
        file.set_file_id(export_id);
        file.set_merchant_id(merchant);
        file.set_profile_id(profile);
        file.set_file_name(file_name);
        file.set_file_type("text/csv");
        file.set_file_size(static_cast<std::int64_t>(csv.size()));
        file.set_purpose("blocklist_export");
        file.set_provider_file_id(file_name);
        file.set_upload_provider("local");
        file.set_status("available");
        file.set_created_at(now_stamp());
        m_context->repository().save_file(std::move(file));

        upsert_job(
            export_id,
            merchant,
            profile,
            "blocklist_export",
            "completed",
            static_cast<std::uint32_t>(std::count(csv.begin(), csv.end(), '\n') > 0
                                           ? std::count(csv.begin(), csv.end(), '\n') - 1
                                           : 0),
            transaction_type,
            file_name,
            download_token,
            true,
            std::nullopt,
            0,
            0,
            {},
            now_stamp(86400),
            now_stamp(900)
        );
        schedule_job("blocklist_export", export_id, merchant, profile, 0);

        app::model::blocklist::BlocklistExportResponse response;
        response.set_export_id(export_id);
        response.set_status(wrap<app::model::blocklist::BatchBlocklistJobStatusValue>("completed"));
        response.set_file_name(file_name);
        emit("blocklist.export.created", response);
        track("create_blocklist_export");
        send_json_status(res, send, serde::Ser::encode_json(response), interfaces::io::types::Status::ACCEPTED);
    }

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
        const auto source = resolve_profile(req);
        if (source.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, "missing profile context"));
            return;
        }
        if (!profile_is_owned(source, merchant)) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "source profile not found for merchant"));
            return;
        }

        std::vector<app::model::blocklist::ProfileIdValue> targets;
        for (const auto& target : typed->get_target_profile_ids()) {
            const auto id = target.get_value();
            if (id.empty() || id == source) {
                send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, "a target is empty or the source profile"));
                return;
            }
            if (!profile_is_owned(id, merchant)) {
                send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "target profile not found for merchant"));
                return;
            }
            const auto exists = std::any_of(
                targets.begin(),
                targets.end(),
                [&id](const app::model::blocklist::ProfileIdValue& item) { return item.get_value() == id; }
            );
            if (!exists) {
                targets.push_back(target);
            }
        }
        if (targets.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "no targets given"));
            return;
        }

        const auto job_id = m_context->ids().payment_id("job_");
        const auto stamp = now_stamp();
        std::size_t cloned{0};
        app::model::blocklist::ProfileCloneJobMetadata metadata;
        std::vector<app::model::blocklist::ProfileCloneTargetMetadata> clone_targets;
        clone_targets.reserve(targets.size());
        for (const auto& target : targets) {
            const auto target_id = target.get_value();
            std::size_t copied{0};
            for (const auto& record : list_entries()) {
                if (record.get_merchant_id() != merchant || record.get_profile_id() != source) {
                    continue;
                }
                const auto key = entry_key(merchant, target_id, record.get_fingerprint(), legacy_transaction_type(record));
                if (!find_entry(key)) {
                    save_entry(make_entry(
                        key,
                        merchant,
                        target_id,
                        record.get_data_kind(),
                        legacy_transaction_type(record),
                        record.get_fingerprint(),
                        number_from(record.get_status()),
                        stamp
                    ));
                }
                ++copied;
            }
            cloned += copied;
            app::model::blocklist::ProfileCloneTargetMetadata item;
            item.set_profile_id(target);
            item.set_status(wrap<app::model::blocklist::BatchBlocklistJobStatusValue>("completed"));
            item.set_processed_rows(static_cast<std::uint32_t>(copied));
            clone_targets.push_back(std::move(item));
        }
        metadata.set_targets(std::move(clone_targets));

        upsert_job(
            job_id,
            merchant,
            source,
            "profile_clone",
            "completed",
            static_cast<std::uint32_t>(targets.size()),
            value_of(typed->get_transaction_type()),
            {},
            {},
            false,
            std::move(metadata),
            static_cast<std::uint32_t>(cloned)
        );
        schedule_job("blocklist_profile_clone", job_id, merchant, source, cloned);

        app::model::blocklist::CloneBlocklistEntriesResponse response;
        response.set_source_profile_id(wrap<app::model::blocklist::ProfileIdValue>(source));
        response.set_job_id(job_id);
        response.set_status(wrap<app::model::blocklist::BatchBlocklistJobStatusValue>("completed"));
        response.set_target_profile_ids(targets);
        emit("blocklist.clone", response);
        track("clone_blocklist_entries");
        send_json_status(res, send, serde::Ser::encode_json(response), interfaces::io::types::Status::ACCEPTED);
    }

    void install(core::router::RouterContext<>& router)
    {
        // Deterministic substitute for the upstream `all(feature = "olap", feature = "v1")`
        // route gate: an injected config row can disable this scope at wiring time.
        if (const auto enabled = m_context->config_store().get_value("route_features", "blocklist");
            enabled.has_value() && *enabled == "false") {
            return;
        }

        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/blocklist"}
            .add_route(
                Route{"/"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_blocked_payment_methods(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_blocked_payment_methods")
            )
            .add_route(
                Route{"/"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_add_entry_to_blocklist(req, res, std::move(send));
                    })
                    .summary("Hyperswitch add_entry_to_blocklist")
            )
            .add_route(
                Route{"/"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_remove_entry_from_blocklist(req, res, std::move(send));
                    })
                    .summary("Hyperswitch remove_entry_from_blocklist")
            )
            .add_route(
                Route{"/toggle"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_toggle_blocklist_guard(req, res, std::move(send));
                    })
                    .summary("Hyperswitch toggle_blocklist_guard")
            )
            .add_route(
                Route{"/count"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_blocklist_count(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_blocklist_count")
            )
            .add_route(
                Route{"/lookup"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_lookup_blocklist_entry(req, res, std::move(send));
                    })
                    .summary("Hyperswitch lookup_blocklist_entry")
            )
            .add_route(
                Route{"/batch"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_upload_batch_blocklist(req, res, std::move(send));
                    })
                    .summary("Hyperswitch upload_batch_blocklist")
            )
            .add_route(
                Route{"/batch"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_batch_blocklist_jobs(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_batch_blocklist_jobs")
            )
            .add_route(
                Route{"/batch/:job_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_batch_blocklist_job_status(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_batch_blocklist_job_status")
            )
            .add_route(
                Route{"/export"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_blocklist_export(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_blocklist_export")
            )
            .add_route(
                Route{"/clone"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
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

    struct EntryWire
    {
        std::string kind{};
        std::string data{};
        std::string transaction_type{};
    };

    struct CsvRow
    {
        std::string kind{};
        std::string data{};
        std::string metadata{};
        std::string transaction_type{};
        bool valid{false};
    };

    struct CsvParse
    {
        std::vector<CsvRow> rows{};
        std::size_t total{0};
        std::size_t succeeded{0};
        std::size_t failed{0};
        std::string first_error{};
        std::optional<core::error::AppError> error{};
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

    [[nodiscard]] std::size_t number_from(std::string_view raw) const
    {
        std::size_t value{};
        const auto [end, ec] = std::from_chars(raw.data(), raw.data() + raw.size(), value);
        (void)end;
        return ec == std::errc{} ? value : 0;
    }

    [[nodiscard]] bool query_bool(const app::http::QueryParams& query, std::string_view key) const
    {
        const auto raw = query.get(key);
        return raw && (*raw == "true" || *raw == "1");
    }

    [[nodiscard]] std::string merchant_id_of(interfaces::io::IRequest& req) const
    {
        return app::http::merchant_id_from_header{}(req);
    }

    [[nodiscard]] std::string org_id_of(interfaces::io::IRequest& req) const
    {
        return app::http::org_id_from_header{}(req);
    }

    [[nodiscard]] std::string profile_id_of(interfaces::io::IRequest& req) const
    {
        const auto from_header = app::http::HeaderReader{}.get(req, "x-profile-id");
        if (!from_header.empty()) {
            return from_header;
        }
        const auto segments = app::http::RequestPath{}(req);
        for (std::size_t i = 0; i + 1 < segments.count(); ++i) {
            const auto current = segments.at(i).value();
            if (current == "profiles" || current == "business_profile") {
                return std::string{segments.at(i + 1).value()};
            }
        }
        if (const auto from_query = app::http::RequestQuery{}(req).get("profile_id")) {
            return *from_query;
        }
        return app::auth::AuthContextBuilder{}(req).profile_id();
    }

    [[nodiscard]] std::string resolve_profile(interfaces::io::IRequest& req) const
    {
        const auto explicit_id = profile_id_of(req);
        if (!explicit_id.empty()) {
            return explicit_id;
        }
        const auto merchant = merchant_id_of(req);
        if (merchant.empty()) {
            return {};
        }
        std::vector<::app::store::profiles::ProfileRecord> candidates;
        for (const auto& profile : m_context->repository().list_profiles()) {
            if (profile.get_merchant_id() == merchant) {
                candidates.push_back(profile);
            }
        }
        if (candidates.size() == 1) {
            return candidates.front().get_profile_id();
        }
        if (candidates.empty()) {
            return {};
        }
        for (const auto& profile : candidates) {
            const auto type = profile.get_type();
            if (type.find("default") != std::string::npos || type.find("DEFAULT") != std::string::npos) {
                return profile.get_profile_id();
            }
        }
        return {};
    }

    [[nodiscard]] std::optional<core::error::AppError> profile_error(
        interfaces::io::IRequest& req,
        std::string_view profile
    ) const
    {
        if (!profile.empty()) {
            return std::nullopt;
        }
        const auto merchant = merchant_id_of(req);
        if (merchant.empty()) {
            return std::nullopt;
        }
        std::size_t count{0};
        for (const auto& candidate : m_context->repository().list_profiles()) {
            if (candidate.get_merchant_id() == merchant) {
                ++count;
            }
        }
        if (count > 1) {
            return error(core::error::AppErrorKind::PRECONDITION_FAILED, "x-profile-id is required for a multi-profile merchant");
        }
        return std::nullopt;
    }

    [[nodiscard]] bool profile_is_owned(std::string_view profile_id, std::string_view merchant) const
    {
        if (profile_id.empty() || merchant.empty()) {
            return false;
        }
        const auto found = m_context->repository().find_profile_by_string(profile_id);
        return found.has_value() && found->get_merchant_id() == merchant;
    }

    [[nodiscard]] std::optional<core::error::AppError> merchant_error(std::string_view merchant) const
    {
        if (merchant.empty()) {
            return error(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant id");
        }
        return std::nullopt;
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

    [[nodiscard]] std::string lowercase(std::string_view raw) const
    {
        std::string text{raw};
        std::transform(
            text.begin(),
            text.end(),
            text.begin(),
            [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); }
        );
        return text;
    }

    [[nodiscard]] bool known_kind(std::string_view kind) const
    {
        return kind == "fingerprint" || kind == "card_bin" ||
               kind == "extended_card_bin" || kind == "generic_card_bin";
    }

    [[nodiscard]] std::string normalize_data_kind(std::string_view raw, bool allow_empty = false) const
    {
        const auto kind = lowercase(raw);
        if (kind.empty() && allow_empty) {
            return {};
        }
        return known_kind(kind) ? kind : std::string{"fingerprint"};
    }

    [[nodiscard]] std::optional<core::error::AppError> validate_filter_kind(std::string_view kind) const
    {
        if (kind.empty() || known_kind(kind)) {
            return std::nullopt;
        }
        return error(core::error::AppErrorKind::VALIDATION_FAILED, "unknown blocklist data kind");
    }

    [[nodiscard]] std::string normalize_transaction_type(std::string_view raw) const
    {
        const auto value = lowercase(raw);
        return (value == "payment" || value == "payout") ? value : std::string{"payment"};
    }

    [[nodiscard]] bool valid_transaction_type(std::string_view raw) const
    {
        return raw.empty() || raw == "payment" || raw == "payout";
    }

    [[nodiscard]] EntryWire entry_from_body(std::string_view body) const
    {
        EntryWire wire;
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!generic) {
            return wire;
        }
        const auto* root = std::get_if<interfaces::Value::Object>(&generic->get());
        if (root == nullptr) {
            return wire;
        }
        if (const auto field = root->get("type")) {
            if (const auto* text = std::get_if<std::string>(&field->get())) {
                wire.kind = normalize_data_kind(*text);
            }
        }
        if (const auto field = root->get("entry")) {
            if (const auto* text = std::get_if<std::string>(&field->get())) {
                wire.kind = normalize_data_kind(*text);
            } else if (const auto* object = std::get_if<interfaces::Value::Object>(&field->get())) {
                if (const auto nested = object->get("data")) {
                    if (const auto* text = std::get_if<std::string>(&nested->get())) {
                        wire.data = *text;
                    }
                }
                if (const auto nested = object->get("card_number")) {
                    if (const auto* text = std::get_if<std::string>(&nested->get())) {
                        wire.data = *text;
                    }
                }
            }
        }
        if (const auto field = root->get("data")) {
            if (const auto* text = std::get_if<std::string>(&field->get())) {
                wire.data = *text;
            } else if (const auto* object = std::get_if<interfaces::Value::Object>(&field->get())) {
                if (const auto nested = object->get("card_number")) {
                    if (const auto* text = std::get_if<std::string>(&nested->get())) {
                        wire.data = *text;
                    }
                }
            }
        }
        if (const auto field = root->get("card_number")) {
            if (const auto* text = std::get_if<std::string>(&field->get())) {
                wire.data = *text;
            }
        }
        if (const auto field = root->get("transaction_type")) {
            if (const auto* text = std::get_if<std::string>(&field->get())) {
                wire.transaction_type = lowercase(*text);
            } else if (const auto* object = std::get_if<interfaces::Value::Object>(&field->get())) {
                if (const auto nested = object->get("value")) {
                    if (const auto* text = std::get_if<std::string>(&nested->get())) {
                        wire.transaction_type = lowercase(*text);
                    }
                }
            }
        }
        if (wire.kind.empty()) {
            wire.kind = "fingerprint";
        }
        return wire;
    }

    [[nodiscard]] std::optional<core::error::AppError> validate_entry(const EntryWire& wire) const
    {
        if (!known_kind(wire.kind)) {
            return error(core::error::AppErrorKind::VALIDATION_FAILED, "unknown blocklist data kind");
        }
        if (!valid_transaction_type(wire.transaction_type)) {
            return error(core::error::AppErrorKind::VALIDATION_FAILED, "transaction type must be payment or payout");
        }
        if (wire.data.empty()) {
            return error(core::error::AppErrorKind::INVALID_REQUEST, "missing blocklist data");
        }
        const auto is_digits = std::all_of(
            wire.data.begin(),
            wire.data.end(),
            [](unsigned char ch) { return std::isdigit(ch) != 0; }
        );
        if (wire.kind == "fingerprint") {
            if (wire.data.size() > 20) {
                return error(core::error::AppErrorKind::VALIDATION_FAILED, "fingerprint data exceeds 20 characters");
            }
        } else if (!is_digits) {
            return error(core::error::AppErrorKind::VALIDATION_FAILED, "card bin data must be digits");
        } else if (wire.kind == "card_bin" && wire.data.size() != 6) {
            return error(core::error::AppErrorKind::VALIDATION_FAILED, "card_bin requires 6 digits");
        } else if (wire.kind == "extended_card_bin" && wire.data.size() != 8) {
            return error(core::error::AppErrorKind::VALIDATION_FAILED, "extended_card_bin requires 8 digits");
        } else if (wire.kind == "generic_card_bin" && (wire.data.size() < 6 || wire.data.size() > 10)) {
            return error(core::error::AppErrorKind::VALIDATION_FAILED, "generic_card_bin requires 6 to 10 digits");
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string fingerprint_for(
        std::string_view kind,
        std::string_view data,
        std::string_view merchant
    ) const
    {
        app::model::blocklist::GenerateFingerprintRequest request;
        request.set_key("blocklist.fingerprint:" + std::string{merchant});
        request.set_data(std::string{kind} + ":" + std::string{data});
        const app::crypto::SecretString secret{request.get_data()};
        const auto digest = app::crypto::KeyedHasher{request.get_key()}.hash(secret.value());
        return "fpg_" + digest;
    }

    [[nodiscard]] std::unordered_set<std::string> candidate_fingerprints(
        std::string_view data,
        std::string_view merchant
    ) const
    {
        return {
            fingerprint_for("fingerprint", data, merchant),
            fingerprint_for("card_bin", data, merchant),
            fingerprint_for("extended_card_bin", data, merchant),
            fingerprint_for("generic_card_bin", data, merchant),
        };
    }

    [[nodiscard]] std::string legacy_transaction_type(const EntryRecord& record) const
    {
        return record.get_transaction_type().empty() ? std::string{"payment"} : record.get_transaction_type();
    }

    [[nodiscard]] std::string now_stamp(std::int64_t offset_seconds = 0) const
    {
        const auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        return std::to_string(now + offset_seconds);
    }

    [[nodiscard]] std::string entry_key(
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

    [[nodiscard]] std::string guard_key(std::string_view merchant, std::string_view profile) const
    {
        return profile.empty()
                   ? "blocklist.guard:" + std::string{merchant}
                   : "blocklist.guard:" + std::string{merchant} + ":" + std::string{profile};
    }

    [[nodiscard]] EntryRecord make_entry(
        std::string_view key,
        std::string_view merchant,
        std::string_view profile,
        std::string_view kind,
        std::string_view transaction_type,
        std::string_view fingerprint,
        std::size_t data_length,
        std::string_view created
    ) const
    {
        EntryRecord record{};
        record.set_entry_id(std::string{key});
        record.set_merchant_id(std::string{merchant});
        record.set_profile_id(std::string{profile});
        record.set_data_kind(std::string{kind});
        record.set_transaction_type(std::string{transaction_type});
        record.set_fingerprint(std::string{fingerprint});
        // The store record has no dedicated length column; `status` carries the
        // BIN/data length used by count-by-length, a documented local substitute.
        record.set_status(std::to_string(data_length));
        record.set_created_at(std::string{created});
        return record;
    }

    [[nodiscard]] std::optional<EntryRecord> find_entry(std::string_view key) const
    {
        return m_context->repository().find_blocklist_entry_by_string(key);
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

    bool remove_entry(std::string_view key) const
    {
        const auto existing = find_entry(key);
        if (!existing) {
            return false;
        }
        return m_context->repository().remove_blocklist_entry(existing->get_id());
    }

    [[nodiscard]] std::vector<EntryRecord> list_entries() const
    {
        return m_context->repository().list_blocklist_entries();
    }

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

    [[nodiscard]] std::string value_of(
        const std::optional<app::model::blocklist::BlocklistTransactionTypeValue>& value
    ) const
    {
        return value ? value->get_value() : std::string{};
    }

    [[nodiscard]] std::string value_of(
        const std::optional<app::model::blocklist::BatchBlocklistJobTypeValue>& value
    ) const
    {
        return value ? value->get_value() : std::string{};
    }

    [[nodiscard]] bool matches_scope(
        const EntryRecord& record,
        std::string_view merchant,
        std::string_view profile,
        std::string_view data_kind,
        std::string_view transaction_type
    ) const
    {
        if (!merchant.empty() && record.get_merchant_id() != merchant) {
            return false;
        }
        if (!profile.empty() && !record.get_profile_id().empty() && record.get_profile_id() != profile) {
            return false;
        }
        // `generic_card_bin` coverage includes the deprecated card-bin kinds.
        const auto kind = record.get_data_kind();
        if (!data_kind.empty()) {
            if (data_kind == "generic_card_bin") {
                if (kind != "generic_card_bin" && kind != "card_bin" && kind != "extended_card_bin") {
                    return false;
                }
            } else if (kind != data_kind) {
                return false;
            }
        }
        // Legacy NULL transaction namespaces belong to `payment`.
        if (!transaction_type.empty() && legacy_transaction_type(record) != transaction_type) {
            return false;
        }
        return true;
    }

    [[nodiscard]] app::model::blocklist::BlocklistResponse response_of(const EntryRecord& record) const
    {
        app::model::blocklist::BlocklistResponse response;
        response.set_fingerprint_id(record.get_fingerprint());
        response.set_data_kind(wrap<app::model::blocklist::BlocklistDataKindValue>(record.get_data_kind()));
        response.set_transaction_type(wrap<app::model::blocklist::BlocklistTransactionTypeValue>(legacy_transaction_type(record)));
        response.set_created_at(wrap<app::model::blocklist::PrimitiveDateTimeValue>(record.get_created_at()));
        if (!record.get_profile_id().empty()) {
            response.set_profile_id(wrap<app::model::blocklist::ProfileIdValue>(record.get_profile_id()));
        }
        return response;
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
        job.set_created_at(wrap<app::model::blocklist::PrimitiveDateTimeValue>(now_stamp()));
        job.set_updated_at(wrap<app::model::blocklist::PrimitiveDateTimeValue>(now_stamp()));
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

        auto record = config_record(
            job_key(job_id),
            merchant,
            profile,
            job_type,
            status,
            "tt=" + std::string{transaction_type},
            serde::Ser::encode_json(job)
        );
        record.set_created_at(now_stamp());
        upsert_job_record(std::move(record));
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
            job_id,
            merchant,
            profile,
            rows
        );
        m_context->process_trackers().schedule(std::string{name}, payload, "0");
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
        send_json_status(
            res,
            send,
            text,
            ok ? interfaces::io::types::Status::OK : interfaces::io::types::Status::BAD_REQUEST
        );
    }

    void send_json_status(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        interfaces::io::types::Status status
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(status);
        res.set_body(std::move(bytes));
        send();
    }

    [[nodiscard]] std::string_view csv_payload(
        interfaces::io::IRequest& req,
        std::string_view body
    ) const
    {
        const auto content_type = app::http::HeaderReader{}.get(req, "content-type");
        if (!content_type.starts_with("multipart/form-data")) {
            return body;
        }
        const auto marker = content_type.find("boundary=");
        if (marker == std::string::npos) {
            return body;
        }
        auto boundary = content_type.substr(marker + 9);
        if (!boundary.empty() && boundary.front() == '"') {
            boundary = boundary.substr(1);
            if (!boundary.empty() && boundary.back() == '"') {
                boundary = boundary.substr(0, boundary.size() - 1);
            }
        }
        const auto delimiter = std::string{"--"} + std::string{boundary};
        const auto header_end = body.find("\r\n\r\n");
        if (header_end == std::string_view::npos) {
            return body;
        }
        const auto start = header_end + 4;
        const auto tail = body.find(delimiter, start);
        return tail == std::string_view::npos ? body.substr(start) : body.substr(start, tail - start);
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
            parsed.error = error(core::error::AppErrorKind::VALIDATION_FAILED, "empty CSV");
            return parsed;
        }
        const auto header = lines.front();
        for (const auto* required : {"type", "data", "metadata", "transaction_type"}) {
            if (header.find(required) == std::string_view::npos) {
                parsed.error = error(core::error::AppErrorKind::VALIDATION_FAILED, "invalid CSV header");
                return parsed;
            }
        }
        if (lines.size() - 1 > 100000) {
            parsed.error = error(core::error::AppErrorKind::VALIDATION_FAILED, "batch exceeds 100000 rows");
            return parsed;
        }
        for (std::size_t i = 1; i < lines.size(); ++i) {
            if (lines[i].empty()) {
                continue;
            }
            CsvRow row;
            const auto parts = split_csv_line(lines[i]);
            row.kind = parts.size() > 0 ? normalize_data_kind(parts[0]) : std::string{"fingerprint"};
            row.data = parts.size() > 1 ? std::string{parts[1]} : std::string{};
            row.metadata = parts.size() > 2 ? std::string{parts[2]} : std::string{};
            row.transaction_type = !override_type.empty()
                                       ? std::string{override_type}
                                       : normalize_transaction_type(parts.size() > 3 ? parts[3] : "");
            EntryWire wire{row.kind, row.data, row.transaction_type};
            row.valid = !validate_entry(wire).has_value();
            if (!row.valid && parsed.first_error.empty()) {
                parsed.first_error = std::format("row {}: invalid blocklist entry", i - 1);
            }
            parsed.rows.push_back(std::move(row));
            ++parsed.total;
            if (row.valid) {
                ++parsed.succeeded;
            } else {
                ++parsed.failed;
            }
        }
        return parsed;
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

    [[nodiscard]] std::string export_csv(
        std::string_view merchant,
        std::string_view profile,
        std::string_view transaction_type
    ) const
    {
        std::string csv{"type,data,metadata,transaction_type\n"};
        for (const auto& record : list_entries()) {
            if (!merchant.empty() && record.get_merchant_id() != merchant) {
                continue;
            }
            if (!profile.empty() && !record.get_profile_id().empty() && record.get_profile_id() != profile) {
                continue;
            }
            if (!transaction_type.empty() && legacy_transaction_type(record) != transaction_type) {
                continue;
            }
            csv += std::format(
                "{},{},,\"{}\"\n",
                record.get_data_kind(),
                record.get_fingerprint(),
                legacy_transaction_type(record)
            );
        }
        return csv;
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::blocklist
