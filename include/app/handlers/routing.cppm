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

class RoutingRouteHandlers
{
public:
    explicit RoutingRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    void handle_routing_entry(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "routing_entry")) {
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = app::model::routing::RoutingEntryRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto merchant = merchant_id_of(req);
        const auto org = org_id_of(req);
        const auto profile = profile_id_of(req);
        const auto key_id = m_context->ids().key_id();
        const auto digest = app::crypto::KeyedHasher{"de-sso"}.hash(key_id);
        std::uint64_t step{};
        const auto [end, ec] = std::from_chars(digest.data(), digest.data() + 8, step, 16);
        (void)end;
        (void)ec;
        const auto code = app::crypto::TotpGenerator{}.generate("de-sso:" + merchant, step);
        const auto payload = std::format(
            R"({{"merchant":"{}","org":"{}","profile":"{}","code":"{}"}})",
            merchant,
            org,
            profile,
            code
        );
        const auto token = app::crypto::JwtSigner{"routing-de-sso"}.sign(payload);

        // One-time SSO state is ephemeral: cache, never the routing-config store.
        const auto cache_key = "routing.entry:" + code;
        m_context->cache().put(cache_key, core::cache::to_cache_value(token));
        const auto cached = m_context->cache().get(cache_key).value_or(std::string{});
        if (cached.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INTERNAL, "SSO token not cached"));
            return;
        }

        app::model::routing::RoutingEntryResponse response;
        const auto cutover = m_context->superposition().resolve("routing", "de_cutover", profile);
        response.set_is_cutover(cutover && cutover->get_value() == "true");
        response.set_redirect_url(
            "https://decisionengine.hyperswitch.io/sso?code=" + code + "&token=" + cached
        );
        track("routing_entry");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

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
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = app::model::routing::RoutingConfigRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto config_id = m_context->ids().payment_id("rcfg_");
        if (find_config(config_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::ALREADY_EXISTS, "routing config already exists"));
            return;
        }

        const auto profile = typed->get_profile_id().get_value().empty()
                                 ? profile_id_of(req)
                                 : typed->get_profile_id().get_value();
        auto record = config_record(
            req,
            config_id,
            typed->get_name(),
            typed->get_description(),
            algorithm_name(typed->get_algorithm()),
            profile,
            serde::Ser::encode_json(*typed)
        );
        upsert_config(std::move(record));
        track("routing_create_config");
        send_json(res, send, serde::Ser::encode_json(*typed), true);
    }

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
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Link = app::model::routing::RoutingLinkWrapper;
        using Activate = app::model::routing::RoutingActivatePayload;
        auto link = serde::Ser::deserialize<Link>("application/json", body);
        auto activate = serde::Ser::deserialize<Activate>("application/json", body);
        if (!link && !activate) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto algorithm = algorithm_id_of(req);
        if (link) {
            const auto& named = link->get_algorithm_id().get_routing_algorithm_id().get_value();
            if (!named.empty()) {
                algorithm = named;
            }
        }

        const auto existing = find_config(algorithm);
        if (!existing) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "routing algorithm not found"));
            return;
        }
        auto updated = *existing;
        updated.set_algorithm_id(algorithm);
        updated.set_config_json("linked:" + algorithm + ":" + existing->get_config_json());
        upsert_config(std::move(updated));

        const auto profile = profile_id_of(req);
        const std::string scope{profile.empty() ? merchant_id_of(req) : profile};
        m_context->superposition().put_config("routing", "active_algorithm", algorithm, scope);

        const auto text = link ? serde::Ser::encode_json(*link)
                               : serde::Ser::encode_json(*activate);
        track("routing_link_config");
        send_json(res, send, text, true);
    }

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
        const auto existing = find_config(algorithm);
        if (!existing) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "routing algorithm not found"));
            return;
        }

        app::model::routing::RoutingConfigName response;
        response.set_value(existing->get_name().empty() ? existing->get_config_id() : existing->get_name());
        track("routing_retrieve_config");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_list_routing_configs(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "list_routing_configs")) {
            return;
        }

        const auto merchant = merchant_id_of(req);
        const auto query = app::http::RequestQuery{}(req);
        app::model::routing::RoutingRetrieveQuery filter;
        if (const auto limit = query_number<std::uint16_t>(query, "limit")) {
            filter.set_limit(limit);
        }
        if (const auto offset = query_number<std::uint8_t>(query, "offset")) {
            filter.set_offset(offset);
        }
        if (const auto transaction = query.get("transaction_type")) {
            filter.set_transaction_type(wrap<app::model::routing::TransactionTypeValue>(*transaction));
        }

        auto records = m_context->repository().list_routing_configs();
        std::vector<RoutingRecord> selected;
        for (auto& record : records) {
            if (!merchant.empty() && record.get_merchant_id() != merchant) {
                continue;
            }
            selected.push_back(std::move(record));
        }
        selected = paginate(std::move(selected), filter);

        auto response = dictionary_for(merchant, selected, active_id_for(req, profile_id_of(req)));
        track("list_routing_configs");
        send_json(res, send, serde::Ser::encode_json(response), true);
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

        const auto merchant = merchant_id_of(req);
        const auto profile = profile_id_of(req);
        const auto query = app::http::RequestQuery{}(req);
        app::model::routing::RoutingRetrieveQuery filter;
        if (const auto limit = query_number<std::uint16_t>(query, "limit")) {
            filter.set_limit(limit);
        }
        if (const auto offset = query_number<std::uint8_t>(query, "offset")) {
            filter.set_offset(offset);
        }

        auto records = m_context->repository().list_routing_configs();
        std::vector<RoutingRecord> selected;
        for (auto& record : records) {
            if (!merchant.empty() && record.get_merchant_id() != merchant) {
                continue;
            }
            if (!profile.empty() && record.get_profile_id() != profile) {
                continue;
            }
            selected.push_back(std::move(record));
        }
        selected = paginate(std::move(selected), filter);

        auto response = dictionary_for(merchant, selected, active_id_for(req, profile));
        track("list_routing_configs_for_profile");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

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
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = app::model::routing::RoutingConfigRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto algorithm = algorithm_id_of(req);
        if (const auto query = app::http::RequestQuery{}(req).get("algorithm_id")) {
            algorithm = *query;
        }
        if (algorithm.empty() && typed) {
            algorithm = typed->get_name();
        }
        if (algorithm.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing algorithm id"));
            return;
        }
        if (!remove_config(algorithm)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "routing algorithm not found"));
            return;
        }

        const auto profile = profile_id_of(req);
        const std::string scope{profile.empty() ? merchant_id_of(req) : profile};
        if (const auto active = active_id_for(req, profile); active && *active == algorithm) {
            m_context->superposition().put_config("routing", "active_algorithm", "", scope);
        }

        app::model::routing::RoutingConfigName response;
        response.set_value(algorithm);
        track("routing_unlink_config");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

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
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = std::vector<app::model::routing::RoutableConnectorChoice>;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            auto generic = serde::Ser::decode_generic("application/json", body);
            if (!generic) {
                send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            (void)generic;
        }

        const auto profile = profile_id_of(req);
        app::model::routing::ProfileDefaultRoutingConfig response;
        response.set_profile_id(wrap<app::model::routing::ProfileIdValue>(profile));
        if (typed) {
            response.set_connectors(*typed);
        }
        const auto config_id = profile.empty() ? std::string{"default"} : profile + ":default";
        auto record = config_record(
            req,
            config_id,
            "default",
            "fallback routing",
            "default",
            profile,
            serde::Ser::encode_json(response)
        );
        upsert_config(std::move(record));
        track("routing_update_default_config");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_routing_retrieve_default_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "routing_retrieve_default_config")) {
            return;
        }

        const auto profile = profile_id_of(req);
        const auto config_id = profile.empty() ? std::string{"default"} : profile + ":default";
        app::model::routing::ProfileDefaultRoutingConfig response;
        response.set_profile_id(wrap<app::model::routing::ProfileIdValue>(profile));
        if (const auto stored = find_config(config_id)) {
            auto parsed = serde::Ser::deserialize<app::model::routing::ProfileDefaultRoutingConfig>(
                "application/json",
                stored->get_config_json()
            );
            if (parsed) {
                response = std::move(*parsed);
            }
        }
        track("routing_retrieve_default_config");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

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
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = app::dto::surcharge_decision_configs::SurchargeDecisionConfigReq;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto profile = profile_id_of(req);
        const auto config_id = profile.empty() ? std::string{"surcharge"} : profile + ":surcharge";
        auto record = config_record(
            req,
            config_id,
            typed->get_name().value_or("surcharge"),
            "surcharge decision manager",
            "surcharge",
            profile,
            body
        );
        upsert_config(std::move(record));
        track("upsert_surcharge_decision_manager_config");
        send_json(res, send, serde::Ser::encode_json(*typed), true);
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
        const auto config_id = profile.empty() ? std::string{"surcharge"} : profile + ":surcharge";
        if (!remove_config(config_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "surcharge decision config not found"));
            return;
        }

        app::model::routing::RoutingConfigName response;
        response.set_value(config_id);
        track("delete_surcharge_decision_manager_config");
        send_json(res, send, serde::Ser::encode_json(response), true);
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
        const auto config_id = profile.empty() ? std::string{"surcharge"} : profile + ":surcharge";
        app::dto::surcharge_decision_configs::SurchargeDecisionConfigs response;
        if (const auto stored = find_config(config_id)) {
            app::dto::surcharge_decision_configs::SurchargeDetailsOutput details;
            response.set_surcharge_details(details);
        }
        track("retrieve_surcharge_decision_manager_config");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

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
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = app::dto::conditional_configs::DecisionManagerRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        std::optional<serde::Value> generic{};
        if (!typed) {
            auto decoded = serde::Ser::decode_generic("application/json", body);
            if (!decoded) {
                send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            generic = std::move(*decoded);
        }

        const auto profile = profile_id_of(req);
        const auto config_id = profile.empty() ? std::string{"decision"} : profile + ":decision";
        const auto name = (typed && typed->get_name()) ? *typed->get_name() : std::string{"decision"};
        auto record = config_record(
            req,
            config_id,
            name,
            "decision manager",
            "decision",
            profile,
            body
        );
        upsert_config(std::move(record));
        if (typed && typed->get_program()) {
            m_context->superposition().put_config(
                "routing",
                config_id + ":program",
                typed->get_program()->get_value(),
                profile
            );
        }
        const auto text = typed ? serde::Ser::encode_json(*typed) : serde::Ser::encode_json(*generic);
        track("upsert_decision_manager_config");
        send_json(res, send, text, true);
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
        const auto config_id = profile.empty() ? std::string{"decision"} : profile + ":decision";
        if (!remove_config(config_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "decision manager config not found"));
            return;
        }

        app::model::routing::RoutingConfigName response;
        response.set_value(config_id);
        track("delete_decision_manager_config");
        send_json(res, send, serde::Ser::encode_json(response), true);
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

        const auto profile = profile_id_of(req);
        const auto config_id = profile.empty() ? std::string{"decision"} : profile + ":decision";
        app::model::routing::RoutingRetrieveResponse response;
        if (const auto stored = find_config(config_id)) {
            app::model::routing::MerchantRoutingAlgorithm algorithm;
            algorithm.set_name(stored->get_name());
            algorithm.set_profile_id(wrap<app::model::routing::ProfileIdValue>(stored->get_profile_id()));
            response.set_algorithm(algorithm);
        }
        track("retrieve_decision_manager_config");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

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
        const auto profile = profile_id_of(req);
        auto records = m_context->repository().list_routing_configs();
        std::vector<RoutingRecord> selected;
        for (auto& record : records) {
            if (!merchant.empty() && record.get_merchant_id() != merchant) {
                continue;
            }
            if (!profile.empty() && record.get_profile_id() != profile) {
                continue;
            }
            selected.push_back(std::move(record));
        }
        auto response = dictionary_for(merchant, selected, active_id_for(req, profile));
        track("routing_retrieve_linked_config");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_routing_retrieve_default_config_for_profiles(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "routing_retrieve_default_config_for_profiles")) {
            return;
        }

        const auto merchant = merchant_id_of(req);
        const auto query = app::http::RequestQuery{}(req);
        app::model::routing::ProfileDefaultRoutingConfig response;
        if (const auto profile = query.get("profile_id")) {
            response.set_profile_id(wrap<app::model::routing::ProfileIdValue>(*profile));
            if (const auto stored = find_config(*profile + ":default")) {
                auto parsed = serde::Ser::deserialize<app::model::routing::ProfileDefaultRoutingConfig>(
                    "application/json",
                    stored->get_config_json()
                );
                if (parsed) {
                    response = std::move(*parsed);
                }
            }
        }
        track("routing_retrieve_default_config_for_profiles");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

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
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Connectors = std::vector<app::model::routing::RoutableConnectorChoice>;
        auto connectors = serde::Ser::deserialize<Connectors>("application/json", body);
        auto toggle = serde::Ser::deserialize<app::model::routing::ToggleDynamicRoutingWrapper>(
            "application/json",
            body
        );
        if (!connectors && !toggle) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto profile = profile_id_of(req);
        if (toggle && !toggle->get_profile_id().get_value().empty()) {
            profile = toggle->get_profile_id().get_value();
        }
        app::model::routing::ProfileDefaultRoutingConfig response;
        response.set_profile_id(wrap<app::model::routing::ProfileIdValue>(profile));
        if (connectors) {
            response.set_connectors(*connectors);
        }
        const auto config_id = profile.empty() ? std::string{"default"} : profile + ":default";
        auto record = config_record(
            req,
            config_id,
            "default",
            "fallback routing",
            "default",
            profile,
            serde::Ser::encode_json(response)
        );
        upsert_config(std::move(record));
        track("routing_update_default_config_for_profile");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_create_success_based_routing(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "create_success_based_routing")) {
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = app::model::routing::CreateDynamicRoutingWrapper;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto profile = profile_id_of(req);
        if (!typed->get_profile_id().get_value().empty()) {
            profile = typed->get_profile_id().get_value();
        }
        const auto algorithm = m_context->ids().payment_id("ralg_");
        auto record = config_record(
            req,
            algorithm,
            "success_based",
            "success based routing",
            "success_rate_based",
            profile,
            body
        );
        upsert_config(std::move(record));
        track("create_success_based_routing");
        send_json(res, send, serde::Ser::encode_json(*typed), true);
    }

    void handle_contract_based_routing_setup_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "contract_based_routing_setup_config")) {
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = app::model::routing::ContractBasedRoutingSetupPayloadWrapper;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto profile = profile_id_of(req);
        const auto algorithm = m_context->ids().payment_id("ralg_");
        auto record = config_record(
            req,
            algorithm,
            "contract_based",
            "contract based routing",
            "contract_based",
            profile,
            body
        );
        upsert_config(std::move(record));
        track("contract_based_routing_setup_config");
        send_json(res, send, serde::Ser::encode_json(*typed), true);
    }

    void handle_create_elimination_routing(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "create_elimination_routing")) {
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = app::model::routing::CreateDynamicRoutingWrapper;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto profile = profile_id_of(req);
        if (!typed->get_profile_id().get_value().empty()) {
            profile = typed->get_profile_id().get_value();
        }
        const auto algorithm = m_context->ids().payment_id("ralg_");
        auto record = config_record(
            req,
            algorithm,
            "elimination",
            "elimination routing",
            "elimination",
            profile,
            body
        );
        upsert_config(std::move(record));
        track("create_elimination_routing");
        send_json(res, send, serde::Ser::encode_json(*typed), true);
    }

    void handle_set_dynamic_routing_volume_split(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "set_dynamic_routing_volume_split")) {
            return;
        }

        auto body = req.body_as_string();
        const auto query = app::http::RequestQuery{}(req);
        if (body.empty()) {
            app::model::routing::DynamicRoutingVolumeSplitQuery from_query;
            if (const auto split = query_number<std::uint8_t>(query, "split")) {
                from_query.set_split(*split);
            }
            body = serde::Ser::encode_json(from_query);
        }

        auto typed = serde::Ser::deserialize<app::model::routing::DynamicRoutingVolumeSplitQuery>(
            "application/json",
            body
        );
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto profile = profile_id_of(req);
        const auto config_id = profile.empty() ? std::string{"volume_split"} : profile + ":volume_split";
        auto record = config_record(
            req,
            config_id,
            "volume_split",
            "dynamic routing volume split",
            "volume_split",
            profile,
            body
        );
        upsert_config(std::move(record));

        app::model::routing::RoutingVolumeSplitResponse response;
        response.set_split(typed->get_split());
        track("set_dynamic_routing_volume_split");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

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
        const auto config_id = profile.empty() ? std::string{"volume_split"} : profile + ":volume_split";
        std::uint8_t split{};
        if (const auto stored = find_config(config_id)) {
            auto parsed = serde::Ser::deserialize<app::model::routing::DynamicRoutingVolumeSplitQuery>(
                "application/json",
                stored->get_config_json()
            );
            if (parsed) {
                split = parsed->get_split();
            }
        } else if (const auto from_query = query_number<std::uint8_t>(app::http::RequestQuery{}(req), "split")) {
            split = *from_query;
        }

        app::model::routing::RoutingVolumeSplitResponse response;
        response.set_split(split);
        track("get_dynamic_routing_volume_split");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_routing_migration_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "routing_migration_status")) {
            return;
        }

        auto body = req.body_as_string();
        const auto query = app::http::RequestQuery{}(req);
        if (body.empty()) {
            app::model::routing::RoutingMigrationStatusQuery from_query;
            if (const auto limit = query_number<std::uint32_t>(query, "limit")) {
                from_query.set_limit(limit);
            }
            if (const auto offset = query_number<std::uint32_t>(query, "offset")) {
                from_query.set_offset(offset);
            }
            body = serde::Ser::encode_json(from_query);
        }
        auto typed = serde::Ser::deserialize<app::model::routing::RoutingMigrationStatusQuery>(
            "application/json",
            body
        );
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        app::model::routing::RoutingMigrationStatusResponse response;
        response.set_limit(typed->get_limit().value_or(20));
        response.set_offset(typed->get_offset().value_or(0));
        response.set_has_more(false);
        track("routing_migration_status");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

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
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto typed = serde::Ser::deserialize<app::model::routing::RuleMigrationRequest>(
            "application/json",
            body
        );
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto repository = m_context->repository();
        const auto configs = repository.list_routing_configs();
        app::model::routing::RuleMigrationTotals totals;
        totals.set_profiles(configs.size());
        totals.set_rules_migrated(configs.size());
        app::model::routing::RuleMigrationResult response;
        response.set_totals(totals);
        track("migrate_routing_rules");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/routing"}
            .add_route(
                Route{"/entry"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_entry(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_entry")
            )
            .add_route(
                Route{"/active"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_retrieve_linked_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_retrieve_linked_config")
            )
            .add_route(
                Route{"/"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_routing_configs(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_routing_configs")
            )
            .add_route(
                Route{"/"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_create_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_create_config")
            )
            .add_route(
                Route{"/list/profile"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_routing_configs_for_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_routing_configs_for_profile")
            )
            .add_route(
                Route{"/default"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_update_default_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_update_default_config")
            )
            .add_route(
                Route{"/rule/migrate"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_migrate_routing_rules(req, res, std::move(send));
                    })
                    .summary("Hyperswitch migrate_routing_rules")
            )
            .add_route(
                Route{"/migration/status"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_migration_status(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_migration_status")
            )
            .add_route(
                Route{"/deactivate"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_unlink_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_unlink_config")
            )
            .add_route(
                Route{"/decision"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_upsert_decision_manager_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch upsert_decision_manager_config")
            )
            .add_route(
                Route{"/decision"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_decision_manager_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_decision_manager_config")
            )
            .add_route(
                Route{"/decision"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_delete_decision_manager_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch delete_decision_manager_config")
            )
            .add_route(
                Route{"/decision/surcharge"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_upsert_surcharge_decision_manager_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch upsert_surcharge_decision_manager_config")
            )
            .add_route(
                Route{"/decision/surcharge"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_surcharge_decision_manager_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_surcharge_decision_manager_config")
            )
            .add_route(
                Route{"/decision/surcharge"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_delete_surcharge_decision_manager_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch delete_surcharge_decision_manager_config")
            )
            .add_route(
                Route{"/default/profile/:profile_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_update_default_config_for_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_update_default_config_for_profile")
            )
            .add_route(
                Route{"/default/profile"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_retrieve_default_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_retrieve_default_config")
            )
            .add_route(
                Route{"/:algorithm_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_retrieve_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_retrieve_config")
            )
            .add_route(
                Route{"/:algorithm_id/activate"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_link_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_link_config")
            )
            .add_route(
                Route{"/payouts"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_routing_configs(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_routing_configs")
            )
            .add_route(
                Route{"/payouts"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_create_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_create_config")
            )
            .add_route(
                Route{"/payouts/list/profile"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_routing_configs_for_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_routing_configs_for_profile")
            )
            .add_route(
                Route{"/payouts/active"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_retrieve_linked_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_retrieve_linked_config")
            )
            .add_route(
                Route{"/payouts/default"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_retrieve_default_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_retrieve_default_config")
            )
            .add_route(
                Route{"/payouts/default"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_update_default_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_update_default_config")
            )
            .add_route(
                Route{"/payouts/:algorithm_id/activate"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_link_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_link_config")
            )
            .add_route(
                Route{"/payouts/deactivate"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_unlink_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_unlink_config")
            )
            .add_route(
                Route{"/payouts/default/profile/:profile_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_update_default_config_for_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_update_default_config_for_profile")
            )
            .add_route(
                Route{"/payouts/default/profile"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_retrieve_default_config_for_profiles(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_retrieve_default_config_for_profiles")
            )
            .add_route(
                Route{"/v2/routing-algorithms"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_create_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_create_config")
            )
            .add_route(
                Route{"/v2/routing-algorithms/:algorithm_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_retrieve_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_retrieve_config")
            )
            .add_route(
                Route{"/v2/profiles/:profile_id/fallback-routing"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_retrieve_default_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_retrieve_default_config")
            )
            .add_route(
                Route{"/v2/profiles/:profile_id/fallback-routing"}
                    .patch([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_update_default_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_update_default_config")
            )
            .add_route(
                Route{"/v2/profiles/:profile_id/activate-routing-algorithm"}
                    .patch([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_link_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_link_config")
            )
            .add_route(
                Route{"/v2/profiles/:profile_id/deactivate-routing-algorithm"}
                    .patch([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_unlink_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_unlink_config")
            )
            .add_route(
                Route{"/v2/profiles/:profile_id/routing-algorithm"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_routing_retrieve_linked_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch routing_retrieve_linked_config")
            )
            .add_route(
                Route{"/v2/profiles/:profile_id/decision"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_upsert_decision_manager_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch upsert_decision_manager_config")
            )
            .add_route(
                Route{"/v2/profiles/:profile_id/decision"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_decision_manager_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_decision_manager_config")
            )
            .add_route(
                Route{"/account/:account_id/business_profile/:profile_id/dynamic_routing/success_based/create"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_success_based_routing(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_success_based_routing")
            )
            .add_route(
                Route{"/account/:account_id/business_profile/:profile_id/dynamic_routing/set_volume_split"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_set_dynamic_routing_volume_split(req, res, std::move(send));
                    })
                    .summary("Hyperswitch set_dynamic_routing_volume_split")
            )
            .add_route(
                Route{"/account/:account_id/business_profile/:profile_id/dynamic_routing/get_volume_split"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_dynamic_routing_volume_split(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_dynamic_routing_volume_split")
            )
            .add_route(
                Route{"/account/:account_id/business_profile/:profile_id/dynamic_routing/elimination/create"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_elimination_routing(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_elimination_routing")
            )
            .add_route(
                Route{"/account/:account_id/business_profile/:profile_id/dynamic_routing/contracts/toggle"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_contract_based_routing_setup_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch contract_based_routing_setup_config")
            );
    }

private:
    using RoutingRecord = std::remove_cvref_t<
        decltype(*std::declval<app::context::AppContext&>()
                     .repository()
                     .find_routing_config_by_string(std::string_view{}))>;

    template<typename T>
    [[nodiscard]] T wrap(std::string value) const
    {
        T result{};
        result.set_value(std::move(value));
        return result;
    }

    template<typename T>
    [[nodiscard]] std::optional<T> query_number(
        const app::http::QueryParams& query,
        std::string_view key
    ) const
    {
        const auto raw = query.get(key);
        if (!raw) {
            return std::nullopt;
        }
        T value{};
        const auto [end, ec] = std::from_chars(raw->data(), raw->data() + raw->size(), value);
        (void)end;
        if (ec != std::errc{}) {
            return std::nullopt;
        }
        return value;
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
        const auto segments = app::http::RequestPath{}(req);
        for (std::size_t i = 0; i + 1 < segments.count(); ++i) {
            const auto current = segments.at(i).value();
            if (current == "profiles" || current == "business_profile") {
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

    /** Auth resolved from the auditable RouteAuthRegistry, never built inline. */
    [[nodiscard]] bool authorize_operation(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) {
            send_error(res, send, *auth);
            return false;
        }
        return true;
    }

    [[nodiscard]] std::string algorithm_name(app::model::routing::StaticRoutingAlgorithm value) const
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
        record.set_merchant_id(app::http::merchant_id_from_header{}(req));
        record.set_profile_id(std::string{profile});
        record.set_name(std::string{name});
        record.set_description(std::string{description});
        record.set_algorithm(std::string{algorithm});
        record.set_config_json(std::string{config_json});
        return record;
    }

    [[nodiscard]] std::optional<RoutingRecord> find_config(std::string_view config_id) const
    {
        return m_context->repository().find_routing_config_by_string(config_id);
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
        if (!existing) {
            return false;
        }
        return repository.remove_routing_config(existing->get_id());
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

    /** Query override first, then the deterministic Superposition active-config record. */
    [[nodiscard]] std::optional<std::string> active_id_for(
        interfaces::io::IRequest& req,
        std::string_view profile
    ) const
    {
        if (const auto from_query = app::http::RequestQuery{}(req).get("active_id")) {
            return *from_query;
        }
        const std::string scope{profile.empty() ? merchant_id_of(req) : std::string{profile}};
        const auto resolved = m_context->superposition().resolve("routing", "active_algorithm", scope);
        if (resolved && !resolved->get_value().empty()) {
            return resolved->get_value();
        }
        return std::nullopt;
    }

    [[nodiscard]] app::model::routing::RoutingDictionary dictionary_for(
        std::string_view merchant,
        const std::vector<RoutingRecord>& records,
        std::optional<std::string> active_id
    ) const
    {
        app::model::routing::RoutingDictionary result;
        result.set_merchant_id(wrap<app::model::routing::MerchantIdValue>(std::string{merchant}));
        result.set_active_id(std::move(active_id));
        std::vector<app::model::routing::RoutingDictionaryRecord> items;
        items.reserve(records.size());
        for (const auto& record : records) {
            app::model::routing::RoutingDictionaryRecord item;
            item.set_id(wrap<app::model::routing::RoutingIdValue>(record.get_config_id()));
            item.set_profile_id(wrap<app::model::routing::ProfileIdValue>(record.get_profile_id()));
            item.set_name(record.get_name());
            item.set_description(record.get_description());
            items.push_back(std::move(item));
        }
        result.set_records(std::move(items));
        return result;
    }

    void track(std::string_view flow) const
    {
        const auto metric = std::string{"routing."} + std::string{flow};
        m_context->metrics().counter(metric).increment();
        m_context->metrics().record_flow(metric);
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


} // namespace app::handlers::routing
