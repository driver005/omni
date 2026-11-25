export module app_handlers_blocklist:runtime_common;

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
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :input;
import :result;

export namespace app::handlers::blocklist {

/**
 * @brief Shared business helpers for the blocklist sub-runtimes.
 *
 * Storage, filtering, job persistence, response mapping, time and event/metric
 * emission. Never touches HTTP request/response types.
 */
class BlocklistRuntimeSupport
{
public:
    explicit BlocklistRuntimeSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] core::error::AppError
    error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    template<typename T>
    [[nodiscard]] T wrap(std::string value) const
    {
        T result{};
        result.set_value(std::move(value));
        return result;
    }

    // ---- storage / filtering ------------------------------------------------

    [[nodiscard]] std::string identity_key(
        std::string_view merchant,
        std::string_view profile,
        std::string_view fingerprint,
        std::string_view transaction_type
    ) const
    {
        return std::format(
            "blocklist:{}:{}:{}:{}",
            merchant,
            profile,
            fingerprint,
            transaction_type
        );
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

    // Legacy NULL profile rows are merchant-wide: profile_id = p OR profile_id IS NULL.
    [[nodiscard]] bool matches_profile(const EntryRecord& record, std::string_view profile) const
    {
        return profile.empty() || record.get_profile_id().empty() ||
               record.get_profile_id() == profile;
    }

    // Legacy NULL transaction namespaces are payment, never payout.
    [[nodiscard]] bool
    tt_matches(const EntryRecord& record, std::string_view transaction_type) const
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
        return m_context.repository().find_blocklist_entry_by_string(key);
    }

    [[nodiscard]] std::optional<EntryRecord> find_by_identity(
        std::string_view merchant,
        std::string_view profile,
        std::string_view fingerprint,
        std::string_view transaction_type
    ) const
    {
        for (const auto& record: list_entries()) {
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
            m_context.repository().update_blocklist_entry(record);
        } else {
            m_context.repository().save_blocklist_entry(std::move(record));
        }
    }

    bool remove_entry(std::int64_t id) const
    {
        return m_context.repository().remove_blocklist_entry(id);
    }

    [[nodiscard]] std::vector<EntryRecord> list_entries() const
    {
        return m_context.repository().list_blocklist_entries();
    }

    void sort_by_created_desc(std::vector<EntryRecord>& records) const
    {
        std::sort(
            records.begin(),
            records.end(),
            [](const EntryRecord& lhs, const EntryRecord& rhs)
            {
                return lhs.get_created_at() > rhs.get_created_at();
            }
        );
    }

