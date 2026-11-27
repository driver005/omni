export module app_payment:payment_methods_input;

import std;
import core_behavior;
import interfaces;
import core_router;
import core_serde;
import core_cache;
import core_error;
import app_model;
import app_context;
import app_auth;
import app_http;
import app_crypto;
import app_store;

export namespace app::handlers::payment_methods {

/**
 * @brief Request parsing / auth / path / query support for payment-method handlers.
 *
 * Business logic stays in runtime/*.cppm; this class only turns request data into
 * the plain Input structs consumed by the domain runtime. It performs the same
 * early auth and path/DB-existence checks, in the same order, as the original
 * monolithic handlers so error precedence on the wire is unchanged.
 */
class PaymentMethodsInputSupport
{
public:
    explicit PaymentMethodsInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] core::error::AppError
    error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    [[nodiscard]] bool legacy_access_allowed(interfaces::io::IRequest& req) const
    {
        (void)req;
        const auto configured =
            m_context.config_store().get_value("payment_methods", "legacy_endpoints_enabled");
        return !configured || (*configured != "false" && *configured != "disabled");
    }

    [[nodiscard]] std::optional<core::error::AppError>
    authorize_client_facing(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto context = ::app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context.auth_rules().rule_for(operation);
        if (!m_context.authorizer().authorize_route(context, rule)) {
            return std::nullopt;
        }
        for (const auto scheme: {
                 ::app::auth::AuthScheme::PUBLISHABLE_KEY,
                 ::app::auth::AuthScheme::CLIENT_SECRET,
                 ::app::auth::AuthScheme::SDK,
             }) {
            const auto sdk_rule = ::app::auth::RouteAuthRuleBuilder{}
                                      .scheme(scheme)
                                      .permission(rule.permission())
                                      .build();
            if (!m_context.authorizer().authorize_route(context, sdk_rule)) {
                return std::nullopt;
            }
        }
        return m_context.authorizer().authorize_route(context, rule);
    }

    [[nodiscard]] std::string
    path_segment(interfaces::io::IRequest& req, std::size_t from_end = 0) const
    {
        const ::app::http::PathSegments segments{::app::http::RequestPath{}(req)};
        const std::size_t count{segments.count()};
        if (from_end >= count) {
            return {};
        }
        const auto value = segments.at(count - 1 - from_end);
        return value ? std::string{*value} : std::string{};
    }

    [[nodiscard]] bool customer_exists(std::string_view customer_id) const
    {
        if (customer_id.empty()) {
            return false;
        }
        for (const auto& record: m_context.repository().list_customers()) {
            if (record.get_customer_id() == customer_id) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] std::optional<::app::store::payment_methods::PaymentMethodRecord>
    find_payment_method(std::string_view payment_method_id) const
    {
        return m_context.repository().find_payment_method_by_string(payment_method_id);
    }

    [[nodiscard]] std::optional<::app::store::payment_method_sessions::PaymentMethodSessionRecord>
    find_payment_method_session(std::string_view session_id) const
    {
        return m_context.repository().find_payment_method_session_by_string(session_id);
    }

    [[nodiscard]] std::optional<core::error::AppError>
    payment_access_error(std::string_view payment_id, std::string_view merchant_id) const
    {
        for (const auto& record: m_context.repository().list_payments()) {
            if (record.get_payment_id() != payment_id) {
                continue;
            }
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) {
                return error(
                    core::error::AppErrorKind::FORBIDDEN,
                    "payment belongs to another merchant"
                );
            }
            return std::nullopt;
        }
        return error(core::error::AppErrorKind::NOT_FOUND, "payment not found");
    }

    [[nodiscard]] bool collect_link_exists(std::string_view collect_id) const
    {
        return m_context.cache().get("payment_methods.collect:" + std::string{collect_id}).hit;
    }

