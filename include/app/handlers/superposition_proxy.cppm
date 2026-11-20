module;
export module app_handlers:superposition_proxy;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import core_cache;
import core_error;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_services;

export namespace app::handlers::superposition_proxy {

class SuperpositionProxyRouteHandlers
{
public:
    explicit SuperpositionProxyRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    void handle_list_contexts(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_request(req, "list_contexts")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto scope = require_proxy_scope(req, res, send);
        if (!scope) {
            return;
        }
        cache_scope(req, *scope);

        // Full typed ListContextsQuery. Every upstream field is parsed; filters that map to
        // local config/context data are applied, the rest are carried as request state.
        const auto query = app::http::RequestQuery{}(req);
        ContextQuery context_query{};
        context_query.pagination = pagination(query);
        if (const auto value = query.get("prefix")) {
            context_query.prefixes = csv_values(*value);
        }
        if (const auto value = query.get("sort_on")) {
            context_query.sort_on = *value;
        }
        if (const auto value = query.get("sort_by")) {
            context_query.sort_by = *value;
        }
        if (const auto value = query.get("created_by")) {
            context_query.created_by = csv_values(*value);
        }
        if (const auto value = query.get("last_modified_by")) {
            context_query.last_modified_by = csv_values(*value);
        }
        if (const auto value = query.get("plaintext")) {
            context_query.plaintext = *value;
        }
        if (const auto value = query.get("dimension_match_strategy")) {
            context_query.dimension_match_strategy = *value;
        }

        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto dimensions = query_dimensions(req);
        if (const auto scope_error = validate_scope(dimensions, auth)) {
            send_error(res, send, *scope_error);
            return;
        }

        auto records = m_context->superposition().list_contexts();
        if (!dimensions.empty()) {
            const bool prefix_match = context_query.dimension_match_strategy == "prefix";
            std::vector<app::services::ContextRecord> scoped{};
            for (const auto& record : records) {
                bool keep{true};
                for (const auto& [name, value] : dimensions) {
                    (void)name;
                    const bool matches = prefix_match
                                            ? record.get_name().starts_with(value)
                                            : record.get_name() == value;
                    if (!matches) {
                        keep = false;
                        break;
                    }
                }
                if (keep) {
                    scoped.push_back(record);
                }
            }
            records = std::move(scoped);
        }
        if (!context_query.prefixes.empty()) {
            std::vector<app::services::ContextRecord> prefixed{};
            for (const auto& record : records) {
                if (matches_any(context_query.prefixes, record.get_name(), true)) {
                    prefixed.push_back(record);
                }
            }
            records = std::move(prefixed);
        }
        if (!context_query.created_by.empty() || !context_query.last_modified_by.empty()) {
            std::vector<app::services::ContextRecord> owned{};
            for (const auto& record : records) {
                const auto creator = context_meta(record.get_name(), "created_by");
                const auto modifier = context_meta(record.get_name(), "last_modified_by");
                if (!context_query.created_by.empty() &&
                    (!creator || !matches_any(context_query.created_by, *creator, false))) {
                    continue;
                }
                if (!context_query.last_modified_by.empty() &&
                    (!modifier || !matches_any(context_query.last_modified_by, *modifier, false))) {
                    continue;
                }
                owned.push_back(record);
            }
            records = std::move(owned);
        }
        sort_contexts(records, context_query.sort_on, context_query.sort_by);

        std::vector<std::string> data{};
        const auto [begin, end] = slice_bounds(records.size(), context_query.pagination);
        for (std::size_t index{begin}; index < end; ++index) {
            const auto& record = records[index];
            app::dto::superposition_proxy::ContextResponse item{};
            item.set_id(record.get_name());
            item.set_value(to_value(record.get_value()));
            item.set_weight(std::to_string(record.get_weight()));
            if (const auto description = context_meta(record.get_name(), "description")) {
                item.set_description(*description);
            }
            if (const auto reason = context_meta(record.get_name(), "change_reason")) {
                item.set_change_reason(*reason);
            }
            if (const auto creator = context_meta(record.get_name(), "created_by")) {
                item.set_created_by(*creator);
            }
            if (const auto modifier = context_meta(record.get_name(), "last_modified_by")) {
                item.set_last_modified_by(*modifier);
            }
            data.push_back(serde::Ser::encode_json(item));
        }

        app::dto::superposition_proxy::PaginatedListResponse response;
        response.set_total_items(static_cast<std::int32_t>(records.size()));
        response.set_total_pages(total_pages(records.size(), context_query.pagination));
        response.set_data(std::move(data));
        track_flow("SuperpositionListContexts", "superposition.list_contexts");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_list_default_configs(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_request(req, "list_default_configs")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto scope = require_proxy_scope(req, res, send);
        if (!scope) {
            return;
        }
        cache_scope(req, *scope);

        // ListDefaultConfigsQuery: count, page, all, name. A name filter narrows to one
        // SDK get; otherwise keys are recovered from the SDK audit log (local ConfigStore
        // has no list() on SuperpositionClient, see shared-file follow-up).
        const auto query = app::http::RequestQuery{}(req);
        const auto paging = pagination(query);
        const auto name = query.get("name");

        std::vector<app::services::ConfigRecord> records{};
        if (name && !name->empty()) {
            if (const auto record = m_context->superposition().get_config(default_config_ns(), *name)) {
                records.push_back(*record);
            }
        } else {
            for (const auto& entry : m_context->superposition().audit_logs()) {
                if (entry.get_config_namespace() != default_config_ns() ||
                    entry.get_action() != "put") {
                    continue;
                }
                if (const auto record = m_context->superposition().get_config(
                        default_config_ns(), entry.get_config_key()
                    )) {
                    records.push_back(*record);
                }
            }
        }

        std::vector<std::string> data{};
        const auto [begin, end] = slice_bounds(records.size(), paging);
        for (std::size_t index{begin}; index < end; ++index) {
            data.push_back(serde::Ser::encode_json(default_config_to_dto(records[index])));
        }

        app::dto::superposition_proxy::PaginatedListResponse response;
        response.set_total_items(static_cast<std::int32_t>(records.size()));
        response.set_total_pages(total_pages(records.size(), paging));
        response.set_data(std::move(data));
        track_flow("SuperpositionListDefaultConfigs", "superposition.list_default_configs");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_list_dimensions(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_request(req, "list_dimensions")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto scope = require_proxy_scope(req, res, send);
        if (!scope) {
            return;
        }
        cache_scope(req, *scope);

        const auto query = app::http::RequestQuery{}(req);
        const auto paging = pagination(query);
        const auto records = m_context->config_store().list(dimension_ns());

        std::vector<std::string> data{};
        const auto [begin, end] = slice_bounds(records.size(), paging);
        for (std::size_t index{begin}; index < end; ++index) {
            data.push_back(serde::Ser::encode_json(dimension_to_dto(records[index])));
        }

        app::dto::superposition_proxy::PaginatedListResponse response;
        response.set_total_items(static_cast<std::int32_t>(records.size()));
        response.set_total_pages(total_pages(records.size(), paging));
        response.set_data(std::move(data));
        track_flow("SuperpositionListDimensions", "superposition.list_dimensions");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_get_dimension(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_request(req, "get_dimension")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto scope = require_proxy_scope(req, res, send);
        if (!scope) {
            return;
        }
        cache_scope(req, *scope);

        const auto dimension = last_path_segment(req);
        if (dimension.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "dimension name missing in path"));
            return;
        }
        const auto record = m_context->config_store().get(dimension_ns(), dimension);
        if (!record) {
            send_error(res, send, map_sdk_error("NotFound", "Failed to get dimension from Superposition"));
            return;
        }
        if (!is_json_document(record->get_value())) {
            send_error(res, send, map_sdk_error("BadRequest", "Invalid dimension document from Superposition"));
            return;
        }

        track_flow("SuperpositionGetDimension", "superposition.get_dimension");
        send_json(res, send, serde::Ser::encode_json(dimension_to_dto(*record)), true);
    }

    void handle_get_default_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_request(req, "get_default_config")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto scope = require_proxy_scope(req, res, send);
        if (!scope) {
            return;
        }
        cache_scope(req, *scope);

        const auto key = last_path_segment(req);
        if (key.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "config key missing in path"));
            return;
        }
        const auto config = m_context->superposition().get_config(default_config_ns(), key);
        if (!config) {
            send_error(res, send, map_sdk_error("NotFound", "Failed to get default config from Superposition"));
            return;
        }
        if (!is_json_document(config->get_value())) {
            send_error(res, send, map_sdk_error("BadRequest", "Invalid default config document from Superposition"));
            return;
        }

