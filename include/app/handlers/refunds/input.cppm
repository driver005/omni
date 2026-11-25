export module app_handlers_refunds:input;

import std;
import interfaces;
import core_error;
import app_context;
import app_model;
import app_store;
import app_auth;
import app_http;

export namespace app::handlers::refunds {

using Error = core::error::AppError;
using Kind = core::error::AppErrorKind;
using Record = app::store::refunds::RefundRecord;
using Response = app::model::refunds::RefundResponse;
using Status = app::model::refunds::RefundStatus;
using Intent = app::store::payments::PaymentIntentRecord;
using Attempt = app::store::payment_attempts::PaymentAttemptRecord;
using Rule = app::auth::RouteAuthRule;
using Scheme = app::auth::AuthScheme;
using Perm = app::auth::AppPermission;

/** @brief Upstream auth extractor profiles, selected per route declaration. */
enum class AuthProfile
{
    V1Create,
    V1Retrieve,
    V1Sync,
    V1Update,
    V1Reverse,
    V1List,
    Platform,
    ProfileList,
    Filter,
    Filters,
    FiltersProfile,
    Aggregates,
    AggregatesProfile,
    Admin,
    V2Create,
    V2Retrieve,
    V2Gateway,
    V2Metadata,
    V2List,
};

/**
 * @brief Auth/request facts extracted from IRequest by the handler and consumed by the
 *        runtime. Keeps HTTP types out of the runtime while preserving the exact upstream
 *        merchant/profile resolution order.
 */
struct RefundRequestFacts
{
    std::string ctx_profile_id{};
    std::string ctx_merchant_id{};
    std::string header_merchant_id{};
    std::string request_path{};
};

/**
 * @brief Parse/auth/path/query support for the refunds handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input structs
 * consumed by the domain runtime.
 */
class RefundsInputSupport
{
public:
    explicit RefundsInputSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ───────────────────────── Auth semantics ─────────────────────────
    // routes/refunds.rs auth_type(...) + the individual auth::* extractors.
    [[nodiscard]] std::optional<Error>
    authorize_op(interfaces::io::IRequest& req, AuthProfile profile) const
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const bool api_key = ctx.claim(app::auth::kClaimApiKey).has_value();
        const bool platform_cfg = config_bool("merchant_id_auth_enabled", "merchant_id_auth");

