export module app_routing:blocklist_runtime_clone_blocklist_entries;

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

import :blocklist_input;
import :blocklist_result;
import :blocklist_runtime_common;

export namespace app::handlers::blocklist {

class CloneBlocklistEntriesRuntime
{
public:
    explicit CloneBlocklistEntriesRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // clone.rs:68-109, 114-225 clone_blocklist_entries.
    [[nodiscard]] std::
        expected<::app::model::blocklist::CloneBlocklistEntriesResponse, core::error::AppError>
        clone_blocklist_entries(const CloneBlocklistEntriesInput& input) const
    {
        const auto job_id = m_context.ids().payment_id("blkclone");
        const auto stamp = m_support.now_iso8601();
        // clone.rs:126-136: the enqueued job starts with Initiated/0 per target.
        ::app::model::blocklist::ProfileCloneJobMetadata init_metadata;
        std::vector<::app::model::blocklist::ProfileCloneTargetMetadata> init_targets;
        init_targets.reserve(input.targets.size());
        for (const auto& target: input.targets) {
            ::app::model::blocklist::ProfileCloneTargetMetadata item;
            item.set_profile_id(target);
            item.set_status(
                m_support.wrap<::app::model::blocklist::BatchBlocklistJobStatusValue>("initiated")
            );
            item.set_processed_rows(0);
            init_targets.push_back(std::move(item));
        }
        init_metadata.set_targets(std::move(init_targets));
        m_support.upsert_job(
            job_id,
            input.merchant,
            input.source,
            "profile_clone",
            "initiated",
            0,
            input.transaction_type,
            {},
            {},
            false,
            std::move(init_metadata),
            0,
            0,
            {}
        );

        ::app::model::blocklist::ProfileCloneJobMetadata metadata;
        std::vector<::app::model::blocklist::ProfileCloneTargetMetadata> clone_targets;
        clone_targets.reserve(input.targets.size());
        std::size_t cloned{0};
        for (const auto& target: input.targets) {
            const auto target_id = target.get_value();
            std::uint32_t copied{0};
            // Worker substitute: clone.rs:230-246 to_target_entry (one target at a time).
            for (const auto& record: m_support.list_entries()) {
                if (!m_support.matches_merchant(record, input.merchant) ||
                    record.get_profile_id() != input.source) {
                    continue;
                }
                const auto key = m_support.identity_key(
                    input.merchant,
                    target_id,
                    record.get_fingerprint(),
                    m_support.legacy_transaction_type(record)
                );
                if (!m_support.find_entry(key)) {
                    m_support.save_entry(m_support.make_entry(
                        record.get_fingerprint(),
                        input.merchant,
                        target_id,
                        record.get_data_kind(),
                        m_support.legacy_transaction_type(record),
                        stamp
                    ));
                }
                ++copied;
            }
            cloned += copied;
            ::app::model::blocklist::ProfileCloneTargetMetadata item;
            item.set_profile_id(target);
            item.set_status(
                m_support.wrap<::app::model::blocklist::BatchBlocklistJobStatusValue>("completed")
            );
            item.set_processed_rows(copied);
            clone_targets.push_back(std::move(item));
        }
        metadata.set_targets(std::move(clone_targets));

        m_support.upsert_job(
            job_id,
            input.merchant,
            input.source,
            "profile_clone",
            "completed",
            0,
            input.transaction_type,
            {},
            {},
            false,
            std::move(metadata),
            static_cast<std::uint32_t>(cloned),
            0,
            {}
        );
        m_support
            .schedule_job("BLOCKLIST_PROFILE_CLONE", job_id, input.merchant, input.source, cloned);

        ::app::model::blocklist::CloneBlocklistEntriesResponse response;
        response.set_source_profile_id(
            m_support.wrap<::app::model::blocklist::ProfileIdValue>(input.source)
        );
        response.set_job_id(job_id);
        response.set_status(
            m_support.wrap<::app::model::blocklist::BatchBlocklistJobStatusValue>("initiated")
        );
        response.set_target_profile_ids(input.targets);
        m_support.emit("blocklist.clone", response);
        m_support.track("clone_blocklist_entries");
        return response;
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistRuntimeSupport m_support;
};

} // namespace app::handlers::blocklist
