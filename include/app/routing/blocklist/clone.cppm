export module app_routing:blocklist_handlers_clone;

import std;
import interfaces;
import core_router;
import core_behavior;
import core_serde;
import utils_openapi;
import app_model;
import app_context;
import core_error;
import app_http;

import :blocklist_input;
import :blocklist_result;
import :blocklist_runtime;

export namespace app::handlers::blocklist {

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

} // namespace app::handlers::blocklist
