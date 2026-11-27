export module app_routing:blocklist_router;

import std;
import interfaces;
import core_router;
import core_behavior;
import core_serde;
import app_model;
import utils_openapi;
import app_context;
import core_error;
import app_http;

import :blocklist_input;
import :blocklist_result;
import :blocklist_runtime;

export namespace app::handlers::blocklist {


class EntriesHandlers
{
public:
    EntriesHandlers(
        ::app::context::AppContext& ctx,
        BlocklistRuntime& runtime,
        BlocklistInputSupport& input_support,
        BlocklistRequestSupport& request_support
    ) noexcept :
        m_context{ctx},
        m_input_support{input_support},
        m_request_support{request_support},
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
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_add_entry_to_blocklist(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch add_entry_to_blocklist")
            )
            .add_route(
                Route{""}
                    .delt(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_remove_entry_from_blocklist(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch remove_entry_from_blocklist")
            );
    }

    // routes/blocklist.rs:30-68 -> core/blocklist.rs:16-26 ->
    // utils::insert_entry_into_blocklist (utils.rs:316-433).
    void handle_add_entry_to_blocklist(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied = m_request_support.authorize(req, "add_entry_to_blocklist", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(sink);
            return;
        }

        auto parsed = m_input_support.parse_entry_body(body);
        if (parsed.failure) {
            const auto& error = *parsed.failure;
            error.send(sink);
            return;
        }

        const auto merchant = m_input_support.merchant_id_of(req);
        if (const auto bad = m_input_support.require_merchant(merchant)) {
            const auto& error = *bad;
            error.send(sink);
            return;
        }
        const auto profile = m_input_support.resolve_required_profile(req, merchant);
        if (profile.failure) {
            const auto& error = *profile.failure;
            error.send(sink);
            return;
        }

        auto ctx = m_request_support.context(req);
        ctx.merchant_id = merchant;
        ctx.profile_id = profile.id;
        m_runtime.add_entry(parsed, ctx).handle(sink);
    }

    // routes/blocklist.rs:89-127 -> core/blocklist.rs:28-38 ->
    // utils::delete_entry_from_blocklist (utils.rs:25-74, 506-525).
    void handle_remove_entry_from_blocklist(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support.authorize(req, "remove_entry_from_blocklist", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(sink);
            return;
        }

        auto parsed = m_input_support.parse_entry_body(body);
        if (parsed.failure) {
            const auto& error = *parsed.failure;
            error.send(sink);
            return;
        }

        const auto merchant = m_input_support.merchant_id_of(req);
        if (const auto bad = m_input_support.require_merchant(merchant)) {
            const auto& error = *bad;
            error.send(sink);
            return;
        }
        const auto profile = m_input_support.resolve_required_profile(req, merchant);
        if (profile.failure) {
            const auto& error = *profile.failure;
            error.send(sink);
            return;
        }

        auto ctx = m_request_support.context(req);
        ctx.merchant_id = merchant;
        ctx.profile_id = profile.id;
        m_runtime.remove_entry(parsed, ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistInputSupport& m_input_support;

    BlocklistRequestSupport& m_request_support;
    BlocklistRuntime& m_runtime;
};

class GuardHandlers
{
public:
    GuardHandlers(
        ::app::context::AppContext& ctx,
        BlocklistRuntime& runtime,
        BlocklistInputSupport& input_support,
        BlocklistRequestSupport& request_support
    ) noexcept :
        m_context{ctx},
        m_input_support{input_support},
        m_request_support{request_support},
        m_runtime{runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "blocklist"}.add_route(
            Route{"toggle"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_toggle_blocklist_guard(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch toggle_blocklist_guard")
        );
    }

    // routes/blocklist.rs:314-343 -> core/blocklist.rs:78-86 ->
    // utils::toggle_blocklist_guard_for_merchant (utils.rs:76-118). Merchant-wide, no profile.
    void handle_toggle_blocklist_guard(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied = m_request_support.authorize(req, "toggle_blocklist_guard", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        const auto query = ::app::http::RequestQuery{}(req);
        auto raw = query.get("status");
        if (!raw) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing required param: status"
            );
            error.send(sink);
            return;
        }
        bool status{false};
        if (*raw == "true") {
            status = true;
        } else if (*raw != "false") {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "invalid bool value for status"
            );
            error.send(sink);
            return;
        }

        const auto merchant = m_input_support.merchant_id_of(req);
        if (const auto bad = m_input_support.require_merchant(merchant)) {
            const auto& error = *bad;
            error.send(sink);
            return;
        }

        auto ctx = m_request_support.context(req);
        ctx.merchant_id = merchant;
        ctx.query["status"] = status ? "true" : "false";
        m_runtime.toggle_blocklist_guard(ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistInputSupport& m_input_support;

    BlocklistRequestSupport& m_request_support;
    BlocklistRuntime& m_runtime;
};

class LookupCountHandlers
{
public:
    LookupCountHandlers(
        ::app::context::AppContext& ctx,
        BlocklistRuntime& runtime,
        BlocklistInputSupport& input_support,
        BlocklistRequestSupport& request_support
    ) noexcept :
        m_context{ctx},
        m_input_support{input_support},
        m_request_support{request_support},
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
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support.authorize(req, "list_blocked_payment_methods", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        const auto merchant = m_input_support.merchant_id_of(req);
        if (const auto bad = m_input_support.require_merchant(merchant)) {
            const auto& error = *bad;
            error.send(sink);
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
            error.send(sink);
            return;
        }
        const auto kind = m_input_support.query_kind(*data_kind);
        if (!kind) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "unknown variant for data_kind"
            );
            error.send(sink);
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
                error.send(sink);
                return;
            }
            transaction_type = *parsed;
        }
        const auto limit = m_input_support.query_number<std::uint16_t>(query, "limit").value_or(10);
        const auto offset =
            m_input_support.query_number<std::uint16_t>(query, "offset").value_or(0);

        (void)profile;
        (void)kind;
        (void)transaction_type;
        (void)limit;
        (void)offset;
        auto ctx = m_request_support.context(req);
        ctx.merchant_id = merchant;
        m_runtime.list_blocked_payment_methods(ctx).handle(sink);
    }

    // routes/blocklist.rs:207-242 -> core/blocklist.rs:56-65 ->
    // utils::get_blocklist_count (utils.rs:192-269).
    void handle_get_blocklist_count(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied = m_request_support.authorize(req, "get_blocklist_count", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        const auto merchant = m_input_support.merchant_id_of(req);
        if (const auto bad = m_input_support.require_merchant(merchant)) {
            const auto& error = *bad;
            error.send(sink);
            return;
        }
        const auto profile = m_input_support.resolve_required_profile(req, merchant);
        if (profile.failure) {
            const auto& error = *profile.failure;
            error.send(sink);
            return;
        }
        const auto query = ::app::http::RequestQuery{}(req);

        auto raw_kind = query.get("data_kind");
        if (!raw_kind) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing required param: data_kind"
            );
            error.send(sink);
            return;
        }
        const auto kind = m_input_support.query_kind(*raw_kind);
        if (!kind) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "unknown variant for data_kind"
            );
            error.send(sink);
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
                error.send(sink);
                return;
            }
            transaction_type = *parsed;
        }

        auto ctx = m_request_support.context(req);
        ctx.merchant_id = merchant;
        ctx.profile_id = profile.id;
        m_runtime.get_blocklist_count(ctx).handle(sink);
    }

    // routes/blocklist.rs:263-298 -> core/blocklist.rs:67-76 ->
    // utils::lookup_blocklist_entry (utils.rs:271-314).
    void handle_lookup_blocklist_entry(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied = m_request_support.authorize(req, "lookup_blocklist_entry", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        const auto merchant = m_input_support.merchant_id_of(req);
        if (const auto bad = m_input_support.require_merchant(merchant)) {
            const auto& error = *bad;
            error.send(sink);
            return;
        }
        const auto profile = m_input_support.resolve_required_profile(req, merchant);
        if (profile.failure) {
            const auto& error = *profile.failure;
            error.send(sink);
            return;
        }
        const auto query = ::app::http::RequestQuery{}(req);

        auto data = query.get("data");
        if (!data || data->empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Missing required param: data"
            );
            error.send(sink);
            return;
        }
        // LengthString<20,1> enforced at deserialization (common_utils/types.rs:1066-1071).
        if (data->size() > 20) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "data contains invalid data. Expected format is a string of 1 to 20 characters"
            );
            error.send(sink);
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
                error.send(sink);
                return;
            }
            transaction_type = *parsed;
        }

        auto ctx = m_request_support.context(req);
        ctx.merchant_id = merchant;
        ctx.profile_id = profile.id;
        m_runtime.lookup_blocklist_entry(ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistInputSupport& m_input_support;

    BlocklistRequestSupport& m_request_support;
    BlocklistRuntime& m_runtime;
};

