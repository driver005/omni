export module app_handlers_customers:handlers_v2;

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
import app_metrics;

import :input;
import :result;
import :runtime;
import :handlers_v2_global_id;

export namespace app::handlers::customers {

class CustomersHandlersV2
{
public:
    CustomersHandlersV2(
        app::context::AppContext& context,
        CustomersRuntime& runtime,
        CustomersInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_global_id{context, runtime, input_support}
    {
    }

    void handle_customers_create_v2(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const bool jwt = m_input_support.is_jwt_auth(req);
        if (const auto denied = m_input_support.authorize(
                req,
                ::app::auth::AppPermission::USER_WRITE,
                jwt,
                true,
                false,
                false,
                false,
                true,
                true
            )) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const std::string body = req.body_as_string();
        if (body.empty()) {
            const auto error =
                m_input_support.upstream_error("InvalidRequestData", "empty request");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            const auto error = m_input_support.upstream_error("InvalidRequestData", "invalid JSON");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const CustomerCreateInput request =
            m_input_support.parse_create_input(JsonValueView{*parsed}, ApiVersion::V2);
        if (const auto invalid =
                m_input_support.validate_document_details(request.document_details)) {
            const auto& error = *invalid;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        if (const auto invalid =
                m_input_support.validate_phone_country_code(request.phone_country_code)) {
            const auto& error = *invalid;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        if (!request.name || !request.email) {
            const auto error =
                m_input_support.upstream_error("InvalidRequestData", "name and email are required");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        CustomersCreateRouteInput input{
            request,
            ApiVersion::V2,
            m_input_support.request_merchant_id(req)
        };
        auto result = m_runtime.create_v2(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    void handle_customers_retrieve_v2(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const bool jwt = m_input_support.is_jwt_auth(req);
        if (const auto denied = m_input_support.authorize(
                req,
                ::app::auth::AppPermission::USER_READ,
                jwt,
                true,
                true,
                true,
                false,
                true,
                true
            )) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const std::string id = m_input_support.path_segment(req);
        if (id.empty()) {
            const auto error =
                m_input_support.upstream_error("InvalidRequestData", "missing customer id");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        CustomersRetrieveRouteInput input{
            id,
            ApiVersion::V2,
            m_input_support.request_merchant_id(req)
        };
        auto result = m_runtime.retrieve_v2(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    void handle_customers_retrieve_by_merchant_reference_id(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize(
                req,
                ::app::auth::AppPermission::USER_READ,
                false,
                true,
                false,
                false,
                false,
                true,
                true
            )) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const std::string reference = m_input_support.path_segment(req);
        if (reference.empty()) {
            const auto error = m_input_support.upstream_error(
                "InvalidRequestData",
                "missing merchant reference id"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        CustomersRetrieveByReferenceRouteInput input{
            reference,
            m_input_support.request_merchant_id(req)
        };
        auto result = m_runtime.retrieve_by_merchant_reference_id(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    void handle_customers_list_v2(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize(
                req,
                ::app::auth::AppPermission::USER_READ,
                m_input_support.is_jwt_auth(req),
                true,
                false,
                false,
                false,
                true,
                true
            )) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
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
            ApiVersion::V2,
            m_input_support.request_merchant_id(req),
            offset,
            limit,
            customer_id
        };
        auto result = m_runtime.list_v2(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    void handle_customers_list_with_count_v2(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize(
                req,
                ::app::auth::AppPermission::USER_READ,
                m_input_support.is_jwt_auth(req),
                true,
                false,
                false,
                false,
                true,
                true
            )) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
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
            ApiVersion::V2,
            m_input_support.request_merchant_id(req),
            off,
            lim,
            customer_id,
            std::move(time_range)
        };
        auto result = m_runtime.list_with_count_v2(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    void handle_customers_update_v2(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const bool jwt = m_input_support.is_jwt_auth(req);
        if (const auto denied = m_input_support.authorize(
                req,
                ::app::auth::AppPermission::USER_WRITE,
                jwt,
                true,
                false,
                false,
                false,
                true,
                true
            )) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const std::string id = m_input_support.path_segment(req);
        if (id.empty()) {
            const auto error =
                m_input_support.upstream_error("InvalidRequestData", "missing customer id");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const std::string body = req.body_as_string();
        if (body.empty()) {
            const auto error =
                m_input_support.upstream_error("InvalidRequestData", "empty request");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            const auto error = m_input_support.upstream_error("InvalidRequestData", "invalid JSON");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const CustomerUpdateInput request =
            m_input_support.parse_update_input(JsonValueView{*parsed});
        if (const auto invalid =
                m_input_support.validate_document_details(request.document_details)) {
            const auto& error = *invalid;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        if (const auto invalid =
                m_input_support.validate_phone_country_code(request.phone_country_code)) {
            const auto& error = *invalid;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        CustomersUpdateRouteInput
            input{request, id, ApiVersion::V2, m_input_support.request_merchant_id(req)};
        auto result = m_runtime.update_v2(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    void handle_customers_delete_v2(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const bool jwt = m_input_support.is_jwt_auth(req);
        if (const auto denied = m_input_support.authorize(
                req,
                ::app::auth::AppPermission::USER_WRITE,
                jwt,
                true,
                false,
                false,
                false,
                true,
                true
            )) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const std::string id = m_input_support.path_segment(req);
        if (id.empty()) {
            const auto error =
                m_input_support.upstream_error("InvalidRequestData", "missing customer id");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        CustomersDeleteRouteInput input{
            id,
            ApiVersion::V2,
            m_input_support.request_merchant_id(req)
        };
        auto result = m_runtime.delete_customer_v2(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    void install(core::router::RouterContext<>& router, std::string_view base)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // app.rs:1545-1586. The C++-only v1 `POST /{id}` alias and v1-only
        // `GET /{id}/mandates` are deliberately not mounted on the v2 scope.
        (void)Router{router, base}
            .add_route(
                Route{"list"}
                    .get(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_customers_list_v2(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch customers_list (v2)")
            )
            .add_route(
                Route{"list_with_count"}
                    .get(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_customers_list_with_count_v2(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch customers_list_with_count (v2)")
            )
            .add_router(m_global_id.make_router(router))
            .add_router(
                Router{router, "reference"}.add_route(
                    Route{":merchant_reference_id"}
                        .get(
                            [this](auto& req, auto& res, auto send)
                            {
                                handle_customers_retrieve_by_merchant_reference_id(
                                    req,
                                    res,
                                    std::move(send)
                                );
                            }
                        )
                        .summary("Hyperswitch customers_retrieve_by_merchant_reference_id")
                )
            )
            .add_route(
                Route{""}
                    .post(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_customers_create_v2(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch customers_create (v2)")
            )
            .add_route(
                Route{":id"}
                    .get(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_customers_retrieve_v2(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch customers_retrieve (v2)")
            )
            .add_route(
                Route{":id"}
                    .put(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_customers_update_v2(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch customers_update (v2)")
            )
            .add_route(
                Route{":id"}
                    .delt(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_customers_delete_v2(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch customers_delete (v2)")
            );
    }

private:
    app::context::AppContext& m_context;
    CustomersRuntime& m_runtime;
    CustomersInputSupport& m_input_support;
    CustomersHandlersV2GlobalId m_global_id;
};

} // namespace app::handlers::customers
