export module blog_handlers:stripe_stripe_checkout_config_put_handler;
import :domain_all;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :stripe_stripe_checkout_config_put_flow;

export namespace blog::handlers::stripe::stripe_checkout_config_put {
class StripeCheckoutConfigPutHandler
{
public:
    StripeCheckoutConfigPutHandler(blog::BlogContext& ctx, blog::api::StripeHandler& sp) noexcept
    {
        workflow::ExecuteStep::s_ctx = &ctx;
        workflow::ExecuteStep::s_stripe = &sp;
        m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
        m_workflow.register_workflow(workflow::MainFlow{}.build());
    }

    void handle(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!req.find_header(blog::handlers::middleware::kGhostContextHeader).empty()) {
            bool denied = false;
            blog::handlers::middleware::GhostPermissionRegistry::set_active(
                &req,
                &res,
                [&]
                {
                    denied = true;
                    send();
                },
                nullptr
            );
            blog::handlers::middleware::require_ghost_permission("products", "edit");
            blog::handlers::middleware::GhostPermissionRegistry::clear_active();
            if (denied) {
                return;
            }
        }
        const auto ex = m_workflow.run("stripe_stripe_checkout_config_put", req);
        const auto* r = ex.result_of("stripe_stripe_checkout_config_put_execute");
        const auto payload =
            r && r->typed_output.has_value()
                ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                : std::map<std::string, std::string>{{"status", "200"}, {"body", "{}"}};
        const int status = payload.contains("status") ? std::stoi(payload.at("status")) : 200;
        const auto body = payload.contains("body") ? payload.at("body") : std::string{"{}"};
        if (payload.contains("changed") && payload.at("changed") == "1") {
            res.set_header("X-Cache-Invalidate", "/*");
        }
        res.set_header("Cache-Control", "private");
        std::vector<std::byte> bytes;
        for (char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        res.set_status(static_cast<interfaces::io::types::Status>(status));
        res.set_body(std::move(bytes));
        send();
    }

private:
    core::workflow::Workflow m_workflow{};
};
} // namespace blog::handlers::stripe::stripe_checkout_config_put
