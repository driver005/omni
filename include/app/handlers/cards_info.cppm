module;
export module app_handlers:cards_info;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_error;
import app_auth;
import app_http;
import app_metrics;
import app_store;

export namespace app::handlers::cards_info {

class CardsInfoRouteHandlers
{
public:
    void handle_card_iin_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream routes/cards_info.rs:44-63 runs check_sdk_auth_and_get_auth (SDK
        // authorization header or api_key/publishable-key) before server_wrap. The
        // registry rule is the primary check; an SDK rule is the documented fallback
        // for the publishable-key/client-secret SDK path.
        const auto auth_context = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize_operation(auth_context, "card_iin_info", true)) {
            send_error(res, send, *auth_error);
            return;
        }

        // Upstream GET /cards/{bin}: IIN is a path segment, client_secret a query param.
        const auto segments = ::app::http::RequestPath{}(req);
        const auto bin = segments.at(1); // /cards/:bin -> "cards", "<bin>"
        if (!bin) {
            send_error(res, send, make_error(::core::error::AppErrorKind::NOT_FOUND, "InvalidCardIin"));
            return;
        }
        ::app::model::cards_info::CardsInfoRequest payload;
        payload.set_card_iin(std::string{*bin});
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto client_secret = query.get("client_secret")) {
            payload.set_client_secret(*client_secret);
        }
        // Upstream routes/cards_info.rs:55-57 overwrites req.client_secret from the
        // authenticated credential. The AuthContext carries that authenticated claim.
        if (const auto authenticated_secret = auth_context.claim(::app::auth::kClaimClientSecret)) {
            payload.set_client_secret(std::string{*authenticated_secret});
        }

        if (const auto invalid = verify_iin_length(payload.get_card_iin())) {
            send_error(res, send, *invalid);
            return;
        }
        if (const auto invalid = validate_client_secret(payload.get_client_secret())) {
            send_error(res, send, *invalid);
            return;
        }

        auto repository = m_context->repository();
        const auto stored = repository.find_card_info_by_string(payload.get_card_iin());
        if (!stored) {
            send_error(res, send, make_error(::core::error::AppErrorKind::NOT_FOUND, "InvalidCardIin"));
            return;
        }
        const auto response = response_from_store(*stored);
        const auto text = serde::Ser::encode_json(response);

        m_context->metrics().record_flow("CardsInfo");
        m_context->metrics().counter("CARDS_INFO_RETRIEVED").increment();

