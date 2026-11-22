module;
export module app_handlers:disputes;

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

/** @brief Upstream `StatusOk` wire shape (delete-evidence success response). */
class StatusOkWire
{
public:
    void set_status(std::string value) { status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return status; }
    std::string status{};
};

/**
 * @brief Local serializable shape for the multipart evidence upload body.
 * @note The local HTTP layer carries JSON, not multipart. This normalizes the
 * file metadata that upstream `routes/disputes/utils.rs::get_attach_evidence_request`
 * derives from the multipart `file`/`dispute_id`/`evidence_type` fields.
 */
class EvidenceUploadWire
{
public:
    void set_dispute_id(std::optional<std::string> value) { dispute_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_dispute_id() const noexcept { return dispute_id; }
    void set_file_id(std::optional<std::string> value) { file_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_file_id() const noexcept { return file_id; }
    void set_file_name(std::optional<std::string> value) { file_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_file_name() const noexcept { return file_name; }
    void set_evidence_type(std::optional<std::string> value) { evidence_type = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_evidence_type() const noexcept { return evidence_type; }
    void set_file_size(std::int32_t value) { file_size = std::move(value); }
    [[nodiscard]] const std::int32_t& get_file_size() const noexcept { return file_size; }
    void set_file_type(std::optional<std::string> value) { file_type = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_file_type() const noexcept { return file_type; }
    std::optional<std::string> dispute_id{};
    std::optional<std::string> file_id{};
    std::optional<std::string> file_name{};
    std::optional<std::string> evidence_type{};
    std::int32_t file_size{};
    std::optional<std::string> file_type{};
};

} // namespace app::handlers::disputes

template<>
struct serde::Serializable<app::handlers::disputes::StatusOkWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"status", &app::handlers::disputes::StatusOkWire::get_status, &app::handlers::disputes::StatusOkWire::set_status, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "status_ok_wire"; }
};

template<>
struct serde::Serializable<app::handlers::disputes::EvidenceUploadWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"dispute_id", &app::handlers::disputes::EvidenceUploadWire::get_dispute_id, &app::handlers::disputes::EvidenceUploadWire::set_dispute_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"file_id", &app::handlers::disputes::EvidenceUploadWire::get_file_id, &app::handlers::disputes::EvidenceUploadWire::set_file_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"file_name", &app::handlers::disputes::EvidenceUploadWire::get_file_name, &app::handlers::disputes::EvidenceUploadWire::set_file_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"evidence_type", &app::handlers::disputes::EvidenceUploadWire::get_evidence_type, &app::handlers::disputes::EvidenceUploadWire::set_evidence_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"file_size", &app::handlers::disputes::EvidenceUploadWire::get_file_size, &app::handlers::disputes::EvidenceUploadWire::set_file_size, serde::FieldOptions{}>{},
            serde::FieldDesc<"file_type", &app::handlers::disputes::EvidenceUploadWire::get_file_type, &app::handlers::disputes::EvidenceUploadWire::set_file_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "evidence_upload_wire"; }
};

export namespace app::handlers::disputes {

/**
 * @brief Port of upstream `routes/disputes.rs` + `core/disputes.rs` (SHA 7736d55).
 *
 * Every upstream core function is reproduced here as a local helper using the
 * in-process facilities (AppContext/AppRepository/ConfigStore/ProcessTracker/
 * ConnectorRouter/EventBus/authorizer). External-only side effects that upstream
 * performs through Dsync/UCS/gateway/file-store/webhook services are modelled by
 * the corresponding local service; they are intentionally in-process.
 */
class DisputesRouteHandlers
{
    using Error = core::error::AppError;
    using Kind = core::error::AppErrorKind;
    using Record = app::store::disputes::DisputeRecord;
    using Query = app::http::QueryParams;
    using Auth = app::auth::AuthContext;
    using Perm = app::auth::AppPermission;
    using Rule = app::auth::RouteAuthRule;

    /** @brief Caller identity resolved from the authenticated context. */
    struct Scope
    {
        std::string merchant_id{};
        std::string profile_id{};
        std::string org_id{};
        std::string connected_merchant_id{};
    };

    /** @brief Upstream `domain::MerchantAccountType` (common_enums/accounts.rs:45-50). */
    enum class AccountType : std::uint8_t { STANDARD, PLATFORM, CONNECTED };

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