class BatchHandlers
{
public:
    BatchHandlers(
        ::app::context::AppContext& ctx,
        BlocklistRuntime& runtime,
        BlocklistInputSupport& input_support,
        BlocklistRequestSupport& request_support
    ) noexcept :
        m_context{ctx},
        m_input_support{input_support},
        m_request_support{request_support},
        m_runtime{runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "blocklist"}
            .add_route(
                Route{"batch"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_upload_batch_blocklist(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch upload_batch_blocklist")
            )
            .add_route(
                Route{"batch"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_list_batch_blocklist_jobs(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch list_batch_blocklist_jobs")
            )
            .add_router(
                Router{router, "batch"}.add_route(
                    // core router supports only `:param`; upstream OpenAPI spells `{job_id}`.
                    Route{":job_id"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_get_batch_blocklist_job_status(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch get_batch_blocklist_job_status")
                )
            );
    }

    // routes/blocklist.rs:383-430 -> core/blocklist.rs:88-106 ->
    // batch::initiate_batch_blocklist_upload (batch.rs:348-510).
    void handle_upload_batch_blocklist(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied = m_request_support.authorize(req, "upload_batch_blocklist", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        const auto query = ::app::http::RequestQuery{}(req);
        std::string override_type;
        if (const auto raw = query.get("transaction_type")) {
            const auto parsed = m_input_support.parse_transaction_type(*raw);
            if (!parsed) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "unknown variant for transaction_type"
                );
                error.send(sink);
                return;
            }
            override_type = *parsed;
        }

        const auto merchant = m_input_support.merchant_id_of(req);
        if (const auto bad = m_input_support.require_merchant(merchant)) {
            const auto& error = *bad;
            error.send(sink);
            return;
        }
        const auto profile = m_input_support.resolve_required_profile(req, merchant);
        if (profile.failure) {
            const auto& error = *profile.failure;
            error.send(sink);
            return;
        }

        const auto body = req.body_as_string();
        auto upload = m_input_support.multipart_file(req, body); // extract `file` (MultipartBytes).
        // routes/blocklist.rs:351: #[multipart(limit = "5MiB")].
        if (upload.content.size() > 5U * 1'024U * 1'024U) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "file exceeds 5MiB limit"
            );
            error.send(sink);
            return;
        }

        auto parsed = m_input_support.parse_blocklist_csv(upload.content, override_type);
        if (parsed.failure) {
            const auto& error = *parsed.failure;
            error.send(sink);
            return;
        }

        auto ctx = m_request_support.context(req);
        ctx.merchant_id = merchant;
        ctx.profile_id = profile.id;
        m_runtime.upload_batch_blocklist(parsed.rows, upload.file_name, ctx).handle(sink);
    }

    // routes/blocklist.rs:446-477 -> core/blocklist.rs:108-122 ->
    // batch::get_batch_blocklist_job_status (batch.rs:553-590).
    void handle_get_batch_blocklist_job_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support.authorize(req, "get_batch_blocklist_job_status", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        const auto job_id = m_input_support.job_id_of(req);
        if (job_id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing job id"
            );
            error.send(sink);
            return;
        }
        const auto stored = m_input_support.find_job(m_input_support.job_key(job_id));
        if (!stored) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "Batch blocklist job `" + job_id + "` not found"
            );
            error.send(sink);
            return;
        }
        const auto merchant = m_input_support.merchant_id_of(req);
        if (const auto bad = m_input_support.require_merchant(merchant)) {
            const auto& error = *bad;
            error.send(sink);
            return;
        }
        const auto profile = m_input_support.explicit_profile_of(req);
        if (!m_input_support.job_scope_matches(*stored, merchant, profile)) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "Batch blocklist job `" + job_id + "` not found"
            );
            error.send(sink);
            return;
        }

        auto ctx = m_request_support.context(req);
        ctx.merchant_id = merchant;
        ctx.profile_id = profile;
        m_runtime.get_batch_blocklist_job_status(*stored, ctx).handle(sink);
    }

    // routes/blocklist.rs:498-528 -> core/blocklist.rs:124-138 ->
    // batch::list_batch_blocklist_jobs (batch.rs:657-697).
    void handle_list_batch_blocklist_jobs(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied = m_request_support.authorize(req, "list_batch_blocklist_jobs", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        const auto merchant = m_input_support.merchant_id_of(req);
        if (const auto bad = m_input_support.require_merchant(merchant)) {
            const auto& error = *bad;
            error.send(sink);
            return;
        }
        const auto profile = m_input_support.explicit_profile_of(req);
        const auto query = ::app::http::RequestQuery{}(req);
        const auto limit = m_input_support.query_number<std::uint32_t>(query, "limit").value_or(10);
        const auto offset =
            m_input_support.query_number<std::uint32_t>(query, "offset").value_or(0);
        std::string job_type;
        if (const auto raw = query.get("job_type")) {
            if (!m_input_support.known_job_type(*raw)) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "unknown variant for job_type"
                );
                error.send(sink);
                return;
            }
            job_type = *raw;
        }

        auto ctx = m_request_support.context(req);
        ctx.merchant_id = merchant;
        ctx.profile_id = profile;
        m_runtime.list_batch_blocklist_jobs(ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistInputSupport& m_input_support;

    BlocklistRequestSupport& m_request_support;
    BlocklistRuntime& m_runtime;
};

