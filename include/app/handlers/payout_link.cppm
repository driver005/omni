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

class PayoutLinkRouteHandlers
{
public:
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
        const std::string& merchant_id = path_ids->first;
        const std::string& payout_id = path_ids->second;

        // Upstream uses MerchantIdAuth(merchant_id) derived from the path. Bind the
        // auth principal to the path merchant id and use the registry rule instead of
        // the deprecated coarse TokenParser + permission check.
        if (const auto auth = authorize_operation(req, "render_payout_link", merchant_id)) {
            send_error(res, send, *auth);
            return;
        }

        m_context->metrics().record_flow("payout_link_initiate");
        m_context->metrics().counter("payout_link_render_total").increment();

        const auto payout = m_context->repository().find_payout_by_string(payout_id);
        if (!payout.has_value()) {
            send_error(res, send, error(
                core::error::AppErrorKind::NOT_FOUND, "payout not found"));
            return;
        }

        // No payout_link table exists in the local store. The link identity and its
        // status are persisted in the repository-backed configs table instead of the
        // cache; this is the deterministic substitute for find_payout_link_by_link_id
        // plus get_required_value("payout link id"). A stored link id that disagrees
        // with the deterministic one is treated as "payout link not found".
        const std::string link_key = "payout_link.link_id." + merchant_id + ":" + payout_id;
        const std::string status_key_db = "payout_link.status." + merchant_id + ":" + payout_id;
        const auto link_id = "pl_" + std::to_string(std::hash<std::string>{}(merchant_id + ":" + payout_id));
        const auto stored_link_id = m_context->repository().get_config(link_key);
        if (stored_link_id.has_value() && *stored_link_id != link_id) {
            send_error(res, send, error(
                core::error::AppErrorKind::NOT_FOUND, "payout link not found"));
            return;
        }
        if (!stored_link_id.has_value()) {
            m_context->repository().set_config(link_key, link_id);
        }

        // Merchant account supplies publishable key / merchant name / return url.
        const auto merchant_account = m_context->repository().find_merchant_account_by_string(merchant_id);
        std::string merchant_name{"merchant"};
        std::string publishable_key{"pk_test_" + merchant_id};
        std::optional<std::string> return_url{};
        if (merchant_account.has_value()) {
            if (!merchant_account->get_merchant_name().empty()) {
                merchant_name = merchant_account->get_merchant_name();
            }
            if (!merchant_account->get_publishable_key().empty()) {
                publishable_key = merchant_account->get_publishable_key();
            }
            if (!merchant_account->get_return_url().empty()) {
                return_url = merchant_account->get_return_url();
            }
        }
        if (publishable_key == "pk_test_" + merchant_id) {
            if (auto connector = m_context->connector_router().select(merchant_id)) {
                publishable_key = "pk_" + std::string{connector->name()};
            }
        }
        const bool test_mode = publishable_key.starts_with("pk_test_");

        // Generic link UI defaults come from state.conf.generic_link.payout_link.
        std::string theme = or_default(config_value("theme"), std::string{"light"});
        const std::string logo = config_value("logo");
        std::string display_name = or_default(config_value("merchant_name"), merchant_name);
        const std::string sdk_url = config_value("sdk_url");
        m_context->metrics().counter("payout_link_config_loaded").increment();

        // Allowed domains from server config; validate the render request headers.
        const auto allowed_domains = parse_csv(config_value("allowed_domains"));
        if (const auto domain_error = validate_allowed_domains(req, test_mode, allowed_domains)) {
            send_error(res, send, *domain_error);
            return;
        }
        m_context->metrics().counter("payout_link_domain_checked").increment();

        // locale from query override, else server state, else "en".
        const auto query = app::http::RequestQuery{}(req);
        std::string locale = query.get("locale").value_or(global_config_value("locale"));
        locale = or_default(locale, std::string{"en"});

        // Link-only ephemeral values remain in the cache (TTL state, not business
        // records). Expiry/amount/currency/customer/client-secret are seeded with
        // persisted payout facts so the cache is never the authoritative source.
        const auto status_key = "payout_link.status:" + link_id;
        const auto expiry_key = "payout_link.expiry:" + link_id;
        const auto amount_key = "payout_link.amount:" + link_id;
        const auto currency_key = "payout_link.currency:" + link_id;
        const auto customer_key = "payout_link.customer:" + link_id;
        const auto payout_status_key = "payout_link.payout_status:" + link_id;
        const auto secret_key = "payout_link.client_secret:" + link_id;
        const auto error_code_key = "payout_link.error_code:" + link_id;
        const auto error_message_key = "payout_link.error_message:" + link_id;

