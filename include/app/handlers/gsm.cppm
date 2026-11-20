module;
export module app_handlers:gsm;

import std;
import interfaces;
import core_router;
import core_serde;
import core_error;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import app_store;
import app_events;

export namespace app::handlers::gsm {

class GsmRouteHandlers
{
public:
    void handle_create_gsm_rule(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_request(req, "create_gsm_rule")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = app::dto::gsm::GsmCreateRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto& in = *typed;
        const auto key = rule_key(
            in.get_connector().get_value(), in.get_flow(), in.get_sub_flow(),
            in.get_code(), in.get_message()
        );

        auto repo = m_context->repository();
        // Upstream: TrackedGsmInterface::add_gsm_rule + to_duplicate_response
        // (core/gsm.rs:18-23). Enforce the natural-key duplicate before insert,
        // then rely on the DB unique index as the backstop.
        if (repo.find_gsm_rule(key)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::ALREADY_EXISTS,
                "GSM with given key already exists in our records"
            ));
            return;
        }

        auto response = response_from_create(in);
        auto created = repo.create_gsm_rule(record_from(key, response));
        if (!created) {
            send_error(res, send, created.error());
            return;
        }

        // Upstream maps the stored domain record back to GsmResponse via
        // foreign_into (core/gsm.rs:24). Round-trip through the persisted record
        // instead of echoing the request.
        const auto persisted = response_from_record(*created);
        const auto text = serde::Ser::encode_json(persisted ? *persisted : response);