        Rule rule{};
        bool platform_account_required = false;
        switch (profile) {
            case AuthProfile::V1Create: // refunds.rs:107-119 (ProfileRefundWrite)
                rule = Rule{
                    api_key ? Scheme::API_KEY : Scheme::JWT_AND_EMBEDDED,
                    Perm::REFUND_WRITE,
                    true,
                    false
                };
                break;
            case AuthProfile::V1Retrieve: // refunds.rs:225-237 (ProfileRefundRead)
                rule = Rule{
                    api_key ? Scheme::API_KEY : Scheme::JWT_AND_EMBEDDED,
                    Perm::REFUND_READ,
                    true,
                    false
                };
                break;
            case AuthProfile::V1Sync: // refunds.rs:385-389 (HeaderAuth only)
                rule = Rule{Scheme::API_KEY, Perm::REFUND_READ, true, false};
                break;
            case AuthProfile::V1Update: // refunds.rs:417-421 (HeaderAuth only)
                rule = Rule{Scheme::API_KEY, Perm::REFUND_WRITE, true, false};
                break;
            case AuthProfile::V1Reverse: // refunds.rs:449-453 (HeaderAuth only)
                rule = Rule{Scheme::API_KEY, Perm::REFUND_WRITE, true, false};
                break;
            case AuthProfile::V1List: // refunds.rs:516-527 (MerchantRefundRead)
                rule =
                    Rule{api_key ? Scheme::API_KEY : Scheme::JWT, Perm::REFUND_READ, true, false};
                break;
            case AuthProfile::Platform: // refunds.rs:590-600 / :625-635
                                        // (allow_platform_self_operation)
                rule =
                    Rule{api_key ? Scheme::API_KEY : Scheme::JWT, Perm::REFUND_READ, false, true};
                platform_account_required = true;
                break;
            case AuthProfile::ProfileList: // refunds.rs:666-678 (JWTAndEmbedded ProfileRefundRead)
                rule = Rule{
                    api_key ? Scheme::API_KEY : Scheme::JWT_AND_EMBEDDED,
                    Perm::REFUND_READ,
                    true,
                    false
                };
                break;
            case AuthProfile::Filter: // refunds.rs:703-713
                rule =
                    Rule{api_key ? Scheme::API_KEY : Scheme::JWT, Perm::REFUND_READ, true, false};
                break;
            case AuthProfile::Filters: // refunds.rs:735-745
                rule =
                    Rule{api_key ? Scheme::API_KEY : Scheme::JWT, Perm::REFUND_READ, true, false};
                break;
            case AuthProfile::FiltersProfile: // refunds.rs:774-786 (JWTAndEmbedded)
                rule = Rule{
                    api_key ? Scheme::API_KEY : Scheme::JWT_AND_EMBEDDED,
                    Perm::REFUND_READ,
                    true,
                    false
                };
                break;
            case AuthProfile::Aggregates: // refunds.rs:809-819
                rule =
                    Rule{api_key ? Scheme::API_KEY : Scheme::JWT, Perm::REFUND_READ, true, false};
                break;
            case AuthProfile::AggregatesProfile: // refunds.rs:871-883 (JWTAndEmbedded)
                rule = Rule{
                    api_key ? Scheme::API_KEY : Scheme::JWT_AND_EMBEDDED,
                    Perm::REFUND_READ,
                    true,
                    false
                };
                break;
            case AuthProfile::Admin: // refunds.rs:843 — AdminApiAuth
                rule = Rule{Scheme::PLATFORM_ADMIN, Perm::ADMIN, false, false};
                break;
            case AuthProfile::V2Create: // refunds.rs:148-163
                if (platform_cfg) {
                    rule = Rule{Scheme::MERCHANT_ID, Perm::REFUND_WRITE};
                } else {
                    rule = Rule{api_key ? Scheme::V2_API_KEY : Scheme::JWT, Perm::REFUND_WRITE};
                }
                break;
            case AuthProfile::V2Retrieve: // refunds.rs:277-288
                rule = Rule{api_key ? Scheme::V2_API_KEY : Scheme::JWT, Perm::REFUND_READ};
                break;
            case AuthProfile::V2Gateway: // refunds.rs:316-331 (MerchantIdAuth when enabled)
                if (platform_cfg) {
                    rule = Rule{Scheme::MERCHANT_ID, Perm::REFUND_READ};
                } else {
                    rule = Rule{api_key ? Scheme::V2_API_KEY : Scheme::JWT, Perm::REFUND_READ};
                }
                break;
            case AuthProfile::V2Metadata: // refunds.rs:488-491 (V2ApiKeyAuth only)
                rule = Rule{Scheme::V2_API_KEY, Perm::REFUND_WRITE};
                break;
            case AuthProfile::V2List: // refunds.rs:554-565
                rule = Rule{
                    api_key ? Scheme::V2_API_KEY : Scheme::JWT,
                    Perm::REFUND_READ,
                    true,
                    false
                };
                break;
        }

        if (const auto denied = check_auth_credentials(ctx, rule)) {
            return denied;
        }
        if (platform_account_required && !is_platform_account(actor_merchant(req, ctx))) {
            // core/refunds.rs:2004-2010 / :2054-2061 — platform-only operation.
            return err(
                Kind::UNAUTHORIZED,
                "platform refunds are only accessible to platform merchants"
            );
        }
        return std::nullopt;
    }

    // ───────────────────────── Path / query helpers ─────────────────────────

    [[nodiscard]] bool is_v2(interfaces::io::IRequest& req) const
    {
        const auto path = app::http::RequestPath{}(req);
        return path.count() > 0 && path.at(0).value_or(std::string_view{}) == "v2";
    }

