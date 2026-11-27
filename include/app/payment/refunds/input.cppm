module;
#include <uuid.h>

export module app_payment:refunds_input;

import std;
import core_behavior;
import interfaces;
import core_error;
import app_context;
import app_context_core;
import app_model;
import app_store;
import app_auth;
import app_http;

import :context;

export namespace app::handlers::refunds {

using Error = core::error::AppError;
using Kind = core::error::AppErrorKind;
using Record = ::app::model::Refund;
using Refund = ::app::model::Refund;
using Response = ::app::model::Refund;
using Status = ::app::model::RefundStatus;
using Intent = ::app::store::payments::PaymentIntentRecord;
using Attempt = ::app::store::payment_attempts::PaymentAttemptRecord;
using Rule = ::app::auth::RouteAuthRule;
using Scheme = ::app::auth::AuthScheme;
using Perm = ::app::auth::AppPermission;

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
    explicit RefundsInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ───────────────────────── Auth semantics ─────────────────────────
    // routes/refunds.rs auth_type(...) + the individual auth::* extractors.
    [[nodiscard]] std::optional<Error>
    authorize_op(interfaces::io::IRequest& req, AuthProfile profile) const
    {
        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        const bool api_key = ctx.claim(::app::auth::kClaimApiKey).has_value();
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
        const auto path = ::app::http::RequestPath{}(req);
        return path.count() > 0 && path.at(0).value_or(std::string_view{}) == "v2";
    }

    [[nodiscard]] std::optional<std::string>
    refund_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
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

    /**
     * @brief Parses the `{id}` path segment as a UUID. A malformed segment is an
     *        INVALID_REQUEST rather than a lookup miss.
     */
    [[nodiscard]] std::expected<uuids::uuid, Error>
    refund_uuid_from_path(interfaces::io::IRequest& req) const
    {
        const auto segment = refund_id_from_path(req);
        if (!segment) {
            return std::unexpected{err(Kind::INVALID_REQUEST, "missing refund id in path")};
        }
        const auto parsed = uuids::uuid::from_string(*segment);
        if (!parsed) {
            return std::unexpected{err(Kind::INVALID_REQUEST, "invalid refund id in path")};
        }
        return *parsed;
    }

