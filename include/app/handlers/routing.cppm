module;
export module app_handlers:routing;

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
import app_services;

export namespace app::handlers::routing {

/**
 * Strict upstream parity port of the routing API.
 *
 * Upstream (SHA 7736d55adaf235f7af74e133ae408573b1826823):
 *   routes: crates/router/src/routes/routing.rs
 *   core:   crates/router/src/core/routing.rs
 *           crates/router/src/core/conditional_config.rs
 *           crates/router/src/core/surcharge_decision_config.rs
 *
 * Local in-process substitutes are used ONLY where upstream talks to an external Decision
 * Engine service (SSO code mint / provisioning / rule store / active-rule listing):
 * superposition config records + the deterministic crypto helpers stand in for that service.
 * Everything else — wire shapes, validation order, precondition errors and persistence —
 * follows the upstream functions named in docs/hyperswitch-parity/routing.md.
 */
class RoutingRouteHandlers
{
public:
    explicit RoutingRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    // ─── core::routing::routing_entry (core/routing.rs:151-239) ────────────────────
    void handle_routing_entry(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "routing_entry")) {
            return;
        }

        // Upstream takes RoutingEntryRequest from web::Query, not the body (routing.rs:26).
        const auto query = app::http::RequestQuery{}(req);
        const auto profile = profile_id_of(req);
        if (profile.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "profile_id"));
            return;
        }
        if (const auto validation_error = profile_validation_error(req, profile)) {
            send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error));
            return;
        }

        // Dimensions::with_* + is_decision_engine_routing_effective (routing.rs:160-166).
        const auto cutover = is_cutover(req, profile);

        app::model::routing::RoutingEntryResponse response;
        response.set_is_cutover(cutover);

        // Mint a one-time code only for (cutover, target) — routing.rs:169-231. DE network calls
        // are substituted by a deterministic local code + token (external-infra substitute).
        const auto target = query.get("target");
        if (cutover && target) {
            const auto merchant = merchant_id_of(req);
            const auto org = org_id_of(req);
            const auto key_id = m_context->ids().key_id();
            const auto digest = app::crypto::KeyedHasher{"de-sso"}.hash(key_id);
            std::uint64_t step{};
            std::from_chars(digest.data(), digest.data() + 8, step, 16);
            const auto code = app::crypto::TotpGenerator{}.generate("de-sso:" + merchant + ":" + profile, step);
            const auto payload = std::format(
                R"({{"merchant":"{}","org":"{}","profile":"{}","target":"{}","code":"{}"}})",
                merchant, org, profile, *target, code
            );
            const auto token = app::crypto::JwtSigner{"routing-de-sso"}.sign(payload);
            const auto cache_key = "routing.entry:" + code;
            m_context->cache().put(cache_key, core::cache::to_cache_value(token));
            const auto cached = m_context->cache().get(cache_key).value_or(std::string{});
            if (cached.empty()) {
                send_error(res, send, error(core::error::AppErrorKind::INTERNAL, "SSO token not cached"));
                return;
            }
            // state.conf.open_router.dashboard_url + "/" + target.dashboard_path() + "?code=" + code
            response.set_redirect_url(
                dashboard_url() + "/" + dashboard_path(*target) + "?code=" + code + "&token=" + cached
            );
        }
        track("routing_entry");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // ─── create_routing_algorithm_under_profile (core:826 v1 / :757 v2) ────────────
    void handle_routing_create_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "routing_create_config")) {
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = app::model::routing::RoutingConfigRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Required fields in upstream order (core:840-873).
        if (typed->get_name().empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "Name of config not given"));
            return;
        }
        if (typed->get_description().empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "Description of config not given"));
            return;
        }
        const auto txn = transaction_type_of(req);
        auto profile = typed->get_profile_id().get_value().empty() ? profile_id_of(req)
                                                                   : typed->get_profile_id().get_value();
        if (profile.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "Profile_id not provided"));
            return;
        }
        if (const auto validation_error = profile_validation_error(req, profile)) {
            send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error));
            return;
        }

        const auto algorithm_id = new_routing_id();
        auto config_json = serde::Ser::encode_json(*typed);
        // Persist the transaction type on the record (app.rs passes it into the core function).
        if (txn == "payout" && !config_json.empty() && config_json.back() == '}') {
            config_json.pop_back();
            config_json += R"(,"algorithm_for":"payout"})";
        }
        auto record = config_record(
            req, algorithm_id, typed->get_name(), typed->get_description(),
            algorithm_name(typed->get_algorithm()), profile, config_json
        );
        record.set_algorithm_id(algorithm_id);
        upsert_config(std::move(record));

        // v1/v2 response: RoutingDictionaryRecord (core:960-970 / :819, foreign_into).
        auto response = dictionary_record(algorithm_id, profile, typed->get_name(),
                                          typed->get_description(), algorithm_name(typed->get_algorithm()), txn);
        track("routing_create_config");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // ─── link_routing_config (core:1119 v1) / link_..._under_profile (:1049 v2) ─────
    void handle_routing_link_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "routing_link_config")) {
            return;
        }

        const auto body = req.body_as_string();
        using Link = app::model::routing::RoutingLinkWrapper;
        using Activate = app::model::routing::RoutingActivatePayload;

        std::string algorithm = algorithm_id_of(req);
        std::string payload_txn{};

        if (!body.empty()) {
            auto link = serde::Ser::deserialize<Link>("application/json", body);
            auto activate = serde::Ser::deserialize<Activate>("application/json", body);
            if (!link && !activate) {
                send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            if (link) {
                const auto& named = link->get_algorithm_id().get_routing_algorithm_id().get_value();
                if (!named.empty()) {
                    algorithm = named;
                }
                if (!link->get_profile_id().get_value().empty()) {
                    if (const auto validation_error = profile_validation_error(req, link->get_profile_id().get_value())) {
                        send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error));
                        return;
                    }
                }
            }
            if (activate && activate->get_transaction_type()) {
                payload_txn = activate->get_transaction_type()->get_value();
            }
        }
        if (algorithm.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "routing algorithm not found"));
            return;
        }

        const auto existing = find_config_by_algorithm(algorithm);
        if (!existing) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "routing algorithm not found"));
            return;
        }

        const auto txn = payload_txn.empty() ? transaction_type_of(req) : payload_txn;
        const auto record_txn = transaction_type_of(existing->get_config_json());
        if (record_txn != txn) {
            send_error(res, send, error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                std::format("Cannot use {}'s routing algorithm for {} operation", record_txn, txn)));
            return;
        }

        auto profile = existing->get_profile_id().empty() ? profile_id_of(req) : existing->get_profile_id();
        if (const auto active = active_id_for(req, profile, txn); active && *active == algorithm) {
            send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, "Algorithm is already active"));
            return;
        }
        if (const auto validation_error = profile_validation_error(req, profile)) {
            send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error));
            return;
        }

        set_active_id(req, profile, txn, algorithm);
        track("routing_link_config");
        send_json(res, send, serde::Ser::encode_json(dictionary_record(*existing)), true);
    }

    // ─── retrieve_routing_algorithm_from_algorithm_id (core:1605/1640) ─────────────
    void handle_routing_retrieve_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "routing_retrieve_config")) {
            return;
        }

        const auto algorithm = algorithm_id_of(req);
        const auto existing = find_config_by_algorithm(algorithm);
        if (!existing) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "routing algorithm not found"));
            return;
        }
        if (const auto validation_error = profile_validation_error(req, existing->get_profile_id())) {
            send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error));
            return;
        }

        // MerchantRoutingAlgorithm (api_models/src/routing.rs:323).
        app::model::routing::MerchantRoutingAlgorithm response;
        response.set_id(wrap<app::model::routing::RoutingIdValue>(existing->get_config_id()));
        response.set_profile_id(wrap<app::model::routing::ProfileIdValue>(existing->get_profile_id()));
        response.set_name(existing->get_name());
        response.set_description(existing->get_description());
        response.set_algorithm(app::model::routing::RoutingAlgorithmWrapper::Static);
        response.set_created_at(now_unix());
        response.set_modified_at(now_unix());
        response.set_algorithm_for(wrap<app::model::routing::TransactionTypeValue>(
            transaction_type_of(existing->get_config_json())));
        track("routing_retrieve_config");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // ─── retrieve_merchant_routing_dictionary (core:590) + merge (:636/:704) ───────
    // Wire: RoutingKind::RoutingAlgorithm -> untagged JSON array (api_models/routing.rs:950).
    void handle_list_routing_configs(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "list_routing_configs")) {
            return;
        }
        list_impl(req, res, send, /*profile_scoped=*/false, "list_routing_configs");
    }

    void handle_list_routing_configs_for_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "list_routing_configs_for_profile")) {
            return;
        }
        list_impl(req, res, send, /*profile_scoped=*/true, "list_routing_configs_for_profile");
    }

    // ─── unlink_routing_config (core:1767 v1) / unlink_..._under_profile (:1714 v2) ─
    void handle_routing_unlink_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "routing_unlink_config")) {
            return;
        }

        const auto body = req.body_as_string();
        using Input = app::model::routing::RoutingConfigRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);

        auto profile = profile_id_of(req);
        if (is_v2_route(req)) {
            profile = first_path_param_after(req, "profiles");
            if (typed && !typed->get_profile_id().get_value().empty()) {
                profile = typed->get_profile_id().get_value();
            }
        } else if (typed && !typed->get_profile_id().get_value().empty()) {
            profile = typed->get_profile_id().get_value();
        }
        if (profile.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "profile_id"));
            return;
        }
        if (const auto validation_error = profile_validation_error(req, profile)) {
            send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error));
            return;
        }

        const auto txn = transaction_type_of(req);
        const auto active = active_id_for(req, profile, txn);
        if (!active) {
            // "Algorithm is already inactive" (core:1941-1943 / :1760-1762).
            send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, "Algorithm is already inactive"));
            return;
        }
        const auto record = find_config_by_algorithm(*active);
        if (!record) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "routing algorithm not found"));
            return;
        }
        set_active_id(req, profile, txn, std::string{});
        auto response = dictionary_record(*record);
        track("routing_unlink_config");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // ─── update_default_routing_config (core:2026 v1) / update_default_fallback (:1954 v2) ─
    void handle_routing_update_default_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "routing_update_default_config")) {
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto typed = serde::Ser::deserialize<Connectors>("application/json", body);
        if (!typed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        const auto profile = is_v2_route(req) ? first_path_param_after(req, "profiles") : profile_id_of(req);
        if (is_v2_route(req)) {
            if (const auto validation_error = profile_validation_error(req, profile)) {
                send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error));
                return;
            }
        }
        if (!validate_default_connectors(req, res, send, profile, *typed)) {
            return;
        }
        store_default_config(req, profile, *typed);

        if (is_v2_route(req)) {
            app::model::routing::ProfileDefaultRoutingConfig response;
            response.set_profile_id(wrap<app::model::routing::ProfileIdValue>(profile));
            response.set_connectors(*typed);
            track("routing_update_default_config");
            send_json(res, send, serde::Ser::encode_json(response), true);
        } else {
            track("routing_update_default_config");
            send_json(res, send, serde::Ser::encode_json(*typed), true);
        }
    }

    // ─── retrieve_default_routing_config (core:2101 v1) / fallback (:2079 v2) ─────
    // Wire: Vec<RoutableConnectorChoice> in both cases.
    void handle_routing_retrieve_default_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "routing_retrieve_default_config")) {
            return;
        }
        const auto profile = is_v2_route(req) ? first_path_param_after(req, "profiles") : profile_id_of(req);
        if (is_v2_route(req)) {
            if (const auto validation_error = profile_validation_error(req, profile)) {
                send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error));
                return;
            }
        }
        const auto connectors = default_config(profile);
        track("routing_retrieve_default_config");
        send_json(res, send, serde::Ser::encode_json(connectors), true);
    }

    // ─── surcharge_decision_config::upsert_surcharge_decision_config ───────────────
    void handle_upsert_surcharge_decision_manager_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "upsert_surcharge_decision_manager_config")) {
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::surcharge_decision_configs::SurchargeDecisionConfigReq;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        if (!typed->get_algorithm()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "Program for config not given"));
            return;
        }
        const auto program = typed->get_algorithm()->get_value();
        if (!valid_program(program)) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "Invalid Request Data"));
            return;
        }

        const auto profile = profile_id_of(req);
        const auto key = decision_key(profile, "surcharge");
        const auto existing = find_config(key);
        app::dto::surcharge_decision_configs::SurchargeDecisionManagerRecord record;
        const auto timestamp = now_unix();
        if (existing) {
            auto previous = serde::Ser::deserialize<app::dto::surcharge_decision_configs::SurchargeDecisionManagerRecord>(
                "application/json", existing->get_config_json());
            record = previous ? std::move(*previous) : record;
            if (typed->get_name()) {
                record.set_name(*typed->get_name());
            }
            record.set_modified_at(timestamp);
        } else {
            if (!typed->get_name()) {
                send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "name of the config not found"));
                return;
            }
            record.set_name(*typed->get_name());
            record.set_created_at(timestamp);
            record.set_modified_at(timestamp);
        }
        record.set_algorithm(typed->get_algorithm().value());
        record.set_merchant_surcharge_configs(typed->get_merchant_surcharge_configs());

        auto stored = config_record(req, key, record.get_name(), "surcharge decision manager",
                                    "surcharge", profile, serde::Ser::encode_json(record));
        upsert_config(std::move(stored));
        set_active_algo_ref(req, profile, "surcharge_config_algo_id", key);
        track("upsert_surcharge_decision_manager_config");
        send_json(res, send, serde::Ser::encode_json(record), true);
    }

    void handle_delete_surcharge_decision_manager_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "delete_surcharge_decision_manager_config")) {
            return;
        }
        const auto profile = profile_id_of(req);
        const auto key = decision_key(profile, "surcharge");
        remove_config(key);
        set_active_algo_ref(req, profile, "surcharge_config_algo_id", std::string{});
        track("delete_surcharge_decision_manager_config");
        send_ok(res, send); // core:198 -> ApplicationResponse::StatusOk
    }

    void handle_retrieve_surcharge_decision_manager_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "retrieve_surcharge_decision_manager_config")) {
            return;
        }
        const auto profile = profile_id_of(req);
        const auto key = decision_key(profile, "surcharge");
        const auto stored = find_config(key);
        if (!stored) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "surcharge conditional config"));
            return;
        }
        auto parsed = serde::Ser::deserialize<app::dto::surcharge_decision_configs::SurchargeDecisionManagerRecord>(
            "application/json", stored->get_config_json());
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INTERNAL, "The Surcharge Decision Config Record was not found"));
            return;
        }
        track("retrieve_surcharge_decision_manager_config");
        send_json(res, send, serde::Ser::encode_json(*parsed), true);
    }

    // ─── conditional_config::upsert_conditional_config (v1 :66 / v2 :18) ───────────
    void handle_upsert_decision_manager_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "upsert_decision_manager_config")) {
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!generic) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        const auto name = generic_string(*generic, "name");
        const auto program = generic_string(*generic, "program", /*fallback_key=*/"algorithm");
        if (program.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "Program for config not given"));
            return;
        }
        if (!valid_program(program)) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "Invalid Request Data"));
            return;
        }

        const auto profile = is_v2_route(req) ? first_path_param_after(req, "profiles") : profile_id_of(req);
        if (is_v2_route(req)) {
            if (const auto validation_error = profile_validation_error(req, profile)) {
                send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error));
                return;
            }
        }
        const auto key = decision_key(profile, "decision");
        const auto existing = find_config(key);
        app::dto::conditional_configs::DecisionManagerRecord record;
        const auto timestamp = now_unix();
        if (existing) {
            auto previous = serde::Ser::deserialize<app::dto::conditional_configs::DecisionManagerRecord>(
                "application/json", existing->get_config_json());
            record = previous ? std::move(*previous) : record;
            record.set_modified_at(timestamp);
        } else {
            if (name.empty()) {
                send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "name of the config not found"));
                return;
            }
            record.set_name(name);
            record.set_created_at(timestamp);
            record.set_modified_at(timestamp);
        }
        if (!name.empty()) {
            record.set_name(name);
        }
        record.set_program(wrap<app::dto::conditional_configs::ProgramValue>(program));

        auto stored = config_record(req, key, record.get_name(), "decision manager",
                                    "decision", profile, serde::Ser::encode_json(record));
        upsert_config(std::move(stored));
        set_active_algo_ref(req, profile, "config_algo_id", key);
        track("upsert_decision_manager_config");
        send_json(res, send, serde::Ser::encode_json(record), true);
    }

    void handle_delete_decision_manager_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "delete_decision_manager_config")) {
            return;
        }
        const auto profile = profile_id_of(req);
        const auto key = decision_key(profile, "decision");
        remove_config(key);
        set_active_algo_ref(req, profile, "config_algo_id", std::string{});
        track("delete_decision_manager_config");
        send_ok(res, send); // core:260 -> ApplicationResponse::StatusOk
    }

    void handle_retrieve_decision_manager_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "retrieve_decision_manager_config")) {
            return;
        }
        const auto profile = is_v2_route(req) ? first_path_param_after(req, "profiles") : profile_id_of(req);
        if (is_v2_route(req)) {
            if (const auto validation_error = profile_validation_error(req, profile)) {
                send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error));
                return;
            }
        }
        const auto key = decision_key(profile, "decision");
        const auto stored = find_config(key);
        if (!stored) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "conditional config"));
            return;
        }
        auto parsed = serde::Ser::deserialize<app::dto::conditional_configs::DecisionManagerRecord>(
            "application/json", stored->get_config_json());
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INTERNAL, "The Conditional Config Record was not found"));
            return;
        }
        track("retrieve_decision_manager_config");
        send_json(res, send, serde::Ser::encode_json(*parsed), true);
    }

    // ─── retrieve_linked_routing_config (core:2168 v1) / under_profile (:2128 v2) ──
    // Wire: LinkedRoutingConfigRetrieveResponse::ProfileBased -> untagged JSON array.
    void handle_routing_retrieve_linked_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "routing_retrieve_linked_config")) {
            return;
        }
        const auto merchant = merchant_id_of(req);
        const auto txn = transaction_type_of(req);
        const auto profile = profile_id_of(req);
        if (!profile.empty()) {
            if (const auto validation_error = profile_validation_error(req, profile)) {
                send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error));
                return;
            }
        }

        std::vector<app::model::routing::RoutingDictionaryRecord> active;
        const auto active_id = active_id_for(req, profile, txn);
        for (auto& record : m_context->repository().list_routing_configs()) {
            if (!merchant.empty() && record.get_merchant_id() != merchant) {
                continue;
            }
            if (!profile.empty() && record.get_profile_id() != profile) {
                continue;
            }
            if (!is_v2_route(req)) {
                if (active_id && record.get_config_id() == *active_id) {
                    active.push_back(dictionary_record(record));
                }
            } else if (transaction_type_of(record.get_config_json()) == txn) {
                // core:2155-2159 filters v2 metadata by transaction type.
                active.push_back(dictionary_record(record));
            }
        }
        track("routing_retrieve_linked_config");
        send_json(res, send, serde::Ser::encode_json(active), true);
    }

    // ─── retrieve_default_routing_config_for_profiles (core:2408) ──────────────────
    // Wire: Vec<ProfileDefaultRoutingConfig>.
    void handle_routing_retrieve_default_config_for_profiles(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "routing_retrieve_default_config_for_profiles")) {
            return;
        }
        std::vector<app::model::routing::ProfileDefaultRoutingConfig> response;
        const auto merchant = merchant_id_of(req);
        std::vector<std::string> profiles;
        for (const auto& record : m_context->repository().list_routing_configs()) {
            if (!merchant.empty() && record.get_merchant_id() != merchant) {
                continue;
            }
            if (record.get_config_id().ends_with(":default") && !record.get_profile_id().empty()) {
                profiles.push_back(record.get_profile_id());
            }
        }
        std::ranges::sort(profiles);
        profiles.erase(std::unique(profiles.begin(), profiles.end()), profiles.end());
        for (const auto& profile : profiles) {
            app::model::routing::ProfileDefaultRoutingConfig item;
            item.set_profile_id(wrap<app::model::routing::ProfileIdValue>(profile));
            item.set_connectors(default_config(profile));
            response.push_back(std::move(item));
        }
        track("routing_retrieve_default_config_for_profiles");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // ─── update_default_routing_config_for_profile (core:2454) ─────────────────────
    void handle_routing_update_default_config_for_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "routing_update_default_config_for_profile")) {
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto typed = serde::Ser::deserialize<Connectors>("application/json", body);
        if (!typed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        auto profile = profile_id_of(req);
        if (profile.empty()) {
            profile = first_path_param_after(req, "profiles");
        }
        if (profile.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "profile_id"));
            return;
        }
        if (const auto validation_error = profile_validation_error(req, profile)) {
            send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error));
            return;
        }
        if (!validate_default_connectors(req, res, send, profile, *typed)) {
            return;
        }
        store_default_config(req, profile, *typed);

        app::model::routing::ProfileDefaultRoutingConfig response;
        response.set_profile_id(wrap<app::model::routing::ProfileIdValue>(profile));
        response.set_connectors(*typed);
        track("routing_update_default_config_for_profile");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // ─── create_specific_dynamic_routing (core:2556), SuccessRateBasedRouting ──────
    void handle_create_success_based_routing(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        dynamic_create(req, res, send, "create_success_based_routing",
                       "Success rate based dynamic routing algorithm",
                       "success_rate_based", "Success rate based routing");
    }

    void handle_create_elimination_routing(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        dynamic_create(req, res, send, "create_elimination_routing",
                       "Elimination based dynamic routing algorithm",
                       "elimination", "Elimination routing");
    }

    // ─── contract_based_dynamic_routing_setup (core:2717) ───────────────────────────
    void handle_contract_based_routing_setup_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "contract_based_routing_setup_config")) {
            return;
        }
        const auto profile = profile_id_of(req);
        const auto feature = app::http::RequestQuery{}(req).get("enable").value_or("metrics");
        const auto body = req.body_as_string();
        if (feature == "none") {
            // Disable path (core:2772-2785): drop the contract algorithm ref.
            put_dynamic_ref(profile, R"({"contract_based_routing":null})");
            track("contract_based_routing_setup_config");
            send_ok(res, send);
            return;
        }
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "Failed to get ContractBasedRoutingConfig from request"));
            return;
        }
        auto config = serde::Ser::decode_generic("application/json", body);
        if (!config) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        if (dynamic_ref_has_feature(profile, "contract_based_routing", feature)) {
            send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED,
                                        "Contract Routing with specified features is already enabled"));
            return;
        }
        if (const auto validation_error = profile_validation_error(req, profile)) {
            send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error));
            return;
        }
        const auto algorithm = new_routing_id();
        auto record = config_record(req, algorithm, "Contract based dynamic routing algorithm",
                                    "", "contract_based", profile, body);
        record.set_algorithm_id(algorithm);
        upsert_config(std::move(record));
        put_dynamic_ref(profile, std::format(
            R"({{"contract_based_routing":{{"algorithm_id_with_timestamp":{{"algorithm_id":"{}","timestamp":{}}},"enabled_feature":"{}"}}}})",
            algorithm, now_unix(), feature));
        track("contract_based_routing_setup_config");
        send_json(res, send, serde::Ser::encode_json(
            dictionary_record(algorithm, profile, "Contract based dynamic routing algorithm", "",
                              "contract_based", "payment")), true);
    }

    // ─── configure_dynamic_routing_volume_split (core:2624) ─────────────────────────
    void handle_set_dynamic_routing_volume_split(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "set_dynamic_routing_volume_split")) {
            return;
        }
        const auto query = app::http::RequestQuery{}(req);
        const auto split = query_number<std::uint8_t>(query, "split");
        if (!split) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "split"));
            return;
        }
        // crate::consts::DYNAMIC_ROUTING_MAX_VOLUME = 100 (consts.rs:242).
        if (*split > 100) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                        "Dynamic routing volume split should be less than 100"));
            return;
        }
        const auto profile = profile_id_of(req);
        if (const auto validation_error = profile_validation_error(req, profile)) {
            send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error));
            return;
        }
        update_dynamic_volume_split(profile, *split);
        app::model::routing::RoutingVolumeSplit response;
        response.set_routing_type(app::model::routing::RoutingType::Dynamic);
        response.set_split(*split);
        track("set_dynamic_routing_volume_split");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // ─── retrieve_dynamic_routing_volume_split (core:2680) ──────────────────────────
    void handle_get_dynamic_routing_volume_split(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "get_dynamic_routing_volume_split")) {
            return;
        }
        const auto profile = profile_id_of(req);
        if (const auto validation_error = profile_validation_error(req, profile)) {
            send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error));
            return;
        }
        app::model::routing::RoutingVolumeSplitResponse response;
        response.set_split(dynamic_volume_split(profile));
        track("get_dynamic_routing_volume_split");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // ─── routing_migration_status (core:3457) ───────────────────────────────────────
    void handle_routing_migration_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "routing_migration_status")) {
            return;
        }
        const auto query = app::http::RequestQuery{}(req);
        const auto limit = query_number<std::uint32_t>(query, "limit").value_or(20);
        const auto offset = query_number<std::uint32_t>(query, "offset").value_or(0);
        if (const auto body = req.body_as_string(); !body.empty()) {
            auto typed = serde::Ser::deserialize<app::model::routing::RoutingMigrationStatusQuery>(
                "application/json", body);
            if (!typed) {
                send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
        }

        // list_routing_scope_page substitute: group persisted records by (profile, merchant).
        std::vector<std::pair<std::string, std::string>> scopes;
        std::map<std::string, std::size_t> in_scope_counts;
        for (const auto& record : m_context->repository().list_routing_configs()) {
            if (record.get_profile_id().empty()) {
                continue;
            }
            bool seen = false;
            for (const auto& scope : scopes) {
                if (scope.first == record.get_profile_id()) {
                    seen = true;
                    break;
                }
            }
            if (!seen) {
                scopes.emplace_back(record.get_profile_id(), record.get_merchant_id());
            }
            if (record.get_algorithm() != "default" && record.get_algorithm() != "decision" &&
                record.get_algorithm() != "surcharge" && record.get_algorithm() != "volume_split") {
                ++in_scope_counts[record.get_profile_id()];
            }
        }
        std::ranges::sort(scopes, [](const auto& a, const auto& b) { return a.first < b.first; });
        const auto begin = scopes.begin() + static_cast<std::ptrdiff_t>(std::min<std::size_t>(offset, scopes.size()));
        const auto end = begin + static_cast<std::ptrdiff_t>(std::min<std::size_t>(limit, static_cast<std::size_t>(scopes.end() - begin)));

        app::model::routing::RoutingMigrationStatusResponse response;
        response.set_limit(limit);
        response.set_offset(offset);
        response.set_has_more(static_cast<std::size_t>(end - begin) >= limit);
        app::model::routing::RoutingMigrationPageTotals totals;
        totals.set_profiles(static_cast<std::size_t>(end - begin));

        std::vector<app::model::routing::RoutingMigrationProfileStatus> profiles;
        for (auto it = begin; it != end; ++it) {
            const auto& profile = it->first;
            const auto hs_count = static_cast<std::int64_t>(in_scope_counts[profile]);
            const auto de = de_rule_ids(profile);
            app::model::routing::RoutingMigrationProfileStatus status;
            status.set_profile_id(wrap<app::model::routing::ProfileIdValue>(profile));
            status.set_merchant_id(wrap<app::model::routing::MerchantIdValue>(it->second));
            status.set_rules_hyperswitch(hs_count);
            status.set_rules_decision_engine(static_cast<std::int64_t>(de.size()));
            const auto source = is_cutover(req, profile)
                ? app::model::routing::RoutingResultSource::DecisionEngine
                : app::model::routing::RoutingResultSource::HyperswitchRouting;
            status.set_routing_source(source);
            app::model::routing::RoutingMigrationState state{};
            if (de.empty()) {
                state = (source == app::model::routing::RoutingResultSource::DecisionEngine)
                    ? app::model::routing::RoutingMigrationState::EnabledWithoutRules
                    : app::model::routing::RoutingMigrationState::Pending;
            } else if (hs_count > static_cast<std::int64_t>(de.size())) {
                state = app::model::routing::RoutingMigrationState::Partial;
            } else if (source == app::model::routing::RoutingResultSource::DecisionEngine) {
                state = app::model::routing::RoutingMigrationState::Enabled;
            } else {
                state = app::model::routing::RoutingMigrationState::Migrated;
            }
            status.set_state(state);
            profiles.push_back(std::move(status));
            switch (state) {
            case app::model::routing::RoutingMigrationState::Pending: totals.set_pending(totals.get_pending() + 1); break;
            case app::model::routing::RoutingMigrationState::Partial: totals.set_partial(totals.get_partial() + 1); break;
            case app::model::routing::RoutingMigrationState::Migrated: totals.set_migrated(totals.get_migrated() + 1); break;
            case app::model::routing::RoutingMigrationState::Enabled: totals.set_enabled(totals.get_enabled() + 1); break;
            case app::model::routing::RoutingMigrationState::EnabledWithoutRules: totals.set_enabled_without_rules(totals.get_enabled_without_rules() + 1); break;
            case app::model::routing::RoutingMigrationState::Unknown: totals.set_unknown(totals.get_unknown() + 1); break;
            case app::model::routing::RoutingMigrationState::Diverged: totals.set_diverged(totals.get_diverged() + 1); break;
            }
        }
        response.set_profiles(std::move(profiles));
        response.set_page_totals(totals);
        track("routing_migration_status");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // ─── migrate_rules_for_profiles (core:3072) + migrate_rules_for_profile (:3193) ─
    void handle_migrate_routing_rules(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "migrate_routing_rules")) {
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto typed = serde::Ser::deserialize<app::model::routing::RuleMigrationRequest>(
            "application/json", body);
        if (!typed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        app::model::routing::RuleMigrationResult response;
        app::model::routing::RuleMigrationTotals totals;
        totals.set_profiles(typed->get_profile_ids().size());

        std::vector<app::model::routing::RuleMigrationProfileResult> profiles;
        for (const auto& profile_value : typed->get_profile_ids()) {
            const auto profile = profile_value.get_value();
            app::model::routing::RuleMigrationProfileResult result;
            result.set_profile_id(profile_value);
            std::vector<app::model::routing::RuleMigrationResponse> success;
            std::vector<app::model::routing::RuleMigrationNotApplicable> not_applicable;

            bool found = false;
            for (const auto& record : m_context->repository().list_routing_configs()) {
                if (record.get_profile_id() != profile || record.get_config_id().ends_with(":default")) {
                    continue;
                }
                found = true;
                const auto algorithm_id = record.get_config_id();
                if (record.get_algorithm() == "three_ds_decision_rule") {
                    app::model::routing::RuleMigrationNotApplicable na;
                    na.set_profile_id(profile_value);
                    na.set_algorithm_id(wrap<app::model::routing::RoutingIdValue>(algorithm_id));
                    na.set_kind(app::model::routing::RoutingAlgorithmKind::Single);
                    na.set_reason("3DS decision rules are not migrated to the Decision Engine");
                    not_applicable.push_back(std::move(na));
                    continue;
                }
                // Local in-process substitute for DE rule create (external infra).
                add_de_rule_id(profile, algorithm_id);
                app::model::routing::RuleMigrationResponse migrated;
                migrated.set_profile_id(profile_value);
                migrated.set_euclid_algorithm_id(wrap<app::model::routing::RoutingIdValue>(algorithm_id));
                migrated.set_decision_engine_algorithm_id(algorithm_id);
                migrated.set_is_active_rule(active_id_for(req, profile, "payment") == algorithm_id);
                success.push_back(std::move(migrated));
            }
            result.set_success(std::move(success));
            result.set_not_applicable(std::move(not_applicable));
            if (!found) {
                result.set_not_attempted("no routing rules in Hyperswitch");
                totals.set_profiles_not_attempted(totals.get_profiles_not_attempted() + 1);
            }
            totals.set_rules_migrated(totals.get_rules_migrated() + result.get_success().size());
            totals.set_rules_not_applicable(
                totals.get_rules_not_applicable() + result.get_not_applicable().size());
            profiles.push_back(std::move(result));
        }
        response.set_profiles(std::move(profiles));
        response.set_totals(totals);
        track("migrate_routing_rules");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // ─── Route installation (app.rs:1251-1477, :1254-1269, :2660-2723, :2724-2766) ─
    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/routing"}
            .add_route(Route{"/entry"}.post([this](auto& req, auto& res, auto send) { handle_routing_entry(req, res, std::move(send)); }).summary("Hyperswitch routing_entry"))
            .add_route(Route{"/active"}.get([this](auto& req, auto& res, auto send) { handle_routing_retrieve_linked_config(req, res, std::move(send)); }).summary("Hyperswitch routing_retrieve_linked_config"))
            .add_route(Route{"/"}.get([this](auto& req, auto& res, auto send) { handle_list_routing_configs(req, res, std::move(send)); }).summary("Hyperswitch list_routing_configs"))
            .add_route(Route{"/"}.post([this](auto& req, auto& res, auto send) { handle_routing_create_config(req, res, std::move(send)); }).summary("Hyperswitch routing_create_config"))
            .add_route(Route{"/list/profile"}.get([this](auto& req, auto& res, auto send) { handle_list_routing_configs_for_profile(req, res, std::move(send)); }).summary("Hyperswitch list_routing_configs_for_profile"))
            .add_route(Route{"/default"}.post([this](auto& req, auto& res, auto send) { handle_routing_update_default_config(req, res, std::move(send)); }).summary("Hyperswitch routing_update_default_config"))
            .add_route(Route{"/rule/migrate"}.post([this](auto& req, auto& res, auto send) { handle_migrate_routing_rules(req, res, std::move(send)); }).summary("Hyperswitch migrate_routing_rules"))
            .add_route(Route{"/migration/status"}.get([this](auto& req, auto& res, auto send) { handle_routing_migration_status(req, res, std::move(send)); }).summary("Hyperswitch routing_migration_status"))
            .add_route(Route{"/deactivate"}.post([this](auto& req, auto& res, auto send) { handle_routing_unlink_config(req, res, std::move(send)); }).summary("Hyperswitch routing_unlink_config"))
            .add_route(Route{"/decision"}.put([this](auto& req, auto& res, auto send) { handle_upsert_decision_manager_config(req, res, std::move(send)); }).summary("Hyperswitch upsert_decision_manager_config"))
            .add_route(Route{"/decision"}.get([this](auto& req, auto& res, auto send) { handle_retrieve_decision_manager_config(req, res, std::move(send)); }).summary("Hyperswitch retrieve_decision_manager_config"))
            .add_route(Route{"/decision"}.delt([this](auto& req, auto& res, auto send) { handle_delete_decision_manager_config(req, res, std::move(send)); }).summary("Hyperswitch delete_decision_manager_config"))
            .add_route(Route{"/decision/surcharge"}.put([this](auto& req, auto& res, auto send) { handle_upsert_surcharge_decision_manager_config(req, res, std::move(send)); }).summary("Hyperswitch upsert_surcharge_decision_manager_config"))
            .add_route(Route{"/decision/surcharge"}.get([this](auto& req, auto& res, auto send) { handle_retrieve_surcharge_decision_manager_config(req, res, std::move(send)); }).summary("Hyperswitch retrieve_surcharge_decision_manager_config"))
            .add_route(Route{"/decision/surcharge"}.delt([this](auto& req, auto& res, auto send) { handle_delete_surcharge_decision_manager_config(req, res, std::move(send)); }).summary("Hyperswitch delete_surcharge_decision_manager_config"))
            .add_route(Route{"/default/profile/:profile_id"}.post([this](auto& req, auto& res, auto send) { handle_routing_update_default_config_for_profile(req, res, std::move(send)); }).summary("Hyperswitch routing_update_default_config_for_profile"))
            .add_route(Route{"/default/profile"}.get([this](auto& req, auto& res, auto send) { handle_routing_retrieve_default_config(req, res, std::move(send)); }).summary("Hyperswitch routing_retrieve_default_config"))
            .add_route(Route{"/:algorithm_id"}.get([this](auto& req, auto& res, auto send) { handle_routing_retrieve_config(req, res, std::move(send)); }).summary("Hyperswitch routing_retrieve_config"))
            .add_route(Route{"/:algorithm_id/activate"}.post([this](auto& req, auto& res, auto send) { handle_routing_link_config(req, res, std::move(send)); }).summary("Hyperswitch routing_link_config"))

            .add_route(Route{"/payouts"}.get([this](auto& req, auto& res, auto send) { handle_list_routing_configs(req, res, std::move(send)); }).summary("Hyperswitch list_routing_configs"))
            .add_route(Route{"/payouts"}.post([this](auto& req, auto& res, auto send) { handle_routing_create_config(req, res, std::move(send)); }).summary("Hyperswitch routing_create_config"))
            .add_route(Route{"/payouts/list/profile"}.get([this](auto& req, auto& res, auto send) { handle_list_routing_configs_for_profile(req, res, std::move(send)); }).summary("Hyperswitch list_routing_configs_for_profile"))
            .add_route(Route{"/payouts/active"}.get([this](auto& req, auto& res, auto send) { handle_routing_retrieve_linked_config(req, res, std::move(send)); }).summary("Hyperswitch routing_retrieve_linked_config"))
            .add_route(Route{"/payouts/default"}.get([this](auto& req, auto& res, auto send) { handle_routing_retrieve_default_config(req, res, std::move(send)); }).summary("Hyperswitch routing_retrieve_default_config"))
            .add_route(Route{"/payouts/default"}.post([this](auto& req, auto& res, auto send) { handle_routing_update_default_config(req, res, std::move(send)); }).summary("Hyperswitch routing_update_default_config"))
            .add_route(Route{"/payouts/:algorithm_id/activate"}.post([this](auto& req, auto& res, auto send) { handle_routing_link_config(req, res, std::move(send)); }).summary("Hyperswitch routing_link_config"))
            .add_route(Route{"/payouts/deactivate"}.post([this](auto& req, auto& res, auto send) { handle_routing_unlink_config(req, res, std::move(send)); }).summary("Hyperswitch routing_unlink_config"))
            .add_route(Route{"/payouts/default/profile/:profile_id"}.post([this](auto& req, auto& res, auto send) { handle_routing_update_default_config_for_profile(req, res, std::move(send)); }).summary("Hyperswitch routing_update_default_config_for_profile"))
            .add_route(Route{"/payouts/default/profile"}.get([this](auto& req, auto& res, auto send) { handle_routing_retrieve_default_config_for_profiles(req, res, std::move(send)); }).summary("Hyperswitch routing_retrieve_default_config_for_profiles"))

            .add_route(Route{"/v2/routing-algorithms"}.post([this](auto& req, auto& res, auto send) { handle_routing_create_config(req, res, std::move(send)); }).summary("Hyperswitch routing_create_config"))
            .add_route(Route{"/v2/routing-algorithms/:algorithm_id"}.get([this](auto& req, auto& res, auto send) { handle_routing_retrieve_config(req, res, std::move(send)); }).summary("Hyperswitch routing_retrieve_config"))
            .add_route(Route{"/v2/profiles/:profile_id/fallback-routing"}.get([this](auto& req, auto& res, auto send) { handle_routing_retrieve_default_config(req, res, std::move(send)); }).summary("Hyperswitch routing_retrieve_default_config"))
            .add_route(Route{"/v2/profiles/:profile_id/fallback-routing"}.patch([this](auto& req, auto& res, auto send) { handle_routing_update_default_config(req, res, std::move(send)); }).summary("Hyperswitch routing_update_default_config"))
            .add_route(Route{"/v2/profiles/:profile_id/activate-routing-algorithm"}.patch([this](auto& req, auto& res, auto send) { handle_routing_link_config(req, res, std::move(send)); }).summary("Hyperswitch routing_link_config"))
            .add_route(Route{"/v2/profiles/:profile_id/deactivate-routing-algorithm"}.patch([this](auto& req, auto& res, auto send) { handle_routing_unlink_config(req, res, std::move(send)); }).summary("Hyperswitch routing_unlink_config"))
            .add_route(Route{"/v2/profiles/:profile_id/routing-algorithm"}.get([this](auto& req, auto& res, auto send) { handle_routing_retrieve_linked_config(req, res, std::move(send)); }).summary("Hyperswitch routing_retrieve_linked_config"))
            .add_route(Route{"/v2/profiles/:profile_id/decision"}.put([this](auto& req, auto& res, auto send) { handle_upsert_decision_manager_config(req, res, std::move(send)); }).summary("Hyperswitch upsert_decision_manager_config"))
            .add_route(Route{"/v2/profiles/:profile_id/decision"}.get([this](auto& req, auto& res, auto send) { handle_retrieve_decision_manager_config(req, res, std::move(send)); }).summary("Hyperswitch retrieve_decision_manager_config"))

            .add_route(Route{"/account/:account_id/business_profile/:profile_id/dynamic_routing/success_based/create"}.post([this](auto& req, auto& res, auto send) { handle_create_success_based_routing(req, res, std::move(send)); }).summary("Hyperswitch create_success_based_routing"))
            .add_route(Route{"/account/:account_id/business_profile/:profile_id/dynamic_routing/set_volume_split"}.post([this](auto& req, auto& res, auto send) { handle_set_dynamic_routing_volume_split(req, res, std::move(send)); }).summary("Hyperswitch set_dynamic_routing_volume_split"))
            .add_route(Route{"/account/:account_id/business_profile/:profile_id/dynamic_routing/get_volume_split"}.get([this](auto& req, auto& res, auto send) { handle_get_dynamic_routing_volume_split(req, res, std::move(send)); }).summary("Hyperswitch get_dynamic_routing_volume_split"))
            .add_route(Route{"/account/:account_id/business_profile/:profile_id/dynamic_routing/elimination/create"}.post([this](auto& req, auto& res, auto send) { handle_create_elimination_routing(req, res, std::move(send)); }).summary("Hyperswitch create_elimination_routing"))
            .add_route(Route{"/account/:account_id/business_profile/:profile_id/dynamic_routing/contracts/toggle"}.post([this](auto& req, auto& res, auto send) { handle_contract_based_routing_setup_config(req, res, std::move(send)); }).summary("Hyperswitch contract_based_routing_setup_config"));
    }

private:
    using RoutingRecord = std::remove_cvref_t<
        decltype(*std::declval<app::context::AppContext&>()
                     .repository()
                     .find_routing_config_by_string(std::string_view{}))>;
    using DictionaryRecord = app::model::routing::RoutingDictionaryRecord;
    using Connectors = std::vector<app::model::routing::RoutableConnectorChoice>;

    // ── Identity / request helpers ────────────────────────────────────────────────

    template<typename T>
    [[nodiscard]] T wrap(std::string value) const
    {
        T result{};
        result.set_value(std::move(value));
        return result;
    }

    template<typename T>
    [[nodiscard]] std::optional<T> query_number(const app::http::QueryParams& query, std::string_view key) const
    {
        const auto raw = query.get(key);
        if (!raw) {
            return std::nullopt;
        }
        T value{};
        const auto [end, ec] = std::from_chars(raw->data(), raw->data() + raw->size(), value);
        (void)end;
        return ec == std::errc{} ? std::optional<T>{value} : std::nullopt;
    }

    [[nodiscard]] std::string merchant_id_of(interfaces::io::IRequest& req) const
    {
        return app::http::merchant_id_from_header{}(req);
    }

    [[nodiscard]] std::string org_id_of(interfaces::io::IRequest& req) const
    {
        return app::http::org_id_from_header{}(req);
    }

    [[nodiscard]] std::string profile_id_of(interfaces::io::IRequest& req) const
    {
        return first_path_param_after(req, "profiles", "business_profile");
    }

    [[nodiscard]] std::string first_path_param_after(
        interfaces::io::IRequest& req,
        std::string_view key,
        std::string_view alt = {}) const
    {
        const auto segments = app::http::RequestPath{}(req);
        for (std::size_t i = 0; i + 1 < segments.count(); ++i) {
            const auto current = segments.at(i).value();
            if (current == key || (!alt.empty() && current == alt)) {
                return std::string{segments.at(i + 1).value()};
            }
        }
        if (const auto from_query = app::http::RequestQuery{}(req).get("profile_id")) {
            return *from_query;
        }
        return app::auth::AuthContextBuilder{}(req).profile_id();
    }

    [[nodiscard]] std::string algorithm_id_of(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        for (std::size_t i = 0; i < segments.count(); ++i) {
            const auto current = segments.at(i).value();
            if (current == "routing-algorithms" && i + 1 < segments.count()) {
                return std::string{segments.at(i + 1).value()};
            }
            if (current == "activate" && i > 0) {
                const auto previous = segments.at(i - 1).value();
                if (previous != "routing-algorithms") {
                    return std::string{previous};
                }
            }
        }
        if (const auto from_query = app::http::RequestQuery{}(req).get("algorithm_id")) {
            return *from_query;
        }
        return {};
    }

    [[nodiscard]] bool is_v2_route(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        for (std::size_t i = 0; i < segments.count(); ++i) {
            if (segments.at(i).value() == "v2") {
                return true;
            }
        }
        return false;
    }

    /** app.rs passes a TransactionType to every v1 route; payout paths force Payout. */
    [[nodiscard]] std::string transaction_type_of(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        for (std::size_t i = 0; i < segments.count(); ++i) {
            if (segments.at(i).value() == "payouts") {
                return "payout";
            }
        }
        if (const auto from_query = app::http::RequestQuery{}(req).get("transaction_type")) {
            return *from_query;
        }
        return "payment";
    }

    [[nodiscard]] static std::string transaction_type_of(std::string_view config_json)
    {
        return config_json.find("\"algorithm_for\":\"payout\"") != std::string::npos ? "payout" : "payment";
    }

    // ── Auth / validation (routes/routing.rs auth_type extractors; core/utils.rs:2889) ──

    /**
     * Local mirror of one upstream route's auth extractor combination:
     *   HeaderAuth(ApiKeyAuth{allow_connected_scope_operation, allow_platform_self_operation})
     *     + JWTAuth{permission, allow_connected, allow_platform}
     *   V2ApiKeyAuth + JWTAuthProfileFromRoute{profile_id, permission, ...}
     *   JWTAuth (dashboard-only) / AdminApiAuth (migration endpoints).
     * No shared auth registry edits are needed: the extractor is resolved per operation here.
     */
    struct AuthSpec
    {
        app::auth::AppPermission permission{app::auth::AppPermission::PROFILE_READ};
        std::string_view required_permission{"routing:read"}; // upstream Permission wire name
        bool allow_connected{true};  // ApiKeyAuth.allow_connected_scope_operation / JWTAuth.allow_connected
        bool allow_platform{false};  // ApiKeyAuth.allow_platform_self_operation / JWTAuth.allow_platform
        bool profile_from_route{false}; // JWTAuthProfileFromRoute
        bool jwt_only{false};        // dashboard-only (no API-key caller)
        bool admin_api{false};       // AdminApiAuth
        bool api_key_v2{false};      // V2ApiKeyAuth instead of HeaderAuth(ApiKeyAuth)
    };

    /** Exact per-operation extractor combination (routes/routing.rs:30-51,63-93,113-151,...). */
    [[nodiscard]] static AuthSpec spec_for(std::string_view operation)
    {
        using P = app::auth::AppPermission;
        AuthSpec spec{};

        // AdminApiAuth on both migration endpoints (routing.rs:1592,1615).
        if (operation == "migrate_routing_rules" || operation == "routing_migration_status") {
            const bool write = operation == "migrate_routing_rules";
            spec.permission = write ? P::ROUTING_WRITE : P::ROUTING_READ;
            spec.required_permission = write ? "routing:write" : "routing:read";
            spec.admin_api = true;
            spec.allow_platform = true; // AdminApiAuth is platform-scoped
            return spec;
        }

        // Dashboard routing entry: JWTAuth ProfileRoutingRead, dashboard-only (routing.rs:45-49).
        if (operation == "routing_entry") {
            spec.permission = P::PROFILE_READ;
            spec.required_permission = "routing:read";
            spec.jwt_only = true;
            return spec;
        }

        // 3DS decision-manager family (routing.rs:861-874,961-977,1065-1078; 921-935,1021-1035).
        if (operation == "upsert_decision_manager_config" || operation == "delete_decision_manager_config") {
            spec.permission = P::MERCHANT_WRITE;
            spec.required_permission = "three_ds_decision_manager:write";
            return spec;
        }
        if (operation == "retrieve_decision_manager_config") {
            spec.permission = P::MERCHANT_READ;
            spec.required_permission = "three_ds_decision_manager:read";
            return spec;
        }

        // Surcharge decision-manager family (routing.rs:733-746,773-786,816-829).
        if (operation == "upsert_surcharge_decision_manager_config" ||
            operation == "delete_surcharge_decision_manager_config") {
            spec.permission = P::MERCHANT_WRITE;
            spec.required_permission = "surcharge_decision_manager:write";
            return spec;
        }
        if (operation == "retrieve_surcharge_decision_manager_config") {
            spec.permission = P::MERCHANT_READ;
            spec.required_permission = "surcharge_decision_manager:read";
            return spec;
        }

        // Dynamic routing: ProfileRouting* + JWTAuthProfileFromRoute (routing.rs:1357-1369,
        // 1408-1420,1463-1477,1515-1528,1558-1570).
        if (operation == "create_success_based_routing" || operation == "create_elimination_routing" ||
            operation == "contract_based_routing_setup_config" ||
            operation == "set_dynamic_routing_volume_split") {
            spec.permission = P::PROFILE_WRITE;
            spec.required_permission = "routing:write";
            spec.profile_from_route = true;
            return spec;
        }
        if (operation == "get_dynamic_routing_volume_split") {
            spec.permission = P::PROFILE_READ;
            spec.required_permission = "routing:read";
            spec.profile_from_route = true;
            return spec;
        }

        // Core routing family — v1 permissions; v2 overrides applied in authorize_operation().
        const bool write = operation == "routing_create_config" || operation == "routing_link_config" ||
                           operation == "routing_unlink_config" ||
                           operation == "routing_update_default_config" ||
                           operation == "routing_update_default_config_for_profile";
        const bool merchant_read = operation == "list_routing_configs" ||
                                   operation == "routing_retrieve_default_config_for_profiles";
        spec.permission = merchant_read ? P::MERCHANT_READ
                          : write ? P::PROFILE_WRITE
                                  : P::PROFILE_READ;
        spec.required_permission = write ? "routing:write" : "routing:read";
        // JWTAuthProfileFromRoute variants: v1 linked-config (when profile_id in query,
        // routing.rs:1113-1124) and v1 default-for-profile (routing.rs:1302-1312).
        spec.profile_from_route = operation == "routing_retrieve_linked_config" ||
                                  operation == "routing_update_default_config_for_profile";
        return spec;
    }

    /** Matches the authorizer's permission-claim check but with upstream grant names. */
    [[nodiscard]] static bool granular_permission_allowed(
        interfaces::io::IRequest& req,
        std::string_view required
    )
    {
        const auto single = req.find_header("permission");
        const auto list = req.find_header("permissions");
        if (single.empty() && list.empty()) {
            return true; // no explicit grant list: bearer/scheme check governs
        }
        const auto grants = [required](std::string_view granted)
        {
            std::string_view rest = granted;
            while (!rest.empty()) {
                const auto comma = rest.find(',');
                const auto token = rest.substr(0, comma);
                if (token == required || token == "ADMIN" || token == "all") {
                    return true;
                }
                if (comma == std::string_view::npos) {
                    break;
                }
                rest = rest.substr(comma + 1);
            }
            return false;
        };
        return (!single.empty() && grants(single)) || (!list.empty() && grants(list));
    }

    [[nodiscard]] bool authorize_operation(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view operation
    ) const
    {
        auto spec = spec_for(operation);

        // v2 routes: V2ApiKeyAuth + JWTAuthProfileFromRoute, with v2 permission names
        // (routing.rs:133-147,237-251,561-575,650-664,894-935,1015-1035,1189-1220).
        if (is_v2_route(req)) {
            spec.api_key_v2 = true;
            spec.profile_from_route = true;
            if (operation == "routing_create_config" || operation == "routing_link_config" ||
                operation == "routing_unlink_config" || operation == "routing_update_default_config") {
                spec.permission = app::auth::AppPermission::MERCHANT_WRITE;
                spec.required_permission = "routing:write";
            } else if (operation == "routing_retrieve_default_config") {
                spec.permission = app::auth::AppPermission::MERCHANT_READ;
                spec.required_permission = "routing:read";
            } else if (operation == "upsert_decision_manager_config" ||
                       operation == "retrieve_decision_manager_config") {
                // v2 retrieve also requires ProfileThreeDsDecisionManagerWrite (routing.rs:1023).
                spec.permission = app::auth::AppPermission::PROFILE_WRITE;
                spec.required_permission = "three_ds_decision_manager:write";
            } else {
                spec.permission = app::auth::AppPermission::PROFILE_READ;
                spec.required_permission = "routing:read";
            }
        }

        const auto context = app::auth::AuthContextBuilder{}(req);

        // Upstream auth_type picks API-key when the header is present, otherwise JWT.
        app::auth::AuthScheme scheme = app::auth::AuthScheme::JWT;
        if (spec.admin_api) {
            scheme = app::auth::AuthScheme::PLATFORM_ADMIN;
        } else if (!spec.jwt_only &&
                   (context.claim(app::auth::kClaimApiKey) ||
                    context.claim(app::auth::kClaimInternalApiKey))) {
            scheme = spec.api_key_v2 ? app::auth::AuthScheme::V2_API_KEY
                                     : app::auth::AuthScheme::API_KEY;
        }
        const app::auth::RouteAuthRule rule{
            scheme, spec.permission, spec.allow_connected, spec.allow_platform
        };
        if (const auto denied = m_context->authorizer().authorize_route(context, rule)) {
            send_error(res, send, *denied);
            return false;
        }

        // allow_connected_scope_operation / allow_connected: a connected-scope credential is
        // rejected on operations whose extractor did not opt in.
        if (!spec.allow_connected && context.claim(app::auth::kClaimConnectedMerchantId)) {
            send_error(res, send, error(core::error::AppErrorKind::FORBIDDEN,
                                        "connected scope not allowed for this operation"));
            return false;
        }
        // allow_platform_self_operation / allow_platform: false (all dashboard/merchant routes).
        if (!spec.allow_platform && context.claim(app::auth::kClaimInternalApiKey)) {
            send_error(res, send, error(core::error::AppErrorKind::FORBIDDEN,
                                        "platform scope not allowed for this operation"));
            return false;
        }
        // Granular permission from the JWT role (upstream Permission::* wire names).
        if (!granular_permission_allowed(req, spec.required_permission)) {
            send_error(res, send, error(core::error::AppErrorKind::FORBIDDEN,
                                        std::format("permission {} not granted", spec.required_permission)));
            return false;
        }
        // JWTAuthProfileFromRoute: route profile must match the authenticated profile.
        if (spec.profile_from_route) {
            const auto route_profile = profile_id_of(req);
            if (const auto validation_error = profile_validation_error(req, route_profile)) {
                send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error));
                return false;
            }
        }
        return true;
    }

    /** core/utils.rs:2889-2912 validate_profile_id_from_auth_layer — pure, no shared state. */
    [[nodiscard]] std::optional<std::string> profile_validation_error(
        interfaces::io::IRequest& req,
        std::string_view profile
    ) const
    {
        const auto auth_profile = app::auth::AuthContextBuilder{}(req).profile_id();
        if (!auth_profile.empty() && !profile.empty() && auth_profile != profile) {
            return std::string{"Profile id authentication failed. Please use the correct JWT token"};
        }
        return std::nullopt;
    }

    // ── Time / id / config helpers ────────────────────────────────────────────────

    [[nodiscard]] static std::int64_t now_unix()
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }

    [[nodiscard]] std::string new_routing_id() const
    {
        return m_context->ids().payment_id("routing_");
    }

    [[nodiscard]] static std::string algorithm_name(app::model::routing::StaticRoutingAlgorithm value)
    {
        switch (value) {
        case app::model::routing::StaticRoutingAlgorithm::Single: return "single";
        case app::model::routing::StaticRoutingAlgorithm::Priority: return "priority";
        case app::model::routing::StaticRoutingAlgorithm::VolumeSplit: return "volume_split";
        case app::model::routing::StaticRoutingAlgorithm::Advanced: return "advanced";
        case app::model::routing::StaticRoutingAlgorithm::ThreeDsDecisionRule: return "three_ds_decision_rule";
        }
        return "unknown";
    }

    [[nodiscard]] static std::string dashboard_url()
    {
        return "https://decisionengine.hyperswitch.io";
    }

    [[nodiscard]] static std::string dashboard_path(std::string_view target)
    {
        if (target == "volume") return "volume-based-routing";
        if (target == "rule") return "rule-based-routing";
        if (target == "multi_objective") return "multi-objective-routing";
        if (target == "debit") return "debit-routing";
        return "routing";
    }

    // ── Repository helpers (local persistence for upstream store) ─────────────────

    [[nodiscard]] RoutingRecord config_record(
        interfaces::io::IRequest& req,
        std::string_view config_id,
        std::string_view name,
        std::string_view description,
        std::string_view algorithm,
        std::string_view profile,
        std::string_view config_json
    ) const
    {
        RoutingRecord record{};
        record.set_config_id(std::string{config_id});
        record.set_merchant_id(merchant_id_of(req));
        record.set_profile_id(std::string{profile});
        record.set_name(std::string{name});
        record.set_description(std::string{description});
        record.set_algorithm(std::string{algorithm});
        record.set_config_json(std::string{config_json});
        record.set_created_at(std::to_string(now_unix()));
        return record;
    }

    [[nodiscard]] std::optional<RoutingRecord> find_config(std::string_view config_id) const
    {
        return m_context->repository().find_routing_config_by_string(config_id);
    }

    [[nodiscard]] std::optional<RoutingRecord> find_config_by_algorithm(std::string_view algorithm_id) const
    {
        if (const auto direct = find_config(algorithm_id)) {
            return direct;
        }
        for (auto& record : m_context->repository().list_routing_configs()) {
            if (record.get_algorithm_id() == algorithm_id) {
                return record;
            }
        }
        return std::nullopt;
    }

    void upsert_config(RoutingRecord record) const
    {
        auto repository = m_context->repository();
        if (const auto existing = repository.find_routing_config_by_string(record.get_config_id())) {
            record.set_id(existing->get_id());
            repository.update_routing_config(record);
        } else {
            repository.save_routing_config(std::move(record));
        }
    }

    bool remove_config(std::string_view config_id) const
    {
        auto repository = m_context->repository();
        const auto existing = repository.find_routing_config_by_string(config_id);
        return existing && repository.remove_routing_config(existing->get_id());
    }

    std::vector<RoutingRecord> paginate(
        std::vector<RoutingRecord> records,
        const app::model::routing::RoutingRetrieveQuery& filter
    ) const
    {
        const std::size_t offset = filter.get_offset() ? *filter.get_offset() : 0;
        if (offset >= records.size()) {
            return {};
        }
        auto begin = records.begin() + static_cast<std::ptrdiff_t>(offset);
        if (filter.get_limit()) {
            const std::size_t limit = *filter.get_limit();
            if (limit < static_cast<std::size_t>(records.end() - begin)) {
                return std::vector<RoutingRecord>{begin, begin + static_cast<std::ptrdiff_t>(limit)};
            }
        }
        return std::vector<RoutingRecord>{begin, records.end()};
    }

    // ── Dictionary / active-config conversion (ForeignInto transformers) ─────────

    [[nodiscard]] DictionaryRecord dictionary_record(const RoutingRecord& record) const
    {
        return dictionary_record(record.get_config_id(), record.get_profile_id(), record.get_name(),
                                 record.get_description(), record.get_algorithm(),
                                 transaction_type_of(record.get_config_json()));
    }

    [[nodiscard]] DictionaryRecord dictionary_record(
        std::string_view id, std::string_view profile, std::string_view name,
        std::string_view description, std::string_view algorithm, std::string_view txn) const
    {
        DictionaryRecord item{};
        item.set_id(wrap<app::model::routing::RoutingIdValue>(std::string{id}));
        item.set_profile_id(wrap<app::model::routing::ProfileIdValue>(std::string{profile}));
        item.set_name(std::string{name});
        item.set_description(std::string{description});
        item.set_kind(app::model::routing::RoutingAlgorithmKind::Single);
        item.set_created_at(now_unix());
        item.set_modified_at(now_unix());
        item.set_algorithm_for(wrap<app::model::routing::TransactionTypeValue>(std::string{txn}));
        item.set_decision_engine_routing_id(std::nullopt);
        return item;
    }

    [[nodiscard]] std::optional<std::string> active_id_for(
        interfaces::io::IRequest& req,
        std::string_view profile,
        std::string_view txn
    ) const
    {
        if (const auto from_query = app::http::RequestQuery{}(req).get("active_id")) {
            return *from_query;
        }
        const std::string scope = std::string{profile} + "|" + std::string{txn};
        const auto resolved = m_context->superposition().resolve("routing", "active_algorithm", scope);
        if (resolved && !resolved->get_value().empty()) {
            return resolved->get_value();
        }
        return std::nullopt;
    }

    void set_active_id(interfaces::io::IRequest& req, std::string_view profile,
                       std::string_view txn, std::string value) const
    {
        (void)req;
        const std::string scope = std::string{profile} + "|" + std::string{txn};
        m_context->superposition().put_config("routing", "active_algorithm", std::move(value), scope);
    }

    // ── Cutover / dimension state (local in-process substitute) ───────────────────

    [[nodiscard]] bool is_cutover(interfaces::io::IRequest& req, std::string_view profile) const
    {
        const auto merchant = merchant_id_of(req);
        const auto global = m_context->superposition().resolve("routing", "de_cutover", merchant);
        const auto per_profile = m_context->superposition().resolve("routing", "de_cutover", merchant + "|" + std::string{profile});
        return (global && global->get_value() == "true") ||
               (per_profile && per_profile->get_value() == "true");
    }

    // ── Decision-manager / surcharge config helpers ───────────────────────────────

    [[nodiscard]] static std::string decision_key(std::string_view profile, std::string_view kind)
    {
        return profile.empty() ? std::string{kind} : std::string{profile} + ":" + std::string{kind};
    }

    void set_active_algo_ref(interfaces::io::IRequest& req, std::string_view profile,
                             std::string_view field, std::string value) const
    {
        (void)req;
        m_context->superposition().put_config("routing", std::string{field}, std::move(value), std::string{profile});
    }

    [[nodiscard]] static std::string generic_string(
        const serde::Value& value, std::string_view key, std::string_view fallback_key = {})
    {
        const auto* object = std::get_if<serde::Value::Object>(&value.get());
        if (object == nullptr) {
            return {};
        }
        auto field = object->get(std::string{key});
        if (!field && !fallback_key.empty()) {
            field = object->get(std::string{fallback_key});
        }
        if (!field) {
            return {};
        }
        if (const auto* text = std::get_if<std::string>(&field->get())) {
            return *text;
        }
        return {};
    }

    [[nodiscard]] bool valid_program(std::string_view program) const
    {
        auto generic = serde::Ser::decode_generic("application/json", program);
        return generic.has_value() && std::holds_alternative<serde::Value::Object>(generic->get());
    }

    // ── Default fallback config helpers (helpers::get/update_merchant_default_config) ──

    [[nodiscard]] std::string default_key(std::string_view profile) const
    {
        return decision_key(profile, "default");
    }

    [[nodiscard]] Connectors default_config(std::string_view profile) const
    {
        const auto stored = find_config(default_key(profile));
        if (!stored) {
            return {};
        }
        auto parsed = serde::Ser::deserialize<Connectors>("application/json", stored->get_config_json());
        return parsed ? std::move(*parsed) : Connectors{};
    }

    void store_default_config(
        interfaces::io::IRequest& req,
        std::string_view profile,
        const Connectors& connectors
    ) const
    {
        const auto key = default_key(profile);
        RoutingRecord record{};
        record.set_config_id(key);
        record.set_merchant_id(merchant_id_of(req));
        record.set_profile_id(std::string{profile});
        record.set_name("default");
        record.set_description("fallback routing");
        record.set_algorithm("default");
        record.set_config_json(serde::Ser::encode_json(connectors));
        record.set_created_at(std::to_string(now_unix()));
        upsert_config(std::move(record));
    }

    [[nodiscard]] bool validate_default_connectors(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view profile,
        const Connectors& updated
    ) const
    {
        (void)req;
        const auto existing = default_config(profile);
        if (existing.size() != updated.size()) {
            send_error(res, std::move(send), error(core::error::AppErrorKind::PRECONDITION_FAILED,
                                                   "current config and updated config have different lengths"));
            return false;
        }
        std::vector<std::string> a, b;
        for (const auto& c : existing) a.push_back(connector_key(c));
        for (const auto& c : updated) b.push_back(connector_key(c));
        std::ranges::sort(a);
        std::ranges::sort(b);
        std::vector<std::string> diff;
        std::ranges::set_symmetric_difference(a, b, std::back_inserter(diff));
        if (!diff.empty()) {
            std::string joined;
            for (std::size_t i = 0; i < diff.size(); ++i) {
                if (i) joined += ", ";
                joined += diff[i];
            }
            send_error(res, std::move(send), error(core::error::AppErrorKind::INVALID_REQUEST,
                                                   std::format("connector mismatch between old and new configs ({})", joined)));
            return false;
        }
        return true;
    }

    [[nodiscard]] static std::string connector_key(const app::model::routing::RoutableConnectorChoice& c)
    {
        return std::string{c.get_connector().get_value()};
    }

    // ── Dynamic routing helpers ───────────────────────────────────────────────────

    void dynamic_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view operation,
        std::string_view name,
        std::string_view algorithm,
        std::string_view description
    )
    {
        if (!authorize_operation(req, res, send, operation)) {
            return;
        }
        const auto profile = profile_id_of(req);
        const auto feature = app::http::RequestQuery{}(req).get("enable").value_or("metrics");
        const auto body = req.body_as_string();
        if (feature == "none") {
            put_dynamic_ref(profile, R"({"success_based_algorithm":null,"elimination_routing_algorithm":null})");
            track(operation);
            send_ok(res, send);
            return;
        }
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        if (!serde::Ser::decode_generic("application/json", body)) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        if (const auto validation_error = profile_validation_error(req, profile)) {
            send_error(res, send, error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error));
            return;
        }
        const auto algorithm_id = new_routing_id();
        auto record = config_record(req, algorithm_id, name, description, algorithm, profile, body);
        record.set_algorithm_id(algorithm_id);
        upsert_config(std::move(record));
        const auto key = algorithm == "success_rate_based"
                             ? "success_based_algorithm" : "elimination_routing_algorithm";
        put_dynamic_ref(profile, std::format(
            R"({{"{}":{{"algorithm_id_with_timestamp":{{"algorithm_id":"{}","timestamp":{}}},"enabled_feature":"{}"}}}})",
            key, algorithm_id, now_unix(), feature));
        track(operation);
        send_json(res, send, serde::Ser::encode_json(
            dictionary_record(algorithm_id, profile, name, description, algorithm, "payment")), true);
    }

    void put_dynamic_ref(std::string_view profile, std::string value) const
    {
        m_context->superposition().put_config("routing", "dynamic_algorithm_ref", std::move(value), std::string{profile});
    }

    [[nodiscard]] bool dynamic_ref_has_feature(std::string_view profile, std::string_view key,
                                               std::string_view feature) const
    {
        const auto ref = m_context->superposition().resolve("routing", "dynamic_algorithm_ref", std::string{profile});
        if (!ref) {
            return false;
        }
        return ref->get_value().find(std::string{key}) != std::string::npos &&
               ref->get_value().find(std::string{feature}) != std::string::npos;
    }

    void update_dynamic_volume_split(std::string_view profile, std::uint8_t split) const
    {
        const auto value = std::format(R"({{"dynamic_routing_volume_split":{}}})", static_cast<unsigned>(split));
        m_context->superposition().put_config("routing", "dynamic_algorithm_ref", value, std::string{profile});
    }

    [[nodiscard]] std::uint8_t dynamic_volume_split(std::string_view profile) const
    {
        const auto ref = m_context->superposition().resolve("routing", "dynamic_algorithm_ref", std::string{profile});
        if (!ref) {
            return 0;
        }
        const auto value = generic_number(ref->get_value(), "dynamic_routing_volume_split");
        return value ? static_cast<std::uint8_t>(*value) : 0;
    }

    [[nodiscard]] static std::optional<std::int64_t> generic_number(std::string_view json, std::string_view key)
    {
        auto parsed = serde::Ser::decode_generic("application/json", json);
        if (!parsed) {
            return std::nullopt;
        }
        const auto* object = std::get_if<serde::Value::Object>(&parsed->get());
        if (!object) {
            return std::nullopt;
        }
        const auto field = object->get(std::string{key});
        if (!field) {
            return std::nullopt;
        }
        if (const auto* n = std::get_if<std::int64_t>(&field->get())) {
            return *n;
        }
        if (const auto* n = std::get_if<double>(&field->get())) {
            return static_cast<std::int64_t>(*n);
        }
        return std::nullopt;
    }

    // ── Local DE rule store (external-infra substitute) ───────────────────────────

    [[nodiscard]] std::vector<std::string> de_rule_ids(std::string_view profile) const
    {
        const auto stored = m_context->superposition().resolve("routing.de_rules", "ids", std::string{profile});
        if (!stored || stored->get_value().empty()) {
            return {};
        }
        std::vector<std::string> ids;
        for (auto part : std::views::split(stored->get_value(), ',')) {
            std::string id{std::ranges::begin(part), std::ranges::end(part)};
            if (!id.empty()) {
                ids.push_back(std::move(id));
            }
        }
        return ids;
    }

    void add_de_rule_id(std::string_view profile, std::string_view id) const
    {
        auto ids = de_rule_ids(profile);
        if (std::ranges::find(ids, id) != ids.end()) {
            return;
        }
        ids.emplace_back(id);
        std::string joined;
        for (std::size_t i = 0; i < ids.size(); ++i) {
            if (i) joined += ',';
            joined += ids[i];
        }
        m_context->superposition().put_config("routing.de_rules", "ids", std::move(joined), std::string{profile});
    }

    // ── List implementation shared by merchant / profile-scoped routes ────────────

    void list_impl(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        bool profile_scoped,
        std::string_view flow
    )
    {
        const auto merchant = merchant_id_of(req);
        const auto profile = profile_id_of(req);
        const auto txn = transaction_type_of(req);
        const auto query = app::http::RequestQuery{}(req);
        app::model::routing::RoutingRetrieveQuery filter;
        if (const auto limit = query_number<std::uint16_t>(query, "limit")) filter.set_limit(limit);
        if (const auto offset = query_number<std::uint8_t>(query, "offset")) filter.set_offset(offset);
        if (const auto from_query = query.get("transaction_type")) {
            filter.set_transaction_type(wrap<app::model::routing::TransactionTypeValue>(*from_query));
        }

        auto records = m_context->repository().list_routing_configs();
        std::vector<RoutingRecord> selected;
        for (auto& record : records) {
            if (!merchant.empty() && record.get_merchant_id() != merchant) continue;
            if (profile_scoped && !profile.empty() && record.get_profile_id() != profile) continue;
            if (!record.get_config_id().ends_with(":default") &&
                transaction_type_of(record.get_config_json()) != txn) {
                continue;
            }
            selected.push_back(std::move(record));
        }
        selected = paginate(std::move(selected), filter);

        // DE list merge (core:636-702). DE listing is unavailable locally, so the local
        // substitute degrades to the Hyperswitch records exactly like upstream on DE failure.
        std::vector<DictionaryRecord> items;
        items.reserve(selected.size());
        for (const auto& record : selected) {
            items.push_back(dictionary_record(record));
        }
        track(flow);
        send_json(res, send, serde::Ser::encode_json(items), true);
    }

    // ── Wire helpers ──────────────────────────────────────────────────────────────

    [[nodiscard]] core::error::AppError error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error_value
    ) const
    {
        auto text = serde::Ser::encode_json(error_value);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error_value.get_kind()));
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

    void send_ok(interfaces::io::IResponse& res, std::function<void()> send) const
    {
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body({});
        send();
    }

    void track(std::string_view flow) const
    {
        const auto metric = std::string{"routing."} + std::string{flow};
        m_context->metrics().counter(metric).increment();
        m_context->metrics().record_flow(metric);
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::routing
