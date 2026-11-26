export module app_payment:disputes_input;

import std;
import interfaces;
import core_router;
import core_serde;
import core_cache;
import core_database;
import core_error;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import app_http;
import app_metrics;
import app_store;
import app_connector;
import app_workflow;
import app_events;
import app_locking;
import app_services;

export namespace app::handlers::disputes {

using Error = core::error::AppError;
using Kind = core::error::AppErrorKind;
using Record = ::app::store::disputes::DisputeRecord;
using Query = ::app::http::QueryParams;
using Auth = ::app::auth::AuthContext;
using Perm = ::app::auth::AppPermission;
using Rule = ::app::auth::RouteAuthRule;

/** @brief Upstream `domain::MerchantAccountType` (common_enums/accounts.rs:45-50). */
enum class AccountType : std::uint8_t
{
    STANDARD,
    PLATFORM,
    CONNECTED
};

/**
 * @brief Exact flag set from one upstream `auth::auth_type(...)` call.
 * @note `routes/disputes.rs` passes one of two shapes to every route:
 *   - HeaderAuth(ApiKeyAuth{allow_connected_scope_operation, allow_platform_self_operation})
 *   - JWTAuth{permission, allow_connected, allow_platform}
 * The upstream permission name is preserved so the permission claim check
 * uses the exact upstream value (ProfileDisputeRead/Write, MerchantDisputeRead).
 */
struct DisputeAuthSpec
{
    Perm permission{Perm::DISPUTE_READ};
    std::string_view upstream_permission{"ProfileDisputeRead"};
    bool allow_connected{true};
    bool allow_platform{false};
    bool allow_connected_scope_operation{true};
    bool allow_platform_self_operation{false};
};

inline constexpr DisputeAuthSpec
    kProfileRead{Perm::DISPUTE_READ, "ProfileDisputeRead", true, false, true, false};
inline constexpr DisputeAuthSpec
    kProfileWrite{Perm::DISPUTE_WRITE, "ProfileDisputeWrite", true, false, true, false};
inline constexpr DisputeAuthSpec
    kMerchantRead{Perm::DISPUTE_READ, "MerchantDisputeRead", true, false, true, false};
inline constexpr DisputeAuthSpec
    kPlatformMerchantRead{Perm::DISPUTE_READ, "MerchantDisputeRead", false, true, false, true};

/** @brief Caller identity resolved from the authenticated context. */
struct DisputeScope
{
    std::string merchant_id{};
    std::string profile_id{};
    std::string org_id{};
    std::string connected_merchant_id{};
};

/** @brief Parse/auth/path/query support for the disputes handlers. */
class DisputesInputSupport
{
public:
    explicit DisputesInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] bool is_jwt_auth(interfaces::io::IRequest& req) const
    {
        const auto header = req.find_header(std::string_view{"authorization"});
        return header.starts_with("Bearer") || header.starts_with("bearer");
    }

    [[nodiscard]] AccountType account_type_of(const DisputeScope& scope) const
    {
        const auto merchant =
            m_context.repository().find_merchant_account_by_string(scope.merchant_id);
        if (!merchant) {
            return AccountType::STANDARD;
        }
        const auto& type = merchant->get_merchant_account_type();
        if (type == "platform") {
            return AccountType::PLATFORM;
        }
        if (type == "connected") {
            return AccountType::CONNECTED;
        }
        return AccountType::STANDARD;
    }

    /** @brief Upstream `check_merchant_access` / `check_merchant_access_for_jwt`. */
    [[nodiscard]] std::optional<Error>
    check_merchant_access(const DisputeScope& scope, const DisputeAuthSpec& spec, bool jwt) const
    {
        const AccountType type = account_type_of(scope);
        if (type == AccountType::STANDARD) {
            return std::nullopt;
        }
        const bool allowed =
            jwt ? (type == AccountType::CONNECTED ? spec.allow_connected : spec.allow_platform)
                : (type == AccountType::CONNECTED ? spec.allow_connected_scope_operation
                                                  : (scope.connected_merchant_id.empty()
                                                         ? spec.allow_platform_self_operation
                                                         : spec.allow_connected_scope_operation));
        if (allowed) {
            return std::nullopt;
        }
        m_context.metrics().counter("disputes_auth_merchant_type_denied").increment();
        const std::string message =
            type == AccountType::CONNECTED
                ? "Connected Merchant is not authorized to access the resource"
                : "Platform Merchant is not authorized to access the resource";
        return core::error::AppErrorBuilder{}(Kind::FORBIDDEN, message);
    }

