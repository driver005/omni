export module app_handlers_cards_info:handler;

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
import app_metrics;
import app_store;

import :input;
import :result;
import :runtime;

export namespace app::handlers::cards_info {

class CardsInfoRouteHandlers
{
public:
    void handle_card_iin_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream routes/cards_info.rs:42-47 runs check_sdk_auth_and_get_auth
        // (SDK authorization header or api_key/publishable-key) before server_wrap.
        const auto auth_context = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error =
                m_input_support.authorize_operation(auth_context, "card_iin_info", true)) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // Upstream GET /cards/{bin}: IIN is a path segment, client_secret a query param.
        const auto bin = m_input_support.bin_from_path(req);
        if (!bin) {
            m_input_support.record_flow("CardsInfo");
            auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "The provided card IIN does not exist"
            );
            error.set_kind_name("InvalidCardIin");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        app::model::cards_info::CardsInfoRequest payload;
        payload.set_card_iin(std::string{*bin});
        if (const auto client_secret = m_input_support.client_secret_from_query(req)) {
            payload.set_client_secret(*client_secret);
        }
        // Upstream routes/cards_info.rs:55-57 overwrites req.client_secret from the
        // authenticated credential. The AuthContext carries that authenticated claim.
        if (const auto authenticated_secret = auth_context.claim(::app::auth::kClaimClientSecret)) {
            payload.set_client_secret(std::string{*authenticated_secret});
        }

        CardIinInfoInput input{payload};
        auto result = m_runtime.card_iin_info(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_create_cards_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error =
                m_input_support.authorize_operation(auth_context, "create_cards_info")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            m_input_support.record_flow("CardsInfoCreate");
            auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Json payload deserialization failed: empty body"
            );
            error.set_kind_name("InvalidRequestData");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        using Input = app::model::cards_info::CardInfoCreateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            m_input_support.record_flow("CardsInfoCreate");
            auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Json payload deserialization failed"
            );
            error.set_kind_name("InvalidRequestData");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        CreateCardsInfoInput input{*parsed};
        auto result = m_runtime.create(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_update_cards_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error =
                m_input_support.authorize_operation(auth_context, "update_cards_info")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            m_input_support.record_flow("CardsInfoUpdate");
            auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Json payload deserialization failed: empty body"
            );
            error.set_kind_name("InvalidRequestData");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        using Input = app::model::cards_info::CardInfoUpdateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            m_input_support.record_flow("CardsInfoUpdate");
            auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Json payload deserialization failed"
            );
            error.set_kind_name("InvalidRequestData");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        UpdateCardsInfoInput input{*parsed};
        auto result = m_runtime.update(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_migrate_cards_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error =
                m_input_support.authorize_operation(auth_context, "migrate_cards_info")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            m_input_support.record_flow("CardsInfoMigrate");
            auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Multipart form deserialization failed: empty body"
            );
            error.set_kind_name("InvalidRequestData");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        std::vector<app::model::cards_info::CardInfoUpdateRequest> records;
        if (const auto csv_bytes = m_input_support.multipart_file_bytes(req, body)) {
            // Upstream CardsInfoUpdateForm caps the multipart file at 1MB and a CSV
            // parse failure maps to PreconditionFailed (core/cards_info.rs:127-157).
            if (csv_bytes->size() > (1'024u * 1'024u)) {
                m_input_support.record_flow("CardsInfoMigrate");
                auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "cards bin CSV exceeds 1MB limit"
                );
                error.set_kind_name("PreconditionFailed");
                res.set_status(error.get_status());
                res.set_body(serde::Ser::serialize("application/json", error));
                send();
                return;
            }
            auto parsed_csv = m_input_support.parse_cards_csv(*csv_bytes);
            if (!parsed_csv) {
                m_input_support.record_flow("CardsInfoMigrate");
                auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "failed to parse cards bin CSV"
                );
                error.set_kind_name("PreconditionFailed");
                res.set_status(error.get_status());
                res.set_body(serde::Ser::serialize("application/json", error));
                send();
                return;
            }
            records = std::move(*parsed_csv);
        } else {
            using Input = std::vector<app::model::cards_info::CardInfoUpdateRequest>;
            auto parsed = serde::Ser::deserialize<Input>("application/json", body);
            if (!parsed) {
                m_input_support.record_flow("CardsInfoMigrate");
                auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::VALIDATION_FAILED,
                    "Json payload deserialization failed"
                );
                error.set_kind_name("InvalidRequestData");
                res.set_status(error.get_status());
                res.set_body(serde::Ser::serialize("application/json", error));
                send();
                return;
            }
            records = std::move(*parsed);
            // Upstream assigns a 1-based line_number from a row counter for every
            // record (core/cards_info.rs:138-143); caller input is never trusted.
            for (std::size_t i{0}; i < records.size(); ++i) {
                records[i].set_line_number(static_cast<std::int64_t>(i + 1));
            }
        }

        MigrateCardsInfoInput input{std::move(records)};
        auto result = m_runtime.migrate(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_items()));
        send();
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "cards"}
            .add_route(
                Route{":bin"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_card_iin_info(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch card_iin_info")
            )
            .add_route(
                Route{"create"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_create_cards_info(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch create_cards_info")
            )
            .add_route(
                Route{"update"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_update_cards_info(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch update_cards_info")
            )
            .add_route(
                Route{"update-batch"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_migrate_cards_info(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch migrate_cards_info")
            );
    }

    explicit CardsInfoRouteHandlers(app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context}
    {
    }

private:
    CardsInfoRuntime m_runtime;
    CardsInfoInputSupport m_input_support;
};

} // namespace app::handlers::cards_info
