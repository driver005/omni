export module app_user:customers_handlers_v1;

import std;
import interfaces;
import core_behavior;
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
import app_metrics;

import :customers_input;
import :customers_runtime;
import :customers_handlers_v1_mandates;

export namespace app::handlers::customers {

class CustomersHandlersV1
{
public:
    CustomersHandlersV1(
        ::app::context::AppContext& context,
        CustomersRuntime& runtime,
        CustomersInputSupport& input_support,
        CustomersRequestSupport& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_request_support{request_support},
        m_mandates{context, runtime, input_support, request_support}
    {
    }

    void handle_customers_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_request_support.authorize(
                req,
                "customers_create",
                ::core::behavior::AuthSpec{
                    .permission = "USER_WRITE",
                    .allow_connected = true,
                    .allow_platform = true
                }
            )) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        if (const auto denied = m_input_support.validate_legacy_endpoint_access(req)) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const std::string body = req.body_as_string();
        if (body.empty()) {
            const auto error =
                m_input_support.upstream_error("InvalidRequestData", "empty request");
            error.send(res, send);
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            const auto error = m_input_support.upstream_error("InvalidRequestData", "invalid JSON");
            error.send(res, send);
            return;
        }
        const CustomerCreateInput request =
            m_input_support.parse_create_input(JsonValueView{*parsed}, ApiVersion::V1);
        if (const auto invalid =
                m_input_support.validate_document_details(request.document_details)) {
            const auto& error = *invalid;
            error.send(res, send);
            return;
        }
        if (const auto invalid =
                m_input_support.validate_phone_country_code(request.phone_country_code)) {
            const auto& error = *invalid;
            error.send(res, send);
            return;
        }
        CustomersCreateRouteInput input{
            request,
            ApiVersion::V1,
            m_input_support.request_merchant_id(req)
        };
        auto result = m_runtime.create(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_customers_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_request_support.authorize(
                req,
                "customers_retrieve",
                ::core::behavior::AuthSpec{
                    .permission = "USER_READ",
                    .allow_connected = true,
                    .allow_platform = true
                }
            )) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        if (const auto denied = m_input_support.validate_legacy_endpoint_access(req)) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const std::string id = m_request_support.path(req, CustomersPolicy::path_index(req, 0));
        if (id.empty()) {
            const auto error =
                m_input_support.upstream_error("InvalidRequestData", "missing customer id");
            error.send(res, send);
            return;
        }
        CustomersRetrieveRouteInput input{
            id,
            ApiVersion::V1,
            m_input_support.request_merchant_id(req)
        };
        auto result = m_runtime.retrieve(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_customers_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_request_support.authorize(
                req,
                "customers_list",
                ::core::behavior::AuthSpec{
                    .permission = "USER_READ",
                    .allow_connected = true,
                    .allow_platform = true
                }
            )) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto query = ::app::http::RequestQuery{}(req);
        std::optional<std::uint32_t> offset{};
        std::optional<std::uint16_t> limit{};
        std::optional<std::string> customer_id{};
        if (const auto value = query.get("offset")) {
            offset = m_input_support.parse_uint32(*value);
        }
        if (const auto value = query.get("limit")) {
            limit = m_input_support.parse_uint16(*value);
        }
        if (const auto value = query.get("customer_id")) {
            customer_id = *value;
        }
        CustomersListRouteInput input{
            ApiVersion::V1,
            m_input_support.request_merchant_id(req),
            offset,
            limit,
            customer_id
        };
        auto result = m_runtime.list(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_customers_list_with_count(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_request_support.authorize(
                req,
                "customers_list_with_count",
                ::core::behavior::AuthSpec{
                    .permission = "USER_READ",
                    .allow_connected = true,
                    .allow_platform = true
                }
            )) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto query = ::app::http::RequestQuery{}(req);
        const std::size_t off = query.get("offset")
                                    ? m_input_support.parse_uint32(*query.get("offset")).value_or(0)
                                    : 0U;
        std::size_t lim = kDefaultCustomerListLimit;
        if (const auto value = query.get("limit")) {
            lim = m_input_support.parse_size(*value).value_or(lim);
        }
        std::optional<std::string> customer_id{};
        if (const auto value = query.get("customer_id")) {
            customer_id = *value;
        }
        std::optional<::app::model::customers::CustomerTimeRange> time_range{};
        auto start = query.get("start_time");
        if (!start) {
            start = query.get("startTime");
        }
        if (start) {
            ::app::model::customers::CustomerTimeRange range{};
            range.set_start_time(*start);
            auto end = query.get("end_time");
            if (!end) {
                end = query.get("endTime");
            }
            if (end) {
                range.set_end_time(*end);
            }
            time_range = std::move(range);
        }
        CustomersListWithCountRouteInput input{
            ApiVersion::V1,
            m_input_support.request_merchant_id(req),
            off,
            lim,
            customer_id,
            std::move(time_range)
        };
        auto result = m_runtime.list_with_count(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_customers_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_request_support.authorize(
                req,
                "customers_update",
                ::core::behavior::AuthSpec{
                    .permission = "USER_WRITE",
                    .allow_connected = true,
                    .allow_platform = true
                }
            )) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        if (const auto denied = m_input_support.validate_legacy_endpoint_access(req)) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const std::string id = m_request_support.path(req, CustomersPolicy::path_index(req, 0));
        if (id.empty()) {
            const auto error =
                m_input_support.upstream_error("InvalidRequestData", "missing customer id");
            error.send(res, send);
            return;
        }
        const std::string body = req.body_as_string();
        if (body.empty()) {
            const auto error =
                m_input_support.upstream_error("InvalidRequestData", "empty request");
            error.send(res, send);
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            const auto error = m_input_support.upstream_error("InvalidRequestData", "invalid JSON");
            error.send(res, send);
            return;
        }
        const CustomerUpdateInput request =
            m_input_support.parse_update_input(JsonValueView{*parsed});
        if (const auto invalid =
                m_input_support.validate_document_details(request.document_details)) {
            const auto& error = *invalid;
            error.send(res, send);
            return;
        }
        if (const auto invalid =
                m_input_support.validate_phone_country_code(request.phone_country_code)) {
            const auto& error = *invalid;
            error.send(res, send);
            return;
        }
        CustomersUpdateRouteInput
            input{request, id, ApiVersion::V1, m_input_support.request_merchant_id(req)};
        auto result = m_runtime.update(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_customers_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_request_support.authorize(
                req,
                "customers_delete",
                ::core::behavior::AuthSpec{
                    .permission = "USER_WRITE",
                    .allow_connected = true,
                    .allow_platform = true
                }
            )) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        if (const auto denied = m_input_support.validate_legacy_endpoint_access(req)) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const std::string id = m_request_support.path(req, CustomersPolicy::path_index(req, 0));
        if (id.empty()) {
            const auto error =
                m_input_support.upstream_error("InvalidRequestData", "missing customer id");
            error.send(res, send);
            return;
        }
        CustomersDeleteRouteInput input{
            id,
            ApiVersion::V1,
            m_input_support.request_merchant_id(req)
        };
        auto result = m_runtime.delete_customer(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // app.rs:1593-1632. Multi-segment resources are mounted before "/{customer_id}".
        (void)Router{router, "customers"}
            .add_route(
                Route{"list"}
                    .get(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_customers_list(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch customers_list")
            )
            .add_route(
                Route{"list_with_count"}
                    .get(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_customers_list_with_count(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch customers_list_with_count")
            )
            .add_router(m_mandates.make_router(router))
            .add_route(
                Route{""}
                    .post(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_customers_create(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch customers_create")
            )
            .add_route(
                Route{":customer_id"}
                    .get(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_customers_retrieve(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch customers_retrieve")
            )
            .add_route(
                Route{":customer_id"}
                    .post(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_customers_update(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch customers_update")
            )
            .add_route(
                Route{":customer_id"}
                    .delt(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_customers_delete(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch customers_delete")
            );
    }

private:
    ::app::context::AppContext& m_context;
    CustomersRuntime& m_runtime;
    CustomersInputSupport& m_input_support;
    CustomersRequestSupport& m_request_support;
    CustomersHandlersV1Mandates m_mandates;
};

} // namespace app::handlers::customers
