export module app_routing:blocklist_handlers_batch;

import std;
import interfaces;
import core_router;
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

} // namespace app::handlers::blocklist