        send_json(res, send, text, true);
    }

    void handle_create_cards_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize_operation(auth_context, "create_cards_info")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(::core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = ::app::model::cards_info::CardInfoCreateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, make_error(::core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream create_card_info has no IIN-length check (core/cards_info.rs:56-67).
        auto repository = m_context->repository();
        if (const auto created = repository.create_card_info(to_store_record(*parsed)); !created) {
            send_error(
                res, send,
                make_error(
                    ::core::error::AppErrorKind::ALREADY_EXISTS,
                    "CardInfo with given key already exists in our records"
                )
            );
            return;
        }

        const auto response = build_response(*parsed);
        const auto text = serde::Ser::encode_json(response);

        m_context->metrics().record_flow("CardsInfoCreate");
        m_context->metrics().counter("CARDS_INFO_CREATED").increment();

        send_json(res, send, text, true);
    }

    void handle_update_cards_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize_operation(auth_context, "update_cards_info")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(::core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = ::app::model::cards_info::CardInfoUpdateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, make_error(::core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream update_card_info: update by IIN, last_updated = now(), and a
        // missing record is GenericNotFoundError (core/cards_info.rs:70-125).
        auto repository = m_context->repository();
        auto stored = repository.find_card_info_by_string(parsed->get_card_iin());
        if (!stored) {
            send_error(
                res, send,
                make_error(
                    ::core::error::AppErrorKind::NOT_FOUND,
                    "Card info with given key does not exist in our records"
                )
            );
            return;
        }
        apply_update(*stored, *parsed);
        if (!repository.update_card_info(*stored)) {
            send_error(res, send, make_error(::core::error::AppErrorKind::INTERNAL_ERROR, "Failed while updating card info"));
            return;
        }

        const auto response = build_response(*parsed);
        const auto text = serde::Ser::encode_json(response);

        m_context->metrics().record_flow("CardsInfoUpdate");
        m_context->metrics().counter("CARDS_INFO_UPDATED").increment();

        send_json(res, send, text, true);
    }

    void handle_migrate_cards_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize_operation(auth_context, "migrate_cards_info")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(::core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        std::vector<::app::model::cards_info::CardInfoUpdateRequest> records;
        if (const auto csv_bytes = multipart_file_bytes(req, body)) {
            // Upstream CardsInfoUpdateForm caps the multipart file at 1MB and a CSV
            // parse failure maps to PreconditionFailed (core/cards_info.rs:127-157).
            if (csv_bytes->size() > (1024u * 1024u)) {
                send_error(res, send, make_error(::core::error::AppErrorKind::PRECONDITION_FAILED, "cards bin CSV exceeds 1MB limit"));
                return;
            }
            auto parsed_csv = parse_cards_csv(*csv_bytes);
            if (!parsed_csv) {
                send_error(res, send, make_error(::core::error::AppErrorKind::PRECONDITION_FAILED, "failed to parse cards bin CSV"));
                return;
            }
            records = std::move(*parsed_csv);
        } else {
            using Input = std::vector<::app::model::cards_info::CardInfoUpdateRequest>;
            auto parsed = serde::Ser::deserialize<Input>("application/json", body);
            if (!parsed) {
                send_error(res, send, make_error(::core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            records = std::move(*parsed);
            // Upstream assigns a 1-based line_number from a row counter for every
            // record (core/cards_info.rs:138-143); caller input is never trusted.
            for (std::size_t i{0}; i < records.size(); ++i) {
                records[i].set_line_number(static_cast<std::int64_t>(i + 1));
            }
        }

        std::vector<::app::model::cards_info::CardInfoMigrationResponse> results;
        results.reserve(records.size());
        std::size_t migrated{0};
        auto repository = m_context->repository();
        for (const auto& record : records) {
            bool ok{false};
            std::string_view failure{};
            if (record.get_card_iin().empty()) {
                failure = "invalid card iin";
            } else if (const auto existing = repository.find_card_info_by_string(record.get_card_iin())) {
                // card_info_flow: fetch -> update when present -> build response.
                auto stored = *existing;
                apply_update(stored, record); // stamps last_updated = now()
                ok = repository.update_card_info(stored);
                if (ok) {
                    m_context->metrics().counter("CARDS_INFO_UPDATED").increment();
                } else {
                    failure = "failed to migrate card info";
                }
            } else {
                // card_info_flow: fetch -> add when absent -> build response.
                ok = repository.create_card_info(to_store_record(record)).has_value();
                if (ok) {
                    m_context->metrics().counter("CARDS_INFO_CREATED").increment();
                } else {
                    failure = "failed to migrate card info";
                }
            }
            if (ok) {
                ++migrated;
            }
            results.push_back(build_migration_response(record, ok, failure));
        }

        m_context->metrics().record_flow("CardsInfoMigrate");
        m_context->metrics().counter("CARDS_INFO_MIGRATED").increment(static_cast<std::uint64_t>(migrated));

        auto text = serde::Ser::encode_json(results);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/cards"}
            .add_route(
                Route{"/:bin"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_card_iin_info(req, res, std::move(send));
                    })
                    .summary("Hyperswitch card_iin_info")
            )
            .add_route(
                Route{"/create"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_cards_info(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_cards_info")
            )
            .add_route(
                Route{"/update"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_update_cards_info(req, res, std::move(send));
                    })
                    .summary("Hyperswitch update_cards_info")
            )
            .add_route(
                Route{"/update-batch"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_migrate_cards_info(req, res, std::move(send));
                    })
                    .summary("Hyperswitch migrate_cards_info")
            );
    }

    explicit CardsInfoRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    [[nodiscard]] ::core::error::AppError make_error(
        ::core::error::AppErrorKind kind,
        std::string message
    ) const
    {
        return ::core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    [[nodiscard]] std::string now_stamp() const
    {
        return std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
    }

    [[nodiscard]] std::optional<::core::error::AppError> verify_iin_length(
        std::string_view card_iin
    ) const
    {
        if (card_iin.size() == 6 || card_iin.size() == 8) {
            return std::nullopt;
        }
        return make_error(
            ::core::error::AppErrorKind::VALIDATION_FAILED,
            "The provided card IIN length is invalid, please provide an IIN with 6 digits"
        );
    }

    /**
     * @brief Deterministic stand-in for verify_payment_intent_time_and_client_secret
     * (core/payments/helpers.rs:5092-5135): derive the payment id, require the intent
     * to exist. Equality/expiry comparison needs payment_intent.client_secret and
     * session_expiry, which the Lane A CardInfo/payment records do not carry.
     */
    [[nodiscard]] std::optional<::core::error::AppError> validate_client_secret(
        const std::optional<std::string>& secret
    ) const
    {
        if (!secret) {
            return std::nullopt;
        }
        constexpr std::string_view separator{"_secret_"};
        const auto pos = secret->rfind("_secret_");
        if (pos == std::string::npos || pos == 0 || pos + separator.size() >= secret->size()) {
            return make_error(::core::error::AppErrorKind::FORBIDDEN, "ClientSecretInvalid");
        }
        const std::string payment_id = secret->substr(0, pos);
        if (!m_context->repository().find_payment_by_string(payment_id)) {
            return make_error(::core::error::AppErrorKind::NOT_FOUND, "PaymentNotFound");
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<::core::error::AppError> authorize_operation(
        const ::app::auth::AuthContext& context,
        std::string_view operation,
        bool sdk_fallback = false
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        auto auth = m_context->authorizer().authorize_route(context, rule);
        if (auth && sdk_fallback) {
            // routes/cards_info.rs:25,44-47: api_key (x-api-key) or publishable_key
            // with client_secret is accepted; both are real upstream auth paths.
            const auto api_key_rule = ::app::auth::RouteAuthRuleBuilder{}
                                          .scheme(::app::auth::AuthScheme::V2_API_KEY)
                                          .permission(rule.permission())
                                          .build();
            auth = m_context->authorizer().authorize_route(context, api_key_rule);
            if (auth) {
                const auto sdk_rule = ::app::auth::RouteAuthRuleBuilder{}
                                          .scheme(::app::auth::AuthScheme::SDK)
                                          .permission(rule.permission())
                                          .build();
                auth = m_context->authorizer().authorize_route(context, sdk_rule);
            }
        }
        return auth;
    }

    [[nodiscard]] ::app::store::cards_info::CardInfoRecord to_store_record(
        const ::app::model::cards_info::CardInfoCreateRequest& in
    ) const
    {
        ::app::store::cards_info::CardInfoRecord record;
        record.set_iin(in.get_card_iin());
        apply_update(record, in);
        record.set_status("active");
        record.set_created_at(now_stamp());
        return record;
    }

    [[nodiscard]] ::app::store::cards_info::CardInfoRecord to_store_record(
        const ::app::model::cards_info::CardInfoUpdateRequest& in
    ) const
    {
        ::app::store::cards_info::CardInfoRecord record;
        record.set_iin(in.get_card_iin());
        apply_update(record, in);
        record.set_status("active");
        record.set_created_at(now_stamp());
        return record;
    }

    void apply_update(
        ::app::store::cards_info::CardInfoRecord& record,
        const ::app::model::cards_info::CardInfoCreateRequest& in
    ) const
    {
        if (in.get_card_network()) {
            record.set_scheme(in.get_card_network()->get_value());
        }
        if (in.get_card_type()) {
            record.set_card_type(*in.get_card_type());
        }
        if (in.get_card_issuer()) {
            record.set_bank_name(*in.get_card_issuer());
        }
        if (in.get_country_code()) {
            record.set_country(*in.get_country_code());
        } else if (in.get_card_issuing_country()) {
            record.set_country(*in.get_card_issuing_country());
        }
        // Upstream UpdateCardInfo.last_updated = Some(now()) (core/cards_info.rs:87,248).
        record.set_updated_at(now_stamp());
    }

    void apply_update(
        ::app::store::cards_info::CardInfoRecord& record,
        const ::app::model::cards_info::CardInfoUpdateRequest& in
    ) const
    {
        if (in.get_card_network()) {
            record.set_scheme(in.get_card_network()->get_value());
        }
        if (in.get_card_type()) {
            record.set_card_type(*in.get_card_type());
        }
        if (in.get_card_issuer()) {
            record.set_bank_name(*in.get_card_issuer());
        }
        if (in.get_country_code()) {
            record.set_country(*in.get_country_code());
        } else if (in.get_card_issuing_country()) {
            record.set_country(*in.get_card_issuing_country());
        }
        record.set_updated_at(now_stamp());
    }

    [[nodiscard]] ::app::model::cards_info::CardInfoResponse response_from_store(
        const ::app::store::cards_info::CardInfoRecord& record
    ) const
    {
        ::app::model::cards_info::CardInfoResponse response;
        response.set_card_iin(record.get_iin());
        if (!record.get_scheme().empty()) {
            response.set_card_network(std::optional<std::string>{record.get_scheme()});
        }
        if (!record.get_card_type().empty()) {
            response.set_card_type(record.get_card_type());
        }
        if (!record.get_bank_name().empty()) {
            response.set_card_issuer(record.get_bank_name());
        }
        if (!record.get_country().empty()) {
            response.set_country_code(record.get_country());
        }
        return response;
    }

    [[nodiscard]] std::optional<std::string> multipart_file_bytes(
        interfaces::io::IRequest& req,
        std::string_view body
    ) const
    {
        const std::string content_type{req.find_header("content-type")};
        if (content_type.find("multipart/form-data") == std::string::npos) {
            return std::nullopt;
        }
        const auto marker = content_type.find("boundary=");
        if (marker == std::string::npos) {
            return std::nullopt;
        }
        std::string boundary = content_type.substr(marker + 9);
        const auto semicolon = boundary.find(';');
        if (semicolon != std::string::npos) {
            boundary = boundary.substr(0, semicolon);
        }
        const auto trim = [](std::string& text)
        {
            while (!text.empty() && (text.front() == '"' || text.front() == ' ' || text.front() == '\t')) {
                text.erase(text.begin());
            }
            while (!text.empty() && (text.back() == '"' || text.back() == ' ' || text.back() == '\t')) {
                text.pop_back();
            }
        };
        trim(boundary);
        if (boundary.empty()) {
            return std::nullopt;
        }

        const std::string delimiter = "--" + boundary;
        std::size_t cursor{0};
        while (cursor < body.size()) {
            const auto start = body.find(delimiter, cursor);
            if (start == std::string_view::npos) {
                break;
            }
            std::size_t header_start = start + delimiter.size();
            if (body.substr(header_start, 2) == "--") {
                break;
            }
            while (header_start < body.size() &&
                   (body[header_start] == '\r' || body[header_start] == '\n')) {
                ++header_start;
            }
            std::size_t header_end = body.find("\r\n\r\n", header_start);
            std::size_t payload_start{};
            if (header_end != std::string_view::npos) {
                payload_start = header_end + 4;
            } else {
                header_end = body.find("\n\n", header_start);
                if (header_end == std::string_view::npos) {
                    break;
                }
                payload_start = header_end + 2;
            }
            const std::string_view part_headers = body.substr(header_start, header_end - header_start);
            const auto next = body.find(delimiter, payload_start);
            if (next == std::string_view::npos) {
                break;
            }
            std::size_t payload_end = next;
            while (payload_end > payload_start &&
                   (body[payload_end - 1] == '\r' || body[payload_end - 1] == '\n')) {
                --payload_end;
            }
            if (part_headers.find("name=\"file\"") != std::string_view::npos ||
                part_headers.find("name=file") != std::string_view::npos) {
                return std::string{body.substr(payload_start, payload_end - payload_start)};
            }
            cursor = next;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::vector<std::string> split_csv_line(std::string_view line) const
    {
        std::vector<std::string> fields;
        std::string current;
        bool quoted{false};
        for (std::size_t i{0}; i < line.size(); ++i) {
            const char ch = line[i];
            if (quoted) {
                if (ch == '"') {
                    if (i + 1 < line.size() && line[i + 1] == '"') {
                        current.push_back('"');
                        ++i;
                    } else {
                        quoted = false;
                    }
                } else {
                    current.push_back(ch);
                }
            } else if (ch == '"') {
                quoted = true;
            } else if (ch == ',') {
                fields.push_back(std::move(current));
                current.clear();
            } else {
                current.push_back(ch);
            }
        }
        fields.push_back(std::move(current));
        return fields;
    }

    [[nodiscard]] std::optional<bool> parse_bool(std::string_view value) const
    {
        std::string lowered{value};
        std::transform(
            lowered.begin(), lowered.end(), lowered.begin(),
            [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); }
        );
        if (lowered == "true" || lowered == "1" || lowered == "yes") {
            return true;
        }
        if (lowered == "false" || lowered == "0" || lowered == "no") {
            return false;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::int64_t> parse_int(std::string_view value) const
    {
        std::int64_t parsed{};
        const auto result = std::from_chars(value.data(), value.data() + value.size(), parsed);
        if (result.ec != std::errc{} || result.ptr != value.data() + value.size()) {
            return std::nullopt;
        }
        return parsed;
    }

    void apply_csv_field(
        ::app::model::cards_info::CardInfoUpdateRequest& out,
        std::string_view header,
        std::string_view value
    ) const
    {
        using ::app::model::cards_info::CardAuthenticationValue;
        using ::app::model::cards_info::CardCostValue;
        using ::app::model::cards_info::CardNetworkValue;
        using ::app::model::cards_info::CoBadgedCardNetworkMetadataValue;
        using ::app::model::cards_info::FundingSourceValue;
        using ::app::model::cards_info::PanOrTokenValue;

        const auto text = [value]() { return std::string{value}; };
        if (header == "card_iin") {
            out.set_card_iin(text());
        } else if (header == "card_issuer") {
            out.set_card_issuer(text());
        } else if (header == "card_network") {
            CardNetworkValue wrapped;
            wrapped.set_value(text());
            out.set_card_network(std::move(wrapped));
        } else if (header == "card_type") {
            out.set_card_type(text());
        } else if (header == "card_subtype") {
            out.set_card_subtype(text());
        } else if (header == "card_issuing_country") {
            out.set_card_issuing_country(text());
        } else if (header == "bank_code_id") {
            out.set_bank_code_id(text());
        } else if (header == "bank_code") {
            out.set_bank_code(text());
        } else if (header == "country_code") {
            out.set_country_code(text());
        } else if (header == "last_updated_provider") {
            out.set_last_updated_provider(text());
        } else if (header == "funding_source") {
            FundingSourceValue wrapped;
            wrapped.set_value(text());
            out.set_funding_source(std::move(wrapped));
        } else if (header == "card_iin_type") {
            PanOrTokenValue wrapped;
            wrapped.set_value(text());
            out.set_card_iin_type(std::move(wrapped));
        } else if (header == "virtual_card") {
            if (const auto flag = parse_bool(value)) { out.set_virtual_card(*flag); }
        } else if (header == "gambling_blocked") {
            if (const auto flag = parse_bool(value)) { out.set_gambling_blocked(*flag); }
        } else if (header == "co_badged_card_networks") {
            CoBadgedCardNetworkMetadataValue wrapped;
            wrapped.set_value(text());
            out.set_co_badged_card_networks(std::move(wrapped));
        } else if (header == "card_segment_type") {
            out.set_card_segment_type(text());
        } else if (header == "numeric_country_code") {
            out.set_numeric_country_code(text());
        } else if (header == "prepaid") {
            if (const auto flag = parse_bool(value)) { out.set_prepaid(*flag); }
        } else if (header == "regulated") {
            if (const auto flag = parse_bool(value)) { out.set_regulated(*flag); }
        } else if (header == "issuer_phone") {
            out.set_issuer_phone(text());
        } else if (header == "issuer_url") {
            out.set_issuer_url(text());
        } else if (header == "regulated_name") {
            out.set_regulated_name(text());
        } else if (header == "reloadable_prepaid") {
            if (const auto flag = parse_bool(value)) { out.set_reloadable_prepaid(*flag); }
        } else if (header == "account_updater") {
            if (const auto flag = parse_bool(value)) { out.set_account_updater(*flag); }
        } else if (header == "account_level_management") {
            if (const auto flag = parse_bool(value)) { out.set_account_level_management(*flag); }
        } else if (header == "domestic_only") {
            if (const auto flag = parse_bool(value)) { out.set_domestic_only(*flag); }
        } else if (header == "level_two_supported") {
            if (const auto flag = parse_bool(value)) { out.set_level_two_supported(*flag); }
        } else if (header == "level_three_supported") {
            if (const auto flag = parse_bool(value)) { out.set_level_three_supported(*flag); }
        } else if (header == "issuer_currency") {
            out.set_issuer_currency(text());
        } else if (header == "combo_card") {
            out.set_combo_card(text());
        } else if (header == "authentication") {
            CardAuthenticationValue wrapped;
            wrapped.set_value(text());
            out.set_authentication(std::move(wrapped));
        } else if (header == "cost") {
            CardCostValue wrapped;
            wrapped.set_value(text());
            out.set_cost(std::move(wrapped));
        } else if (header == "issuer_supports_tokenization") {
            if (const auto flag = parse_bool(value)) { out.set_issuer_supports_tokenization(*flag); }
        } else if (header == "billpay_enabled") {
            if (const auto flag = parse_bool(value)) { out.set_billpay_enabled(*flag); }
        } else if (header == "ecom_enabled") {
            if (const auto flag = parse_bool(value)) { out.set_ecom_enabled(*flag); }
        } else if (header == "flexible_credential_supported") {
            if (const auto flag = parse_bool(value)) { out.set_flexible_credential_supported(*flag); }
        } else if (header == "card_subtype_code") {
            out.set_card_subtype_code(text());
        } else if (header == "multi_account_access_indicator") {
            out.set_multi_account_access_indicator(text());
        } else if (header == "line_number") {
            if (const auto number = parse_int(value)) { out.set_line_number(*number); }
        }
    }

    [[nodiscard]] std::optional<std::vector<::app::model::cards_info::CardInfoUpdateRequest>>
    parse_cards_csv(std::string_view csv) const
    {
        std::vector<std::string_view> lines;
        std::size_t start{0};
        while (start <= csv.size()) {
            const auto newline = csv.find('\n', start);
            const auto end = newline == std::string_view::npos ? csv.size() : newline;
            std::string_view line = csv.substr(start, end - start);
            if (!line.empty() && line.back() == '\r') {
                line.remove_suffix(1);
            }
            if (!line.empty()) {
                lines.push_back(line);
            }
            if (newline == std::string_view::npos) {
                break;
            }
            start = newline + 1;
        }
        if (lines.empty()) {
            return std::vector<::app::model::cards_info::CardInfoUpdateRequest>{};
        }
        const auto headers = split_csv_line(lines.front());
        if (headers.empty() || headers.front().empty()) {
            return std::nullopt;
        }

        std::vector<::app::model::cards_info::CardInfoUpdateRequest> records;
        std::int64_t line_number{0};
        for (std::size_t i{1}; i < lines.size(); ++i) {
            const auto values = split_csv_line(lines[i]);
            if (values.size() == 1 && values.front().empty()) {
                continue;
            }
            ::app::model::cards_info::CardInfoUpdateRequest record;
            for (std::size_t column{0}; column < headers.size() && column < values.size(); ++column) {
                if (!values[column].empty()) {
                    apply_csv_field(record, headers[column], values[column]);
                }
            }
            // core/cards_info.rs:138-143 generates line numbers from a row counter.
            record.set_line_number(++line_number);
            records.push_back(std::move(record));
        }
        return records;
    }

    [[nodiscard]] ::app::model::cards_info::CardInfoResponse build_response(
        const app::model::cards_info::CardInfoCreateRequest& in
    ) const
    {
        using namespace app::model::cards_info;
        CardInfoResponse out;
        out.set_card_iin(in.get_card_iin());
        if (in.get_card_issuer()) {
            out.set_card_issuer(in.get_card_issuer());
        }
        if (in.get_card_network()) {
            out.set_card_network(in.get_card_network()->get_value());
        }
        if (in.get_card_type()) {
            out.set_card_type(in.get_card_type());
        }
        if (in.get_card_subtype()) {
            out.set_card_sub_type(in.get_card_subtype());
        }
        if (in.get_card_issuing_country()) {
            out.set_card_issuing_country(in.get_card_issuing_country());
        }
        if (in.get_country_code()) {
            out.set_country_code(in.get_country_code());
        }
        if (in.get_funding_source()) {
            out.set_funding_source(in.get_funding_source()->get_value());
        }
        if (in.get_card_iin_type()) {
            out.set_card_iin_type(in.get_card_iin_type()->get_value());
        }
        if (in.get_virtual_card()) {
            out.set_virtual_card(in.get_virtual_card());
        }
        if (in.get_gambling_blocked()) {
            out.set_gambling_blocked(in.get_gambling_blocked());
        }
        if (in.get_co_badged_card_networks()) {
            out.set_co_badged_card_networks(in.get_co_badged_card_networks());
        }
        if (in.get_card_segment_type()) {
            out.set_card_segment_type(in.get_card_segment_type());
        }
        if (in.get_numeric_country_code()) {
            out.set_numeric_country_code(in.get_numeric_country_code());
        }
        if (in.get_prepaid()) {
            out.set_prepaid(in.get_prepaid());
        }
        if (in.get_regulated()) {
            out.set_regulated(in.get_regulated());
        }
        if (in.get_issuer_phone()) {
            out.set_issuer_phone(in.get_issuer_phone());
        }
        if (in.get_issuer_url()) {
            out.set_issuer_url(in.get_issuer_url());
        }
        if (in.get_regulated_name()) {
            out.set_regulated_name(in.get_regulated_name());
        }
        if (in.get_reloadable_prepaid()) {
            out.set_reloadable_prepaid(in.get_reloadable_prepaid());
        }
        if (in.get_account_updater()) {
            out.set_account_updater(in.get_account_updater());
        }
        if (in.get_account_level_management()) {
            out.set_account_level_management(in.get_account_level_management());
        }
        if (in.get_domestic_only()) {
            out.set_domestic_only(in.get_domestic_only());
        }
        if (in.get_level_two_supported()) {
            out.set_level_two_supported(in.get_level_two_supported());
        }
        if (in.get_level_three_supported()) {
            out.set_level_three_supported(in.get_level_three_supported());
        }
        if (in.get_issuer_currency()) {
            out.set_issuer_currency(in.get_issuer_currency());
        }
        if (in.get_combo_card()) {
            out.set_combo_card(in.get_combo_card());
        }
        if (in.get_authentication()) {
            out.set_authentication(in.get_authentication());
        }
        if (in.get_cost()) {
            out.set_cost(in.get_cost());
        }
        if (in.get_issuer_supports_tokenization()) {
            out.set_issuer_supports_tokenization(in.get_issuer_supports_tokenization());
        }
        if (in.get_billpay_enabled()) {
            out.set_billpay_enabled(in.get_billpay_enabled());
        }
        if (in.get_ecom_enabled()) {
            out.set_ecom_enabled(in.get_ecom_enabled());
        }
        if (in.get_flexible_credential_supported()) {
            out.set_flexible_credential_supported(in.get_flexible_credential_supported());
        }
        if (in.get_card_subtype_code()) {
            out.set_card_subtype_code(in.get_card_subtype_code());
        }
        if (in.get_multi_account_access_indicator()) {
            out.set_multi_account_access_indicator(in.get_multi_account_access_indicator());
        }
        return out;
    }

    [[nodiscard]] app::model::cards_info::CardInfoResponse build_response(
        const app::model::cards_info::CardInfoUpdateRequest& in
    ) const
    {
        using namespace app::model::cards_info;
        CardInfoResponse out;
        out.set_card_iin(in.get_card_iin());
        if (in.get_card_issuer()) {
            out.set_card_issuer(in.get_card_issuer());
        }
        if (in.get_card_network()) {
            out.set_card_network(in.get_card_network()->get_value());
        }
        if (in.get_card_type()) {
            out.set_card_type(in.get_card_type());
        }
        if (in.get_card_subtype()) {
            out.set_card_sub_type(in.get_card_subtype());
        }
        if (in.get_card_issuing_country()) {
            out.set_card_issuing_country(in.get_card_issuing_country());
        }
        if (in.get_country_code()) {
            out.set_country_code(in.get_country_code());
        }
        if (in.get_funding_source()) {
            out.set_funding_source(in.get_funding_source()->get_value());
        }
        if (in.get_card_iin_type()) {
            out.set_card_iin_type(in.get_card_iin_type()->get_value());
        }
        if (in.get_virtual_card()) {
            out.set_virtual_card(in.get_virtual_card());
        }
        if (in.get_gambling_blocked()) {
            out.set_gambling_blocked(in.get_gambling_blocked());
        }
        if (in.get_co_badged_card_networks()) {
            out.set_co_badged_card_networks(in.get_co_badged_card_networks());
        }
        if (in.get_card_segment_type()) {
            out.set_card_segment_type(in.get_card_segment_type());
        }
        if (in.get_numeric_country_code()) {
            out.set_numeric_country_code(in.get_numeric_country_code());
        }
        if (in.get_prepaid()) {
            out.set_prepaid(in.get_prepaid());
        }
        if (in.get_regulated()) {
            out.set_regulated(in.get_regulated());
        }
        if (in.get_issuer_phone()) {
            out.set_issuer_phone(in.get_issuer_phone());
        }
        if (in.get_issuer_url()) {
            out.set_issuer_url(in.get_issuer_url());
        }
        if (in.get_regulated_name()) {
            out.set_regulated_name(in.get_regulated_name());
        }
        if (in.get_reloadable_prepaid()) {
            out.set_reloadable_prepaid(in.get_reloadable_prepaid());
        }
        if (in.get_account_updater()) {
            out.set_account_updater(in.get_account_updater());
        }
        if (in.get_account_level_management()) {
            out.set_account_level_management(in.get_account_level_management());
        }
        if (in.get_domestic_only()) {
            out.set_domestic_only(in.get_domestic_only());
        }
        if (in.get_level_two_supported()) {
            out.set_level_two_supported(in.get_level_two_supported());
        }
        if (in.get_level_three_supported()) {
            out.set_level_three_supported(in.get_level_three_supported());
        }
        if (in.get_issuer_currency()) {
            out.set_issuer_currency(in.get_issuer_currency());
        }
        if (in.get_combo_card()) {
            out.set_combo_card(in.get_combo_card());
        }
        if (in.get_authentication()) {
            out.set_authentication(in.get_authentication());
        }
        if (in.get_cost()) {
            out.set_cost(in.get_cost());
        }
        if (in.get_issuer_supports_tokenization()) {
            out.set_issuer_supports_tokenization(in.get_issuer_supports_tokenization());
        }
        if (in.get_billpay_enabled()) {
            out.set_billpay_enabled(in.get_billpay_enabled());
        }
        if (in.get_ecom_enabled()) {
            out.set_ecom_enabled(in.get_ecom_enabled());
        }
        if (in.get_flexible_credential_supported()) {
            out.set_flexible_credential_supported(in.get_flexible_credential_supported());
        }
        if (in.get_card_subtype_code()) {
            out.set_card_subtype_code(in.get_card_subtype_code());
        }
        if (in.get_multi_account_access_indicator()) {
            out.set_multi_account_access_indicator(in.get_multi_account_access_indicator());
        }
        return out;
    }

    [[nodiscard]] app::model::cards_info::CardInfoMigrationResponse build_migration_response(
        const app::model::cards_info::CardInfoUpdateRequest& in,
        bool ok,
        std::string_view error
    ) const
    {
        using namespace app::model::cards_info;
        CardInfoMigrationResponse out;
        if (in.get_line_number()) {
            out.set_line_number(in.get_line_number());
        }
        out.set_card_iin(in.get_card_iin());
        if (in.get_card_issuer()) {
            out.set_card_issuer(in.get_card_issuer());
        }
        if (in.get_card_network()) {
            out.set_card_network(in.get_card_network()->get_value());
        }
        if (in.get_card_type()) {
            out.set_card_type(in.get_card_type());
        }
        if (in.get_card_subtype()) {
            out.set_card_sub_type(in.get_card_subtype());
        }
        if (in.get_card_issuing_country()) {
            out.set_card_issuing_country(in.get_card_issuing_country());
        }
        if (in.get_country_code()) {
            out.set_country_code(in.get_country_code());
        }
        if (in.get_funding_source()) {
            out.set_funding_source(in.get_funding_source()->get_value());
        }
        if (in.get_card_iin_type()) {
            out.set_card_iin_type(in.get_card_iin_type()->get_value());
        }
        if (in.get_virtual_card()) {
            out.set_virtual_card(in.get_virtual_card());
        }
        if (in.get_gambling_blocked()) {
            out.set_gambling_blocked(in.get_gambling_blocked());
        }
        if (in.get_co_badged_card_networks()) {
            out.set_co_badged_card_networks(in.get_co_badged_card_networks());
        }
        if (in.get_card_segment_type()) {
            out.set_card_segment_type(in.get_card_segment_type());
        }
        if (in.get_numeric_country_code()) {
            out.set_numeric_country_code(in.get_numeric_country_code());
        }
        if (in.get_prepaid()) {
            out.set_prepaid(in.get_prepaid());
        }
        if (in.get_regulated()) {
            out.set_regulated(in.get_regulated());
        }
        if (in.get_issuer_phone()) {
            out.set_issuer_phone(in.get_issuer_phone());
        }
        if (in.get_issuer_url()) {
            out.set_issuer_url(in.get_issuer_url());
        }
        if (in.get_regulated_name()) {
            out.set_regulated_name(in.get_regulated_name());
        }
        if (in.get_reloadable_prepaid()) {
            out.set_reloadable_prepaid(in.get_reloadable_prepaid());
        }
        if (in.get_account_updater()) {
            out.set_account_updater(in.get_account_updater());
        }
        if (in.get_account_level_management()) {
            out.set_account_level_management(in.get_account_level_management());
        }
        if (in.get_domestic_only()) {
            out.set_domestic_only(in.get_domestic_only());
        }
        if (in.get_level_two_supported()) {
            out.set_level_two_supported(in.get_level_two_supported());
        }
        if (in.get_level_three_supported()) {
            out.set_level_three_supported(in.get_level_three_supported());
        }
        if (in.get_issuer_currency()) {
            out.set_issuer_currency(in.get_issuer_currency());
        }
        if (in.get_combo_card()) {
            out.set_combo_card(in.get_combo_card());
        }
        if (in.get_authentication()) {
            out.set_authentication(in.get_authentication());
        }
        if (in.get_cost()) {
            out.set_cost(in.get_cost());
        }
        if (in.get_issuer_supports_tokenization()) {
            out.set_issuer_supports_tokenization(in.get_issuer_supports_tokenization());
        }
        if (in.get_billpay_enabled()) {
            out.set_billpay_enabled(in.get_billpay_enabled());
        }
        if (in.get_ecom_enabled()) {
            out.set_ecom_enabled(in.get_ecom_enabled());
        }
        if (in.get_flexible_credential_supported()) {
            out.set_flexible_credential_supported(in.get_flexible_credential_supported());
        }
        if (in.get_card_subtype_code()) {
            out.set_card_subtype_code(in.get_card_subtype_code());
        }
        if (in.get_multi_account_access_indicator()) {
            out.set_multi_account_access_indicator(in.get_multi_account_access_indicator());
        }
        out.set_migration_status(
            ok ? CardInfoMigrationStatus::Success : CardInfoMigrationStatus::Failed
        );
        if (!error.empty()) {
            out.set_migration_error(std::string{error});
        }
        return out;
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

} // namespace app::handlers::cards_info