    [[nodiscard]] std::optional<std::string>
    refund_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        for (std::size_t i{0}; i + 1 < segments.count(); ++i) {
            const auto current = segments.at(i).value_or(std::string_view{});
            if (current != "refunds") {
                continue;
            }
            const auto candidate = segments.at(i + 1);
            if (candidate && !candidate->empty() && *candidate != "v2" && *candidate != "list" &&
                *candidate != "sync" && *candidate != "filter" && *candidate != "aggregate" &&
                *candidate != "platform" && *candidate != "profile") {
                return std::string{*candidate};
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] RefundRequestFacts request_facts(interfaces::io::IRequest& req) const
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        RefundRequestFacts facts{};
        facts.ctx_profile_id = ctx.profile_id();
        facts.ctx_merchant_id = ctx.merchant_id();
        facts.header_merchant_id = app::http::merchant_id_from_header{}(req);
        facts.request_path = std::string{req.get_path()};
        return facts;
    }

    [[nodiscard]] std::string json_string(std::string_view json, std::string_view key) const
    {
        const std::string pattern = std::format("\"{}\"", key);
        const auto pos = json.find(pattern);
        if (pos == std::string_view::npos) {
            return {};
        }
        auto cursor = pos + pattern.size();
        while (cursor < json.size() &&
               (json[cursor] == ':' || std::isspace(static_cast<unsigned char>(json[cursor])))) {
            ++cursor;
        }
        if (cursor >= json.size() || json[cursor] != '"') {
            return {};
        }
        ++cursor;
        const auto end = json.find('"', cursor);
        return end == std::string_view::npos ? std::string{}
                                             : std::string{json.substr(cursor, end - cursor)};
    }

    void apply_platform_query(
        const app::http::QueryParams& query,
        app::model::refunds::PlatformRefundListRequest& out
    ) const
    {
        if (const auto v = query.get("merchant_id")) {
            app::model::refunds::MerchantIdValue merchant{};
            merchant.set_value(*v);
            out.set_processor_merchant_id(merchant);
        }
        if (const auto v = query.get("payment_id")) {
            app::model::refunds::PaymentIdValue payment{};
            payment.set_value(*v);
            out.set_payment_id(payment);
        }
        if (const auto v = query.get("refund_id")) {
            out.set_refund_id(v);
        }
        if (const auto v = query.get("limit")) {
            app::model::refunds::PageSizeValue limit{};
            limit.set_value(*v);
            out.set_limit(limit);
        }
        app::model::refunds::PageOffsetValue offset{};
        offset.set_value(query.get("offset").value_or(std::string{}));
        out.set_offset(offset);
    }

private:
    std::optional<Error>
    check_auth_credentials(const app::auth::AuthContext& ctx, const Rule& rule) const
    {
        switch (rule.scheme()) {
            case Scheme::API_KEY:
            case Scheme::V2_API_KEY:
                if (!ctx.claim(app::auth::kClaimApiKey).has_value()) {
                    return err(Kind::UNAUTHORIZED, "missing x-api-key header");
                }
                break;
            case Scheme::JWT:
            case Scheme::JWT_AND_EMBEDDED:
            case Scheme::EMBEDDED:
                if (ctx.token().is_empty()) {
                    return err(Kind::UNAUTHORIZED, "missing bearer token");
                }
                break;
            case Scheme::MERCHANT_ID:
                if (ctx.merchant_id().empty()) {
                    return err(Kind::UNAUTHORIZED, "missing x-merchant-id header");
                }
                break;
            case Scheme::PLATFORM_ADMIN:
                if (!ctx.claim(app::auth::kClaimInternalApiKey).has_value() &&
                    ctx.token().is_empty()) {
                    return err(Kind::UNAUTHORIZED, "admin credentials required");
                }
                break;
            case Scheme::HEADER_MERCHANT:
                if (!rule.allow_platform() && ctx.merchant_id().empty()) {
                    const auto connected = rule.allow_connected()
                                               ? ctx.claim(app::auth::kClaimConnectedMerchantId)
                                               : std::nullopt;
                    if (!connected || connected->empty()) {
                        return err(Kind::UNAUTHORIZED, "missing x-merchant-id header");
                    }
                }
                break;
            case Scheme::NO_AUTH:
                return std::nullopt;
            default:
                break;
        }
        return std::nullopt;
    }