    // Static upstream enum iterators (common_enums::DisputeStage/DisputeStatus/Currency).
    inline static constexpr std::string_view kStages[] = {
        "pre_dispute", "dispute", "pre_arbitration", "arbitration", "dispute_reversal"
    };
    inline static constexpr std::string_view kStatuses[] = {
        "dispute_opened", "dispute_expired", "dispute_accepted", "dispute_cancelled",
        "dispute_challenged", "dispute_won", "dispute_lost"
    };
    // Upstream storage_enums::Currency::iter() — all ISO-4217 codes exposed by Hyperswitch.
    inline static constexpr std::string_view kCurrencies[] = {
        "AED","ARS","AUD","BGN","BRL","CAD","CHF","CNY","COP","CZK","DKK","EUR","GBP",
        "HKD","HUF","IDR","ILS","INR","ISK","JPY","KEN","KRW","MXN","MYR","NOK","NZD",
        "PHP","PLN","RON","SEK","SGD","THB","TRY","TWD","USD","UYU","VND","ZAR"
    };

public:
    // ── routes/disputes.rs:33-69  retrieve_dispute ────────────────────────────
    void handle_retrieve_dispute(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        if (const auto denied = authorize_dispute(req, ctx, kProfileRead)) { send_error(res, send, *denied); return; }
        const auto scope = scope_of(ctx, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        app::model::disputes::DisputeRetrieveRequest request{};
        request.set_dispute_id(path_segment(req));
        if (request.get_dispute_id().empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing dispute id"));
            return;
        }
        if (const auto force = query.get("force_sync")) {
            request.set_force_sync(*force == "true" || *force == "1");
        }

        m_context->metrics().record_flow("dispute_retrieve");
        auto response = retrieve_dispute(m_context->repository(), scope, request);
        if (!response) { send_error(res, send, response.error()); return; }
        send_json(res, send, serde::Ser::encode_json(*response), true);
    }

    // ── routes/disputes.rs:73-113  fetch_disputes ─────────────────────────────
    void handle_fetch_disputes(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        if (const auto denied = authorize_dispute(req, ctx, kProfileRead)) { send_error(res, send, *denied); return; }
        const auto scope = scope_of(ctx, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        const std::string connector_id = path_segment(req, 1);
        if (connector_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing connector id"));
            return;
        }

        // core/disputes.rs:968-980 — parse fetch_from/fetch_till as RFC3339-like timestamps.
        app::model::disputes::DisputeFetchQueryData payload{};
        payload.set_fetch_from(query.get("fetch_from").value_or(std::string{}));
        payload.set_fetch_till(query.get("fetch_till").value_or(std::string{}));
        if (const auto invalid = parse_fetch_window(payload)) { send_error(res, send, *invalid); return; }

        m_context->metrics().record_flow("dispute_fetch");
        m_context->metrics().counter("disputes_fetch_total").increment();
        auto response = connector_sync_disputes(scope, connector_id, payload);
        if (!response) { send_error(res, send, response.error()); return; }
        send_json(res, send, serde::Ser::encode_json(*response), true);
    }

    // ── routes/disputes.rs:140-175  retrieve_disputes_list ────────────────────
    void handle_retrieve_disputes_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        if (const auto denied = authorize_dispute(req, ctx, kMerchantRead)) { send_error(res, send, *denied); return; }
        const auto scope = scope_of(ctx, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        const auto constraints = list_constraints_from(query);
        m_context->metrics().record_flow("dispute_list");
        auto response = retrieve_disputes_list(scope, constraints, std::nullopt);
        if (!response) { send_error(res, send, response.error()); return; }
        send_json(res, send, serde::Ser::encode_json(*response), true);
    }

    // ── routes/disputes.rs:300-335  retrieve_disputes_list_profile ────────────
    void handle_retrieve_disputes_list_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        if (const auto denied = authorize_dispute(req, ctx, kProfileRead)) { send_error(res, send, *denied); return; }
        const auto scope = scope_of(ctx, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        const auto constraints = list_constraints_from(query);
        m_context->metrics().record_flow("dispute_profile_list");
        const std::vector<std::string> profiles = scope.profile_id.empty()
            ? std::vector<std::string>{} : std::vector<std::string>{scope.profile_id};
        auto response = retrieve_disputes_list(scope, constraints, profiles);
        if (!response) { send_error(res, send, response.error()); return; }
        send_json(res, send, serde::Ser::encode_json(*response), true);
    }

    // ── routes/disputes.rs:199-229  retrieve_disputes_list_for_platform ───────
    void handle_retrieve_disputes_list_for_platform(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        if (const auto denied = authorize_dispute(req, ctx, kPlatformMerchantRead)) { send_error(res, send, *denied); return; }
        auto scope = scope_of(ctx, query, req);
        if (const auto invalid = scope_error(scope, true)) { send_error(res, send, *invalid); return; }

        // core/disputes.rs:310 — PlatformDisputeListConstraints carries processor_merchant_id.
        auto constraints = platform_constraints_from(query);
        if (const auto processor = query.get("processor_merchant_id")) {
            app::model::disputes::MerchantIdValue merchant{};
            merchant.set_value(*processor);
            constraints.set_processor_merchant_id(merchant);
        }
        m_context->metrics().record_flow("dispute_platform_list");
        auto response = retrieve_disputes_list_for_platform(scope, constraints);
        if (!response) { send_error(res, send, response.error()); return; }
        send_json(res, send, serde::Ser::encode_json(*response), true);
    }

    // ── routes/disputes.rs:350-375  get_disputes_filters ──────────────────────
    void handle_get_disputes_filters(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        if (const auto denied = authorize_dispute(req, ctx, kMerchantRead)) { send_error(res, send, *denied); return; }
        const auto scope = scope_of(ctx, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        m_context->metrics().record_flow("dispute_filters");
        auto response = get_filters_for_disputes(scope, std::nullopt);
        if (!response) { send_error(res, send, response.error()); return; }
        send_json(res, send, serde::Ser::encode_json(*response), true);
    }

    // ── routes/disputes.rs:390-422  get_disputes_filters_profile ──────────────
    void handle_get_disputes_filters_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        if (const auto denied = authorize_dispute(req, ctx, kProfileRead)) { send_error(res, send, *denied); return; }
        const auto scope = scope_of(ctx, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        m_context->metrics().record_flow("dispute_profile_filters");
        const std::vector<std::string> profiles = scope.profile_id.empty()
            ? std::vector<std::string>{} : std::vector<std::string>{scope.profile_id};
        auto response = get_filters_for_disputes(scope, profiles);
        if (!response) { send_error(res, send, response.error()); return; }
        send_json(res, send, serde::Ser::encode_json(*response), true);
    }

    // ── routes/disputes.rs:244-272  get_platform_disputes_filters ─────────────
    void handle_get_platform_disputes_filters(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        if (const auto denied = authorize_dispute(req, ctx, kPlatformMerchantRead)) { send_error(res, send, *denied); return; }
        const auto scope = scope_of(ctx, query, req);
        if (const auto invalid = scope_error(scope, true)) { send_error(res, send, *invalid); return; }

        m_context->metrics().record_flow("dispute_platform_filters");
        auto response = get_platform_disputes_filters(scope);
        if (!response) { send_error(res, send, response.error()); return; }
        send_json(res, send, serde::Ser::encode_json(*response), true);
    }

    // ── routes/disputes.rs:441-479  accept_dispute ────────────────────────────
    void handle_accept_dispute(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        if (const auto denied = authorize_dispute(req, ctx, kProfileWrite)) { send_error(res, send, *denied); return; }
        const auto scope = scope_of(ctx, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        app::model::disputes::DisputeId dispute_id{};
        dispute_id.set_dispute_id(path_segment(req));
        if (dispute_id.get_dispute_id().empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing dispute id"));
            return;
        }
        m_context->metrics().record_flow("dispute_accept");
        auto response = accept_dispute(scope, dispute_id);
        if (!response) { send_error(res, send, response.error()); return; }
        send_json(res, send, serde::Ser::encode_json(*response), true);
    }

    // ── routes/disputes.rs:496-532  submit_dispute_evidence ───────────────────
    void handle_submit_dispute_evidence(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        if (const auto denied = authorize_dispute(req, ctx, kProfileWrite)) { send_error(res, send, *denied); return; }
        const auto scope = scope_of(ctx, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::deserialize<app::model::disputes::SubmitEvidenceRequest>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }
        m_context->metrics().record_flow("dispute_submit_evidence");
        auto response = submit_evidence(scope, *parsed);
        if (!response) { send_error(res, send, response.error()); return; }
        send_json(res, send, serde::Ser::encode_json(*response), true);
    }

    // ── routes/disputes.rs:550-586  attach_dispute_evidence ───────────────────
    void handle_attach_dispute_evidence(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        if (const auto denied = authorize_dispute(req, ctx, kProfileWrite)) { send_error(res, send, *denied); return; }
        const auto scope = scope_of(ctx, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::deserialize<EvidenceUploadWire>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }
        m_context->metrics().record_flow("dispute_attach_evidence");
        auto response = attach_evidence(scope, *parsed);
        if (!response) { send_error(res, send, response.error()); return; }
        send_json(res, send, serde::Ser::encode_json(*response), true);
    }

    // ── routes/disputes.rs:605-643  retrieve_dispute_evidence ─────────────────
    void handle_retrieve_dispute_evidence(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        if (const auto denied = authorize_dispute(req, ctx, kProfileRead)) { send_error(res, send, *denied); return; }
        const auto scope = scope_of(ctx, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        app::model::disputes::DisputeId dispute_id{};
        dispute_id.set_dispute_id(path_segment(req));
        if (dispute_id.get_dispute_id().empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "missing dispute id"));
            return;
        }
        m_context->metrics().record_flow("dispute_retrieve_evidence");
        auto response = retrieve_dispute_evidence(scope, dispute_id);
        if (!response) { send_error(res, send, response.error()); return; }
        send_json(res, send, serde::Ser::encode_json(*response), true);
    }

    // ── routes/disputes.rs:661-699  delete_dispute_evidence ───────────────────
    void handle_delete_dispute_evidence(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        if (const auto denied = authorize_dispute(req, ctx, kProfileWrite)) { send_error(res, send, *denied); return; }
        const auto scope = scope_of(ctx, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::deserialize<app::model::disputes::DeleteEvidenceRequest>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }
        m_context->metrics().record_flow("dispute_delete_evidence");
        auto error = delete_evidence(scope, *parsed);
        if (error) { send_error(res, send, *error); return; }
        StatusOkWire response{};
        response.set_status("success");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // ── routes/disputes.rs:702-738  get_disputes_aggregate ────────────────────
    void handle_get_disputes_aggregate(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        if (const auto denied = authorize_dispute(req, ctx, kMerchantRead)) { send_error(res, send, *denied); return; }
        const auto scope = scope_of(ctx, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        m_context->metrics().record_flow("dispute_aggregate");
        auto response = get_aggregates_for_disputes(scope, time_range_from(query), std::nullopt);
        if (!response) { send_error(res, send, response.error()); return; }
        send_json(res, send, serde::Ser::encode_json(*response), true);
    }

    // ── routes/disputes.rs:742-778  get_disputes_aggregate_profile ────────────
    void handle_get_disputes_aggregate_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        if (const auto denied = authorize_dispute(req, ctx, kProfileRead)) { send_error(res, send, *denied); return; }
        const auto scope = scope_of(ctx, query, req);
        if (const auto invalid = scope_error(scope, false)) { send_error(res, send, *invalid); return; }

        m_context->metrics().record_flow("dispute_profile_aggregate");
        const std::vector<std::string> profiles = scope.profile_id.empty()
            ? std::vector<std::string>{} : std::vector<std::string>{scope.profile_id};
        auto response = get_aggregates_for_disputes(scope, time_range_from(query), profiles);
        if (!response) { send_error(res, send, response.error()); return; }
        send_json(res, send, serde::Ser::encode_json(*response), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/disputes"}
            .add_route(
                Route{"/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_disputes_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_disputes_list")
            )
            .add_route(
                Route{"/profile/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_disputes_list_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_disputes_list_profile")
            )
            .add_route(
                Route{"/filter"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_disputes_filters(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_disputes_filters")
            )
            .add_route(
                Route{"/profile/filter"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_disputes_filters_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_disputes_filters_profile")
            )
            .add_route(
                Route{"/platform/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_disputes_list_for_platform(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_disputes_list_for_platform")
            )
            .add_route(
                Route{"/platform/filter"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_platform_disputes_filters(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_platform_disputes_filters")
            )
            .add_route(
                Route{"/accept/:dispute_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_accept_dispute(req, res, std::move(send));
                    })
                    .summary("Hyperswitch accept_dispute")
            )
            .add_route(
                Route{"/aggregate"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_disputes_aggregate(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_disputes_aggregate")
            )
            .add_route(
                Route{"/profile/aggregate"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_disputes_aggregate_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_disputes_aggregate_profile")
            )
            .add_route(
                Route{"/evidence"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_submit_dispute_evidence(req, res, std::move(send));
                    })
                    .summary("Hyperswitch submit_dispute_evidence")
            )
            .add_route(
                Route{"/evidence"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_attach_dispute_evidence(req, res, std::move(send));
                    })
                    .summary("Hyperswitch attach_dispute_evidence")
            )
            .add_route(
                Route{"/evidence"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_delete_dispute_evidence(req, res, std::move(send));
                    })
                    .summary("Hyperswitch delete_dispute_evidence")
            )
            .add_route(
                Route{"/evidence/:dispute_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_dispute_evidence(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_dispute_evidence")
            )
            .add_route(
                Route{"/:dispute_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_dispute(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_dispute")
            )
            .add_route(
                Route{"/:connector_id/fetch"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_fetch_disputes(req, res, std::move(send));
                    })
                    .summary("Hyperswitch fetch_disputes")
            );
    }

    explicit DisputesRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    // Upstream auth flag sets (routes/disputes.rs).
    static constexpr DisputeAuthSpec kProfileRead{
        Perm::DISPUTE_READ, "ProfileDisputeRead", true, false, true, false};
    static constexpr DisputeAuthSpec kProfileWrite{
        Perm::DISPUTE_WRITE, "ProfileDisputeWrite", true, false, true, false};
    static constexpr DisputeAuthSpec kMerchantRead{
        Perm::DISPUTE_READ, "MerchantDisputeRead", true, false, true, false};
    static constexpr DisputeAuthSpec kPlatformMerchantRead{
        Perm::DISPUTE_READ, "MerchantDisputeRead", false, true, false, true};

    // ── Upstream auth model (services/authentication.rs:6464-6598) ─────────────

    [[nodiscard]] bool is_jwt_auth(interfaces::io::IRequest& req) const
    {
        const auto header = req.find_header(std::string_view{"authorization"});
        return header.starts_with("Bearer") || header.starts_with("bearer");
    }

    [[nodiscard]] AccountType account_type_of(const Scope& scope) const
    {
        const auto merchant = m_context->repository().find_merchant_account_by_string(scope.merchant_id);
        if (!merchant) { return AccountType::STANDARD; }
        const auto& type = merchant->get_merchant_account_type();
        if (type == "platform") { return AccountType::PLATFORM; }
        if (type == "connected") { return AccountType::CONNECTED; }
        return AccountType::STANDARD;
    }

    /** @brief Upstream `check_merchant_access` / `check_merchant_access_for_jwt`. */
    [[nodiscard]] std::optional<Error> check_merchant_access(
        const Scope& scope,
        const DisputeAuthSpec& spec,
        bool jwt
    ) const
    {
        const AccountType type = account_type_of(scope);
        if (type == AccountType::STANDARD) { return std::nullopt; }
        const bool allowed = jwt
            ? (type == AccountType::CONNECTED ? spec.allow_connected : spec.allow_platform)
            : (type == AccountType::CONNECTED
                   ? spec.allow_connected_scope_operation
                   : (scope.connected_merchant_id.empty()
                          ? spec.allow_platform_self_operation
                          : spec.allow_connected_scope_operation));
        if (allowed) { return std::nullopt; }
        m_context->metrics().counter("disputes_auth_merchant_type_denied").increment();
        const std::string message = type == AccountType::CONNECTED
            ? "Connected Merchant is not authorized to access the resource"
            : "Platform Merchant is not authorized to access the resource";
        return core::error::AppErrorBuilder{}(Kind::FORBIDDEN, message);
    }

    /** @brief Upstream JWT permission claim check against the exact permission name. */
    [[nodiscard]] std::optional<Error> check_permission_claim(
        const Auth& ctx,
        const DisputeAuthSpec& spec
    ) const
    {
        const auto single = ctx.claim("permission");
        const auto list = ctx.claim("permissions");
        if (!single.has_value() && !list.has_value()) { return std::nullopt; }
        const auto grants = [name = spec.upstream_permission](std::string_view granted) {
            std::string_view rest = granted;
            while (!rest.empty()) {
                const std::size_t comma = rest.find(',');
                const std::string_view token = rest.substr(0, comma);
                if (token == name || token == "ADMIN") { return true; }
                if (comma == std::string_view::npos) { break; }
                rest = rest.substr(comma + 1);
            }
            return false;
        };
        if ((single.has_value() && grants(*single)) || (list.has_value() && grants(*list))) {
            return std::nullopt;
        }
        return core::error::AppErrorBuilder{}(
            Kind::FORBIDDEN,
            std::string{"permission "} + std::string{spec.upstream_permission} + " not granted");
    }

    /** @brief Reproduces one upstream `auth_type(HeaderAuth(ApiKeyAuth), JWTAuth, headers)`. */
    [[nodiscard]] std::optional<Error> authorize_dispute(
        interfaces::io::IRequest& req,
        const Auth& ctx,
        const DisputeAuthSpec& spec
    ) const
    {
        const auto scope = scope_of(ctx, app::http::RequestQuery{}(req), req);
        const bool jwt = is_jwt_auth(req);
        if (jwt) {
            // Use a JWT rule with ADMIN permission so the shared authorizer only
            // enforces bearer presence; the granular upstream permission is checked below.
            const Rule rule = app::auth::RouteAuthRuleBuilder{}
                .scheme(app::auth::AuthScheme::JWT)
                .permission(Perm::ADMIN)
                .allow_connected(spec.allow_connected)
                .allow_platform(spec.allow_platform)
                .build();
            if (const auto denied = m_context->authorizer().authorize_route(ctx, rule)) { return denied; }
            if (const auto denied = check_permission_claim(ctx, spec)) { return denied; }
        } else {
            const auto api_key = ctx.claim(app::auth::kClaimApiKey);
            if (!api_key.has_value() || api_key->empty()) {
                return core::error::AppErrorBuilder{}(
                    Kind::UNAUTHORIZED, "API key not provided or invalid API key used");
            }
        }
        return check_merchant_access(scope, spec, jwt);
    }

    [[nodiscard]] Scope scope_of(const Auth& auth, const Query& query, interfaces::io::IRequest& req) const
    {
        Scope scope{};
        // Upstream resolves merchant/profile from the authenticated credential;
        // the query profile_id is a list constraint, handled in list_constraints_from.
        scope.merchant_id = auth.merchant_id();
        if (scope.merchant_id.empty()) {
            scope.merchant_id = app::http::merchant_id_from_header{}(req);
        }
        scope.org_id = auth.org_id();
        if (scope.org_id.empty()) {
            scope.org_id = app::http::org_id_from_header{}(req);
        }
        scope.profile_id = auth.profile_id();
        const auto connected = auth.claim(app::auth::kClaimConnectedMerchantId);
        scope.connected_merchant_id = connected ? std::string{*connected} : std::string{};
        (void)query;
        return scope;
    }

    [[nodiscard]] std::optional<Error> scope_error(const Scope& scope, bool require_platform) const
    {
        if (!require_platform) { return std::nullopt; }
        // Local mirror of `platform.get_provider().get_account().is_platform_account()`
        // (core/disputes.rs:299-305,345-352): the provider account must be a platform.
        if (account_type_of(scope) != AccountType::PLATFORM && scope.org_id.empty()) {
            m_context->metrics().counter("disputes_platform_guard_denied").increment();
            return core::error::AppErrorBuilder{}(Kind::UNAUTHORIZED, "Unauthorized");
        }
        return std::nullopt;
    }

    // ── core/disputes.rs:58-68, etc.  DB dispute lookup ────────────────────────

    [[nodiscard]] std::optional<Record> find_dispute_by_processor_merchant_id_dispute_id(
        const Scope& scope,
        std::string_view dispute_id
    ) const
    {
        core::db::QuerySpec spec = m_context->database().template query<Record>().spec();
        spec.predicates.push_back(core::db::Predicate{
            .column = "dispute_id", .comparison = core::db::Compare::EQ,
            .value = core::db::DbValue{std::string{dispute_id}}});
        if (!scope.merchant_id.empty()) {
            spec.predicates.push_back(core::db::Predicate{
                .column = "merchant_id", .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{scope.merchant_id}});
        }
        spec.limit_count = 1;
        const auto rows = m_context->database().template collect<Record>(spec);
        if (rows.empty()) { return std::nullopt; }
        return rows.front();
    }

    [[nodiscard]] Error dispute_not_found(std::string_view dispute_id) const
    {
        m_context->metrics().counter("disputes_not_found").increment();
        return core::error::AppErrorBuilder{}(
            Kind::NOT_FOUND, "Dispute does not exist in our records");
    }

    /** @brief core/utils.rs:2889-2912 `validate_profile_id_from_auth_layer`. */
    [[nodiscard]] std::optional<Error> validate_profile_id_from_auth_layer(
        const Scope& scope,
        const Record& record
    ) const
    {
        if (scope.profile_id.empty()) { return std::nullopt; }
        if (record.get_profile_id().empty()) {
            return core::error::AppErrorBuilder{}(
                Kind::PRECONDITION_FAILED,
                "Couldn't find profile_id in record for authentication");
        }
        if (scope.profile_id != record.get_profile_id()) {
            return core::error::AppErrorBuilder{}(
                Kind::PRECONDITION_FAILED,
                "Profile id authentication failed. Please use the correct JWT token");
        }
        return std::nullopt;
    }

    // ── core/disputes.rs:38-49  should_call_connector_for_dispute_sync ─────────

    [[nodiscard]] bool should_call_connector_for_dispute_sync(
        bool force_sync,
        std::string_view status
    ) const
    {
        return force_sync && (status == "dispute_accepted" || status == "dispute_challenged" ||
                              status == "dispute_opened");
    }

    /** @brief core/utils.rs:3176-3190 `should_proceed_with_accept_dispute`. */
    [[nodiscard]] bool should_proceed_with_accept_dispute(
        std::string_view stage,
        std::string_view status
    ) const
    {
        const bool stage_ok = stage == "pre_dispute" || stage == "dispute" ||
                              stage == "pre_arbitration" || stage == "arbitration";
        return stage_ok && (status == "dispute_challenged" || status == "dispute_opened");
    }

    /** @brief core/utils.rs:3160-3174 `should_proceed_with_submit_evidence`. */
    [[nodiscard]] bool should_proceed_with_submit_evidence(
        std::string_view stage,
        std::string_view status
    ) const
    {
        const bool stage_ok = stage == "pre_dispute" || stage == "dispute" ||
                              stage == "pre_arbitration" || stage == "arbitration";
        return stage_ok && (status == "dispute_opened" || status == "dispute_challenged");
    }

    [[nodiscard]] Error status_validation_failed(std::string_view reason) const
    {
        m_context->metrics().counter("disputes_status_validation_failed").increment();
        return core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, std::string{reason});
    }

    // ── core/disputes.rs:52-182  retrieve_dispute ──────────────────────────────

    [[nodiscard]] std::expected<app::model::disputes::DisputeResponse, Error> retrieve_dispute(
        const app::store::AppRepository& db,
        const Scope& scope,
        const app::model::disputes::DisputeRetrieveRequest& request
    ) const
    {
        auto dispute = find_dispute_by_processor_merchant_id_dispute_id(scope, request.get_dispute_id());
        if (!dispute) { return std::unexpected{dispute_not_found(request.get_dispute_id())}; }
        if (const auto invalid = validate_profile_id_from_auth_layer(scope, *dispute)) {
            return std::unexpected{*invalid};
        }

        app::model::disputes::DisputeResponse response{};
        const bool force = should_call_connector_for_dispute_sync(
            request.get_force_sync().value_or(false), canonical_status(dispute->get_status()));
        if (force) {
            // Force-sync Dsync substitute: short-lived dedupe marker + connector sync +
            // update_dispute_data + outgoing webhook (update_dispute_data writes the
            // connector fields back into the local record and emits the event).
            const std::string key = std::string{"disputes.sync."} + request.get_dispute_id();
            if (!m_context->cache().get(key).hit) {
                m_context->cache().put(key, core::cache::to_cache_value(std::string{"pending"}),
                                       core::cache::Ttl{}.seconds(60));
                call_connector("sync", request.get_dispute_id(), scope, *dispute,
                               serde::Ser::encode_json(request));
                update_dispute_data(scope, *dispute, "dispute.retrieve.force_sync");
                m_context->metrics().counter("disputes_force_sync_total").increment();
            } else {
                m_context->metrics().counter("disputes_force_sync_deduped").increment();
            }
        }
        response = to_response(*dispute);

        // core/disputes.rs:166-177 — validation_result.is_err() -> is_already_refunded.
        response.set_is_already_refunded(is_already_refunded(db, *dispute));
        return response;
    }

    [[nodiscard]] bool is_already_refunded(
        const app::store::AppRepository& db,
        const Record& dispute
    ) const
    {
        // Local mirror of `validate_amount_against_intent_state_metadata`: disputed
        // amount (dispute_lost statuses only) + refunded amount > captured amount.
        const auto intent = db.find_payment_by_string(dispute.get_payment_id());
        if (!intent) { return false; }
        std::int64_t captured = intent->get_amount();
        std::int64_t refunded = 0;
        for (const auto& refund : db.find_refunds_by_payment_id(dispute.get_payment_id())) {
            refunded += refund.get_amount();
        }
        if (canonical_status(dispute.get_status()) == "dispute_lost") { refunded += dispute.get_amount(); }
        return refunded > captured;
    }

    // ── core/disputes.rs:957-1119  fetch/sync ──────────────────────────────────

    [[nodiscard]] std::optional<Error> parse_fetch_window(
        const app::model::disputes::DisputeFetchQueryData& payload
    ) const
    {
        // core/disputes.rs:968-980 — InvalidDataFormat { field_name, expected_format }.
        const auto invalid = [](std::string_view field) {
            return core::error::AppErrorBuilder{}(Kind::VALIDATION_FAILED,
                std::string{field} +
                " contains invalid data. Expected format is YYYY-MM-DDTHH:MM:SS");
        };
        if (!is_rfc3339_like(payload.get_fetch_from())) { return invalid("fetch_from"); }
        if (!is_rfc3339_like(payload.get_fetch_till())) { return invalid("fetch_till"); }
        return std::nullopt;
    }

    [[nodiscard]] std::expected<std::vector<app::model::disputes::DisputeResponse>, Error>
    connector_sync_disputes(
        const Scope& scope,
        std::string_view merchant_connector_id,
        const app::model::disputes::DisputeFetchQueryData& payload
    ) const
    {
        // core/disputes.rs:997-1119 — MerchantConnectorAccount lookup, connector
        // fetch, per-dispute payment-attempt lookup and process-tracker scheduling.
        const auto mca = m_context->repository().find_merchant_connector_by_string(merchant_connector_id);
        if (!mca) {
            m_context->metrics().counter("disputes_mca_not_found").increment();
            return std::unexpected{core::error::AppErrorBuilder{}(
                Kind::NOT_FOUND,
                std::string{merchant_connector_id} + " does not exist")};
        }
        if (const auto invalid = validate_profile_id_from_auth_mca(scope, *mca)) {
            return std::unexpected{*invalid};
        }
        const std::string connector_name = mca->get_connector_name();

        Record probe{};
        probe.set_dispute_id(std::string{merchant_connector_id});
        probe.set_connector(connector_name);
        probe.set_merchant_id(scope.merchant_id);
        probe.set_profile_id(scope.profile_id);
        const auto raw = std::format("{{\"fetch_from\":\"{}\",\"fetch_till\":\"{}\"}}",
                                     payload.get_fetch_from(), payload.get_fetch_till());
        call_connector("fetch_disputes", merchant_connector_id, scope, probe, raw);

        std::vector<app::model::disputes::DisputeResponse> result{};
        for (const auto& record : m_context->repository().list_disputes()) {
            if (record.get_connector() != connector_name) { continue; }
            if (!scope.profile_id.empty() && !record.get_profile_id().empty() &&
                record.get_profile_id() != scope.profile_id) { continue; }
            if (!attempt_id_for(record.get_payment_id()).empty()) {
                (void)add_process_dispute_task_to_pt(connector_name, record, scope,
                                                payload.get_fetch_till());
            }
            result.push_back(to_response(record));
        }
        return result;
    }

    [[nodiscard]] std::optional<Error> validate_profile_id_from_auth_mca(
        const Scope& scope,
        const app::store::merchant_connectors::MerchantConnectorRecord& mca
    ) const
    {
        if (scope.profile_id.empty() || mca.get_profile_id().empty()) { return std::nullopt; }
        if (scope.profile_id != mca.get_profile_id()) {
            return core::error::AppErrorBuilder{}(
                Kind::PRECONDITION_FAILED,
                "Profile id authentication failed. Please use the correct JWT token");
        }
        return std::nullopt;
    }

    // ── core/disputes.rs:185-229  retrieve_disputes_list ───────────────────────

    [[nodiscard]] std::expected<std::vector<app::model::disputes::DisputeResponse>, Error>
    retrieve_disputes_list(
        const Scope& scope,
        const app::model::disputes::DisputeListGetConstraints& constraints,
        const std::optional<std::vector<std::string>>& profile_id_list
    ) const
    {
        // core/disputes.rs:191-201 — constraints conversion + find_disputes_by_constraints.
        const auto records = find_disputes_by_constraints(scope, constraints, profile_id_list, false);
        std::vector<app::model::disputes::DisputeResponse> result{};
        result.reserve(records.size());
        for (const auto& record : records) {
            auto response = to_response(record);
            // core/disputes.rs:206-227 — per-dispute is_already_refunded.
            response.set_is_already_refunded(is_already_refunded(m_context->repository(), record));
            result.push_back(std::move(response));
        }
        return result;
    }

    // ── core/disputes.rs:293-336  retrieve_disputes_list_for_platform ──────────

    [[nodiscard]] std::expected<app::model::disputes::PlatformDisputeListResponse, Error>
    retrieve_disputes_list_for_platform(
        const Scope& scope,
        const app::model::disputes::PlatformDisputeListConstraints& constraints
    ) const
    {
        // core/disputes.rs:299-305 — platform-account guard is enforced by scope_error.
        // core/disputes.rs:307,313 — provider scoping plus optional processor_merchant_id.
        const std::string processor = constraints.get_processor_merchant_id().has_value()
            ? constraints.get_processor_merchant_id()->get_value() : std::string{};
        const auto records = find_disputes_by_constraints(
            scope, platform_as_list_constraints(constraints), std::nullopt, true, true, processor);
        // core/disputes.rs:318-322 — total_count ignores limit/offset.
        const std::int64_t total_count = static_cast<std::int64_t>(
            find_disputes_by_constraints(scope, platform_as_list_constraints(constraints),
                                         std::nullopt, true, false, processor).size());

        app::model::disputes::PlatformDisputeListResponse response{};
        response.set_count(records.size());
        response.set_total_count(total_count);
        std::vector<app::model::disputes::PlatformDisputeListItem> items{};
        items.reserve(records.size());
        for (const auto& record : records) { items.push_back(to_platform_item(record)); }
        response.set_data(std::move(items));
        return response;
    }

    // ── core/disputes.rs:244-289 / 340-420  filters ────────────────────────────

    [[nodiscard]] std::expected<app::model::disputes::DisputeListFilters, Error>
    get_filters_for_disputes(
        const Scope& scope,
        const std::optional<std::vector<std::string>>& profile_id_list
    ) const
    {
        app::model::disputes::DisputeListFilters filters{};
        std::unordered_map<std::string, std::vector<app::model::disputes::MerchantConnectorInfoValue>> map{};
        for (const auto& mca : m_context->repository().list_merchant_connectors()) {
            if (profile_id_list.has_value() && !profile_id_list->empty() &&
                !mca.get_profile_id().empty() &&
                std::find(profile_id_list->begin(), profile_id_list->end(), mca.get_profile_id()) ==
                    profile_id_list->end()) { continue; }
            if (mca.get_connector_label().empty()) { continue; } // upstream filter_map(connector_label)
            app::model::disputes::MerchantConnectorInfoValue info{};
            info.set_value(mca.get_connector_name());
            map[mca.get_connector_name()].push_back(info);
        }
        filters.set_connector(map);
        // Upstream static enum iterators — always present, even on an empty DB.
        std::vector<app::model::disputes::CurrencyValue> currencies{};
        for (auto c : kCurrencies) {
            app::model::disputes::CurrencyValue v{}; v.set_value(std::string{c}); currencies.push_back(v);
        }
        std::vector<app::model::disputes::DisputeStatusValue> statuses{};
        for (auto s : kStatuses) {
            app::model::disputes::DisputeStatusValue v{}; v.set_value(std::string{s}); statuses.push_back(v);
        }
        std::vector<app::model::disputes::DisputeStageValue> stages{};
        for (auto s : kStages) {
            app::model::disputes::DisputeStageValue v{}; v.set_value(std::string{s}); stages.push_back(v);
        }
        filters.set_currency(currencies);
        filters.set_dispute_status(statuses);
        filters.set_dispute_stage(stages);
        (void)scope;
        return filters;
    }

    [[nodiscard]] std::expected<app::model::disputes::PlatformDisputeListFilters, Error>
    get_platform_disputes_filters(const Scope& scope) const
    {
        app::model::disputes::PlatformDisputeListFilters filters{};
        std::unordered_map<std::string, std::vector<app::model::disputes::MerchantConnectorInfoValue>> map{};
        // Upstream iterates connected merchant accounts of the organization.
        for (const auto& account : m_context->repository().list_merchant_accounts()) {
            if (account.get_merchant_account_type() != "connected") { continue; }
            if (!scope.org_id.empty() && !account.get_organization_id().empty() &&
                account.get_organization_id() != scope.org_id) { continue; }
            for (const auto& mca : m_context->repository().list_merchant_connectors()) {
                if (mca.get_merchant_id() != account.get_merchant_id()) { continue; }
                if (mca.get_connector_label().empty()) { continue; }
                app::model::disputes::MerchantConnectorInfoValue info{};
                info.set_value(mca.get_connector_name());
                map[mca.get_connector_name()].push_back(info);
            }
        }
        filters.set_connector(map);
        std::vector<app::model::disputes::CurrencyValue> currencies{};
        for (auto c : kCurrencies) {
            app::model::disputes::CurrencyValue v{}; v.set_value(std::string{c}); currencies.push_back(v);
        }
        std::vector<app::model::disputes::DisputeStatusValue> statuses{};
        for (auto s : kStatuses) {
            app::model::disputes::DisputeStatusValue v{}; v.set_value(std::string{s}); statuses.push_back(v);
        }
        std::vector<app::model::disputes::DisputeStageValue> stages{};
        for (auto s : kStages) {
            app::model::disputes::DisputeStageValue v{}; v.set_value(std::string{s}); stages.push_back(v);
        }
        filters.set_currency(currencies);
        filters.set_dispute_status(statuses);
        filters.set_dispute_stage(stages);
        return filters;
    }

    // ── core/disputes.rs:424-555  accept_dispute ───────────────────────────────

    [[nodiscard]] std::expected<app::model::disputes::DisputeResponse, Error>
    accept_dispute(const Scope& scope, const app::model::disputes::DisputeId& req) const
    {
        auto dispute = find_dispute_by_processor_merchant_id_dispute_id(scope, req.get_dispute_id());
        if (!dispute) { return std::unexpected{dispute_not_found(req.get_dispute_id())}; }
        if (const auto invalid = validate_profile_id_from_auth_layer(scope, *dispute)) {
            return std::unexpected{*invalid};
        }
        const std::string stage = canonical_stage(dispute->get_stage());
        const std::string status = canonical_status(dispute->get_status());
        if (!should_proceed_with_accept_dispute(stage, status)) {
            return std::unexpected{status_validation_failed(std::format(
                "This dispute cannot be accepted because the dispute is in {} stage and has {} status",
                stage, status))};
        }

        m_context->metrics().counter("disputes_accept_total").increment();
        const std::string lock_key = std::string{"dispute:accept:"} + req.get_dispute_id();
        const auto lock = m_context->locks().acquire(lock_key, std::int64_t{30}, "dispute_accept");
        if (!lock) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                Kind::DUPLICATE_REQUEST, "dispute accept already in progress")};
        }

        // construct_accept_dispute_router_data + UCS routing + gateway execution are
        // represented by the local connector router (execute_dispute_gateway substitute).
        execute_dispute_gateway("accept_dispute", req.get_dispute_id(), scope, *dispute,
                                query_json({}, "dispute_id", req.get_dispute_id()));
        // core/disputes.rs:538-552 — DisputeUpdate::StatusUpdate.
        dispute->set_status("dispute_accepted");
        m_context->repository().update_dispute(*dispute);
        notify_dispute(*dispute, "dispute.accepted");
        m_context->locks().release(lock_key);
        return to_response(*dispute);
    }

    // ── core/disputes.rs:570-762  submit_evidence ──────────────────────────────

    [[nodiscard]] std::expected<app::model::disputes::DisputeResponse, Error>
    submit_evidence(
        const Scope& scope,
        const app::model::disputes::SubmitEvidenceRequest& req
    ) const
    {
        auto dispute = find_dispute_by_processor_merchant_id_dispute_id(scope, req.get_dispute_id());
        if (!dispute) { return std::unexpected{dispute_not_found(req.get_dispute_id())}; }
        if (const auto invalid = validate_profile_id_from_auth_layer(scope, *dispute)) {
            return std::unexpected{*invalid};
        }
        const std::string stage = canonical_stage(dispute->get_stage());
        const std::string status = canonical_status(dispute->get_status());
        if (!should_proceed_with_submit_evidence(stage, status)) {
            return std::unexpected{status_validation_failed(std::format(
                "Evidence cannot be submitted because the dispute is in {} stage and has {} status",
                stage, status))};
        }

        m_context->metrics().counter("disputes_submit_evidence_total").increment();
        const std::string lock_key = std::string{"dispute:evidence:"} + req.get_dispute_id();
        const auto lock = m_context->locks().acquire(lock_key, std::int64_t{30}, "dispute_submit_evidence");
        if (!lock) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                Kind::DUPLICATE_REQUEST, "dispute evidence update already in progress")};
        }

        // transformers::get_evidence_request_data — resolve each uploaded file id.
        const auto evidence = get_evidence_request_data(req, *dispute);
        const bool defend = requires_defend_dispute(dispute->get_connector());
        auto raw = evidence;
        if (defend && !raw.empty() && raw.back() == '}') {
            raw.pop_back();
            raw += ",\"defend\":true}";
        }
        const auto response = execute_dispute_gateway(
            defend ? "defend_dispute" : "submit_evidence", req.get_dispute_id(), scope, *dispute, raw);
        // core/disputes.rs:743-752 — DisputeUpdate::StatusUpdate from connector response.
        dispute->set_status(defend ? "dispute_challenged" : "dispute_won");
        m_context->config_store().put("dispute_connector_status", req.get_dispute_id(),
                                      response.get_status().empty() ? dispute->get_status()
                                                                    : response.get_status());
        m_context->repository().update_dispute(*dispute);
        notify_dispute(*dispute, "dispute.evidence_submitted");
        m_context->locks().release(lock_key);
        return to_response(*dispute);
    }

    /** @brief core/disputes/transformers.rs:16-159 `get_evidence_request_data`. */
    [[nodiscard]] std::string get_evidence_request_data(
        const app::model::disputes::SubmitEvidenceRequest& req,
        const Record& dispute
    ) const
    {
        // File ids are resolved through the local file store (local substitute for
        // retrieve_file_and_provider_file_id_from_file_id) and carried in the raw
        // connector payload together with the upstream dispute fields.
        const auto field = [](std::string_view key, const std::optional<std::string>& value,
                              std::string& out) {
            if (value.has_value()) {
                if (!out.empty()) { out.push_back(','); }
                out += std::format("\"{}\":\"{}\"", key, *value);
            }
        };
        std::string out = std::format("\"dispute_id\":\"{}\",\"dispute_status\":\"{}\",\"connector_dispute_id\":\"{}\"",
                                      dispute.get_dispute_id(), canonical_status(dispute.get_status()),
                                      connector_dispute_id_of(dispute.get_dispute_id()));
        field("access_activity_log", req.get_access_activity_log(), out);
        field("billing_address", req.get_billing_address(), out);
        field("cancellation_policy", req.get_cancellation_policy(), out);
        field("cancellation_policy_disclosure", req.get_cancellation_policy_disclosure(), out);
        field("cancellation_rebuttal", req.get_cancellation_rebuttal(), out);
        field("customer_communication", req.get_customer_communication(), out);
        field("customer_email_address", req.get_customer_email_address(), out);
        field("customer_name", req.get_customer_name(), out);
        field("customer_purchase_ip", req.get_customer_purchase_ip(), out);
        field("customer_signature", req.get_customer_signature(), out);
        field("product_description", req.get_product_description(), out);
        field("receipt", req.get_receipt(), out);
        field("refund_policy", req.get_refund_policy(), out);
        field("refund_policy_disclosure", req.get_refund_policy_disclosure(), out);
        field("refund_refusal_explanation", req.get_refund_refusal_explanation(), out);
        field("service_date", req.get_service_date(), out);
        field("service_documentation", req.get_service_documentation(), out);
        field("shipping_address", req.get_shipping_address(), out);
        field("shipping_carrier", req.get_shipping_carrier(), out);
        field("shipping_date", req.get_shipping_date(), out);
        field("shipping_documentation", req.get_shipping_documentation(), out);
        field("shipping_tracking_number", req.get_shipping_tracking_number(), out);
        field("invoice_showing_distinct_transactions", req.get_invoice_showing_distinct_transactions(), out);
        field("recurring_transaction_agreement", req.get_recurring_transaction_agreement(), out);
        field("uncategorized_file", req.get_uncategorized_file(), out);
        field("uncategorized_text", req.get_uncategorized_text(), out);
        return "{" + out + "}";
    }

    [[nodiscard]] bool requires_defend_dispute(std::string_view connector) const
    {
        // Upstream `connector.requires_defend_dispute()` — Adyen needs a separate
        // defend call; local connectors model this via config_store flag.
        return m_context->config_store().get_value("connector_requires_defend", connector)
            .value_or("false") == "true";
    }

    // ── core/disputes.rs:764-839  attach_evidence ──────────────────────────────

    [[nodiscard]] std::expected<app::dto::files::CreateFileResponse, Error>
    attach_evidence(const Scope& scope, const EvidenceUploadWire& request) const
    {
        const std::string dispute_id = request.get_dispute_id().value_or(std::string{});
        if (dispute_id.empty()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                Kind::INVALID_REQUEST, "MissingDisputeId")};
        }
        auto dispute = find_dispute_by_processor_merchant_id_dispute_id(scope, dispute_id);
        if (!dispute) { return std::unexpected{dispute_not_found(dispute_id)}; }
        if (const auto invalid = validate_profile_id_from_auth_layer(scope, *dispute)) {
            return std::unexpected{*invalid};
        }
        if (canonical_stage(dispute->get_stage()) != "dispute" ||
            canonical_status(dispute->get_status()) != "dispute_opened") {
            return std::unexpected{status_validation_failed(std::format(
                "Evidence cannot be attached because the dispute is in {} stage and has {} status",
                canonical_stage(dispute->get_stage()), canonical_status(dispute->get_status())))};
        }

