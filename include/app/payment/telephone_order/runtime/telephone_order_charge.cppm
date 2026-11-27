export module app_payment:telephone_order_runtime_telephone_order_charge;
import core_behavior;

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
import app_locking;
import app_events;
import app_workflow;
import app_keystore;

import :telephone_order_input;
import :telephone_order_result;
import :telephone_order_runtime_common;

export namespace app::handlers::telephone_order {

class TelephoneOrderChargeRuntime
{
public:
    explicit TelephoneOrderChargeRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief routes/telephone_order.rs:19-91 — telephone_order_charge /
     * charge_with<TwilioGenericPay>. */
    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<TwilioGenericPayResponse>
    telephone_order_charge(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = TelephoneOrderChargeData{dto, ctx};
        const auto& request = input.request;
        const std::string& transaction_reference = input.transaction_reference;

        m_context.logger().record_flow("TelephoneOrderCharge");
        m_context.logger().counter("telephone_order_charge_total").increment();

        // The route resolves the id once — span + lock.
        auto resolved_id = TelephoneOrderCore::resolve_payment_id(request);
        std::optional<std::string> payment_id{};
        if (resolved_id.ok) {
            payment_id = std::move(resolved_id.value);
        }
        if (payment_id) {
            m_context.logger().counter("telephone_order_charge_payment_id_resolved").increment();
        }

        const auto action = TelephoneOrderCore::locking_action(payment_id);
        std::optional<::app::locking::ApiLock> lock{};
        if (action.applicable) {
            lock =
                m_context.locks().acquire(action.key, std::int64_t{30}, "telephone_order_charge");
            if (!lock) {
                m_context.logger().counter("telephone_order_charge_lock_contended").increment();
                TwilioGenericPayResponse response = TwilioGenericPay::failure_response(
                    Failure::make(
                        error_codes::PAYMENT_PENDING,
                        "A charge for this payment is already in flight"
                    )
                );
                return to_result(response);
            }
        }

        if (m_support.authorize(input.auth_context)) {
            if (lock) {
                m_context.locks().release(action.key);
            }
            return std::unexpected{m_support.api_error(
                core::error::AppErrorKind::UNAUTHORIZED,
                "API key not provided or invalid API key used",
                "IR_01"
            )};
        }

        const AuthenticationData auth = m_support.authentication_data(dto, ctx);
        TwilioGenericPayResponse response =
            charge(auth, request, payment_id, transaction_reference);

        if (lock) {
            m_context.locks().release(action.key);
        }
        return to_result(response);
    }

private:
    /** @brief Copy the provider response into the typed Result DTO the handler serializes. */
    [[nodiscard]] static TwilioGenericPayResponse
    to_result(const TwilioGenericPayResponse& response)
    {
        TwilioGenericPayResponse result{};
        result.set_charge_id(response.get_charge_id());
        result.set_error_code(response.get_error_code());
        result.set_error_message(response.get_error_message());
        result.set_parameters(response.get_parameters());
        return result;
    }

