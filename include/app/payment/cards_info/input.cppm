export module app_payment:cards_info_input;

import std;
import core_behavior;
import interfaces;
import core_error;
import core_serde;
import app_context;
import app_model;
import app_auth;
import app_http;
import app_metrics;

import :context;

export namespace app::handlers::cards_info {
struct CardIinInfoData
{
    ::app::model::cards_info::CardsInfoRequest payload{};

    template<class Dto>
    explicit CardIinInfoData(const Dto& dto, const core::behavior::CallContext& ctx) :
        payload{::app::payment::detail::as_dto<::app::model::cards_info::CardsInfoRequest>(dto)}
    {
    }
};

struct CreateCardsInfoData
{
    ::app::model::cards_info::CardInfoCreateRequest request{};

    template<class Dto>
    explicit CreateCardsInfoData(const Dto& dto, const core::behavior::CallContext& ctx) :
        request{
            ::app::payment::detail::as_dto<::app::model::cards_info::CardInfoCreateRequest>(dto)
        }
    {
    }
};

struct UpdateCardsInfoData
{
    ::app::model::cards_info::CardInfoUpdateRequest request{};

    template<class Dto>
    explicit UpdateCardsInfoData(const Dto& dto, const core::behavior::CallContext& ctx) :
        request{
            ::app::payment::detail::as_dto<::app::model::cards_info::CardInfoUpdateRequest>(dto)
        }
    {
    }
};

struct MigrateCardsInfoData
{
    std::vector<::app::model::cards_info::CardInfoUpdateRequest> records{};

    explicit MigrateCardsInfoData(const core::behavior::CallContext& ctx) :
        records{}
    {
    }
};

/**
 * @brief Parse/auth/path/query support for the cards-info handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime.
 */
class CardsInfoInputSupport
{
public:
    explicit CardsInfoInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ── Upstream ApiErrorResponse wire mapping is built by the handler; this
    // support only evaluates the route's authorization rule and the SDK fallback
    // documented in routes/cards_info.rs:25,44-47.
    [[nodiscard]] std::optional<core::error::AppError> authorize_operation(
        const ::app::auth::AuthContext& context,
        std::string_view operation,
        bool sdk_fallback = false
    ) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        auto auth = m_context.authorizer().authorize_route(context, rule);
        if (auth && sdk_fallback) {
            // api_key (x-api-key) or publishable_key with client_secret is accepted;
            // both are real upstream auth paths.
            const auto api_key_rule = ::app::auth::RouteAuthRuleBuilder{}
                                          .scheme(::app::auth::AuthScheme::V2_API_KEY)
                                          .permission(rule.permission())
                                          .build();
            auth = m_context.authorizer().authorize_route(context, api_key_rule);
            if (auth) {
                const auto sdk_rule = ::app::auth::RouteAuthRuleBuilder{}
                                          .scheme(::app::auth::AuthScheme::SDK)
                                          .permission(rule.permission())
                                          .build();
                auth = m_context.authorizer().authorize_route(context, sdk_rule);
            }
            // auth is nullopt when either SDK path authorizes; otherwise it carries
            // the SDK path's error, matching check_sdk_auth_and_get_auth's rejection.
        }
        return auth;
    }

    // Flow/counter instrumentation used to live in the handler's server_wrap.
    void record_flow(std::string_view flow) const
    {
        m_context.logger().record_flow(flow);
    }

    void increment_counter(std::string_view counter) const
    {
        m_context.logger().counter(counter).increment();
    }