    [[nodiscard]] RefundRequestFacts request_facts(interfaces::io::IRequest& req) const
    {
        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        RefundRequestFacts facts{};
        facts.ctx_profile_id = ctx.profile_id();
        facts.ctx_merchant_id = ctx.merchant_id();
        facts.header_merchant_id = ::app::http::merchant_id_from_header{}(req);
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

    void apply_platform_query(const ::app::http::QueryParams& query, Refund& out) const
    {
        if (const auto v = query.get("merchant_id")) {
            out.processor_merchant_id = *v;
        }
        if (const auto v = query.get("payment_id")) {
            out.payment_id = uuids::uuid::from_string(*v).value_or(uuids::uuid{});
        }
        if (const auto v = query.get("refund_id")) {
            out.refund_id = *v;
        }
        if (const auto v = query.get("limit")) {
            out.limit = *v;
        }
        out.offset = query.get("offset").value_or(std::string{});
    }

private:
    std::optional<Error>
    check_auth_credentials(const ::app::auth::AuthContext& ctx, const Rule& rule) const
    {
        switch (rule.scheme()) {
            case Scheme::API_KEY:
            case Scheme::V2_API_KEY:
                if (!ctx.claim(::app::auth::kClaimApiKey).has_value()) {
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
                if (!ctx.claim(::app::auth::kClaimInternalApiKey).has_value() &&
                    ctx.token().is_empty()) {
                    return err(Kind::UNAUTHORIZED, "admin credentials required");
                }
                break;
            case Scheme::HEADER_MERCHANT:
                if (!rule.allow_platform() && ctx.merchant_id().empty()) {
                    const auto connected = rule.allow_connected()
                                               ? ctx.claim(::app::auth::kClaimConnectedMerchantId)
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
    actor_merchant(interfaces::io::IRequest& req, const ::app::auth::AuthContext& ctx) const
    {
        auto merchant = ::app::http::merchant_id_from_header{}(req);
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

    ::app::context::AppContext& m_context;
};

struct RefundsCreateData
{
    bool is_v2{};
    std::string payment_id{};
    std::optional<Refund> v1_payload{};
    std::optional<Refund> v2_payload{};
    RefundRequestFacts facts{};

    explicit RefundsCreateData(const Refund& dto, const core::behavior::CallContext& ctx) :
        is_v2{!ctx.path_segments.empty() && ctx.path_segments.front() == "v2"},
        payment_id{uuids::to_string(dto.payment_id)},
        v1_payload{dto},
        v2_payload{dto},
        facts{}
    {
        (void)ctx;
    }
};

struct RefundsRetrieveData
{
    Refund request{};
    bool include_raw_requested{};
    RefundRequestFacts facts{};

    template<class Dto>
    explicit RefundsRetrieveData(const Dto& dto, const core::behavior::CallContext& ctx) :
        request{::app::payment::detail::as_dto<Refund>(dto)},
        include_raw_requested{},
        facts{}
    {
    }
};

struct RefundsRetrieveGatewayData
{
    Refund request{};
    bool include_raw_requested{};
    RefundRequestFacts facts{};

    template<class Dto>
    explicit RefundsRetrieveGatewayData(const Dto& dto, const core::behavior::CallContext& ctx) :
        request{::app::payment::detail::as_dto<Refund>(dto)},
        include_raw_requested{},
        facts{}
    {
    }
};

struct RefundsRetrieveBodyData
{
    Refund request{};
    RefundRequestFacts facts{};

    template<class Dto>
    explicit RefundsRetrieveBodyData(const Dto& dto, const core::behavior::CallContext& ctx) :
        request{::app::payment::detail::as_dto<Refund>(dto)},
        facts{}
    {
    }
};

struct RefundsUpdateData
{
    Refund request{};

    template<class Dto>
    explicit RefundsUpdateData(const Dto& dto, const core::behavior::CallContext& ctx) :
        request{::app::payment::detail::as_dto<Refund>(dto)}
    {
    }
};

struct RefundsReverseData
{
    Refund request{};

    template<class Dto>
    explicit RefundsReverseData(const Dto& dto, const core::behavior::CallContext& ctx) :
        request{::app::payment::detail::as_dto<Refund>(dto)}
    {
    }
};

struct RefundsMetadataUpdateData
{
    Refund request{};

    template<class Dto>
    explicit RefundsMetadataUpdateData(const Dto& dto, const core::behavior::CallContext& ctx) :
        request{::app::payment::detail::as_dto<Refund>(dto)}
    {
        (void)ctx;
    }
};

struct RefundsListData
{
    Refund request{};
    bool profile_scoped{};
    RefundRequestFacts facts{};

    template<class Dto>
    explicit RefundsListData(const Dto& dto, const core::behavior::CallContext& ctx) :
        request{::app::payment::detail::as_dto<Refund>(dto)},
        profile_scoped{},
        facts{}
    {
    }
};

struct RefundsListPlatformData
{
    Refund request{};
    RefundRequestFacts facts{};

    template<class Dto>
    explicit RefundsListPlatformData(const Dto& dto, const core::behavior::CallContext& ctx) :
        request{::app::payment::detail::as_dto<Refund>(dto)},
        facts{}
    {
    }
};

struct RefundsFilterPlatformData
{
    RefundRequestFacts facts{};

    explicit RefundsFilterPlatformData(const core::behavior::CallContext& ctx) :
        facts{}
    {
    }
};

struct RefundsListProfileData
{
    Refund request{};
    RefundRequestFacts facts{};

    template<class Dto>
    explicit RefundsListProfileData(const Dto& dto, const core::behavior::CallContext& ctx) :
        request{::app::payment::detail::as_dto<Refund>(dto)},
        facts{}
    {
    }
};

struct RefundsFilterListData
{
    std::string start_time{};
    std::string end_time{};
    RefundRequestFacts facts{};

    explicit RefundsFilterListData(const core::behavior::CallContext& ctx) :
        start_time{::app::payment::detail::last_path(ctx)},
        end_time{::app::payment::detail::last_path(ctx)},
        facts{}
    {
    }
};

struct RefundsFiltersData
{
    RefundRequestFacts facts{};

    explicit RefundsFiltersData(const core::behavior::CallContext& ctx) :
        facts{}
    {
    }
};

struct RefundsFiltersProfileData
{
    RefundRequestFacts facts{};

    explicit RefundsFiltersProfileData(const core::behavior::CallContext& ctx) :
        facts{}
    {
    }
};

struct RefundsAggregatesData
{
    std::optional<std::string> start_time{};
    std::optional<std::string> end_time{};
    RefundRequestFacts facts{};

    explicit RefundsAggregatesData(const core::behavior::CallContext& ctx) :
        start_time{std::optional<std::string>{::app::payment::detail::query_of(ctx, "start_time")}},
        end_time{std::optional<std::string>{::app::payment::detail::query_of(ctx, "end_time")}},
        facts{}
    {
    }
};

struct RefundsAggregatesProfileData
{
    std::optional<std::string> start_time{};
    std::optional<std::string> end_time{};
    RefundRequestFacts facts{};

    explicit RefundsAggregatesProfileData(const core::behavior::CallContext& ctx) :
        start_time{std::optional<std::string>{::app::payment::detail::query_of(ctx, "start_time")}},
        end_time{std::optional<std::string>{::app::payment::detail::query_of(ctx, "end_time")}},
        facts{}
    {
    }
};

struct RefundsManualUpdateData
{
    Refund request{};

    template<class Dto>
    explicit RefundsManualUpdateData(const Dto& dto, const core::behavior::CallContext& ctx) :
        request{::app::payment::detail::as_dto<Refund>(dto)}
    {
    }
};

/** @brief Request policy for the refunds domain: wraps the domain auth rules. */
class RefundsPolicy
{
public:
    RefundsPolicy(::app::context::AppContext& /*context*/, RefundsInputSupport& support) noexcept :
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
        const AuthProfile profile = profile_for(operation);
        return m_support.authorize_op(req, profile);
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

    [[nodiscard]] static AuthProfile profile_for(std::string_view operation) noexcept
    {
        if (operation == "V1Create") {
            return AuthProfile::V1Create;
        }
        if (operation == "V1Retrieve") {
            return AuthProfile::V1Retrieve;
        }
        if (operation == "V1Sync") {
            return AuthProfile::V1Sync;
        }
        if (operation == "V1Update") {
            return AuthProfile::V1Update;
        }
        if (operation == "V1Reverse") {
            return AuthProfile::V1Reverse;
        }
        if (operation == "V1List") {
            return AuthProfile::V1List;
        }
        if (operation == "Platform") {
            return AuthProfile::Platform;
        }
        if (operation == "ProfileList") {
            return AuthProfile::ProfileList;
        }
        if (operation == "Filter") {
            return AuthProfile::Filter;
        }
        if (operation == "Filters") {
            return AuthProfile::Filters;
        }
        if (operation == "FiltersProfile") {
            return AuthProfile::FiltersProfile;
        }
        if (operation == "Aggregates") {
            return AuthProfile::Aggregates;
        }
        if (operation == "AggregatesProfile") {
            return AuthProfile::AggregatesProfile;
        }
        if (operation == "Admin") {
            return AuthProfile::Admin;
        }
        if (operation == "V2Create") {
            return AuthProfile::V2Create;
        }
        if (operation == "V2Retrieve") {
            return AuthProfile::V2Retrieve;
        }
        if (operation == "V2Gateway") {
            return AuthProfile::V2Gateway;
        }
        if (operation == "V2Metadata") {
            return AuthProfile::V2Metadata;
        }
        return AuthProfile::V2List;
    }

private:
    RefundsInputSupport& m_support;
};

using RefundsRequestSupport = core::behavior::RequestSupport<RefundsPolicy>;
using RefundsPolicySpec = core::behavior::AuthSpec;

[[nodiscard]] RefundsPolicySpec spec_for(std::string_view operation) noexcept
{
    return RefundsPolicySpec{.permission = operation};
}
} // namespace app::handlers::refunds