        const auto amount_minor = payout->get_amount();
        const bool negative_amount = amount_minor < 0;
        const auto magnitude = negative_amount ? -amount_minor : amount_minor;
        const std::string amount_default = std::format(
            "{}{}.{:02d}", negative_amount ? "-" : "", magnitude / 100, magnitude % 100
        );

        const std::string currency_default = payout->get_currency().empty()
            ? std::string{"USD"} : payout->get_currency();

        // Customer fetch: authoritative id comes from the persisted customer record.
        // Address: no address_id on PayoutRecord and no address entity/table exists,
        // so the required-field override is derived from customer metadata.
        std::string customer_id = payout->get_customer_id();
        bool has_address{false};
        const auto customer = find_customer(customer_id);
        if (customer.has_value()) {
            if (!customer->get_customer_id().empty()) {
                customer_id = customer->get_customer_id();
            }
            has_address = customer->get_metadata().find("address") != std::string::npos;
        }
        if (customer_id.empty()) {
            customer_id = payout_id;
        }
        m_context->metrics().counter("payout_link_customer_loaded").increment();

        const std::string payout_status_default = payout->get_status().empty()
            ? std::string{"pending"} : payout->get_status();
        const bool payout_not_confirmed = payout_status_default == "pending"
            || payout_status_default == "initiated"
            || payout_status_default == "requires_confirmation";
        const std::string link_status_default{payout_not_confirmed ? "initiated" : "submitted"};

        std::string status{link_status_default};
        if (const auto stored_status = m_context->repository().get_config(status_key_db)) {
            status = *stored_status;
        } else {
            const auto cached_status = m_context->cache().get(status_key);
            if (cached_status.hit) {
                status = cached_status.value_or<std::string>(link_status_default);
            }
        }

        const auto expiry_raw = m_context->cache().get(expiry_key).value_or<std::string>("4102444800");
        const auto amount = m_context->cache().get(amount_key).value_or<std::string>(amount_default);
        const auto currency = m_context->cache().get(currency_key).value_or<std::string>(currency_default);
        const auto cached_customer = m_context->cache().get(customer_key).value_or<std::string>(customer_id);
        if (!cached_customer.empty()) {
            customer_id = cached_customer;
        }
        const auto payout_status = m_context->cache().get(payout_status_key).value_or<std::string>(payout_status_default);

        // Payout attempt fetch + unified code/message translation. There is no local
        // translation catalog, so the attempt's unified code/message are used directly
        // (with the cache values as a fallback) — documented substitute.
        const auto attempt = find_payout_attempt(payout_id, merchant_id);
        std::string error_code = attempt.has_value() ? attempt->get_error_code() : std::string{};
        std::string error_message = attempt.has_value() ? attempt->get_error_message() : std::string{};
        if (error_code.empty()) {
            error_code = m_context->cache().get(error_code_key).value_or<std::string>("");
        }
        if (error_message.empty()) {
            error_message = m_context->cache().get(error_message_key).value_or<std::string>("");
        }
        m_context->metrics().counter("payout_link_attempt_loaded").increment();

        std::int64_t expiry_seconds{};
        const auto converted = std::from_chars(expiry_raw.data(), expiry_raw.data() + expiry_raw.size(), expiry_seconds);
        const auto now_seconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        const bool has_expired = converted.ec == std::errc{}
            && converted.ptr == expiry_raw.data() + expiry_raw.size()
            && expiry_seconds < static_cast<std::int64_t>(now_seconds);

        app::model::payouts::GenericLinkUiConfigFormDataValue ui_config{};
        ui_config.set_value(std::format(
            R"({{"merchant_name":{},"logo":{},"theme":{}}})",
            serde::Ser::encode_json(display_name),
            serde::Ser::encode_json(logo),
            serde::Ser::encode_json(theme)
        ));