    [[nodiscard]] app::model::blocklist::BlocklistResponse
    response_of(const EntryRecord& record) const
    {
        app::model::blocklist::BlocklistResponse response;
        response.set_fingerprint_id(record.get_fingerprint());
        response.set_data_kind(
            wrap<app::model::blocklist::BlocklistDataKindValue>(record.get_data_kind())
        );
        response.set_transaction_type(
            wrap<app::model::blocklist::BlocklistTransactionTypeValue>(
                legacy_transaction_type(record)
            )
        );
        response.set_created_at(
            wrap<app::model::blocklist::PrimitiveDateTimeValue>(record.get_created_at())
        );
        if (!record.get_profile_id().empty()) {
            response.set_profile_id(
                wrap<app::model::blocklist::ProfileIdValue>(record.get_profile_id())
            );
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

    void upsert_job_record(JobRecord record) const
    {
        auto repository = m_context.repository();
        if (const auto existing =
                repository.find_routing_config_by_string(record.get_config_id())) {
            record.set_id(existing->get_id());
            repository.update_routing_config(record);
        } else {
            repository.save_routing_config(std::move(record));
        }
    }

    [[nodiscard]] std::vector<JobRecord> list_jobs(std::string_view prefix) const
    {
        std::vector<JobRecord> selected;
        for (auto& record: m_context.repository().list_routing_configs()) {
            if (record.get_config_id().starts_with(prefix)) {
                selected.push_back(std::move(record));
            }
        }
        return selected;
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
        job.set_job_type(
            wrap<app::model::blocklist::BatchBlocklistJobTypeValue>(std::string{job_type})
        );
        job.set_status(
            wrap<app::model::blocklist::BatchBlocklistJobStatusValue>(std::string{status})
        );
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
            job.set_expires_at(
                wrap<app::model::blocklist::PrimitiveDateTimeValue>(std::string{expires_at})
            );
        }
        if (!download_url_expires_at.empty()) {
            job.set_download_url_expires_at(
                wrap<app::model::blocklist::PrimitiveDateTimeValue>(
                    std::string{download_url_expires_at}
                )
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
        record.set_created_at(now_iso8601());
        upsert_job_record(std::move(record));
    }

    [[nodiscard]] app::model::blocklist::BatchBlocklistJobStatusResponse
    job_status_of(const JobRecord& stored) const
    {
        app::model::blocklist::BatchBlocklistJobStatusResponse response;
        if (auto parsed =
                serde::Ser::deserialize<app::model::blocklist::BatchBlocklistJobStatusResponse>(
                    "application/json",
                    stored.get_config_json()
                )) {
            response = std::move(*parsed);
        } else {
            response.set_job_id(
                stored.get_config_id().substr(std::string{"blocklist.job:"}.size())
            );
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
    [[nodiscard]] bool
    is_export_downloadable(const app::model::blocklist::BatchBlocklistJobStatusResponse& job) const
    {
        return job.get_job_type().get_value() == "export" &&
               job.get_status().get_value() == "completed" && job.get_file_name().has_value() &&
               job.get_downloadable();
    }

    [[nodiscard]] std::string
    signed_download_url(std::string_view file_name, std::string_view expires) const
    {
        // In-process substitute for S3 presigning (export.rs:177-213).
        const auto token = app::crypto::JwtSigner{"blocklist.export"}.sign(
            std::string{file_name} + ":" + std::string{expires}
        );
        return std::format(
            "http://localhost/blocklist/export/{}?token={}&expires={}",
            file_name,
            token,
            expires
        );
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
        m_context.process_trackers().schedule(std::string{name}, payload, "0");
    }

    // ---- export CSV --------------------------------------------------------------

    [[nodiscard]] std::string type_from_kind(std::string_view kind) const
    {
        return kind == "payment_method" ? std::string{"fingerprint"} : std::string{kind};
    }

    // batch.rs:45-46 CSV_HEADER + export.rs:64-70 rows_to_csv_bytes.
    [[nodiscard]] std::string export_csv(
        std::string_view merchant,
        std::string_view profile,
        std::string_view transaction_type
    ) const
    {
        std::string csv{"type,data,metadata,transaction_type\n"};
        for (const auto& record: list_entries()) {
            if (!matches_scope(record, merchant, profile, "payment_method", {}) &&
                !matches_scope(record, merchant, profile, "card_bin", {}) &&
                !matches_scope(record, merchant, profile, "extended_card_bin", {}) &&
                !matches_scope(record, merchant, profile, "generic_card_bin", {})) {
                continue;
            }
            if (!transaction_type.empty() && !tt_matches(record, transaction_type)) {
                continue;
            }
            csv += std::format(
                "{},{},,\"{}\"\n",
                type_from_kind(record.get_data_kind()),
                record.get_fingerprint(),
                legacy_transaction_type(record)
            );
        }
        return csv;
    }

    // ---- time / value / response helpers -----------------------------------------

    // common_utils/custom_serde.rs:16-18: YYYY-MM-DDTHH:MM:SS.mmmZ.
    [[nodiscard]] std::string now_iso8601(std::int64_t offset_seconds = 0) const
    {
        const auto now = std::chrono::system_clock::now() + std::chrono::seconds{offset_seconds};
        const auto millis =
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        const auto seconds = millis / 1'000;
        const auto days = seconds / 86'400;
        const auto sod = seconds % 86'400;
        const auto [year, month, day] = civil_from_days(days);
        return std::format(
            "{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}.{:03d}Z",
            year,
            month,
            day,
            sod / 3'600,
            (sod % 3'600) / 60,
            sod % 60,
            static_cast<int>(millis % 1'000)
        );
    }

    [[nodiscard]] std::tuple<int, int, int> civil_from_days(std::int64_t z) const
    {
        z += 719'468;
        const auto era = (z >= 0 ? z : z - 146'096) / 146'097;
        const auto doe = static_cast<unsigned long>(z - era * 146'097);
        const auto yoe = (doe - doe / 1'460 + doe / 36'524 - doe / 146'096) / 365;
        auto year = static_cast<int>(yoe) + static_cast<int>(era) * 400;
        const auto doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        const auto mp = (5 * doy + 2) / 153;
        const auto day = static_cast<int>(doy - (153 * mp + 2) / 5 + 1);
        const auto month = static_cast<int>(mp + (mp < 10 ? 3 : -9));
        year += (month <= 2) ? 1 : 0;
        return {year, month, day};
    }

    [[nodiscard]] std::string
    value_of(const std::optional<app::model::blocklist::BlocklistTransactionTypeValue>& value) const
    {
        return value ? value->get_value() : std::string{};
    }

    template<typename T>
    [[nodiscard]] std::vector<T>
    paginate(std::vector<T> items, std::size_t offset, std::size_t limit) const
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
        m_context.events().publish(std::string{topic}, serde::Ser::encode_json(payload));
    }

    void track(std::string_view flow) const
    {
        const auto metric = std::string{"blocklist."} + std::string{flow};
        m_context.metrics().counter(metric).increment();
        m_context.metrics().record_flow(metric);
    }

private:
    app::context::AppContext& m_context;
};

} // namespace app::handlers::blocklist
