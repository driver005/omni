export module app_payment:mandates_input;

import std;
import interfaces;
import core_error;
import app_context;
import app_auth;
import app_http;
import app_model;

export namespace app::handlers::mandates {

// Upstream routes/mandates.rs extractor modes: get/revoke use HeaderAuth(ApiKeyAuth),
// list adds JWTAuth{MerchantMandateRead}. Kept here as a parse-time concern only.
enum class MandateAuthMode
{
    ApiKey,
    ApiKeyOrJwtMandateRead,
};

/**
 * @brief Parse/auth/path/query support for the mandates handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime. Auth, path and query checks mirror
 * routes/mandates.rs:14-40,42-68,69-101 and api_models/mandates.rs:139-179.
 */
class MandatesInputSupport
{
public:
    explicit MandatesInputSupport(::app::context::AppContext& /*context*/) noexcept {}

    [[nodiscard]] MandateAuthMode auth_mode_for(std::string_view operation) const noexcept
    {
        // HeaderAuth(ApiKeyAuth{allow_connected_scope_operation:false,
        // allow_platform_self_operation:false}) for get/revoke (mandates.rs:33-36,61-64).
        // auth_type(HeaderAuth(...), JWTAuth{MerchantMandateRead, allow_connected:false,
        // allow_platform:false}) for list (mandates.rs:86-97).
        return operation == "retrieve_mandates_list" ? MandateAuthMode::ApiKeyOrJwtMandateRead
                                                     : MandateAuthMode::ApiKey;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    authorize(const ::app::auth::AuthContext& auth, std::string_view operation) const
    {
        const auto api_key = auth.claim(::app::auth::kClaimApiKey);
        const bool has_api_key = api_key.has_value() && !api_key->empty();
        const bool has_token = !auth.token().is_empty();
        const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId);
        const bool has_connected = connected.has_value() && !connected->empty();
        const bool has_platform = !auth.org_id().empty();

        if (auth_mode_for(operation) == MandateAuthMode::ApiKey) {
            // HeaderAuth(ApiKeyAuth): the local API-key claim or bearer token is the
            // credential. Both scope-allowing flags are false upstream.
            if (has_connected) {
                return make_error(
                    core::error::AppErrorKind::UNAUTHORIZED,
                    "connected scope operation is not allowed for mandates"
                );
            }
            if (has_platform) {
                return make_error(
                    core::error::AppErrorKind::UNAUTHORIZED,
                    "platform self operation is not allowed for mandates"
                );
            }
            if (!has_api_key && !has_token) {
                return make_error(core::error::AppErrorKind::UNAUTHORIZED, "missing API key");
            }
            return std::nullopt;
        }

        // List: API key path first (authenticated, no connected/platform scope).
        if (has_api_key && !has_connected && !has_platform) {
            return std::nullopt;
        }
        if (has_connected || has_platform) {
            return make_error(
                core::error::AppErrorKind::UNAUTHORIZED,
                "connected/platform scope is not allowed for mandate list"
            );
        }
        if (!has_token) {
            return make_error(core::error::AppErrorKind::UNAUTHORIZED, "missing bearer token");
        }
        return check_mandate_permission(auth, "MerchantMandateRead");
    }