    /** @brief core/telephone_order.rs:209-283 — charge (the shared create+confirm flow). */
    [[nodiscard]] TwilioGenericPayResponse charge(
        const AuthenticationData& auth,
        const TwilioGenericPayRequest& request,
        const std::optional<std::string>& route_payment_id,
        const std::string& transaction_reference
    ) const
    {
        (void)route_payment_id;
        auto payments_request =
            TelephoneOrderCore::resolve_payment_id(request)
                .and_then(
                    [this, &auth, &request](std::optional<std::string> id)
                    {
                        return TelephoneOrderCore::build_context(
                            m_context,
                            auth,
                            TwilioGenericPay::parameters(request),
                            id.value_or("")
                        );
                    }
                )
                .and_then(
                    [&request](const Context& context)
                    {
                        return TwilioGenericPay::build_payments_request(request, context);
                    }
                )
                .and_then(
                    [this](TelephoneOrderPaymentsRequest value)
                    {
                        return TelephoneOrderCore::finalize_payments_request(
                            std::move(value),
                            m_context.ids()
                        );
                    }
                );

        if (!payments_request.ok) {
            return TwilioGenericPay::report_failure(
                m_context,
                transaction_reference,
                std::move(payments_request.failure),
                "telephone_order_charge"
            );
        }

        const TelephoneOrderPaymentsRequest& mapped = payments_request.value;
        const std::string& payment_id = mapped.payment_id;

        // payments_core substitute: a retry collides with the existing payment and replays its
        // own outcome instead of charging the card again (core/telephone_order.rs:263-274).
        const auto existing = m_context.repository().find_payment_by_string(payment_id);
        if (existing) {
            m_context.logger().counter("telephone_order_charge_duplicate_replay").increment();
            m_context.logger().record_flow("TelephoneOrderChargeDuplicateReplay");
            return answer(transaction_reference, retrieve_payment(payment_id, *existing));
        }

        ::app::connector::ConnectorRequest connector_request =
            ::app::connector::ConnectorRequestBuilder{}
                .operation("authorize")
                .merchant_id(mapped.merchant_id)
                .amount(mapped.amount)
                .currency(mapped.currency)
                .payment_method_type("card")
                .resource_id(payment_id)
                .raw_payload(encode_connector_payload(mapped))();

        auto connector = m_context.connector_router().select_for(connector_request);
        if (!connector) {
            const auto router_error = m_context.connector_router().last_error();
            const auto kind = router_error ? router_error->get_kind()
                                           : core::error::AppErrorKind::CONNECTOR_ERROR;
            const std::string message =
                router_error ? router_error->get_message() : "no connector available";
            m_context.logger().counter("telephone_order_charge_processing_error").increment();
            return TwilioGenericPay::report_failure(
                m_context,
                transaction_reference,
                TelephoneOrderCore::classify_core_error(kind, message),
                "telephone_order_charge"
            );
        }

        m_context.logger().counter("telephone_order_charge_connector_authorize").increment();
        const auto connector_response = connector->authorize(connector_request);
        PaymentView payment{};
        payment.payment_id = payment_id;
        payment.status = TelephoneOrderCore::canonical_status(connector_response.get_status());
        payment.amount_capturable =
            payment.status == "requires_capture" ? mapped.amount : std::int64_t{0};
        if (payment.status == "succeeded" || payment.status == "partially_captured") {
            payment.amount_received = mapped.amount;
        }
        payment.connector = std::string{connector->name()};
        if (!connector_response.get_reference_id().empty()) {
            payment.connector_transaction_id = connector_response.get_reference_id();
        }
        payment.merchant_order_reference_id = mapped.merchant_order_reference_id;
        decode_connector_outcome(connector_response.get_raw_json(), payment);

        persist(payment, mapped);
        return answer(transaction_reference, payment);
    }

    /** @brief core/telephone_order.rs:285-309 — answer with Paid/Declined/Indeterminate. */
    [[nodiscard]] TwilioGenericPayResponse
    answer(const std::string& transaction_reference, const PaymentView& payment) const
    {
        const Outcome outcome = TelephoneOrderCore::payment_outcome(payment);
        switch (outcome.kind) {
            case OutcomeKind::Paid:
                m_context.logger().counter("telephone_order_charge_paid").increment();
                return TwilioGenericPay::success_response(
                    payment,
                    TelephoneOrderCore::outcome_parameters(payment)
                );
            case OutcomeKind::Declined:
                m_context.logger().counter("telephone_order_charge_declined").increment();
                return TwilioGenericPay::report_failure(
                    m_context,
                    transaction_reference,
                    outcome.failure,
                    "telephone_order_charge"
                );
            case OutcomeKind::Indeterminate:
            default:
                m_context.logger().counter("telephone_order_charge_pending").increment();
                m_context.logger().record_flow("TelephoneOrderChargeIndeterminate");
                return TwilioGenericPay::report_failure(
                    m_context,
                    transaction_reference,
                    outcome.failure,
                    "telephone_order_charge"
                );
        }
    }

    /**
     * @brief core/telephone_order.rs:446-500 — retrieve_payment via PSync substitute.
     * The lock guarantees the original charge finished, so the stored payment is read back
     * verbatim (force_sync=false, CallConnectorAction::Avoid); no second connector call.
     */
    [[nodiscard]] PaymentView retrieve_payment(
        const std::string& payment_id,
        const ::app::store::payments::PaymentIntentRecord& record
    ) const
    {
        PaymentView payment{};
        payment.payment_id = payment_id;
        payment.status = TelephoneOrderCore::canonical_status(record.get_status());
        payment.amount_capturable = record.get_amount();
        payment.merchant_order_reference_id = record.get_merchant_reference_id();

        const std::string attempt_id = payment_id + "_1";
        if (const auto attempt =
                m_context.repository().find_payment_attempt_by_string(attempt_id)) {
            payment.connector = attempt->get_connector();
            if (!attempt->get_error_code().empty()) {
                payment.unified_code = attempt->get_error_code();
            }
            if (!attempt->get_error_message().empty()) {
                payment.unified_message = attempt->get_error_message();
            }
        }
        return payment;
    }