    /** @brief Upstream JWT permission claim check against the exact permission name. */
    [[nodiscard]] std::optional<Error>
    check_permission_claim(const Auth& ctx, const DisputeAuthSpec& spec) const
    {
        const auto single = ctx.claim("permission");
        const auto list = ctx.claim("permissions");
        if (!single.has_value() && !list.has_value()) {
            return std::nullopt;
        }
        const auto grants = [name = spec.upstream_permission](std::string_view granted)
        {
            std::string_view rest = granted;
            while (!rest.empty()) {
                const std::size_t comma = rest.find(',');
                const std::string_view token = rest.substr(0, comma);
                if (token == name || token == "ADMIN") {
                    return true;
                }
                if (comma == std::string_view::npos) {
                    break;
                }
                rest = rest.substr(comma + 1);
            }
            return false;
        };
        if ((single.has_value() && grants(*single)) || (list.has_value() && grants(*list))) {
            return std::nullopt;
        }
        return core::error::AppErrorBuilder{}(
            Kind::FORBIDDEN,
            std::string{"permission "} + std::string{spec.upstream_permission} + " not granted"
        );
    }

    /** @brief Reproduces one upstream `auth_type(HeaderAuth(ApiKeyAuth), JWTAuth, headers)`. */
    [[nodiscard]] std::optional<Error> authorize_dispute(
        interfaces::io::IRequest& req,
        const Auth& ctx,
        const DisputeAuthSpec& spec
    ) const
    {
        const auto scope = scope_of(ctx, ::app::http::RequestQuery{}(req), req);
        const bool jwt = is_jwt_auth(req);
        if (jwt) {
            // Use a JWT rule with ADMIN permission so the shared authorizer only
            // enforces bearer presence; the granular upstream permission is checked below.
            const Rule rule = ::app::auth::RouteAuthRuleBuilder{}
                                  .scheme(::app::auth::AuthScheme::JWT)
                                  .permission(Perm::ADMIN)
                                  .allow_connected(spec.allow_connected)
                                  .allow_platform(spec.allow_platform)
                                  .build();
            if (const auto denied = m_context.authorizer().authorize_route(ctx, rule)) {
                return denied;
            }
            if (const auto denied = check_permission_claim(ctx, spec)) {
                return denied;
            }
        } else {
            const auto api_key = ctx.claim(::app::auth::kClaimApiKey);
            if (!api_key.has_value() || api_key->empty()) {
                return core::error::AppErrorBuilder{}(
                    Kind::UNAUTHORIZED,
                    "API key not provided or invalid API key used"
                );
            }
        }
        return check_merchant_access(scope, spec, jwt);
    }