        std::string text{};
        if (has_expired || status == "invalidated") {
            // Persist Invalidated through the repository (substitute for update_payout_link).
            if (status != "invalidated") {
                m_context->repository().set_config(status_key_db, std::string{"invalidated"});
            }
            const std::string expired = std::format(
                R"({{"type":"ExpiredLink","title":{},"message":{},"theme":{}}})",
                serde::Ser::encode_json(std::string{"Payout Expired"}),
                serde::Ser::encode_json(std::string{"This payout link has expired."}),
                serde::Ser::encode_json(theme)
            );
            text = generic_links(allowed_domains, expired, locale);
        } else if (status == "submitted") {
            app::model::payouts::PayoutLinkStatusDetails details{};
            details.set_payout_link_id(link_id);
            app::model::payouts::PayoutIdValue payout_value{};
            payout_value.set_value(payout_id);
            details.set_payout_id(std::move(payout_value));
            app::model::payouts::CustomerIdValue customer_value{};
            customer_value.set_value(customer_id);
            details.set_customer_id(std::move(customer_value));
            app::model::payouts::PrimitiveDateTimeValue session_expiry{};
            session_expiry.set_value(expiry_raw);
            details.set_session_expiry(std::move(session_expiry));
            if (return_url.has_value()) {
                app::model::payouts::UrlValue url_value{};
                url_value.set_value(*return_url);
                details.set_return_url(std::move(url_value));
            }
            app::model::payouts::PayoutStatusValue status_value{};
            status_value.set_value(payout_status);
            details.set_status(std::move(status_value));
            if (!error_code.empty()) {
                app::model::payouts::UnifiedCodeValue code_value{};
                code_value.set_value(error_code);
                details.set_error_code(std::move(code_value));
            }
            if (!error_message.empty()) {
                app::model::payouts::UnifiedMessageValue message_value{};
                message_value.set_value(error_message);
                details.set_error_message(std::move(message_value));
            }
            details.set_ui_config(std::move(ui_config));
            details.set_test_mode(test_mode);
            const auto inner = serde::Ser::encode_json(details);
            text = generic_links(allowed_domains, status_form_data(inner), locale);
        } else {
            // Initiated flow: build the payout-method form data from enabled MCAs,
            // config filters, default-method fallback and cards-first sorting.
            app::model::payouts::PayoutLinkDetails details{};
            app::model::payouts::SecretValue publishable{};
            publishable.set_value(publishable_key);
            details.set_publishable_key(std::move(publishable));
            app::model::payouts::SecretValue client_secret{};
            client_secret.set_value(m_context->cache().get(secret_key).value_or<std::string>(link_id));
            details.set_client_secret(std::move(client_secret));
            details.set_payout_link_id(link_id);
            app::model::payouts::PayoutIdValue payout_value{};
            payout_value.set_value(payout_id);
            details.set_payout_id(std::move(payout_value));
            app::model::payouts::CustomerIdValue customer_value{};
            customer_value.set_value(customer_id);
            details.set_customer_id(std::move(customer_value));
            app::model::payouts::PrimitiveDateTimeValue session_expiry{};
            session_expiry.set_value(expiry_raw);
            details.set_session_expiry(std::move(session_expiry));
            if (return_url.has_value()) {
                app::model::payouts::UrlValue url_value{};
                url_value.set_value(*return_url);
                details.set_return_url(std::move(url_value));
            }
            details.set_ui_config(std::move(ui_config));
            app::model::payouts::StringMajorUnitValue amount_value{};
            amount_value.set_value(amount);
            details.set_amount(std::move(amount_value));
            app::model::payouts::CurrencyValue currency_value{};
            currency_value.set_value(currency);
            details.set_currency(std::move(currency_value));
            details.set_locale(locale);
            details.set_test_mode(test_mode);
            apply_payout_methods(details, merchant_id, currency, has_address);
            const auto inner = serde::Ser::encode_json(details);
            text = generic_links(allowed_domains, link_form_data(inner, sdk_url), locale);
        }