    /** @brief Mirrors services/authorization.rs check_permission for the JWT branch. */
    [[nodiscard]] std::optional<core::error::AppError>
    check_mandate_permission(const ::app::auth::AuthContext& auth, std::string_view required) const
    {
        const auto single = auth.claim("permission");
        const auto list = auth.claim("permissions");
        if (!single.has_value() && !list.has_value()) {
            return std::nullopt;
        }
        const auto grants = [required](std::string_view granted)
        {
            std::string_view rest{granted};
            while (!rest.empty()) {
                const std::size_t comma{rest.find(',')};
                const std::string_view token{rest.substr(0, comma)};
                if (token == required || token == "ADMIN") {
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
        return make_error(
            core::error::AppErrorKind::FORBIDDEN,
            std::format("Access forbidden: {}", required)
        );
    }

    [[nodiscard]] std::string
    merchant_scope(interfaces::io::IRequest& req, const ::app::auth::AuthContext& auth) const
    {
        const auto header_id = ::app::http::merchant_id_from_header{}(req);
        return header_id.empty() ? auth.merchant_id() : header_id;
    }

    [[nodiscard]] std::optional<std::string>
    mandate_id_from_path(interfaces::io::IRequest& req, std::size_t expected_index) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        if (const auto segment = segments.at(expected_index)) {
            if (!segment->empty()) {
                return std::string{*segment};
            }
        }
        for (std::size_t i{0}; i < segments.count(); ++i) {
            if (segments.at(i).value_or(std::string_view{}) != "mandates") {
                continue;
            }
            for (std::size_t j{i + 1}; j < segments.count(); ++j) {
                const auto candidate = segments.at(j).value_or(std::string_view{});
                if (!candidate.empty() && candidate != "revoke" && candidate != "list") {
                    return std::string{candidate};
                }
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::size_t>
    parse_positive(const std::optional<std::string>& raw) const
    {
        if (!raw.has_value() || raw->empty()) {
            return std::nullopt;
        }
        std::size_t value{};
        const auto [end, ec] = std::from_chars(raw->data(), raw->data() + raw->size(), value);
        if (ec != std::errc{} || end != raw->data() + raw->size() || value == 0) {
            return std::nullopt;
        }
        return value;
    }

    /**
     * @brief web::Query<MandateListConstraints> (routes/mandates.rs:74;
     * api_models/mandates.rs:139-179). Pure query parsing, no business logic.
     */
    [[nodiscard]] ::app::model::mandates::MandateListConstraints
    constraints_from_query(const ::app::http::QueryParams& query) const
    {
        using Constraints = ::app::model::mandates::MandateListConstraints;
        Constraints constraints{};
        if (const auto limit = parse_positive(query.get("limit"))) {
            constraints.set_limit(static_cast<std::int64_t>(*limit));
        }
        if (const auto offset = parse_positive(query.get("offset"))) {
            constraints.set_offset(static_cast<std::int64_t>(*offset));
        }
        if (const auto status = query.get("mandate_status")) {
            ::app::model::mandates::MandateStatusValue value{};
            value.set_value(*status);
            constraints.set_mandate_status(std::move(value));
        }
        constraints.set_connector(query.get("connector"));
        if (const auto v = query.get("created_time")) {
            ::app::model::mandates::PrimitiveDateTimeValue t{};
            t.set_value(*v);
            constraints.set_created_time(std::move(t));
        }
        if (const auto v = query.get("created_time.lt")) {
            ::app::model::mandates::PrimitiveDateTimeValue t{};
            t.set_value(*v);
            constraints.set_created_time_lt(std::move(t));
        }
        if (const auto v = query.get("created_time.gt")) {
            ::app::model::mandates::PrimitiveDateTimeValue t{};
            t.set_value(*v);
            constraints.set_created_time_gt(std::move(t));
        }
        if (const auto v = query.get("created_time.lte")) {
            ::app::model::mandates::PrimitiveDateTimeValue t{};
            t.set_value(*v);
            constraints.set_created_time_lte(std::move(t));
        }
        if (const auto v = query.get("created_time.gte")) {
            ::app::model::mandates::PrimitiveDateTimeValue t{};
            t.set_value(*v);
            constraints.set_created_time_gte(std::move(t));
        }
        return constraints;
    }

    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old inline core block received (merchant
// scope, mandate id and — for list — the parsed constraints plus effective filter).

struct GetMandateInput
{
    std::string merchant_id{};
    std::string mandate_id{};
};

struct RevokeMandateInput
{
    std::string merchant_id{};
    std::string mandate_id{};
};

struct RetrieveMandatesListInput
{
    ::app::model::mandates::MandateListConstraints constraints{};
    std::optional<std::string> merchant_filter{};
};

} // namespace app::handlers::mandates