        track_flow("SuperpositionGetDefaultConfig", "superposition.get_default_config");
        send_json(res, send, serde::Ser::encode_json(default_config_to_dto(*config)), true);
    }

    void handle_create_context(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_request(req, "create_context")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto scope = require_proxy_scope(req, res, send);
        if (!scope) {
            return;
        }
        cache_scope(req, *scope);

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // context_put_from_request normalization: the body's context map is converted to a
        // JSON document used for scoped validation and for the local context record.
        const auto put = context_put_from_request(*parsed);
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto scope_error = validate_scope(put.dimensions, auth)) {
            send_error(res, send, *scope_error);
            return;
        }
        if (put.dimensions.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::VALIDATION_FAILED, "context dimensions are required"));
            return;
        }

        const auto merchant_id = auth.merchant_id();
        if (!merchant_id.empty() &&
            !m_context->repository().find_merchant_account_by_string(merchant_id).has_value()) {
            send_error(res, send, error(core::error::AppErrorKind::FORBIDDEN, "merchant account not found"));
            return;
        }
        auto created_by = std::string{app::auth::TokenParser{}(req).value()};
        if (const auto sub = auth.claim("sub"); sub && !sub->empty()) {
            created_by = std::string{*sub};
        }

        // No upstream SDK is linked: the SDK-owned id is substituted with a deterministic
        // local identifier and the record is persisted through the injected client.
        const auto id = m_context->ids().payment_id("ctx_");
        app::services::ContextRecord context{};
        context.set_name(id);
        context.set_value(put.context_json);
        context.set_weight(1);
        m_context->superposition().add_context(context);
        m_context->superposition().put_config(default_config_ns(), id, put.context_json, id);
        m_context->config_store().put(context_meta_ns(), id + ".created_by", created_by);
        m_context->config_store().put(context_meta_ns(), id + ".last_modified_by", created_by);
        if (!put.description.empty()) {
            m_context->config_store().put(context_meta_ns(), id + ".description", put.description);
        }
        if (!put.change_reason.empty()) {
            m_context->config_store().put(context_meta_ns(), id + ".change_reason", put.change_reason);
        }

        app::dto::superposition_proxy::ContextResponse response;
        response.set_id(id);
        response.set_value(to_value(put.context_json));
        response.set_weight("1");
        response.set_description(put.description);
        response.set_change_reason(put.change_reason);
        response.set_created_by(created_by);
        response.set_last_modified_by(created_by);
        track_flow("SuperpositionCreateContext", "superposition.create_context");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_resolve_detailed_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_request(req, "resolve_detailed_config")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto scope = require_proxy_scope(req, res, send);
        if (!scope) {
            return;
        }
        cache_scope(req, *scope);

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto entries = context_entries(*parsed);
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto scope_error = validate_scope(scoping_dimensions(entries), auth)) {
            send_error(res, send, *scope_error);
            return;
        }
        if (entries.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::VALIDATION_FAILED, "context dimensions are required"));
            return;
        }
        const auto context = context_document(entries);

        std::unordered_map<std::string, app::dto::superposition_proxy::ResolvedConfigEntry> resolved{};
        const auto keys = default_config_keys();
        if (!keys.empty()) {
            for (const auto& key : keys) {
                const auto value = m_context->superposition().resolve(default_config_ns(), key, context);
                auto decoded = serde::Ser::decode_generic(
                    "application/json", value ? value->get_value() : std::string{"null"}
                );
                if (!decoded) {
                    send_error(res, send, map_sdk_error("ClientError", "Failed to parse resolved config from Superposition"));
                    return;
                }
                app::dto::superposition_proxy::ResolvedConfigEntry item{};
                item.set_value(std::move(*decoded));
                if (value) {
                    item.set_description(value->get_context());
                }
                resolved.emplace(key, std::move(item));
            }
        } else {
            // No stored default config: resolve each supplied context key so the call still
            // returns a deterministic, service-derived document rather than a stub.
            for (const auto& [key, value] : entries) {
                const auto found = m_context->superposition().resolve(default_config_ns(), key, value);
                auto decoded = serde::Ser::decode_generic(
                    "application/json", found ? found->get_value() : json_quote(value)
                );
                if (!decoded) {
                    send_error(res, send, map_sdk_error("ClientError", "Failed to parse resolved config from Superposition"));
                    return;
                }
                app::dto::superposition_proxy::ResolvedConfigEntry item{};
                item.set_value(std::move(*decoded));
                resolved.emplace(key, std::move(item));
            }
        }

        app::dto::superposition_proxy::ResolveConfigResponse response;
        response.set_value(std::move(resolved));
        track_flow("SuperpositionResolveDetailedConfig", "superposition.resolve_detailed_config");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_resolve_config_explanation(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_request(req, "resolve_config_explanation")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto scope = require_proxy_scope(req, res, send);
        if (!scope) {
            return;
        }
        cache_scope(req, *scope);

        const auto key = last_path_segment(req);
        if (key.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "config key missing in path"));
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        const auto entries = context_entries(*parsed);
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto scope_error = validate_scope(scoping_dimensions(entries), auth)) {
            send_error(res, send, *scope_error);
            return;
        }
        if (entries.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::VALIDATION_FAILED, "context dimensions are required"));
            return;
        }
        const auto context = context_document(entries);

        std::vector<app::dto::superposition_proxy::ResolveExplanationEntry> timeline{};
        for (const auto& log : m_context->superposition().audit_logs()) {
            if (log.get_config_key() != key) {
                continue;
            }
            const auto before = m_context->superposition().get_config(default_config_ns(), key);
            const auto after = m_context->superposition().resolve(default_config_ns(), key, context);
            app::dto::superposition_proxy::ResolveExplanationEntry item{};
            item.set_context_id(log.get_config_namespace());
            item.set_condition(to_value(context));
            item.set_override_id(std::to_string(log.get_id()));
            item.set_value_before(to_value(before ? before->get_value() : std::string{"null"}));
            item.set_value_after(to_value(after ? after->get_value() : std::string{"null"}));
            timeline.push_back(std::move(item));
        }

        app::dto::superposition_proxy::ResolveConfigExplanationResponse response;
        response.set_key(key);
        response.set_timeline(std::move(timeline));
        track_flow("SuperpositionResolveConfigExplanation", "superposition.resolve_config_explanation");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_list_audit_logs(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_request(req, "list_audit_logs")) {
            send_error(res, send, *auth_error);
            return;
        }
        const auto scope = require_proxy_scope(req, res, send);
        if (!scope) {
            return;
        }
        cache_scope(req, *scope);

        // Full typed ListAuditLogsQuery: CSV table/action, lenient dates, username, sort.
        const auto query = app::http::RequestQuery{}(req);
        AuditQuery audit_query{};
        audit_query.pagination = pagination(query);
        if (const auto value = query.get("from_date")) {
            audit_query.from_date = parse_date_lenient(*value).value_or("");
        }
        if (const auto value = query.get("to_date")) {
            audit_query.to_date = parse_date_lenient(*value).value_or("");
        }
        if (const auto value = query.get("table")) {
            audit_query.tables = csv_values(*value);
        }
        if (const auto value = query.get("action")) {
            audit_query.actions = csv_values(*value);
        }
        if (const auto value = query.get("username")) {
            audit_query.username = *value;
        }
        if (const auto value = query.get("sort_by")) {
            audit_query.sort_by = *value;
        }

        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto scope_error = validate_scope(query_dimensions(req), auth)) {
            send_error(res, send, *scope_error);
            return;
        }

        auto logs = m_context->superposition().audit_logs();
        std::vector<app::services::AuditLogRecord> filtered{};
        for (const auto& log : logs) {
            if (!audit_query.tables.empty() &&
                !matches_any(audit_query.tables, log.get_config_namespace(), false)) {
                continue;
            }
            if (!audit_query.actions.empty() &&
                !matches_any(audit_query.actions, log.get_action(), false)) {
                continue;
            }
            const auto username = audit_meta(log.get_id(), "username");
            if (!audit_query.username.empty() &&
                (!username || *username != audit_query.username)) {
                continue;
            }
            const auto timestamp = audit_meta(log.get_id(), "timestamp");
            if (!audit_query.from_date.empty() && (!timestamp || *timestamp < audit_query.from_date)) {
                continue;
            }
            if (!audit_query.to_date.empty() && (!timestamp || *timestamp > audit_query.to_date)) {
                continue;
            }
            filtered.push_back(log);
        }
        if (audit_query.sort_by == "desc") {
            std::ranges::sort(filtered, {}, &app::services::AuditLogRecord::get_sequence);
            std::ranges::reverse(filtered);
        } else if (audit_query.sort_by == "asc") {
            std::ranges::sort(filtered, {}, &app::services::AuditLogRecord::get_sequence);
        }

        std::vector<std::string> data{};
        const auto [begin, end] = slice_bounds(filtered.size(), audit_query.pagination);
        for (std::size_t index{begin}; index < end; ++index) {
            data.push_back(serde::Ser::encode_json(audit_log_to_dto(filtered[index])));
        }

        app::dto::superposition_proxy::PaginatedListResponse response;
        response.set_total_items(static_cast<std::int32_t>(filtered.size()));
        response.set_total_pages(total_pages(filtered.size(), audit_query.pagination));
        response.set_data(std::move(data));
        track_flow("SuperpositionListAuditLogs", "superposition.list_audit_logs");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/v1/superposition"}
            .add_route(
                Route{"/context"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_contexts(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_contexts")
            )
            .add_route(
                Route{"/context"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_context(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_context")
            )
            .add_route(
                Route{"/default-config"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_default_configs(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_default_configs")
            )
            .add_route(
                Route{"/default-config/:key"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_default_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_default_config")
            )
            .add_route(
                Route{"/dimension"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_dimensions(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_dimensions")
            )
            .add_route(
                Route{"/dimension/:dimension_name"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_dimension(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_dimension")
            )
            .add_route(
                Route{"/config/resolve/detailed"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_resolve_detailed_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch resolve_detailed_config")
            )
            .add_route(
                Route{"/config/resolve/explain/:key"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_resolve_config_explanation(req, res, std::move(send));
                    })
                    .summary("Hyperswitch resolve_config_explanation")
            )
            .add_route(
                Route{"/audit"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_audit_logs(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_audit_logs")
            );
    }

private:
    struct ProxyScope
    {
        std::string org_id{};
        std::string workspace_id{};
    };

    struct Pagination
    {
        std::int32_t count{20};
        std::int32_t page{1};
        bool all{false};
    };

    struct ContextQuery
    {
        Pagination pagination{};
        std::vector<std::string> prefixes{};
        std::string sort_on{};
        std::string sort_by{};
        std::vector<std::string> created_by{};
        std::vector<std::string> last_modified_by{};
        std::string plaintext{};
        std::string dimension_match_strategy{};
    };

    struct AuditQuery
    {
        Pagination pagination{};
        std::string from_date{};
        std::string to_date{};
        std::vector<std::string> tables{};
        std::vector<std::string> actions{};
        std::string username{};
        std::string sort_by{};
    };

    struct ContextPut
    {
        std::vector<std::pair<std::string, std::string>> dimensions{};
        std::string description{};
        std::string change_reason{};
        std::string context_json{};
    };

    [[nodiscard]] std::string_view default_config_ns() const noexcept
    {
        return "superposition.default_config";
    }

    [[nodiscard]] std::string_view dimension_ns() const noexcept
    {
        return "superposition.dimension";
    }

    [[nodiscard]] std::string_view context_meta_ns() const noexcept
    {
        return "superposition.context_meta";
    }

    [[nodiscard]] std::string_view audit_meta_ns() const noexcept
    {
        return "superposition.audit_meta";
    }

    [[nodiscard]] std::string_view proxy_config_ns() const noexcept
    {
        return "superposition_proxy";
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize_request(
        interfaces::io::IRequest& req,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        return m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
    }

    [[nodiscard]] std::optional<ProxyScope> require_proxy_scope(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()>& send
    ) const
    {
        // Upstream extract_proxy_headers reads x-org-id/x-workspace and compares both to
        // configured_org_id()/configured_workspace_id(). Config records are the local source
        // for configured values; an unset value means "no configured expectation".
        const auto org_id = app::http::org_id_from_header{}(req);
        if (org_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "missing required header: x-org-id"));
            return std::nullopt;
        }
        const auto workspace_id = app::http::HeaderReader{}.get(req, "x-workspace");
        if (workspace_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "missing required header: x-workspace"));
            return std::nullopt;
        }
        const auto configured_org = m_context->config_store().get_value(proxy_config_ns(), "configured_org_id");
        const auto configured_workspace = m_context->config_store().get_value(proxy_config_ns(), "configured_workspace_id");
        if ((configured_org && *configured_org != org_id) ||
            (configured_workspace && *configured_workspace != workspace_id)) {
            send_error(res, send, error(core::error::AppErrorKind::FORBIDDEN, "access forbidden: superposition org and workspace"));
            return std::nullopt;
        }
        return ProxyScope{org_id, workspace_id};
    }

    void cache_scope(interfaces::io::IRequest& req, const ProxyScope& scope) const
    {
        // Per-request scope digest is ephemeral TTL state, never a business record.
        const auto token = app::auth::TokenParser{}(req).value();
        const auto digest = app::crypto::KeyedHasher{"superposition_proxy.scope"}.hash(
            scope.org_id + ":" + scope.workspace_id + ":" + token
        );
        const auto key = "superposition_proxy.scope." + digest;
        if (!m_context->cache().get(key).hit) {
            m_context->cache().put(
                key, core::cache::to_cache_value(scope.org_id + ":" + scope.workspace_id)
            );
        }
    }

    [[nodiscard]] std::string last_path_segment(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        if (segments.count() == 0U) {
            return {};
        }
        return std::string{segments.at(segments.count() - 1U).value_or("")};
    }

    [[nodiscard]] bool is_known_dimension(std::string_view key) const noexcept
    {
        return key == "organization_id" || key == "merchant_id" || key == "profile_id" ||
               key == "provider_merchant_id" || key == "processor_merchant_id";
    }

    [[nodiscard]] std::vector<std::pair<std::string, std::string>> query_dimensions(
        interfaces::io::IRequest& req
    ) const
    {
        // app::http::QueryParams exposes get() only; dimension[...] keys need enumeration,
        // so the raw query pairs are scanned once here. The match is strict: prefix, closing
        // bracket and a known inner name all required (ScopingDimension::from_dimension_param).
        std::vector<std::pair<std::string, std::string>> dimensions;
        const auto path = std::string{req.get_path()};
        const auto pos = path.find('?');
        if (pos == std::string::npos) {
            return dimensions;
        }
        const auto query = std::string_view{path}.substr(pos + 1);
        std::size_t start{0};
        while (start < query.size()) {
            const auto amp = query.find('&', start);
            const auto end = amp == std::string_view::npos ? query.size() : amp;
            const auto pair = query.substr(start, end - start);
            const auto eq = pair.find('=');
            if (eq != std::string_view::npos) {
                const auto key = pair.substr(0, eq);
                const auto scoped = scoped_name(key);
                if (key.starts_with("dimension[") && key.ends_with(']') &&
                    scoped && is_known_dimension(*scoped)) {
                    dimensions.emplace_back(
                        std::string{key}, std::string{pair.substr(eq + 1)}
                    );
                }
            }
            if (amp == std::string_view::npos) {
                break;
            }
            start = amp + 1;
        }
        return dimensions;
    }

    [[nodiscard]] std::optional<std::string_view> scoped_name(std::string_view key) const noexcept
    {
        if (key.starts_with("dimension[") && key.ends_with(']') && key.size() > 11U) {
            return key.substr(10U, key.size() - 11U);
        }
        if (is_known_dimension(key)) {
            return key;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::string> scalar_text(const serde::Value& value) const
    {
        if (const auto* text = std::get_if<std::string>(&value.get())) {
            return *text;
        }
        if (const auto* number = std::get_if<std::int64_t>(&value.get())) {
            return std::to_string(*number);
        }
        if (const auto* flag = std::get_if<bool>(&value.get())) {
            return *flag ? std::string{"true"} : std::string{"false"};
        }
        return std::nullopt;
    }

    [[nodiscard]] ContextPut context_put_from_request(const serde::Value& root) const
    {
        ContextPut put{};
        const auto* object = std::get_if<interfaces::Value::Object>(&root.get());
        if (object == nullptr) {
            return put;
        }
        const auto context = object->get("context");
        if (context) {
            if (const auto* context_object = std::get_if<interfaces::Value::Object>(&context.value().get())) {
                std::string json{"{"};
                bool first{true};
                for (const auto& [key, value] : *context_object) {
                    const auto text = scalar_text(value);
                    if (!text) {
                        continue;
                    }
                    if (!first) {
                        json.push_back(',');
                    }
                    first = false;
                    json += json_quote(key);
                    json.push_back(':');
                    json += json_quote(*text);
                    if (is_known_dimension(key)) {
                        put.dimensions.emplace_back(key, *text);
                    }
                }
                json.push_back('}');
                put.context_json = std::move(json);
            }
        }
        if (const auto description = object->get("description")) {
            if (const auto text = scalar_text(description.value())) {
                put.description = *text;
            }
        }
        if (const auto reason = object->get("change_reason")) {
            if (const auto text = scalar_text(reason.value())) {
                put.change_reason = *text;
            }
        }
        return put;
    }

    [[nodiscard]] std::vector<std::pair<std::string, std::string>> context_entries(
        const serde::Value& root
    ) const
    {
        std::vector<std::pair<std::string, std::string>> entries{};
        const auto* object = std::get_if<interfaces::Value::Object>(&root.get());
        if (object == nullptr) {
            return entries;
        }
        const auto context = object->get("context");
        if (!context) {
            return entries;
        }
        const auto* context_object = std::get_if<interfaces::Value::Object>(&context.value().get());
        if (context_object == nullptr) {
            return entries;
        }
        for (const auto& [key, value] : *context_object) {
            if (const auto text = scalar_text(value)) {
                entries.emplace_back(key, *text);
            }
        }
        return entries;
    }

    [[nodiscard]] std::vector<std::pair<std::string, std::string>> scoping_dimensions(
        const std::vector<std::pair<std::string, std::string>>& entries
    ) const
    {
        std::vector<std::pair<std::string, std::string>> dimensions{};
        for (const auto& entry : entries) {
            if (is_known_dimension(entry.first)) {
                dimensions.push_back(entry);
            }
        }
        return dimensions;
    }

    [[nodiscard]] std::string context_document(
        const std::vector<std::pair<std::string, std::string>>& entries
    ) const
    {
        std::string json{"{"};
        bool first{true};
        for (const auto& [key, value] : entries) {
            if (!first) {
                json.push_back(',');
            }
            first = false;
            json += json_quote(key);
            json.push_back(':');
            json += json_quote(value);
        }
        json.push_back('}');
        return json;
    }

    [[nodiscard]] std::optional<core::error::AppError> validate_scope(
        const std::vector<std::pair<std::string, std::string>>& dimensions,
        const app::auth::AuthContext& auth
    ) const
    {
        const std::array<std::string_view, 4> required{
            "organization_id", "profile_id",
            "provider_merchant_id", "processor_merchant_id"
        };
        std::vector<std::string> missing;
        for (const auto required_key : required) {
            bool present{false};
            for (const auto& entry : dimensions) {
                const auto name = scoped_name(entry.first);
                if (name && *name == required_key) {
                    present = true;
                    break;
                }
            }
            if (!present) {
                missing.emplace_back(required_key);
            }
        }
        if (!missing.empty()) {
            std::string message{"scope is missing: "};
            for (std::size_t i{0}; i < missing.size(); ++i) {
                if (i > 0U) {
                    message += ", ";
                }
                message += missing[i];
            }
            return error(core::error::AppErrorKind::INVALID_REQUEST, std::move(message));
        }

        for (const auto& [name, value] : dimensions) {
            const auto scoped = scoped_name(name);
            if (!scoped) {
                continue;
            }
            std::string expected;
            if (*scoped == "organization_id") {
                expected = auth.org_id();
            } else if (*scoped == "profile_id") {
                expected = auth.profile_id();
            } else {
                expected = auth.merchant_id();
            }
            if (!expected.empty() && value != expected) {
                return error(core::error::AppErrorKind::FORBIDDEN, "access forbidden: superposition");
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::vector<std::string> default_config_keys() const
    {
        std::vector<std::string> keys{};
        for (const auto& log : m_context->superposition().audit_logs()) {
            if (log.get_config_namespace() != default_config_ns() || log.get_action() != "put") {
                continue;
            }
            if (std::ranges::find(keys, log.get_config_key()) == keys.end()) {
                keys.push_back(log.get_config_key());
            }
        }
        return keys;
    }

    [[nodiscard]] std::optional<std::string> context_meta(
        std::string_view name,
        std::string_view key
    ) const
    {
        return m_context->config_store().get_value(
            context_meta_ns(), std::string{name} + "." + std::string{key}
        );
    }

    [[nodiscard]] std::optional<std::string> audit_meta(
        std::int64_t id,
        std::string_view key
    ) const
    {
        return m_context->config_store().get_value(
            audit_meta_ns(), std::to_string(id) + "." + std::string{key}
        );
    }

    [[nodiscard]] app::dto::superposition_proxy::DefaultConfigResponse
    default_config_to_dto(const app::services::ConfigRecord& record) const
    {
        app::dto::superposition_proxy::DefaultConfigResponse dto{};
        dto.set_key(record.get_config_key());
        dto.set_value(to_value(record.get_value()));
        dto.set_description(record.get_context());
        dto.set_created_by(record.get_context());
        dto.set_last_modified_by(record.get_context());
        return dto;
    }

    [[nodiscard]] app::dto::superposition_proxy::DimensionResponse
    dimension_to_dto(const app::services::ConfigRecord& record) const
    {
        app::dto::superposition_proxy::DimensionResponse dto{};
        dto.set_dimension(record.get_config_key());
        dto.set_position(static_cast<std::int32_t>(record.get_sequence()));
        dto.set_schema(to_value(record.get_value()));
        dto.set_description(record.get_context());
        dto.set_created_by(record.get_context());
        dto.set_last_modified_by(record.get_context());
        return dto;
    }

    [[nodiscard]] app::dto::superposition_proxy::AuditLogResponse
    audit_log_to_dto(const app::services::AuditLogRecord& log) const
    {
        app::dto::superposition_proxy::AuditLogResponse dto{};
        dto.set_id(std::to_string(log.get_id()));
        dto.set_table_name(log.get_config_namespace());
        dto.set_action(log.get_action());
        dto.set_query(log.get_detail());
        if (const auto username = audit_meta(log.get_id(), "username")) {
            dto.set_user_name(*username);
        }
        if (const auto timestamp = audit_meta(log.get_id(), "timestamp")) {
            dto.set_timestamp(*timestamp);
        }
        auto decoded = serde::Ser::decode_generic("application/json", log.get_detail());
        if (decoded) {
            dto.set_new_data(std::move(*decoded));
        }
        return dto;
    }

    void sort_contexts(
        std::vector<app::services::ContextRecord>& records,
        std::string_view sort_on,
        std::string_view sort_by
    ) const
    {
        if (sort_on != "name" && sort_on != "value" && sort_on != "weight") {
            return;
        }
        const bool descending = sort_by == "desc";
        std::ranges::sort(records, [sort_on, descending](
            const app::services::ContextRecord& lhs,
            const app::services::ContextRecord& rhs
        ) {
            if (sort_on == "name") {
                return descending ? lhs.get_name() > rhs.get_name()
                                  : lhs.get_name() < rhs.get_name();
            }
            if (sort_on == "value") {
                return descending ? lhs.get_value() > rhs.get_value()
                                  : lhs.get_value() < rhs.get_value();
            }
            return descending ? lhs.get_weight() > rhs.get_weight()
                              : lhs.get_weight() < rhs.get_weight();
        });
    }

    [[nodiscard]] bool matches_any(
        const std::vector<std::string>& values,
        std::string_view candidate,
        bool prefix
    ) const
    {
        for (const auto& value : values) {
            if (prefix ? candidate.starts_with(value) : candidate == value) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] Pagination pagination(const app::http::QueryParams& query) const
    {
        Pagination result{};
        if (const auto value = query.get("count")) {
            result.count = to_int(*value);
        }
        if (const auto value = query.get("page")) {
            result.page = to_int(*value);
        }
        if (const auto value = query.get("all")) {
            result.all = to_bool(*value);
        }
        if (result.count <= 0) {
            result.count = 20;
        }
        if (result.page <= 0) {
            result.page = 1;
        }
        return result;
    }

    [[nodiscard]] std::pair<std::size_t, std::size_t> slice_bounds(
        std::size_t total,
        const Pagination& paging
    ) const noexcept
    {
        if (paging.all || total == 0U) {
            return {0U, total};
        }
        const auto limit = static_cast<std::size_t>(paging.count);
        const auto start = (static_cast<std::size_t>(paging.page) - 1U) * limit;
        if (start >= total) {
            return {total, total};
        }
        return {start, std::min(total, start + limit)};
    }

    [[nodiscard]] std::int32_t total_pages(std::size_t total, const Pagination& paging) const noexcept
    {
        if (total == 0U) {
            return 0;
        }
        if (paging.all) {
            return 1;
        }
        const auto limit = static_cast<std::size_t>(paging.count);
        return static_cast<std::int32_t>((total + limit - 1U) / limit);
    }

    [[nodiscard]] std::vector<std::string> csv_values(const std::optional<std::string>& value) const
    {
        std::vector<std::string> values{};
        if (!value) {
            return values;
        }
        std::size_t start{0};
        while (start <= value->size()) {
            const auto comma = value->find(',', start);
            const auto end = comma == std::string::npos ? value->size() : comma;
            const auto token = value->substr(start, end - start);
            const auto first = token.find_first_not_of(" \t");
            if (first != std::string::npos) {
                const auto last = token.find_last_not_of(" \t");
                values.push_back(token.substr(first, last - first + 1U));
            }
            if (comma == std::string::npos) {
                break;
            }
            start = comma + 1U;
        }
        return values;
    }

    [[nodiscard]] std::optional<std::string> parse_date_lenient(std::string_view value) const
    {
        // Upstream parse_datetime is lenient: malformed values are ignored, not rejected.
        // Local records have no timestamp type, so an ISO-shaped string is the substitute.
        if (value.empty() || value.find('-') == std::string_view::npos) {
            return std::nullopt;
        }
        return std::string{value};
    }

    [[nodiscard]] std::string json_quote(std::string_view text) const
    {
        std::string out{"\""};
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
        out.push_back('"');
        return out;
    }

    [[nodiscard]] bool is_json_document(std::string_view json) const
    {
        return serde::Ser::decode_generic("application/json", json).has_value();
    }

    [[nodiscard]] serde::Value to_value(std::string_view json) const
    {
        auto decoded = serde::Ser::decode_generic("application/json", json);
        if (decoded) {
            return std::move(*decoded);
        }
        return serde::Value{};
    }

    [[nodiscard]] std::int32_t to_int(const std::optional<std::string>& value) const
    {
        if (!value) {
            return 0;
        }
        try {
            return std::stoi(*value);
        } catch (...) {
            return 0;
        }
    }

    [[nodiscard]] bool to_bool(const std::optional<std::string>& value) const
    {
        if (!value) {
            return false;
        }
        return *value == "true" || *value == "1" || *value == "yes";
    }

    [[nodiscard]] core::error::AppError error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    // map_superposition_err: BadRequest -> INVALID_REQUEST, NotFound -> the documented
    // org/workspace message, everything else -> INTERNAL_ERROR with printable context.
    [[nodiscard]] core::error::AppError map_sdk_error(
        std::string_view sdk_kind,
        std::string_view context
    ) const
    {
        if (sdk_kind == "BadRequest") {
            return error(core::error::AppErrorKind::INVALID_REQUEST, std::string{context});
        }
        if (sdk_kind == "NotFound") {
            return error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "The specified org_id or workspace_id was not found in Superposition"
            );
        }
        return error(core::error::AppErrorKind::INTERNAL_ERROR, std::string{context});
    }

    void track_flow(std::string_view flow, std::string_view counter) const
    {
        m_context->metrics().record_flow(flow);
        m_context->metrics().counter(counter).increment();
        record_metric(flow, counter);
    }

    // ApiEventMetric analogue: upstream instruments every response type via ApiEventMetric
    // and ApplicationResponse::Json. The local substitute is a paired named flow/counter.
    void record_metric(std::string_view flow, std::string_view counter) const
    {
        m_context->metrics().record_flow(std::string{"api_event."} + std::string{flow});
        m_context->metrics().counter(std::string{"api_event."} + std::string{counter}).increment();
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

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::superposition_proxy
