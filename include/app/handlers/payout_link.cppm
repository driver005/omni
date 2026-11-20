module;
export module app_handlers:payout_link;

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

export namespace app::handlers::payout_link {

/**
 * Strict local port of upstream `crates/router/src/routes/payout_link.rs` and
 * `crates/router/src/core/payout_link.rs` (v1, SHA 7736d55). Every upstream function is
 * mirrored by a same-named private helper below; the only local stand-ins are for
 * infrastructure the in-process store does not provide (a payout_link/address table,
 * a translation table, the HTML shell and the env/config registries). Those are listed
 * in `docs/hyperswitch-parity/payout_link.md` under "Shared contract requests".
 */
class PayoutLinkRouteHandlers
{
public:
    // ─────────────────────────────────────────────────────────────────────────
    // routes/payout_link.rs:14-39 — render_payout_link
    // ─────────────────────────────────────────────────────────────────────────
    void handle_render_payout_link(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto path_ids = payout_link_ids_from_path(req);
        if (!path_ids.has_value()) {
            send_error(res, send, error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "payout link path requires merchant_id and payout_id"));
            return;
        }

        // web::Path<(MerchantId, PayoutId)> -> typed PayoutLinkInitiateRequest
        // (routes/payout_link.rs:17-27). The payload is the exact upstream request
        // type from api_models::payouts (app/model/payouts.cppm:716-725).
        app::model::payouts::PayoutLinkInitiateRequest payload{};
        app::model::payouts::MerchantIdValue merchant_value{};
        merchant_value.set_value(path_ids->first);
        payload.set_merchant_id(std::move(merchant_value));
        app::model::payouts::PayoutIdValue payout_value{};
        payout_value.set_value(path_ids->second);
        payload.set_payout_id(std::move(payout_value));

        // &auth::MerchantIdAuth(merchant_id) (routes/payout_link.rs:35): bind the
        // principal to the path merchant id and reject a conflicting header.
        if (const auto denied = authorize_operation(
                req, "render_payout_link", payload.get_merchant_id().get_value())) {
            send_error(res, send, *denied);
            return;
        }

        m_context->metrics().record_flow("PayoutLinkInitiate");
        m_context->metrics().counter("payout_link_render_total").increment();

        auto result = initiate_payout_link(req, payload);
        if (!result.has_value()) {
            send_error(res, send, result.error());
            return;
        }
        send_json(res, send, *result, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/payout_link"}
            .add_route(
                Route{"/:merchant_id/:payout_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_render_payout_link(req, res, std::move(send));
                    })
                    .summary("Hyperswitch render_payout_link")
            );
    }

    explicit PayoutLinkRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    // ─────────────────────────────────────────────────────────────────────────
    // Local mirrors of upstream data structures (payout_link model / link_data).
    // The in-process store has no payout_link/address tables, so the persisted
    // fields live in the repository configs table under link-scoped keys; the
    // read/write shape mirrors diesel_models::PayoutLink + link_utils::PayoutLinkData.
    // ─────────────────────────────────────────────────────────────────────────
    struct UiConfig
    {
        std::string merchant_name{};
        std::string logo{};
        std::string theme{};
    };

    struct LinkData
    {
        std::string customer_id{};
        std::optional<std::string> client_secret{};
        std::optional<std::vector<std::string>> enabled_payment_methods{};
        std::optional<std::string> form_layout{};
        std::optional<bool> test_mode{};
        std::vector<std::string> allowed_domains{};
        UiConfig ui_config{};
    };

    struct PayoutLinkEntity
    {
        std::string link_id{};
        std::string link_status{};            // initiated | submitted | invalidated
        std::string expiry{};                 // epoch seconds
        std::optional<std::string> return_url{};
        std::string primary_reference{};      // payout id
        std::string merchant_id{};            // local mirror of the platform account id
        LinkData link_data{};
    };

    struct PmEnabledEntry
    {
        std::string payment_method{};
        std::vector<std::string> types{};
    };

    struct EnabledPaymentMethodEntry
    {
        std::string payment_method{};
        std::vector<std::string> payment_method_types{};
    };

    struct RequiredFieldEntry
    {
        std::string field_name{};
        std::string required_field{};
        std::string display_name{};
        std::string field_type{};
        std::optional<std::string> value{};
    };

    struct PmtTypeInfoEntry
    {
        std::string payment_method_type{};
        std::vector<RequiredFieldEntry> required_fields{};
    };

    struct PmtInfoEntry
    {
        std::string payment_method{};
        std::vector<PmtTypeInfoEntry> types_info{};
    };

    struct PayoutMethodsResult
    {
        std::vector<EnabledPaymentMethodEntry> enabled{};
        std::map<std::pair<std::string, std::string>, std::vector<std::string>> connector_per_pmt{};
    };

    // ─────────────────────────────────────────────────────────────────────────
    // core/payout_link.rs:39-329 — initiate_payout_link
    // ─────────────────────────────────────────────────────────────────────────
    [[nodiscard]] std::expected<std::string, core::error::AppError> initiate_payout_link(
        interfaces::io::IRequest& req,
        const app::model::payouts::PayoutLinkInitiateRequest& payload
    ) const
    {
        const std::string merchant_id{payload.get_merchant_id().get_value()};
        const std::string payout_id{payload.get_payout_id().get_value()};

        // core:49-57 find_payout_by_merchant_id_payout_id
        const auto payout = find_payout_by_merchant_id_payout_id(merchant_id, payout_id);
        if (!payout.has_value()) {
            return std::unexpected(error(
                core::error::AppErrorKind::NOT_FOUND, "payout not found"));
        }

        // core:58-66 find_payout_attempt_by_..._payout_attempt_id + utils.rs:176-178
        const std::string attempt_id = get_payout_attempt_id(payout_id, payout_attempt_count(*payout));
        const auto attempt = find_payout_attempt_by_merchant_id_payout_id_payout_attempt_id(
            merchant_id, payout_id, attempt_id);
        if (!attempt.has_value()) {
            return std::unexpected(error(
                core::error::AppErrorKind::NOT_FOUND, "payout not found"));
        }

        // core:67-80 payout.payout_link_id.get_required_value("payout link id")
        // and find_payout_link_by_link_id.
        const std::string link_id = payout_link_id(*payout, merchant_id, payout_id);
        if (link_id.empty()) {
            return std::unexpected(error(
                core::error::AppErrorKind::NOT_FOUND, "payout link not found"));
        }
        const auto link = find_payout_link_by_link_id(link_id);
        if (!link.has_value()) {
            return std::unexpected(error(
                core::error::AppErrorKind::NOT_FOUND, "payout link not found"));
        }

        // core:82-85 validate_payout_link_render_request_and_get_allowed_domains
        auto allowed = validate_payout_link_render_request_and_get_allowed_domains(req, *link);
        if (!allowed.has_value()) {
            return std::unexpected(allowed.error());
        }

        // core:88-104 status/expiry/link_data and UI defaults.
        const std::int64_t now = now_epoch();
        std::int64_t expiry_epoch{};
        const auto converted = std::from_chars(
            link->expiry.data(), link->expiry.data() + link->expiry.size(), expiry_epoch);
        const bool has_expired = converted.ec == std::errc{}
            && converted.ptr == link->expiry.data() + link->expiry.size()
            && expiry_epoch < now;
        const std::string status = link->link_status;
        const std::string locale = locale_from_request(req);
        const UiConfig ui = resolved_ui_config(*link);
        const bool test_mode = link->link_data.test_mode.value_or(false);

        // core:105-131 (true, _) | (_, Invalidated) -> GenericExpiredLinkData
        if (has_expired || status == "invalidated") {
            if (status != "invalidated") {
                // core:114-122 PayoutLinkUpdate::StatusUpdate + update_payout_link
                update_payout_link(link_id);
            }
            const std::string expired = std::format(
                R"({{"type":"ExpiredLink","title":{},"message":{},"theme":{}}})",
                serde::Ser::encode_json(std::string{"Payout Expired"}),
                serde::Ser::encode_json(std::string{"This payout link has expired."}),
                serde::Ser::encode_json(ui.theme)
            );
            return generic_links(*allowed, expired, locale);
        }

        // core:134-275 (_, Initiated) -> PayoutLinkDetails + GenericLinkFormData
        if (status == "initiated") {
            return initiate_flow(*payout, *link, *allowed, locale, ui, test_mode);
        }

        // core:278-327 (_, Submitted) -> PayoutLinkStatusDetails + GenericLinkStatusData
        if (status == "submitted") {
            return submitted_flow(*payout, *link, *attempt, *allowed, locale, ui, test_mode);
        }

        return std::unexpected(error(
            core::error::AppErrorKind::NOT_FOUND, "payout link not found"));
    }