    /** @brief Persist the intent + attempt (payments_core create+confirm substitute). */
    void persist(const PaymentView& payment, const TelephoneOrderPaymentsRequest& mapped) const
    {
        ::app::store::payments::PaymentIntentRecord record{};
        record.set_payment_id(payment.payment_id);
        record.set_merchant_id(mapped.merchant_id);
        record.set_merchant_reference_id(mapped.merchant_order_reference_id.value_or(""));
        record.set_customer_id(mapped.customer_id.value_or(""));
        record.set_status(payment.status);
        record.set_amount(mapped.amount);
        record.set_currency(mapped.currency);
        record.set_description(mapped.description.value_or(""));
        record.set_metadata(mapped.metadata.value_or(""));
        m_context.repository().save_payment(std::move(record));

        ::app::store::payment_attempts::PaymentAttemptRecord attempt{};
        attempt.set_attempt_id(payment.payment_id + "_1");
        attempt.set_payment_id(payment.payment_id);
        attempt.set_merchant_id(mapped.merchant_id);
        attempt.set_connector(payment.connector.value_or(""));
        attempt.set_status(payment.status);
        attempt.set_amount(mapped.amount);
        attempt.set_currency(mapped.currency);
        attempt.set_error_code(payment.unified_code.value_or(""));
        attempt.set_error_message(payment.unified_message.value_or(""));
        m_context.repository().save_payment_attempt(std::move(attempt));
    }

    /** @brief Carry the mapped PaymentsRequest flags on the connector raw_payload. */
    [[nodiscard]] std::string
    encode_connector_payload(const TelephoneOrderPaymentsRequest& mapped) const
    {
        std::string out{"{"};
        out += R"("amount":)" + std::to_string(mapped.amount);
        out += R"(,"currency":)" + json_string(mapped.currency);
        out += R"(,"payment_method":"card")";
        out += R"(,"payment_id":)" + json_string(mapped.payment_id);
        out += R"(,"merchant_id":)" + json_string(mapped.merchant_id);
        out += R"(,"confirm":true)";
        out += R"(,"capture_method":"automatic")";
        out += R"(,"authentication_type":"no_three_ds")";
        out += R"(,"payment_channel":"telephone_order")";
        out += R"(,"merchant_order_reference_id":)" +
               json_nullable(mapped.merchant_order_reference_id);
        out += R"(,"description":)" + json_nullable(mapped.description);
        out += R"(,"card_last4":)" + json_nullable(mapped.card_last4);
        out += R"(,"billing_postal_code":)" + json_nullable(mapped.billing_zip);
        out += R"(,"metadata":)" + json_nullable(mapped.metadata);
        out += R"(,"profile_id":)" + json_nullable(mapped.profile_id);
        out += R"(,"customer_id":)" + json_nullable(mapped.customer_id);
        out.push_back('}');
        return out;
    }

    /** @brief Decode connector decline codes from its raw JSON for decline_failure. */
    void decode_connector_outcome(std::string_view raw, PaymentView& payment) const
    {
        if (raw.empty()) {
            return;
        }
        const auto read = [raw](std::string_view key) -> std::optional<std::string>
        {
            const std::string pattern{"\"" + std::string{key} + "\""};
            const std::size_t key_pos = raw.find(pattern);
            if (key_pos == std::string_view::npos) {
                return std::nullopt;
            }
            std::size_t cursor = key_pos + pattern.size();
            while (cursor < raw.size() && std::isspace(static_cast<unsigned char>(raw[cursor]))) {
                ++cursor;
            }
            if (cursor >= raw.size() || raw[cursor] != ':') {
                return std::nullopt;
            }
            ++cursor;
            while (cursor < raw.size() && std::isspace(static_cast<unsigned char>(raw[cursor]))) {
                ++cursor;
            }
            if (cursor >= raw.size() || raw[cursor] != '"') {
                return std::nullopt;
            }
            ++cursor;
            const std::size_t end = raw.find('"', cursor);
            if (end == std::string_view::npos) {
                return std::nullopt;
            }
            return std::string{raw.substr(cursor, end - cursor)};
        };
        if (const auto code = read("unified_code")) {
            payment.unified_code = code;
        } else if (const auto error_code = read("error_code")) {
            payment.unified_code = error_code;
        }
        if (const auto message = read("unified_message")) {
            payment.unified_message = message;
        } else if (const auto error_message = read("error_message")) {
            payment.unified_message = error_message;
        }
    }

    ::app::context::AppContext& m_context;
    TelephoneOrderRuntimeSupport m_support;
};

} // namespace app::handlers::telephone_order
