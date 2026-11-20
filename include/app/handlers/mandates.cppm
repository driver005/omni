module;
export module app_handlers:mandates;

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
import app_metrics;
import app_store;
import app_connector;
import app_keystore;

export namespace app::handlers::mandates {

class MandatesRouteHandlers
{
public:
    void handle_get_mandate(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = ::app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for("get_mandate");
        const auto auth = m_context->authorizer().authorize_route(auth_context, rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto mandate_id = mandate_id_from_path(req, 1);
        if (!mandate_id.has_value() || mandate_id->empty()) {
            send_error(res, send, ::core::error::AppErrorBuilder{}(
                ::core::error::AppErrorKind::INVALID_REQUEST, "missing mandate id in path"));
            return;
        }

        const auto header_merchant_id = ::app::http::merchant_id_from_header{}(req);
        const auto merchant_id = header_merchant_id.empty() ? auth_context.merchant_id() : header_merchant_id;
        (void)::app::http::org_id_from_header{}(req);

        m_context->metrics().record_flow("mandate_retrieve");
        m_context->metrics().counter("mandates_retrieve_total").increment();

        const auto found = m_context->repository().find_mandate_by_string(*mandate_id);
        if (!found.has_value()) {
            send_error(res, send, ::core::error::AppErrorBuilder{}(
                ::core::error::AppErrorKind::NOT_FOUND, "mandate not found"));
            return;
        }
        const auto& record = *found;
        if (!merchant_id.empty() && !record.get_merchant_id().empty()
            && record.get_merchant_id() != merchant_id) {
            send_error(res, send, ::core::error::AppErrorBuilder{}(
                ::core::error::AppErrorKind::FORBIDDEN, "mandate belongs to a different merchant"));
            return;
        }

        m_context->cache().put(
            std::string{"mandates.last_retrieved."} + record.get_mandate_id(),
            ::core::cache::to_cache_value(record.get_status()));

        auto text = serde::Ser::encode_json(to_response(record));
        send_json(res, send, text, true);
    }

    void handle_revoke_mandate(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = ::app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for("revoke_mandate");
        const auto auth = m_context->authorizer().authorize_route(auth_context, rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto mandate_id = mandate_id_from_path(req, 2);
        if (!mandate_id.has_value() || mandate_id->empty()) {
            send_error(res, send, ::core::error::AppErrorBuilder{}(
                ::core::error::AppErrorKind::INVALID_REQUEST, "missing mandate id in path"));
            return;
        }

        const auto header_merchant_id = ::app::http::merchant_id_from_header{}(req);
        const auto merchant_id = header_merchant_id.empty() ? auth_context.merchant_id() : header_merchant_id;
        (void)::app::http::org_id_from_header{}(req);

        m_context->metrics().record_flow("mandate_revoke");
        m_context->metrics().counter("mandates_revoke_total").increment();

        const auto found = m_context->repository().find_mandate_by_string(*mandate_id);
        if (!found.has_value()) {
            send_error(res, send, ::core::error::AppErrorBuilder{}(
                ::core::error::AppErrorKind::NOT_FOUND, "mandate not found"));
            return;
        }
        auto record = *found;
        if (!merchant_id.empty() && !record.get_merchant_id().empty()
            && record.get_merchant_id() != merchant_id) {
            send_error(res, send, ::core::error::AppErrorBuilder{}(
                ::core::error::AppErrorKind::FORBIDDEN, "mandate belongs to a different merchant"));
            return;
        }

        // Revocable-status state machine (upstream common_enums::MandateStatus):
        // Active | Inactive | Pending may be revoked; Revoked is a conflict; every
        // other value is a validation failure. Raw string comparison is replaced by
        // this explicit classification.
        const auto revocable = classify_revoke_status(record.get_status());
        if (revocable == RevokeStatus::AlreadyRevoked) {
            send_error(res, send, ::core::error::AppErrorBuilder{}(
                ::core::error::AppErrorKind::PRECONDITION_FAILED,
                "Mandate has already been revoked"));
            return;
        }
        if (revocable == RevokeStatus::NotRevocable) {
            send_error(res, send, ::core::error::AppErrorBuilder{}(
                ::core::error::AppErrorKind::VALIDATION_FAILED,
                "mandate status is not revocable"));
            return;
        }

        // Connector-side revoke chain substitute. Upstream resolves profile -> MCA ->
        // connector by name, builds a MandateRevoke router data and triggers the
        // connector before persisting the Revoked status. Here the profile id already
        // lives on the persisted record, the connector router stands in for connector
        // data lookup and selection, and mandate_revoke() is the Trigger call.
        const auto scope_merchant = record.get_merchant_id().empty()
            ? merchant_id : record.get_merchant_id();
        auto connector_request = ::app::connector::ConnectorRequestBuilder{}
                                     .operation("mandate_revoke")
                                     .merchant_id(scope_merchant)
                                     .resource_id(record.get_mandate_id())
                                     .raw_payload(std::format(
                                         R"({{"mandate_id":"{}","profile_id":"{}","connector":"{}","status":"{}"}})",
                                         record.get_mandate_id(), record.get_profile_id(),
                                         record.get_connector(), record.get_status()))();

        auto connector = m_context->connector_router().select_for(connector_request);
        if (!connector) {
            connector = m_context->connector_router().select(scope_merchant);
        }
        if (!connector) {
            m_context->metrics().record_flow("mandate.revoke.connector_unavailable");
            m_context->metrics().counter("mandates_revoke_connector_unavailable").increment();
            const auto last = m_context->connector_router().last_error();
            send_error(res, send, last.value_or(::core::error::AppErrorBuilder{}(
                ::core::error::AppErrorKind::CONNECTOR_ERROR,
                "no connector available for mandate revoke")));
            return;
        }
        if (!record.get_profile_id().empty()) {
            // get_profile_id_for_mandate substitute: profile scoping is carried on the
            // connector request payload built above.
            m_context->metrics().counter("mandates_revoke_profile_scoped").increment();
        }

        const auto connector_response = connector->mandate_revoke(connector_request);
        if (connector_response.get_status().empty() || connector_response.get_retryable()) {
            // ExternalConnectorError substitute: surface the connector failure with a
            // semantic CONNECTOR_ERROR (502) instead of flipping local status.
            m_context->metrics().record_flow("mandate.revoke.connector_failed");
            m_context->metrics().counter("mandates_revoke_connector_failed").increment();
            std::string message = connector_response.get_message();
            if (message.empty()) { message = "connector rejected mandate revoke"; }
            send_error(res, send, ::core::error::AppErrorBuilder{}(
                ::core::error::AppErrorKind::CONNECTOR_ERROR, std::move(message)));
            return;
        }
        m_context->metrics().record_flow("mandate.revoke.connector_succeeded");
        m_context->metrics().counter("mandates_revoke_connector_succeeded").increment();

        m_context->cache().put(
            std::string{"mandates.revoking."} + record.get_mandate_id(),
            ::core::cache::to_cache_value(record.get_status()));

        // MandateUpdate::StatusUpdate { Revoked } after a successful connector trigger.
        record.set_status("revoked");
        m_context->repository().update_mandate(record);

        using Response = ::app::model::mandates::MandateRevokedResponse;
        Response response{};
        response.set_mandate_id(record.get_mandate_id());
        ::app::model::mandates::MandateStatusValue status{};
        status.set_value("revoked");
        response.set_status(std::move(status));
        response.set_error_code(std::nullopt);
        response.set_error_message(std::nullopt);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_retrieve_mandates_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = ::app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for("retrieve_mandates_list");
        const auto auth = m_context->authorizer().authorize_route(auth_context, rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto query = ::app::http::RequestQuery{}(req);

        // Typed query DTO. Nested wrappers (MandateStatusValue, PrimitiveDateTimeValue)
        // have no standalone serde mapping, so the DTO is populated from the typed
        // RequestQuery accessors instead of a JSON body.
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
            ::app::model::mandates::PrimitiveDateTimeValue t{}; t.set_value(*v);
            constraints.set_created_time(std::move(t));
        }
        if (const auto v = query.get("created_time.lt")) {
            ::app::model::mandates::PrimitiveDateTimeValue t{}; t.set_value(*v);
            constraints.set_created_time_lt(std::move(t));
        }
        if (const auto v = query.get("created_time.gt")) {
            ::app::model::mandates::PrimitiveDateTimeValue t{}; t.set_value(*v);
            constraints.set_created_time_gt(std::move(t));
        }
        if (const auto v = query.get("created_time.lte")) {
            ::app::model::mandates::PrimitiveDateTimeValue t{}; t.set_value(*v);
            constraints.set_created_time_lte(std::move(t));
        }
        if (const auto v = query.get("created_time.gte")) {
            ::app::model::mandates::PrimitiveDateTimeValue t{}; t.set_value(*v);
            constraints.set_created_time_gte(std::move(t));
        }

        auto merchant_filter = query.get("merchant_id");
        const auto header_merchant_id = ::app::http::merchant_id_from_header{}(req);
        std::string effective_merchant = header_merchant_id.empty()
            ? auth_context.merchant_id() : header_merchant_id;
        (void)::app::http::org_id_from_header{}(req);
        if (!merchant_filter.has_value() && !effective_merchant.empty()) {
            merchant_filter = effective_merchant;
        }

        m_context->metrics().record_flow("mandate_list");
        m_context->metrics().counter("mandates_list_total").increment();

        const auto records = m_context->repository().list_mandates();
        const auto offset = constraints.get_offset().value_or(0);
        const auto limit = constraints.get_limit().value_or(0);
        std::size_t skipped{0};
        std::size_t emitted{0};
        std::string data{"["};
        bool first{true};
        const auto& status_filter = constraints.get_mandate_status();
        const auto& connector_filter = constraints.get_connector();
        for (const auto& record : records) {
            if (status_filter && record.get_status() != status_filter->get_value()) {
                continue;
            }
            if (connector_filter && !connector_filter->empty()
                && record.get_connector() != *connector_filter) {
                continue;
            }
            if (merchant_filter.has_value() && !merchant_filter->empty()
                && record.get_merchant_id() != *merchant_filter) {
                continue;
            }
            if (!matches_created_time(record.get_created_at(), constraints)) {
                continue;
            }
            if (offset > 0 && skipped < static_cast<std::size_t>(offset)) {
                ++skipped;
                continue;
            }
            if (limit > 0 && emitted >= static_cast<std::size_t>(limit)) {
                break;
            }
            if (!first) {
                data.push_back(',');
            }
            data += serde::Ser::encode_json(to_response(record));
            first = false;
            ++emitted;
        }
        data.push_back(']');

        // Upstream returns ApplicationResponse::Json(mandates_list): a bare array.
        send_json(res, send, data, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/mandates"}
            .add_route(
                Route{"/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_mandates_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_mandates_list")
            )
            .add_route(
                Route{"/:id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_mandate(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_mandate")
            )
            .add_route(
                Route{"/revoke/:id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_revoke_mandate(req, res, std::move(send));
                    })
                    .summary("Hyperswitch revoke_mandate")
            );
    }

    explicit MandatesRouteHandlers(::app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    enum class RevokeStatus { Revocable, AlreadyRevoked, NotRevocable };

    /** @brief Classify a raw persisted status with the upstream MandateStatus state machine. */
    [[nodiscard]] RevokeStatus classify_revoke_status(std::string_view status) const noexcept
    {
        if (status == "active" || status == "inactive" || status == "pending") {
            return RevokeStatus::Revocable;
        }
        if (status == "revoked") {
            return RevokeStatus::AlreadyRevoked;
        }
        return RevokeStatus::NotRevocable;
    }

    /** @brief Extract the :id segment for a v1 mandate route via RequestPath. */
    [[nodiscard]] std::optional<std::string> mandate_id_from_path(
        interfaces::io::IRequest& req,
        std::size_t expected_index
    ) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        if (const auto segment = segments.at(expected_index)) {
            if (!segment->empty()) {
                return std::string{*segment};
            }
        }
        // Fallback for paths that place the id after the "mandates" literal.
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

    /** @brief Apply the created_time family of MandateListConstraints filters. */
    [[nodiscard]] bool matches_created_time(
        std::string_view actual,
        const ::app::model::mandates::MandateListConstraints& constraints
    ) const
    {
        const auto& exact = constraints.get_created_time();
        const auto& lt = constraints.get_created_time_lt();
        const auto& gt = constraints.get_created_time_gt();
        const auto& lte = constraints.get_created_time_lte();
        const auto& gte = constraints.get_created_time_gte();
        if (!exact && !lt && !gt && !lte && !gte) {
            return true;
        }
        if (actual.empty()) {
            return false;
        }
        if (exact && actual != exact->get_value()) { return false; }
        if (lt && actual >= lt->get_value()) { return false; }
        if (gt && actual <= gt->get_value()) { return false; }
        if (lte && actual > lte->get_value()) { return false; }
        if (gte && actual < gte->get_value()) { return false; }
        return true;
    }

    [[nodiscard]] std::optional<std::size_t> parse_positive(const std::optional<std::string>& raw) const
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

    /** @brief Map a persisted mandate record onto its upstream response DTO.
     *
     *  Upstream MandateResponse::from_db_mandate clones the processor key store and
     *  decrypts card/acceptance data. This runtime has no processor key manager; the
     *  AppContext KeyStore is the deterministic stand-in. Card JSON and acceptance
     *  text, when present, are resolved from (purpose="mandate_card"|
     *  "customer_acceptance", key_id=mandate_id) and mirrored into the typed DTO.
     */
    template<typename Record>
    [[nodiscard]] ::app::model::mandates::MandateResponse to_response(const Record& record) const
    {
        using namespace ::app::model::mandates;
        MandateResponse response{};
        response.set_mandate_id(record.get_mandate_id());

        if (!record.get_payment_id().empty()) {
            PaymentIdValue payment_id{};
            payment_id.set_value(record.get_payment_id());
            response.set_payment_id(std::move(payment_id));
        }
        if (record.get_amount() != 0) {
            MinorUnitValue amount{};
            amount.set_value(std::to_string(record.get_amount()));
            response.set_amount(std::move(amount));
        }
        if (!record.get_currency().empty()) {
            CurrencyValue currency{};
            currency.set_value(record.get_currency());
            response.set_currency(std::move(currency));
        }
        if (!record.get_profile_id().empty()) {
            ProfileIdValue profile_id{};
            profile_id.set_value(record.get_profile_id());
            response.set_profile_id(std::move(profile_id));
        }
        if (!record.get_mandate_metadata().empty()) {
            SecretSerdeValueValue metadata{};
            metadata.set_value(record.get_mandate_metadata());
            response.set_mandate_metadata(std::move(metadata));
        }
        MandateStatusValue status{};
        status.set_value(record.get_status());
        response.set_status(std::move(status));
        response.set_payment_method_id(record.get_payment_method_id());
        response.set_payment_method(record.get_payment_method());
        if (!record.get_payment_method_type().empty()) {
            response.set_payment_method_type(record.get_payment_method_type());
        }

        if (const auto card_secret = m_context->key_store().get("mandate_card", record.get_mandate_id())) {
            const auto& raw = card_secret->value();
            if (!raw.empty()) {
                if (auto card = serde::Ser::deserialize<MandateCardDetails>("application/json", raw)) {
                    response.set_card(std::move(*card));
                }
            }
        }
        if (const auto acceptance_secret = m_context->key_store().get("customer_acceptance", record.get_mandate_id())) {
            CustomerAcceptanceValue acceptance{};
            acceptance.set_value(acceptance_secret->value());
            response.set_customer_acceptance(std::move(acceptance));
        }
        return response;
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        ::core::error::AppError error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(::core::error::AppErrorStatus{}(error.get_kind()));
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

    ::app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::mandates