    /**
     * @brief core:91-104 GenericLinkUiConfigFormData with state.conf defaults.
     * link_data.ui_config values win; empty values fall back to the configured
     * generic_link.payout_link.ui_config defaults, then the built-in defaults.
     */
    [[nodiscard]] UiConfig resolved_ui_config(const PayoutLinkEntity& link) const
    {
        UiConfig ui = link.link_data.ui_config;
        std::optional<app::store::merchant_accounts::MerchantAccountRecord> account{};
        if (!link.merchant_id.empty()) {
            account = m_context->repository().find_merchant_account_by_string(link.merchant_id);
        }
        if (ui.merchant_name.empty()) {
            ui.merchant_name = or_default(config_value("merchant_name"),
                (account.has_value() && !account->get_merchant_name().empty())
                    ? account->get_merchant_name() : std::string{"merchant"});
        }
        if (ui.logo.empty()) { ui.logo = config_value("logo"); }
        if (ui.theme.empty()) { ui.theme = or_default(config_value("theme"), std::string{"light"}); }
        return ui;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // core:134-275 — Initiated flow
    // ─────────────────────────────────────────────────────────────────────────
    [[nodiscard]] std::expected<std::string, core::error::AppError> initiate_flow(
        const app::store::payouts::PayoutRecord& payout,
        const PayoutLinkEntity& link,
        const std::vector<std::string>& allowed_domains,
        const std::string& locale,
        const UiConfig& ui,
        bool test_mode
    ) const
    {
        // core:135 customer id comes from link_data.
        const std::string customer_id = link.link_data.customer_id;

        // core:136-139 StringMajorUnitForConnector.convert(amount, destination_currency).
        const std::string amount = major_unit_amount(
            payout.get_amount(), payout.get_currency());

        // core:140-156 find_customer_by_customer_id_merchant_id.
        const auto customer = find_customer_by_customer_id_merchant_id(
            customer_id, payout.get_merchant_id());
        if (!customer.has_value()) {
            return std::unexpected(error(
                core::error::AppErrorKind::NOT_FOUND,
                std::format("Customer [\"{}\"] not found for link_id - {}",
                            customer_id, link.link_id)));
        }

        // core:157-175 find_address_by_address_id(payout.address_id). The local
        // PayoutRecord has no address_id and there is no address table; the address
        // is derived from the customer metadata / config as a local substitute.
        const std::string address = address_for_payout(*customer);
        const bool has_address = !address.empty();
        m_context->metrics().counter("payout_link_customer_loaded").increment();

        // core:177-220 filter_payout_methods + required fields.
        const auto methods = filter_payout_methods(payout, has_address);
        const auto with_fields = enabled_payment_methods_with_required_fields(
            methods.enabled, methods.connector_per_pmt, has_address);

        // core:179-199 default/request enabled payout methods.
        std::vector<EnabledPaymentMethodEntry> enabled = methods.enabled;
        if (link.link_data.enabled_payment_methods.has_value()) {
            // Link-data override, one entry per configured payment method.
            enabled.clear();
            for (const auto& pmt : *link.link_data.enabled_payment_methods) {
                EnabledPaymentMethodEntry entry{};
                entry.payment_method = pmt;
                entry.payment_method_types.push_back(pmt);
                enabled.push_back(std::move(entry));
            }
        }
        if (enabled.empty()) {
            for (const auto& pmt : parse_csv(config_value("enabled_payment_methods"))) {
                EnabledPaymentMethodEntry entry{};
                entry.payment_method = pmt;
                entry.payment_method_types.push_back(pmt);
                enabled.push_back(std::move(entry));
            }
        }
        if (enabled.empty()) {
            for (const std::string_view pmt : {"card", "bank_debit", "upi"}) {
                EnabledPaymentMethodEntry entry{};
                entry.payment_method = std::string{pmt};
                entry.payment_method_types.push_back(std::string{pmt});
                enabled.push_back(std::move(entry));
            }
        }

        // core:201-206 sort payment methods (cards first).
        std::stable_sort(enabled.begin(), enabled.end(),
            [](const EnabledPaymentMethodEntry& a, const EnabledPaymentMethodEntry& b) {
                if (a.payment_method == "card" && b.payment_method != "card") { return true; }
                if (a.payment_method != "card" && b.payment_method == "card") { return false; }
                return false;
            });
        enabled.erase(std::unique(enabled.begin(), enabled.end(),
            [](const EnabledPaymentMethodEntry& a, const EnabledPaymentMethodEntry& b) {
                return a.payment_method == b.payment_method;
            }), enabled.end());

        // core:222-250 PayoutLinkDetails.
        const auto account = m_context->repository()
            .find_merchant_account_by_string(payout.get_merchant_id());
        std::string publishable_key = account && !account->get_publishable_key().empty()
            ? account->get_publishable_key()
            : "pk_test_" + payout.get_merchant_id();
        if (publishable_key == "pk_test_" + payout.get_merchant_id()) {
            if (auto connector = m_context->connector_router().select(payout.get_merchant_id())) {
                publishable_key = "pk_" + std::string{connector->name()};
            }
        }
        const std::string client_secret = link.link_data.client_secret.value_or(link.link_id);
        const std::string expiry = to_iso8601(link.expiry);

        std::string details{"{"};
        details += std::format(
            R"("publishable_key":{},"client_secret":{},"payout_link_id":{},"payout_id":{},"customer_id":{},"session_expiry":{},"return_url":{},)",
            serde::Ser::encode_json(publishable_key),
            serde::Ser::encode_json(client_secret),
            serde::Ser::encode_json(link.link_id),
            serde::Ser::encode_json(link.primary_reference),
            serde::Ser::encode_json(customer_id),
            serde::Ser::encode_json(expiry),
            json_optional_string(link.return_url));
        // #[serde(flatten)] GenericLinkUiConfigFormData (api_models::payouts:1438-1439)
        details += std::format(
            R"("logo":{},"merchant_name":{},"theme":{},)",
            serde::Ser::encode_json(ui.logo),
            serde::Ser::encode_json(ui.merchant_name),
            serde::Ser::encode_json(ui.theme));
        details += std::format(
            R"("enabled_payment_methods":{},"enabled_payment_methods_with_required_fields":{},"amount":{},"currency":{},"locale":{},"form_layout":{},"test_mode":{})",
            enabled_payment_methods_json(enabled),
            pmt_info_json(with_fields),
            serde::Ser::encode_json(amount),
            serde::Ser::encode_json(payout.get_currency()),
            serde::Ser::encode_json(locale),
            json_optional_string(link.link_data.form_layout),
            test_mode ? "true" : "false");
        details.push_back('}');

        // core:252-267 css/js/sdk/meta wrapper.
        const std::string js = "window.__PAYOUT_DETAILS = " + details;
        const std::string sdk_url = config_value("sdk_url");
        const std::string form_data = std::format(
            R"({{"js_data":{},"css_data":{},"sdk_url":{},"html_meta_tags":{}}})",
            serde::Ser::encode_json(js),
            serde::Ser::encode_json(std::string{}),
            serde::Ser::encode_json(sdk_url),
            serde::Ser::encode_json(std::string{}));
        m_context->metrics().counter("payout_link_methods_filtered").increment();
        return generic_links(allowed_domains, form_data, locale);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // core:278-327 — Submitted flow
    // ─────────────────────────────────────────────────────────────────────────
    [[nodiscard]] std::expected<std::string, core::error::AppError> submitted_flow(
        const app::store::payouts::PayoutRecord& payout,
        const PayoutLinkEntity& link,
        const app::store::payout_attempts::PayoutAttemptRecord& attempt,
        const std::vector<std::string>& allowed_domains,
        const std::string& locale,
        const UiConfig& ui,
        bool test_mode
    ) const
    {
        // core:279-286 helpers::get_translated_unified_code_and_message
        const std::optional<std::string> error_code = attempt.get_error_code().empty()
            ? std::nullopt
            : std::optional<std::string>{attempt.get_error_code()};
        std::optional<std::string> unified_message = attempt.get_error_message().empty()
            ? std::nullopt
            : std::optional<std::string>{attempt.get_error_message()};
        unified_message = get_translated_unified_code_and_message(
            error_code, unified_message, locale);

        if (link.return_url.has_value() && !valid_url(*link.return_url)) {
            return std::unexpected(error(
                core::error::AppErrorKind::INTERNAL,
                "Failed to parse payout status link's return URL"));
        }

        std::string details{"{"};
        details += std::format(
            R"("payout_link_id":{},"payout_id":{},"customer_id":{},"session_expiry":{},"return_url":{},"status":{},"error_code":{},"error_message":{},)",
            serde::Ser::encode_json(link.link_id),
            serde::Ser::encode_json(link.primary_reference),
            serde::Ser::encode_json(link.link_data.customer_id),
            serde::Ser::encode_json(to_iso8601(link.expiry)),
            json_optional_string(link.return_url),
            serde::Ser::encode_json(payout.get_status()),
            json_optional_string(error_code),
            json_optional_string(unified_message));
        details += std::format(
            R"("logo":{},"merchant_name":{},"theme":{},"test_mode":{})",
            serde::Ser::encode_json(ui.logo),
            serde::Ser::encode_json(ui.merchant_name),
            serde::Ser::encode_json(ui.theme),
            test_mode ? "true" : "false");
        details.push_back('}');

        // core:306-319 GenericLinkStatusData { js_data, css_data }.
        const std::string js = "window.__PAYOUT_DETAILS = " + details;
        const std::string status_data = std::format(
            R"({{"js_data":{},"css_data":{}}})",
            serde::Ser::encode_json(js),
            serde::Ser::encode_json(std::string{}));
        return generic_links(allowed_domains, status_data, locale);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // core/payouts/validator.rs:622-723 — render request validation
    // ─────────────────────────────────────────────────────────────────────────
    [[nodiscard]] std::expected<std::vector<std::string>, core::error::AppError>
    validate_payout_link_render_request_and_get_allowed_domains(
        interfaces::io::IRequest& req,
        const PayoutLinkEntity& link
    ) const
    {
        const bool test_mode = link.link_data.test_mode.value_or(false);

        // (Env::Production, true) -> LinkConfigurationError. The local env is a
        // config value (shared contract request).
        const std::string env = scoped_config("generic_link", "environment", "", "");
        if (env == "production" && test_mode) {
            return std::unexpected(error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "test_mode cannot be true for rendering payout_links in production"));
        }
        // (_, true) -> Ok(HashSet::new()): no domain checks, empty allowed list.
        if (test_mode) {
            return std::vector<std::string>{};
        }

        const app::http::HeaderReader read{};
        if (read.get(req, "sec-fetch-dest") != "iframe") {
            return std::unexpected(error(
                core::error::AppErrorKind::FORBIDDEN,
                std::format("Access to payout_link [{}] is forbidden when sec-fetch-dest is not iframe",
                            link.link_id)));
        }
        std::string origin_or_referer = read.get(req, "origin");
        if (origin_or_referer.empty()) {
            origin_or_referer = read.get(req, "referer");
        }
        if (origin_or_referer.empty()) {
            return std::unexpected(error(
                core::error::AppErrorKind::FORBIDDEN,
                std::format("Access to payout_link [{}] is forbidden when origin or referer is not present",
                            link.link_id)));
        }
        const std::string domain = host_from_url(origin_or_referer);
        if (domain.empty()) {
            return std::unexpected(error(
                core::error::AppErrorKind::FORBIDDEN,
                "host or port not found in request headers"));
        }
        // common_utils/src/validation.rs:95-113 glob match over link_data.allowed_domains.
        for (const auto& allowed : link.link_data.allowed_domains) {
            if (glob_match(allowed, domain)) {
                return link.link_data.allowed_domains;
            }
        }
        return std::unexpected(error(
            core::error::AppErrorKind::FORBIDDEN,
            std::format("Access to payout_link [{}] is forbidden from requestor - {}",
                        link.link_id, domain)));
    }

    // ─────────────────────────────────────────────────────────────────────────
    // core/payout_link.rs:331-451 — filter_payout_methods
    // core/payout_link.rs:453-489 — check_currency_country_filters
    // ─────────────────────────────────────────────────────────────────────────
    [[nodiscard]] std::optional<bool> check_currency_country_filters(
        std::string_view connector,
        std::string_view pmt,
        std::string_view currency,
        std::string_view country
    ) const
    {
        // Credit/Debit are always allowed (core:459-464).
        if (pmt == "credit" || pmt == "debit") {
            return true;
        }
        const auto currency_filter = payout_method_filter_values(connector, pmt, "currency");
        const auto country_filter = payout_method_filter_values(connector, pmt, "country");
        std::optional<bool> currency_match{};
        if (!currency_filter.empty()) {
            currency_match = contains(currency_filter, std::string{currency});
        }
        std::optional<bool> country_match{};
        if (!country.empty() && !country_filter.empty()) {
            country_match = contains(country_filter, std::string{country});
        }
        // core:487 currency_filter.or(country_filter)
        return currency_match.has_value() ? currency_match : country_match;
    }

    [[nodiscard]] PayoutMethodsResult filter_payout_methods(
        const app::store::payouts::PayoutRecord& payout,
        bool /*has_address*/
    ) const
    {
        PayoutMethodsResult result{};
        std::map<std::string, std::set<std::string>> payment_method_list{};
        std::set<std::string> card{}, wallet{}, bank_transfer{}, bank_redirect{};

        const std::string country = scoped_config("payouts", "business_country", "", "");
        for (const auto& mca : m_context->repository().list_merchant_connectors()) {
            if (mca.get_merchant_id() != payout.get_merchant_id() || mca.get_disabled()) {
                continue;
            }
            // all_mcas.filter_by_connector_type(ConnectorType::PayoutProcessor) (core:353).
            const auto& type = mca.get_connector_type();
            if (type != "payout_processor" && type != "PayoutProcessor") {
                continue;
            }
            const auto entries = parse_payment_methods_enabled(mca.get_payment_methods_enabled());
            for (const auto& entry : entries) {
                for (const auto& pmt : entry.types) {
                    const auto filter = check_currency_country_filters(
                        mca.get_connector_name(), pmt, payout.get_currency(), country);
                    if (filter.has_value() && !*filter) {
                        continue;
                    }
                    std::set<std::string>* set = nullptr;
                    if (entry.payment_method == "card") { set = &card; }
                    else if (entry.payment_method == "wallet") { set = &wallet; }
                    else if (entry.payment_method == "bank_transfer") { set = &bank_transfer; }
                    else if (entry.payment_method == "bank_redirect") { set = &bank_redirect; }
                    else { continue; } // core:419-430 unsupported methods
                    set->insert(pmt);
                    payment_method_list[entry.payment_method] = *set;
                    result.connector_per_pmt[{entry.payment_method, pmt}].push_back(
                        mca.get_connector_name());
                }
            }
        }

        // core:441-450 build EnabledPaymentMethod from the non-empty sets.
        for (const auto& [payment_method, types] : payment_method_list) {
            if (types.empty()) { continue; }
            EnabledPaymentMethodEntry entry{};
            entry.payment_method = payment_method;
            entry.payment_method_types.assign(types.begin(), types.end());
            result.enabled.push_back(std::move(entry));
        }
        m_context->metrics().counter("payout_link_methods_filtered").increment();
        return result;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // core/payouts/transformers.rs:128-199 — ForeignFrom to
    // Vec<PayoutEnabledPaymentMethodsInfo> (required fields + connector union +
    // address value override).
    // ─────────────────────────────────────────────────────────────────────────
    [[nodiscard]] std::vector<PmtInfoEntry> enabled_payment_methods_with_required_fields(
        const std::vector<EnabledPaymentMethodEntry>& enabled,
        const std::map<std::pair<std::string, std::string>, std::vector<std::string>>& connector_per_pmt,
        bool has_address
    ) const
    {
        const std::string raw = scoped_config("payouts", "required_fields", "", "");
        std::vector<PmtInfoEntry> out{};
        for (const auto& entry : enabled) {
            PmtInfoEntry info{};
            info.payment_method = entry.payment_method;
            for (const auto& pmt : entry.payment_method_types) {
                PmtTypeInfoEntry type_info{};
                type_info.payment_method_type = pmt;

                auto pm_node = json_value(raw, entry.payment_method);
                auto pmt_node = pm_node ? json_value(*pm_node, pmt) : std::nullopt;
                const auto connectors_it = connector_per_pmt.find({entry.payment_method, pmt});
                if (pmt_node.has_value() && connectors_it != connector_per_pmt.end()) {
                    for (const auto& connector : connectors_it->second) {
                        auto connector_node = json_value(*pmt_node, connector);
                        if (!connector_node.has_value()) { continue; }
                        for (const auto& [field, definition] : json_members(*connector_node)) {
                            RequiredFieldEntry required{};
                            required.field_name = field;
                            required.required_field = json_value(definition, "required_field")
                                .value_or(field);
                            required.display_name = json_value(definition, "display_name")
                                .value_or(field);
                            required.field_type = json_value(definition, "field_type")
                                .value_or(std::string{"text"});
                            if (has_address && address_override_field(field)) {
                                required.value = std::string{"billing_address"};
                            }
                            const bool already = std::any_of(
                                type_info.required_fields.begin(),
                                type_info.required_fields.end(),
                                [&](const RequiredFieldEntry& existing) {
                                    return existing.field_name == field;
                                });
                            if (!already) {
                                type_info.required_fields.push_back(std::move(required));
                            }
                        }
                    }
                }
                info.types_info.push_back(std::move(type_info));
            }
            out.push_back(std::move(info));
        }
        return out;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Persistence helpers — configs table substitutes for the PayoutLink table.
    // ─────────────────────────────────────────────────────────────────────────
    [[nodiscard]] std::string link_key(std::string_view link_id, std::string_view field) const
    {
        return std::format("payout_link.{}.{}", link_id, field);
    }

    [[nodiscard]] std::string config_link_value(std::string_view link_id, std::string_view field) const
    {
        return m_context->repository().get_config(link_key(link_id, field)).value_or(std::string{});
    }

    void set_link_value(std::string_view link_id, std::string_view field, std::string value) const
    {
        m_context->repository().set_config(link_key(link_id, field), std::move(value));
    }

    /** @brief payout.payout_link_id.get_required_value("payout link id"). */
    [[nodiscard]] std::string payout_link_id(
        const app::store::payouts::PayoutRecord& payout,
        std::string_view merchant_id,
        std::string_view payout_id
    ) const
    {
        const std::string map_key = std::format("payout_link.link_id.{}:{}", merchant_id, payout_id);
        if (const auto stored = m_context->repository().get_config(map_key)) {
            return *stored;
        }
        const std::string link_id = "pl_" + std::to_string(
            std::hash<std::string>{}(std::string{merchant_id} + ":" + std::string{payout_id}));
        m_context->repository().set_config(map_key, link_id);
        seed_link(link_id, payout, merchant_id);
        return link_id;
    }

    /** @brief Seed the link entity from the payout + config on first render. */
    void seed_link(
        std::string_view link_id,
        const app::store::payouts::PayoutRecord& payout,
        std::string_view merchant_id
    ) const
    {
        const std::string payout_status = payout.get_status().empty()
            ? std::string{"pending"} : payout.get_status();
        const bool not_confirmed = payout_status == "pending"
            || payout_status == "initiated"
            || payout_status == "requires_confirmation";
        set_link_value(link_id, "link_status", not_confirmed ? "initiated" : "submitted");

        const std::string configured_expiry = config_value("session_expiry");
        set_link_value(link_id, "expiry",
                       configured_expiry.empty() ? std::string{"4102444800"} : configured_expiry);
        set_link_value(link_id, "primary_reference", payout.get_payout_id());
        set_link_value(link_id, "merchant_id", std::string{merchant_id});

        const auto account = m_context->repository().find_merchant_account_by_string(merchant_id);
        std::string return_url = account && !account->get_return_url().empty()
            ? account->get_return_url() : config_value("return_url");
        set_link_value(link_id, "return_url", return_url);

        std::string customer_id = payout.get_customer_id();
        if (customer_id.empty()) { customer_id = payout.get_payout_id(); }
        set_link_value(link_id, "customer_id", customer_id);

        set_link_value(link_id, "client_secret",
                       or_default(config_value("client_secret"), "cs_" + std::string{link_id}));
        set_link_value(link_id, "form_layout", config_value("form_layout"));
        const bool test_mode = parse_bool(or_default(config_value("test_mode"),
                                                      payout.get_connector().empty() ? "true" : "false"),
                                          true);
        set_link_value(link_id, "test_mode", test_mode ? "true" : "false");
        set_link_value(link_id, "allowed_domains", join_csv(parse_csv(config_value("allowed_domains"))));
        set_link_value(link_id, "enabled_payment_methods",
                       join_csv(parse_csv(config_value("enabled_payment_methods"))));

        const std::string merchant_name = or_default(
            config_value("merchant_name"),
            (account && !account->get_merchant_name().empty())
                ? account->get_merchant_name() : std::string{"merchant"});
        set_link_value(link_id, "merchant_name", merchant_name);
        set_link_value(link_id, "logo", config_value("logo"));
        set_link_value(link_id, "theme", or_default(config_value("theme"), std::string{"light"}));
    }

    /** @brief core:75-80 find_payout_link_by_link_id. */
    [[nodiscard]] std::optional<PayoutLinkEntity> find_payout_link_by_link_id(
        std::string_view link_id
    ) const
    {
        const std::string status = config_link_value(link_id, "link_status");
        if (status.empty()) {
            return std::nullopt;
        }
        PayoutLinkEntity link{};
        link.link_id = std::string{link_id};
        link.link_status = status;
        link.expiry = or_default(config_link_value(link_id, "expiry"), std::string{"4102444800"});
        const std::string return_url = config_link_value(link_id, "return_url");
        if (!return_url.empty()) { link.return_url = return_url; }
        link.primary_reference = config_link_value(link_id, "primary_reference");
        link.merchant_id = config_link_value(link_id, "merchant_id");

        link.link_data.customer_id = config_link_value(link_id, "customer_id");
        const std::string client_secret = config_link_value(link_id, "client_secret");
        if (!client_secret.empty()) { link.link_data.client_secret = client_secret; }
        const std::string form_layout = config_link_value(link_id, "form_layout");
        if (!form_layout.empty()) { link.link_data.form_layout = form_layout; }
        const std::string test_mode = config_link_value(link_id, "test_mode");
        if (!test_mode.empty()) { link.link_data.test_mode = parse_bool(test_mode, false); }
        link.link_data.allowed_domains = parse_csv(config_link_value(link_id, "allowed_domains"));
        const auto enabled = parse_csv(config_link_value(link_id, "enabled_payment_methods"));
        if (!enabled.empty()) { link.link_data.enabled_payment_methods = enabled; }
        link.link_data.ui_config.merchant_name = config_link_value(link_id, "merchant_name");
        link.link_data.ui_config.logo = config_link_value(link_id, "logo");
        link.link_data.ui_config.theme = config_link_value(link_id, "theme");
        return link;
    }

    /** @brief core:114-122 update_payout_link(StatusUpdate { Invalidated }). */
    void update_payout_link(std::string_view link_id) const
    {
        set_link_value(link_id, "link_status", "invalidated");
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Store lookups (core:49-83, 140-175).
    // ─────────────────────────────────────────────────────────────────────────
    [[nodiscard]] std::optional<app::store::payouts::PayoutRecord>
    find_payout_by_merchant_id_payout_id(
        std::string_view merchant_id,
        std::string_view payout_id
    ) const
    {
        const auto payout = m_context->repository().find_payout_by_string(payout_id);
        if (!payout.has_value()) { return std::nullopt; }
        // Merchant scoping from platform.get_processor().get_account().get_id().
        if (!merchant_id.empty() && payout->get_merchant_id() != std::string{merchant_id}) {
            return std::nullopt;
        }
        return payout;
    }

    /** @brief utils.rs:176-178 get_payout_attempt_id. */
    [[nodiscard]] std::string get_payout_attempt_id(
        std::string_view payout_id,
        std::int16_t attempt_count
    ) const
    {
        return std::format("{}_{}", payout_id, attempt_count);
    }

    [[nodiscard]] std::int16_t payout_attempt_count(
        const app::store::payouts::PayoutRecord& payout
    ) const
    {
        const std::string marker = metadata_value(payout.get_metadata(), "attempt_count");
        if (!marker.empty()) {
            std::int64_t parsed{};
            const auto converted = std::from_chars(
                marker.data(), marker.data() + marker.size(), parsed);
            if (converted.ec == std::errc{} && parsed >= 0) {
                return static_cast<std::int16_t>(parsed);
            }
        }
        std::size_t count{0};
        for (const auto& attempt : m_context->repository().list_payout_attempts()) {
            if (attempt.get_payout_id() == payout.get_payout_id()) { ++count; }
        }
        return static_cast<std::int16_t>(count);
    }

    [[nodiscard]] std::optional<app::store::payout_attempts::PayoutAttemptRecord>
    find_payout_attempt_by_merchant_id_payout_id_payout_attempt_id(
        std::string_view merchant_id,
        std::string_view payout_id,
        std::string_view attempt_id
    ) const
    {
        if (const auto found = m_context->repository().find_payout_attempt_by_string(attempt_id)) {
            if (found->get_merchant_id() == std::string{merchant_id}) { return found; }
        }
        for (const auto& attempt : m_context->repository().list_payout_attempts()) {
            if (attempt.get_payout_id() == std::string{payout_id}
                && attempt.get_merchant_id() == std::string{merchant_id}) {
                return attempt;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<app::store::customers::CustomerRecord>
    find_customer_by_customer_id_merchant_id(
        std::string_view customer_id,
        std::string_view merchant_id
    ) const
    {
        if (customer_id.empty()) { return std::nullopt; }
        const auto customer = m_context->repository().find_customer_by_string(customer_id);
        if (!customer.has_value()) { return std::nullopt; }
        if (!merchant_id.empty() && customer->get_merchant_id() != std::string{merchant_id}) {
            return std::nullopt;
        }
        return customer;
    }

    /** @brief core:157-175 address substitute: no payout.address_id/address table. */
    [[nodiscard]] std::string address_for_payout(
        const app::store::customers::CustomerRecord& customer
    ) const
    {
        const std::string metadata = customer.get_metadata();
        for (const std::string_view key : {"address", "billing_address", "address_id"}) {
            const std::string value = metadata_value(metadata, key);
            if (!value.empty()) { return value; }
        }
        return scoped_config("payouts", "billing_address", "", "");
    }

    [[nodiscard]] bool address_override_field(std::string_view field) const
    {
        return field == "billing_address" || field == "address" || field == "address_id"
            || field == "country" || field == "city" || field == "line1"
            || field == "postal_code" || field == "state";
    }

    // ─────────────────────────────────────────────────────────────────────────
    // core/payouts/helpers.rs:1680-1704 — translated unified message.
    // No translation table locally: config substitute then unified_message fallback.
    // ─────────────────────────────────────────────────────────────────────────
    [[nodiscard]] std::optional<std::string> get_translated_unified_code_and_message(
        const std::optional<std::string>& unified_code,
        const std::optional<std::string>& unified_message,
        const std::string& locale
    ) const
    {
        if (!unified_code.has_value() || !unified_message.has_value()) {
            return unified_message;
        }
        const std::string translated = scoped_config(
            "unified_translations", *unified_code + ":" + locale, "", "");
        if (!translated.empty()) { return translated; }
        return unified_message;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Wire shapes (GenericLinks / GenericLinksData / GenericLinkFormData /
    // GenericLinkStatusData — hyperswitch_domain_models/src/api.rs:112-164).
    // ─────────────────────────────────────────────────────────────────────────
    [[nodiscard]] std::string enabled_payment_methods_json(
        const std::vector<EnabledPaymentMethodEntry>& enabled
    ) const
    {
        std::string out{"["};
        for (std::size_t i{0}; i < enabled.size(); ++i) {
            if (i != 0) { out.push_back(','); }
            out += std::format(R"({{"payment_method":{},"payment_method_types":{}}})",
                               serde::Ser::encode_json(enabled[i].payment_method),
                               json_array(enabled[i].payment_method_types));
        }
        out.push_back(']');
        return out;
    }

    [[nodiscard]] std::string pmt_info_json(const std::vector<PmtInfoEntry>& infos) const
    {
        std::string out{"["};
        for (std::size_t i{0}; i < infos.size(); ++i) {
            if (i != 0) { out.push_back(','); }
            out += std::format(R"({{"payment_method":{},"payment_method_types_info":)",
                               serde::Ser::encode_json(infos[i].payment_method));
            out.push_back('[');
            for (std::size_t j{0}; j < infos[i].types_info.size(); ++j) {
                if (j != 0) { out.push_back(','); }
                const auto& type = infos[i].types_info[j];
                out += std::format(R"({{"payment_method_type":{},"required_fields":)",
                                   serde::Ser::encode_json(type.payment_method_type));
                if (type.required_fields.empty()) {
                    out += "null}";
                    continue;
                }
                out.push_back('{');
                for (std::size_t k{0}; k < type.required_fields.size(); ++k) {
                    if (k != 0) { out.push_back(','); }
                    const auto& field = type.required_fields[k];
                    out += std::format(
                        R"({}:{{"required_field":{},"display_name":{},"field_type":{},"value":{}}})",
                        serde::Ser::encode_json(field.field_name),
                        serde::Ser::encode_json(field.required_field),
                        serde::Ser::encode_json(field.display_name),
                        serde::Ser::encode_json(field.field_type),
                        json_optional_string(field.value));
                }
                out += "}}";
            }
            out += "]}";
        }
        out.push_back(']');
        return out;
    }

    [[nodiscard]] std::string generic_links(
        const std::vector<std::string>& allowed_domains,
        std::string_view data_json,
        std::string_view locale
    ) const
    {
        return std::format(
            R"({{"allowed_domains":{},"data":{},"locale":{}}})",
            json_array(allowed_domains),
            data_json,
            serde::Ser::encode_json(std::string{locale}));
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Generic helpers.
    // ─────────────────────────────────────────────────────────────────────────
    [[nodiscard]] core::error::AppError error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    [[nodiscard]] std::string or_default(std::string value, std::string fallback) const
    {
        return value.empty() ? std::move(fallback) : std::move(value);
    }

    [[nodiscard]] std::string config_value(std::string_view key) const
    {
        return m_context->config_store()
            .get_value("generic_link.payout_link", key).value_or(std::string{});
    }

    [[nodiscard]] std::string global_config_value(std::string_view key) const
    {
        return m_context->config_store()
            .get_value("generic_link", key).value_or(std::string{});
    }

    [[nodiscard]] std::string scoped_config(
        std::string_view ns,
        std::string_view key,
        std::string_view context,
        std::string_view fallback
    ) const
    {
        if (const auto record = m_context->superposition().resolve(ns, key, context)) {
            if (!record->get_value().empty()) { return record->get_value(); }
        }
        if (const auto value = m_context->config_store().get_value(ns, key)) {
            if (!value->empty()) { return *value; }
        }
        return std::string{fallback};
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize_operation(
        interfaces::io::IRequest& req,
        std::string_view operation,
        std::string_view path_merchant_id
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        auto context = app::auth::AuthContextBuilder{}(req);
        if (!context.merchant_id().empty()
            && context.merchant_id() != std::string{path_merchant_id}) {
            return error(core::error::AppErrorKind::FORBIDDEN,
                         "merchant id header does not match payout link path");
        }
        context.set_merchant_id(std::string{path_merchant_id});
        return m_context->authorizer().authorize_route(context, rule);
    }

    [[nodiscard]] std::vector<std::string> parse_csv(std::string_view raw) const
    {
        std::vector<std::string> out{};
        std::string current{};
        const auto push = [&]() {
            std::string value{};
            for (const char c : current) {
                if (c != '"' && c != '\'' && c != '[' && c != ']' && c != ' ' && c != '\t') {
                    value.push_back(c);
                }
            }
            if (!value.empty()) { out.push_back(std::move(value)); }
            current.clear();
        };
        for (const char c : raw) {
            if (c == ',') { push(); }
            else { current.push_back(c); }
        }
        push();
        return out;
    }

    [[nodiscard]] std::string join_csv(const std::vector<std::string>& values) const
    {
        std::string out{};
        for (std::size_t i{0}; i < values.size(); ++i) {
            if (i != 0) { out.push_back(','); }
            out += values[i];
        }
        return out;
    }

    [[nodiscard]] bool contains(const std::vector<std::string>& values, const std::string& needle) const
    {
        return std::find(values.begin(), values.end(), needle) != values.end();
    }

    [[nodiscard]] bool parse_bool(std::string_view value, bool fallback) const
    {
        if (value == "true" || value == "1") { return true; }
        if (value == "false" || value == "0") { return false; }
        return fallback;
    }

    [[nodiscard]] std::string metadata_value(std::string_view metadata, std::string_view key) const
    {
        const std::string marker{std::string{key} + "="};
        const auto pos = metadata.find(marker);
        if (pos == std::string_view::npos) { return {}; }
        const std::size_t begin = pos + marker.size();
        std::size_t end = metadata.find(';', begin);
        if (end == std::string_view::npos) { end = metadata.find(',', begin); }
        if (end == std::string_view::npos) { end = metadata.size(); }
        return std::string{metadata.substr(begin, end - begin)};
    }

    [[nodiscard]] std::int64_t now_epoch() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    /** @brief ISO-8601 UTC rendering used by PrimitiveDateTime's custom serde. */
    [[nodiscard]] std::string to_iso8601(std::string_view epoch_text) const
    {
        std::int64_t seconds{};
        const auto converted = std::from_chars(
            epoch_text.data(), epoch_text.data() + epoch_text.size(), seconds);
        if (converted.ec != std::errc{} || epoch_text.empty()) {
            return std::string{epoch_text};
        }
        std::int64_t days = seconds / 86400;
        std::int64_t remainder = seconds % 86400;
        if (remainder < 0) { remainder += 86400; --days; }
        // Howard Hinnant's civil-from-days algorithm (days from 1970-01-01).
        const std::int64_t z = days + 719468;
        const std::int64_t era = (z >= 0 ? z : z - 146096) / 146097;
        const std::int64_t doe = z - era * 146097;
        const std::int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
        std::int64_t year = yoe + era * 400;
        const std::int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        const std::int64_t mp = (5 * doy + 2) / 153;
        const std::int64_t day = doy - (153 * mp + 2) / 5 + 1;
        const std::int64_t month = mp < 10 ? mp + 3 : mp - 9;
        if (month <= 2) { ++year; }
        const std::int64_t hour = remainder / 3600;
        const std::int64_t minute = (remainder % 3600) / 60;
        const std::int64_t second = remainder % 60;
        return std::format("{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}",
                           year, month, day, hour, minute, second);
    }

    /** @brief StringMajorUnitForConnector with zero-decimal currency handling. */
    [[nodiscard]] std::string major_unit_amount(std::int64_t amount, std::string_view currency) const
    {
        constexpr std::string_view zero_decimal[] = {"JPY", "KRW", "VND", "CLP", "ISK", "UGX"};
        bool zero_decimals{false};
        for (const auto code : zero_decimal) {
            if (code == currency) { zero_decimals = true; break; }
        }
        const bool negative = amount < 0;
        const std::int64_t magnitude = negative ? -amount : amount;
        const std::string_view sign = negative ? "-" : "";
        if (zero_decimals) { return std::format("{}{}", sign, magnitude); }
        return std::format("{}{}.{:02d}", sign, magnitude / 100, magnitude % 100);
    }

    /** @brief common_utils/src/validation.rs:95-113 glob matcher (glob::Pattern). */
    [[nodiscard]] bool glob_match(std::string_view pattern, std::string_view text) const
    {
        std::size_t p{0};
        std::size_t t{0};
        std::size_t star{std::string_view::npos};
        std::size_t match{0};
        while (t < text.size()) {
            if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == text[t])) {
                ++p;
                ++t;
            } else if (p < pattern.size() && pattern[p] == '*') {
                star = p++;
                match = t;
            } else if (star != std::string_view::npos) {
                p = star + 1;
                t = ++match;
            } else {
                return false;
            }
        }
        while (p < pattern.size() && pattern[p] == '*') { ++p; }
        return p == pattern.size();
    }

    [[nodiscard]] std::string host_from_url(std::string_view url) const
    {
        std::string_view value{url};
        const auto scheme = value.find("://");
        if (scheme != std::string_view::npos) { value.remove_prefix(scheme + 3); }
        const auto end = value.find_first_of("/?#");
        if (end != std::string_view::npos) { value = value.substr(0, end); }
        const auto at = value.rfind('@');
        if (at != std::string_view::npos) { value.remove_prefix(at + 1); }
        return std::string{value};
    }

    [[nodiscard]] bool valid_url(std::string_view url) const
    {
        if (url.empty()) { return false; }
        if (url.find("://") != std::string_view::npos) { return true; }
        return url.front() == '/';
    }

    [[nodiscard]] std::string locale_from_request(interfaces::io::IRequest& req) const
    {
        const app::http::HeaderReader read{};
        std::string header = read.get(req, "accept-language");
        if (!header.empty()) {
            const auto comma = header.find(',');
            if (comma != std::string::npos) { header.erase(comma); }
            const auto semi = header.find(';');
            if (semi != std::string::npos) { header.erase(semi); }
            const auto begin = header.find_first_not_of(" \t");
            if (begin != std::string::npos) {
                const auto end = header.find_last_not_of(" \t");
                return header.substr(begin, end - begin + 1);
            }
        }
        return or_default(global_config_value("locale"), std::string{"en"});
    }

    [[nodiscard]] std::string json_array(const std::vector<std::string>& values) const
    {
        std::string out{"["};
        for (std::size_t i{0}; i < values.size(); ++i) {
            if (i != 0) { out.push_back(','); }
            out += serde::Ser::encode_json(values[i]);
        }
        out.push_back(']');
        return out;
    }

    [[nodiscard]] std::string json_optional_string(const std::optional<std::string>& value) const
    {
        return value.has_value() ? serde::Ser::encode_json(*value) : std::string{"null"};
    }

    // ── Minimal JSON helpers for MCA payment_methods_enabled / required-fields config ──
    [[nodiscard]] std::string_view skip_ws(std::string_view json, std::size_t& pos) const
    {
        while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) { ++pos; }
        return json.substr(pos);
    }

    [[nodiscard]] std::size_t balanced_end(std::string_view json, std::size_t start) const
    {
        if (start >= json.size()) { return start; }
        const char open = json[start];
        // A bare JSON string value ends at its unescaped closing quote.
        if (open == '"') {
            bool escaped{false};
            for (std::size_t i{start + 1}; i < json.size(); ++i) {
                if (escaped) { escaped = false; continue; }
                if (json[i] == '\\') { escaped = true; continue; }
                if (json[i] == '"') { return i + 1; }
            }
            return json.size();
        }
        const char close = open == '{' ? '}' : open == '[' ? ']' : open;
        std::size_t depth{0};
        bool in_string{false};
        bool escaped{false};
        for (std::size_t i{start}; i < json.size(); ++i) {
            const char c = json[i];
            if (in_string) {
                if (escaped) { escaped = false; }
                else if (c == '\\') { escaped = true; }
                else if (c == '"') { in_string = false; }
                continue;
            }
            if (c == '"') { in_string = true; }
            else if (c == open) { ++depth; }
            else if (c == close) {
                if (--depth == 0) { return i + 1; }
            }
        }
        return json.size();
    }

    [[nodiscard]] std::vector<std::string> json_array_elements(std::string_view array) const
    {
        std::vector<std::string> out{};
        std::size_t pos{0};
        skip_ws(array, pos);
        if (pos >= array.size() || array[pos] != '[') { return out; }
        ++pos;
        while (pos < array.size()) {
            skip_ws(array, pos);
            if (pos >= array.size() || array[pos] == ']') { break; }
            if (array[pos] == ',') { ++pos; continue; }
            const std::size_t end = balanced_end(array, pos);
            if (end <= pos) { break; }
            out.emplace_back(array.substr(pos, end - pos));
            pos = end;
        }
        return out;
    }

    [[nodiscard]] std::optional<std::string> json_value(
        std::string_view json,
        std::string_view key,
        std::size_t from = 0
    ) const
    {
        const std::string needle = std::format("\"{}\"", key);
        const auto pos = json.find(needle, from);
        if (pos == std::string_view::npos) { return std::nullopt; }
        std::size_t cursor = pos + needle.size();
        skip_ws(json, cursor);
        if (cursor >= json.size() || json[cursor] != ':') { return std::nullopt; }
        ++cursor;
        skip_ws(json, cursor);
        if (cursor >= json.size()) { return std::nullopt; }
        if (json[cursor] == '"') {
            const auto end = balanced_end(json, cursor);
            if (end <= cursor + 1) { return std::string{}; }
            return std::string{json.substr(cursor + 1, end - cursor - 2)};
        }
        if (json[cursor] == '{' || json[cursor] == '[') {
            const auto end = balanced_end(json, cursor);
            return std::string{json.substr(cursor, end - cursor)};
        }
        std::size_t end = cursor;
        while (end < json.size() && json[end] != ',' && json[end] != '}' && json[end] != ']') {
            ++end;
        }
        return std::string{json.substr(cursor, end - cursor)};
    }

    [[nodiscard]] std::vector<std::pair<std::string, std::string>> json_members(
        std::string_view object
    ) const
    {
        std::vector<std::pair<std::string, std::string>> out{};
        std::size_t pos{0};
        skip_ws(object, pos);
        if (pos >= object.size() || object[pos] != '{') { return out; }
        ++pos;
        while (pos < object.size()) {
            skip_ws(object, pos);
            if (pos >= object.size() || object[pos] == '}') { break; }
            if (object[pos] == ',') { ++pos; continue; }
            if (object[pos] != '"') { ++pos; continue; }
            const auto key_end = balanced_end(object, pos);
            if (key_end <= pos + 1) { break; }
            const std::string key{object.substr(pos + 1, key_end - pos - 2)};
            std::size_t colon = key_end;
            skip_ws(object, colon);
            if (colon >= object.size() || object[colon] != ':') { break; }
            ++colon;
            skip_ws(object, colon);
            std::size_t value_end{colon};
            if (colon < object.size()
                && (object[colon] == '{' || object[colon] == '[' || object[colon] == '"')) {
                value_end = balanced_end(object, colon);
            } else {
                while (value_end < object.size() && object[value_end] != ','
                       && object[value_end] != '}') { ++value_end; }
            }
            out.emplace_back(key, std::string{object.substr(colon, value_end - colon)});
            pos = value_end;
        }
        return out;
    }

    [[nodiscard]] std::vector<PmEnabledEntry> parse_payment_methods_enabled(
        std::string_view raw
    ) const
    {
        std::vector<PmEnabledEntry> out{};
        if (raw.empty()) { return out; }
        const auto array = std::string{raw};
        for (const auto& element : json_array_elements(array)) {
            const auto payment_method = json_value(element, "payment_method");
            if (!payment_method.has_value()) { continue; }
            PmEnabledEntry entry{};
            entry.payment_method = *payment_method;
            const auto types_json = json_value(element, "payment_method_types");
            if (types_json.has_value()) {
                for (const auto& type_element : json_array_elements(*types_json)) {
                    const auto type = json_value(type_element, "payment_method_type");
                    if (type.has_value() && !type->empty()) { entry.types.push_back(*type); }
                }
            }
            if (!entry.types.empty()) { out.push_back(std::move(entry)); }
        }
        return out;
    }

    [[nodiscard]] std::vector<std::string> payout_method_filter_values(
        std::string_view connector,
        std::string_view pmt,
        std::string_view kind
    ) const
    {
        const std::string specific = scoped_config(
            "payout_method_filters",
            std::string{connector} + "." + std::string{pmt} + "." + std::string{kind}, "", "");
        if (!specific.empty()) { return parse_csv(specific); }
        if (kind == "country") {
            const std::string country = scoped_config("payout_method_filters",
                                                       std::string{connector} + ".country", "", "");
            if (!country.empty()) { return parse_csv(country); }
        }
        return {};
    }

    // ── Route plumbing ──
    /** @brief Extract (merchant_id, payout_id) from the :merchant_id/:payout_id path. */
    [[nodiscard]] std::optional<std::pair<std::string, std::string>> payout_link_ids_from_path(
        interfaces::io::IRequest& req
    ) const
    {
        const auto segments = app::http::RequestPath{}(req);
        for (std::size_t i{0}; i + 2 < segments.count(); ++i) {
            if (segments.at(i).value_or(std::string_view{}) == "payout_link") {
                const auto merchant = segments.at(i + 1);
                const auto payout = segments.at(i + 2);
                if (merchant.has_value() && payout.has_value()
                    && !merchant->empty() && !payout->empty()) {
                    return std::pair<std::string, std::string>{
                        std::string{*merchant}, std::string{*payout}
                    };
                }
                return std::nullopt;
            }
        }
        if (segments.count() >= 2) {
            return std::pair<std::string, std::string>{
                std::string{*segments.at(segments.count() - 2)},
                std::string{*segments.at(segments.count() - 1)}
            };
        }
        return std::nullopt;
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error_value
    ) const
    {
        auto text = serde::Ser::encode_json(error_value);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error_value.get_kind()));
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

} // namespace app::handlers::payout_link
