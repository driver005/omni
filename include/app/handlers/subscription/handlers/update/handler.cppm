export module app_handlers_subscription:handlers_update;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_context;
import core_error;
import app_auth;

import :input;
import :result;
import :runtime;

export namespace app::handlers::subscription {

class UpdateHandlers
{
public:
    UpdateHandlers(app::context::AppContext& ctx, SubscriptionRuntime& runtime) noexcept :
        m_context{ctx},
        m_runtime{runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/subscriptions"}.add_router(
            Router{router, ":subscription_id"}.add_route(
                Route{"update"}
                    .put(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_update_subscription(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch update_subscription")
            )
        );
    }

    // ── PUT /subscriptions/:id/update — update_subscription ─────────────────────
    void handle_update_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto failure = m_input_support.authorize_api_key(req, auth)) {
            const auto& error = *failure;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        auto profile = m_input_support.extract_profile_id(req);
        if (!profile) {
            const auto& error = profile.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto id = m_input_support.subscription_id_from_request(req);
        if (id.empty()) {
            const auto error = m_input_support.generic_not_found("subscription not found for id: ");
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        auto payload = m_input_support.parse_json_body(req);
        if (!payload) {
            const auto& error = payload.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto plan_id = m_input_support.json_string(payload->root, "plan_id");
        const auto item_price_id = m_input_support.json_string(payload->root, "item_price_id");
        if (!plan_id || !item_price_id) {
            const auto error = m_input_support.json_body_error(
                "missing field `plan_id` or `item_price_id` at line 1 column 2"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto merchant = m_input_support.merchant_id(req, auth);
        UpdateSubscriptionInput input{profile.value(), merchant, id, *plan_id, *item_price_id};
        auto result = m_runtime.update_subscription(input);
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

private:
    app::context::AppContext& m_context;
    SubscriptionRuntime& m_runtime;
    SubscriptionInputSupport m_input_support{};
};

} // namespace app::handlers::subscription
