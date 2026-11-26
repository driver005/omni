export module app_routing:blocklist_handlers_lookup_count;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_context;
import core_error;
import app_http;

import :blocklist_input;
import :blocklist_result;
import :blocklist_runtime;

export namespace app::handlers::blocklist {

class LookupCountHandlers
{
public:
    LookupCountHandlers(::app::context::AppContext& ctx, BlocklistRuntime& runtime) noexcept :
        m_context{ctx},
        m_input_support{ctx},
        m_runtime{runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "blocklist"}
            .add_route(
                Route{""}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_list_blocked_payment_methods(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch list_blocked_payment_methods")
            )
            .add_route(
                Route{"count"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_blocklist_count(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_blocklist_count")
            )
            .add_route(
                Route{"lookup"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_lookup_blocklist_entry(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch lookup_blocklist_entry")
            );
    }

    // routes/blocklist.rs:148-186 -> core/blocklist.rs:40-54 ->
    // utils::list_blocklist_entries_for_merchant (utils.rs:120-162).
    void handle_list_blocked_payment_methods(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied =
                m_input_support.authorize_operation(req, "list_blocked_payment_methods")) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }

        const auto merchant = m_input_support.merchant_id_of(req);
        if (const auto bad = m_input_support.require_merchant(merchant)) {
            const auto& error = *bad;
            error.send(res, send);
            return;
        }
        // list is merchant-wide when no profile resolves (routes.rs:162-163).
        const auto profile = m_input_support.explicit_profile_of(req);
        const auto query = ::app::http::RequestQuery{}(req);

        auto data_kind = query.get("data_kind");
        if (!data_kind) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing required param: data_kind"
            );
            error.send(res, send);
            return;
        }
        const auto kind = m_input_support.query_kind(*data_kind);
        if (!kind) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "unknown variant for data_kind"
            );
            error.send(res, send);
            return;
        }
        std::string transaction_type{"payment"};
        if (const auto raw = query.get("transaction_type")) {
            const auto parsed = m_input_support.parse_transaction_type(*raw);
            if (!parsed) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "unknown variant for transaction_type"
                );
                error.send(res, send);
                return;
            }
            transaction_type = *parsed;
        }
        const auto limit = m_input_support.query_number<std::uint16_t>(query, "limit").value_or(10);
        const auto offset =
            m_input_support.query_number<std::uint16_t>(query, "offset").value_or(0);

        ListBlockedPaymentMethodsInput
            input{merchant, profile, *kind, transaction_type, limit, offset};
        auto result = m_runtime.list_blocked_payment_methods(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // routes/blocklist.rs:207-242 -> core/blocklist.rs:56-65 ->
    // utils::get_blocklist_count (utils.rs:192-269).
    void handle_get_blocklist_count(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_input_support.authorize_operation(req, "get_blocklist_count")) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }

        const auto merchant = m_input_support.merchant_id_of(req);
        if (const auto bad = m_input_support.require_merchant(merchant)) {
            const auto& error = *bad;
            error.send(res, send);
            return;
        }
        const auto profile = m_input_support.resolve_required_profile(req, merchant);
        if (profile.failure) {
            const auto& error = *profile.failure;
            error.send(res, send);
            return;
        }
        const auto query = ::app::http::RequestQuery{}(req);

        auto raw_kind = query.get("data_kind");
        if (!raw_kind) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing required param: data_kind"
            );
            error.send(res, send);
            return;
        }
        const auto kind = m_input_support.query_kind(*raw_kind);
        if (!kind) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "unknown variant for data_kind"
            );
            error.send(res, send);
            return;
        }
        std::string transaction_type; // empty = both, including legacy NULL (api_models:109-112).
        if (const auto raw = query.get("transaction_type")) {
            const auto parsed = m_input_support.parse_transaction_type(*raw);
            if (!parsed) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "unknown variant for transaction_type"
                );
                error.send(res, send);
                return;
            }
            transaction_type = *parsed;
        }

        GetBlocklistCountInput input{merchant, profile.id, *kind, transaction_type};
        auto result = m_runtime.get_blocklist_count(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // routes/blocklist.rs:263-298 -> core/blocklist.rs:67-76 ->
    // utils::lookup_blocklist_entry (utils.rs:271-314).
    void handle_lookup_blocklist_entry(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied =
                m_input_support.authorize_operation(req, "lookup_blocklist_entry")) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }

        const auto merchant = m_input_support.merchant_id_of(req);
        if (const auto bad = m_input_support.require_merchant(merchant)) {
            const auto& error = *bad;
            error.send(res, send);
            return;
        }
        const auto profile = m_input_support.resolve_required_profile(req, merchant);
        if (profile.failure) {
            const auto& error = *profile.failure;
            error.send(res, send);
            return;
        }
        const auto query = ::app::http::RequestQuery{}(req);

        auto data = query.get("data");
        if (!data || data->empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Missing required param: data"
            );
            error.send(res, send);
            return;
        }
        // LengthString<20,1> enforced at deserialization (common_utils/types.rs:1066-1071).
        if (data->size() > 20) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "data contains invalid data. Expected format is a string of 1 to 20 characters"
            );
            error.send(res, send);
            return;
        }
        std::string transaction_type;
        if (const auto raw = query.get("transaction_type")) {
            const auto parsed = m_input_support.parse_transaction_type(*raw);
            if (!parsed) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "unknown variant for transaction_type"
                );
                error.send(res, send);
                return;
            }
            transaction_type = *parsed;
        }

        LookupBlocklistEntryInput input{merchant, profile.id, *data, transaction_type};
        auto result = m_runtime.lookup_blocklist_entry(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistInputSupport m_input_support;
    BlocklistRuntime& m_runtime;
};

} // namespace app::handlers::blocklist
