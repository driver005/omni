export module app_routing:blocklist_handlers_clone;

import std;
import interfaces;
import core_router;
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
    CloneHandlers(::app::context::AppContext& ctx, BlocklistRuntime& runtime) noexcept :
        m_context{ctx},
        m_input_support{ctx},
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
        if (const auto denied =
                m_input_support.authorize_operation(req, "clone_blocklist_entries")) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        using Input = ::app::model::blocklist::CloneBlocklistEntriesRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        if (!typed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }

        const auto merchant = m_input_support.merchant_id_of(req);
        if (const auto bad = m_input_support.require_merchant(merchant)) {
            const auto& error = *bad;
            error.send(res, send);
            return;
        }
        // clone.rs:76-80: source is auth.profile, no default-profile fallback here.
        const auto source = m_input_support.explicit_profile_of(req);
        if (source.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Missing required param: profile_id"
            );
            error.send(res, send);
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
            error.send(res, send);
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
            error.send(res, send);
            return;
        }
        // clone.rs:26-63 resolve_clone_targets: any foreign profile fails the whole request.
        if (!m_input_support.profile_is_owned(source, merchant)) {
            const auto error = m_input_support.profile_not_found(source);
            error.send(res, send);
            return;
        }
        for (const auto& target: targets) {
            if (!m_input_support.profile_is_owned(target.get_value(), merchant)) {
                const auto error = m_input_support.profile_not_found(target.get_value());
                error.send(res, send);
                return;
            }
        }

        std::string transaction_type;
        if (typed->get_transaction_type()) {
            transaction_type = typed->get_transaction_type()->get_value();
        }
        CloneBlocklistEntriesInput input{merchant, source, targets, transaction_type};
        auto result = m_runtime.clone_blocklist_entries(input);
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
