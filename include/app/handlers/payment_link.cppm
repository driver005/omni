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

        // Persistence is authoritative: read the dedicated payment_link row by
        // its natural key. Intent records remain a documented deterministic
        // fallback for links not yet mirrored into the link table.
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
        if (unsafe_text(config.return_url) || unsafe_text(config.theme)) {
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
        // The wire field is a string wrapper; upstream emits StringMajorUnit.
        amount.set_value(major_unit_amount(amount_minor, currency));
        response.set_amount(std::move(amount));

        const std::string created = link && !link->get_created_at().empty()
            ? link->get_created_at()
            : (intent ? intent->get_created_at() : std::string{});
        app::model::payments::PrimitiveDateTimeValue created_value{};
        created_value.set_value(created);
        response.set_created_at(std::move(created_value));

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

        // check_payment_link_status(): expired OR terminal intent => Expired.
        const bool terminal = intent && terminal_status(intent->get_status());
        const bool expired = is_expired(expiry) || terminal;
        response.set_status(expired
            ? app::model::payments::PaymentLinkStatus::Expired
            : app::model::payments::PaymentLinkStatus::Active);

        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_initiate_payment_link(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "initiate_payment_link")) {
            send_error(res, send, *denied);
            return;
        }

        m_context->metrics().record_flow("payment_link.initiate");
        m_context->metrics().counter("payment_link.initiate").increment();

        using Input = app::model::payments::PaymentLinkInitiateRequest;
        const auto body = req.body_as_string();
        std::optional<Input> input{};
        if (!body.empty()) {
            auto parsed = serde::Ser::deserialize<Input>("application/json", body);
            if (!parsed) {
                send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            input = *parsed;
        }

        auto ids = path_merchant_payment(req);
        if (ids.first.empty() && input) { ids.first = input->get_merchant_id().get_value(); }
        if (ids.second.empty() && input) { ids.second = input->get_payment_id().get_value(); }
        if (ids.first.empty() || ids.second.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "merchant id or payment id not found in path"));
            return;
        }
        const std::string& merchant_id = ids.first;
        const std::string& payment_id = ids.second;

        // form_payment_link_data(): resolve config and require a persisted link
        // or payment intent before rendering the form.
        const auto link = find_link_by_payment(merchant_id, payment_id);
        auto intent = find_intent(payment_id);
        if (!link && !intent) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "payment link not found"));
            return;
        }
        const std::string metadata = intent ? intent->get_metadata() : std::string{};
        const auto config = resolve_config(merchant_id, link ? link->get_profile_id() : std::string{},
                                           payment_id, metadata);
        if (unsafe_text(config.return_url) || unsafe_text(config.theme)) {
            send_error(res, send, make_error(core::error::AppErrorKind::VALIDATION_FAILED, "unsafe payment link configuration"));
            return;
        }
        if (is_expired(link ? link->get_expires_at() : std::string{})
            || (intent && terminal_status(intent->get_status()))) {
            send_error(res, send, make_error(core::error::AppErrorKind::PRECONDITION_FAILED, "payment link is no longer active"));
            return;
        }

        const std::string full_link = std::format("https://{}/{}/{}", config.domain, merchant_id, payment_id);
        app::model::payments::PaymentLinkResponse response{};
        response.set_payment_link_id(payment_id);
        response.set_link(full_link);

        // Ephemeral render state only; business facts stay in the repository.
        const auto cache_key = "payment_link.initiate:" + merchant_id + ":" + payment_id;
        m_context->cache().put(cache_key, core::cache::to_cache_value(full_link));
        m_context->cache().put(cache_key + ".theme", core::cache::to_cache_value(config.theme));
        m_context->cache().put(cache_key + ".client_secret", core::cache::to_cache_value(config.client_secret));

        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_initiate_secure_payment_link(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "initiate_secure_payment_link")) {
            send_error(res, send, *denied);
            return;
        }

        m_context->metrics().record_flow("payment_link.initiate_secure");
        m_context->metrics().counter("payment_link.initiate_secure").increment();

        using Input = app::model::payments::PaymentLinkInitiateRequest;
        const auto body = req.body_as_string();
        std::optional<Input> input{};
        if (!body.empty()) {
            auto parsed = serde::Ser::deserialize<Input>("application/json", body);
            if (!parsed) {
                send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            input = *parsed;
        }

        auto ids = path_merchant_payment(req);
        if (ids.first.empty() && input) { ids.first = input->get_merchant_id().get_value(); }
        if (ids.second.empty() && input) { ids.second = input->get_payment_id().get_value(); }
        if (ids.first.empty() || ids.second.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "merchant id or payment id not found in path"));
            return;
        }
        const std::string& merchant_id = ids.first;
        const std::string& payment_id = ids.second;

        const auto link = find_link_by_payment(merchant_id, payment_id);
        auto intent = find_intent(payment_id);
        if (!link && !intent) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "payment link not found"));
            return;
        }
        const std::string metadata = intent ? intent->get_metadata() : std::string{};
        const auto config = resolve_config(merchant_id, link ? link->get_profile_id() : std::string{},
                                           payment_id, metadata);
        if (unsafe_text(config.return_url) || unsafe_text(config.theme)) {
            send_error(res, send, make_error(core::error::AppErrorKind::VALIDATION_FAILED, "unsafe payment link configuration"));
            return;
        }

        // validator::validate_secure_payment_link_render_request substitute:
        // validate SDK claim pair when present and enforce configured allowed
        // domains against the request Origin header.
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto publishable = auth_context.claim("x-publishable-key");
        const auto client_secret = auth_context.claim("x-client-secret");
        if (publishable.has_value() != client_secret.has_value()) {
            send_error(res, send, make_error(core::error::AppErrorKind::FORBIDDEN, "secure link requires publishable key and client secret"));
            return;
        }
        if (publishable && !publishable->starts_with("pk_")) {
            send_error(res, send, make_error(core::error::AppErrorKind::FORBIDDEN, "invalid publishable key"));
            return;
        }
        if (client_secret && *client_secret != std::string_view{config.client_secret}) {
            send_error(res, send, make_error(core::error::AppErrorKind::FORBIDDEN, "client secret mismatch"));
            return;
        }
        if (config.require_allowed_domains && config.allowed_domains.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::PRECONDITION_FAILED, "no allowed domains configured"));
            return;
        }
        const auto origin = req.find_header("origin");
        if (!origin.empty() && !config.allowed_domains.empty()) {
            bool allowed = false;
            for (const auto& domain : config.allowed_domains) {
                if (!domain.empty() && origin.find(domain) != std::string_view::npos) {
                    allowed = true;
                    break;
                }
            }
            if (!allowed) {
                send_error(res, send, make_error(core::error::AppErrorKind::FORBIDDEN, "origin not allowed"));
                return;
            }
        }

        const std::string link_url = std::format("https://{}/s/{}/{}", config.domain, merchant_id, payment_id);
        app::model::payments::PaymentLinkResponse response{};
        response.set_payment_link_id(payment_id);
        response.set_link(link_url);
        response.set_secure_link(std::string{"s/"} + merchant_id + "/" + payment_id);

        const auto cache_key = "payment_link.secure:" + merchant_id + ":" + payment_id;
        m_context->cache().put(cache_key, core::cache::to_cache_value(link_url));
        m_context->cache().put(cache_key + ".client_secret", core::cache::to_cache_value(config.client_secret));

        send_json(res, send, serde::Ser::encode_json(response), true);
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

    void handle_payment_link_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "payment_link_status")) {
            send_error(res, send, *denied);
            return;
        }

        m_context->metrics().record_flow("payment_link.status");
        m_context->metrics().counter("payment_link.status").increment();

        using Input = app::model::payments::PaymentLinkInitiateRequest;
        const auto body = req.body_as_string();
        std::optional<Input> input{};
        if (!body.empty()) {
            auto parsed = serde::Ser::deserialize<Input>("application/json", body);
            if (!parsed) {
                send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            input = *parsed;
        }

        auto ids = path_merchant_payment(req);
        if (ids.first.empty() && input) { ids.first = input->get_merchant_id().get_value(); }
        if (ids.second.empty() && input) { ids.second = input->get_payment_id().get_value(); }
        if (ids.second.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "payment id not found in path"));
            return;
        }
        const std::string merchant_id = ids.first.empty() ? app::http::merchant_id_from_header{}(req) : ids.first;
        const std::string& payment_id = ids.second;

        const auto link = find_link_by_payment(merchant_id, payment_id);
        const auto intent = find_intent(payment_id);
        if (!link && !intent) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }
        const std::string owner = link ? link->get_merchant_id()
            : (intent ? intent->get_merchant_id() : merchant_id);
        const std::string merchant_from_credential = app::http::merchant_id_from_header{}(req);
        if (!merchant_from_credential.empty() && !owner.empty() && owner != merchant_from_credential) {
            send_error(res, send, make_error(core::error::AppErrorKind::FORBIDDEN, "payment belongs to another merchant"));
            return;
        }

        const std::string metadata = intent ? intent->get_metadata() : std::string{};
        const auto config = resolve_config(owner, link ? link->get_profile_id() : std::string{},
                                           payment_id, metadata);
        if (unsafe_text(config.return_url) || unsafe_text(config.theme)) {
            send_error(res, send, make_error(core::error::AppErrorKind::VALIDATION_FAILED, "unsafe payment link configuration"));
            return;
        }

        const std::string currency = link && !link->get_currency().empty()
            ? link->get_currency()
            : (intent ? intent->get_currency() : std::string{});
        const std::int64_t amount_minor = link ? link->get_amount()
            : (intent ? intent->get_amount() : 0);

        // Active-attempt lookup, terminal-state detection and redirect decision
        // from get_payment_link_status(): terminal/expired => PaymentLinkStatus
        // with redirect=false, otherwise IntentStatus with redirect=true.
        std::string attempt_error_code{};
        std::string attempt_error_message{};
        for (const auto& attempt : m_context->repository().list_payment_attempts()) {
            if (attempt.get_payment_id() == payment_id) {
                if (!attempt.get_error_code().empty()) { attempt_error_code = attempt.get_error_code(); }
                if (!attempt.get_error_message().empty()) { attempt_error_message = attempt.get_error_message(); }
            }
        }

        const bool expired = is_expired(link ? link->get_expires_at() : std::string{});
        const bool terminal = (intent && terminal_status(intent->get_status())) || expired;
        app::model::payments::PaymentLinkStatusDetails response{};
        app::model::payments::PaymentIdValue payment{};
        payment.set_value(payment_id);
        response.set_payment_id(std::move(payment));
        app::model::payments::StringMajorUnitValue amount{};
        amount.set_value(major_unit_amount(amount_minor, currency));
        response.set_amount(std::move(amount));
        app::model::payments::CurrencyValue currency_value{};
        currency_value.set_value(currency);
        response.set_currency(std::move(currency_value));
        const std::string created = link && !link->get_created_at().empty()
            ? link->get_created_at()
            : (intent ? intent->get_created_at() : std::string{});
        app::model::payments::PrimitiveDateTimeValue created_value{};
        created_value.set_value(created);
        response.set_created(std::move(created_value));
        response.set_status(terminal
            ? app::model::payments::PaymentLinkStatusWrap::PaymentLinkStatus
            : app::model::payments::PaymentLinkStatusWrap::IntentStatus);
        response.set_redirect(!terminal);
        response.set_merchant_name(capitalized(config.merchant_name));
        response.set_merchant_logo(config.logo);
        response.set_theme(config.theme);
        response.set_return_url(config.return_url);
        response.set_locale(config.locale);
        if (expired) {
            response.set_error_code(std::string{"payment_link_expired"});
            response.set_error_message(std::string{"This payment link has expired."});
        }
        if (!attempt_error_code.empty()) {
            response.set_unified_code(std::move(attempt_error_code));
        }
        if (!attempt_error_message.empty() && !unsafe_text(attempt_error_message)) {
            response.set_unified_message(std::move(attempt_error_message));
        }

        send_json(res, send, serde::Ser::encode_json(response), true);
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
    /** @brief Resolved link configuration with the upstream priority order applied. */
    struct LinkConfig
    {
        std::string theme{"light"};
        std::string return_url{};
        std::string merchant_name{"merchant"};
        std::string logo{};
        std::string publishable_key{};
        std::string client_secret{};
        std::string locale{"en"};
        std::string domain{"pay.hyperswitch.io"};
        std::vector<std::string> allowed_domains{};
        bool require_client_secret{false};
        bool require_allowed_domains{false};
    };

    struct TimeWindow
    {
        std::string start{};
        std::string end{};
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
            return std::unexpected{make_error(core::error::AppErrorKind::INVALID_REQUEST, "time range end must be after start")};
        }
        if (end - start > 90 * 86400) {
            return std::unexpected{make_error(core::error::AppErrorKind::INVALID_REQUEST, "time range exceeds 90 days")};
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
     * @brief get_payment_link_config_based_on_priority() substitute.
     * Priority: payment-create metadata > profile scope > merchant scope >
     * default config > built-in default. Merchant name/return URL/publishable
     * key come from the persisted merchant account, SDK/client-secret material
     * from the deterministic keyed hasher and the secret resolver.
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
        config.theme = pick("theme", "light");
        config.locale = pick("locale", "en");
        config.logo = pick("merchant_logo", "");
        config.domain = pick("domain_name", "pay.hyperswitch.io");
        config.return_url = pick("return_url", "");
        config.require_client_secret = pick("require_client_secret", "false") == "true";
        config.require_allowed_domains = pick("require_allowed_domains", "false") == "true";
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
            if (!account->get_merchant_name().empty()) { config.merchant_name = account->get_merchant_name(); }
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

    /** @brief ValidateXSSOrSQLi substitute over config and order-detail strings. */
    [[nodiscard]] bool unsafe_text(std::string_view value) const
    {
        std::string lower{value};
        for (char& c : lower) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
        for (const std::string_view needle : {
                 "<script", "javascript:", "onerror=", "onload=", "<iframe", "<img",
                 "' or ", "\" or ", "drop table", "; drop", "--"
             }) {
            if (lower.find(needle) != std::string::npos) { return true; }
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

    [[nodiscard]] bool terminal_status(std::string_view status) const
    {
        return status == "succeeded" || status == "failed" || status == "cancelled"
            || status == "canceled" || status == "paid";
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
        item.set_status(terminal_status(record.get_status())
            ? app::model::payments::PaymentLinkStatus::Expired
            : app::model::payments::PaymentLinkStatus::Active);
        return item;
    }

    /**
     * @brief list_payment_link_by_processor_merchant_id + real total count.
     * Reads the dedicated link table first; payment intents remain a documented
     * fallback because no handler yet mirrors created links into that table.
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

        // get_total_count_of_payment_links(): real filtered count, computed
        // before pagination rather than from the page size.
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
