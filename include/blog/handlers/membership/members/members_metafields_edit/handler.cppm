export module blog_handlers:members_members_metafields_edit_handler;
import :domain_all;
import :members_dto_member_subscription;
import :members_dto_member_subscription_input;
import :members_dto_member_subscription_response;
import :members_dto_signin_url;
import :members_dto_signin_urls_response;
import :members_dto_stats_member_entry;
import :members_dto_stats_member_meta;
import :members_dto_stats_member_response;
import :members_dto_stats_member_totals;
import :members_dto_stats_mrr_entry;
import :members_dto_stats_mrr_meta;
import :members_dto_stats_mrr_response;
import :members_dto_stats_mrr_total;
import :members_dto_subscription_member;
import :members_dto_subscription_tier;
import :members_model_member;
import :members_model_wire_fixtures;
import std;
import interfaces;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :members_operations;
import :members_members_metafields_edit_flow;

export namespace blog::handlers::members::members_metafields_edit {
class MembersMetafieldsEditHandler
{
public:
    MembersMetafieldsEditHandler(
        blog::BlogContext& ctx,
        blog::api::members::MembersOperations& ops
    ) noexcept
    {
        workflow::ExecuteStep::s_ctx = &ctx;
        workflow::ExecuteStep::s_ops = &ops;
        m_workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
        m_workflow.register_workflow(workflow::MainFlow{}.build());
    }

    void handle(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        namespace support = blog::api::members::metafields_support;
        if (blog::handlers::middleware::token(req).empty()) {
            const std::string b = support::unauthorized();
            std::vector<std::byte> bytes;
            for (char c: b) {
                bytes.push_back(static_cast<std::byte>(c));
            }
            res.set_status(interfaces::io::types::Status::UNAUTHORIZED);
            res.set_body(std::move(bytes));
            send();
            return;
        }
        // Ghost edit: canManage(frame,'edit',key) (member-metafields.ts:122-124).
        if (const auto denial = support::authorize(req, "member_custom_field", "edit")) {
            const std::string b = denial->second;
            std::vector<std::byte> bytes;
            for (char c: b) {
                bytes.push_back(static_cast<std::byte>(c));
            }
            res.set_status(static_cast<interfaces::io::types::Status>(denial->first));
            res.set_body(std::move(bytes));
            send();
            return;
        }
        const auto ex = m_workflow.run("members_members_metafields_edit", req);
        const auto* r = ex.result_of("members_members_metafields_edit_execute");
        const auto payload =
            r && r->typed_output.has_value()
                ? std::any_cast<std::map<std::string, std::string>>(r->typed_output)
                : std::map<std::string, std::string>{{"status", "200"}, {"body", "{}"}};
        const int status = payload.contains("status") ? std::stoi(payload.at("status")) : 200;
        const auto body = payload.contains("body") ? payload.at("body") : std::string{"{}"};
        // Ghost edit: X-Cache-Invalidate /* only when changedForMembers && isBound
        // (member-metafields.ts:133-137). carried in the step's string map.
        if (payload.contains("bound") && payload.at("bound") == "true" &&
            payload.contains("changed") && payload.at("changed") == "true") {
            res.set_header("X-Cache-Invalidate", "/*");
        }
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
} // namespace blog::handlers::members::members_metafields_edit