    // ── /cards/:bin path segment ─────────────────────────────────────────────
    [[nodiscard]] std::optional<std::string> bin_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        const auto bin = segments.at(1);
        if (!bin) {
            return std::nullopt;
        }
        return std::optional<std::string>{std::string{*bin}};
    }

    [[nodiscard]] std::optional<std::string>
    client_secret_from_query(interfaces::io::IRequest& req) const
    {
        const auto query = ::app::http::RequestQuery{}(req);
        return query.get("client_secret");
    }

    // ── Multipart file extraction (routes/cards_info.rs:127-157) ──────────────
    [[nodiscard]] std::optional<std::string>
    multipart_file_bytes(interfaces::io::IRequest& req, std::string_view body) const
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
            while (!text.empty() &&
                   (text.front() == '"' || text.front() == ' ' || text.front() == '\t')) {
                text.erase(text.begin());
            }
            while (!text.empty() &&
                   (text.back() == '"' || text.back() == ' ' || text.back() == '\t')) {
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
            const std::string_view part_headers =
                body.substr(header_start, header_end - header_start);
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

    // ── core/cards_info.rs:133-157 parse_cards_bin_csv / get_cards_bin_records ─
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
            for (std::size_t column{0}; column < headers.size() && column < values.size();
                 ++column) {
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

private:
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
            lowered.begin(),
            lowered.end(),
            lowered.begin(),
            [](unsigned char ch)
            {
                return static_cast<char>(std::tolower(ch));
            }
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

        const auto text = [value]()
        {
            return std::string{value};
        };
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
            if (const auto flag = parse_bool(value)) {
                out.set_virtual_card(*flag);
            }
        } else if (header == "gambling_blocked") {
            if (const auto flag = parse_bool(value)) {
                out.set_gambling_blocked(*flag);
            }
        } else if (header == "co_badged_card_networks") {
            CoBadgedCardNetworkMetadataValue wrapped;
            wrapped.set_value(text());
            out.set_co_badged_card_networks(std::move(wrapped));
        } else if (header == "card_segment_type") {
            out.set_card_segment_type(text());
        } else if (header == "numeric_country_code") {
            out.set_numeric_country_code(text());
        } else if (header == "prepaid") {
            if (const auto flag = parse_bool(value)) {
                out.set_prepaid(*flag);
            }
        } else if (header == "regulated") {
            if (const auto flag = parse_bool(value)) {
                out.set_regulated(*flag);
            }
        } else if (header == "issuer_phone") {
            out.set_issuer_phone(text());
        } else if (header == "issuer_url") {
            out.set_issuer_url(text());
        } else if (header == "regulated_name") {
            out.set_regulated_name(text());
        } else if (header == "reloadable_prepaid") {
            if (const auto flag = parse_bool(value)) {
                out.set_reloadable_prepaid(*flag);
            }
        } else if (header == "account_updater") {
            if (const auto flag = parse_bool(value)) {
                out.set_account_updater(*flag);
            }
        } else if (header == "account_level_management") {
            if (const auto flag = parse_bool(value)) {
                out.set_account_level_management(*flag);
            }
        } else if (header == "domestic_only") {
            if (const auto flag = parse_bool(value)) {
                out.set_domestic_only(*flag);
            }
        } else if (header == "level_two_supported") {
            if (const auto flag = parse_bool(value)) {
                out.set_level_two_supported(*flag);
            }
        } else if (header == "level_three_supported") {
            if (const auto flag = parse_bool(value)) {
                out.set_level_three_supported(*flag);
            }
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
            if (const auto flag = parse_bool(value)) {
                out.set_issuer_supports_tokenization(*flag);
            }
        } else if (header == "billpay_enabled") {
            if (const auto flag = parse_bool(value)) {
                out.set_billpay_enabled(*flag);
            }
        } else if (header == "ecom_enabled") {
            if (const auto flag = parse_bool(value)) {
                out.set_ecom_enabled(*flag);
            }
        } else if (header == "flexible_credential_supported") {
            if (const auto flag = parse_bool(value)) {
                out.set_flexible_credential_supported(*flag);
            }
        } else if (header == "card_subtype_code") {
            out.set_card_subtype_code(text());
        } else if (header == "multi_account_access_indicator") {
            out.set_multi_account_access_indicator(text());
        } else if (header == "line_number") {
            if (const auto number = parse_int(value)) {
                out.set_line_number(*number);
            }
        }
    }

    ::app::context::AppContext& m_context;
};

/** @brief Request policy for the cards_info domain: wraps the domain auth rules. */
class CardsInfoPolicy
{
public:
    CardsInfoPolicy(
        ::app::context::AppContext& /*context*/,
        CardsInfoInputSupport& support
    ) noexcept :
        m_support{support}
    {
    }

    [[nodiscard]] core::behavior::CallContext context(interfaces::io::IRequest& req) const
    {
        return ::app::payment::detail::make_call_context(req);
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec& spec
    ) const
    {
        (void)spec;
        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        return m_support.authorize_operation(ctx, operation, operation == "card_iin_info");
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        core::behavior::Scope result;
        result.merchant_id = std::string{auth.merchant_id()};
        result.profile_id = std::string{auth.profile_id()};
        result.org_id = std::string{auth.org_id()};
        const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId);
        result.connected_merchant_id = connected ? std::string{*connected} : std::string{};
        return result;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope&) const
    {
        return std::nullopt;
    }


private:
    CardsInfoInputSupport& m_support;
};

using CardsInfoRequestSupport = core::behavior::RequestSupport<CardsInfoPolicy>;
using CardsInfoPolicySpec = core::behavior::AuthSpec;

[[nodiscard]] CardsInfoPolicySpec spec_for(std::string_view operation) noexcept
{
    return CardsInfoPolicySpec{.permission = operation};
}
} // namespace app::handlers::cards_info
