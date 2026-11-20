module;
export module app_handlers:customers;

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
import app_store;
import app_http;
import app_crypto;
import app_metrics;

namespace app::handlers::customers {

/** @brief Local wire shape for the v2 global-id migration substitute.
 *
 *  Upstream `api_models::customers::migrate` carries the same counts plus a
 *  per-row result list. The local type mirrors that shape without requiring a
 *  change to the shared app_model partition.
 */
class GlobalIdMigrationRow
{
public:
    void set_row_number(std::int64_t value) { row_number = value; }
    [[nodiscard]] const std::int64_t& get_row_number() const noexcept { return row_number; }
    void set_merchant_id(std::string value) { merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return merchant_id; }
    void set_customer_id(std::string value) { customer_id = std::move(value); }
    [[nodiscard]] const std::string& get_customer_id() const noexcept { return customer_id; }
    void set_status(std::string value) { status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return status; }
    void set_old_id(std::string value) { old_id = std::move(value); }
    [[nodiscard]] const std::string& get_old_id() const noexcept { return old_id; }
    void set_new_id(std::string value) { new_id = std::move(value); }
    [[nodiscard]] const std::string& get_new_id() const noexcept { return new_id; }
    void set_message(std::string value) { message = std::move(value); }
    [[nodiscard]] const std::string& get_message() const noexcept { return message; }

    std::int64_t row_number{};
    std::string merchant_id{};
    std::string customer_id{};
    std::string status{};
    std::string old_id{};
    std::string new_id{};
    std::string message{};
};

class GlobalIdMigrationResponse
{
public:
    void set_total_rows(std::int64_t value) { total_rows = value; }
    [[nodiscard]] const std::int64_t& get_total_rows() const noexcept { return total_rows; }
    void set_updated_count(std::int64_t value) { updated_count = value; }
    [[nodiscard]] const std::int64_t& get_updated_count() const noexcept { return updated_count; }
    void set_skipped_count(std::int64_t value) { skipped_count = value; }
    [[nodiscard]] const std::int64_t& get_skipped_count() const noexcept { return skipped_count; }
    void set_failed_count(std::int64_t value) { failed_count = value; }
    [[nodiscard]] const std::int64_t& get_failed_count() const noexcept { return failed_count; }
    void set_results(std::vector<GlobalIdMigrationRow> value) { results = std::move(value); }
    [[nodiscard]] const std::vector<GlobalIdMigrationRow>& get_results() const noexcept { return results; }

    std::int64_t total_rows{};
    std::int64_t updated_count{};
    std::int64_t skipped_count{};
    std::int64_t failed_count{};
    std::vector<GlobalIdMigrationRow> results{};
};

} // namespace app::handlers::customers

// Serialization must be declared before the handler bodies that encode these DTOs.
template<>
struct serde::Serializable<::app::handlers::customers::GlobalIdMigrationRow>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"row_number", &::app::handlers::customers::GlobalIdMigrationRow::get_row_number, &::app::handlers::customers::GlobalIdMigrationRow::set_row_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &::app::handlers::customers::GlobalIdMigrationRow::get_merchant_id, &::app::handlers::customers::GlobalIdMigrationRow::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_id", &::app::handlers::customers::GlobalIdMigrationRow::get_customer_id, &::app::handlers::customers::GlobalIdMigrationRow::set_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &::app::handlers::customers::GlobalIdMigrationRow::get_status, &::app::handlers::customers::GlobalIdMigrationRow::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"old_id", &::app::handlers::customers::GlobalIdMigrationRow::get_old_id, &::app::handlers::customers::GlobalIdMigrationRow::set_old_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"new_id", &::app::handlers::customers::GlobalIdMigrationRow::get_new_id, &::app::handlers::customers::GlobalIdMigrationRow::set_new_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"message", &::app::handlers::customers::GlobalIdMigrationRow::get_message, &::app::handlers::customers::GlobalIdMigrationRow::set_message, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "customer_global_id_migration_row"; }
};

template<>
struct serde::Serializable<::app::handlers::customers::GlobalIdMigrationResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"total_rows", &::app::handlers::customers::GlobalIdMigrationResponse::get_total_rows, &::app::handlers::customers::GlobalIdMigrationResponse::set_total_rows, serde::FieldOptions{}>{},
            serde::FieldDesc<"updated_count", &::app::handlers::customers::GlobalIdMigrationResponse::get_updated_count, &::app::handlers::customers::GlobalIdMigrationResponse::set_updated_count, serde::FieldOptions{}>{},
            serde::FieldDesc<"skipped_count", &::app::handlers::customers::GlobalIdMigrationResponse::get_skipped_count, &::app::handlers::customers::GlobalIdMigrationResponse::set_skipped_count, serde::FieldOptions{}>{},
            serde::FieldDesc<"failed_count", &::app::handlers::customers::GlobalIdMigrationResponse::get_failed_count, &::app::handlers::customers::GlobalIdMigrationResponse::set_failed_count, serde::FieldOptions{}>{},
            serde::FieldDesc<"results", &::app::handlers::customers::GlobalIdMigrationResponse::get_results, &::app::handlers::customers::GlobalIdMigrationResponse::set_results, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "customer_global_id_migration_response"; }
};

