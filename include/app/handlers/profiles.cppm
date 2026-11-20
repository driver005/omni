module;
export module app_handlers:profiles;

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
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_events;
import app_services;

export namespace app::handlers::profiles {

/**
 * Strict port of upstream `routes/profiles.rs` + the profile functions of
 * `core/admin.rs`. Upstream external infrastructure (accounts_store, master
 * key, key store, decision engine, outbound destination resolver) is replaced
 * by in-process AppContext facilities; every guard, error and wire shape
 * otherwise follows the upstream source.
 */
class ProfilesRouteHandlers
{
public:
    explicit ProfilesRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    // ------------------------------------------------------------------
    // Route adapters — docs/hyperswitch/crates/router/src/routes/profiles.rs
    // ------------------------------------------------------------------

    void handle_profile_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_profile_operation(req, "profile_create")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::admin::ProfileCreate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // profiles.rs:22-45 (v1) / :79-90 (v2) — route-layer request validation.
        if (const auto error = validate_profile_create_request(*parsed)) {
            send_error(res, send, *error);
            return;
        }

        // profiles.rs:21 / :76 — merchant id from route (v1) or header (v2).
        const std::string merchant_id = merchant_from_request(req);
        const auto result = create_profile(merchant_id, *parsed, is_v2(req));
        if (!result) {
            send_error(res, send, result.error());
            return;
        }

        m_context->metrics().record_flow("profiles.create");
        m_context->metrics().record_flow("Flow::ProfileCreate");
        m_context->metrics().counter("profiles_create").increment();
        send_json(res, send, *result, true);
    }

