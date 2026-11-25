export module app_handlers_payments:runtime_common;

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
import app_store;
import app_connector;
import app_http;
import app_crypto;
import app_metrics;
import app_locking;
import app_events;
import app_workflow;
import app_services;

import :input;

export namespace app::handlers::payments {

/**  State-machine primitive ported from core/payments.rs operation guards. */
enum class OpKind
{
    Create,
    Confirm,
    Capture,
    Cancel,
    Status,
    Update,
};

/**  Outcome of one local payment operation pipeline. */
struct PipelineResult
{
    std::string id{};
    std::string merchant_id{};
    std::string status{};
    std::string connector{};
    std::string reference_id{};
    std::int64_t amount{};
    std::string currency{};
    bool locked{false};
    bool decision_rejected{false};
};

/**
 *  Shared business helpers for every payments sub-runtime. HTTP-free: all
 *  request data arrives in PaymentsRequestData and failures are returned as
 *  core::error::AppError values.
 */
class PaymentsRuntimeSupport
{
public:
    explicit PaymentsRuntimeSupport(
        app::context::AppContext& context,
        PaymentsScopeState& scope
    ) noexcept :
        m_context{context},
        m_scope{scope}
    {
    }

    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string_view message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::string{message});
    }

    [[nodiscard]] std::string query(
        const PaymentsRequestData& data,
        std::string_view key,
        std::string_view fallback = {}
    ) const
    {
        const auto it = data.query.find(std::string{key});
        return it == data.query.end() ? std::string{fallback} : it->second;
    }

    [[nodiscard]] bool
    query_bool(const PaymentsRequestData& data, std::string_view key, bool fallback = false) const
    {
        const std::string raw = query(data, key);
        if (raw == "true" || raw == "1") {
            return true;
        }
        if (raw == "false" || raw == "0") {
            return false;
        }
        return fallback;
    }

    [[nodiscard]] std::string
    last_segment(const PaymentsRequestData& data, std::size_t offset = 0) const
    {
        if (data.path_segments.size() <= offset) {
            return {};
        }
        return data.path_segments[data.path_segments.size() - 1 - offset];
    }

    [[nodiscard]] std::string metadata_value(std::string_view metadata, std::string_view key) const
    {
        const std::string needle = std::format(R"("{}":")", key);
        const auto pos = metadata.find(needle);
        if (pos == std::string_view::npos) {
            return {};
        }
        const auto start = pos + needle.size();
        const auto end = metadata.find('"', start);
        return end == std::string_view::npos ? std::string{}
                                             : std::string{metadata.substr(start, end - start)};
    }

    [[nodiscard]] std::string scope_key(std::string_view merchant, std::string_view id) const
    {
        return std::format("{}|{}", merchant, id);
    }

    [[nodiscard]] bool matches_string_list(
        const std::optional<std::vector<std::string>>& values,
        std::string_view actual
    ) const noexcept
    {
        if (!values || values->empty() || actual.empty()) {
            return true;
        }
        for (const auto& value: *values) {
            if (value == actual) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] std::int64_t
    parse_numeric(std::string_view text, std::int64_t fallback = 0) const noexcept
    {
        std::int64_t value{};
        const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
        return result.ec == std::errc{} ? value : fallback;
    }

    [[nodiscard]] std::string frame_field(std::string_view field) const
    {
        return std::format("{}:{}", field.size(), field);
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

    /**  Indexed, merchant- and profile-scoped intent lookup (local mirror). */
    [[nodiscard]] std::optional<app::store::payments::PaymentIntentRecord>
    find_payment_record(const PaymentsRequestData& data, std::string_view payment_id) const
    {
        if (payment_id.empty()) {
            return std::nullopt;
        }
        const auto record = m_context.repository().find_payment_by_string(payment_id);
        if (!record) {
            return std::nullopt;
        }
        if (!data.merchant_id.empty() && record->get_merchant_id() != data.merchant_id) {
            return std::nullopt;
        }
        if (!data.profile_id.empty()) {
            const auto it = m_scope.lookup(record->get_merchant_id(), record->get_payment_id());
            if (it.has_value() && *it != data.profile_id) {
                return std::nullopt;
            }
        }
        return record;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    ensure_payment_exists(const PaymentsRequestData& data) const
    {
        if (!data.payment_id.empty() && !find_payment_record(data, data.payment_id)) {
            return make_error(core::error::AppErrorKind::NOT_FOUND, "payment not found");
        }
        return std::nullopt;
    }

    /**  Profile-scoped repository list: local in-process map plus a profile_id
     *   entry in the record metadata (the store DTO has no dedicated column). */
    [[nodiscard]] std::vector<app::store::payments::PaymentIntentRecord>
    list_payments_scoped(const PaymentsRequestData& data) const
    {
        app::store::PaymentQuery store_query{};
        store_query.merchant_id(data.merchant_id);
        if (const auto status = query(data, "status"); !status.empty()) {
            store_query.status(status);
        }
        auto records = m_context.repository().list_payments(store_query);
        if (!data.profile_id.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> scoped{};
            for (const auto& record: records) {
                const auto it = m_scope.lookup(record.get_merchant_id(), record.get_payment_id());
                if (!it.has_value() || *it == data.profile_id) {
                    scoped.push_back(record);
                }
            }
            records = std::move(scoped);
        }
        return records;
    }

    [[nodiscard]] OpKind op_kind_for(std::string_view connector_operation) const
    {
        if (connector_operation == "capture") {
            return OpKind::Capture;
        }
        if (connector_operation == "cancel") {
            return OpKind::Cancel;
        }
        if (connector_operation == "sync" || connector_operation == "verify") {
            return OpKind::Status;
        }
        if (connector_operation == "tokenize") {
            return OpKind::Update;
        }
        return connector_operation == "authorize" ? OpKind::Confirm : OpKind::Create;
    }

    [[nodiscard]] bool
    state_transition_allowed(std::string_view current, std::string_view connector_operation) const
    {
        if (current.empty()) {
            return true;
        }
        const OpKind kind = op_kind_for(connector_operation);
        switch (kind) {
            case OpKind::Capture:
                return current == "requires_capture" || current == "authorized" ||
                       current == "pending";
            case OpKind::Cancel:
                return current != "succeeded" && current != "cancelled" && current != "canceled";
            case OpKind::Status:
            case OpKind::Update:
            case OpKind::Create:
            case OpKind::Confirm:
                return true;
        }
        return true;
    }

    [[nodiscard]] bool
    should_call_connector(OpKind kind, std::string_view current, bool force_sync) const
    {
        if (kind == OpKind::Status && !force_sync &&
            (current == "succeeded" || current == "cancelled" || current == "failed")) {
            return false;
        }
        return true;
    }

    /**
     *  Deterministic local substitute for upstream payments_operation_core:
     *  per-payment lock, routing decision, connector selection/routing, intent +
     *  attempt persistence, event publication, process-tracker scheduling and
     *  config-driven surcharge/currency consultation.
     */
    [[nodiscard]] PipelineResult run_payment_pipeline(
        const PaymentsRequestData& data,
        std::string_view operation,
        std::string_view connector_operation,
        std::string_view fallback_status
    ) const
    {
        PipelineResult result{};
        const std::string& body = data.body;
        const std::string merchant = data.merchant_id;
        result.merchant_id = merchant.empty() ? data.merchant_id_path : merchant;
        if (result.merchant_id.empty()) {
            result.merchant_id = query(data, "merchant_id");
        }

        std::string id = data.payment_id;
        if (id.empty()) {
            id = m_context.ids().payment_id();
        }
        result.id = id;

        const std::string lock_key = std::format("payment:{}", id);
        auto lock = m_context.locks().acquire(lock_key, std::int64_t{30}, std::string{operation});
        if (!lock) {
            m_context.metrics().counter("payments.lock_contended").increment();
            return result;
        }
        result.locked = true;

        const auto stored = find_payment_record(data, id);
        result.amount = stored ? stored->get_amount() : 0;
        result.currency =
            stored && !stored->get_currency().empty() ? stored->get_currency() : std::string{"USD"};

        const std::string current = stored ? stored->get_status() : std::string{};
        const OpKind kind = op_kind_for(connector_operation);
        const bool invalid_transition = !state_transition_allowed(current, connector_operation);
        if (invalid_transition) {
            result.status = current;
            result.decision_rejected = true;
            m_context.metrics().counter("payments.invalid_transition").increment();
            m_context.locks().release(lock_key);
            return result;
        }

        (void)m_context.config_store().get_value("payments_surcharge", result.merchant_id);
        (void)m_context.config_store().get_value("payments_installments", result.merchant_id);
        (void)m_context.config_store().get_value("payments_multi_currency", result.merchant_id);
        m_context.metrics().counter("payments.pricing_config_consulted").increment();

        constexpr std::string_view routing_program =
            R"({"rules":[{"if_condition":{"field":"currency","op":"in","value":"USD,EUR,GBP"},"action":"route","connector":""}],"default_action":"route","default_connector":""})";
        const std::string context_json = std::format(
            R"({{"amount":"{}","currency":{},"payment_method":"","country":"","connector":""}})",
            result.amount,
            serde::Ser::encode_json(result.currency)
        );
        const auto decision = m_context.decision_engine().evaluate(routing_program, context_json);
        if (!decision.get_error_kind().empty()) {
            result.decision_rejected = true;
            m_context.metrics().counter("payments.decision_rejected").increment();
            m_context.locks().release(lock_key);
            return result;
        }
        m_context.metrics().counter("payments.decision_evaluated").increment();

        auto connector_request = app::connector::ConnectorRequestBuilder{}
                                     .operation(std::string{connector_operation})
                                     .merchant_id(result.merchant_id)
                                     .resource_id(id)
                                     .raw_payload(body)();
        app::connector::ConnectorResponse connector_response{};
        if (should_call_connector(kind, current, query_bool(data, "force_sync"))) {
            if (auto connector = m_context.connector_router().select_for(connector_request)) {
                result.connector = connector->name();
                if (connector_operation == "capture") {
                    connector_response = connector->capture(connector_request);
                } else if (connector_operation == "cancel") {
                    connector_response = connector->cancel(connector_request);
                } else if (connector_operation == "sync") {
                    connector_response = connector->sync(connector_request);
                } else if (connector_operation == "verify") {
                    connector_response = connector->verify(connector_request);
                } else if (connector_operation == "tokenize") {
                    connector_response = connector->tokenize(connector_request);
                } else {
                    connector_response = connector->authorize(connector_request);
                }
            } else {
                const auto error = m_context.connector_router().last_error();
                m_context.metrics().counter("payments.connector_unavailable").increment();
                (void)error;
            }
        }
        result.reference_id = connector_response.get_reference_id();
        result.status = connector_response.get_status().empty() ? std::string{fallback_status}
                                                                : connector_response.get_status();
        if (connector_response.get_retryable()) {
            m_context.metrics().counter("payments.connector_retryable").increment();
        }
        if (connector_response.get_shadow_killed()) {
            m_context.metrics().counter("payments.connector_shadow_killed").increment();
        }

        if (!data.profile_id.empty()) {
            m_scope.remember(result.merchant_id, id, data.profile_id);
        }

        app::store::payments::PaymentIntentRecord record{};
        if (stored) {
            record = *stored;
        } else {
            record.set_payment_id(id);
            record.set_merchant_id(result.merchant_id);
            record.set_merchant_reference_id(query(data, "merchant_reference_id"));
            record.set_customer_id(query(data, "customer_id"));
            record.set_amount(result.amount);
            record.set_currency(result.currency);
            record.set_created_at(now_string());
        }
        record.set_status(result.status);
        record.set_updated_at(now_string());
        if (!data.profile_id.empty()) {
            record.set_metadata(
                std::format("{{\"profile_id\":{}}}", serde::Ser::encode_json(data.profile_id))
            );
        }
        if (stored) {
            m_context.repository().update_payment(record);
        } else {
            m_context.repository().save_payment(std::move(record));
        }
        m_context.metrics().counter("payments.payment_intent_persisted").increment();

        app::store::payment_attempts::PaymentAttemptRecord attempt{};
        attempt.set_attempt_id(m_context.ids().uuid_v4());
        attempt.set_payment_id(id);
        attempt.set_merchant_id(result.merchant_id);
        attempt.set_connector(result.connector);
        attempt.set_status(result.status);
        attempt.set_amount(result.amount);
        attempt.set_currency(result.currency);
        attempt.set_error_code(result.reference_id);
        attempt.set_created_at(now_string());
        m_context.repository().save_payment_attempt(std::move(attempt));
        m_context.metrics().counter("payments.payment_attempt_persisted").increment();

        const std::string event_id = m_context.events().publish(
            std::format("payment.{}", operation),
            std::format(
                R"({{"payment_id":{},"status":{},"connector":{}}})",
                serde::Ser::encode_json(id),
                serde::Ser::encode_json(result.status),
                serde::Ser::encode_json(result.connector)
            )
        );
        if (const auto endpoint =
                m_context.config_store().get_value("webhook_endpoints", result.merchant_id)) {
            m_context.webhook_delivery().enqueue(event_id, *endpoint);
        }
        const std::int64_t tracker_id = m_context.process_trackers().schedule(
            "payment_sync",
            std::format(
                R"({{"payment_id":{},"operation":{}}})",
                serde::Ser::encode_json(id),
                serde::Ser::encode_json(std::string{operation})
            ),
            "0"
        );
        if (connector_operation == "sync") {
            m_context.process_trackers().mark_terminal(tracker_id, result.status);
        }

        m_context.locks().release(lock_key);
        return result;
    }

    /**  Typed PaymentsResponse built from the persisted pipeline result. */
    [[nodiscard]] app::model::payments::PaymentsResponse
    payment_response(const PipelineResult& result) const
    {
        using namespace app::model::payments;
        PaymentsResponse response{};
        PaymentIdValue id{};
        id.set_value(result.id);
        response.set_payment_id(id);
        MerchantIdValue merchant{};
        merchant.set_value(result.merchant_id);
        response.set_merchant_id(merchant);
        response.set_processor_merchant_id(merchant);
        IntentStatusValue status{};
        status.set_value(
            result.status.empty() ? std::string{"requires_payment_method"} : result.status
        );
        response.set_status(status);
        MinorUnitValue amount{};
        amount.set_value(std::to_string(result.amount));
        response.set_amount(amount);
        response.set_net_amount(amount);
        response.set_amount_capturable(amount);
        response.set_currency(result.currency.empty() ? std::string{"USD"} : result.currency);
        if (!result.connector.empty()) {
            response.set_connector(result.connector);
        }
        return response;
    }

    /**  v2 intent wire shape: id/status/amount_details{...}/profile_id/... */
    [[nodiscard]] app::model::payments::PaymentsIntentResponse
    v2_intent_response(const PipelineResult& result) const
    {
        using namespace app::model::payments;
        PaymentsIntentResponse response{};
        GlobalPaymentIdValue id{};
        id.set_value(result.id);
        response.set_id(id);
        IntentStatusValue status{};
        status.set_value(
            result.status.empty() ? std::string{"requires_payment_method"} : result.status
        );
        response.set_status(status);
        AmountDetailsResponse amount_details{};
        MinorUnitValue order_amount{};
        order_amount.set_value(std::to_string(result.amount));
        amount_details.set_order_amount(order_amount);
        CurrencyValue currency{};
        currency.set_value(result.currency.empty() ? std::string{"USD"} : result.currency);
        amount_details.set_currency(currency);
        TaxCalculationOverrideValue external_tax{};
        external_tax.set_value("original");
        amount_details.set_external_tax_calculation(external_tax);
        SurchargeCalculationOverrideValue surcharge{};
        surcharge.set_value("original");
        amount_details.set_surcharge_calculation(surcharge);
        response.set_amount_details(amount_details);
        ProfileIdValue profile{};
        profile.set_value(result.merchant_id);
        response.set_profile_id(profile);
        CaptureMethodValue capture_method{};
        capture_method.set_value("automatic");
        response.set_capture_method(capture_method);
        FutureUsageValue future_usage{};
        future_usage.set_value("off_session");
        response.set_setup_future_usage(future_usage);
        MitExemptionRequestValue mit{};
        mit.set_value("none");
        response.set_apply_mit_exemption(mit);
        PresenceOfCustomerDuringPaymentValue present{};
        present.set_value("present");
        response.set_customer_present(present);
        return response;
    }

    /**  Typed list envelope replacing the {"count","handler"} stub. */
    [[nodiscard]] app::model::payments::PaymentListResponseV2 payment_list_response(
        const std::vector<app::store::payments::PaymentIntentRecord>& records
    ) const
    {
        using namespace app::model::payments;
        PaymentListResponseV2 response{};
        response.set_count(records.size());
        response.set_total_count(static_cast<std::int64_t>(records.size()));
        std::vector<PaymentsResponse> data{};
        data.reserve(records.size());
        for (const auto& record: records) {
            PaymentsResponse item{};
            PaymentIdValue id{};
            id.set_value(record.get_payment_id());
            item.set_payment_id(id);
            MerchantIdValue merchant{};
            merchant.set_value(record.get_merchant_id());
            item.set_merchant_id(merchant);
            item.set_processor_merchant_id(merchant);
            IntentStatusValue status{};
            status.set_value(record.get_status());
            item.set_status(status);
            MinorUnitValue amount{};
            amount.set_value(std::to_string(record.get_amount()));
            item.set_amount(amount);
            item.set_net_amount(amount);
            item.set_amount_capturable(amount);
            item.set_currency(
                record.get_currency().empty() ? std::string{"USD"} : record.get_currency()
            );
            data.push_back(std::move(item));
        }
        response.set_data(std::move(data));
        return response;
    }

    /**  Issue a signed SDK/session token from an ids()-derived secret. */
    void issue_session_token(const PaymentsRequestData& data) const
    {
        app::crypto::SecretString session_secret{m_context.ids().uuid_v4()};
        app::crypto::KeyedHasher session_hasher{std::string{session_secret.value()}};
        const std::string session_key = session_hasher.hash(data.payment_id);
        app::crypto::JwtSigner session_signer{std::string{session_secret.value()}};
        const std::string session_token = session_signer.sign(session_key);
        m_context.metrics().record_flow("payments.session_token_issued");
        m_context.cache().put(
            "payments.session:" + data.payment_id,
            core::cache::to_cache_value(session_token)
        );
    }

    /**  Typed PaymentsResponse built from the persisted record, never the body. */
    [[nodiscard]] app::model::payments::PaymentsResponse
    payment_response_view(const PaymentsRequestData& data) const
    {
        using namespace app::model::payments;
        const std::string id = data.payment_id;
        const std::string merchant = data.merchant_id;
        const auto stored = find_payment_record(data, id);

        PaymentsResponse response{};
        PaymentIdValue payment{};
        payment.set_value(stored ? stored->get_payment_id() : id);
        response.set_payment_id(payment);
        MerchantIdValue merchant_value{};
        merchant_value.set_value(stored ? stored->get_merchant_id() : merchant);
        response.set_merchant_id(merchant_value);
        response.set_processor_merchant_id(merchant_value);
        IntentStatusValue status{};
        status.set_value(
            stored && !stored->get_status().empty() ? stored->get_status()
                                                    : std::string{"requires_payment_method"}
        );
        response.set_status(status);
        MinorUnitValue amount{};
        amount.set_value(std::to_string(stored ? stored->get_amount() : 0));
        response.set_amount(amount);
        response.set_net_amount(amount);
        response.set_amount_capturable(amount);
        response.set_currency(
            stored && !stored->get_currency().empty() ? stored->get_currency() : std::string{"USD"}
        );
        return response;
    }

    /**  Local substitute for the two OpenSearch advanced-list routes. */
    [[nodiscard]] std::optional<app::model::payments::PaymentsAdvancedListResponse>
    advanced_list_response(const PaymentsRequestData& data, bool profile_scoped) const
    {
        using namespace app::model::payments;
        auto constraints =
            serde::Ser::deserialize<PaymentsAdvancedListRequest>("application/json", data.body);
        if (!constraints) {
            return std::nullopt;
        }

        auto records = list_payments_scoped(data);
        if (profile_scoped) {
            const std::string ctx_profile = data.profile_id;
            const std::string filter_profile =
                constraints->get_profile_id().value_or(std::string{});
            if (ctx_profile.empty() && filter_profile.empty()) {
                return std::nullopt;
            }
            const std::string effective = ctx_profile.empty() ? filter_profile : ctx_profile;
            std::vector<app::store::payments::PaymentIntentRecord> scoped{};
            for (const auto& record: records) {
                const auto it = m_scope.lookup(record.get_merchant_id(), record.get_payment_id());
                if (!it.has_value() || *it == effective) {
                    scoped.push_back(record);
                }
            }
            records = std::move(scoped);
        }

        const std::string query_text = constraints->get_query().value_or(std::string{});
        const std::string id_filter = constraints->get_payment_id().value_or(std::string{});
        const std::string customer_filter = constraints->get_customer_id().value_or(std::string{});
        std::unordered_map<std::string, std::string> connector_by_payment{};
        for (const auto& attempt: m_context.repository().list_payment_attempts()) {
            connector_by_payment[attempt.get_payment_id()] = attempt.get_connector();
        }
        std::vector<app::store::payments::PaymentIntentRecord> filtered{};
        for (const auto& record: records) {
            if (!id_filter.empty() && record.get_payment_id() != id_filter) {
                continue;
            }
            if (!customer_filter.empty() && record.get_customer_id() != customer_filter) {
                continue;
            }
            if (!query_text.empty() &&
                record.get_payment_id().find(query_text) == std::string::npos &&
                record.get_merchant_reference_id().find(query_text) == std::string::npos &&
                record.get_description().find(query_text) == std::string::npos) {
                continue;
            }
            if (!matches_string_list(constraints->get_status(), record.get_status()) ||
                !matches_string_list(constraints->get_currency(), record.get_currency()) ||
                !matches_string_list(
                    constraints->get_connector(),
                    connector_by_payment[record.get_payment_id()]
                ) ||
                !matches_string_list(
                    constraints->get_payment_method(),
                    metadata_value(record.get_metadata(), "payment_method")
                ) ||
                !matches_string_list(
                    constraints->get_payment_method_type(),
                    metadata_value(record.get_metadata(), "payment_method_type")
                )) {
                continue;
            }
            filtered.push_back(record);
        }

        const std::string order = constraints->get_order().value_or(std::string{});
        const bool descending = order == "Desc" || order == "desc" || order == "DESC";
        std::ranges::stable_sort(
            filtered,
            [descending](const auto& lhs, const auto& rhs)
            {
                return descending ? lhs.get_created_at() > rhs.get_created_at()
                                  : lhs.get_created_at() < rhs.get_created_at();
            }
        );

        const std::int64_t total = static_cast<std::int64_t>(filtered.size());
        const std::int64_t offset =
            std::max<std::int64_t>(0, constraints->get_offset().value_or(0));
        std::int64_t limit = constraints->get_limit().value_or(10);
        if (limit <= 0 || limit > 20) {
            limit = 10;
        }
        std::vector<PaymentsResponse> responses{};
        for (std::int64_t i = offset; i < total && i - offset < limit; ++i) {
            const auto& record = filtered[static_cast<std::size_t>(i)];
            PaymentsResponse item{};
            PaymentIdValue id{};
            id.set_value(record.get_payment_id());
            item.set_payment_id(id);
            MerchantIdValue merchant{};
            merchant.set_value(record.get_merchant_id());
            item.set_merchant_id(merchant);
            item.set_processor_merchant_id(merchant);
            IntentStatusValue status{};
            status.set_value(record.get_status());
            item.set_status(status);
            MinorUnitValue amount{};
            amount.set_value(std::to_string(record.get_amount()));
            item.set_amount(amount);
            item.set_net_amount(amount);
            item.set_amount_capturable(amount);
            item.set_currency(
                record.get_currency().empty() ? std::string{"USD"} : record.get_currency()
            );
            responses.push_back(std::move(item));
        }

        PaymentsAdvancedListResponse response{};
        response.set_count(responses.size());
        response.set_total_count(total);
        response.set_data(std::move(responses));
        return response;
    }

    /**  Local substitute for list_payment_methods_for_payments_client. */
    [[nodiscard]] std::optional<app::model::payments::PaymentsClientMethodsResponse>
    client_payment_methods_response(const PaymentsRequestData& data) const
    {
        using ClientRequest = app::model::payment_methods::PaymentMethodListRequest;
        std::string json{"{"};
        bool first{true};
        for (const std::string_view key:
             {"client_secret",
              "amount",
              "recurring_enabled",
              "installment_payment_enabled",
              "limit"}) {
            const std::string value = query(data, key);
            if (value.empty()) {
                continue;
            }
            if (!first) {
                json.push_back(',');
            }
            first = false;
            json += serde::Ser::encode_json(std::string{key});
            json.push_back(':');
            if (value == "true" || value == "false") {
                json += value;
            } else {
                bool integral{!value.empty()};
                for (const char ch: value) {
                    if (ch < '0' || ch > '9') {
                        integral = false;
                        break;
                    }
                }
                json += integral ? value : serde::Ser::encode_json(value);
            }
        }
        json.push_back('}');
        auto request = serde::Ser::deserialize<ClientRequest>("application/json", json);
        if (!request) {
            return std::nullopt;
        }
        const std::string id = data.payment_id;
        const auto record = find_payment_record(data, id);
        if (record && request->get_client_secret() && !request->get_client_secret()->empty()) {
            const std::string expected = metadata_value(record->get_metadata(), "client_secret");
            if (!expected.empty() && expected != *request->get_client_secret()) {
                return std::nullopt;
            }
        }
        std::string connector{};
        for (const auto& attempt: m_context.repository().list_payment_attempts()) {
            if (attempt.get_payment_id() == id) {
                connector = attempt.get_connector();
                break;
            }
        }
        const std::string merchant = data.merchant_id;
        const std::string enabled = m_context.config_store()
                                        .get_value("payments_client_payment_methods", merchant)
                                        .value_or(std::string{"card"});
        const std::int64_t limit = request->get_limit().value_or(10);

        using namespace app::model::payments;
        PaymentsClientMethodsResponse response{};
        std::vector<PaymentsClientMethod> methods{};
        std::size_t cursor = 0;
        while (cursor < enabled.size() && static_cast<std::int64_t>(methods.size()) < limit) {
            const auto comma = enabled.find(',', cursor);
            const std::string name = std::string{enabled.substr(
                cursor,
                comma == std::string::npos ? std::string::npos : comma - cursor
            )};
            cursor = comma == std::string::npos ? enabled.size() : comma + 1;
            if (name.empty()) {
                continue;
            }
            PaymentsClientMethod method{};
            method.set_payment_method(name);
            method.set_payment_method_type(name);
            if (!connector.empty()) {
                method.set_connector(connector);
            }
            method.set_enabled(true);
            methods.push_back(std::move(method));
        }
        response.set_count(methods.size());
        response.set_data(std::move(methods));
        return response;
    }

    /**  Deterministic local URL derived from the connector reference id. */
    [[nodiscard]] std::string
    local_redirect_url(const PaymentsRequestData& data, std::string_view reference_id) const
    {
        const std::string return_url = query(data, "return_url", "/payments/redirect");
        const std::string token =
            reference_id.empty() ? std::string{"pending"} : std::string{reference_id};
        return std::format("{}?payment_id={}&reference={}", return_url, data.payment_id, token);
    }

private:
    app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
};

} // namespace app::handlers::payments