    std::string
    actor_merchant(interfaces::io::IRequest& req, const app::auth::AuthContext& ctx) const
    {
        auto merchant = app::http::merchant_id_from_header{}(req);
        if (merchant.empty()) {
            merchant = ctx.merchant_id();
        }
        if (merchant.empty()) {
            merchant = actor_merchant_hint();
        }
        return merchant;
    }

    std::string actor_merchant_hint() const
    {
        return m_context.config_store()
            .get_value("default_merchant_id", "refund")
            .value_or(std::string{});
    }

    bool is_platform_account(std::string_view merchant) const
    {
        if (merchant.empty()) {
            return false;
        }
        const auto account = m_context.repository().find_merchant_account_by_string(merchant);
        if (!account) {
            return false;
        }
        std::string type{};
        for (const char ch: account->get_merchant_account_type()) {
            type.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
        return type == "platform";
    }

    bool config_bool(std::string_view key, std::string_view ns) const
    {
        return m_context.config_store().get_value(ns, key).value_or("false") == "true";
    }

    Error err(Kind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    app::context::AppContext& m_context;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old core call received, plus the auth/request
// facts the old core derived from IRequest internally.

struct RefundsCreateInput
{
    bool is_v2{};
    std::string payment_id{};
    std::optional<app::model::refunds::RefundRequest> v1_payload{};
    std::optional<app::model::refunds::RefundsCreateRequest> v2_payload{};
    RefundRequestFacts facts{};
};

struct RefundsRetrieveInput
{
    app::model::refunds::RefundsRetrieveRequest request{};
    bool include_raw_requested{};
    RefundRequestFacts facts{};
};

struct RefundsRetrieveGatewayInput
{
    app::model::refunds::RefundsRetrieveRequest request{};
    bool include_raw_requested{};
    RefundRequestFacts facts{};
};

struct RefundsRetrieveBodyInput
{
    app::model::refunds::RefundsRetrieveRequest request{};
    RefundRequestFacts facts{};
};

struct RefundsUpdateInput
{
    app::model::refunds::RefundUpdateRequest request{};
};

struct RefundsReverseInput
{
    app::model::refunds::RefundReverseRequest request{};
};

struct RefundsMetadataUpdateInput
{
    std::string refund_id{};
    app::model::refunds::RefundMetadataUpdateRequest request{};
};

struct RefundsListInput
{
    app::model::refunds::RefundListRequest request{};
    bool profile_scoped{};
    RefundRequestFacts facts{};
};

struct RefundsListPlatformInput
{
    app::model::refunds::PlatformRefundListRequest request{};
    RefundRequestFacts facts{};
};

struct RefundsFilterPlatformInput
{
    RefundRequestFacts facts{};
};

struct RefundsListProfileInput
{
    app::model::refunds::RefundListRequest request{};
    RefundRequestFacts facts{};
};

struct RefundsFilterListInput
{
    std::string start_time{};
    std::string end_time{};
    RefundRequestFacts facts{};
};

struct RefundsFiltersInput
{
    RefundRequestFacts facts{};
};

struct RefundsFiltersProfileInput
{
    RefundRequestFacts facts{};
};

struct RefundsAggregatesInput
{
    std::optional<std::string> start_time{};
    std::optional<std::string> end_time{};
    RefundRequestFacts facts{};
};

struct RefundsAggregatesProfileInput
{
    std::optional<std::string> start_time{};
    std::optional<std::string> end_time{};
    RefundRequestFacts facts{};
};

struct RefundsManualUpdateInput
{
    app::model::refunds::RefundManualUpdateRequest request{};
};

} // namespace app::handlers::refunds