class ExportHandlers
{
public:
    ExportHandlers(
        ::app::context::AppContext& ctx,
        BlocklistRuntime& runtime,
        BlocklistInputSupport& input_support,
        BlocklistRequestSupport& request_support
    ) noexcept :
        m_context{ctx},
        m_input_support{input_support},
        m_request_support{request_support},
        m_runtime{runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "blocklist"}.add_route(
            Route{"export"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_create_blocklist_export(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch create_blocklist_export")
        );
    }

    // routes/blocklist.rs:549-584 -> core/blocklist.rs:140-149 ->
    // export::initiate_blocklist_export (export.rs:73-162).
    void handle_create_blocklist_export(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied = m_request_support.authorize(req, "create_blocklist_export", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        std::string transaction_type;
        const auto query = ::app::http::RequestQuery{}(req);
        if (const auto raw = query.get("transaction_type")) {
            const auto parsed = m_input_support.parse_transaction_type(*raw);
            if (!parsed) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "unknown variant for transaction_type"
                );
                error.send(sink);
                return;
            }
            transaction_type = *parsed;
        }

        const auto merchant = m_input_support.merchant_id_of(req);
        if (const auto bad = m_input_support.require_merchant(merchant)) {
            const auto& error = *bad;
            error.send(sink);
            return;
        }
        const auto profile = m_input_support.resolve_required_profile(req, merchant);
        if (profile.failure) {
            const auto& error = *profile.failure;
            error.send(sink);
            return;
        }

        auto ctx = m_request_support.context(req);
        ctx.merchant_id = merchant;
        ctx.profile_id = profile.id;
        m_runtime.create_blocklist_export(ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistInputSupport& m_input_support;

    BlocklistRequestSupport& m_request_support;
    BlocklistRuntime& m_runtime;
};

class CloneHandlers
{
public:
    CloneHandlers(
        ::app::context::AppContext& ctx,
        BlocklistRuntime& runtime,
        BlocklistInputSupport& input_support,
        BlocklistRequestSupport& request_support
    ) noexcept :
        m_context{ctx},
        m_input_support{input_support},
        m_request_support{request_support},
        m_runtime{runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "blocklist"}.add_route(
            Route{"clone"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_clone_blocklist_entries(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch clone_blocklist_entries")
        );
    }

    // routes/blocklist.rs:602-632 -> core/blocklist.rs:151-160 ->
    // clone::clone_blocklist_entries (clone.rs:68-109, 114-225).
    void handle_clone_blocklist_entries(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied = m_request_support.authorize(req, "clone_blocklist_entries", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        auto typed =
            core::behavior::Serializer<::app::model::blocklist::CloneBlocklistEntriesRequest>::pass(
                req,
                sink
            );
        if (!typed) {
            return;
        }

        const auto merchant = m_input_support.merchant_id_of(req);
        if (const auto bad = m_input_support.require_merchant(merchant)) {
            const auto& error = *bad;
            error.send(sink);
            return;
        }
        // clone.rs:76-80: source is auth.profile, no default-profile fallback here.
        const auto source = m_input_support.explicit_profile_of(req);
        if (source.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Missing required param: profile_id"
            );
            error.send(sink);
            return;
        }

        std::vector<::app::model::blocklist::ProfileIdValue> targets;
        for (const auto& target: typed->get_target_profile_ids()) {
            const auto id = target.get_value();
            if (std::find_if(
                    targets.begin(),
                    targets.end(),
                    [&id](const auto& item)
                    {
                        return item.get_value() == id;
                    }
                ) != targets.end()) {
                continue; // HashSet dedup.
            }
            targets.push_back(target);
        }
        // api_models:314-329 CloneBlocklistEntriesRequest::validate.
        if (targets.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "target_profile_ids must contain at least one profile"
            );
            error.send(sink);
            return;
        }
        if (std::any_of(
                targets.begin(),
                targets.end(),
                [&source](const auto& item)
                {
                    return item.get_value() == source;
                }
            )) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "target_profile_ids must not contain the source profile"
            );
            error.send(sink);
            return;
        }
        // clone.rs:26-63 resolve_clone_targets: any foreign profile fails the whole request.
        if (!m_input_support.profile_is_owned(source, merchant)) {
            const auto error = m_input_support.profile_not_found(source);
            error.send(sink);
            return;
        }
        for (const auto& target: targets) {
            if (!m_input_support.profile_is_owned(target.get_value(), merchant)) {
                const auto error = m_input_support.profile_not_found(target.get_value());
                error.send(sink);
                return;
            }
        }

        auto request = *typed;
        request.set_target_profile_ids(targets);
        auto ctx = m_request_support.context(req);
        ctx.merchant_id = merchant;
        ctx.profile_id = source;
        m_runtime.clone_blocklist_entries(request, ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistInputSupport& m_input_support;

    BlocklistRequestSupport& m_request_support;
    BlocklistRuntime& m_runtime;
};

// Facade ("handler of handlers"). Owns the single shared BlocklistRuntime; each
// family child holds a reference to it, never its own runtime instance.
class BlocklistRouteHandlers
{
public:
    explicit BlocklistRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_input_support{context},
        m_policy{m_input_support},
        m_request_support{m_policy},
        m_entries{context, m_runtime, m_input_support, m_request_support},
        m_guard{context, m_runtime, m_input_support, m_request_support},
        m_lookup_count{context, m_runtime, m_input_support, m_request_support},
        m_batch{context, m_runtime, m_input_support, m_request_support},
        m_export{context, m_runtime, m_input_support, m_request_support},
        m_clone{context, m_runtime, m_input_support, m_request_support}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        // Deterministic in-process substitute for the compile-time
        // `all(feature = "olap", feature = "v1")` gate at routes/app.rs:2053.
        if (const auto enabled = m_context.config_store().get_value("route_features", "blocklist");
            enabled.has_value() && *enabled == "false") {
            return;
        }

        m_entries.install(router);
        m_guard.install(router);
        m_lookup_count.install(router);
        m_batch.install(router);
        m_export.install(router);
        m_clone.install(router);
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistRuntime m_runtime;
    BlocklistInputSupport m_input_support;     // SINGLE parse/auth helper.
    BlocklistPolicy m_policy;                  // Domain auth/scope policy.
    BlocklistRequestSupport m_request_support; // Shared by all family handlers.
    EntriesHandlers m_entries;
    GuardHandlers m_guard;
    LookupCountHandlers m_lookup_count;
    BatchHandlers m_batch;
    ExportHandlers m_export;
    CloneHandlers m_clone;
};

} // namespace app::handlers::blocklist