        m_context->events().publish("gsm.rule.created", audit_payload("created", key));
        m_context->metrics().record_flow("GsmRuleCreate");
        m_context->metrics().counter("GSM_RULE_CREATED").increment();
        send_json(res, send, text, true);
    }

    void handle_get_gsm_rule(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_request(req, "get_gsm_rule")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = app::dto::gsm::GsmRetrieveRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto& in = *typed;
        const auto key = rule_key(
            in.get_connector().get_value(), in.get_flow(), in.get_sub_flow(),
            in.get_code(), in.get_message()
        );

        auto repo = m_context->repository();
        // Upstream TrackedGsmInterface::find_gsm_rule + to_not_found_response
        // (core/gsm.rs:32-45).
        const auto stored = repo.find_gsm_rule(key);
        if (!stored) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND,
                "GSM with given key does not exist in our records"
            ));
            return;
        }

        const auto response = response_from_record(*stored);
        if (!response) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INTERNAL,
                "stored GSM rule could not be decoded"
            ));
            return;
        }

        const auto text = serde::Ser::encode_json(*response);
        m_context->events().publish("gsm.rule.retrieved", audit_payload("retrieved", key));
        m_context->metrics().record_flow("GsmRuleRetrieve");
        m_context->metrics().counter("GSM_RULE_RETRIEVED").increment();
        send_json(res, send, text, true);
    }

    void handle_update_gsm_rule(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_request(req, "update_gsm_rule")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = app::dto::gsm::GsmUpdateRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto& in = *typed;
        const auto key = rule_key(in.get_connector(), in.get_flow(), in.get_sub_flow(), in.get_code(), in.get_message());

        auto repo = m_context->repository();
        // Upstream find_gsm_rule -> to_not_found_response (core/gsm.rs:60-71).
        const auto stored = repo.find_gsm_rule(key);
        if (!stored) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND,
                "GSM with given key does not exist in our records"
            ));
            return;
        }

        auto existing = response_from_record(*stored);
        if (!existing) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INTERNAL,
                "stored GSM rule could not be decoded"
            ));
            return;
        }

        auto response = std::move(*existing);

        // Local substitute for
        // <(GsmFeature, GsmFeatureData)>::foreign_from((&gsm_request, gsm_db_record))
        // (core/gsm.rs:73-76): infer from the persisted response when the update
        // omits feature/feature_data. GsmFeature has one variant (`retry`) and
        // GsmFeatureData has one variant (`retry` with a RetryFeatureData body).
        const auto inferred_feature = canonical_enum(response.get_feature().get_value(), "feature", "retry");
        auto inferred_feature_data_value = app::dto::gsm::GsmFeatureDataValue{};
        if (response.get_feature_data()) {
            inferred_feature_data_value.set_value(canonical_enum(response.get_feature_data()->get_value(), "feature_data"));
        } else {
            inferred_feature_data_value.set_value(retry_feature_data_json(
                response.get_step_up_possible(),
                response.get_clear_pan_possible(),
                canonical_enum(response.get_decision().get_value(), "decision", "do_default")
            ));
        }

        auto feature_value = app::dto::gsm::GsmFeatureValue{};
        if (in.get_feature()) {
            feature_value.set_value(canonical_enum(in.get_feature()->get_value(), "feature", inferred_feature));
        } else {
            feature_value.set_value(inferred_feature);
        }
        response.set_feature(feature_value);

        auto feature_data_value = app::dto::gsm::GsmFeatureDataValue{};
        std::optional<std::pair<std::optional<bool>, std::optional<bool>>> request_flags;
        if (in.get_feature_data()) {
            const auto canonical = canonical_enum(in.get_feature_data()->get_value(), "feature_data");
            feature_data_value.set_value(canonical);
            request_flags = retry_flags(canonical);
        } else {
            feature_data_value.set_value(inferred_feature_data_value.get_value());
        }
        response.set_feature_data(feature_data_value);

        // Upstream step_up_possible/clear_pan_possible derivation with `.or(fallback)`
        // (core/gsm.rs:109-121): retry flags from feature_data win, then the
        // explicit update bool, then the stored value is left untouched.
        bool step_from_flags = false;
        bool clear_from_flags = false;
        if (request_flags) {
            if (request_flags->first) {
                response.set_step_up_possible(*request_flags->first);
                step_from_flags = true;
            }
            if (request_flags->second) {
                response.set_clear_pan_possible(*request_flags->second);
                clear_from_flags = true;
            }
        }
        if (!step_from_flags && in.get_step_up_possible()) {
            response.set_step_up_possible(*in.get_step_up_possible());
        }
        if (!clear_from_flags && in.get_clear_pan_possible()) {
            response.set_clear_pan_possible(*in.get_clear_pan_possible());
        }

        if (in.get_status()) {
            response.set_status(*in.get_status());
        }
        if (in.get_router_error()) {
            response.set_router_error(in.get_router_error());
        }
        if (in.get_decision()) {
            app::dto::gsm::GsmDecisionValue decision;
            decision.set_value(canonical_enum(in.get_decision()->get_value(), "decision", "do_default"));
            response.set_decision(std::move(decision));
        }
        if (in.get_unified_code()) {
            response.set_unified_code(in.get_unified_code());
        }
        if (in.get_unified_message()) {
            response.set_unified_message(in.get_unified_message());
        }
        if (in.get_error_category()) {
            app::dto::gsm::ErrorCategoryValue category;
            category.set_value(canonical_enum(in.get_error_category()->get_value(), "error_category"));
            response.set_error_category(category);
        }
        if (in.get_standardised_code()) {
            app::dto::gsm::StandardisedCodeValue code;
            code.set_value(canonical_enum(in.get_standardised_code()->get_value(), "standardised_code"));
            response.set_standardised_code(code);
        }
        if (in.get_description()) {
            response.set_description(in.get_description());
        }
        if (in.get_user_guidance_message()) {
            response.set_user_guidance_message(in.get_user_guidance_message());
        }
        response.set_connector(canonical_enum(in.get_connector(), "connector"));
        response.set_flow(in.get_flow());
        response.set_sub_flow(in.get_sub_flow());
        response.set_code(in.get_code());
        response.set_message(in.get_message());

        auto updated = *stored;
        updated.set_decision(response.get_decision().get_value());
        updated.set_status(response.get_status());
        updated.set_program_json(serde::Ser::encode_json(response));
        // Upstream maps a failed update to not_found_response with an attached
        // "Failed while updating Gsm rule" message (core/gsm.rs:129-133).
        if (!repo.update_gsm_rule(updated)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND,
                "GSM with given key does not exist in our records"
            ));
            return;
        }

        const auto text = serde::Ser::encode_json(response);
        m_context->events().publish("gsm.rule.updated", audit_payload("updated", key));
        m_context->metrics().record_flow("GsmRuleUpdate");
        m_context->metrics().counter("GSM_RULE_UPDATED").increment();
        send_json(res, send, text, true);
    }

    void handle_delete_gsm_rule(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_request(req, "delete_gsm_rule")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = app::dto::gsm::GsmDeleteRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto& in = *typed;
        const auto key = rule_key(in.get_connector(), in.get_flow(), in.get_sub_flow(), in.get_code(), in.get_message());

        auto repo = m_context->repository();
        // Upstream delete_gsm_rule + to_not_found_response (core/gsm.rs:150-163).
        const auto stored = repo.find_gsm_rule(key);
        if (!stored) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND,
                "GSM with given key does not exist in our records"
            ));
            return;
        }

        // Upstream returns InternalServerError when the delete reports false
        // (core/gsm.rs:164-178).
        if (!repo.remove_gsm_rule(stored->get_id())) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INTERNAL,
                "Failed while Deleting Gsm rule"
            ));
            return;
        }

        app::dto::gsm::GsmDeleteResponse response;
        response.set_gsm_rule_delete(true);
        response.set_connector(in.get_connector());
        response.set_flow(in.get_flow());
        response.set_sub_flow(in.get_sub_flow());
        response.set_code(in.get_code());
        auto text = serde::Ser::encode_json(response);

        m_context->events().publish("gsm.rule.deleted", audit_payload("deleted", key));
        m_context->metrics().record_flow("GsmRuleDelete");
        m_context->metrics().counter("GSM_RULE_DELETED").increment();
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/gsm"}
            .add_route(
                Route{"/"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_gsm_rule(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_gsm_rule")
            )
            .add_route(
                Route{"/get"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_gsm_rule(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_gsm_rule")
            )
            .add_route(
                Route{"/update"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_update_gsm_rule(req, res, std::move(send));
                    })
                    .summary("Hyperswitch update_gsm_rule")
            )
            .add_route(
                Route{"/delete"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_delete_gsm_rule(req, res, std::move(send));
                    })
                    .summary("Hyperswitch delete_gsm_rule")
            );

        // Upstream exposes the same four operations under /v2/gsm (app.rs:2825-2835)
        // with V2AdminApiAuth instead of AdminApiAuth. Both extractors resolve to an
        // admin-level rule, so the shared handlers use the same RouteAuthRegistry rows.
        Router{router, "/v2/gsm"}
            .add_route(
                Route{"/"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_gsm_rule(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_gsm_rule (v2)")
            )
            .add_route(
                Route{"/get"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_gsm_rule(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_gsm_rule (v2)")
            )
            .add_route(
                Route{"/update"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_update_gsm_rule(req, res, std::move(send));
                    })
                    .summary("Hyperswitch update_gsm_rule (v2)")
            )
            .add_route(
                Route{"/delete"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_delete_gsm_rule(req, res, std::move(send));
                    })
                    .summary("Hyperswitch delete_gsm_rule (v2)")
            );
    }

    explicit GsmRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    [[nodiscard]] std::optional<core::error::AppError> authorize_request(
        interfaces::io::IRequest& req,
        std::string_view operation
    ) const
    {
        const auto context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for(operation);
        return m_context->authorizer().authorize_route(context, rule);
    }

    // Natural key from core/gsm.rs:40,60-67,150-157:
    // (connector, flow, sub_flow, code, message).
    [[nodiscard]] std::string rule_key(
        std::string_view connector,
        std::string_view flow,
        std::string_view sub_flow,
        std::string_view code,
        std::string_view message
    ) const
    {
        std::string key{"gsm.rule:"};
        const auto append_part = [&key](std::string_view part) {
            key.push_back('\x1f');
            key.append(part.data(), part.size());
        };
        key.append(connector.data(), connector.size());
        append_part(flow);
        append_part(sub_flow);
        append_part(code);
        append_part(message);
        return key;
    }

    // Upstream serde enums are snake_case with strum aliases. Normalize the
    // flattened string wrappers to canonical wire values rather than echoing raw
    // input. Unknown tokens are preserved in canonicalized form.
    [[nodiscard]] std::string canonical_enum(
        std::string_view raw,
        std::string_view kind,
        std::string_view fallback = {}
    ) const
    {
        while (!raw.empty() && std::isspace(static_cast<unsigned char>(raw.front())) != 0) {
            raw.remove_prefix(1);
        }
        while (!raw.empty() && std::isspace(static_cast<unsigned char>(raw.back())) != 0) {
            raw.remove_suffix(1);
        }
        if (kind == "feature_data" && !raw.empty() && raw.front() == '{') {
            return std::string{raw};
        }

        std::string token;
        token.reserve(raw.size());
        for (const char c : raw) {
            if (c == '-' || std::isspace(static_cast<unsigned char>(c)) != 0) {
                token.push_back('_');
            } else {
                token.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
            }
        }
        while (!token.empty() && token.front() == '_') {
            token.erase(token.begin());
        }
        while (!token.empty() && token.back() == '_') {
            token.pop_back();
        }
        if (token.empty()) {
            return std::string{fallback};
        }

        if (kind == "decision") {
            if (token == "dodefault") {
                return "do_default";
            }
            if (token == "retry") {
                return "retry";
            }
        } else if (kind == "feature") {
            if (token == "retry") {
                return "retry";
            }
        } else if (kind == "error_category") {
            if (token == "frmdecline") { return "frm_decline"; }
            if (token == "processordowntime") { return "processor_downtime"; }
            if (token == "processordeclineunauthorized") { return "processor_decline_unauthorized"; }
            if (token == "issuewithpaymentmethod") { return "issue_with_payment_method"; }
            if (token == "processordeclineincorrectdata") { return "processor_decline_incorrect_data"; }
            if (token == "harddecline") { return "hard_decline"; }
            if (token == "softdecline") { return "soft_decline"; }
        }
        return token;
    }

    // RetryFeatureData JSON shape from common_types/src/domain.rs:171-181.
    [[nodiscard]] std::string retry_feature_data_json(
        bool step_up_possible,
        bool clear_pan_possible,
        std::string_view decision
    ) const
    {
        return std::format(
            "{{\"retry\":{{\"step_up_possible\":{},\"clear_pan_possible\":{},\"alternate_network_possible\":false,\"decision\":\"{}\"}}}}",
            step_up_possible ? "true" : "false",
            clear_pan_possible ? "true" : "false",
            decision
        );
    }

    [[nodiscard]] std::optional<bool> flag_value(std::string_view text, std::string_view key) const
    {
        const auto pos = text.find(key);
        if (pos == std::string_view::npos) {
            return std::nullopt;
        }
        const auto rest = text.substr(pos + key.size());
        const auto yes = rest.find("true");
        const auto no = rest.find("false");
        if (yes == std::string_view::npos && no == std::string_view::npos) {
            return std::nullopt;
        }
        if (yes != std::string_view::npos && (no == std::string_view::npos || yes < no)) {
            return true;
        }
        return false;
    }

    // Reads RetryFeatureData flags out of the flattened feature_data value. A
    // JSON payload gives explicit booleans; a token value contributes an
    // implicit true for the step_up/clear_pan markers.
    [[nodiscard]] std::pair<std::optional<bool>, std::optional<bool>> retry_flags(
        std::string_view value
    ) const
    {
        const auto text = canonical_enum(value, "feature_data", "retry");
        auto step = flag_value(text, "step_up_possible");
        auto clear = flag_value(text, "clear_pan_possible");
        if (!step && (text.find("step_up") != std::string_view::npos ||
                      text.find("stepup") != std::string_view::npos)) {
            step = true;
        }
        if (!clear && (text.find("clear_pan") != std::string_view::npos ||
                       text.find("clearpan") != std::string_view::npos)) {
            clear = true;
        }
        return {step, clear};
    }

    [[nodiscard]] app::dto::gsm::GsmResponse response_from_create(
        const app::dto::gsm::GsmCreateRequest& in
    ) const
    {
        app::dto::gsm::GsmResponse response;
        response.set_connector(canonical_enum(in.get_connector().get_value(), "connector"));
        response.set_flow(in.get_flow());
        response.set_sub_flow(in.get_sub_flow());
        response.set_code(in.get_code());
        response.set_message(in.get_message());
        response.set_status(in.get_status());
        response.set_router_error(in.get_router_error());
        app::dto::gsm::GsmDecisionValue decision;
        decision.set_value(canonical_enum(in.get_decision().get_value(), "decision", "do_default"));
        response.set_decision(decision);
        response.set_step_up_possible(in.get_step_up_possible());
        response.set_unified_code(in.get_unified_code());
        response.set_unified_message(in.get_unified_message());
        if (in.get_error_category()) {
            app::dto::gsm::ErrorCategoryValue category;
            category.set_value(canonical_enum(in.get_error_category()->get_value(), "error_category"));
            response.set_error_category(category);
        }
        response.set_clear_pan_possible(in.get_clear_pan_possible());
        app::dto::gsm::GsmFeatureValue feature;
        feature.set_value(in.get_feature()
            ? canonical_enum(in.get_feature()->get_value(), "feature", "retry")
            : "retry");
        response.set_feature(feature);
        if (in.get_feature_data()) {
            app::dto::gsm::GsmFeatureDataValue feature_data;
            feature_data.set_value(canonical_enum(in.get_feature_data()->get_value(), "feature_data"));
            response.set_feature_data(feature_data);
        }
        if (in.get_standardised_code()) {
            app::dto::gsm::StandardisedCodeValue code;
            code.set_value(canonical_enum(in.get_standardised_code()->get_value(), "standardised_code"));
            response.set_standardised_code(code);
        }
        response.set_description(in.get_description());
        response.set_user_guidance_message(in.get_user_guidance_message());
        return response;
    }

    [[nodiscard]] app::store::gsm_rules::GsmRuleRecord record_from(
        std::string_view key,
        const app::dto::gsm::GsmResponse& response
    ) const
    {
        app::store::gsm_rules::GsmRuleRecord record;
        record.set_natural_key(std::string{key});
        record.set_merchant_id({});
        record.set_rule_name(std::format("{}/{}", response.get_connector(), response.get_flow()));
        record.set_decision(response.get_decision().get_value());
        record.set_program_json(serde::Ser::encode_json(response));
        record.set_status(response.get_status());
        record.set_created_at({});
        return record;
    }

    [[nodiscard]] std::optional<app::dto::gsm::GsmResponse> response_from_record(
        const app::store::gsm_rules::GsmRuleRecord& record
    ) const
    {
        if (record.get_program_json().empty()) {
            return std::nullopt;
        }
        auto parsed = serde::Ser::deserialize<app::dto::gsm::GsmResponse>(
            "application/json", record.get_program_json()
        );
        if (!parsed) {
            return std::nullopt;
        }
        return std::move(*parsed);
    }

    // Deterministic substitute for server_wrap/instrument flow plumbing.
    // Upstream routes use api_locking::LockAction::NotApplicable (routes/gsm.rs),
    // so no ApiLock key is acquired; the published event records the operation.
    [[nodiscard]] std::string audit_payload(std::string_view operation, std::string_view key) const
    {
        std::string escaped;
        escaped.reserve(key.size());
        for (const char c : key) {
            const auto byte = static_cast<unsigned char>(c);
            if (c == '"' || c == '\\' || byte < 0x20) {
                escaped += std::format("\\u{:04x}", static_cast<unsigned int>(byte));
            } else {
                escaped.push_back(c);
            }
        }
        return std::format(
            "{{\"natural_key\":\"{}\",\"operation\":\"{}\"}}",
            escaped,
            operation
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

} // namespace app::handlers::gsm
