export module app_payment:disputes_runtime_common;

import std;
import interfaces;
import core_router;
import core_serde;
import core_cache;
import core_database;
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

import :disputes_input;
import :disputes_result;

export namespace app::handlers::disputes {

/** @brief Shared business helpers for the disputes route sub-runtimes. */
class DisputesRuntimeSupport
{
public:
    explicit DisputesRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_input{context}
    {
    }

    [[nodiscard]] std::optional<Record> find_dispute_by_processor_merchant_id_dispute_id(
        const DisputeScope& scope,
        std::string_view dispute_id
    ) const
    {
        core::db::QuerySpec spec = m_context.database().template query<Record>().spec();
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "dispute_id",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{dispute_id}}
            }
        );
        if (!scope.merchant_id.empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "merchant_id",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{scope.merchant_id}
                }
            );
        }
        spec.limit_count = 1;
        const auto rows = m_context.database().template collect<Record>(spec);
        if (rows.empty()) {
            return std::nullopt;
        }
        return rows.front();
    }

    [[nodiscard]] Error dispute_not_found(std::string_view dispute_id) const
    {
        m_context.metrics().counter("disputes_not_found").increment();
        return core::error::AppErrorBuilder{}(
            Kind::NOT_FOUND,
            "Dispute does not exist in our records"
        );
    }

    /** @brief core/utils.rs:2889-2912 `validate_profile_id_from_auth_layer`. */
    [[nodiscard]] std::optional<Error>
    validate_profile_id_from_auth_layer(const DisputeScope& scope, const Record& record) const
    {
        if (scope.profile_id.empty()) {
            return std::nullopt;
        }
        if (record.get_profile_id().empty()) {
            return core::error::AppErrorBuilder{}(
                Kind::PRECONDITION_FAILED,
                "Couldn't find profile_id in record for authentication"
            );
        }
        if (scope.profile_id != record.get_profile_id()) {
            return core::error::AppErrorBuilder{}(
                Kind::PRECONDITION_FAILED,
                "Profile id authentication failed. Please use the correct JWT token"
            );
        }
        return std::nullopt;
    }

    // ── core/disputes.rs:38-49  should_call_connector_for_dispute_sync ─────────

    [[nodiscard]] bool
    should_call_connector_for_dispute_sync(bool force_sync, std::string_view status) const
    {
        return force_sync && (status == "dispute_accepted" || status == "dispute_challenged" ||
                              status == "dispute_opened");
    }

    /** @brief core/utils.rs:3176-3190 `should_proceed_with_accept_dispute`. */
    [[nodiscard]] bool
    should_proceed_with_accept_dispute(std::string_view stage, std::string_view status) const
    {
        const bool stage_ok = stage == "pre_dispute" || stage == "dispute" ||
                              stage == "pre_arbitration" || stage == "arbitration";
        return stage_ok && (status == "dispute_challenged" || status == "dispute_opened");
    }

    /** @brief core/utils.rs:3160-3174 `should_proceed_with_submit_evidence`. */
    [[nodiscard]] bool
    should_proceed_with_submit_evidence(std::string_view stage, std::string_view status) const
    {
        const bool stage_ok = stage == "pre_dispute" || stage == "dispute" ||
                              stage == "pre_arbitration" || stage == "arbitration";
        return stage_ok && (status == "dispute_opened" || status == "dispute_challenged");
    }

    [[nodiscard]] Error status_validation_failed(std::string_view reason) const
    {
        m_context.metrics().counter("disputes_status_validation_failed").increment();
        return core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, std::string{reason});
    }

    [[nodiscard]] bool
    is_already_refunded(const ::app::store::AppRepository& db, const Record& dispute) const
    {
        // Local mirror of `validate_amount_against_intent_state_metadata`: disputed
        // amount (dispute_lost statuses only) + refunded amount > captured amount.
        const auto intent = db.find_payment_by_string(dispute.get_payment_id());
        if (!intent) {
            return false;
        }
        std::int64_t captured = intent->get_amount();
        std::int64_t refunded = 0;
        for (const auto& refund: db.find_refunds_by_payment_id(dispute.get_payment_id())) {
            refunded += refund.get_amount();
        }
        if (m_input.canonical_status(dispute.get_status()) == "dispute_lost") {
            refunded += dispute.get_amount();
        }
        return refunded > captured;
    }

    [[nodiscard]] std::optional<Error> validate_profile_id_from_auth_mca(
        const DisputeScope& scope,
        const ::app::store::merchant_connectors::MerchantConnectorRecord& mca
    ) const
    {
        if (scope.profile_id.empty() || mca.get_profile_id().empty()) {
            return std::nullopt;
        }
        if (scope.profile_id != mca.get_profile_id()) {
            return core::error::AppErrorBuilder{}(
                Kind::PRECONDITION_FAILED,
                "Profile id authentication failed. Please use the correct JWT token"
            );
        }
        return std::nullopt;
    }

    /** @brief core/disputes/transformers.rs:16-159 `get_evidence_request_data`. */
    [[nodiscard]] std::string get_evidence_request_data(
        const ::app::model::disputes::SubmitEvidenceRequest& req,
        const Record& dispute
    ) const
    {
        // File ids are resolved through the local file store (local substitute for
        // retrieve_file_and_provider_file_id_from_file_id) and carried in the raw
        // connector payload together with the upstream dispute fields.
        const auto field =
            [](std::string_view key, const std::optional<std::string>& value, std::string& out)
        {
            if (value.has_value()) {
                if (!out.empty()) {
                    out.push_back(',');
                }
                out += std::format("\"{}\":\"{}\"", key, *value);
            }
        };
        std::string out = std::format(
            "\"dispute_id\":\"{}\",\"dispute_status\":\"{}\",\"connector_dispute_id\":\"{}\"",
            dispute.get_dispute_id(),
            m_input.canonical_status(dispute.get_status()),
            connector_dispute_id_of(dispute.get_dispute_id())
        );
        field("access_activity_log", req.get_access_activity_log(), out);
        field("billing_address", req.get_billing_address(), out);
        field("cancellation_policy", req.get_cancellation_policy(), out);
        field("cancellation_policy_disclosure", req.get_cancellation_policy_disclosure(), out);
        field("cancellation_rebuttal", req.get_cancellation_rebuttal(), out);
        field("customer_communication", req.get_customer_communication(), out);
        field("customer_email_address", req.get_customer_email_address(), out);
        field("customer_name", req.get_customer_name(), out);
        field("customer_purchase_ip", req.get_customer_purchase_ip(), out);
        field("customer_signature", req.get_customer_signature(), out);
        field("product_description", req.get_product_description(), out);
        field("receipt", req.get_receipt(), out);
        field("refund_policy", req.get_refund_policy(), out);
        field("refund_policy_disclosure", req.get_refund_policy_disclosure(), out);
        field("refund_refusal_explanation", req.get_refund_refusal_explanation(), out);
        field("service_date", req.get_service_date(), out);
        field("service_documentation", req.get_service_documentation(), out);
        field("shipping_address", req.get_shipping_address(), out);
        field("shipping_carrier", req.get_shipping_carrier(), out);
        field("shipping_date", req.get_shipping_date(), out);
        field("shipping_documentation", req.get_shipping_documentation(), out);
        field("shipping_tracking_number", req.get_shipping_tracking_number(), out);
        field(
            "invoice_showing_distinct_transactions",
            req.get_invoice_showing_distinct_transactions(),
            out
        );
        field("recurring_transaction_agreement", req.get_recurring_transaction_agreement(), out);
        field("uncategorized_file", req.get_uncategorized_file(), out);
        field("uncategorized_text", req.get_uncategorized_text(), out);
        return "{" + out + "}";
    }

    [[nodiscard]] bool requires_defend_dispute(std::string_view connector) const
    {
        // Upstream `connector.requires_defend_dispute()` — Adyen needs a separate
        // defend call; local connectors model this via config_store flag.
        return m_context.config_store()
                   .get_value("connector_requires_defend", connector)
                   .value_or("false") == "true";
    }

    // ── core/disputes/transformers.rs:162-398  evidence transforms ─────────────

    [[nodiscard]] std::vector<::app::model::disputes::DisputeEvidenceBlock> update_dispute_evidence(
        std::vector<::app::model::disputes::DisputeEvidenceBlock> evidence,
        ::app::model::disputes::EvidenceType type,
        std::string_view file_id
    ) const
    {
        for (auto& block: evidence) {
            if (block.get_evidence_type() == type) {
                auto metadata = block.get_file_metadata_response();
                metadata.set_file_id(std::string{file_id});
                metadata.set_available(true);
                block.set_file_metadata_response(metadata);
                return evidence;
            }
        }
        ::app::model::disputes::DisputeEvidenceBlock block{};
        block.set_evidence_type(type);
        ::app::model::disputes::FileMetadataResponseValue metadata{};
        metadata.set_file_id(std::string{file_id});
        metadata.set_available(true);
        block.set_file_metadata_response(metadata);
        evidence.push_back(block);
        return evidence;
    }

    [[nodiscard]] std::vector<::app::model::disputes::DisputeEvidenceBlock> delete_evidence_file(
        std::vector<::app::model::disputes::DisputeEvidenceBlock> evidence,
        ::app::model::disputes::EvidenceType type
    ) const
    {
        std::vector<::app::model::disputes::DisputeEvidenceBlock> kept{};
        for (const auto& block: evidence) {
            if (block.get_evidence_type() != type) {
                kept.push_back(block);
            }
        }
        return kept;
    }

    /** @brief transformers.rs:211-232 + 282-398 `get_dispute_evidence_vec`. */
    [[nodiscard]] std::vector<::app::model::disputes::DisputeEvidenceBlock>
    get_dispute_evidence_vec(std::string_view dispute_id) const
    {
        std::vector<::app::model::disputes::DisputeEvidenceBlock> result{};
        const auto stored = m_context.config_store().get_value("dispute_evidence", dispute_id);
        if (!stored || stored->empty()) {
            return result;
        }
        for (const auto& part: split(*stored, ';')) {
            const auto fields = split(part, '|');
            if (fields.size() < 2) {
                continue;
            }
            ::app::model::disputes::DisputeEvidenceBlock block{};
            block.set_evidence_type(evidence_type_from(fields[0]));
            ::app::model::disputes::FileMetadataResponseValue metadata{};
            metadata.set_file_id(fields[1]);
            if (fields.size() > 2) {
                metadata.set_file_name(fields[2]);
            }
            if (fields.size() > 3) {
                metadata.set_file_type(fields[3]);
            }
            metadata.set_file_size(0);
            metadata.set_available(true);
            block.set_file_metadata_response(metadata);
            result.push_back(block);
        }
        return result;
    }

    [[nodiscard]] std::string
    encode_evidence(const std::vector<::app::model::disputes::DisputeEvidenceBlock>& blocks) const
    {
        std::string out{};
        for (const auto& block: blocks) {
            if (!out.empty()) {
                out.push_back(';');
            }
            out += evidence_type_name(block.get_evidence_type());
            out.push_back('|');
            out += block.get_file_metadata_response().get_file_id();
            out.push_back('|');
            out += block.get_file_metadata_response().get_file_name().value_or(std::string{});
            out.push_back('|');
            out += block.get_file_metadata_response().get_file_type();
        }
        return out;
    }

    [[nodiscard]] std::string evidence_type_name(::app::model::disputes::EvidenceType type) const
    {
        using T = ::app::model::disputes::EvidenceType;
        switch (type) {
            case T::CancellationPolicy:
                return "cancellation_policy";
            case T::CustomerCommunication:
                return "customer_communication";
            case T::CustomerSignature:
                return "customer_signature";
            case T::Receipt:
                return "receipt";
            case T::RefundPolicy:
                return "refund_policy";
            case T::ServiceDocumentation:
                return "service_documentation";
            case T::ShippingDocumentation:
                return "shipping_documentation";
            case T::InvoiceShowingDistinctTransactions:
                return "invoice_showing_distinct_transactions";
            case T::RecurringTransactionAgreement:
                return "recurring_transaction_agreement";
            case T::UncategorizedFile:
                return "uncategorized_file";
        }
        return "uncategorized_file";
    }

    [[nodiscard]] ::app::model::disputes::EvidenceType
    evidence_type_from(std::string_view name) const
    {
        using T = ::app::model::disputes::EvidenceType;
        if (name == "cancellation_policy") {
            return T::CancellationPolicy;
        }
        if (name == "customer_communication") {
            return T::CustomerCommunication;
        }
        if (name == "customer_signature") {
            return T::CustomerSignature;
        }
        if (name == "receipt") {
            return T::Receipt;
        }
        if (name == "refund_policy") {
            return T::RefundPolicy;
        }
        if (name == "service_documentation") {
            return T::ServiceDocumentation;
        }
        if (name == "shipping_documentation") {
            return T::ShippingDocumentation;
        }
        if (name == "invoice_showing_distinct_transactions") {
            return T::InvoiceShowingDistinctTransactions;
        }
        if (name == "recurring_transaction_agreement") {
            return T::RecurringTransactionAgreement;
        }
        return T::UncategorizedFile;
    }

    // ── core/disputes.rs:1173-1350  process-tracker helpers ────────────────────

    [[nodiscard]] std::int64_t add_process_dispute_task_to_pt(
        std::string_view connector_name,
        const Record& dispute,
        const DisputeScope& scope,
        std::string_view schedule_time
    ) const
    {
        // add_process_dispute_task_to_pt — task "DISPUTE_PROCESS", runner
        // ProcessDisputeWorkflow, tag ["PROCESS","DISPUTE"] (core/disputes.rs:1188-1215).
        return m_context.process_trackers().schedule(
            "DISPUTE_PROCESS",
            std::format(
                "{{\"connector\":\"{}\",\"dispute_payload\":\"{}\",\"merchant_id\":\"{}\","
                "\"processor_merchant_id\":\"{}\"}}",
                connector_name,
                dispute.get_dispute_id(),
                scope.merchant_id,
                scope.merchant_id
            ),
            schedule_time.empty() ? std::string{"0"} : std::string{schedule_time}
        );
    }

    [[nodiscard]] std::int64_t
    add_dispute_list_task_to_pt(std::string_view name, const DisputeScope& scope) const
    {
        // add_dispute_list_task_to_pt — task "DISPUTE_LIST", runner
        // DisputeListWorkflow, tag ["LIST","DISPUTE"] (core/disputes.rs:1235-1265).
        return m_context.process_trackers().schedule(
            std::string{name},
            std::format(
                "{{\"merchant_id\":\"{}\",\"profile_id\":\"{}\"}}",
                std::string{scope.merchant_id},
                std::string{scope.profile_id}
            ),
            "0"
        );
    }

    void schedule_dispute_sync_task(
        const DisputeScope& scope,
        std::string_view connector_name
    ) const
    {
        // schedule_dispute_sync_task only fires for connectors in the upstream
        // `list_dispute_supported_connectors` config; local config_store mirrors it.
        if (m_context.config_store()
                .get_value("list_dispute_supported_connectors", connector_name)
                .value_or("false") != "true") {
            return;
        }
        (void)add_dispute_list_task_to_pt("DISPUTE_LIST", scope);
    }

    // ── gateway / connector / webhook substitutes ─────────────────────────────

    ::app::connector::ConnectorResponse execute_dispute_gateway(
        std::string_view operation,
        std::string_view dispute_id,
        const DisputeScope& scope,
        Record& record,
        std::string_view raw_payload
    ) const
    {
        return call_connector(operation, dispute_id, scope, record, raw_payload);
    }

    ::app::connector::ConnectorResponse call_connector(
        std::string_view operation,
        std::string_view dispute_id,
        const DisputeScope& scope,
        Record& record,
        std::string_view raw_payload
    ) const
    {
        ::app::connector::ConnectorRequest request{};
        request.set_operation(std::string{operation});
        request.set_merchant_id(
            scope.merchant_id.empty() ? record.get_merchant_id() : scope.merchant_id
        );
        request.set_resource_id(std::string{dispute_id});
        request.set_raw_payload(std::string{raw_payload});
        if (!record.get_currency().empty()) {
            request.set_currency(record.get_currency());
        }
        if (record.get_amount() > 0) {
            request.set_amount(record.get_amount());
        }

        auto connector = m_context.connector_router().select_for(request);
        if (!connector) {
            connector = m_context.connector_router().select(request.get_merchant_id());
        }
        if (!connector) {
            m_context.metrics().counter("disputes_connector_unavailable").increment();
            return {};
        }
        const std::string name{connector->name()};
        record.set_connector(name);
        if (m_context.config_store().is_killed(name)) {
            m_context.metrics().counter("disputes_connector_killed").increment();
        }
        if (m_context.config_store().is_shadow_killed(name)) {
            m_context.metrics().counter("disputes_connector_shadow_killed").increment();
        }

        ::app::connector::ConnectorResponse response{};
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
            if (!response.get_reference_id().empty()) {
                store_connector_dispute_id(dispute_id, response.get_reference_id());
            }
            m_context.config_store()
                .put("dispute_connector_status", std::string{dispute_id}, response.get_status());
        } else {
            m_context.metrics().counter("disputes_connector_no_status").increment();
        }
        m_context.metrics().record_flow(std::string{"dispute.connector."} + std::string{operation});
        m_context.metrics()
            .counter(std::string{"disputes_connector_"} + std::string{operation})
            .increment();
        return response;
    }

    /** @brief core/disputes.rs:1123-1170 `update_dispute_data` (local store + webhook). */
    void update_dispute_data(
        const DisputeScope& scope,
        const Record& dispute,
        std::string_view flow
    ) const
    {
        store_connector_dispute_id(
            dispute.get_dispute_id(),
            connector_dispute_id_of(dispute.get_dispute_id())
        );
        notify_dispute(dispute, flow);
    }

    void notify_dispute(const Record& record, std::string_view flow) const
    {
        const std::string payload = std::format(
            "{{\"dispute_id\":\"{}\",\"payment_id\":\"{}\",\"merchant_id\":\"{}\",\"status\":\"{}"
            "\",\"flow\":\"{}\"}}",
            record.get_dispute_id(),
            record.get_payment_id(),
            record.get_merchant_id(),
            m_input.canonical_status(record.get_status()),
            flow
        );
        const std::string topic =
            std::string{"dispute."} + m_input.canonical_status(record.get_status());
        const std::string event_id = m_context.events().publish(topic, payload);
        const std::string endpoint =
            m_context.config_store()
                .get_value("dispute_webhook_endpoint", record.get_merchant_id())
                .value_or(std::string{"local://dispute-webhook/"} + record.get_merchant_id());
        m_context.webhook_delivery().enqueue(event_id, endpoint);
        m_context.metrics().counter("disputes_webhook_notified").increment();
    }

    // ── DTO conversion ────────────────────────────────────────────────────────

    [[nodiscard]] ::app::model::disputes::DisputeResponse to_response(const Record& record) const
    {
        using namespace ::app::model::disputes;
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
        stage.set_value(m_input.canonical_stage(record.get_stage()));
        response.set_dispute_stage(stage);
        DisputeStatusValue status{};
        status.set_value(m_input.canonical_status(record.get_status()));
        response.set_dispute_status(status);
        response.set_connector(record.get_connector());
        response.set_connector_status(
            m_context.config_store()
                .get_value("dispute_connector_status", record.get_dispute_id())
                .value_or(std::string{})
        );
        response.set_connector_dispute_id(connector_dispute_id_of(record.get_dispute_id()));
        PrimitiveDateTimeValue created{};
        created.set_value(record.get_created_at());
        response.set_created_at(created);
        if (!record.get_profile_id().empty()) {
            ProfileIdValue profile{};
            profile.set_value(record.get_profile_id());
            response.set_profile_id(profile);
        }
        if (const auto mca_id = m_context.config_store().get_value(
                "dispute_merchant_connector_id",
                record.get_dispute_id()
            )) {
            MerchantConnectorAccountIdValue mca{};
            mca.set_value(*mca_id);
            response.set_merchant_connector_id(mca);
        }
        response.set_is_already_refunded(false);
        return response;
    }

    [[nodiscard]] ::app::model::disputes::PlatformDisputeListItem
    to_platform_item(const Record& record) const
    {
        using namespace ::app::model::disputes;
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
        stage.set_value(m_input.canonical_stage(record.get_stage()));
        item.set_dispute_stage(stage);
        DisputeStatusValue status{};
        status.set_value(m_input.canonical_status(record.get_status()));
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
        for (const auto& attempt: m_context.repository().list_payment_attempts()) {
            if (attempt.get_payment_id() == payment_id) {
                return attempt.get_attempt_id();
            }
        }
        return {};
    }

    [[nodiscard]] ::app::model::disputes::DisputeListGetConstraints platform_as_list_constraints(
        const ::app::model::disputes::PlatformDisputeListConstraints& c
    ) const
    {
        ::app::model::disputes::DisputeListGetConstraints out{};
        out.set_dispute_id(c.get_dispute_id());
        out.set_payment_id(c.get_payment_id());
        out.set_limit(c.get_limit());
        out.set_offset(c.get_offset());
        out.set_profile_id(c.get_profile_id());
        out.set_dispute_status(c.get_dispute_status());
        out.set_dispute_stage(c.get_dispute_stage());
        out.set_reason(c.get_reason());
        out.set_connector(c.get_connector());
        out.set_time_range(c.get_time_range());
        return out;
    }

    /** @brief `find_disputes_by_constraints` over the local store's query API. */
    [[nodiscard]] std::vector<Record> find_disputes_by_constraints(
        const DisputeScope& scope,
        const ::app::model::disputes::DisputeListGetConstraints& constraints,
        const std::optional<std::vector<std::string>>& profile_id_list,
        bool platform,
        bool paginate = true,
        std::string_view platform_merchant_override = {}
    ) const
    {
        core::db::QuerySpec spec = m_context.database().template query<Record>().spec();
        if (platform) {
            // Upstream scopes by the provider (platform) account id. A
            // processor_merchant_id query param further narrows to a connected merchant.
            const std::string merchant = platform_merchant_override.empty()
                                             ? scope.merchant_id
                                             : std::string{platform_merchant_override};
            if (!merchant.empty()) {
                spec.predicates.push_back(
                    core::db::Predicate{
                        .column = "merchant_id",
                        .comparison = core::db::Compare::EQ,
                        .value = core::db::DbValue{merchant}
                    }
                );
            }
        } else if (!scope.merchant_id.empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "merchant_id",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{scope.merchant_id}
                }
            );
        }
        if (constraints.get_dispute_id().has_value() && !constraints.get_dispute_id()->empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "dispute_id",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{*constraints.get_dispute_id()}
                }
            );
        }
        if (constraints.get_payment_id().has_value() &&
            !constraints.get_payment_id()->get_value().empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "payment_id",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{constraints.get_payment_id()->get_value()}
                }
            );
        }
        if (constraints.get_reason().has_value() && !constraints.get_reason()->empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "reason",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{*constraints.get_reason()}
                }
            );
        }
        if (paginate) {
            if (const std::size_t limit = to_size(constraints.get_limit().get_value(), 10);
                limit > 0) {
                spec.limit_count = limit;
            }
            spec.offset_count = to_size(constraints.get_offset().get_value(), 0);
        }
        std::vector<Record> records = m_context.database().template collect<Record>(spec);

        // Filters the local query layer can express via predicates are applied above;
        // the remaining enum/list filters are applied in-process to match upstream.
        std::vector<Record> filtered{};
        for (const auto& record: records) {
            if (profile_id_list.has_value() && !profile_id_list->empty() &&
                !record.get_profile_id().empty() &&
                std::find(
                    profile_id_list->begin(),
                    profile_id_list->end(),
                    record.get_profile_id()
                ) == profile_id_list->end()) {
                continue;
            }
            if (constraints.get_profile_id().has_value() &&
                !constraints.get_profile_id()->get_value().empty() &&
                record.get_profile_id() != constraints.get_profile_id()->get_value()) {
                continue;
            }
            if (constraints.get_dispute_status().has_value() &&
                !constraints.get_dispute_status()->empty()) {
                bool hit{false};
                for (const auto& s: *constraints.get_dispute_status()) {
                    if (m_input.canonical_status(record.get_status()) == s.get_value()) {
                        hit = true;
                        break;
                    }
                }
                if (!hit) {
                    continue;
                }
            }
            if (constraints.get_dispute_stage().has_value() &&
                !constraints.get_dispute_stage()->empty()) {
                bool hit{false};
                for (const auto& s: *constraints.get_dispute_stage()) {
                    if (m_input.canonical_stage(record.get_stage()) == s.get_value()) {
                        hit = true;
                        break;
                    }
                }
                if (!hit) {
                    continue;
                }
            }
            if (constraints.get_connector().has_value() && !constraints.get_connector()->empty()) {
                bool hit{false};
                for (const auto& c: *constraints.get_connector()) {
                    if (record.get_connector() == c) {
                        hit = true;
                        break;
                    }
                }
                if (!hit) {
                    continue;
                }
            }
            if (constraints.get_time_range().has_value()) {
                const auto& start = constraints.get_time_range()->get_value();
                if (!start.empty() && record.get_created_at() < start) {
                    continue;
                }
            }
            filtered.push_back(record);
        }
        return filtered;
    }

    [[nodiscard]] std::vector<std::string> split(std::string_view text, char delimiter) const
    {
        std::vector<std::string> parts{};
        std::size_t start{0};
        while (start <= text.size()) {
            const std::size_t at = text.find(delimiter, start);
            const std::size_t end = at == std::string_view::npos ? text.size() : at;
            parts.emplace_back(text.substr(start, end - start));
            if (at == std::string_view::npos) {
                break;
            }
            start = at + 1;
        }
        return parts;
    }

    [[nodiscard]] std::size_t to_size(std::string_view text, std::size_t fallback) const
    {
        if (text.empty()) {
            return fallback;
        }
        std::size_t result{0};
        for (const char ch: text) {
            if (ch < '0' || ch > '9') {
                return fallback;
            }
            result = result * 10 + static_cast<std::size_t>(ch - '0');
        }
        return result;
    }

    [[nodiscard]] std::string now_string() const
    {
        return std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            )
                .count()
        );
    }

    /** @brief Build a JSON object from an id key plus known query keys. */
    [[nodiscard]] std::string query_json(
        const Query& query,
        std::string_view id_key,
        std::string_view id_value,
        std::initializer_list<std::string_view> keys = {}
    ) const
    {
        const auto escape = [](std::string_view text)
        {
            std::string out{};
            out.reserve(text.size());
            for (const char ch: text) {
                switch (ch) {
                    case '"':
                        out += "\\\"";
                        break;
                    case '\\':
                        out += "\\\\";
                        break;
                    case '\n':
                        out += "\\n";
                        break;
                    case '\r':
                        out += "\\r";
                        break;
                    case '\t':
                        out += "\\t";
                        break;
                    default:
                        out.push_back(ch);
                        break;
                }
            }
            return out;
        };
        std::string out{"{"};
        bool first{true};
        const auto add = [&out, &first, &escape](std::string_view key, std::string_view value)
        {
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
        for (const auto key: keys) {
            if (const auto value = query.get(key)) {
                add(key, *value);
            }
        }
        out.push_back('}');
        return out;
    }

    void store_connector_dispute_id(std::string_view dispute_id, std::string_view value) const
    {
        m_context.config_store()
            .put("dispute_connector_ref", std::string{dispute_id}, std::string{value});
    }

    [[nodiscard]] std::string connector_dispute_id_of(std::string_view dispute_id) const
    {
        return m_context.config_store()
            .get_value("dispute_connector_ref", dispute_id)
            .value_or(std::string{});
    }

    [[nodiscard]] std::expected<std::vector<::app::model::disputes::DisputeResponse>, Error>
    retrieve_disputes_list(
        const DisputeScope& scope,
        const ::app::model::disputes::DisputeListGetConstraints& constraints,
        const std::optional<std::vector<std::string>>& profile_id_list
    ) const
    {
        // core/disputes.rs:191-201 — constraints conversion + find_disputes_by_constraints.
        const auto records =
            find_disputes_by_constraints(scope, constraints, profile_id_list, false);
        std::vector<::app::model::disputes::DisputeResponse> result{};
        result.reserve(records.size());
        for (const auto& record: records) {
            auto response = to_response(record);
            // core/disputes.rs:206-227 — per-dispute is_already_refunded.
            response.set_is_already_refunded(is_already_refunded(m_context.repository(), record));
            result.push_back(std::move(response));
        }
        return result;
    }

    [[nodiscard]] std::expected<::app::model::disputes::DisputeListFilters, Error>
    get_filters_for_disputes(
        const DisputeScope& scope,
        const std::optional<std::vector<std::string>>& profile_id_list
    ) const
    {
        ::app::model::disputes::DisputeListFilters filters{};
        std::unordered_map<
            std::string,
            std::vector<::app::model::disputes::MerchantConnectorInfoValue>>
            map{};
        for (const auto& mca: m_context.repository().list_merchant_connectors()) {
            if (profile_id_list.has_value() && !profile_id_list->empty() &&
                !mca.get_profile_id().empty() &&
                std::find(profile_id_list->begin(), profile_id_list->end(), mca.get_profile_id()) ==
                    profile_id_list->end()) {
                continue;
            }
            if (mca.get_connector_label().empty()) {
                continue;
            } // upstream filter_map(connector_label)
            ::app::model::disputes::MerchantConnectorInfoValue info{};
            info.set_value(mca.get_connector_name());
            map[mca.get_connector_name()].push_back(info);
        }
        filters.set_connector(map);
        // Upstream static enum iterators — always present, even on an empty DB.
        std::vector<::app::model::disputes::CurrencyValue> currencies{};
        for (auto c: kCurrencies) {
            ::app::model::disputes::CurrencyValue v{};
            v.set_value(std::string{c});
            currencies.push_back(v);
        }
        std::vector<::app::model::disputes::DisputeStatusValue> statuses{};
        for (auto s: kStatuses) {
            ::app::model::disputes::DisputeStatusValue v{};
            v.set_value(std::string{s});
            statuses.push_back(v);
        }
        std::vector<::app::model::disputes::DisputeStageValue> stages{};
        for (auto s: kStages) {
            ::app::model::disputes::DisputeStageValue v{};
            v.set_value(std::string{s});
            stages.push_back(v);
        }
        filters.set_currency(currencies);
        filters.set_dispute_status(statuses);
        filters.set_dispute_stage(stages);
        (void)scope;
        return filters;
    }

    [[nodiscard]] std::expected<::app::model::disputes::PlatformDisputeListFilters, Error>
    get_platform_disputes_filters(const DisputeScope& scope) const
    {
        ::app::model::disputes::PlatformDisputeListFilters filters{};
        std::unordered_map<
            std::string,
            std::vector<::app::model::disputes::MerchantConnectorInfoValue>>
            map{};
        // Upstream iterates connected merchant accounts of the organization.
        for (const auto& account: m_context.repository().list_merchant_accounts()) {
            if (account.get_merchant_account_type() != "connected") {
                continue;
            }
            if (!scope.org_id.empty() && !account.get_organization_id().empty() &&
                account.get_organization_id() != scope.org_id) {
                continue;
            }
            for (const auto& mca: m_context.repository().list_merchant_connectors()) {
                if (mca.get_merchant_id() != account.get_merchant_id()) {
                    continue;
                }
                if (mca.get_connector_label().empty()) {
                    continue;
                }
                ::app::model::disputes::MerchantConnectorInfoValue info{};
                info.set_value(mca.get_connector_name());
                map[mca.get_connector_name()].push_back(info);
            }
        }
        filters.set_connector(map);
        std::vector<::app::model::disputes::CurrencyValue> currencies{};
        for (auto c: kCurrencies) {
            ::app::model::disputes::CurrencyValue v{};
            v.set_value(std::string{c});
            currencies.push_back(v);
        }
        std::vector<::app::model::disputes::DisputeStatusValue> statuses{};
        for (auto s: kStatuses) {
            ::app::model::disputes::DisputeStatusValue v{};
            v.set_value(std::string{s});
            statuses.push_back(v);
        }
        std::vector<::app::model::disputes::DisputeStageValue> stages{};
        for (auto s: kStages) {
            ::app::model::disputes::DisputeStageValue v{};
            v.set_value(std::string{s});
            stages.push_back(v);
        }
        filters.set_currency(currencies);
        filters.set_dispute_status(statuses);
        filters.set_dispute_stage(stages);
        return filters;
    }

    [[nodiscard]] std::expected<::app::model::disputes::DisputesAggregateResponse, Error>
    get_aggregates_for_disputes(
        const DisputeScope& scope,
        const ::app::model::disputes::TimeRange& time_range,
        const std::optional<std::vector<std::string>>& profile_id_list
    ) const
    {
        // get_dispute_status_with_count: DB fetch + in-process group-by
        // (the local store has no GROUP BY primitive).
        std::unordered_map<std::string, std::int64_t> counts{};
        for (const auto& record: m_context.repository().list_disputes()) {
            if (!scope.merchant_id.empty() && !record.get_merchant_id().empty() &&
                record.get_merchant_id() != scope.merchant_id) {
                continue;
            }
            if (profile_id_list.has_value() && !profile_id_list->empty() &&
                !record.get_profile_id().empty() &&
                std::find(
                    profile_id_list->begin(),
                    profile_id_list->end(),
                    record.get_profile_id()
                ) == profile_id_list->end()) {
                continue;
            }
            const std::string created = record.get_created_at();
            if (time_range.get_start_time().has_value() && !time_range.get_start_time()->empty() &&
                created < *time_range.get_start_time()) {
                continue;
            }
            if (time_range.get_end_time().has_value() && !time_range.get_end_time()->empty() &&
                created > *time_range.get_end_time()) {
                continue;
            }
            ++counts[m_input.canonical_status(record.get_status())];
        }
        // core/disputes.rs:944-946 — ensure every DisputeStatus is present.
        for (auto status: kStatuses) {
            counts.try_emplace(std::string{status}, 0);
        }
        ::app::model::disputes::DisputesAggregateResponse response{};
        response.set_status_with_count(counts);
        return response;
    }

    [[nodiscard]] std::string canonical_status(std::string_view value) const
    {
        return m_input.canonical_status(value);
    }

    [[nodiscard]] std::string canonical_stage(std::string_view value) const
    {
        return m_input.canonical_stage(value);
    }


private:
    // Static upstream enum iterators (common_enums::DisputeStage/DisputeStatus/Currency).
    inline static constexpr std::string_view kStages[] =
        {"pre_dispute", "dispute", "pre_arbitration", "arbitration", "dispute_reversal"};
    inline static constexpr std::string_view kStatuses[] = {
        "dispute_opened",
        "dispute_expired",
        "dispute_accepted",
        "dispute_cancelled",
        "dispute_challenged",
        "dispute_won",
        "dispute_lost"
    };
    // Upstream storage_enums::Currency::iter() — all ISO-4217 codes exposed by Hyperswitch.
    inline static constexpr std::string_view kCurrencies[] = {
        "AED", "ARS", "AUD", "BGN", "BRL", "CAD", "CHF", "CNY", "COP", "CZK", "DKK", "EUR", "GBP",
        "HKD", "HUF", "IDR", "ILS", "INR", "ISK", "JPY", "KEN", "KRW", "MXN", "MYR", "NOK", "NZD",
        "PHP", "PLN", "RON", "SEK", "SGD", "THB", "TRY", "TWD", "USD", "UYU", "VND", "ZAR"
    };

    ::app::context::AppContext& m_context;
    DisputesInputSupport m_input;
};

} // namespace app::handlers::disputes
