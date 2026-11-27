export module app_payment:payout_link_runtime_render_payout_link;

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
import app_connector;
import app_http;
import app_metrics;
import app_store;
import app_services;

import :payout_link_input;
import :payout_link_result;
import :payout_link_runtime_common;

export namespace app::handlers::payout_link {

class PayoutLinkRenderRuntime
{
public:
    explicit PayoutLinkRenderRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ─────────────────────────────────────────────────────────────────────────
    // core/payout_link.rs:39-329 — initiate_payout_link
    // ─────────────────────────────────────────────────────────────────────────
    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    render(const PayoutLinkRenderInput& input) const
    {
        // routes/payout_link.rs:37-38 flow/counter (previously recorded by the
        // route handler just before calling the core).
        m_context.metrics().record_flow("PayoutLinkInitiate");
        m_context.metrics().counter("payout_link_render_total").increment();

        const std::string merchant_id = input.merchant_id;
        const std::string payout_id = input.payout_id;

        // core:49-57 find_payout_by_merchant_id_payout_id
        const auto payout = m_support.find_payout_by_merchant_id_payout_id(merchant_id, payout_id);
        if (!payout.has_value()) {
            return std::unexpected(
                m_support.make_error(core::error::AppErrorKind::NOT_FOUND, "payout not found")
            );
        }

        // core:58-66 find_payout_attempt_by_..._payout_attempt_id + utils.rs:176-178
        const std::string attempt_id =
            m_support.get_payout_attempt_id(payout_id, m_support.payout_attempt_count(*payout));
        const auto attempt =
            m_support.find_payout_attempt_by_merchant_id_payout_id_payout_attempt_id(
                merchant_id,
                payout_id,
                attempt_id
            );
        if (!attempt.has_value()) {
            return std::unexpected(
                m_support.make_error(core::error::AppErrorKind::NOT_FOUND, "payout not found")
            );
        }

        // core:67-80 payout.payout_link_id.get_required_value("payout link id")
        // and find_payout_link_by_link_id.
        const std::string link_id = m_support.payout_link_id(*payout, merchant_id, payout_id);
        if (link_id.empty()) {
            return std::unexpected(
                m_support.make_error(core::error::AppErrorKind::NOT_FOUND, "payout link not found")
            );
        }
        const auto link = m_support.find_payout_link_by_link_id(link_id);
        if (!link.has_value()) {
            return std::unexpected(
                m_support.make_error(core::error::AppErrorKind::NOT_FOUND, "payout link not found")
            );
        }

        // core:82-85 validate_payout_link_render_request_and_get_allowed_domains
        auto allowed = m_support.validate_payout_link_render_request_and_get_allowed_domains(
            input.sec_fetch_dest,
            input.origin,
            input.referer,
            *link
        );
        if (!allowed.has_value()) {
            return std::unexpected(allowed.error());
        }

        // core:88-104 status/expiry/link_data and UI defaults.
        const std::int64_t now = m_support.now_epoch();
        std::int64_t expiry_epoch{};
        const auto converted = std::from_chars(
            link->expiry.data(),
            link->expiry.data() + link->expiry.size(),
            expiry_epoch
        );
        const bool has_expired = converted.ec == std::errc{} &&
                                 converted.ptr == link->expiry.data() + link->expiry.size() &&
                                 expiry_epoch < now;
        const std::string status = link->link_status;
        const std::string locale = m_support.locale_from_request(input.accept_language);
        const UiConfig ui = m_support.resolved_ui_config(*link);
        const bool test_mode = link->link_data.test_mode.value_or(false);

        // core:105-131 (true, _) | (_, Invalidated) -> GenericExpiredLinkData
        if (has_expired || status == "invalidated") {
            if (status != "invalidated") {
                // core:114-122 PayoutLinkUpdate::StatusUpdate + update_payout_link
                m_support.update_payout_link(link_id);
            }
            const serde::Value expired = WireObject{}
                                             .str("type", "ExpiredLink")
                                             .str("title", "Payout Expired")
                                             .str("message", "This payout link has expired.")
                                             .str("theme", ui.theme)
                                             .done();
            return serde::Value{m_support.generic_links(*allowed, expired, locale)};
        }

        // core:134-275 (_, Initiated) -> PayoutLinkDetails + GenericLinkFormData
        if (status == "initiated") {
            auto result = m_support.initiate_flow(*payout, *link, *allowed, locale, ui, test_mode);
            if (!result.has_value()) {
                return std::unexpected(result.error());
            }
            return serde::Value{std::move(*result)};
        }

        // core:278-327 (_, Submitted) -> PayoutLinkStatusDetails + GenericLinkStatusData
        if (status == "submitted") {
            auto result =
                m_support.submitted_flow(*payout, *link, *attempt, *allowed, locale, ui, test_mode);
            if (!result.has_value()) {
                return std::unexpected(result.error());
            }
            return serde::Value{std::move(*result)};
        }

        return std::unexpected(
            m_support.make_error(core::error::AppErrorKind::NOT_FOUND, "payout link not found")
        );
    }

private:
    ::app::context::AppContext& m_context;
    PayoutLinkRuntimeSupport m_support;
};

} // namespace app::handlers::payout_link