export namespace app::handlers::customers {

class CustomersRouteHandlers
{
public:
    void handle_customers_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("customers_create");
        const auto auth = m_context->authorizer().authorize_route(
            ::app::auth::AuthContextBuilder{}(req), rule
        );
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(::core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = ::app::model::customers::CustomerRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(::core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        std::optional<std::string_view> document_number{};
        std::optional<std::string_view> document_type{};
        if (parsed->get_document_details()) {
            document_number = std::string_view{
                parsed->get_document_details()->get_document_number()
            };
            document_type = std::string_view{
                parsed->get_document_details()->get_document_type().get_value()
            };
        }
        if (const auto invalid = validate_customer_fields(
                document_number, document_type, parsed->get_phone_country_code()
            )) {
            send_error(res, send, *invalid);
            return;
        }

        std::string customer_id = parsed->get_customer_id()
            ? parsed->get_customer_id()->get_value()
            : m_context->ids().payment_id("cus_");

        ::app::store::AppRepository repository = m_context->repository();
        // Merchant-reference duplicate pre-check before insert (core/customers.rs:96-104).
        if (repository.find_customer_by_string(customer_id)) {
            send_error(res, send, error(::core::error::AppErrorKind::ALREADY_EXISTS, "customer already exists"));
            return;
        }

        std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) {
            merchant_id = parsed->get_merchant_id().get_value();
        }

        ::app::store::customers::CustomerRecord record;
        record.set_customer_id(customer_id);
        record.set_merchant_id(std::move(merchant_id));
        if (parsed->get_name()) {
            record.set_name(encrypt_pii(*parsed->get_name()));
        }
        if (parsed->get_email()) {
            record.set_email(encrypt_pii(parsed->get_email()->get_value()));
        }
        if (parsed->get_phone()) {
            record.set_phone(encrypt_pii(*parsed->get_phone()));
        }
        if (parsed->get_description()) {
            record.set_description(parsed->get_description()->get_value());
        }
        if (parsed->get_metadata()) {
            record.set_metadata(parsed->get_metadata()->get_value());
        }
        const auto saved = repository.save_customer(std::move(record));

        auto response = to_customer_response(saved);
        if (parsed->get_phone_country_code()) {
            response.set_phone_country_code(*parsed->get_phone_country_code());
        }
        if (parsed->get_description()) {
            response.set_description(*parsed->get_description());
        }
        if (parsed->get_address()) {
            response.set_address(*parsed->get_address());
        }
        if (parsed->get_metadata()) {
            response.set_metadata(*parsed->get_metadata());
        }
        ::app::model::customers::OptionalEncryptableSecretStringValue response_tax_id;
        if (parsed->get_tax_registration_id()) {
            response_tax_id.set_value(*parsed->get_tax_registration_id());
        }
        response.set_tax_registration_id(std::move(response_tax_id));
        if (parsed->get_document_details()) {
            response.set_document_details(*parsed->get_document_details());
        }

        m_context->metrics().record_flow("customers.create");
        m_context->metrics().counter("customers_create").increment();
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_customers_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("customers_retrieve");
        const auto auth = m_context->authorizer().authorize_route(
            ::app::auth::AuthContextBuilder{}(req), rule
        );
        if (auth) { send_error(res, send, *auth); return; }

        const std::string customer_id = path_segment(req);
        if (customer_id.empty()) {
            send_error(res, send, error(::core::error::AppErrorKind::INVALID_REQUEST, "missing customer id"));
            return;
        }

        ::app::store::AppRepository repository = m_context->repository();
        const auto found = locate_customer(repository, req, customer_id);
        if (!found) {
            send_error(res, send, error(::core::error::AppErrorKind::NOT_FOUND, "customer not found"));
            return;
        }

        m_context->metrics().record_flow("customers.retrieve");
        m_context->metrics().counter("customers_retrieve").increment();
        send_json(res, send, serde::Ser::encode_json(to_customer_response(*found)), true);
    }

    void handle_customers_retrieve_by_merchant_reference_id(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for(
            "customers_retrieve_by_merchant_reference_id"
        );
        const auto auth = m_context->authorizer().authorize_route(
            ::app::auth::AuthContextBuilder{}(req), rule
        );
        if (auth) { send_error(res, send, *auth); return; }

        const std::string reference = path_segment(req);
        if (reference.empty()) {
            send_error(res, send, error(::core::error::AppErrorKind::INVALID_REQUEST, "missing merchant reference id"));
            return;
        }

        ::app::store::AppRepository repository = m_context->repository();
        const auto found = locate_customer(repository, req, reference);
        if (!found) {
            send_error(res, send, error(::core::error::AppErrorKind::NOT_FOUND, "customer not found"));
            return;
        }

        m_context->metrics().record_flow("customers.retrieve_by_merchant_reference_id");
        m_context->metrics().counter("customers_retrieve_by_merchant_reference_id").increment();
        send_json(res, send, serde::Ser::encode_json(to_customer_response(*found)), true);
    }