        send_json(res, send, text, true);
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
            .get_value("generic_link.payout_link", key)
            .value_or(std::string{});
    }

    [[nodiscard]] std::string global_config_value(std::string_view key) const
    {
        return m_context->config_store()
            .get_value("generic_link", key)
            .value_or(std::string{});
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize_operation(
        interfaces::io::IRequest& req,
        std::string_view operation,
        std::string_view path_merchant_id
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        auto context = app::auth::AuthContextBuilder{}(req);

        // MerchantIdAuth binds the caller to the path merchant id. When the header
        // merchant id is present it must match the path; the auth principal is then
        // the path id so authorize_route applies HEADER_MERCHANT to the right subject.
        if (!context.merchant_id().empty()
            && context.merchant_id() != std::string{path_merchant_id}) {
            return error(
                core::error::AppErrorKind::FORBIDDEN,
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
            if (!value.empty()) {
                out.push_back(std::move(value));
            }
            current.clear();
        };
        for (const char c : raw) {
            if (c == ',') {
                push();
            } else {
                current.push_back(c);
            }
        }
        push();
        return out;
    }

    [[nodiscard]] std::string host_from_url(std::string_view url) const
    {
        const auto scheme = url.find("://");
        std::size_t start{0};
        if (scheme != std::string_view::npos) {
            start = scheme + 3;
        }
        std::size_t end = url.find('/', start);
        if (end == std::string_view::npos) {
            end = url.size();
        }
        std::string authority{url.substr(start, end - start)};
        const auto at = authority.rfind('@');
        if (at != std::string::npos) {
            authority.erase(0, at + 1);
        }
        return authority;
    }

    [[nodiscard]] std::optional<core::error::AppError> validate_allowed_domains(
        interfaces::io::IRequest& req,
        bool test_mode,
        const std::vector<std::string>& allowed_domains
    ) const
    {
        // Upstream skips all checks in test mode. Locally an empty configured list
        // is the equivalent skip, so default deployments keep their existing behavior.
        if (test_mode || allowed_domains.empty()) {
            return std::nullopt;
        }
        const app::http::HeaderReader read{};
        if (read.get(req, "sec-fetch-dest") != "iframe") {
            return error(core::error::AppErrorKind::FORBIDDEN, "payout_link access forbidden");
        }
        std::string origin = read.get(req, "origin");
        if (origin.empty()) {
            origin = read.get(req, "referer");
        }
        if (origin.empty()) {
            return error(core::error::AppErrorKind::FORBIDDEN, "payout_link origin missing");
        }
        const std::string domain = host_from_url(origin);
        if (domain.empty()) {
            return error(core::error::AppErrorKind::FORBIDDEN, "payout_link origin invalid");
        }
        for (const auto& allowed : allowed_domains) {
            if (domain == allowed || (allowed.starts_with('.') && domain.ends_with(allowed))) {
                return std::nullopt;
            }
        }
        return error(core::error::AppErrorKind::FORBIDDEN, "payout_link domain not allowed");
    }

    [[nodiscard]] std::optional<app::store::payout_attempts::PayoutAttemptRecord> find_payout_attempt(
        std::string_view payout_id,
        std::string_view merchant_id
    ) const
    {
        for (const auto& attempt : m_context->repository().list_payout_attempts()) {
            if (attempt.get_payout_id() == std::string{payout_id}
                && (merchant_id.empty() || attempt.get_merchant_id() == std::string{merchant_id})) {
                return attempt;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<app::store::customers::CustomerRecord> find_customer(
        std::string_view customer_id
    ) const
    {
        if (customer_id.empty()) {
            return std::nullopt;
        }
        return m_context->repository().find_customer_by_string(customer_id);
    }

    void collect_json_string_values(
        std::string_view json,
        std::string_view key,
        std::vector<std::string>& out
    ) const
    {
        const std::string needle = std::format("\"{}\"", key);
        std::size_t cursor{0};
        while ((cursor = json.find(needle, cursor)) != std::string_view::npos) {
            std::size_t colon = json.find(':', cursor + needle.size());
            if (colon == std::string_view::npos) {
                break;
            }
            ++colon;
            while (colon < json.size() && std::isspace(static_cast<unsigned char>(json[colon]))) {
                ++colon;
            }
            if (colon < json.size() && json[colon] == '"') {
                const std::size_t end = json.find('"', colon + 1);
                if (end != std::string_view::npos) {
                    out.emplace_back(json.substr(colon + 1, end - colon - 1));
                    cursor = end + 1;
                    continue;
                }
            }
            ++cursor;
        }
    }

    void apply_payout_methods(
        app::model::payouts::PayoutLinkDetails& details,
        std::string_view merchant_id,
        std::string_view currency,
        bool has_address
    ) const
    {
        std::vector<std::string> pmts{};
        const auto supported_currencies = parse_csv(config_value("supported_currencies"));
        const bool currency_allowed = supported_currencies.empty()
            || std::find(supported_currencies.begin(), supported_currencies.end(), std::string{currency})
               != supported_currencies.end();

        // Enabled MCAs carry their supported methods; business_country filtering is
        // a config-driven substitute because PayoutRecord has no country field.
        if (currency_allowed) {
            for (const auto& mca : m_context->repository().list_merchant_connectors()) {
                if (mca.get_merchant_id() != std::string{merchant_id} || mca.get_disabled()) {
                    continue;
                }
                const auto enabled = mca.get_payment_methods_enabled();
                if (enabled.empty()) {
                    continue;
                }
                collect_json_string_values(enabled, "payment_method", pmts);
                collect_json_string_values(enabled, "payment_method_type", pmts);
            }
        }
        if (pmts.empty()) {
            pmts = parse_csv(config_value("enabled_payment_methods"));
        }
        if (pmts.empty()) {
            pmts = std::vector<std::string>{"card", "bank_debit", "upi"};
        }

        std::stable_sort(pmts.begin(), pmts.end(), [](const std::string& a, const std::string& b) {
            return a == "card" && b != "card";
        });
        pmts.erase(std::unique(pmts.begin(), pmts.end()), pmts.end());

        std::vector<app::model::payouts::EnabledPaymentMethodValue> enabled{};
        std::vector<app::model::payouts::PayoutEnabledPaymentMethodsInfo> with_fields{};
        for (const auto& pmt : pmts) {
            app::model::payouts::EnabledPaymentMethodValue value{};
            value.set_value(pmt);
            enabled.push_back(std::move(value));

            app::model::payouts::PaymentMethodTypeValue type{};
            type.set_value(pmt);
            app::model::payouts::RequiredFieldInfoValue required{};
            required.set_value(has_address ? std::string{"optional"} : std::string{"required"});
            std::unordered_map<std::string, app::model::payouts::RequiredFieldInfoValue> fields{};
            fields.emplace("billing_address", required);
            app::model::payouts::PaymentMethodTypeInfo type_info{};
            type_info.set_payment_method_type(std::move(type));
            type_info.set_required_fields(std::move(fields));

            app::model::payouts::PaymentMethodValue method{};
            method.set_value(pmt);
            app::model::payouts::PayoutEnabledPaymentMethodsInfo info{};
            info.set_payment_method(std::move(method));
            info.set_payment_method_types_info({std::move(type_info)});
            with_fields.push_back(std::move(info));
        }
        details.set_enabled_payment_methods(std::move(enabled));
        details.set_enabled_payment_methods_with_required_fields(std::move(with_fields));

        const auto layout = config_value("form_layout");
        if (!layout.empty()) {
            app::model::payouts::UIWidgetFormLayoutValue value{};
            value.set_value(layout);
            details.set_form_layout(std::move(value));
        }
        m_context->metrics().counter("payout_link_methods_filtered").increment();
    }

    [[nodiscard]] std::string link_form_data(std::string_view inner, std::string_view sdk_url) const
    {
        const std::string js = "window.__PAYOUT_DETAILS = " + std::string{inner};
        return std::format(
            R"({{"js_data":{},"css_data":{},"sdk_url":{},"html_meta_tags":{}}})",
            serde::Ser::encode_json(js),
            serde::Ser::encode_json(std::string{}),
            serde::Ser::encode_json(std::string{sdk_url}),
            serde::Ser::encode_json(std::string{})
        );
    }

    [[nodiscard]] std::string status_form_data(std::string_view inner) const
    {
        const std::string js = "window.__PAYOUT_DETAILS = " + std::string{inner};
        return std::format(
            R"({{"js_data":{},"css_data":{}}})",
            serde::Ser::encode_json(js),
            serde::Ser::encode_json(std::string{})
        );
    }

    [[nodiscard]] std::string generic_links(
        const std::vector<std::string>& allowed_domains,
        std::string_view data_json,
        std::string_view locale
    ) const
    {
        std::string domains{"["};
        for (std::size_t i{0}; i < allowed_domains.size(); ++i) {
            if (i != 0) {
                domains.push_back(',');
            }
            domains += serde::Ser::encode_json(allowed_domains[i]);
        }
        domains.push_back(']');
        return std::format(
            R"({{"allowed_domains":{},"data":{},"locale":{}}})",
            domains,
            data_json,
            serde::Ser::encode_json(std::string{locale})
        );
    }

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