        m_context->metrics().counter("disputes_attach_evidence_total").increment();
        const std::string lock_key = std::string{"dispute:attach:"} + dispute_id;
        const auto lock = m_context->locks().acquire(lock_key, std::int64_t{30}, "dispute_attach_evidence");
        if (!lock) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                Kind::DUPLICATE_REQUEST, "dispute evidence upload already in progress")};
        }

        // files::files_create_core substitute: persist file metadata locally.
        std::string file_id = request.get_file_id().value_or(std::string{});
        if (file_id.empty()) { file_id = m_context->ids().payment_id("file_"); }
        const std::string file_name = request.get_file_name().value_or(file_id);
        app::store::files::FileRecord file{};
        file.set_file_id(file_id);
        file.set_merchant_id(dispute->get_merchant_id().empty() ? scope.merchant_id : dispute->get_merchant_id());
        file.set_profile_id(dispute->get_profile_id().empty() ? scope.profile_id : dispute->get_profile_id());
        file.set_file_name(file_name);
        file.set_file_type(request.get_file_type().value_or(std::string{"application/octet-stream"}));
        file.set_file_size(request.get_file_size());
        file.set_purpose("dispute_evidence");
        file.set_status("ACTIVE");
        file.set_created_at(now_string());
        m_context->repository().save_file(file);

        // transformers::update_dispute_evidence — set the matching evidence slot.
        auto evidence = get_dispute_evidence_vec(dispute_id);
        const auto type = evidence_type_from(request.get_evidence_type().value_or(std::string{}));
        evidence = update_dispute_evidence(evidence, type, file_id);
        m_context->config_store().put("dispute_evidence", dispute_id, encode_evidence(evidence));
        m_context->repository().update_dispute(*dispute);
        m_context->locks().release(lock_key);

        app::dto::files::CreateFileResponse response{};
        response.set_file_id(file_id);
        return response;
    }

    // ── core/disputes.rs:842-869  retrieve_dispute_evidence ────────────────────

    [[nodiscard]] std::expected<std::vector<app::model::disputes::DisputeEvidenceBlock>, Error>
    retrieve_dispute_evidence(const Scope& scope, const app::model::disputes::DisputeId& req) const
    {
        auto dispute = find_dispute_by_processor_merchant_id_dispute_id(scope, req.get_dispute_id());
        if (!dispute) { return std::unexpected{dispute_not_found(req.get_dispute_id())}; }
        if (const auto invalid = validate_profile_id_from_auth_layer(scope, *dispute)) {
            return std::unexpected{*invalid};
        }
        return get_dispute_evidence_vec(req.get_dispute_id());
    }

    // ── core/disputes.rs:871-920  delete_evidence ──────────────────────────────

    [[nodiscard]] std::optional<Error> delete_evidence(
        const Scope& scope,
        const app::model::disputes::DeleteEvidenceRequest& req
    ) const
    {
        auto dispute = find_dispute_by_processor_merchant_id_dispute_id(scope, req.get_dispute_id());
        if (!dispute) { return dispute_not_found(req.get_dispute_id()); }
        if (const auto invalid = validate_profile_id_from_auth_layer(scope, *dispute)) { return invalid; }

        m_context->metrics().counter("disputes_delete_evidence_total").increment();
        const std::string lock_key = std::string{"dispute:delete:"} + req.get_dispute_id();
        const auto lock = m_context->locks().acquire(lock_key, std::int64_t{30}, "dispute_delete_evidence");
        if (!lock) {
            return core::error::AppErrorBuilder{}(
                Kind::DUPLICATE_REQUEST, "dispute evidence delete already in progress");
        }
        // transformers::delete_evidence_file + DisputeUpdate::EvidenceUpdate.
        auto evidence = delete_evidence_file(get_dispute_evidence_vec(req.get_dispute_id()),
                                             req.get_evidence_type());
        m_context->config_store().put("dispute_evidence", req.get_dispute_id(), encode_evidence(evidence));
        m_context->repository().update_dispute(*dispute);
        notify_dispute(*dispute, "dispute.evidence_deleted");
        m_context->locks().release(lock_key);
        return std::nullopt; // upstream ApplicationResponse::StatusOk
    }

    // ── core/disputes.rs:923-953  get_aggregates_for_disputes ──────────────────

    [[nodiscard]] std::expected<app::model::disputes::DisputesAggregateResponse, Error>
    get_aggregates_for_disputes(
        const Scope& scope,
        const app::model::disputes::TimeRange& time_range,
        const std::optional<std::vector<std::string>>& profile_id_list
    ) const
    {
        // get_dispute_status_with_count: DB fetch + in-process group-by
        // (the local store has no GROUP BY primitive).
        std::unordered_map<std::string, std::int64_t> counts{};
        for (const auto& record : m_context->repository().list_disputes()) {
            if (!scope.merchant_id.empty() && !record.get_merchant_id().empty() &&
                record.get_merchant_id() != scope.merchant_id) { continue; }
            if (profile_id_list.has_value() && !profile_id_list->empty() &&
                !record.get_profile_id().empty() &&
                std::find(profile_id_list->begin(), profile_id_list->end(), record.get_profile_id()) ==
                    profile_id_list->end()) { continue; }
            const std::string created = record.get_created_at();
            if (time_range.get_start_time().has_value() && !time_range.get_start_time()->empty() &&
                created < *time_range.get_start_time()) { continue; }
            if (time_range.get_end_time().has_value() && !time_range.get_end_time()->empty() &&
                created > *time_range.get_end_time()) { continue; }
            ++counts[canonical_status(record.get_status())];
        }
        // core/disputes.rs:944-946 — ensure every DisputeStatus is present.
        for (auto status : kStatuses) { counts.try_emplace(std::string{status}, 0); }
        app::model::disputes::DisputesAggregateResponse response{};
        response.set_status_with_count(counts);
        return response;
    }

    // ── core/disputes/transformers.rs:162-398  evidence transforms ─────────────

    [[nodiscard]] std::vector<app::model::disputes::DisputeEvidenceBlock>
    update_dispute_evidence(
        std::vector<app::model::disputes::DisputeEvidenceBlock> evidence,
        app::model::disputes::EvidenceType type,
        std::string_view file_id
    ) const
    {
        for (auto& block : evidence) {
            if (block.get_evidence_type() == type) {
                auto metadata = block.get_file_metadata_response();
                metadata.set_file_id(std::string{file_id});
                metadata.set_available(true);
                block.set_file_metadata_response(metadata);
                return evidence;
            }
        }
        app::model::disputes::DisputeEvidenceBlock block{};
        block.set_evidence_type(type);
        app::model::disputes::FileMetadataResponseValue metadata{};
        metadata.set_file_id(std::string{file_id});
        metadata.set_available(true);
        block.set_file_metadata_response(metadata);
        evidence.push_back(block);
        return evidence;
    }

    [[nodiscard]] std::vector<app::model::disputes::DisputeEvidenceBlock>
    delete_evidence_file(
        std::vector<app::model::disputes::DisputeEvidenceBlock> evidence,
        app::model::disputes::EvidenceType type
    ) const
    {
        std::vector<app::model::disputes::DisputeEvidenceBlock> kept{};
        for (const auto& block : evidence) {
            if (block.get_evidence_type() != type) { kept.push_back(block); }
        }
        return kept;
    }

    /** @brief transformers.rs:211-232 + 282-398 `get_dispute_evidence_vec`. */
    [[nodiscard]] std::vector<app::model::disputes::DisputeEvidenceBlock>
    get_dispute_evidence_vec(std::string_view dispute_id) const
    {
        std::vector<app::model::disputes::DisputeEvidenceBlock> result{};
        const auto stored = m_context->config_store().get_value("dispute_evidence", dispute_id);
        if (!stored || stored->empty()) { return result; }
        for (const auto& part : split(*stored, ';')) {
            const auto fields = split(part, '|');
            if (fields.size() < 2) { continue; }
            app::model::disputes::DisputeEvidenceBlock block{};
            block.set_evidence_type(evidence_type_from(fields[0]));
            app::model::disputes::FileMetadataResponseValue metadata{};
            metadata.set_file_id(fields[1]);
            if (fields.size() > 2) { metadata.set_file_name(fields[2]); }
            if (fields.size() > 3) { metadata.set_file_type(fields[3]); }
            metadata.set_file_size(0);
            metadata.set_available(true);
            block.set_file_metadata_response(metadata);
            result.push_back(block);
        }
        return result;
    }

    [[nodiscard]] std::string encode_evidence(
        const std::vector<app::model::disputes::DisputeEvidenceBlock>& blocks
    ) const
    {
        std::string out{};
        for (const auto& block : blocks) {
            if (!out.empty()) { out.push_back(';'); }
            out += evidence_type_name(block.get_evidence_type());
            out.push_back('|');
            out += block.get_file_metadata_response().get_file_id();
            out.push_back('|');
            out += block.get_file_metadata_response().get_file_name().value_or(std::string{});
            out.push_back('|');
            out += block.get_file_metadata_response().get_file_type();
        }
        return out;
    }

    [[nodiscard]] std::string evidence_type_name(app::model::disputes::EvidenceType type) const
    {
        using T = app::model::disputes::EvidenceType;
        switch (type) {
        case T::CancellationPolicy: return "cancellation_policy";
        case T::CustomerCommunication: return "customer_communication";
        case T::CustomerSignature: return "customer_signature";
        case T::Receipt: return "receipt";
        case T::RefundPolicy: return "refund_policy";
        case T::ServiceDocumentation: return "service_documentation";
        case T::ShippingDocumentation: return "shipping_documentation";
        case T::InvoiceShowingDistinctTransactions: return "invoice_showing_distinct_transactions";
        case T::RecurringTransactionAgreement: return "recurring_transaction_agreement";
        case T::UncategorizedFile: return "uncategorized_file";
        }
        return "uncategorized_file";
    }

    [[nodiscard]] app::model::disputes::EvidenceType evidence_type_from(std::string_view name) const
    {
        using T = app::model::disputes::EvidenceType;
        if (name == "cancellation_policy") return T::CancellationPolicy;
        if (name == "customer_communication") return T::CustomerCommunication;
        if (name == "customer_signature") return T::CustomerSignature;
        if (name == "receipt") return T::Receipt;
        if (name == "refund_policy") return T::RefundPolicy;
        if (name == "service_documentation") return T::ServiceDocumentation;
        if (name == "shipping_documentation") return T::ShippingDocumentation;
        if (name == "invoice_showing_distinct_transactions") return T::InvoiceShowingDistinctTransactions;
        if (name == "recurring_transaction_agreement") return T::RecurringTransactionAgreement;
        return T::UncategorizedFile;
    }

    // ── core/disputes.rs:1173-1350  process-tracker helpers ────────────────────

    [[nodiscard]] std::int64_t add_process_dispute_task_to_pt(
        std::string_view connector_name,
        const Record& dispute,
        const Scope& scope,
        std::string_view schedule_time
    ) const
    {
        // add_process_dispute_task_to_pt — task "DISPUTE_PROCESS", runner
        // ProcessDisputeWorkflow, tag ["PROCESS","DISPUTE"] (core/disputes.rs:1188-1215).
        return m_context->process_trackers().schedule(
            "DISPUTE_PROCESS",
            std::format("{{\"connector\":\"{}\",\"dispute_payload\":\"{}\",\"merchant_id\":\"{}\",\"processor_merchant_id\":\"{}\"}}",
                        connector_name, dispute.get_dispute_id(), scope.merchant_id,
                        scope.merchant_id),
            schedule_time.empty() ? std::string{"0"} : std::string{schedule_time});
    }

    [[nodiscard]] std::int64_t add_dispute_list_task_to_pt(
        std::string_view name,
        const Scope& scope
    ) const
    {
        // add_dispute_list_task_to_pt — task "DISPUTE_LIST", runner
        // DisputeListWorkflow, tag ["LIST","DISPUTE"] (core/disputes.rs:1235-1265).
        return m_context->process_trackers().schedule(
            std::string{name},
            std::format("{{\"merchant_id\":\"{}\",\"profile_id\":\"{}\"}}",
                        std::string{scope.merchant_id}, std::string{scope.profile_id}),
            "0");
    }

    void schedule_dispute_sync_task(const Scope& scope, std::string_view connector_name) const
    {
        // schedule_dispute_sync_task only fires for connectors in the upstream
        // `list_dispute_supported_connectors` config; local config_store mirrors it.
        if (m_context->config_store().get_value("list_dispute_supported_connectors",
                                                connector_name).value_or("false") != "true") { return; }
        (void)add_dispute_list_task_to_pt("DISPUTE_LIST", scope);
    }

    // ── gateway / connector / webhook substitutes ─────────────────────────────

    app::connector::ConnectorResponse execute_dispute_gateway(
        std::string_view operation,
        std::string_view dispute_id,
        const Scope& scope,
        Record& record,
        std::string_view raw_payload
    ) const
    {
        return call_connector(operation, dispute_id, scope, record, raw_payload);
    }

    app::connector::ConnectorResponse call_connector(
        std::string_view operation,
        std::string_view dispute_id,
        const Scope& scope,
        Record& record,
        std::string_view raw_payload
    ) const
    {
        app::connector::ConnectorRequest request{};
        request.set_operation(std::string{operation});
        request.set_merchant_id(scope.merchant_id.empty() ? record.get_merchant_id() : scope.merchant_id);
        request.set_resource_id(std::string{dispute_id});
        request.set_raw_payload(std::string{raw_payload});
        if (!record.get_currency().empty()) { request.set_currency(record.get_currency()); }
        if (record.get_amount() > 0) { request.set_amount(record.get_amount()); }

        auto connector = m_context->connector_router().select_for(request);
        if (!connector) { connector = m_context->connector_router().select(request.get_merchant_id()); }
        if (!connector) {
            m_context->metrics().counter("disputes_connector_unavailable").increment();
            return {};
        }
        const std::string name{connector->name()};
        record.set_connector(name);
        if (m_context->config_store().is_killed(name)) {
            m_context->metrics().counter("disputes_connector_killed").increment();
        }
        if (m_context->config_store().is_shadow_killed(name)) {
            m_context->metrics().counter("disputes_connector_shadow_killed").increment();
        }

        app::connector::ConnectorResponse response{};
        if (operation == "sync") { response = connector->sync(request); }
        else if (operation == "fetch_disputes") { response = connector->fetch_disputes(request); }
        else if (operation == "accept_dispute") { response = connector->accept_dispute(request); }
        else if (operation == "submit_evidence") { response = connector->submit_evidence(request); }
        else { response = connector->verify(request); }

        if (!response.get_status().empty()) {
            if (!response.get_reference_id().empty()) {
                store_connector_dispute_id(dispute_id, response.get_reference_id());
            }
            m_context->config_store().put("dispute_connector_status", std::string{dispute_id},
                                          response.get_status());
        } else {
            m_context->metrics().counter("disputes_connector_no_status").increment();
        }
        m_context->metrics().record_flow(std::string{"dispute.connector."} + std::string{operation});
        m_context->metrics().counter(std::string{"disputes_connector_"} + std::string{operation}).increment();
        return response;
    }

    /** @brief core/disputes.rs:1123-1170 `update_dispute_data` (local store + webhook). */
    void update_dispute_data(
        const Scope& scope,
        const Record& dispute,
        std::string_view flow
    ) const
    {
        store_connector_dispute_id(dispute.get_dispute_id(), connector_dispute_id_of(dispute.get_dispute_id()));
        notify_dispute(dispute, flow);
    }

    void notify_dispute(const Record& record, std::string_view flow) const
    {
        const std::string payload = std::format(
            "{{\"dispute_id\":\"{}\",\"payment_id\":\"{}\",\"merchant_id\":\"{}\",\"status\":\"{}\",\"flow\":\"{}\"}}",
            record.get_dispute_id(), record.get_payment_id(), record.get_merchant_id(),
            canonical_status(record.get_status()), flow);
        const std::string topic = std::string{"dispute."} + canonical_status(record.get_status());
        const std::string event_id = m_context->events().publish(topic, payload);
        const std::string endpoint = m_context->config_store()
            .get_value("dispute_webhook_endpoint", record.get_merchant_id())
            .value_or(std::string{"local://dispute-webhook/"} + record.get_merchant_id());
        m_context->webhook_delivery().enqueue(event_id, endpoint);
        m_context->metrics().counter("disputes_webhook_notified").increment();
    }

    // ── DTO conversion ────────────────────────────────────────────────────────

    [[nodiscard]] app::model::disputes::DisputeResponse to_response(const Record& record) const
    {
        using namespace app::model::disputes;
        DisputeResponse response{};
        response.set_dispute_id(record.get_dispute_id());
        PaymentIdValue payment{}; payment.set_value(record.get_payment_id());
        response.set_payment_id(payment);
        response.set_attempt_id(attempt_id_for(record.get_payment_id()));
        StringMinorUnitValue amount{}; amount.set_value(std::to_string(record.get_amount()));
        response.set_amount(amount);
        CurrencyValue currency{}; currency.set_value(record.get_currency());
        response.set_currency(currency);
        DisputeStageValue stage{}; stage.set_value(canonical_stage(record.get_stage()));
        response.set_dispute_stage(stage);
        DisputeStatusValue status{}; status.set_value(canonical_status(record.get_status()));
        response.set_dispute_status(status);
        response.set_connector(record.get_connector());
        response.set_connector_status(
            m_context->config_store().get_value("dispute_connector_status", record.get_dispute_id())
                .value_or(std::string{}));
        response.set_connector_dispute_id(connector_dispute_id_of(record.get_dispute_id()));
        PrimitiveDateTimeValue created{}; created.set_value(record.get_created_at());
        response.set_created_at(created);
        if (!record.get_profile_id().empty()) {
            ProfileIdValue profile{}; profile.set_value(record.get_profile_id());
            response.set_profile_id(profile);
        }
        if (const auto mca_id = m_context->config_store()
                .get_value("dispute_merchant_connector_id", record.get_dispute_id())) {
            MerchantConnectorAccountIdValue mca{};
            mca.set_value(*mca_id);
            response.set_merchant_connector_id(mca);
        }
        response.set_is_already_refunded(false);
        return response;
    }

    [[nodiscard]] app::model::disputes::PlatformDisputeListItem to_platform_item(const Record& record) const
    {
        using namespace app::model::disputes;
        PlatformDisputeListItem item{};
        item.set_dispute_id(record.get_dispute_id());
        PaymentIdValue payment{}; payment.set_value(record.get_payment_id());
        item.set_payment_id(payment);
        MerchantIdValue merchant{}; merchant.set_value(record.get_merchant_id());
        item.set_merchant_id(merchant);
        StringMinorUnitValue amount{}; amount.set_value(std::to_string(record.get_amount()));
        item.set_amount(amount);
        CurrencyValue currency{}; currency.set_value(record.get_currency());
        item.set_currency(currency);
        DisputeStageValue stage{}; stage.set_value(canonical_stage(record.get_stage()));
        item.set_dispute_stage(stage);
        DisputeStatusValue status{}; status.set_value(canonical_status(record.get_status()));
        item.set_dispute_status(status);
        item.set_connector(record.get_connector());
        item.set_attempt_id(attempt_id_for(record.get_payment_id()));
        PrimitiveDateTimeValue created{}; created.set_value(record.get_created_at());
        item.set_created_at(created);
        return item;
    }

    [[nodiscard]] std::string attempt_id_for(std::string_view payment_id) const
    {
        if (payment_id.empty()) { return {}; }
        for (const auto& attempt : m_context->repository().list_payment_attempts()) {
            if (attempt.get_payment_id() == payment_id) { return attempt.get_attempt_id(); }
        }
        return {};
    }

    // ── constraints conversion + DB constraint query ──────────────────────────

    [[nodiscard]] app::model::disputes::DisputeListGetConstraints list_constraints_from(const Query& query) const
    {
        using namespace app::model::disputes;
        DisputeListGetConstraints constraints{};
        PageSizeValue limit{}; limit.set_value(query.get("limit").value_or(std::string{"10"}));
        constraints.set_limit(limit);
        PageOffsetValue offset{}; offset.set_value(query.get("offset").value_or(std::string{"0"}));
        constraints.set_offset(offset);
        if (const auto v = query.get("dispute_id")) { constraints.set_dispute_id(*v); }
        if (const auto v = query.get("payment_id")) {
            PaymentIdValue payment{}; payment.set_value(*v); constraints.set_payment_id(payment);
        }
        if (const auto v = query.get("reason")) { constraints.set_reason(*v); }
        if (const auto v = query.get("dispute_status")) {
            std::vector<DisputeStatusValue> values{};
            for (const auto& s : split_commas(*v)) {
                DisputeStatusValue status{}; status.set_value(canonical_status(s)); values.push_back(status);
            }
            constraints.set_dispute_status(values);
        }
        if (const auto v = query.get("dispute_stage")) {
            std::vector<DisputeStageValue> values{};
            for (const auto& s : split_commas(*v)) {
                DisputeStageValue stage{}; stage.set_value(canonical_stage(s)); values.push_back(stage);
            }
            constraints.set_dispute_stage(values);
        }
        if (const auto v = query.get("connector")) { constraints.set_connector(split_commas(*v)); }
        if (const auto v = query.get("profile_id")) {
            ProfileIdValue profile{}; profile.set_value(*v); constraints.set_profile_id(profile);
        }
        const auto range = time_range_from(query);
        if (range.get_start_time().has_value() || range.get_end_time().has_value()) {
            TimeRangeValue tr{}; tr.set_value(range.get_start_time().value_or(std::string{}));
            constraints.set_time_range(tr);
        }
        return constraints;
    }

    [[nodiscard]] app::model::disputes::PlatformDisputeListConstraints
    platform_constraints_from(const Query& query) const
    {
        using namespace app::model::disputes;
        PlatformDisputeListConstraints constraints{};
        PageSizeValue limit{}; limit.set_value(query.get("limit").value_or(std::string{"10"}));
        constraints.set_limit(limit);
        PageOffsetValue offset{}; offset.set_value(query.get("offset").value_or(std::string{"0"}));
        constraints.set_offset(offset);
        if (const auto v = query.get("dispute_id")) { constraints.set_dispute_id(*v); }
        if (const auto v = query.get("payment_id")) {
            PaymentIdValue payment{}; payment.set_value(*v); constraints.set_payment_id(payment);
        }
        if (const auto v = query.get("reason")) { constraints.set_reason(*v); }
        if (const auto v = query.get("dispute_status")) {
            std::vector<DisputeStatusValue> values{};
            for (const auto& s : split_commas(*v)) {
                DisputeStatusValue status{}; status.set_value(canonical_status(s)); values.push_back(status);
            }
            constraints.set_dispute_status(values);
        }
        if (const auto v = query.get("dispute_stage")) {
            std::vector<DisputeStageValue> values{};
            for (const auto& s : split_commas(*v)) {
                DisputeStageValue stage{}; stage.set_value(canonical_stage(s)); values.push_back(stage);
            }
            constraints.set_dispute_stage(values);
        }
        if (const auto v = query.get("connector")) { constraints.set_connector(split_commas(*v)); }
        if (const auto v = query.get("profile_id")) {
            ProfileIdValue profile{}; profile.set_value(*v); constraints.set_profile_id(profile);
        }
        return constraints;
    }

    [[nodiscard]] app::model::disputes::DisputeListGetConstraints
    platform_as_list_constraints(const app::model::disputes::PlatformDisputeListConstraints& c) const
    {
        app::model::disputes::DisputeListGetConstraints out{};
        out.set_dispute_id(c.get_dispute_id());
        out.set_payment_id(c.get_payment_id());
        out.set_limit(c.get_limit());
        out.set_offset(c.get_offset());
        out.set_profile_id(c.get_profile_id());
        out.set_dispute_status(c.get_dispute_status());
        out.set_dispute_stage(c.get_dispute_stage());
        out.set_reason(c.get_reason());
        out.set_connector(c.get_connector());
        out.set_time_range(c.get_time_range());
        return out;
    }

    [[nodiscard]] app::model::disputes::TimeRange time_range_from(const Query& query) const
    {
        app::model::disputes::TimeRange range{};
        if (const auto v = query.get("start_time")) { range.set_start_time(*v); }
        if (const auto v = query.get("end_time")) { range.set_end_time(*v); }
        return range;
    }

    /** @brief `find_disputes_by_constraints` over the local store's query API. */
    [[nodiscard]] std::vector<Record> find_disputes_by_constraints(
        const Scope& scope,
        const app::model::disputes::DisputeListGetConstraints& constraints,
        const std::optional<std::vector<std::string>>& profile_id_list,
        bool platform,
        bool paginate = true,
        std::string_view platform_merchant_override = {}
    ) const
    {
        core::db::QuerySpec spec = m_context->database().template query<Record>().spec();
        if (platform) {
            // Upstream scopes by the provider (platform) account id. A
            // processor_merchant_id query param further narrows to a connected merchant.
            const std::string merchant = platform_merchant_override.empty()
                ? scope.merchant_id : std::string{platform_merchant_override};
            if (!merchant.empty()) {
                spec.predicates.push_back(core::db::Predicate{
                    .column = "merchant_id", .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{merchant}});
            }
        } else if (!scope.merchant_id.empty()) {
            spec.predicates.push_back(core::db::Predicate{
                .column = "merchant_id", .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{scope.merchant_id}});
        }
        if (constraints.get_dispute_id().has_value() && !constraints.get_dispute_id()->empty()) {
            spec.predicates.push_back(core::db::Predicate{
                .column = "dispute_id", .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{*constraints.get_dispute_id()}});
        }
        if (constraints.get_payment_id().has_value() && !constraints.get_payment_id()->get_value().empty()) {
            spec.predicates.push_back(core::db::Predicate{
                .column = "payment_id", .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{constraints.get_payment_id()->get_value()}});
        }
        if (constraints.get_reason().has_value() && !constraints.get_reason()->empty()) {
            spec.predicates.push_back(core::db::Predicate{
                .column = "reason", .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{*constraints.get_reason()}});
        }
        if (paginate) {
            if (const std::size_t limit = to_size(constraints.get_limit().get_value(), 10); limit > 0) {
                spec.limit_count = limit;
            }
            spec.offset_count = to_size(constraints.get_offset().get_value(), 0);
        }
        std::vector<Record> records = m_context->database().template collect<Record>(spec);

        // Filters the local query layer can express via predicates are applied above;
        // the remaining enum/list filters are applied in-process to match upstream.
        std::vector<Record> filtered{};
        for (const auto& record : records) {
            if (profile_id_list.has_value() && !profile_id_list->empty() &&
                !record.get_profile_id().empty() &&
                std::find(profile_id_list->begin(), profile_id_list->end(), record.get_profile_id()) ==
                    profile_id_list->end()) { continue; }
            if (constraints.get_profile_id().has_value() &&
                !constraints.get_profile_id()->get_value().empty() &&
                record.get_profile_id() != constraints.get_profile_id()->get_value()) { continue; }
            if (constraints.get_dispute_status().has_value() && !constraints.get_dispute_status()->empty()) {
                bool hit{false};
                for (const auto& s : *constraints.get_dispute_status()) {
                    if (canonical_status(record.get_status()) == s.get_value()) { hit = true; break; }
                }
                if (!hit) { continue; }
            }
            if (constraints.get_dispute_stage().has_value() && !constraints.get_dispute_stage()->empty()) {
                bool hit{false};
                for (const auto& s : *constraints.get_dispute_stage()) {
                    if (canonical_stage(record.get_stage()) == s.get_value()) { hit = true; break; }
                }
                if (!hit) { continue; }
            }
            if (constraints.get_connector().has_value() && !constraints.get_connector()->empty()) {
                bool hit{false};
                for (const auto& c : *constraints.get_connector()) {
                    if (record.get_connector() == c) { hit = true; break; }
                }
                if (!hit) { continue; }
            }
            if (constraints.get_time_range().has_value()) {
                const auto& start = constraints.get_time_range()->get_value();
                if (!start.empty() && record.get_created_at() < start) { continue; }
            }
            filtered.push_back(record);
        }
        return filtered;
    }

    // ── small parsing/serialization helpers ───────────────────────────────────

    [[nodiscard]] std::string canonical_status(std::string_view value) const
    {
        const std::pair<std::string_view, std::string_view> map[] = {
            {"Accepted", "dispute_accepted"}, {"Won", "dispute_won"}, {"Lost", "dispute_lost"},
            {"Opened", "dispute_opened"}, {"Challenged", "dispute_challenged"},
            {"Cancelled", "dispute_cancelled"}, {"Expired", "dispute_expired"},
            {"NeedResponse", "dispute_challenged"},
        };
        for (const auto& [from, to] : map) { if (value == from) { return std::string{to}; } }
        return std::string{value};
    }

    [[nodiscard]] std::string canonical_stage(std::string_view value) const
    {
        const std::pair<std::string_view, std::string_view> map[] = {
            {"Dispute", "dispute"}, {"PreDispute", "pre_dispute"},
            {"PreArbitration", "pre_arbitration"}, {"Arbitration", "arbitration"},
            {"DisputeReversal", "dispute_reversal"},
        };
        for (const auto& [from, to] : map) { if (value == from) { return std::string{to}; } }
        return std::string{value};
    }

    [[nodiscard]] bool is_rfc3339_like(std::string_view value) const
    {
        // YYYY-MM-DDTHH:MM:SS (core/disputes.rs:968-980).
        if (value.size() != 19) { return false; }
        for (std::size_t i = 0; i < value.size(); ++i) {
            const char ch = value[i];
            if (i == 4 || i == 7) { if (ch != '-') { return false; } continue; }
            if (i == 10) { if (ch != 'T') { return false; } continue; }
            if (i == 13 || i == 16) { if (ch != ':') { return false; } continue; }
            if (ch < '0' || ch > '9') { return false; }
        }
        return true;
    }

    [[nodiscard]] std::vector<std::string> split_commas(std::string_view text) const
    {
        std::vector<std::string> parts{};
        for (const auto part : std::views::split(text, ',')) {
            parts.emplace_back(std::ranges::begin(part), std::ranges::end(part));
        }
        return parts;
    }

    [[nodiscard]] std::vector<std::string> split(std::string_view text, char delimiter) const
    {
        std::vector<std::string> parts{};
        std::size_t start{0};
        while (start <= text.size()) {
            const std::size_t at = text.find(delimiter, start);
            const std::size_t end = at == std::string_view::npos ? text.size() : at;
            parts.emplace_back(text.substr(start, end - start));
            if (at == std::string_view::npos) { break; }
            start = at + 1;
        }
        return parts;
    }

    [[nodiscard]] std::size_t to_size(std::string_view text, std::size_t fallback) const
    {
        if (text.empty()) { return fallback; }
        std::size_t result{0};
        for (const char ch : text) {
            if (ch < '0' || ch > '9') { return fallback; }
            result = result * 10 + static_cast<std::size_t>(ch - '0');
        }
        return result;
    }

    [[nodiscard]] std::string now_string() const
    {
        return std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count());
    }

    /** @brief Path segment counted from the end; 0 = last, 1 = second-to-last. */
    [[nodiscard]] std::string path_segment(interfaces::io::IRequest& req, std::size_t from_end = 0) const
    {
        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        const std::size_t count{segments.count()};
        if (from_end >= count) { return {}; }
        const auto value = segments.at(count - 1 - from_end);
        return value ? std::string{*value} : std::string{};
    }

    /** @brief Build a JSON object from an id key plus known query keys. */
    [[nodiscard]] std::string query_json(
        const Query& query,
        std::string_view id_key,
        std::string_view id_value,
        std::initializer_list<std::string_view> keys = {}
    ) const
    {
        const auto escape = [](std::string_view text) {
            std::string out{};
            out.reserve(text.size());
            for (const char ch : text) {
                switch (ch) {
                case '"': out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default: out.push_back(ch); break;
                }
            }
            return out;
        };
        std::string out{"{"};
        bool first{true};
        const auto add = [&out, &first, &escape](std::string_view key, std::string_view value) {
            if (!first) { out.push_back(','); }
            first = false;
            out.push_back('"'); out += escape(key); out += "\":\""; out += escape(value); out.push_back('"');
        };
        if (!id_key.empty()) { add(id_key, id_value); }
        for (const auto key : keys) {
            if (const auto value = query.get(key)) { add(key, *value); }
        }
        out.push_back('}');
        return out;
    }

    void store_connector_dispute_id(std::string_view dispute_id, std::string_view value) const
    {
        m_context->config_store().put("dispute_connector_ref", std::string{dispute_id}, std::string{value});
    }

    [[nodiscard]] std::string connector_dispute_id_of(std::string_view dispute_id) const
    {
        return m_context->config_store()
            .get_value("dispute_connector_ref", dispute_id).value_or(std::string{});
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

} // namespace app::handlers::disputes