    void handle_customers_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("customers_list");
        const auto auth = m_context->authorizer().authorize_route(
            ::app::auth::AuthContextBuilder{}(req), rule
        );
        if (auth) { send_error(res, send, *auth); return; }

        using Input = ::app::model::customers::CustomerListRequest;
        Input input{};
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto value = query.get("offset")) {
            std::uint32_t parsed_offset{};
            const auto converted = std::from_chars(
                value->data(), value->data() + value->size(), parsed_offset
            );
            if (converted.ec == std::errc{} && converted.ptr == value->data() + value->size()) {
                input.set_offset(parsed_offset);
            }
        }
        if (const auto value = query.get("limit")) {
            std::uint16_t parsed_limit{};
            const auto converted = std::from_chars(
                value->data(), value->data() + value->size(), parsed_limit
            );
            if (converted.ec == std::errc{} && converted.ptr == value->data() + value->size()) {
                input.set_limit(parsed_limit);
            }
        }
        if (const auto value = query.get("customer_id")) {
            ::app::model::customers::CustomerIdValue customer_id;
            customer_id.set_value(*value);
            input.set_customer_id(std::move(customer_id));
        }
        const auto body = req.body_as_string();
        if (!body.empty()) {
            auto parsed = serde::Ser::deserialize<Input>("application/json", body);
            if (!parsed) {
                send_error(res, send, error(::core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            if (!input.get_offset()) { input.set_offset(parsed->get_offset()); }
            if (!input.get_limit()) { input.set_limit(parsed->get_limit()); }
            if (!input.get_customer_id()) { input.set_customer_id(parsed->get_customer_id()); }
        }

        const std::size_t offset = input.get_offset().value_or(0);
        // Upstream DEFAULT_LIST_API_LIMIT is 10 (crates/router/src/consts.rs:42).
        const std::size_t limit = input.get_limit() ? static_cast<std::size_t>(*input.get_limit()) : 10U;

        ::app::store::AppRepository repository = m_context->repository();
        const auto records = repository.list_customers();
        std::vector<::app::model::customers::CustomerResponse> customers{};
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        std::size_t position = 0;
        for (const auto& record : records) {
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) { continue; }
            if (input.get_customer_id()
                && record.get_customer_id() != input.get_customer_id()->get_value()) { continue; }
            if (position < offset) { ++position; continue; }
            if (position - offset >= limit) { break; }
            customers.push_back(to_customer_response(record));
            ++position;
        }

        m_context->metrics().record_flow("customers.list");
        m_context->metrics().counter("customers_list").increment();
        send_json(res, send, serde::Ser::encode_json(customers), true);
    }

    void handle_customers_list_with_count(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("customers_list_with_count");
        const auto auth = m_context->authorizer().authorize_route(
            ::app::auth::AuthContextBuilder{}(req), rule
        );
        if (auth) { send_error(res, send, *auth); return; }

        using Input = ::app::model::customers::CustomerListRequestWithConstraints;
        Input input{};
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto value = query.get("offset")) {
            std::uint32_t parsed_offset{};
            const auto converted = std::from_chars(
                value->data(), value->data() + value->size(), parsed_offset
            );
            if (converted.ec == std::errc{} && converted.ptr == value->data() + value->size()) {
                input.set_offset(parsed_offset);
            }
        }
        if (const auto value = query.get("limit")) {
            ::app::model::customers::CustomerListLimitValue limit;
            limit.set_value(*value);
            input.set_limit(std::move(limit));
        }
        if (const auto value = query.get("customer_id")) {
            ::app::model::customers::CustomerIdValue customer_id;
            customer_id.set_value(*value);
            input.set_customer_id(std::move(customer_id));
        }
        if (const auto value = query.get("time_range")) {
            ::app::model::customers::TimeRangeValue time_range;
            time_range.set_value(*value);
            input.set_time_range(std::move(time_range));
        }
        const auto body = req.body_as_string();
        if (!body.empty()) {
            auto parsed = serde::Ser::deserialize<Input>("application/json", body);
            if (!parsed) {
                send_error(res, send, error(::core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            if (!input.get_offset()) { input.set_offset(parsed->get_offset()); }
            if (!input.get_limit()) { input.set_limit(parsed->get_limit()); }
            if (!input.get_customer_id()) { input.set_customer_id(parsed->get_customer_id()); }
            if (!input.get_time_range()) { input.set_time_range(parsed->get_time_range()); }
        }

        const std::size_t offset = input.get_offset().value_or(0);
        std::size_t limit = 10U; // DEFAULT_LIST_API_LIMIT
        if (input.get_limit()) {
            const std::string& raw = input.get_limit()->get_value();
            std::size_t parsed_limit{};
            const auto converted = std::from_chars(raw.data(), raw.data() + raw.size(), parsed_limit);
            if (converted.ec == std::errc{} && converted.ptr == raw.data() + raw.size()) {
                limit = parsed_limit;
            }
        }
        const auto time_range = input.get_time_range()
            ? parse_time_range(input.get_time_range()->get_value())
            : std::nullopt;

        ::app::store::AppRepository repository = m_context->repository();
        const auto all_records = repository.list_customers();
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        std::vector<::app::store::customers::CustomerRecord> records{};
        for (const auto& record : all_records) {
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) { continue; }
            if (input.get_customer_id()
                && record.get_customer_id() != input.get_customer_id()->get_value()) { continue; }
            if (time_range && !within_range(record.get_created_at(), *time_range)) { continue; }
            records.push_back(record);
        }
        std::vector<::app::model::customers::CustomerResponse> customers{};
        ::app::model::customers::CustomerListResponse response;
        response.set_total_count(records.size());
        std::size_t position = 0;
        for (const auto& record : records) {
            if (position < offset) { ++position; continue; }
            if (position - offset >= limit) { break; }
            customers.push_back(to_customer_response(record));
            ++position;
        }
        response.set_data(std::move(customers));

        m_context->metrics().record_flow("customers.list_with_count");
        m_context->metrics().counter("customers_list_with_count").increment();
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_customers_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("customers_update");
        const auto auth = m_context->authorizer().authorize_route(
            ::app::auth::AuthContextBuilder{}(req), rule
        );
        if (auth) { send_error(res, send, *auth); return; }

        const std::string customer_id = path_segment(req);
        if (customer_id.empty()) {
            send_error(res, send, error(::core::error::AppErrorKind::INVALID_REQUEST, "missing customer id"));
            return;
        }
        ::app::store::AppRepository repository = m_context->repository();
        const auto existing = locate_customer(repository, req, customer_id);
        if (!existing) {
            send_error(res, send, error(::core::error::AppErrorKind::NOT_FOUND, "customer not found"));
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(::core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = ::app::model::customers::CustomerUpdateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(::core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        std::optional<std::string_view> document_number{};
        std::optional<std::string_view> document_type{};
        if (parsed->get_document_details()) {
            document_number = std::string_view{
                parsed->get_document_details()->get_document_number()
            };
            document_type = std::string_view{
                parsed->get_document_details()->get_document_type().get_value()
            };
        }
        if (const auto invalid = validate_customer_fields(
                document_number, document_type, parsed->get_phone_country_code()
            )) {
            send_error(res, send, *invalid);
            return;
        }

        std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) {
            merchant_id = parsed->get_merchant_id().get_value();
        }
        if (!::app::http::merchant_id_from_header{}(req).empty()
            && !existing->get_merchant_id().empty()
            && existing->get_merchant_id() != merchant_id) {
            send_error(res, send, error(::core::error::AppErrorKind::FORBIDDEN, "customer belongs to another merchant"));
            return;
        }

        // Verify-id before applying CustomerUpdate::Update (core/customers.rs:1245-1262).
        if (!is_global_customer_id(customer_id)
            && !existing->get_customer_id().empty()
            && existing->get_customer_id() != customer_id) {
            send_error(res, send, error(::core::error::AppErrorKind::PRECONDITION_FAILED, "customer id mismatch"));
            return;
        }

        ::app::store::customers::CustomerRecord record{*existing};
        record.set_merchant_id(std::move(merchant_id));
        if (parsed->get_name()) { record.set_name(encrypt_pii(*parsed->get_name())); }
        if (parsed->get_email()) { record.set_email(encrypt_pii(parsed->get_email()->get_value())); }
        if (parsed->get_phone()) { record.set_phone(encrypt_pii(*parsed->get_phone())); }
        if (parsed->get_description()) { record.set_description(parsed->get_description()->get_value()); }
        if (parsed->get_metadata()) { record.set_metadata(parsed->get_metadata()->get_value()); }
        if (!repository.update_customer(record)) {
            send_error(res, send, error(::core::error::AppErrorKind::INTERNAL_ERROR, "customer update failed"));
            return;
        }

        auto response = to_customer_response(record);
        if (parsed->get_phone_country_code()) { response.set_phone_country_code(*parsed->get_phone_country_code()); }
        if (parsed->get_description()) { response.set_description(*parsed->get_description()); }
        if (parsed->get_address()) { response.set_address(*parsed->get_address()); }
        if (parsed->get_metadata()) { response.set_metadata(*parsed->get_metadata()); }
        ::app::model::customers::OptionalEncryptableSecretStringValue response_tax_id;
        if (parsed->get_tax_registration_id()) { response_tax_id.set_value(*parsed->get_tax_registration_id()); }
        response.set_tax_registration_id(std::move(response_tax_id));
        if (parsed->get_document_details()) { response.set_document_details(*parsed->get_document_details()); }

        m_context->metrics().record_flow("customers.update");
        m_context->metrics().counter("customers_update").increment();
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_customers_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("customers_delete");
        const auto auth = m_context->authorizer().authorize_route(
            ::app::auth::AuthContextBuilder{}(req), rule
        );
        if (auth) { send_error(res, send, *auth); return; }

        const std::string customer_id = path_segment(req);
        if (customer_id.empty()) {
            send_error(res, send, error(::core::error::AppErrorKind::INVALID_REQUEST, "missing customer id"));
            return;
        }
        ::app::store::AppRepository repository = m_context->repository();
        const auto existing = locate_customer(repository, req, customer_id);
        if (!existing) {
            send_error(res, send, error(::core::error::AppErrorKind::NOT_FOUND, "customer not found"));
            return;
        }
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        if (!merchant_id.empty() && !existing->get_merchant_id().empty()
            && existing->get_merchant_id() != merchant_id) {
            send_error(res, send, error(::core::error::AppErrorKind::FORBIDDEN, "customer belongs to another merchant"));
            return;
        }

        // Redaction is a non-reentrant pipeline upstream (api_locking). Use the
        // injected lock registry so concurrent deletes cannot double-redact.
        const std::string lock_key = "customer_redact:" + existing->get_customer_id();
        const auto lock = m_context->locks().acquire(lock_key, std::int64_t{30}, "customers_delete");
        if (!lock) {
            send_error(res, send, error(::core::error::AppErrorKind::PRECONDITION_FAILED, "customer redaction already in progress"));
            return;
        }

        // Active-mandate rejection (core/customers.rs:834-866).
        for (const auto& mandate : repository.find_mandates_by_customer_id(existing->get_customer_id())) {
            if (mandate.get_status() == "ACTIVE") {
                m_context->locks().release(lock_key);
                send_error(res, send, error(::core::error::AppErrorKind::PRECONDITION_FAILED, "customer has an active mandate"));
                return;
            }
        }

        // Payment-method deletion (core/customers.rs:890-929). Card and network-token
        // rows are vault records and have no local store entity; see disposition.
        std::size_t payment_methods_deleted{0};
        for (const auto& method : repository.list_payment_methods()) {
            if (method.get_customer_id() != existing->get_customer_id()) { continue; }
            if (!merchant_id.empty() && method.get_merchant_id() != merchant_id) { continue; }
            if (repository.remove_payment_method(method.get_id())) { ++payment_methods_deleted; }
        }

        // Encrypted REDACTED update and DeleteStatus::Redacted (core/customers.rs:1007-1185).
        ::app::store::customers::CustomerRecord redacted{*existing};
        const std::string tombstone = encrypt_pii("REDACTED");
        redacted.set_name(tombstone);
        redacted.set_email(tombstone);
        redacted.set_phone(tombstone);
        redacted.set_description("REDACTED");
        redacted.set_metadata("REDACTED");
        if (!repository.update_customer(redacted)) {
            m_context->locks().release(lock_key);
            send_error(res, send, error(::core::error::AppErrorKind::INTERNAL_ERROR, "customer redaction failed"));
            return;
        }

        const std::int64_t tracker = m_context->process_trackers().schedule(
            "customer_redaction_followup", existing->get_customer_id(), "0"
        );
        (void)tracker;
        m_context->events().publish("customers.redacted", existing->get_customer_id());
        m_context->metrics().record_flow("customers.redacted");
        m_context->metrics().counter("CUSTOMER_REDACTED").increment();
        m_context->locks().release(lock_key);

        m_context->metrics().record_flow("customers.delete");
        m_context->metrics().counter("customers_delete").increment();

        ::app::model::customers::CustomerDeleteResponse response;
        ::app::model::customers::CustomerIdValue response_id;
        response_id.set_value(existing->get_customer_id());
        response.set_customer_id(std::move(response_id));
        response.set_customer_deleted(true);
        response.set_address_deleted(true);
        response.set_payment_methods_deleted(payment_methods_deleted > 0);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_get_customer_mandates(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_customer_mandates");
        const auto auth = m_context->authorizer().authorize_route(
            ::app::auth::AuthContextBuilder{}(req), rule
        );
        if (auth) { send_error(res, send, *auth); return; }

        const std::string customer_id = path_segment(req, 1);
        if (customer_id.empty()) {
            send_error(res, send, error(::core::error::AppErrorKind::INVALID_REQUEST, "missing customer id"));
            return;
        }
        ::app::store::AppRepository repository = m_context->repository();
        const auto customer = locate_customer(repository, req, customer_id);
        if (!customer) {
            send_error(res, send, error(::core::error::AppErrorKind::NOT_FOUND, "customer not found"));
            return;
        }

        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        std::vector<::app::model::mandates::MandateResponse> mandates{};
        // Upstream core::mandate::get_customer_mandates is customer-scoped, not a scan
        // of every mandate in the merchant (core/customers.rs:834).
        for (const auto& record : repository.find_mandates_by_customer_id(customer->get_customer_id())) {
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) { continue; }
            ::app::model::mandates::MandateResponse item;
            item.set_mandate_id(record.get_mandate_id());
            ::app::model::mandates::MandateStatusValue status;
            status.set_value(record.get_status());
            item.set_status(std::move(status));
            item.set_payment_method_id(record.get_payment_method_id());
            item.set_payment_method(record.get_payment_method());
            mandates.push_back(std::move(item));
        }

        m_context->metrics().record_flow("customers.get_customer_mandates");
        m_context->metrics().counter("customers_get_customer_mandates").increment();
        send_json(res, send, serde::Ser::encode_json(mandates), true);
    }

    void handle_migrate_global_id(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("migrate_global_id");
        const auto auth = m_context->authorizer().authorize_route(
            ::app::auth::AuthContextBuilder{}(req), rule
        );
        if (auth) { send_error(res, send, *auth); return; }

        // Deterministic local substitute for the multipart CSV upload. The body is
        // read as text and multipart boundary / header lines are skipped; the CSV
        // header must be merchant_id,customer_id as upstream requires.
        std::vector<std::pair<std::string, std::string>> rows{};
        bool header_seen{false};
        std::string current{};
        const std::string body = req.body_as_string();
        const auto finish_line = [&]()
        {
            std::string line = trim(current);
            current.clear();
            if (line.empty() || line.starts_with("--") || line.starts_with("Content-")) { return; }
            if (!header_seen) {
                if (line != "merchant_id,customer_id") { return; }
                header_seen = true;
                return;
            }
            const std::size_t comma = line.find(',');
            if (comma == std::string::npos) { return; }
            const std::string merchant = trim(line.substr(0, comma));
            const std::string customer = trim(line.substr(comma + 1));
            if (!merchant.empty() && !customer.empty()) {
                rows.emplace_back(merchant, customer);
            }
        };
        for (const char ch : body) {
            if (ch == '\n') { finish_line(); } else if (ch != '\r') { current.push_back(ch); }
        }
        finish_line();
        if (!header_seen) {
            send_error(res, send, error(::core::error::AppErrorKind::INVALID_REQUEST, "CSV must include header: merchant_id,customer_id"));
            return;
        }

        ::app::store::AppRepository repository = m_context->repository();
        GlobalIdMigrationResponse response{};
        std::vector<GlobalIdMigrationRow> results{};
        results.reserve(rows.size());
        response.set_total_rows(static_cast<std::int64_t>(rows.size()));
        for (std::size_t i{0}; i < rows.size(); ++i) {
            GlobalIdMigrationRow result{};
            result.set_row_number(static_cast<std::int64_t>(i + 2));
            result.set_merchant_id(rows[i].first);
            result.set_customer_id(rows[i].second);
            auto record = repository.find_customer_by_string(rows[i].second);
            if (!record || (!rows[i].first.empty() && record->get_merchant_id() != rows[i].first)) {
                result.set_status("NotFound");
                result.set_message("Customer not found for merchant_id and customer_id");
                response.set_failed_count(response.get_failed_count() + 1);
            } else if (is_global_customer_id(record->get_customer_id())) {
                result.set_status("AlreadyGlobalId");
                result.set_old_id(record->get_customer_id());
                result.set_message("Customer id is already in global id format");
                response.set_skipped_count(response.get_skipped_count() + 1);
            } else {
                const std::string old_id = record->get_customer_id();
                const std::string new_id = m_context->ids().payment_id("cus_");
                record->set_customer_id(new_id);
                if (repository.update_customer(*record)) {
                    result.set_status(old_id.empty() ? "UpdatedNullId" : "UpdatedNonGlobalId");
                    result.set_old_id(old_id);
                    result.set_new_id(new_id);
                    result.set_message("Customer id updated successfully");
                    response.set_updated_count(response.get_updated_count() + 1);
                } else {
                    result.set_status("UpdateFailed");
                    result.set_old_id(old_id);
                    result.set_message("Failed to update customer id");
                    response.set_failed_count(response.get_failed_count() + 1);
                }
            }
            results.push_back(std::move(result));
        }
        response.set_results(std::move(results));

        m_context->metrics().record_flow("customers.migrate_global_id");
        m_context->metrics().counter("customers_migrate_global_id").increment();
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        install_scope(router, "/customers", false);

        // Upstream v2 mounts under /{micro_services.payment_methods_prefix}/customers.
        // Resolve the configured prefix from ConfigStore with a deterministic local
        // default so the v2 surface exists even before config is populated.
        std::string v2_prefix{"/v2/customers"};
        if (m_context) {
            const auto configured = m_context->config_store().get_value(
                "micro_services", "payment_methods_prefix"
            );
            if (configured && !configured->empty()) {
                v2_prefix = "/" + *configured + "/customers";
            }
        }
        if (v2_prefix != "/customers") {
            install_scope(router, v2_prefix, true);
        }
    }

    explicit CustomersRouteHandlers(::app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    void install_scope(
        core::router::RouterContext<>& router,
        std::string_view prefix,
        bool include_migration
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        auto chain = Router{router, prefix}
            .add_route(
                Route{"/"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_customers_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch customers_create")
            )
            .add_route(
                Route{"/:id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_customers_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch customers_retrieve")
            )
            .add_route(
                Route{"/reference/:merchant_reference_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_customers_retrieve_by_merchant_reference_id(req, res, std::move(send));
                    })
                    .summary("Hyperswitch customers_retrieve_by_merchant_reference_id")
            )
            .add_route(
                Route{"/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_customers_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch customers_list")
            )
            .add_route(
                Route{"/list_with_count"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_customers_list_with_count(req, res, std::move(send));
                    })
                    .summary("Hyperswitch customers_list_with_count")
            )
            .add_route(
                Route{"/:id"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_customers_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch customers_update")
            )
            .add_route(
                Route{"/:id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_customers_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch customers_update (v1 POST alias)")
            )
            .add_route(
                Route{"/:id"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_customers_delete(req, res, std::move(send));
                    })
                    .summary("Hyperswitch customers_delete")
            )
            .add_route(
                Route{"/:customer_id/mandates"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_customer_mandates(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_customer_mandates")
            );

        if (include_migration) {
            std::move(chain).add_route(
                Route{"/migrate/global-id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_migrate_global_id(req, res, std::move(send));
                    })
                    .summary("Hyperswitch customers migrate_global_id")
            );
        }
        (void)chain;
    }

    /** @brief Build a typed error through the semantic core_error builder. */
    [[nodiscard]] ::core::error::AppError error(
        ::core::error::AppErrorKind kind,
        std::string message
    ) const
    {
        return ::core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    /** @brief Path segment counted from the end; 0 = last, 1 = second-to-last. */
    [[nodiscard]] std::string path_segment(
        interfaces::io::IRequest& req,
        std::size_t from_end = 0
    ) const
    {
        const ::app::http::PathSegments segments{::app::http::RequestPath{}(req)};
        const std::size_t count{segments.count()};
        if (from_end >= count) { return {}; }
        const auto value = segments.at(count - 1 - from_end);
        return value ? std::string{*value} : std::string{};
    }

    /** @brief v2 ids are generated global ids; v1 ids are merchant references. */
    [[nodiscard]] bool is_global_customer_id(std::string_view id) const noexcept
    {
        constexpr std::string_view prefix{"cus_"};
        return id.starts_with(prefix) && id.size() >= prefix.size() + 16U;
    }

    /** @brief Direct natural-key lookup with merchant-reference and numeric fallbacks. */
    [[nodiscard]] std::optional<::app::store::customers::CustomerRecord>
    locate_customer(
        ::app::store::AppRepository& repository,
        interfaces::io::IRequest& req,
        std::string_view id
    ) const
    {
        if (auto found = repository.find_customer_by_string(id)) { return found; }
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        if (!merchant_id.empty()) {
            for (const auto& record : repository.list_customers()) {
                if (record.get_merchant_id() == merchant_id && record.get_customer_id() == id) {
                    return record;
                }
            }
        }
        std::int64_t numeric{};
        const auto converted = std::from_chars(id.data(), id.data() + id.size(), numeric);
        if (converted.ec == std::errc{} && converted.ptr == id.data() + id.size()) {
            return repository.find_customer(numeric);
        }
        return std::nullopt;
    }

    /** @brief Document-details and phone-country-code validation (core/customers.rs:64-80). */
    [[nodiscard]] std::optional<::core::error::AppError> validate_customer_fields(
        std::optional<std::string_view> document_number,
        std::optional<std::string_view> document_type,
        std::optional<std::string> phone_country_code
    ) const
    {
        if (document_number || document_type) {
            if (!document_type || document_type->empty()
                || !document_number || document_number->empty()
                || document_number->size() > 255U) {
                return error(::core::error::AppErrorKind::VALIDATION_FAILED, "invalid document_details");
            }
        }
        if (phone_country_code && !valid_phone_country_code(*phone_country_code)) {
            return error(
                ::core::error::AppErrorKind::VALIDATION_FAILED,
                "phone_country_code must be a valid country calling code"
            );
        }
        return std::nullopt;
    }

    [[nodiscard]] bool valid_phone_country_code(std::string_view code) const noexcept
    {
        constexpr std::size_t max_length{8U};
        if (code.empty() || code.size() > max_length) { return false; }
        const std::size_t start = code.front() == '+' ? 1U : 0U;
        if (start == code.size()) { return false; }
        for (std::size_t i = start; i < code.size(); ++i) {
            if (code[i] < '0' || code[i] > '9') { return false; }
        }
        return true;
    }

    /** @brief Deterministic PII substitute for upstream key-manager encryption.
     *
     *  Production AES-GCM / vault encryption needs external certified infrastructure;
     *  the local stand-in wraps the value with AesWrap using a key resolved from the
     *  injected KeyStore, falling back to a fixed local key. Ciphertext is hex framed
     *  with "enc1:" so legacy plaintext rows remain readable.
     */
    [[nodiscard]] std::string pii_key() const
    {
        if (m_context) {
            const auto secret = m_context->key_store().get("customer_pii", "v1");
            if (secret) { return secret->value(); }
        }
        return std::string{"customer-pii-local-substitute"};
    }

    [[nodiscard]] std::string encrypt_pii(std::string_view value) const
    {
        if (value.empty()) { return {}; }
        const std::string cipher = ::app::crypto::AesWrap{}.encrypt(value, pii_key());
        std::string out{"enc1:"};
        out.reserve(out.size() + cipher.size() * 2U);
        constexpr std::string_view digits{"0123456789abcdef"};
        for (const char ch : cipher) {
            const auto byte = static_cast<unsigned char>(ch);
            out.push_back(digits[(byte >> 4U) & 0xFU]);
            out.push_back(digits[byte & 0xFU]);
        }
        return out;
    }

    [[nodiscard]] std::string decrypt_pii(std::string_view value) const
    {
        constexpr std::string_view marker{"enc1:"};
        if (!value.starts_with(marker)) { return std::string{value}; }
        const std::string_view encoded = value.substr(marker.size());
        std::string raw{};
        raw.reserve(encoded.size() / 2U);
        const auto nibble = [](char ch) -> int
        {
            if (ch >= '0' && ch <= '9') { return ch - '0'; }
            if (ch >= 'a' && ch <= 'f') { return ch - 'a' + 10; }
            if (ch >= 'A' && ch <= 'F') { return ch - 'A' + 10; }
            return -1;
        };
        for (std::size_t i = 0; i + 1 < encoded.size(); i += 2) {
            const int high = nibble(encoded[i]);
            const int low = nibble(encoded[i + 1]);
            if (high < 0 || low < 0) { return std::string{value}; }
            raw.push_back(static_cast<char>((high << 4) | low));
        }
        return ::app::crypto::AesWrap{}.decrypt(raw, pii_key());
    }

    /** @brief Decrypted, typed response from a persisted customer record. */
    [[nodiscard]] ::app::model::customers::CustomerResponse to_customer_response(
        const ::app::store::customers::CustomerRecord& record
    ) const
    {
        ::app::model::customers::CustomerResponse response;
        ::app::model::customers::CustomerIdValue customer_id;
        customer_id.set_value(record.get_customer_id());
        response.set_customer_id(std::move(customer_id));
        ::app::model::customers::OptionalEncryptableNameValue name;
        name.set_value(decrypt_pii(record.get_name()));
        response.set_name(std::move(name));
        ::app::model::customers::OptionalEncryptableEmailValue email;
        email.set_value(decrypt_pii(record.get_email()));
        response.set_email(std::move(email));
        ::app::model::customers::OptionalEncryptablePhoneValue phone;
        phone.set_value(decrypt_pii(record.get_phone()));
        response.set_phone(std::move(phone));
        ::app::model::customers::PrimitiveDateTimeValue created_at;
        created_at.set_value(record.get_created_at());
        response.set_created_at(std::move(created_at));
        ::app::model::customers::OptionalEncryptableSecretStringValue tax_id;
        response.set_tax_registration_id(std::move(tax_id));
        if (!record.get_description().empty()) {
            ::app::model::customers::DescriptionValue description;
            description.set_value(record.get_description());
            response.set_description(std::move(description));
        }
        if (!record.get_metadata().empty()) {
            ::app::model::customers::SecretSerdeValueValue metadata;
            metadata.set_value(record.get_metadata());
            response.set_metadata(std::move(metadata));
        }
        return response;
    }

    [[nodiscard]] std::optional<std::pair<std::string, std::string>> parse_time_range(
        std::string_view raw
    ) const
    {
        std::string value{trim(raw)};
        if (value.size() >= 2U && value.front() == '[' && value.back() == ']') {
            value = value.substr(1, value.size() - 2);
        }
        const std::size_t comma = value.find(',');
        if (comma == std::string::npos) { return std::nullopt; }
        const std::string start = trim(value.substr(0, comma));
        const std::string end = trim(value.substr(comma + 1));
        if (start.empty() || end.empty()) { return std::nullopt; }
        return std::pair<std::string, std::string>{start, end};
    }

    [[nodiscard]] bool within_range(
        std::string_view created_at,
        const std::pair<std::string, std::string>& range
    ) const noexcept
    {
        return created_at >= range.first && created_at <= range.second;
    }

    [[nodiscard]] std::string trim(std::string_view value) const
    {
        std::size_t begin{0};
        std::size_t end{value.size()};
        while (begin < end && std::isspace(static_cast<unsigned char>(value[begin]))) { ++begin; }
        while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) { --end; }
        if (end > begin && value[begin] == '"' && value[end - 1] == '"') {
            ++begin;
            --end;
        }
        return std::string{value.substr(begin, end - begin)};
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        ::core::error::AppError error_value
    ) const
    {
        auto text = serde::Ser::encode_json(error_value);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(::core::error::AppErrorStatus{}(error_value.get_kind()));
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

} // namespace app::handlers::customers