    [[nodiscard]] DisputeScope
    scope_of(const Auth& auth, const Query& query, interfaces::io::IRequest& req) const
    {
        DisputeScope scope{};
        // Upstream resolves merchant/profile from the authenticated credential;
        // the query profile_id is a list constraint, handled in list_constraints_from.
        scope.merchant_id = auth.merchant_id();
        if (scope.merchant_id.empty()) {
            scope.merchant_id = ::app::http::merchant_id_from_header{}(req);
        }
        scope.org_id = auth.org_id();
        if (scope.org_id.empty()) {
            scope.org_id = ::app::http::org_id_from_header{}(req);
        }
        scope.profile_id = auth.profile_id();
        const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId);
        scope.connected_merchant_id = connected ? std::string{*connected} : std::string{};
        (void)query;
        return scope;
    }

    [[nodiscard]] std::optional<Error>
    scope_error(const DisputeScope& scope, bool require_platform) const
    {
        if (!require_platform) {
            return std::nullopt;
        }
        // Local mirror of `platform.get_provider().get_account().is_platform_account()`
        // (core/disputes.rs:299-305,345-352): the provider account must be a platform.
        if (account_type_of(scope) != AccountType::PLATFORM && scope.org_id.empty()) {
            m_context.metrics().counter("disputes_platform_guard_denied").increment();
            return core::error::AppErrorBuilder{}(Kind::UNAUTHORIZED, "Unauthorized");
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<Error>
    parse_fetch_window(const ::app::model::disputes::DisputeFetchQueryData& payload) const
    {
        // core/disputes.rs:968-980 — InvalidDataFormat { field_name, expected_format }.
        const auto invalid = [](std::string_view field)
        {
            return core::error::AppErrorBuilder{}(
                Kind::VALIDATION_FAILED,
                std::string{field} +
                    " contains invalid data. Expected format is YYYY-MM-DDTHH:MM:SS"
            );
        };
        if (!is_rfc3339_like(payload.get_fetch_from())) {
            return invalid("fetch_from");
        }
        if (!is_rfc3339_like(payload.get_fetch_till())) {
            return invalid("fetch_till");
        }
        return std::nullopt;
    }

    [[nodiscard]] ::app::model::disputes::DisputeListGetConstraints
    list_constraints_from(const Query& query) const
    {
        using namespace ::app::model::disputes;
        DisputeListGetConstraints constraints{};
        PageSizeValue limit{};
        limit.set_value(query.get("limit").value_or(std::string{"10"}));
        constraints.set_limit(limit);
        PageOffsetValue offset{};
        offset.set_value(query.get("offset").value_or(std::string{"0"}));
        constraints.set_offset(offset);
        if (const auto v = query.get("dispute_id")) {
            constraints.set_dispute_id(*v);
        }
        if (const auto v = query.get("payment_id")) {
            PaymentIdValue payment{};
            payment.set_value(*v);
            constraints.set_payment_id(payment);
        }
        if (const auto v = query.get("reason")) {
            constraints.set_reason(*v);
        }
        if (const auto v = query.get("dispute_status")) {
            std::vector<DisputeStatusValue> values{};
            for (const auto& s: split_commas(*v)) {
                DisputeStatusValue status{};
                status.set_value(canonical_status(s));
                values.push_back(status);
            }
            constraints.set_dispute_status(values);
        }
        if (const auto v = query.get("dispute_stage")) {
            std::vector<DisputeStageValue> values{};
            for (const auto& s: split_commas(*v)) {
                DisputeStageValue stage{};
                stage.set_value(canonical_stage(s));
                values.push_back(stage);
            }
            constraints.set_dispute_stage(values);
        }
        if (const auto v = query.get("connector")) {
            constraints.set_connector(split_commas(*v));
        }
        if (const auto v = query.get("profile_id")) {
            ProfileIdValue profile{};
            profile.set_value(*v);
            constraints.set_profile_id(profile);
        }
        const auto range = time_range_from(query);
        if (range.get_start_time().has_value() || range.get_end_time().has_value()) {
            TimeRangeValue tr{};
            tr.set_value(range.get_start_time().value_or(std::string{}));
            constraints.set_time_range(tr);
        }
        return constraints;
    }

    [[nodiscard]] ::app::model::disputes::PlatformDisputeListConstraints
    platform_constraints_from(const Query& query) const
    {
        using namespace ::app::model::disputes;
        PlatformDisputeListConstraints constraints{};
        PageSizeValue limit{};
        limit.set_value(query.get("limit").value_or(std::string{"10"}));
        constraints.set_limit(limit);
        PageOffsetValue offset{};
        offset.set_value(query.get("offset").value_or(std::string{"0"}));
        constraints.set_offset(offset);
        if (const auto v = query.get("dispute_id")) {
            constraints.set_dispute_id(*v);
        }
        if (const auto v = query.get("payment_id")) {
            PaymentIdValue payment{};
            payment.set_value(*v);
            constraints.set_payment_id(payment);
        }
        if (const auto v = query.get("reason")) {
            constraints.set_reason(*v);
        }
        if (const auto v = query.get("dispute_status")) {
            std::vector<DisputeStatusValue> values{};
            for (const auto& s: split_commas(*v)) {
                DisputeStatusValue status{};
                status.set_value(canonical_status(s));
                values.push_back(status);
            }
            constraints.set_dispute_status(values);
        }
        if (const auto v = query.get("dispute_stage")) {
            std::vector<DisputeStageValue> values{};
            for (const auto& s: split_commas(*v)) {
                DisputeStageValue stage{};
                stage.set_value(canonical_stage(s));
                values.push_back(stage);
            }
            constraints.set_dispute_stage(values);
        }
        if (const auto v = query.get("connector")) {
            constraints.set_connector(split_commas(*v));
        }
        if (const auto v = query.get("profile_id")) {
            ProfileIdValue profile{};
            profile.set_value(*v);
            constraints.set_profile_id(profile);
        }
        return constraints;
    }

    [[nodiscard]] ::app::model::disputes::TimeRange time_range_from(const Query& query) const
    {
        ::app::model::disputes::TimeRange range{};
        if (const auto v = query.get("start_time")) {
            range.set_start_time(*v);
        }
        if (const auto v = query.get("end_time")) {
            range.set_end_time(*v);
        }
        return range;
    }

    [[nodiscard]] std::string canonical_status(std::string_view value) const
    {
        const std::pair<std::string_view, std::string_view> map[] = {
            {"Accepted", "dispute_accepted"},
            {"Won", "dispute_won"},
            {"Lost", "dispute_lost"},
            {"Opened", "dispute_opened"},
            {"Challenged", "dispute_challenged"},
            {"Cancelled", "dispute_cancelled"},
            {"Expired", "dispute_expired"},
            {"NeedResponse", "dispute_challenged"},
        };
        for (const auto& [from, to]: map) {
            if (value == from) {
                return std::string{to};
            }
        }
        return std::string{value};
    }

    [[nodiscard]] std::string canonical_stage(std::string_view value) const
    {
        const std::pair<std::string_view, std::string_view> map[] = {
            {"Dispute", "dispute"},
            {"PreDispute", "pre_dispute"},
            {"PreArbitration", "pre_arbitration"},
            {"Arbitration", "arbitration"},
            {"DisputeReversal", "dispute_reversal"},
        };
        for (const auto& [from, to]: map) {
            if (value == from) {
                return std::string{to};
            }
        }
        return std::string{value};
    }

    [[nodiscard]] bool is_rfc3339_like(std::string_view value) const
    {
        // YYYY-MM-DDTHH:MM:SS (core/disputes.rs:968-980).
        if (value.size() != 19) {
            return false;
        }
        for (std::size_t i = 0; i < value.size(); ++i) {
            const char ch = value[i];
            if (i == 4 || i == 7) {
                if (ch != '-') {
                    return false;
                }
                continue;
            }
            if (i == 10) {
                if (ch != 'T') {
                    return false;
                }
                continue;
            }
            if (i == 13 || i == 16) {
                if (ch != ':') {
                    return false;
                }
                continue;
            }
            if (ch < '0' || ch > '9') {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] std::vector<std::string> split_commas(std::string_view text) const
    {
        std::vector<std::string> parts{};
        for (const auto part: std::views::split(text, ',')) {
            parts.emplace_back(std::ranges::begin(part), std::ranges::end(part));
        }
        return parts;
    }

    /** @brief Path segment counted from the end; 0 = last, 1 = second-to-last. */
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

private:
    ::app::context::AppContext& m_context;
};

/** @brief Local serializable shape for the multipart evidence upload body. */
class EvidenceUploadWire
{
public:
    void set_dispute_id(std::optional<std::string> value)
    {
        dispute_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_dispute_id() const noexcept
    {
        return dispute_id;
    }

    void set_file_id(std::optional<std::string> value)
    {
        file_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_file_id() const noexcept
    {
        return file_id;
    }

    void set_file_name(std::optional<std::string> value)
    {
        file_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_file_name() const noexcept
    {
        return file_name;
    }

    void set_evidence_type(std::optional<std::string> value)
    {
        evidence_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_evidence_type() const noexcept
    {
        return evidence_type;
    }

    void set_file_size(std::int32_t value)
    {
        file_size = std::move(value);
    }

    [[nodiscard]] const std::int32_t& get_file_size() const noexcept
    {
        return file_size;
    }

    void set_file_type(std::optional<std::string> value)
    {
        file_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_file_type() const noexcept
    {
        return file_type;
    }

    std::optional<std::string> dispute_id{};
    std::optional<std::string> file_id{};
    std::optional<std::string> file_name{};
    std::optional<std::string> evidence_type{};
    std::int32_t file_size{};
    std::optional<std::string> file_type{};
};

struct RetrieveDisputeInput
{
    DisputeScope scope{};
    ::app::model::disputes::DisputeRetrieveRequest request{};
};

struct FetchDisputesInput
{
    DisputeScope scope{};
    std::string connector_id{};
    ::app::model::disputes::DisputeFetchQueryData payload{};
};

struct RetrieveDisputesListInput
{
    DisputeScope scope{};
    ::app::model::disputes::DisputeListGetConstraints constraints{};
};

struct RetrieveDisputesListProfileInput
{
    DisputeScope scope{};
    ::app::model::disputes::DisputeListGetConstraints constraints{};
    std::optional<std::vector<std::string>> profile_id_list{};
};

struct RetrieveDisputesListForPlatformInput
{
    DisputeScope scope{};
    ::app::model::disputes::PlatformDisputeListConstraints constraints{};
};

struct GetDisputesFiltersInput
{
    DisputeScope scope{};
};

struct GetDisputesFiltersProfileInput
{
    DisputeScope scope{};
    std::optional<std::vector<std::string>> profile_id_list{};
};

struct GetPlatformDisputesFiltersInput
{
    DisputeScope scope{};
};

struct AcceptDisputeInput
{
    DisputeScope scope{};
    ::app::model::disputes::DisputeId dispute_id{};
};

struct SubmitDisputeEvidenceInput
{
    DisputeScope scope{};
    ::app::model::disputes::SubmitEvidenceRequest request{};
};

struct AttachDisputeEvidenceInput
{
    DisputeScope scope{};
    EvidenceUploadWire request{};
};

struct RetrieveDisputeEvidenceInput
{
    DisputeScope scope{};
    ::app::model::disputes::DisputeId dispute_id{};
};

struct DeleteDisputeEvidenceInput
{
    DisputeScope scope{};
    ::app::model::disputes::DeleteEvidenceRequest request{};
};

struct GetDisputesAggregateInput
{
    DisputeScope scope{};
    ::app::model::disputes::TimeRange time_range{};
};

struct GetDisputesAggregateProfileInput
{
    DisputeScope scope{};
    ::app::model::disputes::TimeRange time_range{};
    std::optional<std::vector<std::string>> profile_id_list{};
};


} // namespace app::handlers::disputes

template<>
struct serde::Serializable<::app::handlers::disputes::EvidenceUploadWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "dispute_id",
                &::app::handlers::disputes::EvidenceUploadWire::get_dispute_id,
                &::app::handlers::disputes::EvidenceUploadWire::set_dispute_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "file_id",
                &::app::handlers::disputes::EvidenceUploadWire::get_file_id,
                &::app::handlers::disputes::EvidenceUploadWire::set_file_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "file_name",
                &::app::handlers::disputes::EvidenceUploadWire::get_file_name,
                &::app::handlers::disputes::EvidenceUploadWire::set_file_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "evidence_type",
                &::app::handlers::disputes::EvidenceUploadWire::get_evidence_type,
                &::app::handlers::disputes::EvidenceUploadWire::set_evidence_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "file_size",
                &::app::handlers::disputes::EvidenceUploadWire::get_file_size,
                &::app::handlers::disputes::EvidenceUploadWire::set_file_size,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "file_type",
                &::app::handlers::disputes::EvidenceUploadWire::get_file_type,
                &::app::handlers::disputes::EvidenceUploadWire::set_file_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "evidence_upload_wire";
    }
};
