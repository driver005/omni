export module app_routing:blocklist_handlers_entries;

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

} // namespace app::handlers::blocklist
