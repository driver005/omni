module;
export module app_handlers:payment_link;

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
import app_http;
import app_metrics;
import app_crypto;
import app_connector;
import app_services;

export namespace app::handlers::payment_link {

class PaymentLinkRouteHandlers
{
public:
    // ─────────────────────────────────────────────────────────────────────────
    // Routes (routes/payment_link.rs)
    // ─────────────────────────────────────────────────────────────────────────

    void handle_payment_link_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "payment_link_retrieve")) {
            send_error(res, send, *denied);
            return;
        }

        m_context->metrics().record_flow("payment_link.retrieve");
        m_context->metrics().counter("payment_link.retrieve").increment();

        // Upstream takes Query<RetrievePaymentLinkRequest> (client_secret) plus a
        // path id. The request type is a typed model, not the response type.
        using Input = app::model::payments::RetrievePaymentLinkRequest;
        std::optional<Input> input{};
        const auto body = req.body_as_string();
        if (!body.empty()) {
            auto parsed = serde::Ser::deserialize<Input>("application/json", body);
            if (!parsed) {
                send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            input = *parsed;
        }
        const auto query = app::http::RequestQuery{}(req);
        std::optional<std::string> client_secret = query.get("client_secret");
        if (!client_secret && input && input->get_client_secret()) {
            client_secret = *input->get_client_secret();
        }

        // check_sdk_auth_and_get_auth(): SDK key material plus an optional
        // client-secret assertion from the query payload (routes:22-42).
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto secret_claim = auth_context.claim("x-client-secret");
        if (secret_claim && client_secret && *secret_claim != std::string_view{*client_secret}) {
            send_error(res, send, make_error(core::error::AppErrorKind::FORBIDDEN, "client secret mismatch"));
            return;
        }

        const std::string payment_link_id = path_link_id(req);
        if (payment_link_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "missing payment link id"));
            return;
        }

        // retrieve_payment_link(): find the dedicated link row and authorize the
        // processor/merchant id against it (core:54-83).
        const auto link = find_link_by_id(payment_link_id);
        const auto intent = find_intent(payment_link_id);
        if (!link && !intent) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "payment link not found"));
            return;
        }

        const std::string merchant_from_credential = app::http::merchant_id_from_header{}(req);
        const std::string owner = link ? link->get_merchant_id()
                                       : intent->get_merchant_id();
        if (!merchant_from_credential.empty() && owner != merchant_from_credential) {
            send_error(res, send, make_error(core::error::AppErrorKind::FORBIDDEN, "payment link belongs to another merchant"));
            return;
        }

        const std::string currency = link && !link->get_currency().empty()
            ? link->get_currency()
            : (intent ? intent->get_currency() : std::string{});
        const std::string metadata = intent ? intent->get_metadata() : std::string{};
        const auto config = resolve_config(owner, link ? link->get_profile_id() : std::string{},
                                           payment_link_id, metadata);
        if (unsafe_config(config)) {
            send_error(res, send, make_error(core::error::AppErrorKind::VALIDATION_FAILED, "unsafe payment link configuration"));
            return;
        }
        if (config.require_client_secret && !client_secret && !secret_claim) {
            send_error(res, send, make_error(core::error::AppErrorKind::FORBIDDEN, "client secret required"));
            return;
        }

        using Response = app::model::payments::RetrievePaymentLinkResponse;
        Response response{};
        response.set_payment_link_id(payment_link_id);

        app::model::payments::PaymentIdValue payment{};
        payment.set_value(payment_link_id);
        response.set_payment_id(std::move(payment));

        app::model::payments::MerchantIdValue merchant{};
        merchant.set_value(owner);
        response.set_merchant_id(std::move(merchant));
        app::model::payments::MerchantIdValue processor{};
        processor.set_value(owner);
        response.set_processor_merchant_id(std::move(processor));

        if (link && !link->get_profile_id().empty()) {
            app::model::payments::ProfileIdValue profile{};
            profile.set_value(link->get_profile_id());
            response.set_profile_id(std::move(profile));
        }

        const std::string link_url = link && !link->get_url().empty()
            ? link->get_url()
            : std::format("https://{}/{}/{}", config.domain, owner, payment_link_id);
        response.set_link_to_pay(link_url);
        if (config.require_client_secret || (link && !link->get_url().empty())) {
            response.set_secure_link(std::string{"s/"} + owner + "/" + payment_link_id);
        }

        const std::int64_t amount_minor = link ? link->get_amount()
            : (intent ? intent->get_amount() : 0);
        app::model::payments::MinorUnitValue amount{};
        // StringMajorUnitForCore (core:201-207).
        amount.set_value(major_unit_amount(amount_minor, currency));
        response.set_amount(std::move(amount));

        const std::string created = link && !link->get_created_at().empty()
            ? link->get_created_at()
            : (intent ? intent->get_created_at() : std::string{});
        app::model::payments::PrimitiveDateTimeValue created_value{};
        created_value.set_value(created);
        response.set_created_at(std::move(created_value));

        // expiry is the raw fulfilment_time (transformers.rs:1990); only the
        // status uses session_expiry = fulfilment_time.unwrap_or(created_at +
        // DEFAULT_SESSION_EXPIRY) via check_payment_link_status (core:71-81,621-631).
        const std::string expiry = link ? link->get_expires_at() : std::string{};
        if (!expiry.empty()) {
            app::model::payments::PrimitiveDateTimeValue expiry_value{};
            expiry_value.set_value(expiry);
            response.set_expiry(std::move(expiry_value));
        }

        if (intent && !intent->get_description().empty() && !unsafe_text(intent->get_description())) {
            response.set_description(intent->get_description());
        }

        if (!currency.empty()) {
            app::model::payments::CurrencyValue currency_value{};
            currency_value.set_value(currency);
            response.set_currency(std::move(currency_value));
        }

        std::int64_t status_expiry_epoch{};
        if (!expiry.empty()) {
            status_expiry_epoch = iso_to_epoch(expiry).value_or(now_epoch());
        } else {
            status_expiry_epoch = iso_to_epoch(created).value_or(now_epoch()) + default_session_expiry;
        }
        response.set_status(now_epoch() > status_expiry_epoch
            ? app::model::payments::PaymentLinkStatus::Expired
            : app::model::payments::PaymentLinkStatus::Active);

        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // initiate_payment_link (routes:66-98) -> initiate_payment_link_flow
    // (core:472-522). Upstream renders a PaymentLinkForm: either the payment
    // form data (js/css/sdk/meta/redirection) or the status page data.
    void handle_initiate_payment_link(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ids = path_merchant_payment(req);
        const std::string merchant_id = ids.first;
        const std::string payment_id = ids.second;

        // MerchantIdAuth(merchant_id) from the path (routes:94).
        if (const auto denied = authorize_operation(req, "initiate_payment_link", merchant_id)) {
            send_error(res, send, *denied);
            return;
        }
        if (merchant_id.empty() || payment_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST,
                                             "merchant id or payment id not found in path"));
            return;
        }

        m_context->metrics().record_flow("payment_link.initiate");
        m_context->metrics().counter("payment_link.initiate").increment();

        auto form = form_payment_link_data(merchant_id, payment_id);
        if (!form) {
            send_error(res, send, form.error());
            return;
        }

        const std::string css = get_css_script(form->config);
        if (form->is_status) {
            send_json(res, send, status_page_data_json(form->status, css, form->status_inner), true);
            return;
        }
        send_json(res, send, payment_form_data_json(form->details, css,
                                                    form->config.sdk_url), true);
    }

    // initiate_secure_payment_link (routes:100-133) ->
    // initiate_secure_payment_link_flow (core:362-470). Renders a GenericLinks
    // form (SecurePaymentLink) or the status page, after header validation.
    void handle_initiate_secure_payment_link(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ids = path_merchant_payment(req);
        const std::string merchant_id = ids.first;
        const std::string payment_id = ids.second;

        // MerchantIdAuth(merchant_id) from the path (routes:129).
        if (const auto denied = authorize_operation(req, "initiate_secure_payment_link", merchant_id)) {
            send_error(res, send, *denied);
            return;
        }
        if (merchant_id.empty() || payment_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST,
                                             "merchant id or payment id not found in path"));
            return;
        }

        m_context->metrics().record_flow("payment_link.initiate_secure");
        m_context->metrics().counter("payment_link.initiate_secure").increment();

        auto form = form_payment_link_data(merchant_id, payment_id);
        if (!form) {
            send_error(res, send, form.error());
            return;
        }

        // validate_secure_payment_link_render_request (core:374-378,
        // validator:12-109): allowed_domains + secure_link + iframe + origin.
        if (const auto denied = validate_secure_render_request(req, payment_id, form->config)) {
            send_error(res, send, *denied);
            return;
        }

        const std::string css = get_css_script(form->config);
        if (form->is_status) {
            // core:382-397: status page is a PaymentLinkStatus payload (not a
            // GenericLinks wrapper), even on the secure route.
            send_json(res, send, status_page_data_json(form->status, css, form->status_inner), true);
            return;
        }

        // core:398-468: SecurePaymentLinkDetails -> URL-encoded js script,
        // meta tags, sdk url, wrapped in GenericLinks with allowed_domains.
        const std::string inner = secure_payment_link_details_json(form->details, form->config);
        const std::string js = "window.__PAYMENT_DETAILS = '" + url_encode(inner) + "';";
        const std::string data = payment_form_data_json_fields(
            js, css, form->config.sdk_url, get_meta_tags_html(form->details));
        send_json(res, send, generic_links_json(
            form->config.allowed_domains, data, "en"), true);
    }

    void handle_payments_link_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "payments_link_list")) {
            send_error(res, send, *denied);
            return;
        }

        m_context->metrics().record_flow("payment_link.list");
        m_context->metrics().counter("payment_link.list").increment();

        auto response = build_list_response(req, false);
        if (!response) {
            send_error(res, send, response.error());
            return;
        }
        send_json(res, send, serde::Ser::encode_json(*response), true);
    }

    void handle_profile_payment_link_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "profile_payment_link_list")) {
            send_error(res, send, *denied);
            return;
        }

        m_context->metrics().record_flow("payment_link.profile_list");
        m_context->metrics().counter("payment_link.profile_list").increment();

        auto response = build_list_response(req, true);
        if (!response) {
            send_error(res, send, response.error());
            return;
        }
        send_json(res, send, serde::Ser::encode_json(*response), true);
    }

    // payment_link_status (routes:217-249) -> get_payment_link_status
    // (core:892-1073). Upstream always renders an IntentStatus page with
    // redirect=true plus the status-page js/css/redirection endpoint.
    void handle_payment_link_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ids = path_merchant_payment(req);
        std::string merchant_id = ids.first;
        const std::string payment_id = ids.second;
        const auto header_merchant = app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) { merchant_id = header_merchant; }

        // MerchantIdAuth(merchant_id) from the path (routes:245).
        if (const auto denied = authorize_operation(req, "payment_link_status", merchant_id)) {
            send_error(res, send, *denied);
            return;
        }
        if (payment_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST,
                                             "payment id not found in path"));
            return;
        }

        m_context->metrics().record_flow("payment_link.status");
        m_context->metrics().counter("payment_link.status").increment();

        auto page = build_status_page_data(merchant_id, payment_id);
        if (!page) {
            send_error(res, send, page.error());
            return;
        }
        send_json(res, send, status_page_data_json(page->first, page->second, page->third), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/payment_link"}
            .add_route(
                Route{"/list"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_link_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_link_list")
            )
            .add_route(
                Route{"/profile/list"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profile_payment_link_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profile_payment_link_list")
            )
            .add_route(
                Route{"/:payment_link_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_link_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_link_retrieve")
            )
            .add_route(
                Route{"/:merchant_id/:payment_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_initiate_payment_link(req, res, std::move(send));
                    })
                    .summary("Hyperswitch initiate_payment_link")
            )
            .add_route(
                Route{"/s/:merchant_id/:payment_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_initiate_secure_payment_link(req, res, std::move(send));
                    })
                    .summary("Hyperswitch initiate_secure_payment_link")
            )
            .add_route(
                Route{"/status/:merchant_id/:payment_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_link_status(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_link_status")
            );
    }

    explicit PaymentLinkRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    // ─────────────────────────────────────────────────────────────────────────
    // Upstream defaults (router/src/consts.rs:155-190,
    // common_utils/src/consts.rs:85,164; payment_link/src/consts.rs:4-8).
    // ─────────────────────────────────────────────────────────────────────────
    static constexpr std::int64_t default_session_expiry = 15 * 60;
    static constexpr std::string_view default_background_color = "#212E46";
    static constexpr std::string_view default_merchant_logo =
        "https://live.hyperswitch.io/payment-link-assets/Merchant_placeholder.png";
    static constexpr std::string_view default_sdk_layout = "tabs";
    static constexpr std::string_view default_product_img =
        "https://live.hyperswitch.io/payment-link-assets/cart_placeholder.png";
    static constexpr std::string_view default_sdk_url =
        "https://checkout.hyperswitch.io/v0/HyperLoader.js";
    static constexpr std::string_view redirection_log_path = "logs/redirection";
    static constexpr std::string_view default_unified_code = "UE_9000";
    static constexpr std::string_view default_unified_message = "Something went wrong";

    /**
     * @brief Full PaymentLinkConfig with the upstream field defaults applied.
     * Populated by get_payment_link_config_based_on_priority() substitute.
     */
    struct LinkConfig
    {
        std::string theme{default_background_color};
        std::string return_url{};
        std::string merchant_name{};
        std::string logo{default_merchant_logo};
        std::string publishable_key{};
        std::string client_secret{};
        std::string locale{"en"};
        std::string domain{"pay.hyperswitch.io"};
        std::string sdk_layout{default_sdk_layout};
        std::string sdk_url{default_sdk_url};
        std::vector<std::string> allowed_domains{};
        bool display_sdk_only{false};
        bool enabled_saved_payment_method{false};
        bool hide_card_nickname_field{false};
        bool show_card_form_by_default{true};
        bool enable_button_only_on_form_ready{false};
        bool show_merchant_name{true};
        std::optional<std::string> background_colour{};
        std::optional<std::string> payment_button_text{};
        std::optional<std::string> custom_message_for_card_terms{};
        std::optional<std::string> custom_message_for_payment_method_types{};
        std::optional<std::string> payment_button_colour{};
        std::optional<std::string> payment_button_text_colour{};
        std::optional<std::string> payment_form_header_text{};
        std::optional<std::string> payment_methods_separator_text{};
        std::optional<bool> skip_status_screen{};
        std::optional<bool> branding_visibility{};
        std::optional<std::uint32_t> redirect_delay_seconds{};
        bool require_client_secret{false};
        bool require_allowed_domains{false};
    };

    struct TimeWindow
    {
        std::string start{};
        std::string end{};
    };

    /** @brief Output of the form_payment_link_data() port (core:96-360). */
    struct FormLinkData
    {
        bool is_status{false};
        app::model::payments::PaymentLinkDetails details{};
        app::model::payments::PaymentLinkStatusDetails status{};
        // Untagged PaymentLinkStatusWrap serializes its inner value: either the
        // IntentStatus string or "expired" (core:249-267, api_models:14014-14020).
        std::string status_inner{};
        LinkConfig config{};
    };

    [[nodiscard]] core::error::AppError make_error(
        core::error::AppErrorKind kind, std::string message
    ) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    /** @brief Auth rules come from RouteAuthRegistry, one auditable row per operation. */
    [[nodiscard]] std::optional<core::error::AppError> authorize_operation(
        interfaces::io::IRequest& req, std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        return m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
    }

    /**
     * @brief MerchantIdAuth(merchant_id) substitute: bind the auth principal to
     * the path merchant id and reject a conflicting credential merchant id.
     */
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
            return make_error(core::error::AppErrorKind::FORBIDDEN,
                              "merchant id header does not match payment link path");
        }
        context.set_merchant_id(std::string{path_merchant_id});
        return m_context->authorizer().authorize_route(context, rule);
    }

    [[nodiscard]] std::int64_t now_epoch() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    [[nodiscard]] std::string iso_utc(std::int64_t epoch_seconds) const
    {
        constexpr std::int64_t seconds_per_day{86400};
        std::int64_t days = epoch_seconds / seconds_per_day;
        std::int64_t seconds_of_day = epoch_seconds % seconds_per_day;
        if (seconds_of_day < 0) {
            seconds_of_day += seconds_per_day;
            --days;
        }
        days += 719468;
        const std::int64_t era = (days >= 0 ? days : days - 146096) / 146097;
        const std::int64_t doe = days - era * 146097;
        const std::int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
        std::int64_t year = yoe + era * 400;
        const std::int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        const std::int64_t mp = (5 * doy + 2) / 153;
        const std::int64_t day = doy - (153 * mp + 2) / 5 + 1;
        const std::int64_t month = mp < 10 ? mp + 3 : mp - 9;
        year += (month <= 2) ? 1 : 0;
        return std::format(
            "{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}Z",
            year, month, day,
            seconds_of_day / 3600,
            (seconds_of_day % 3600) / 60,
            seconds_of_day % 60
        );
    }

    [[nodiscard]] std::optional<std::int64_t> iso_to_epoch(std::string_view value) const
    {
        if (value.size() < 10) { return std::nullopt; }
        int year{};
        int month{};
        int day{};
        if (std::from_chars(value.data(), value.data() + 4, year).ec != std::errc{}
            || value[4] != '-') { return std::nullopt; }
        if (std::from_chars(value.data() + 5, value.data() + 7, month).ec != std::errc{}
            || value[7] != '-') { return std::nullopt; }
        if (std::from_chars(value.data() + 8, value.data() + 10, day).ec != std::errc{}) {
            return std::nullopt;
        }
        year -= (month <= 2) ? 1 : 0;
        const std::int64_t era = (year >= 0 ? year : year - 399) / 400;
        const std::int64_t yoe = year - era * 400;
        const std::int64_t doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
        const std::int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
        const std::int64_t days = era * 146097 + doe - 719468;
        return days * 86400;
    }

    [[nodiscard]] std::string trim(std::string_view value) const
    {
        std::size_t begin{0};
        std::size_t end{value.size()};
        while (begin < end && std::isspace(static_cast<unsigned char>(value[begin]))) { ++begin; }
        while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) { --end; }
        return std::string{value.substr(begin, end - begin)};
    }

    [[nodiscard]] std::size_t parse_size(std::string_view value) const
    {
        std::size_t parsed{};
        const auto converted = std::from_chars(value.data(), value.data() + value.size(), parsed);
        return converted.ec == std::errc{} && converted.ptr == value.data() + value.size()
            ? parsed : 0U;
    }

    [[nodiscard]] bool parse_bool(std::string_view value, bool fallback) const
    {
        std::string lower{value};
        for (char& c : lower) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
        if (lower == "true" || lower == "1") { return true; }
        if (lower == "false" || lower == "0") { return false; }
        return fallback;
    }

    /** @brief Upstream list defaults to 30 days, rejects end<=start, caps at 90 days. */
    [[nodiscard]] std::expected<TimeWindow, core::error::AppError> time_window(
        std::optional<std::string> raw
    ) const
    {
        const auto now = now_epoch();
        std::int64_t start = now - 30 * 86400;
        std::int64_t end = now;
        if (raw) {
            std::string value = trim(*raw);
            if (value.size() >= 2 && value.front() == '[' && value.back() == ']') {
                value = value.substr(1, value.size() - 2);
            }
            const std::size_t comma = value.find(',');
            if (comma == std::string::npos) {
                return std::unexpected{make_error(core::error::AppErrorKind::INVALID_REQUEST, "invalid time range")};
            }
            const auto parsed_start = iso_to_epoch(trim(value.substr(0, comma)));
            const auto parsed_end = iso_to_epoch(trim(value.substr(comma + 1)));
            if (!parsed_start || !parsed_end) {
                return std::unexpected{make_error(core::error::AppErrorKind::INVALID_REQUEST, "invalid time range")};
            }
            start = *parsed_start;
            end = *parsed_end;
        }
        if (start >= end) {
            return std::unexpected{make_error(core::error::AppErrorKind::INVALID_REQUEST,
                                              "time range end must be after start")};
        }
        if (end - start > 90 * 86400) {
            return std::unexpected{make_error(core::error::AppErrorKind::INVALID_REQUEST,
                                              "time range exceeds 90 days")};
        }
        return TimeWindow{iso_utc(start), iso_utc(end)};
    }

    [[nodiscard]] bool within_range(std::string_view created, const TimeWindow& window) const noexcept
    {
        if (created.empty()) { return true; }
        return created >= window.start && created <= window.end;
    }

    [[nodiscard]] std::string metadata_value(std::string_view metadata, std::string_view key) const
    {
        const std::string marker{std::string{key} + "="};
        const std::size_t pos = metadata.find(marker);
        if (pos == std::string_view::npos) { return {}; }
        const std::size_t begin = pos + marker.size();
        std::size_t end = metadata.find(';', begin);
        if (end == std::string_view::npos) { end = metadata.find(',', begin); }
        if (end == std::string_view::npos) { end = metadata.size(); }
        return std::string{metadata.substr(begin, end - begin)};
    }

    [[nodiscard]] std::string config_value(
        std::string_view config_namespace,
        std::string_view key,
        std::string_view context,
        std::string_view fallback
    ) const
    {
        if (const auto record = m_context->superposition().resolve(config_namespace, key, context)) {
            if (!record->get_value().empty()) { return record->get_value(); }
        }
        if (const auto value = m_context->config_store().get_value(config_namespace, key)) {
            if (!value->empty()) { return *value; }
        }
        return std::string{fallback};
    }

    /**
     * @brief get_payment_link_config_based_on_priority() + extract_payment_link_config()
     * substitute (core:96-169,699-859). Priority is payment-create metadata >
     * profile scope > merchant scope > default config > built-in upstream
     * default. Config store / superposition / merchant account are the local
     * stand-ins for state.conf, business_profile and payment_link_config JSON.
     */
    [[nodiscard]] LinkConfig resolve_config(
        std::string_view merchant_id,
        std::string_view profile_id,
        std::string_view payment_id,
        std::string_view metadata
    ) const
    {
        const std::string merchant_context = merchant_id.empty() ? "default" : std::string{merchant_id};
        const std::string scoped_context = profile_id.empty()
            ? merchant_context
            : std::string{merchant_id} + "/" + std::string{profile_id};
        const auto pick = [&](std::string_view key, std::string_view fallback)
        {
            std::string value = metadata_value(metadata, key);
            if (value.empty()) { value = config_value("payment_link", key, scoped_context, ""); }
            if (value.empty()) { value = config_value("payment_link", key, merchant_context, ""); }
            if (value.empty()) { value = config_value("payment_link", key, "default", ""); }
            return value.empty() ? std::string{fallback} : value;
        };

        LinkConfig config{};
        config.theme = pick("theme", std::string{default_background_color});
        config.logo = pick("logo", std::string{default_merchant_logo});
        config.locale = pick("locale", "en");
        config.sdk_layout = pick("sdk_layout", std::string{default_sdk_layout});
        config.domain = pick("domain_name", "pay.hyperswitch.io");
        config.sdk_url = config_value("payment_link", "sdk_url", "default",
                                      std::string{default_sdk_url});
        config.return_url = pick("return_url", "");
        config.display_sdk_only = parse_bool(pick("display_sdk_only", "false"), false);
        config.enabled_saved_payment_method = parse_bool(pick("enabled_saved_payment_method", "false"), false);
        config.hide_card_nickname_field = parse_bool(pick("hide_card_nickname_field", "false"), false);
        config.show_card_form_by_default = parse_bool(pick("show_card_form_by_default", "true"), true);
        config.enable_button_only_on_form_ready = parse_bool(pick("enable_button_only_on_form_ready", "false"), false);
        config.show_merchant_name = parse_bool(pick("show_merchant_name", "true"), true);
        config.require_client_secret = parse_bool(pick("require_client_secret", "false"), false);
        config.require_allowed_domains = parse_bool(pick("require_allowed_domains", "false"), false);

        const auto optional_string = [&](std::string_view key) -> std::optional<std::string>
        {
            const std::string value = pick(key, "");
            return value.empty() ? std::nullopt : std::optional<std::string>{value};
        };
        config.background_colour = optional_string("background_colour");
        config.payment_button_text = optional_string("payment_button_text");
        config.custom_message_for_card_terms = optional_string("custom_message_for_card_terms");
        config.custom_message_for_payment_method_types = optional_string("custom_message_for_payment_method_types");
        config.payment_button_colour = optional_string("payment_button_colour");
        config.payment_button_text_colour = optional_string("payment_button_text_colour");
        config.payment_form_header_text = optional_string("payment_form_header_text");
        config.payment_methods_separator_text = optional_string("payment_methods_separator_text");
        const std::string skip = pick("skip_status_screen", "");
        if (!skip.empty()) { config.skip_status_screen = parse_bool(skip, false); }
        const std::string branding = pick("branding_visibility", "");
        if (!branding.empty()) { config.branding_visibility = parse_bool(branding, true); }
        const std::string delay = pick("redirect_delay_seconds", "");
        if (!delay.empty()) { config.redirect_delay_seconds = static_cast<std::uint32_t>(parse_size(delay)); }

        const std::string domains = pick("allowed_domains", "");
        std::size_t begin{0};
        while (begin <= domains.size()) {
            const std::size_t comma = domains.find(',', begin);
            const std::string part = trim(std::string_view{domains}.substr(
                begin, comma == std::string::npos ? std::string_view::npos : comma - begin
            ));
            if (!part.empty()) { config.allowed_domains.push_back(part); }
            if (comma == std::string::npos) { break; }
            begin = comma + 1;
        }

        const auto account = m_context->repository().find_merchant_account_by_string(merchant_id);
        if (account) {
            if (!account->get_merchant_name().empty()) {
                config.merchant_name = account->get_merchant_name();
            }
            if (!account->get_return_url().empty() && config.return_url.empty()) {
                config.return_url = account->get_return_url();
            }
            if (!account->get_publishable_key().empty()) {
                config.publishable_key = account->get_publishable_key();
            }
        }
        if (config.publishable_key.empty()) {
            if (auto connector = m_context->connector_router().select(merchant_id)) {
                config.publishable_key = "pk_" + std::string{connector->name()};
            }
        }
        if (config.publishable_key.empty()) {
            config.publishable_key = "pk_test_" + std::string{merchant_id};
        }

        std::string secret_key{"hyperswitch-payment-link-client-secret"};
        if (const auto resolved = m_context->secret_resolver().resolve("payment_link_client_secret", "default")) {
            if (!resolved->empty()) { secret_key = *resolved; }
        }
        config.client_secret = "cs_" + app::crypto::KeyedHasher{std::move(secret_key)}
            .hash_with_domain(app::crypto::HashDomain::ClientSecret,
                              std::string{merchant_id} + ":" + std::string{payment_id});
        return config;
    }

    /**
     * @brief ValidateXSSOrSQLi substitute over the resolved config (core:850-856).
     * The real engine is a shared common_utils module; this heuristic covers the
     * same injection vectors over every rendered config string.
     */
    [[nodiscard]] bool unsafe_text(std::string_view value) const
    {
        std::string lower{value};
        for (char& c : lower) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
        for (const std::string_view needle : {
                 "<script", "javascript:", "onerror=", "onload=", "<iframe", "<img",
                 "' or ", "\" or ", "drop table", "; drop", "--", "</style>", "expression("
             }) {
            if (lower.find(needle) != std::string::npos) { return true; }
        }
        return false;
    }

    [[nodiscard]] bool unsafe_config(const LinkConfig& config) const
    {
        for (const std::string_view value : {
                 std::string_view{config.theme}, std::string_view{config.logo},
                 std::string_view{config.return_url}, std::string_view{config.sdk_layout},
                 std::string_view{config.publishable_key}
             }) {
            if (unsafe_text(value)) { return true; }
        }
        for (const auto& domain : config.allowed_domains) {
            if (unsafe_text(domain)) { return true; }
        }
        for (const auto& optional : {
                 config.background_colour, config.payment_button_text,
                 config.custom_message_for_card_terms,
                 config.custom_message_for_payment_method_types,
                 config.payment_button_colour, config.payment_button_text_colour,
                 config.payment_form_header_text, config.payment_methods_separator_text
             }) {
            if (optional && unsafe_text(*optional)) { return true; }
        }
        return false;
    }

    [[nodiscard]] std::string capitalized(std::string value) const
    {
        if (!value.empty()) {
            value.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(value.front())));
        }
        return value;
    }

    /**
     * @brief check_payment_link_invalid_conditions() (core:221-233,874-879).
     * Upstream blocks rendering when the intent is in any of these states.
     */
    [[nodiscard]] bool terminal_status(std::string_view status) const
    {
        static constexpr std::string_view blocked[] = {
            "cancelled", "canceled", "failed", "processing", "requires_capture",
            "requires_merchant_action", "succeeded", "partially_captured",
            "requires_customer_action", "paid"
        };
        for (const auto value : blocked) {
            if (status == value) { return true; }
        }
        return false;
    }

    [[nodiscard]] bool is_expired(std::string_view raw) const
    {
        if (raw.empty()) { return false; }
        std::int64_t expiry_seconds{};
        const auto converted = std::from_chars(raw.data(), raw.data() + raw.size(), expiry_seconds);
        if (converted.ec != std::errc{} || converted.ptr != raw.data() + raw.size()) {
            return false;
        }
        return expiry_seconds < now_epoch();
    }

    /** @brief StringMajorUnitForCore substitute with zero-decimal currency handling. */
    [[nodiscard]] std::string major_unit_amount(std::int64_t amount, std::string_view currency) const
    {
        constexpr std::string_view zero_decimal[] = {"JPY", "KRW", "VND", "CLP", "ISK", "UGX"};
        bool zero_decimals = false;
        for (const auto code : zero_decimal) {
            if (code == currency) { zero_decimals = true; break; }
        }
        const bool negative = amount < 0;
        const std::int64_t magnitude = negative ? -amount : amount;
        const std::string_view sign = negative ? "-" : "";
        if (zero_decimals) {
            return std::format("{}{}", sign, magnitude);
        }
        return std::format("{}{}.{:02d}", sign, magnitude / 100, magnitude % 100);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Upstream rendering helpers (payment_link/src/js_generator.rs,
    // css_generator.rs, meta_tags.rs; core/payment_link.rs:43-52,524-541).
    // ─────────────────────────────────────────────────────────────────────────

    /** @brief urlencoding::encode: percent-encode every byte outside RFC 3986 unreserved. */
    [[nodiscard]] std::string url_encode(std::string_view value) const
    {
        static constexpr char hex[] = "0123456789ABCDEF";
        std::string out{};
        out.reserve(value.size());
        for (const unsigned char c : value) {
            const bool unreserved =
                (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~';
            if (unreserved) {
                out.push_back(static_cast<char>(c));
            } else {
                out.push_back('%');
                out.push_back(hex[c >> 4]);
                out.push_back(hex[c & 0x0F]);
            }
        }
        return out;
    }

    [[nodiscard]] std::string html_escape(std::string_view value) const
    {
        std::string out{};
        for (const char c : value) {
            switch (c) {
            case '"': out += "&quot;"; break;
            case '\'': out += "&#x27;"; break;
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            default: out.push_back(c); break;
            }
        }
        return out;
    }

    /** @brief payment_link/src/js_generator.rs:86-101. */
    template<typename T>
    [[nodiscard]] std::string get_js_script(const T& details) const
    {
        const std::string json = serde::Ser::encode_json(details);
        // convert_custom_message_keys_to_camel() is a no-op for the local DTO
        // shape (custom_message_for_payment_method_types is a string wrapper,
        // and there are no test_mode / preload_sdk_with_params fields).
        return std::format("window.__PAYMENT_DETAILS = '{}';", url_encode(json));
    }

    /**
     * @brief Upstream PaymentLinkStatusWrap is #[serde(untagged)]
     * (api_models/payments.rs:14014-14020): the wire carries the inner value,
     * not the local enum name. Rewrite the serialized `status` field locally.
     */
    [[nodiscard]] std::string replace_status_value(std::string json, std::string_view inner) const
    {
        const std::size_t pos = json.find("\"status\"");
        if (pos == std::string::npos) { return json; }
        std::size_t cursor = pos + 8;
        while (cursor < json.size() && json[cursor] != ':') { ++cursor; }
        if (cursor >= json.size()) { return json; }
        ++cursor;
        while (cursor < json.size() && std::isspace(static_cast<unsigned char>(json[cursor]))) { ++cursor; }
        if (cursor >= json.size() || json[cursor] != '"') { return json; }
        const std::size_t end = json.find('"', cursor + 1);
        if (end == std::string::npos) { return json; }
        json.replace(cursor, end - cursor + 1, serde::Ser::encode_json(std::string{inner}));
        return json;
    }

    [[nodiscard]] std::string get_status_js_script(
        const app::model::payments::PaymentLinkStatusDetails& details,
        std::string_view inner
    ) const
    {
        const std::string json = replace_status_value(serde::Ser::encode_json(details), inner);
        return std::format("window.__PAYMENT_DETAILS = '{}';", url_encode(json));
    }

    /** @brief payment_link/src/css_generator.rs:25-91. */
    [[nodiscard]] std::string sanitize_color(std::string_view raw) const
    {
        const std::string value = trim(raw);
        if (value.empty()) { return {}; }
        if (value.starts_with('#')) {
            const std::string hex = value.substr(1);
            if ((hex.size() == 3 || hex.size() == 4 || hex.size() == 6 || hex.size() == 8)
                && std::all_of(hex.begin(), hex.end(), [](unsigned char c) {
                       return std::isxdigit(c) != 0;
                   })) {
                return value;
            }
            return {};
        }
        if ((value.starts_with("rgb(") || value.starts_with("rgba(")
             || value.starts_with("hsl(") || value.starts_with("hsla("))
            && value.ends_with(')')) {
            const auto start = value.find('(');
            const std::string inner = value.substr(start + 1, value.size() - start - 2);
            const bool valid = std::all_of(inner.begin(), inner.end(), [](unsigned char c) {
                return std::isdigit(c) || c == ',' || c == '.' || c == '%' || c == ' ';
            });
            return valid ? value : std::string{};
        }
        const bool named = std::all_of(value.begin(), value.end(), [](unsigned char c) {
            return std::isalpha(c) || c == '-';
        });
        return named ? value : std::string{};
    }

    [[nodiscard]] std::string get_css_script(const LinkConfig& config) const
    {
        const std::string raw = config.background_colour.value_or(config.theme);
        const std::string color = sanitize_color(raw);
        const std::string primary = color.empty() ? std::string{"#4E6ADD"} : color;
        std::string css = std::format(":root {{\n      --primary-color: {};\n    }}", primary);
        // payment_link_ui_rules has no local store column; when one is supplied
        // via config it is emitted verbatim after the color-scheme block.
        const std::string ui_rules = config_value("payment_link", "payment_link_ui_rules",
                                                  "default", "");
        if (!ui_rules.empty()) {
            css += "\n" + ui_rules;
        }
        return css;
    }

    [[nodiscard]] std::string get_meta_tags_html(
        const app::model::payments::PaymentLinkDetails& details
    ) const
    {
        const std::string description = details.get_merchant_description()
            ? *details.get_merchant_description() : std::string{};
        return std::format(
            "<meta property=\"og:title\" content=\"Payment request from {}\"/>\n"
            "        <meta property=\"og:description\" content=\"{}\"/>",
            html_escape(details.get_merchant_name()), html_escape(description)
        );
    }

    [[nodiscard]] std::string get_redirection_log_endpoint() const
    {
        std::string base = config_value("payment_link", "base_url", "default", "");
        if (base.empty()) {
            base = m_context->config_store().get_value("application", "base_url")
                .value_or(std::string{"https://api.hyperswitch.io"});
        }
        while (base.ends_with('/')) { base.pop_back(); }
        return base + "/" + std::string{redirection_log_path};
    }

    // ─────────────────────────────────────────────────────────────────────────
    // form_payment_link_data() port (core:96-360) and status route port
    // (core:892-1073).
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] std::optional<app::store::payment_attempts::PaymentAttemptRecord>
    find_attempt(std::string_view payment_id, std::string_view merchant_id) const
    {
        for (const auto& attempt : m_context->repository().list_payment_attempts()) {
            if (attempt.get_payment_id() == payment_id
                && (merchant_id.empty() || attempt.get_merchant_id() == merchant_id)) {
                return attempt;
            }
        }
        return std::nullopt;
    }

    /**
     * @brief validate_order_details() (core:633-687). The source order details
     * live in intent metadata (`order_details=`); each entry is
     * `name|quantity|amount_minor|img`. Amounts are converted to major units,
     * product names capitalized, and a default image applied when absent.
     */
    [[nodiscard]] std::optional<std::vector<app::model::payments::OrderDetailsWithStringAmount>>
    validate_order_details(std::string_view metadata, std::string_view currency) const
    {
        const std::string raw = metadata_value(metadata, "order_details");
        if (raw.empty()) { return std::nullopt; }
        std::vector<app::model::payments::OrderDetailsWithStringAmount> out{};
        std::size_t begin{0};
        while (begin <= raw.size()) {
            std::size_t end = raw.find(';', begin);
            if (end == std::string::npos) { end = raw.size(); }
            const std::string entry = std::string{raw.substr(begin, end - begin)};
            std::vector<std::string> parts{};
            std::size_t field_begin{0};
            while (field_begin <= entry.size()) {
                const std::size_t bar = entry.find('|', field_begin);
                parts.push_back(entry.substr(field_begin,
                    bar == std::string::npos ? std::string::npos : bar - field_begin));
                if (bar == std::string::npos) { break; }
                field_begin = bar + 1;
            }
            if (parts.size() >= 3) {
                app::model::payments::OrderDetailsWithStringAmount order{};
                order.set_product_name(capitalized(parts[0]));
                order.set_quantity(static_cast<std::uint16_t>(
                    std::clamp(parse_size(parts[1]), std::size_t{0}, std::size_t{65535})));
                app::model::payments::StringMajorUnitValue amount{};
                amount.set_value(major_unit_amount(static_cast<std::int64_t>(parse_size(parts[2])), currency));
                order.set_amount(std::move(amount));
                order.set_product_img_link(parts.size() >= 4 && !parts[3].empty()
                    ? std::optional<std::string>{parts[3]}
                    : std::optional<std::string>{std::string{default_product_img}});
                out.push_back(std::move(order));
            }
            if (end == raw.size()) { break; }
            begin = end + 1;
        }
        return out.empty() ? std::nullopt : std::optional{std::move(out)};
    }

    [[nodiscard]] std::expected<FormLinkData, core::error::AppError>
    form_payment_link_data(std::string_view merchant_id, std::string_view payment_id) const
    {
        // find_payment_intent_by_payment_id_processor_merchant_id (core:105-113).
        auto intent = find_intent(payment_id);
        if (!intent || (!merchant_id.empty() && !intent->get_merchant_id().empty()
                        && intent->get_merchant_id() != merchant_id)) {
            return std::unexpected{make_error(core::error::AppErrorKind::NOT_FOUND, "payment not found")};
        }
        // get_required_value("payment_link_id") (core:115-118).
        const auto link = find_link_by_payment(merchant_id, payment_id);
        const std::string payment_link_id = link ? link->get_link_id() : std::string{payment_id};
        if (link && !link->get_merchant_id().empty()
            && link->get_merchant_id() != merchant_id) {
            return std::unexpected{make_error(core::error::AppErrorKind::NOT_FOUND, "payment link not found")};
        }

        const auto config = resolve_config(merchant_id,
                                           link ? link->get_profile_id() : std::string{},
                                           payment_link_id, intent->get_metadata());
        if (unsafe_config(config)) {
            return std::unexpected{make_error(core::error::AppErrorKind::VALIDATION_FAILED,
                                              "unsafe payment link configuration")};
        }

        // validate_sdk_requirements() (core:543-555): currency + client secret.
        const std::string currency = link && !link->get_currency().empty()
            ? link->get_currency() : intent->get_currency();
        if (currency.empty()) {
            return std::unexpected{make_error(core::error::AppErrorKind::INVALID_REQUEST, "currency")};
        }
        if (config.client_secret.empty()) {
            return std::unexpected{make_error(core::error::AppErrorKind::INVALID_REQUEST, "client_secret")};
        }

        const std::int64_t amount_minor = link ? link->get_amount() : intent->get_amount();
        const std::string amount = major_unit_amount(amount_minor, currency);
        auto order_details = validate_order_details(intent->get_metadata(), currency);

        // session_expiry = fulfilment_time.unwrap_or(created_at + DEFAULT_SESSION_EXPIRY)
        // (core:211-215).
        std::string session_expiry = link ? link->get_expires_at() : std::string{};
        if (session_expiry.empty()) {
            const auto created_epoch = iso_to_epoch(intent->get_created_at()).value_or(now_epoch());
            session_expiry = iso_utc(created_epoch + default_session_expiry);
        }

        const std::string merchant_name = capitalized(config.merchant_name);
        const bool terminal_intent = terminal_status(intent->get_status());
        const bool expired = is_expired(link ? link->get_expires_at() : std::string{});

        FormLinkData result{};
        result.config = config;

        if (terminal_intent || expired) {
            // core:246-307: status page with redirect=false.
            result.is_status = true;
            app::model::payments::PaymentLinkStatusDetails& details = result.status;
            app::model::payments::StringMajorUnitValue amount_value{};
            amount_value.set_value(amount);
            details.set_amount(std::move(amount_value));
            app::model::payments::CurrencyValue currency_value{};
            currency_value.set_value(currency);
            details.set_currency(std::move(currency_value));
            app::model::payments::PaymentIdValue id{};
            id.set_value(payment_id);
            details.set_payment_id(std::move(id));
            details.set_merchant_name(merchant_name);
            details.set_merchant_logo(config.logo);
            app::model::payments::PrimitiveDateTimeValue created{};
            created.set_value(link ? link->get_created_at() : intent->get_created_at());
            details.set_created(std::move(created));
            // core:249-267: terminal intent wins; otherwise an expired link.
            details.set_status(terminal_intent
                ? app::model::payments::PaymentLinkStatusWrap::IntentStatus
                : app::model::payments::PaymentLinkStatusWrap::PaymentLinkStatus);
            result.status_inner = terminal_intent ? intent->get_status() : std::string{"expired"};
            details.set_redirect(false);
            details.set_theme(config.theme);
            details.set_return_url(config.return_url);
            details.set_locale(config.locale);
            const auto attempt = find_attempt(payment_id, merchant_id);
            if (attempt) {
                if (!attempt->get_error_code().empty()) {
                    details.set_error_code(attempt->get_error_code());
                }
                if (!attempt->get_error_message().empty()) {
                    details.set_error_message(attempt->get_error_message());
                }
                if (!attempt->get_error_code().empty()) {
                    details.set_unified_code(attempt->get_error_code());
                }
                if (!attempt->get_error_message().empty() && !unsafe_text(attempt->get_error_message())) {
                    details.set_unified_message(attempt->get_error_message());
                }
            }
            if (expired && !terminal_intent) {
                details.set_error_code(std::string{"payment_link_expired"});
                details.set_error_message(std::string{"This payment link has expired."});
            }
            details.set_redirect_delay_seconds(config.redirect_delay_seconds);
            return result;
        }

        // core:309-353: PaymentLinkDetails.
        app::model::payments::PaymentLinkDetails& details = result.details;
        app::model::payments::StringMajorUnitValue amount_value{};
        amount_value.set_value(amount);
        details.set_amount(std::move(amount_value));
        app::model::payments::CurrencyValue currency_value{};
        currency_value.set_value(currency);
        details.set_currency(std::move(currency_value));
        details.set_pub_key(config.publishable_key);
        details.set_client_secret(config.client_secret);
        app::model::payments::PaymentIdValue id{};
        id.set_value(payment_id);
        details.set_payment_id(std::move(id));
        app::model::payments::PrimitiveDateTimeValue expiry{};
        expiry.set_value(session_expiry);
        details.set_session_expiry(std::move(expiry));
        details.set_merchant_logo(config.logo);
        details.set_return_url(config.return_url);
        details.set_merchant_name(merchant_name);
        details.set_order_details(std::move(order_details));
        details.set_max_items_visible_after_collapse(3);
        details.set_theme(config.theme);
        if (!intent->get_description().empty() && !unsafe_text(intent->get_description())) {
            details.set_merchant_description(intent->get_description());
        }
        details.set_sdk_layout(config.sdk_layout);
        details.set_display_sdk_only(config.display_sdk_only);
        details.set_hide_card_nickname_field(config.hide_card_nickname_field);
        details.set_show_card_form_by_default(config.show_card_form_by_default);
        details.set_locale(config.locale);
        details.set_branding_visibility(config.branding_visibility);
        details.set_payment_button_text(config.payment_button_text);
        details.set_skip_status_screen(config.skip_status_screen);
        details.set_custom_message_for_card_terms(config.custom_message_for_card_terms);
        details.set_payment_button_colour(config.payment_button_colour);
        details.set_background_colour(config.background_colour);
        details.set_payment_button_text_colour(config.payment_button_text_colour);
        app::model::payments::IntentStatusValue status{};
        status.set_value(intent->get_status());
        details.set_status(std::move(status));
        details.set_enable_button_only_on_form_ready(config.enable_button_only_on_form_ready);
        details.set_payment_form_header_text(config.payment_form_header_text);
        details.set_show_merchant_name(config.show_merchant_name);
        details.set_payment_methods_separator_text(config.payment_methods_separator_text);
        return result;
    }

    /** @brief get_payment_link_status() (core:892-1073) for the status route. */
    [[nodiscard]] std::expected<std::tuple<app::model::payments::PaymentLinkStatusDetails,
                                           std::string, std::string>,
                                core::error::AppError>
    build_status_page_data(std::string_view merchant_id, std::string_view payment_id) const
    {
        auto intent = find_intent(payment_id);
        if (!intent || (!merchant_id.empty() && !intent->get_merchant_id().empty()
                        && intent->get_merchant_id() != merchant_id)) {
            return std::unexpected{make_error(core::error::AppErrorKind::NOT_FOUND, "payment not found")};
        }
        const auto attempt = find_attempt(payment_id, merchant_id);
        if (!attempt) {
            return std::unexpected{make_error(core::error::AppErrorKind::NOT_FOUND, "payment not found")};
        }
        const auto link = find_link_by_payment(merchant_id, payment_id);
        const std::string payment_link_id = link ? link->get_link_id() : std::string{payment_id};

        const auto config = resolve_config(merchant_id,
                                           link ? link->get_profile_id() : std::string{},
                                           payment_link_id, intent->get_metadata());
        if (unsafe_config(config)) {
            return std::unexpected{make_error(core::error::AppErrorKind::VALIDATION_FAILED,
                                              "unsafe payment link configuration")};
        }

        // currency is required (core:978-983).
        const std::string currency = link && !link->get_currency().empty()
            ? link->get_currency() : intent->get_currency();
        if (currency.empty()) {
            return std::unexpected{make_error(core::error::AppErrorKind::INVALID_REQUEST, "currency")};
        }
        // Amount is taken from the active attempt (core:985-991).
        const std::int64_t amount_minor = attempt->get_amount() != 0
            ? attempt->get_amount() : intent->get_amount();

        const std::string unified_code = attempt->get_error_code().empty()
            ? std::string{default_unified_code} : attempt->get_error_code();
        const std::string unified_message = attempt->get_error_message().empty()
            ? std::string{default_unified_message} : attempt->get_error_message();

        app::model::payments::PaymentLinkStatusDetails details{};
        app::model::payments::StringMajorUnitValue amount{};
        amount.set_value(major_unit_amount(amount_minor, currency));
        details.set_amount(std::move(amount));
        app::model::payments::CurrencyValue currency_value{};
        currency_value.set_value(currency);
        details.set_currency(std::move(currency_value));
        app::model::payments::PaymentIdValue id{};
        id.set_value(payment_id);
        details.set_payment_id(std::move(id));
        details.set_merchant_name(capitalized(config.merchant_name));
        details.set_merchant_logo(config.logo);
        app::model::payments::PrimitiveDateTimeValue created{};
        created.set_value(link ? link->get_created_at() : intent->get_created_at());
        details.set_created(std::move(created));
        // core:1041-1061: the status route always renders IntentStatus/redirect=true.
        details.set_status(app::model::payments::PaymentLinkStatusWrap::IntentStatus);
        details.set_redirect(true);
        if (!attempt->get_error_code().empty()) { details.set_error_code(attempt->get_error_code()); }
        if (!attempt->get_error_message().empty()) { details.set_error_message(attempt->get_error_message()); }
        details.set_theme(config.theme);
        details.set_return_url(config.return_url);
        details.set_locale(config.locale);
        details.set_unified_code(unified_code);
        details.set_unified_message(unified_message);
        details.set_redirect_delay_seconds(config.redirect_delay_seconds);

        return std::tuple{std::move(details), get_css_script(config), intent->get_status()};
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Upstream ApplicationResponse wire shapes.
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] std::string payment_form_data_json_fields(
        std::string_view js, std::string_view css,
        std::string_view sdk_url, std::string_view meta_tags
    ) const
    {
        // payment_link::PaymentLinkFormData (types.rs:11-17).
        return std::format(
            R"({{"js_script":{},"css_script":{},"sdk_url":{},"html_meta_tags":{},"redirection_log_endpoint":{}}})",
            serde::Ser::encode_json(std::string{js}),
            serde::Ser::encode_json(std::string{css}),
            serde::Ser::encode_json(std::string{sdk_url}),
            serde::Ser::encode_json(std::string{meta_tags}),
            serde::Ser::encode_json(get_redirection_log_endpoint())
        );
    }

    template<typename T>
    [[nodiscard]] std::string payment_form_data_json(
        const T& details, std::string_view css, std::string_view sdk_url
    ) const
    {
        return payment_form_data_json_fields(
            get_js_script(details), css, sdk_url, get_meta_tags_html(details));
    }

    [[nodiscard]] std::string status_page_data_json(
        const app::model::payments::PaymentLinkStatusDetails& details,
        std::string_view css,
        std::string_view status_inner
    ) const
    {
        // payment_link::PaymentLinkStatusData (types.rs:20-24).
        return std::format(
            R"({{"js_script":{},"css_script":{},"redirection_log_endpoint":{}}})",
            serde::Ser::encode_json(get_status_js_script(details, status_inner)),
            serde::Ser::encode_json(std::string{css}),
            serde::Ser::encode_json(get_redirection_log_endpoint())
        );
    }

    /**
     * @brief SecurePaymentLinkDetails wire shape (api_models; core:399-421).
     * Serialized, URL-encoded and placed into the secure js script by the caller.
     */
    [[nodiscard]] std::string secure_payment_link_details_json(
        const app::model::payments::PaymentLinkDetails& d,
        const LinkConfig& config
    ) const
    {
        const auto opt_string = [](const std::optional<std::string>& value)
        {
            return value ? serde::Ser::encode_json(*value) : std::string{"null"};
        };
        const auto opt_bool = [](const std::optional<bool>& value)
        {
            return value ? std::string{*value ? "true" : "false"} : std::string{"null"};
        };
        return std::format(
            R"({{"enabled_saved_payment_method":{},"hide_card_nickname_field":{},"show_card_form_by_default":{},"payment_link_details":{},"payment_button_text":{},"skip_status_screen":{},"custom_message_for_card_terms":{},"custom_message_for_payment_method_types":{},"payment_button_colour":{},"payment_button_text_colour":{},"background_colour":{},"sdk_ui_rules":null,"enable_button_only_on_form_ready":{},"payment_form_header_text":{},"payment_form_label_type":null,"show_card_terms":null,"color_icon_card_cvc_error":null,"payment_methods_separator_text":{}}})",
            config.enabled_saved_payment_method ? "true" : "false",
            config.hide_card_nickname_field ? "true" : "false",
            config.show_card_form_by_default ? "true" : "false",
            serde::Ser::encode_json(d),
            opt_string(config.payment_button_text),
            opt_bool(config.skip_status_screen),
            opt_string(config.custom_message_for_card_terms),
            opt_string(config.custom_message_for_payment_method_types),
            opt_string(config.payment_button_colour),
            opt_string(config.payment_button_text_colour),
            opt_string(config.background_colour),
            config.enable_button_only_on_form_ready ? "true" : "false",
            opt_string(config.payment_form_header_text),
            opt_string(config.payment_methods_separator_text)
        );
    }

    [[nodiscard]] std::string generic_links_json(
        const std::vector<std::string>& allowed_domains,
        std::string_view data_json,
        std::string_view locale
    ) const
    {
        std::string domains{"["};
        for (std::size_t i{0}; i < allowed_domains.size(); ++i) {
            if (i != 0) { domains.push_back(','); }
            domains += serde::Ser::encode_json(allowed_domains[i]);
        }
        domains.push_back(']');
        return std::format(
            R"({{"allowed_domains":{},"data":{},"locale":{}}})",
            domains, data_json, serde::Ser::encode_json(std::string{locale})
        );
    }

    // ─────────────────────────────────────────────────────────────────────────
    // validator::validate_secure_payment_link_render_request (validator:12-109).
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] std::string host_from_url(std::string_view url) const
    {
        const auto scheme = url.find("://");
        std::size_t start{0};
        if (scheme != std::string_view::npos) { start = scheme + 3; }
        std::size_t end = url.find('/', start);
        if (end == std::string_view::npos) { end = url.size(); }
        std::string authority{url.substr(start, end - start)};
        const auto at = authority.rfind('@');
        if (at != std::string::npos) { authority.erase(0, at + 1); }
        return authority;
    }

    [[nodiscard]] std::optional<core::error::AppError> validate_secure_render_request(
        interfaces::io::IRequest& req,
        std::string_view payment_link_id,
        const LinkConfig& config
    ) const
    {
        // allowed_domains must be present and non-empty (validator:18-24,
        // core:434-453).
        if (config.allowed_domains.empty()) {
            return make_error(core::error::AppErrorKind::INVALID_REQUEST,
                              "secure payment link was not generated for " +
                              std::string{payment_link_id} + ": missing allowed_domains");
        }
        // sec-fetch-dest must be "iframe" (validator:33-54).
        const app::http::HeaderReader read{};
        if (read.get(req, "sec-fetch-dest") != "iframe") {
            return make_error(core::error::AppErrorKind::FORBIDDEN,
                              "access to payment_link is forbidden when sec-fetch-dest is not iframe");
        }
        // origin or referer (validator:56-95).
        std::string origin = read.get(req, "origin");
        if (origin.empty()) { origin = read.get(req, "referer"); }
        if (origin.empty()) {
            return make_error(core::error::AppErrorKind::FORBIDDEN,
                              "access to payment_link is forbidden when origin/referer is missing");
        }
        const std::string domain = host_from_url(origin);
        if (domain.empty()) {
            return make_error(core::error::AppErrorKind::FORBIDDEN,
                              "invalid origin/referer for payment_link");
        }
        for (const auto& allowed : config.allowed_domains) {
            if (domain == allowed || (allowed.starts_with('.') && domain.ends_with(allowed))) {
                return std::nullopt;
            }
        }
        return make_error(core::error::AppErrorKind::FORBIDDEN,
                          "access to payment_link is forbidden from " + domain);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Persistence / list helpers.
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] std::optional<app::store::payment_links::PaymentLinkRecord> find_link_by_id(
        std::string_view id
    ) const
    {
        if (auto record = m_context->repository().find_payment_link_by_string(id)) {
            return record;
        }
        for (const auto& record : m_context->repository().list_payment_links()) {
            if (record.get_link_id() == id
                || (!record.get_url().empty() && record.get_url().find(id) != std::string::npos)) {
                return record;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<app::store::payment_links::PaymentLinkRecord> find_link_by_payment(
        std::string_view merchant_id, std::string_view payment_id
    ) const
    {
        for (const auto& record : m_context->repository().list_payment_links()) {
            const bool matches_payment = record.get_link_id() == payment_id
                || (!record.get_url().empty() && record.get_url().find(payment_id) != std::string::npos);
            if (matches_payment
                && (merchant_id.empty() || record.get_merchant_id() == merchant_id)) {
                return record;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<app::store::payments::PaymentIntentRecord> find_intent(
        std::string_view payment_id
    ) const
    {
        if (auto record = m_context->repository().find_payment_by_string(payment_id)) {
            return record;
        }
        for (const auto& record : m_context->repository().list_payments()) {
            if (record.get_payment_id() == payment_id
                || record.get_merchant_reference_id() == payment_id) {
                return record;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string path_link_id(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        std::size_t index{0};
        while (index < segments.count() && segments.at(index).value_or("") != "payment_link") {
            ++index;
        }
        if (index < segments.count()) { ++index; }
        while (index < segments.count()) {
            const std::string token{segments.at(index).value_or("")};
            if (token.empty() || token == "list" || token == "profile"
                || token == "s" || token == "status") {
                ++index;
                continue;
            }
            return token;
        }
        if (segments.count() > 0) {
            return std::string{segments.at(segments.count() - 1).value_or("")};
        }
        return {};
    }

    [[nodiscard]] std::pair<std::string, std::string> path_merchant_payment(
        interfaces::io::IRequest& req
    ) const
    {
        const auto segments = app::http::RequestPath{}(req);
        std::size_t index{0};
        while (index < segments.count() && segments.at(index).value_or("") != "payment_link") {
            ++index;
        }
        if (index < segments.count()) { ++index; }
        while (index < segments.count()
               && (segments.at(index).value_or("") == "s"
                   || segments.at(index).value_or("") == "status")) {
            ++index;
        }
        if (index + 1 < segments.count()) {
            const std::string merchant{segments.at(index).value_or("")};
            const std::string payment{segments.at(index + 1).value_or("")};
            if (!merchant.empty() && !payment.empty()) {
                return std::pair<std::string, std::string>{merchant, payment};
            }
        }
        if (segments.count() >= 2) {
            return std::pair<std::string, std::string>{
                std::string{segments.at(segments.count() - 2).value_or("")},
                std::string{segments.at(segments.count() - 1).value_or("")}
            };
        }
        return {};
    }

    [[nodiscard]] app::model::payments::RetrievePaymentLinkResponse response_from_link(
        const app::store::payment_links::PaymentLinkRecord& record,
        const LinkConfig& config
    ) const
    {
        using Response = app::model::payments::RetrievePaymentLinkResponse;
        Response item{};
        item.set_payment_link_id(record.get_link_id());
        app::model::payments::PaymentIdValue payment{};
        payment.set_value(record.get_link_id());
        item.set_payment_id(std::move(payment));
        app::model::payments::MerchantIdValue merchant{};
        merchant.set_value(record.get_merchant_id());
        item.set_merchant_id(std::move(merchant));
        app::model::payments::MerchantIdValue processor{};
        processor.set_value(record.get_merchant_id());
        item.set_processor_merchant_id(std::move(processor));
        if (!record.get_profile_id().empty()) {
            app::model::payments::ProfileIdValue profile{};
            profile.set_value(record.get_profile_id());
            item.set_profile_id(std::move(profile));
        }
        item.set_link_to_pay(record.get_url().empty()
            ? std::format("https://{}/{}/{}", config.domain, record.get_merchant_id(), record.get_link_id())
            : record.get_url());
        app::model::payments::MinorUnitValue amount{};
        amount.set_value(major_unit_amount(record.get_amount(), record.get_currency()));
        item.set_amount(std::move(amount));
        app::model::payments::PrimitiveDateTimeValue created{};
        created.set_value(record.get_created_at());
        item.set_created_at(std::move(created));
        if (!record.get_expires_at().empty()) {
            app::model::payments::PrimitiveDateTimeValue expiry{};
            expiry.set_value(record.get_expires_at());
            item.set_expiry(std::move(expiry));
        }
        if (!record.get_currency().empty()) {
            app::model::payments::CurrencyValue currency{};
            currency.set_value(record.get_currency());
            item.set_currency(std::move(currency));
        }
        item.set_status(is_expired(record.get_expires_at())
            ? app::model::payments::PaymentLinkStatus::Expired
            : app::model::payments::PaymentLinkStatus::Active);
        item.set_secure_link("s/" + record.get_merchant_id() + "/" + record.get_link_id());
        return item;
    }

    [[nodiscard]] app::model::payments::RetrievePaymentLinkResponse response_from_intent(
        const app::store::payments::PaymentIntentRecord& record,
        const LinkConfig& config
    ) const
    {
        using Response = app::model::payments::RetrievePaymentLinkResponse;
        Response item{};
        item.set_payment_link_id(record.get_payment_id());
        app::model::payments::PaymentIdValue payment{};
        payment.set_value(record.get_payment_id());
        item.set_payment_id(std::move(payment));
        app::model::payments::MerchantIdValue merchant{};
        merchant.set_value(record.get_merchant_id());
        item.set_merchant_id(std::move(merchant));
        item.set_link_to_pay(std::format("https://{}/{}/{}", config.domain,
                                         record.get_merchant_id(), record.get_payment_id()));
        app::model::payments::MinorUnitValue amount{};
        amount.set_value(major_unit_amount(record.get_amount(), record.get_currency()));
        item.set_amount(std::move(amount));
        app::model::payments::PrimitiveDateTimeValue created{};
        created.set_value(record.get_created_at());
        item.set_created_at(std::move(created));
        if (!record.get_currency().empty()) {
            app::model::payments::CurrencyValue currency{};
            currency.set_value(record.get_currency());
            item.set_currency(std::move(currency));
        }
        if (!record.get_description().empty() && !unsafe_text(record.get_description())) {
            item.set_description(record.get_description());
        }
        item.set_status(app::model::payments::PaymentLinkStatus::Active);
        return item;
    }

    /**
     * @brief list_payment_link_by_processor_merchant_id + real total count
     * (core:557-619). Reads the dedicated link table first; payment intents
     * remain a documented fallback for links not mirrored into that table.
     */
    [[nodiscard]] std::expected<app::model::payments::PaymentLinkListResponse, core::error::AppError>
    build_list_response(interfaces::io::IRequest& req, bool profile_scoped) const
    {
        using Input = app::model::payments::PaymentLinkListConstraints;
        const auto body = req.body_as_string();
        if (body.empty()) {
            return std::unexpected{make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request")};
        }
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            return std::unexpected{make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")};
        }

        const auto query = app::http::RequestQuery{}(req);
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        std::size_t limit = parse_size(parsed->get_limit().get_value());
        std::size_t offset = parse_size(parsed->get_offset().get_value());
        if (const auto value = query.get("limit")) { limit = parse_size(*value); }
        if (const auto value = query.get("offset")) { offset = parse_size(*value); }
        if (limit == 0) { limit = 10U; }

        std::optional<std::string> raw_range = parsed->get_time_range()
            ? std::optional<std::string>{parsed->get_time_range()->get_value()}
            : std::nullopt;
        if (const auto value = query.get("time_range")) { raw_range = *value; }
        auto window = time_window(raw_range);
        if (!window) { return std::unexpected{window.error()}; }

        const std::string merchant_filter = app::http::merchant_id_from_header{}(req).empty()
            ? auth_context.merchant_id()
            : app::http::merchant_id_from_header{}(req);
        std::string profile_filter{};
        if (const auto value = query.get("profile_id")) {
            profile_filter = *value;
        } else if (profile_scoped) {
            profile_filter = auth_context.profile_id();
        }

        using Response = app::model::payments::RetrievePaymentLinkResponse;
        std::vector<Response> matched{};
        const auto links = m_context->repository().list_payment_links();
        const bool use_link_table = !links.empty();
        if (use_link_table) {
            for (const auto& record : links) {
                if (!merchant_filter.empty() && record.get_merchant_id() != merchant_filter) { continue; }
                if (!profile_filter.empty() && record.get_profile_id() != profile_filter) { continue; }
                if (!within_range(record.get_created_at(), *window)) { continue; }
                const auto config = resolve_config(record.get_merchant_id(), record.get_profile_id(),
                                                   record.get_link_id(), std::string{});
                matched.push_back(response_from_link(record, config));
            }
        } else {
            for (const auto& record : m_context->repository().list_payments()) {
                if (!merchant_filter.empty() && record.get_merchant_id() != merchant_filter) { continue; }
                if (!within_range(record.get_created_at(), *window)) { continue; }
                const auto config = resolve_config(record.get_merchant_id(), std::string{},
                                                   record.get_payment_id(), record.get_metadata());
                matched.push_back(response_from_intent(record, config));
            }
        }

        // get_total_count_of_payment_links(): real filtered count before pagination.
        const std::int64_t total_count = static_cast<std::int64_t>(matched.size());
        std::vector<Response> data{};
        data.reserve(std::min(limit, matched.size()));
        for (std::size_t index = offset; index < matched.size() && data.size() < limit; ++index) {
            data.push_back(std::move(matched[index]));
        }

        app::model::payments::PaymentLinkListResponse response{};
        response.set_size(data.size());
        response.set_total_count(total_count);
        response.set_data(std::move(data));
        return response;
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
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::payment_link