    void handle_profile_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_profile_operation(req, "profile_retrieve")) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string profile_id = profile_id_from_request(req);
        if (profile_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "missing profile id"));
            return;
        }
        // profiles.rs:149-161 (v1) / :203-205 (v2) — retrieve with the
        // authenticated processor merchant (route for v1, header for v2).
        const auto result = retrieve_profile(profile_id, merchant_from_request(req));
        if (!result) {
            send_error(res, send, result.error());
            return;
        }

        m_context->metrics().record_flow("profiles.retrieve");
        m_context->metrics().record_flow("Flow::ProfileRetrieve");
        m_context->metrics().counter("profiles_retrieve").increment();
        send_json(res, send, *result, true);
    }

    void handle_profile_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_profile_operation(req, "profile_update")) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string profile_id = profile_id_from_request(req);
        if (profile_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "missing profile id"));
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::admin::ProfileUpdate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // profiles.rs:235-259 (v1) / :314-325 (v2) — route-layer validation.
        if (const auto error = validate_profile_update_request(*parsed)) {
            send_error(res, send, *error);
            return;
        }

        const auto result = update_profile(profile_id, merchant_from_request(req), *parsed);
        if (!result) {
            send_error(res, send, result.error());
            return;
        }

        m_context->metrics().record_flow("profiles.update");
        m_context->metrics().record_flow("Flow::ProfileUpdate");
        m_context->metrics().counter("profiles_update").increment();
        send_json(res, send, *result, true);
    }

    void handle_profile_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_profile_operation(req, "profile_delete")) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string profile_id = profile_id_from_request(req);
        if (profile_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "missing profile id"));
            return;
        }
        if (const auto error = delete_profile(profile_id, merchant_from_request(req))) {
            send_error(res, send, *error);
            return;
        }

        // core/admin.rs:4195 — Json(bool): literal true from the store delete result.
        m_context->metrics().record_flow("profiles.delete");
        m_context->metrics().record_flow("Flow::ProfileDelete");
        m_context->metrics().counter("profiles_delete").increment();
        send_json(res, send, "true", true);
    }

    void handle_profiles_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_profile_operation(req, "profiles_list")) {
            send_error(res, send, *auth_error);
            return;
        }

        // profiles.rs:402-406 (v1) / :438 (v2) — list_profile(merchant, None).
        const auto result = list_profile(merchant_from_request(req), std::nullopt);
        if (!result) {
            send_error(res, send, result.error());
            return;
        }

        m_context->metrics().record_flow("profiles.list");
        m_context->metrics().record_flow("Flow::ProfileList");
        m_context->metrics().counter("profiles_list").increment();
        send_json(res, send, *result, true);
    }

    void handle_profiles_list_at_profile_level(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_profile_operation(req, "profiles_list_at_profile_level")) {
            send_error(res, send, *auth_error);
            return;
        }

        // profiles.rs:471-476 — list_profile(merchant,
        // auth.profile.map(|p| vec![p.get_id().clone()])).
        const auto auth = app::auth::AuthContextBuilder{}(req);
        auto filter = std::optional<std::vector<std::string>>{};
        if (!auth.profile_id().empty()) {
            filter = std::vector<std::string>{auth.profile_id()};
        }
        const auto result = list_profile(merchant_from_request(req), filter);
        if (!result) {
            send_error(res, send, result.error());
            return;
        }

        m_context->metrics().record_flow("profiles.list_at_profile_level");
        m_context->metrics().record_flow("Flow::ProfileList");
        m_context->metrics().counter("profiles_list_at_profile_level").increment();
        send_json(res, send, *result, true);
    }

    void handle_toggle_connector_agnostic_mit(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_profile_operation(req, "toggle_connector_agnostic_mit")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::admin::ConnectorAgnosticMitChoice;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const std::string profile_id = profile_id_from_request(req, 1);
        if (profile_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "missing profile id"));
            return;
        }
        const auto result = connector_agnostic_mit_toggle(
            merchant_from_request(req), profile_id, *parsed
        );
        if (!result) {
            send_error(res, send, result.error());
            return;
        }

        m_context->metrics().record_flow("profiles.toggle_connector_agnostic_mit");
        m_context->metrics().record_flow("Flow::ToggleConnectorAgnosticMit");
        m_context->metrics().counter("profiles_toggle_connector_agnostic_mit").increment();
        send_json(res, send, *result, true);
    }

    void handle_toggle_extended_card_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_profile_operation(req, "toggle_extended_card_info")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::admin::ExtendedCardInfoChoice;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const std::string profile_id = profile_id_from_request(req, 1);
        if (profile_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "missing profile id"));
            return;
        }
        const auto result = extended_card_info_toggle(
            merchant_from_request(req), profile_id, *parsed
        );
        if (!result) {
            send_error(res, send, result.error());
            return;
        }

        m_context->metrics().record_flow("profiles.toggle_extended_card_info");
        m_context->metrics().record_flow("Flow::ToggleExtendedCardInfo");
        m_context->metrics().counter("profiles_toggle_extended_card_info").increment();
        send_json(res, send, *result, true);
    }

    // ------------------------------------------------------------------
    // Registration — app.rs:2661-2722 (v2), :2727-2784 (v1), :2170 (v2 list),
    // :2796-2800 (v1 profile-level list). C++-only routes removed:
    // /profiles/payment_connector_list_profile (upstream handler unregistered)
    // and /account/:account_id/profile/connectors (owned by admin.cppm:1707).
    // ------------------------------------------------------------------
    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        Router{router, "/v2/profiles"}
            .add_route(
                Route{""}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profile_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profile_create")
            )
            .add_route(
                Route{"/:profile_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profile_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profile_retrieve")
            )
            .add_route(
                Route{"/:profile_id"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profile_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profile_update")
            );

        Router{router, "/v2/merchant-accounts/:id/profiles"}
            .add_route(
                Route{""}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profiles_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profiles_list")
            );

        Router{router, "/account/:account_id/business_profile"}
            .add_route(
                Route{""}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profile_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profile_create")
            )
            .add_route(
                Route{""}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profiles_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profiles_list")
            )
            .add_route(
                Route{"/:profile_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profile_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profile_retrieve")
            )
            .add_route(
                Route{"/:profile_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profile_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profile_update")
            )
            .add_route(
                // delt() is the ApiRoute DELETE binding (core/router/builder.cppm:542-548).
                Route{"/:profile_id"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profile_delete(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profile_delete")
            )
            .add_route(
                Route{"/:profile_id/toggle_extended_card_info"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_toggle_extended_card_info(req, res, std::move(send));
                    })
                    .summary("Hyperswitch toggle_extended_card_info")
            )
            .add_route(
                Route{"/:profile_id/toggle_connector_agnostic_mit"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_toggle_connector_agnostic_mit(req, res, std::move(send));
                    })
                    .summary("Hyperswitch toggle_connector_agnostic_mit")
            );

        Router{router, "/account/:account_id/profile"}
            .add_route(
                Route{""}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profiles_list_at_profile_level(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profiles_list_at_profile_level")
            );
    }

private:
    using JsonResult = std::expected<std::string, core::error::AppError>;
    using Response = app::dto::admin::ProfileResponse;
    using CreateInput = app::dto::admin::ProfileCreate;
    using UpdateInput = app::dto::admin::ProfileUpdate;

    // ==================================================================
    // Upstream core/admin.rs functions, ported as local helpers.
    // ==================================================================

    // core/admin.rs:4061-4119 — create_profile.
    [[nodiscard]] JsonResult create_profile(
        std::string_view merchant_id,
        const CreateInput& request,
        bool v2_route
    ) const
    {
        auto repository = m_context->repository();
        const auto account = repository.find_merchant_account_by_string(merchant_id);
        const std::string account_type = account ? account->get_merchant_account_type() : std::string{};

        // core/admin.rs:4066-4070.
        if (const auto error = validate_external_vault_config_for_merchant_account_type(
                account_type,
                request.get_is_external_vault_enabled(),
                request.get_external_vault_connector_details().has_value()
            )) {
            return std::unexpected(*error);
        }

        // core/admin.rs:4072-4084 — create_domain_model_from_request.
        auto domain = create_domain_model_from_request(merchant_id, request);
        if (!domain) {
            return std::unexpected(domain.error());
        }
        const std::string profile_id = domain->get_profile_id().get_value();

        // core/admin.rs:4088-4098 — insert_business_profile + duplicate mapping
        // (GenericDuplicateError -> ALREADY_EXISTS / HTTP 409).
        if (repository.find_profile_by_string(profile_id)) {
            return std::unexpected(make_error(
                core::error::AppErrorKind::ALREADY_EXISTS,
                std::format("Business Profile with the profile_id {} already exists", profile_id)
            ));
        }
        const auto payload = insert_business_profile(profile_id, merchant_id, domain->get_profile_name(), *domain);
        if (!payload) {
            return std::unexpected(*payload);
        }

        // core/admin.rs:4100-4112 (v1) — unset an existing default profile.
        if (!v2_route && account && !account->get_default_profile().empty()
            && account->get_default_profile() != profile_id) {
            account->set_default_profile({});
            (void)repository.update_merchant_account(*account);
        }

        emit_event("create", *payload);
        return *payload;
    }

    // core/admin.rs:4122-4150 — list_profile.
    [[nodiscard]] JsonResult list_profile(
        std::string_view merchant_id,
        const std::optional<std::vector<std::string>>& profile_id_list
    ) const
    {
        // core/admin.rs:4127-4133 — master key + key store lookup.
        const auto account = get_merchant_key_store_by_merchant_id(merchant_id, master_key());
        if (!account) {
            return std::unexpected(make_error(
                core::error::AppErrorKind::NOT_FOUND, "merchant account not found"
            ));
        }
        // core/admin.rs:4134-4139 — list_profile_by_merchant_id.
        auto profiles = list_profile_by_merchant_id(merchant_id);
        // core/admin.rs:4140 — filter_objects_based_on_profile_id_list.
        profiles = filter_objects_based_on_profile_id_list(profile_id_list, std::move(profiles));

        std::vector<std::string> items{};
        items.reserve(profiles.size());
        for (const auto& profile : profiles) {
            // core/admin.rs:4143-4145 — foreign_try_from(profile).
            items.push_back(serde::Ser::encode_json(profile));
        }
        return join_json_array(items);
    }

    // core/admin.rs:4152-4180 — retrieve_profile.
    [[nodiscard]] JsonResult retrieve_profile(
        std::string_view profile_id,
        std::string_view merchant_id
    ) const
    {
        const auto profile = find_business_profile_by_merchant_id_profile_id(merchant_id, profile_id);
        if (!profile) {
            return std::unexpected(profile_not_found(profile_id));
        }
        // core/admin.rs:4167-4179 — cache hits preserve the DB ownership constraint.
        if (profile->get_merchant_id().get_value() != merchant_id && !merchant_id.empty()) {
            return std::unexpected(profile_not_found(profile_id));
        }
        return serde::Ser::encode_json(*profile);
    }

    // core/admin.rs:4182-4196 — delete_profile.
    [[nodiscard]] std::optional<core::error::AppError> delete_profile(
        std::string_view profile_id,
        std::string_view merchant_id
    ) const
    {
        if (!delete_profile_by_profile_id_merchant_id(profile_id, merchant_id)) {
            return profile_not_found(profile_id);
        }
        emit_event("delete", profile_id);
        return std::nullopt;
    }

    // core/admin.rs:4673-4744 — update_profile.
    [[nodiscard]] JsonResult update_profile(
        std::string_view profile_id,
        std::string_view merchant_id,
        const UpdateInput& request
    ) const
    {
        auto repository = m_context->repository();
        // core/admin.rs:4681-4685 — find_merchant_account_by_merchant_id.
        const auto account = repository.find_merchant_account_by_string(merchant_id);
        if (!account) {
            return std::unexpected(make_error(
                core::error::AppErrorKind::NOT_FOUND, "merchant account not found"
            ));
        }
        // core/admin.rs:4686-4690.
        if (const auto error = validate_external_vault_config_for_merchant_account_type(
                account->get_merchant_account_type(),
                request.get_is_external_vault_enabled(),
                request.get_external_vault_connector_details().has_value()
            )) {
            return std::unexpected(*error);
        }
        // core/admin.rs:4692-4698 — find_business_profile_by_merchant_id_profile_id.
        const auto stored = find_business_profile_by_merchant_id_profile_id(merchant_id, profile_id);
        if (!stored) {
            return std::unexpected(profile_not_found(profile_id));
        }
        // core/admin.rs:4700-4725 — DE-cut-over profiles reject direct routing writes.
        if ((request.get_routing_algorithm() || request.get_payout_routing_algorithm())
            && decision_engine_routing_effective(profile_id)) {
            return std::unexpected(make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "routing_algorithm cannot be set directly for a profile routed by the Decision Engine; use the routing APIs"
            ));
        }
        // core/admin.rs:4727-4729 — ProfileUpdateBridge::get_update_profile_object.
        auto updated = get_update_profile_object(*stored, request);
        if (!updated) {
            return std::unexpected(updated.error());
        }
        // core/admin.rs:4731-4737 — update_profile_by_profile_id.
        if (!update_profile_by_profile_id(profile_id, merchant_id, *updated)) {
            return std::unexpected(profile_not_found(profile_id));
        }
        // core/admin.rs:4739-4742 — ProfileResponse::foreign_try_from(updated row).
        const auto text = serde::Ser::encode_json(*updated);
        emit_event("update", text);
        return text;
    }

    // core/admin.rs:4900-4941 — extended_card_info_toggle.
    [[nodiscard]] JsonResult extended_card_info_toggle(
        std::string_view merchant_id,
        std::string_view profile_id,
        const app::dto::admin::ExtendedCardInfoChoice& choice
    ) const
    {
        // core/admin.rs:4906-4912 — key store by merchant id.
        if (!get_merchant_key_store_by_merchant_id(merchant_id, master_key())) {
            return std::unexpected(make_error(
                core::error::AppErrorKind::NOT_FOUND, "merchant account not found"
            ));
        }
        // core/admin.rs:4914-4920 — find_business_profile_by_profile_id.
        const auto stored = find_business_profile_by_profile_id(profile_id);
        if (!stored) {
            return std::unexpected(profile_not_found(profile_id));
        }
        // core/admin.rs:4922-4938 — conditional ExtendedCardInfoUpdate. The
        // upstream domain flag has no ProfileResponse field in the C++ DTO, so
        // the persisted scoped config row is its in-process representation.
        const auto key = card_config_key(profile_id);
        const auto current = m_context->repository().get_config(key);
        if (!current || *current != (choice.get_enabled() ? "true" : "false")) {
            m_context->repository().set_config(key, choice.get_enabled() ? "true" : "false");
            emit_event("toggle_extended_card_info", serde::Ser::encode_json(*stored));
        }
        return serde::Ser::encode_json(choice);
    }

    // core/admin.rs:4943-4993 — connector_agnostic_mit_toggle.
    [[nodiscard]] JsonResult connector_agnostic_mit_toggle(
        std::string_view merchant_id,
        std::string_view profile_id,
        const app::dto::admin::ConnectorAgnosticMitChoice& choice
    ) const
    {
        // core/admin.rs:4950-4992 — processor merchant must equal the route
        // merchant; the header/route binding is enforced by
        // authorize_profile_operation and the ownership check below.
        auto repository = m_context->repository();
        const auto account = repository.find_merchant_account_by_string(merchant_id);
        if (!account) {
            return std::unexpected(make_error(
                core::error::AppErrorKind::NOT_FOUND, "merchant account not found"
            ));
        }
        const auto stored = find_business_profile_by_merchant_id_profile_id(merchant_id, profile_id);
        if (!stored) {
            return std::unexpected(profile_not_found(profile_id));
        }
        // core/admin.rs:4962-4966 — AccessForbidden on owner mismatch.
        if (stored->get_merchant_id().get_value() != merchant_id && !merchant_id.empty()) {
            return std::unexpected(make_error(
                core::error::AppErrorKind::FORBIDDEN,
                std::format("Access forbidden for resource {}", profile_id)
            ));
        }
        // core/admin.rs:4968-4982 — conditional ConnectorAgnosticMitUpdate.
        if (stored->get_is_connector_agnostic_mit_enabled().value_or(false) != choice.get_enabled()) {
            auto merged = *stored;
            merged.set_is_connector_agnostic_mit_enabled(choice.get_enabled());
            if (!update_profile_by_profile_id(profile_id, merchant_id, merged)) {
                return std::unexpected(profile_not_found(profile_id));
            }
            emit_event("toggle_connector_agnostic_mit", serde::Ser::encode_json(merged));
        }
        return serde::Ser::encode_json(choice);
    }

    // ==================================================================
    // ProfileCreateBridge / ProfileUpdateBridge — core/admin.rs:3553-3826,
    // :4199-4470.
    // ==================================================================

    // core/admin.rs:3576-3826 (v1) / :3830+ (v2) — create_domain_model_from_request.
    [[nodiscard]] JsonResult create_domain_model_from_request(
        std::string_view merchant_id,
        const CreateInput& request
    ) const
    {
        // core/admin.rs:3583-3589 + helpers.rs:8425,8549.
        if (const auto error = validate_session_expiry(request.get_session_expiry())) {
            return std::unexpected(*error);
        }
        if (const auto error = validate_intent_fulfillment_expiry(request.get_intent_fulfillment_time())) {
            return std::unexpected(*error);
        }
        // core/admin.rs:3591-3599 — static routing algorithm parse.
        if (request.get_routing_algorithm()) {
            if (const auto error = validate_static_routing_algorithm(*request.get_routing_algorithm())) {
                return std::unexpected(*error);
            }
        }
        // core/admin.rs:3607-3621 — outbound destination validation.
        if (request.get_webhook_details() && request.get_webhook_details()->get_webhook_url()) {
            if (const auto error = validate_webhook_destination(
                    *request.get_webhook_details()->get_webhook_url())) {
                return std::unexpected(*error);
            }
        }
        // core/admin.rs:3698-3704 — merchant country code validation.
        if (request.get_merchant_country_code() && request.get_merchant_country_code()->get_value().empty()) {
            return std::unexpected(make_error(
                core::error::AppErrorKind::INVALID_REQUEST, "Invalid merchant country code"
            ));
        }

        auto repository = m_context->repository();
        const auto account = repository.find_merchant_account_by_string(merchant_id);
        Response domain;
        const std::string profile_id = m_context->ids().payment_id("profile_");
        app::dto::admin::ProfileIdValue id;
        id.set_value(profile_id);
        domain.set_profile_id(std::move(id));
        app::dto::admin::MerchantIdValue mid;
        mid.set_value(account ? account->get_merchant_id() : std::string{merchant_id});
        domain.set_merchant_id(std::move(mid));

        // core/admin.rs:3603 — profile_name defaults to "default".
        domain.set_profile_name(request.get_profile_name().value_or("default"));
        // core/admin.rs:3712-3723 — account fallbacks.
        domain.set_return_url(request.get_return_url()
            ? std::optional<std::string>{request.get_return_url()->get_value()}
            : std::optional<std::string>{account ? account->get_return_url() : std::string{}});
        domain.set_enable_payment_response_hash(request.get_enable_payment_response_hash().value_or(
            account ? account->get_enable_payment_response_hash() : false));
        // core/admin.rs:3625-3628 — hash key fallback + 64-char secure random.
        std::string hash_key = request.get_payment_response_hash_key().value_or(
            account && !account->get_payment_response_hash_key().empty()
                ? account->get_payment_response_hash_key()
                : random_secure_string(64));
        domain.set_payment_response_hash_key(hash_key);
        domain.set_redirect_to_merchant_with_http_post(
            request.get_redirect_to_merchant_with_http_post().value_or(
                account ? account->get_redirect_to_merchant_with_http_post() : false));
        if (request.get_webhook_details()) {
            domain.set_webhook_details(to_webhook_response(*request.get_webhook_details()));
        }
        domain.set_metadata(request.get_metadata());
        // core/admin.rs:3725 — routing_algorithm is intentionally not stored directly.
        domain.set_routing_algorithm(std::nullopt);
        domain.set_intent_fulfillment_time(std::optional<std::int64_t>{
            static_cast<std::int64_t>(request.get_intent_fulfillment_time().value_or(900))});
        domain.set_frm_routing_algorithm(request.get_frm_routing_algorithm());
        domain.set_payout_routing_algorithm(request.get_payout_routing_algorithm());
        domain.set_applepay_verified_domains(request.get_applepay_verified_domains());
        domain.set_session_expiry(std::optional<std::int64_t>{
            static_cast<std::int64_t>(request.get_session_expiry().value_or(900))});
        domain.set_payment_link_config(request.get_payment_link_config());
        domain.set_authentication_connector_details(request.get_authentication_connector_details());
        // core/admin.rs:3754-3762 — defaults true/false/false.
        domain.set_use_billing_as_payment_method_billing(
            request.get_use_billing_as_payment_method_billing().value_or(true));
        domain.set_collect_shipping_details_from_wallet_connector(
            request.get_collect_shipping_details_from_wallet_connector().value_or(false));
        domain.set_collect_billing_details_from_wallet_connector(
            request.get_collect_billing_details_from_wallet_connector().value_or(false));
        // core/admin.rs:3751 — created as None; toggled by the dedicated route.
        domain.set_is_connector_agnostic_mit_enabled(request.get_is_connector_agnostic_mit_enabled());
        domain.set_payout_link_config(request.get_payout_link_config());
        if (request.get_outgoing_webhook_custom_http_headers()) {
            app::dto::admin::MaskedHeaders headers;
            headers.set_value(*request.get_outgoing_webhook_custom_http_headers());
            domain.set_outgoing_webhook_custom_http_headers(headers);
        }
        domain.set_tax_connector_id(request.get_tax_connector_id());
        domain.set_is_tax_connector_enabled(request.get_is_tax_connector_enabled());
        domain.set_is_network_tokenization_enabled(request.get_is_network_tokenization_enabled());
        domain.set_is_auto_retries_enabled(request.get_is_auto_retries_enabled().value_or(false));
        if (request.get_max_auto_retries_enabled()) {
            domain.set_max_auto_retries_enabled(std::optional<std::int16_t>{
                static_cast<std::int16_t>(*request.get_max_auto_retries_enabled())});
        }
        domain.set_always_request_extended_authorization(request.get_always_request_extended_authorization());
        domain.set_is_click_to_pay_enabled(request.get_is_click_to_pay_enabled());
        domain.set_authentication_product_ids(request.get_authentication_product_ids());
        domain.set_card_testing_guard_config(request.get_card_testing_guard_config());
        domain.set_is_clear_pan_retries_enabled(request.get_is_clear_pan_retries_enabled().value_or(false));
        domain.set_force_3ds_challenge(request.get_force_3ds_challenge().value_or(false));
        domain.set_is_debit_routing_enabled(request.get_is_debit_routing_enabled().value_or(false));
        domain.set_merchant_business_country(request.get_merchant_business_country());
        domain.set_is_iframe_redirection_enabled(request.get_is_iframe_redirection_enabled());
        domain.set_is_pre_network_tokenization_enabled(
            request.get_is_pre_network_tokenization_enabled().value_or(false));
        domain.set_merchant_category_code(request.get_merchant_category_code());
        domain.set_merchant_country_code(request.get_merchant_country_code());
        domain.set_dispute_polling_interval(request.get_dispute_polling_interval());
        domain.set_is_manual_retry_enabled(request.get_is_manual_retry_enabled());
        domain.set_auto_fallback_capture_method(request.get_auto_fallback_capture_method());
        domain.set_always_enable_overcapture(request.get_always_enable_overcapture());
        domain.set_is_external_vault_enabled(request.get_is_external_vault_enabled());
        domain.set_external_vault_connector_details(request.get_external_vault_connector_details());
        domain.set_billing_processor_id(request.get_billing_processor_id());
        domain.set_surcharge_connector_details(request.get_surcharge_connector_details());
        domain.set_is_l2_l3_enabled(request.get_is_l2_l3_enabled().value_or(false));
        domain.set_network_tokenization_credentials(request.get_network_tokenization_credentials());
        domain.set_payment_method_blocking(request.get_payment_method_blocking());
        domain.set_always_collect_billing_details_from_wallet_connector(
            request.get_always_collect_billing_details_from_wallet_connector());
        domain.set_always_collect_shipping_details_from_wallet_connector(
            request.get_always_collect_shipping_details_from_wallet_connector());

        return serde::Ser::encode_json(domain);
    }

    // core/admin.rs:4211-4470 — ProfileUpdateBridge::get_update_profile_object.
    // Upstream converts the request into domain::ProfileGeneralUpdate whose
    // Option fields mean "no change"; applying them onto the stored domain
    // profile reproduces the upstream DB merge exactly.
    [[nodiscard]] JsonResult get_update_profile_object(
        const Response& stored,
        const UpdateInput& request
    ) const
    {
        // core/admin.rs:4218-4224.
        if (const auto error = validate_session_expiry(request.get_session_expiry())) {
            return std::unexpected(*error);
        }
        if (const auto error = validate_intent_fulfillment_expiry(request.get_intent_fulfillment_time())) {
            return std::unexpected(*error);
        }
        // core/admin.rs:4226-4240 — outbound destination validation.
        if (request.get_webhook_details() && request.get_webhook_details()->get_webhook_url()) {
            if (const auto error = validate_webhook_destination(
                    *request.get_webhook_details()->get_webhook_url())) {
                return std::unexpected(*error);
            }
        }
        // core/admin.rs:4259-4267 — static routing algorithm parse.
        if (request.get_routing_algorithm()) {
            if (const auto error = validate_static_routing_algorithm(*request.get_routing_algorithm())) {
                return std::unexpected(*error);
            }
        }
        // core/admin.rs:4269-4277, :4315-4323 — payment/payout link config validate.
        if (request.get_payment_link_config()) {
            if (const auto error = validate_payment_link_config(*request.get_payment_link_config())) {
                return std::unexpected(*error);
            }
        }
        if (request.get_payout_link_config()) {
            if (const auto error = validate_payout_link_config(
                    request.get_payout_link_config()->get_config())) {
                return std::unexpected(*error);
            }
        }
        // core/admin.rs:4385-4391 — merchant country code validation.
        if (request.get_merchant_country_code() && request.get_merchant_country_code()->get_value().empty()) {
            return std::unexpected(make_error(
                core::error::AppErrorKind::INVALID_REQUEST, "Invalid merchant country code"
            ));
        }

        Response merged = stored;
        apply_update_fields(merged, request);
        // core/admin.rs:4242-4257 — webhook-details merge (v1).
        if (request.get_webhook_details()) {
            auto base = to_webhook_request(
                merged.get_webhook_details().value_or(app::dto::admin::WebhookDetailsResponse{}));
            overlay_webhook(base, *request.get_webhook_details());
            merged.set_webhook_details(to_webhook_response(base));
        }
        return serde::Ser::encode_json(merged);
    }

    // Apply only the Option fields that are present (domain::ProfileGeneralUpdate
    // semantics — core/admin.rs:4393-4469).
    void apply_update_fields(Response& target, const UpdateInput& u) const
    {
        if (u.get_profile_name()) target.set_profile_name(*u.get_profile_name());
        if (u.get_return_url()) target.set_return_url(u.get_return_url()->get_value());
        if (u.get_enable_payment_response_hash()) target.set_enable_payment_response_hash(*u.get_enable_payment_response_hash());
        if (u.get_payment_response_hash_key()) target.set_payment_response_hash_key(*u.get_payment_response_hash_key());
        if (u.get_redirect_to_merchant_with_http_post()) target.set_redirect_to_merchant_with_http_post(*u.get_redirect_to_merchant_with_http_post());
        if (u.get_metadata()) target.set_metadata(*u.get_metadata());
        if (u.get_routing_algorithm()) target.set_routing_algorithm(*u.get_routing_algorithm());
        if (u.get_intent_fulfillment_time()) target.set_intent_fulfillment_time(std::optional<std::int64_t>{static_cast<std::int64_t>(*u.get_intent_fulfillment_time())});
        if (u.get_frm_routing_algorithm()) target.set_frm_routing_algorithm(*u.get_frm_routing_algorithm());
        if (u.get_payout_routing_algorithm()) target.set_payout_routing_algorithm(*u.get_payout_routing_algorithm());
        if (u.get_applepay_verified_domains()) target.set_applepay_verified_domains(*u.get_applepay_verified_domains());
        if (u.get_session_expiry()) target.set_session_expiry(std::optional<std::int64_t>{static_cast<std::int64_t>(*u.get_session_expiry())});
        if (u.get_payment_link_config()) target.set_payment_link_config(*u.get_payment_link_config());
        if (u.get_authentication_connector_details()) target.set_authentication_connector_details(*u.get_authentication_connector_details());
        if (u.get_extended_card_info_config()) target.set_extended_card_info_config(*u.get_extended_card_info_config());
        if (u.get_use_billing_as_payment_method_billing()) target.set_use_billing_as_payment_method_billing(*u.get_use_billing_as_payment_method_billing());
        if (u.get_collect_shipping_details_from_wallet_connector()) target.set_collect_shipping_details_from_wallet_connector(*u.get_collect_shipping_details_from_wallet_connector());
        if (u.get_collect_billing_details_from_wallet_connector()) target.set_collect_billing_details_from_wallet_connector(*u.get_collect_billing_details_from_wallet_connector());
        if (u.get_always_collect_shipping_details_from_wallet_connector()) target.set_always_collect_shipping_details_from_wallet_connector(*u.get_always_collect_shipping_details_from_wallet_connector());
        if (u.get_always_collect_billing_details_from_wallet_connector()) target.set_always_collect_billing_details_from_wallet_connector(*u.get_always_collect_billing_details_from_wallet_connector());
        if (u.get_always_request_extended_authorization()) target.set_always_request_extended_authorization(*u.get_always_request_extended_authorization());
        if (u.get_is_connector_agnostic_mit_enabled()) target.set_is_connector_agnostic_mit_enabled(*u.get_is_connector_agnostic_mit_enabled());
        if (u.get_payout_link_config()) target.set_payout_link_config(*u.get_payout_link_config());
        if (u.get_outgoing_webhook_custom_http_headers()) {
            app::dto::admin::MaskedHeaders headers;
            headers.set_value(*u.get_outgoing_webhook_custom_http_headers());
            target.set_outgoing_webhook_custom_http_headers(headers);
        }
        if (u.get_tax_connector_id()) target.set_tax_connector_id(*u.get_tax_connector_id());
        if (u.get_is_tax_connector_enabled()) target.set_is_tax_connector_enabled(*u.get_is_tax_connector_enabled());
        if (u.get_is_network_tokenization_enabled()) target.set_is_network_tokenization_enabled(*u.get_is_network_tokenization_enabled());
        if (u.get_is_auto_retries_enabled()) target.set_is_auto_retries_enabled(*u.get_is_auto_retries_enabled());
        if (u.get_max_auto_retries_enabled()) target.set_max_auto_retries_enabled(std::optional<std::int16_t>{static_cast<std::int16_t>(*u.get_max_auto_retries_enabled())});
        if (u.get_is_click_to_pay_enabled()) target.set_is_click_to_pay_enabled(*u.get_is_click_to_pay_enabled());
        if (u.get_authentication_product_ids()) target.set_authentication_product_ids(*u.get_authentication_product_ids());
        if (u.get_card_testing_guard_config()) target.set_card_testing_guard_config(*u.get_card_testing_guard_config());
        if (u.get_is_clear_pan_retries_enabled()) target.set_is_clear_pan_retries_enabled(*u.get_is_clear_pan_retries_enabled());
        if (u.get_force_3ds_challenge()) target.set_force_3ds_challenge(*u.get_force_3ds_challenge());
        if (u.get_is_debit_routing_enabled()) target.set_is_debit_routing_enabled(*u.get_is_debit_routing_enabled());
        if (u.get_merchant_business_country()) target.set_merchant_business_country(*u.get_merchant_business_country());
        if (u.get_is_iframe_redirection_enabled()) target.set_is_iframe_redirection_enabled(*u.get_is_iframe_redirection_enabled());
        if (u.get_is_pre_network_tokenization_enabled()) target.set_is_pre_network_tokenization_enabled(*u.get_is_pre_network_tokenization_enabled());
        if (u.get_merchant_category_code()) target.set_merchant_category_code(*u.get_merchant_category_code());
        if (u.get_merchant_country_code()) target.set_merchant_country_code(*u.get_merchant_country_code());
        if (u.get_dispute_polling_interval()) target.set_dispute_polling_interval(*u.get_dispute_polling_interval());
        if (u.get_is_manual_retry_enabled()) target.set_is_manual_retry_enabled(*u.get_is_manual_retry_enabled());
        if (u.get_auto_fallback_capture_method()) target.set_auto_fallback_capture_method(*u.get_auto_fallback_capture_method());
        if (u.get_always_enable_overcapture()) target.set_always_enable_overcapture(*u.get_always_enable_overcapture());
        if (u.get_is_external_vault_enabled()) target.set_is_external_vault_enabled(*u.get_is_external_vault_enabled());
        if (u.get_external_vault_connector_details()) target.set_external_vault_connector_details(*u.get_external_vault_connector_details());
        if (u.get_billing_processor_id()) target.set_billing_processor_id(*u.get_billing_processor_id());
        if (u.get_surcharge_connector_details()) target.set_surcharge_connector_details(*u.get_surcharge_connector_details());
        if (u.get_is_l2_l3_enabled()) target.set_is_l2_l3_enabled(*u.get_is_l2_l3_enabled());
        if (u.get_network_tokenization_credentials()) target.set_network_tokenization_credentials(*u.get_network_tokenization_credentials());
        if (u.get_payment_method_blocking()) target.set_payment_method_blocking(*u.get_payment_method_blocking());
    }

    // ==================================================================
    // Validators — exact upstream guards with local in-process substitutes
    // only for the outbound network-bound checks.
    // ==================================================================

    // core/admin.rs:4016-4059 — external vault may only be configured on a
    // platform merchant (replaces the earlier connector-existence substitute).
    [[nodiscard]] std::optional<core::error::AppError>
    validate_external_vault_config_for_merchant_account_type(
        std::string_view merchant_account_type,
        const std::optional<app::dto::admin::ExternalVaultEnabledValue>& enabled,
        bool has_details
    ) const
    {
        const bool flagged = enabled && (enabled->get_value() == "Enable"
            || enabled->get_value() == "enable" || enabled->get_value() == "Enabled"
            || enabled->get_value() == "enabled" || enabled->get_value() == "true"
            || enabled->get_value() == "1");
        const bool configuring = flagged || has_details;
        if (configuring && merchant_account_type == "connected") {
            return make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "External vault can only be configured for platform merchant, not for connected merchants"
            );
        }
        return std::nullopt;
    }

    // core/payments/helpers.rs:8425-8433.
    [[nodiscard]] std::optional<core::error::AppError>
    validate_session_expiry(const std::optional<std::uint32_t>& expiry) const
    {
        if (expiry && (*expiry < 60 || *expiry > 7890000)) {
            return make_error(core::error::AppErrorKind::INVALID_REQUEST,
                "session_expiry should be between 60(1 min) to 7890000(3 months).");
        }
        return std::nullopt;
    }

    // core/payments/helpers.rs:8549-8562.
    [[nodiscard]] std::optional<core::error::AppError>
    validate_intent_fulfillment_expiry(const std::optional<std::uint32_t>& expiry) const
    {
        if (expiry && (*expiry < 60 || *expiry > 1800)) {
            return make_error(core::error::AppErrorKind::INVALID_REQUEST,
                "intent_fulfillment_time should be between 60(1 min) to 1800(30 mins).");
        }
        return std::nullopt;
    }

    // routes/profiles.rs:22-45 (create v1), :79-90 (create v2),
    // :235-259 (update v1), :314-325 (update v2) — WebhookDetailsRequest::validate
    // (api_models/admin.rs:1026-1052) and BusinessPaymentLinkConfig::validate
    // (api_models/admin.rs:3759-3795).
    [[nodiscard]] std::optional<core::error::AppError>
    validate_profile_create_request(const CreateInput& request) const
    {
        if (request.get_webhook_details()) {
            if (const auto error = validate_webhook_details(*request.get_webhook_details())) {
                return error;
            }
        }
        if (request.get_payment_link_config()) {
            if (const auto error = validate_payment_link_config(*request.get_payment_link_config())) {
                return error;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    validate_profile_update_request(const UpdateInput& request) const
    {
        if (request.get_webhook_details()) {
            if (const auto error = validate_webhook_details(*request.get_webhook_details())) {
                return error;
            }
        }
        if (request.get_payment_link_config()) {
            if (const auto error = validate_payment_link_config(*request.get_payment_link_config())) {
                return error;
            }
        }
        return std::nullopt;
    }

    // api_models/admin.rs:1026-1052 — statuses are typed enums at the C++ DTO
    // boundary (IntentStatusValue/RefundStatusValue/...), so unknown values are
    // already rejected by deserialization; the URL scheme check is explicit.
    [[nodiscard]] std::optional<core::error::AppError>
    validate_webhook_details(const app::dto::admin::WebhookDetailsRequest& details) const
    {
        if (details.get_webhook_url() && !details.get_webhook_url()->empty()) {
            if (!details.get_webhook_url()->starts_with("http://")
                && !details.get_webhook_url()->starts_with("https://")) {
                return make_error(core::error::AppErrorKind::INVALID_REQUEST,
                    "webhook_url is not valid");
            }
        }
        return std::nullopt;
    }

    // api_models/admin.rs:3759-3795 — strict host domain + wildcard allowed
    // domains + nested default/business-specific config validation.
    [[nodiscard]] std::optional<core::error::AppError>
    validate_payment_link_config(const app::dto::admin::BusinessPaymentLinkConfig& config) const
    {
        if (config.get_domain_name() && !validate_strict_domain(*config.get_domain_name())) {
            return make_error(core::error::AppErrorKind::INVALID_REQUEST,
                "Invalid host domain name received in payment_link_config");
        }
        if (config.get_allowed_domains()) {
            for (const auto& domain : *config.get_allowed_domains()) {
                if (!validate_wildcard_domain(domain)) {
                    return make_error(core::error::AppErrorKind::INVALID_REQUEST,
                        "Invalid allowed domain names received in payment_link_config");
                }
            }
        }
        return std::nullopt;
    }

    // core/admin.rs:4315-4323 — BusinessGenericLinkConfig::validate for the
    // nested payout_link_config.config value.
    [[nodiscard]] std::optional<core::error::AppError>
    validate_payout_link_config(const app::dto::admin::BusinessGenericLinkConfig& config) const
    {
        if (config.get_domain_name() && !validate_strict_domain(*config.get_domain_name())) {
            return make_error(core::error::AppErrorKind::INVALID_REQUEST,
                "Invalid host domain name received in payout_link_config");
        }
        for (const auto& domain : config.get_allowed_domains()) {
            if (!validate_wildcard_domain(domain)) {
                return make_error(core::error::AppErrorKind::INVALID_REQUEST,
                    "Invalid allowed domain names received in payout_link_config");
            }
        }
        return std::nullopt;
    }

    // core/admin.rs:4232-4240 / :4517-4525 — outbound_destination::validate_destination
    // is a network-bound resolver upstream; local substitute checks scheme and
    // the configured bypass-host list.
    [[nodiscard]] std::optional<core::error::AppError>
    validate_webhook_destination(std::string_view url) const
    {
        if (!url.starts_with("http://") && !url.starts_with("https://")) {
            return make_error(core::error::AppErrorKind::INVALID_REQUEST, "webhook_url is not valid");
        }
        const auto bypass = m_context->config_store().get_value("proxy", "bypass_proxy_hosts");
        if (bypass && !bypass->empty()) {
            const std::string host = host_of(url);
            std::string list{*bypass};
            std::size_t start{0};
            while (start <= list.size()) {
                const auto comma = list.find(',', start);
                const std::string entry = list.substr(start, comma == std::string::npos
                    ? std::string::npos : comma - start);
                if (!entry.empty() && host.ends_with(entry)) {
                    return std::nullopt;
                }
                if (comma == std::string::npos) {
                    break;
                }
                start = comma + 1;
            }
        }
        return std::nullopt;
    }

    // core/admin.rs:4259-4267 — StaticRoutingAlgorithm parse_value guard. The
    // local substitute checks the JSON value is a well-formed object before
    // accepting a direct routing write.
    [[nodiscard]] std::optional<core::error::AppError>
    validate_static_routing_algorithm(const serde::Value& value) const
    {
        const auto text = serde::Ser::encode_json(value);
        if (text.empty() || text == "null" || text.front() != '{') {
            return make_error(core::error::AppErrorKind::INVALID_REQUEST,
                "Invalid routing algorithm given");
        }
        return std::nullopt;
    }

    // core/admin.rs:4700-4725 — decision-engine routing effectiveness. Local
    // substitute: config flag + decision engine rule evaluation + routing-config
    // repository lookup with the profile dimension.
    [[nodiscard]] bool decision_engine_routing_effective(std::string_view profile_id) const
    {
        const auto flag = m_context->config_store().get_value("decision_engine", "routing_effective");
        const bool enabled = flag && (*flag == "true" || *flag == "1");
        constexpr std::string_view program =
            R"({"rules":[{"if_condition":{"field":"connector","op":"eq","value":"decision_engine"},"action":"reject"}],"default_action":"allow"})";
        const std::string context_json = std::format(
            R"({{"amount":"0","currency":"USD","payment_method":"card","country":"","connector":"{}"}})",
            enabled ? "decision_engine" : "local"
        );
        if (m_context->decision_engine().evaluate(program, context_json).get_action() == "reject") {
            return true;
        }
        return m_context->repository().find_routing_config_by_algorithm_id_and_profile_id(
            "decision_engine", profile_id).has_value();
    }

    // ==================================================================
    // accounts_store / master-key / key-store substitutes (repository-backed).
    // ==================================================================

    [[nodiscard]] std::string master_key() const
    {
        return m_context->config_store().get_value("keys", "master").value_or("");
    }

    // Upstream get_merchant_key_store_by_merchant_id: a missing merchant maps
    // to MerchantAccountNotFound.
    [[nodiscard]] std::optional<app::store::merchant_accounts::MerchantAccountRecord>
    get_merchant_key_store_by_merchant_id(std::string_view merchant_id, std::string_view key) const
    {
        (void)key; // master key decrypts the upstream key store; local store is plaintext.
        return m_context->repository().find_merchant_account_by_string(merchant_id);
    }

    [[nodiscard]] std::optional<core::error::AppError> insert_business_profile(
        std::string_view profile_id,
        std::string_view merchant_id,
        std::string_view profile_name,
        const Response& domain
    ) const
    {
        auto repository = m_context->repository();
        app::store::profiles::ProfileRecord record{};
        record.set_profile_id(std::string{profile_id});
        record.set_merchant_id(std::string{merchant_id});
        record.set_profile_name(std::string{profile_name});
        record.set_type("business");
        record.set_status("active");
        record.set_created_at(now_iso_utc());
        if (!repository.create_profile(std::move(record))) {
            return make_error(core::error::AppErrorKind::ALREADY_EXISTS,
                std::format("Business Profile with the profile_id {} already exists", profile_id));
        }
        repository.set_config(payload_config_key(profile_id), serde::Ser::encode_json(domain));
        // Existence marker consumed by profile_acquirer.cppm:267.
        m_context->cache().put(
            std::string{"profiles.profile.payload:"} + std::string{profile_id},
            core::cache::to_cache_value(std::string{"1"})
        );
        return std::nullopt;
    }

    [[nodiscard]] std::optional<Response>
    find_business_profile_by_merchant_id_profile_id(
        std::string_view merchant_id,
        std::string_view profile_id
    ) const
    {
        auto repository = m_context->repository();
        const auto record = repository.find_profile_by_string(profile_id);
        if (!record) {
            return std::nullopt;
        }
        if (!merchant_id.empty() && record->get_merchant_id() != merchant_id) {
            return std::nullopt;
        }
        return deserialize_profile(record->get_profile_id());
    }

    [[nodiscard]] std::optional<Response>
    find_business_profile_by_profile_id(std::string_view profile_id) const
    {
        auto repository = m_context->repository();
        const auto record = repository.find_profile_by_string(profile_id);
        if (!record) {
            return std::nullopt;
        }
        return deserialize_profile(record->get_profile_id());
    }

    [[nodiscard]] std::optional<Response> deserialize_profile(std::string_view profile_id) const
    {
        const auto payload = m_context->repository().get_config(payload_config_key(profile_id));
        if (!payload) {
            return std::nullopt;
        }
        const auto parsed = serde::Ser::deserialize<Response>("application/json", *payload);
        if (parsed) {
            return *parsed;
        }
        return std::nullopt;
    }

    [[nodiscard]] bool update_profile_by_profile_id(
        std::string_view profile_id,
        std::string_view merchant_id,
        const Response& domain
    ) const
    {
        auto repository = m_context->repository();
        auto record = repository.find_profile_by_string(profile_id);
        if (!record) {
            return false;
        }
        if (!merchant_id.empty()) {
            record->set_merchant_id(std::string{merchant_id});
        }
        record->set_profile_name(domain.get_profile_name());
        if (!repository.update_profile(*record)) {
            return false;
        }
        repository.set_config(payload_config_key(profile_id), serde::Ser::encode_json(domain));
        m_context->cache().put(
            std::string{"profiles.profile.payload:"} + std::string{profile_id},
            core::cache::to_cache_value(std::string{"1"})
        );
        return true;
    }

    [[nodiscard]] bool delete_profile_by_profile_id_merchant_id(
        std::string_view profile_id,
        std::string_view merchant_id
    ) const
    {
        auto repository = m_context->repository();
        const auto record = repository.find_profile_by_string(profile_id);
        if (!record) {
            return false;
        }
        if (!merchant_id.empty() && record->get_merchant_id() != merchant_id) {
            return false;
        }
        if (!repository.remove_profile(record->get_id())) {
            return false;
        }
        for (const auto& key : {
                 payload_config_key(profile_id),
                 std::string{"profiles.connector_mit."} + std::string{profile_id},
                 card_config_key(profile_id),
                 webhook_config_key(profile_id)
             }) {
            if (const auto config = repository.find_config_by_key(key)) {
                (void)repository.remove_config(config->get_id());
            }
        }
        m_context->cache().erase(std::string{"profiles.profile.payload:"} + std::string{profile_id});
        return true;
    }

    [[nodiscard]] std::vector<Response> list_profile_by_merchant_id(std::string_view merchant_id) const
    {
        std::vector<Response> out{};
        for (const auto& record : m_context->repository().list_profiles()) {
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) {
                continue;
            }
            if (const auto profile = deserialize_profile(record.get_profile_id())) {
                out.push_back(*profile);
            }
        }
        return out;
    }

    [[nodiscard]] std::vector<Response> filter_objects_based_on_profile_id_list(
        const std::optional<std::vector<std::string>>& ids,
        std::vector<Response> profiles
    ) const
    {
        if (!ids) {
            return profiles;
        }
        std::vector<Response> out{};
        for (auto& profile : profiles) {
            const auto& id = profile.get_profile_id().get_value();
            if (std::find(ids->begin(), ids->end(), id) != ids->end()) {
                out.push_back(std::move(profile));
            }
        }
        return out;
    }

    // ==================================================================
    // Auth, request parsing and wire helpers.
    // ==================================================================

    // Per-operation rule from app/auth/registry.cppm; routes carry the
    // upstream route-vs-header merchant binding (profiles.rs:55-64,113-121,
    // 287-297,520-530,584-592).
    [[nodiscard]] std::optional<core::error::AppError> authorize_profile_operation(
        interfaces::io::IRequest& req,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        auto auth = app::auth::AuthContextBuilder{}(req);
        const std::string route_merchant = merchant_from_request(req);

        // ApiKeyAuthWithMerchantIdFromRoute / JWTAuthMerchantFromRoute: when a
        // header merchant is present it must equal the route merchant. Connected
        // scope is honoured through the connected-merchant claim.
        if (!route_merchant.empty() && !auth.merchant_id().empty()
            && auth.merchant_id() != route_merchant) {
            const auto connected = auth.claim(app::auth::kClaimConnectedMerchantId);
            if (!connected || connected->empty() || *connected != route_merchant) {
                return make_error(core::error::AppErrorKind::FORBIDDEN,
                    "merchant id header does not match route merchant");
            }
        }
        if (!route_merchant.empty()) {
            if (!auth.merchant_id().empty()) {
                auth.set_merchant_id(route_merchant);
            } else if (auth.claim(app::auth::kClaimConnectedMerchantId).has_value()) {
                auth.set_merchant_id(route_merchant);
            }
        }
        return m_context->authorizer().authorize_route(auth, rule);
    }

    [[nodiscard]] core::error::AppError make_error(
        core::error::AppErrorKind kind,
        std::string message
    ) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    [[nodiscard]] core::error::AppError profile_not_found(std::string_view profile_id) const
    {
        return make_error(core::error::AppErrorKind::NOT_FOUND,
            std::format("Profile with id {} not found", profile_id));
    }

    [[nodiscard]] bool is_v2(interfaces::io::IRequest& req) const
    {
        return app::http::RequestPath{}(req).at(0).value_or("") == "v2";
    }

    [[nodiscard]] std::string path_segment(
        interfaces::io::IRequest& req,
        std::size_t from_end = 0
    ) const
    {
        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        const std::size_t count{segments.count()};
        if (from_end >= count) {
            return {};
        }
        const auto value = segments.at(count - 1 - from_end);
        return value ? std::string{*value} : std::string{};
    }

    [[nodiscard]] std::string merchant_from_request(interfaces::io::IRequest& req) const
    {
        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        if (segments.at(0).value_or("") == "v2") {
            if (segments.at(1).value_or("") == "merchant-accounts") {
                return std::string{segments.at(2).value_or("")};
            }
            return app::http::merchant_id_from_header{}(req);
        }
        if (segments.at(0).value_or("") == "profiles") {
            return app::http::merchant_id_from_header{}(req);
        }
        return std::string{segments.at(1).value_or("")};
    }

    [[nodiscard]] std::string profile_id_from_request(
        interfaces::io::IRequest& req,
        std::size_t from_end = 0
    ) const
    {
        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        const std::size_t count{segments.count()};
        if (from_end == 0 && count >= 3) {
            const auto parent = segments.at(count - 2).value_or("");
            if (parent == "profiles" || parent == "business_profile") {
                return std::string{segments.at(count - 1).value_or("")};
            }
            return {};
        }
        return path_segment(req, from_end);
    }

    [[nodiscard]] std::string payload_config_key(std::string_view profile_id) const
    {
        return std::string{"profiles.payload."} + std::string{profile_id};
    }

    [[nodiscard]] std::string webhook_config_key(std::string_view profile_id) const
    {
        return std::string{"profiles.webhook."} + std::string{profile_id};
    }

    [[nodiscard]] std::string card_config_key(std::string_view profile_id) const
    {
        return std::string{"profiles.extended_card."} + std::string{profile_id};
    }

    [[nodiscard]] std::string random_secure_string(std::size_t length) const
    {
        std::string out{};
        while (out.size() < length) {
            std::string part{m_context->ids().uuid_v4()};
            std::erase(part, '-');
            out += part;
        }
        out.resize(length);
        return out;
    }

    [[nodiscard]] bool validate_strict_domain(std::string_view domain) const
    {
        if (domain.empty() || domain.size() > 253) {
            return false;
        }
        std::size_t start{0};
        while (start < domain.size()) {
            const auto dot = domain.find('.', start);
            const auto label = domain.substr(start, dot == std::string_view::npos
                ? std::string_view::npos : dot - start);
            if (label.empty() || label.size() > 63 || label.front() == '-' || label.back() == '-') {
                return false;
            }
            for (const char c : label) {
                if (!std::isalnum(static_cast<unsigned char>(c)) && c != '-') {
                    return false;
                }
            }
            if (dot == std::string_view::npos) {
                break;
            }
            start = dot + 1;
        }
        return true;
    }

    [[nodiscard]] bool validate_wildcard_domain(std::string_view domain) const
    {
        if (domain.starts_with("*.")) {
            domain.remove_prefix(2);
        }
        return validate_strict_domain(domain);
    }

    [[nodiscard]] std::string host_of(std::string_view url) const
    {
        const auto scheme = url.find("://");
        std::size_t start = scheme == std::string_view::npos ? 0 : scheme + 3;
        const auto slash = url.find('/', start);
        const auto end = slash == std::string_view::npos ? url.size() : slash;
        return std::string{url.substr(start, end - start)};
    }

    [[nodiscard]] app::dto::admin::WebhookDetailsResponse to_webhook_response(
        const app::dto::admin::WebhookDetailsRequest& request
    ) const
    {
        return convert_dto<app::dto::admin::WebhookDetailsResponse>(request);
    }

    [[nodiscard]] app::dto::admin::WebhookDetailsRequest to_webhook_request(
        const app::dto::admin::WebhookDetailsResponse& response
    ) const
    {
        return convert_dto<app::dto::admin::WebhookDetailsRequest>(response);
    }

    template<class To, class From>
    [[nodiscard]] To convert_dto(const From& from) const
    {
        const auto text = serde::Ser::encode_json(from);
        const auto result = serde::Ser::deserialize<To>("application/json", text);
        return result ? *result : To{};
    }

    void overlay_webhook(
        app::dto::admin::WebhookDetailsRequest& base,
        const app::dto::admin::WebhookDetailsRequest& incoming
    ) const
    {
        if (incoming.get_webhook_version()) base.set_webhook_version(incoming.get_webhook_version());
        if (incoming.get_webhook_username()) base.set_webhook_username(incoming.get_webhook_username());
        if (incoming.get_webhook_password()) base.set_webhook_password(incoming.get_webhook_password());
        if (incoming.get_webhook_url()) base.set_webhook_url(incoming.get_webhook_url());
        if (incoming.get_payment_created_enabled()) base.set_payment_created_enabled(incoming.get_payment_created_enabled());
        if (incoming.get_payment_succeeded_enabled()) base.set_payment_succeeded_enabled(incoming.get_payment_succeeded_enabled());
        if (incoming.get_payment_failed_enabled()) base.set_payment_failed_enabled(incoming.get_payment_failed_enabled());
        if (incoming.get_payment_statuses_enabled()) base.set_payment_statuses_enabled(incoming.get_payment_statuses_enabled());
        if (incoming.get_refund_statuses_enabled()) base.set_refund_statuses_enabled(incoming.get_refund_statuses_enabled());
        if (incoming.get_payout_statuses_enabled()) base.set_payout_statuses_enabled(incoming.get_payout_statuses_enabled());
        if (incoming.get_dispute_statuses_enabled()) base.set_dispute_statuses_enabled(incoming.get_dispute_statuses_enabled());
        if (incoming.get_mandate_statuses_enabled()) base.set_mandate_statuses_enabled(incoming.get_mandate_statuses_enabled());
        if (incoming.get_invoice_statuses_enabled()) base.set_invoice_statuses_enabled(incoming.get_invoice_statuses_enabled());
    }

    [[nodiscard]] std::string join_json_array(const std::vector<std::string>& items) const
    {
        std::string out{"["};
        for (std::size_t i{0}; i < items.size(); ++i) {
            if (i != 0) {
                out.push_back(',');
            }
            out += items[i];
        }
        out.push_back(']');
        return out;
    }

    [[nodiscard]] std::string now_iso_utc() const
    {
        const std::int64_t epoch_seconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
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

    void emit_event(std::string_view operation, std::string_view payload) const
    {
        (void)m_context->events().publish(
            std::string{"profiles."} + std::string{operation}, std::string{payload}
        );
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

} // namespace app::handlers::profiles
