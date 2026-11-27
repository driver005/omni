export module app_payment:payment_link_runtime_retrieve;

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

import :payment_link_input;
import :payment_link_result;
import :payment_link_runtime_common;

export namespace app::handlers::payment_link {

class PaymentLinkRetrieveRuntime
{
public:
    explicit PaymentLinkRetrieveRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::
        expected<::app::model::payments::RetrievePaymentLinkResponse, core::error::AppError>
        retrieve(const PaymentLinkRetrieveInput& input) const
    {
        m_context.metrics().record_flow("payment_link.retrieve");
        m_context.metrics().counter("payment_link.retrieve").increment();

        // retrieve_payment_link(): find the dedicated link row and authorize the
        // processor/merchant id against it (core:54-83).
        const auto link = m_support.find_link_by_id(input.payment_link_id);
        const auto intent = m_support.find_intent(input.payment_link_id);
        if (!link && !intent) {
            return std::unexpected{
                m_support.make_error(core::error::AppErrorKind::NOT_FOUND, "payment link not found")
            };
        }

        const std::string owner = link ? link->get_merchant_id() : intent->get_merchant_id();
        if (!input.merchant_from_credential.empty() && owner != input.merchant_from_credential) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::FORBIDDEN,
                "payment link belongs to another merchant"
            )};
        }

        const std::string currency = link && !link->get_currency().empty()
                                         ? link->get_currency()
                                         : (intent ? intent->get_currency() : std::string{});
        const std::string metadata = intent ? intent->get_metadata() : std::string{};
        const auto config = m_support.resolve_config(
            owner,
            link ? link->get_profile_id() : std::string{},
            input.payment_link_id,
            metadata
        );
        if (m_support.unsafe_config(config)) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "unsafe payment link configuration"
            )};
        }
        if (config.require_client_secret && !input.client_secret && !input.secret_claim) {
            return std::unexpected{
                m_support.make_error(core::error::AppErrorKind::FORBIDDEN, "client secret required")
            };
        }

        using Response = ::app::model::payments::RetrievePaymentLinkResponse;
        Response response{};
        response.set_payment_link_id(input.payment_link_id);

        ::app::model::payments::PaymentIdValue payment{};
        payment.set_value(input.payment_link_id);
        response.set_payment_id(std::move(payment));

        ::app::model::payments::MerchantIdValue merchant{};
        merchant.set_value(owner);
        response.set_merchant_id(std::move(merchant));
        ::app::model::payments::MerchantIdValue processor{};
        processor.set_value(owner);
        response.set_processor_merchant_id(std::move(processor));

        if (link && !link->get_profile_id().empty()) {
            ::app::model::payments::ProfileIdValue profile{};
            profile.set_value(link->get_profile_id());
            response.set_profile_id(std::move(profile));
        }

        const std::string link_url =
            link && !link->get_url().empty()
                ? link->get_url()
                : std::format("https://{}/{}/{}", config.domain, owner, input.payment_link_id);
        response.set_link_to_pay(link_url);
        if (config.require_client_secret || (link && !link->get_url().empty())) {
            response.set_secure_link(std::string{"s/"} + owner + "/" + input.payment_link_id);
        }

        const std::int64_t amount_minor =
            link ? link->get_amount() : (intent ? intent->get_amount() : 0);
        ::app::model::payments::MinorUnitValue amount{};
        // StringMajorUnitForCore (core:201-207).
        amount.set_value(m_support.major_unit_amount(amount_minor, currency));
        response.set_amount(std::move(amount));

        const std::string created = link && !link->get_created_at().empty()
                                        ? link->get_created_at()
                                        : (intent ? intent->get_created_at() : std::string{});
        ::app::model::payments::PrimitiveDateTimeValue created_value{};
        created_value.set_value(created);
        response.set_created_at(std::move(created_value));

        // expiry is the raw fulfilment_time (transformers.rs:1990); only the
        // status uses session_expiry = fulfilment_time.unwrap_or(created_at +
        // DEFAULT_SESSION_EXPIRY) via check_payment_link_status (core:71-81,621-631).
        const std::string expiry = link ? link->get_expires_at() : std::string{};
        if (!expiry.empty()) {
            ::app::model::payments::PrimitiveDateTimeValue expiry_value{};
            expiry_value.set_value(expiry);
            response.set_expiry(std::move(expiry_value));
        }

        if (intent && !intent->get_description().empty() &&
            !m_support.unsafe_text(intent->get_description())) {
            response.set_description(intent->get_description());
        }

        if (!currency.empty()) {
            ::app::model::payments::CurrencyValue currency_value{};
            currency_value.set_value(currency);
            response.set_currency(std::move(currency_value));
        }

        std::int64_t status_expiry_epoch{};
        if (!expiry.empty()) {
            status_expiry_epoch = m_support.iso_to_epoch(expiry).value_or(m_support.now_epoch());
        } else {
            status_expiry_epoch = m_support.iso_to_epoch(created).value_or(m_support.now_epoch()) +
                                  default_session_expiry;
        }
        response.set_status(
            m_support.now_epoch() > status_expiry_epoch
                ? ::app::model::payments::PaymentLinkStatus::Expired
                : ::app::model::payments::PaymentLinkStatus::Active
        );

        return ::app::model::payments::RetrievePaymentLinkResponse{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentLinkRuntimeSupport m_support;
};

} // namespace app::handlers::payment_link