    [[nodiscard]] std::expected<std::string, core::error::AppError>
    token_payment_method(std::string_view temporary_token) const
    {
        const auto hit =
            m_context.cache().get("payment_methods.token:" + std::string{temporary_token});
        if (!hit.hit) {
            return std::unexpected{error(core::error::AppErrorKind::NOT_FOUND, "token not found")};
        }
        const std::string payment_method_id = core::cache::value_as<std::string>(hit.value);
        if (!m_context.repository().find_payment_method_by_string(payment_method_id)) {
            return std::unexpected{error(
                core::error::AppErrorKind::NOT_FOUND,
                "Payment method does not exist in our records"
            )};
        }
        return payment_method_id;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    check_client_secret(std::string_view key, const std::optional<std::string>& presented) const
    {
        if (!presented) {
            return std::nullopt;
        }
        const auto hit = m_context.cache().get(
            std::string{"payment_methods.client_secret_hash:"} + std::string{key}
        );
        if (!hit.hit) {
            return std::nullopt;
        }
        const auto expected = core::cache::value_as<std::string>(hit.value);
        if (!::app::crypto::KeyedHasher{"payment_methods.client_secret"}
                 .verify(*presented, expected)) {
            return error(core::error::AppErrorKind::FORBIDDEN, "invalid client secret");
        }
        return std::nullopt;
    }

    /** @brief Internal JSON text for connector/cache payloads, not wire output. */
    template<typename T>
    [[nodiscard]] std::string json_payload(const T& value) const
    {
        return serde::Ser::encode_json(value);
    }

    [[nodiscard]] std::vector<std::string> csv_cells(std::string_view line) const
    {
        std::vector<std::string> cells{};
        std::size_t start{0};
        while (start <= line.size()) {
            const auto end = line.find(',', start);
            std::string cell{line.substr(
                start,
                end == std::string_view::npos ? std::string_view::npos : end - start
            )};
            while (!cell.empty() &&
                   (cell.back() == '\r' || cell.back() == '\n' || cell.back() == ' ')) {
                cell.pop_back();
            }
            cells.push_back(std::move(cell));
            if (end == std::string_view::npos) {
                break;
            }
            start = end + 1;
        }
        return cells;
    }

    [[nodiscard]] std::string parse_batch_rows(std::string_view body) const
    {
        const auto first = body.find_first_not_of(" \t\r\n");
        if (first == std::string_view::npos) {
            return "[]";
        }
        if (body[first] == '[' || body[first] == '{') {
            return std::string{body};
        }
        std::vector<std::string> lines{};
        std::size_t pos{0};
        while (pos < body.size()) {
            const auto end = body.find('\n', pos);
            std::string line{
                body.substr(pos, end == std::string_view::npos ? std::string_view::npos : end - pos)
            };
            while (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (!line.empty() && line.front() != '-' && !line.starts_with("Content-")) {
                lines.push_back(std::move(line));
            }
            if (end == std::string_view::npos) {
                break;
            }
            pos = end + 1;
        }
        if (lines.size() < 2) {
            return "[]";
        }
        const auto header = csv_cells(lines.front());
        std::string out = "[";
        for (std::size_t row = 1; row < lines.size(); ++row) {
            const auto cells = csv_cells(lines[row]);
            if (row > 1) {
                out += ",";
            }
            out += "{";
            bool wrote = false;
            std::string card{};
            for (std::size_t col = 0; col < header.size() && col < cells.size(); ++col) {
                const auto& name = header[col];
                if (name.empty() || cells[col].empty()) {
                    continue;
                }
                const auto value = serde::Ser::encode_json(cells[col]);
                if (name == "card_number" || name == "card_exp_month" || name == "card_exp_year") {
                    if (!card.empty()) {
                        card += ",";
                    }
                    card += serde::Ser::encode_json(name) + ":" + value;
                    continue;
                }
                if (wrote) {
                    out += ",";
                }
                wrote = true;
                if (name == "merchant_id" || name == "customer_id" || name == "payment_method" ||
                    name == "payment_method_type" || name == "card_network" ||
                    name == "network_transaction_id") {
                    out += serde::Ser::encode_json(name) + ":{\"value\":" + value + "}";
                } else {
                    out += serde::Ser::encode_json(name) + ":" + value;
                }
            }
            if (!card.empty()) {
                if (wrote) {
                    out += ",";
                }
                out += "\"card\":{" + card + "}";
            }
            out += "}";
        }
        out += "]";
        return out;
    }

    /** @brief Upstream batch_retrieve::parse_csv — one (payment_method_id, line_number) per CSV
     * row. */
    [[nodiscard]] std::vector<std::pair<std::string, std::int64_t>>
    batch_retrieve_rows(std::string_view body) const
    {
        std::vector<std::pair<std::string, std::int64_t>> out{};
        std::size_t pos{0};
        bool header{true};
        std::size_t id_column{0};
        std::int64_t line_number{0};
        while (pos < body.size()) {
            const auto end = body.find('\n', pos);
            std::string text{
                body.substr(pos, end == std::string_view::npos ? std::string_view::npos : end - pos)
            };
            while (!text.empty() && text.back() == '\r') {
                text.pop_back();
            }
            if (!text.empty() && text.front() != '-' && !text.starts_with("Content-")) {
                const auto cells = csv_cells(text);
                if (header) {
                    for (std::size_t i = 0; i < cells.size(); ++i) {
                        if (cells[i] == "payment_method_id") {
                            id_column = i;
                            break;
                        }
                    }
                    header = false;
                } else if (id_column < cells.size() && !cells[id_column].empty()) {
                    out.emplace_back(cells[id_column], ++line_number);
                }
            }
            if (end == std::string_view::npos) {
                break;
            }
            pos = end + 1;
        }
        return out;
    }

private:
    ::app::context::AppContext& m_context;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old handler handed to its core block,
// plus the path/query/auth values the core could no longer read from req.

struct CreatePaymentMethodApiInput
{
    ::app::model::payment_methods::PaymentMethodCreate request{};
    std::string merchant_id{};
    std::string profile_id{};
};

struct GetPmNtEligibilityApiInput
{
    std::string card_bin{};
    std::string org_id{};
};

struct CreatePaymentMethodIntentApiInput
{
    ::app::model::payment_methods::PaymentMethodIntentCreate request{};
    std::string merchant_id{};
};

struct PaymentMethodUpdateApiInput
{
    ::app::model::payment_methods::PaymentMethodUpdate request{};
    std::string payment_method_id{};
    ::app::store::payment_methods::PaymentMethodRecord record{};
};

struct PaymentMethodRetrieveApiInput
{
    ::app::store::payment_methods::PaymentMethodRecord record{};
    std::string payment_method_id{};
    std::string merchant_id{};
    bool force_sync{false};
};

struct PaymentMethodDeleteApiInput
{
    ::app::store::payment_methods::PaymentMethodRecord record{};
    std::string payment_method_id{};
};

struct MigratePaymentMethodApiInput
{
    ::app::model::payment_methods::PaymentMethodMigrate request{};
    std::string merchant_id{};
};

struct MigratePaymentMethodsInput
{
    std::vector<::app::model::payment_methods::PaymentMethodMigrate> rows{};
};

struct UpdatePaymentMethodsInput
{
    std::vector<::app::model::payment_methods::UpdatePaymentMethodRecord> rows{};
};

struct PaymentMethodsBatchRetrieveApiInput
{
    std::vector<std::pair<std::string, std::int64_t>> rows{};
};

struct SavePaymentMethodApiInput
{
    ::app::model::payment_methods::PaymentMethodCreate request{};
    std::string payment_method_id{};
    std::string merchant_id{};
};

struct ListPaymentMethodApiInput
{
    std::optional<std::string> client_secret{};
    std::optional<std::string> amount{};
    std::size_t limit{20};
    std::string merchant_id{};
};

struct ListCustomerPaymentMethodApiInput
{
    std::string customer_id{};
    std::string merchant_id{};
};

struct ListCustomerPaymentMethodApiClientInput
{
    std::string customer_id{};
    std::optional<std::string> client_secret{};
};

struct InitiatePmCollectLinkFlowInput
{
    ::app::model::payment_methods::PaymentMethodCollectLinkRequest request{};
    std::string merchant_id{};
    std::string body{};
};

struct PaymentMethodRetrieveOlapApiInput
{
    ::app::store::payment_methods::PaymentMethodRecord record{};
};

struct GetPaymentMethodTokenDataInput
{
    ::app::store::payment_methods::PaymentMethodRecord record{};
    std::string payment_method_id{};
    std::string token_type{};
};

struct GetTotalPaymentMethodCountInput
{
    std::string customer_id{};
    std::string merchant_id{};
};

struct RenderPmCollectLinkInput
{
    std::string merchant_id{};
    std::string collect_id{};
};

struct ListCountriesCurrenciesForConnectorPaymentMethodInput
{
    std::string connector{};
    std::string payment_method_type{};
};

struct DefaultPaymentMethodSetApiInput
{
    ::app::model::payment_methods::DefaultPaymentMethod request{};
    std::string customer_id{};
    std::string payment_method_id{};
    ::app::store::payment_methods::PaymentMethodRecord record{};
};

struct TokenizeCardApiInput
{
    ::app::model::payment_methods::TokenizeCardRequest request{};
    std::string merchant_id{};
};

struct TokenizeCardUsingPmApiInput
{
    ::app::model::payment_methods::TokenizePaymentMethodRequest request{};
    std::string payment_method_id{};
    ::app::store::payment_methods::PaymentMethodRecord record{};
};

struct TokenizeCardBatchApiInput
{
    std::vector<std::string> payloads{};
    std::string merchant_id{};
};

struct PaymentMethodsSessionCreateInput
{
    ::app::model::payment_methods::PaymentMethodSessionRequest request{};
    std::string merchant_id{};
    std::string org_id{};
};

struct PaymentMethodsSessionUpdateInput
{
    ::app::model::payment_methods::PaymentMethodsSessionUpdateRequest request{};
    std::string session_id{};
    ::app::store::payment_method_sessions::PaymentMethodSessionRecord record{};
};

struct PaymentMethodsSessionRetrieveInput
{
    std::string session_id{};
    ::app::store::payment_method_sessions::PaymentMethodSessionRecord record{};
    std::optional<std::string> client_secret{};
};

struct PaymentMethodSessionListPaymentMethodsInput
{
    ::app::store::payment_method_sessions::PaymentMethodSessionRecord session{};
    std::string customer_id{};
};

struct PaymentMethodSessionConfirmInput
{
    ::app::model::payment_methods::PaymentMethodSessionConfirmRequest request{};
    ::app::store::payment_method_sessions::PaymentMethodSessionRecord session{};
    std::string session_id{};
    std::string body{};
    std::string payload{};
    std::optional<std::string> client_secret{};
};

struct PaymentMethodSessionUpdateSavedPaymentMethodInput
{
    ::app::model::payment_methods::PaymentMethodSessionUpdateSavedPaymentMethod request{};
    ::app::store::payment_method_sessions::PaymentMethodSessionRecord session{};
    ::app::store::payment_methods::PaymentMethodRecord record{};
};

struct PaymentMethodSessionDeleteSavedPaymentMethodInput
{
    std::string token{};
    ::app::store::payment_method_sessions::PaymentMethodSessionRecord session{};
    ::app::store::payment_methods::PaymentMethodRecord record{};
};

struct NetworkTokenStatusCheckApiInput
{
    ::app::store::payment_methods::PaymentMethodRecord record{};
    std::string payment_method_id{};
};

struct PaymentMethodGetTokenDetailsApiInput
{
    std::string payment_method_id{};
    std::string temporary_token{};
};

struct ListPaymentMethodsForPaymentsClientInput
{
    std::string payment_id{};
    std::string merchant_id{};
    std::optional<std::string> client_secret{};
};

struct ModularMigratePaymentMethodsInput
{
    std::vector<::app::model::payment_methods::PaymentMethodMigrate> rows{};
};

/** @brief Request policy for the payment_methods domain: wraps the domain auth rules. */
class PaymentMethodsPolicy
{
public:
    PaymentMethodsPolicy(
        ::app::context::AppContext& /*context*/,
        PaymentMethodsInputSupport& support
    ) noexcept :
        m_support{support}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec& spec
    ) const
    {
        (void)spec;
        if (spec.allow_connected_scope_operation) {
            return m_support.authorize_client_facing(req, operation);
        }
        return m_support.authorize(req, operation);
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
    PaymentMethodsInputSupport& m_support;
};

using PaymentMethodsRequestSupport = core::behavior::RequestSupport<PaymentMethodsPolicy>;
using PaymentMethodsPolicySpec = core::behavior::AuthSpec;

[[nodiscard]] PaymentMethodsPolicySpec spec_for(std::string_view operation) noexcept
{
    return PaymentMethodsPolicySpec{.permission = operation};
}

[[nodiscard]] PaymentMethodsPolicySpec client_facing_spec_for(std::string_view operation) noexcept
{
    return PaymentMethodsPolicySpec{
        .permission = operation,
        .allow_connected_scope_operation = true,
    };
}
} // namespace app::handlers::payment_methods
