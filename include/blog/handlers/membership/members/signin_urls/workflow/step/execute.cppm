export module blog_handlers:members_signin_urls_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import :members_operations;

// Strict-parity GET /members/:id/signin_urls. Ghost reads the member (404),
// then getMagicLink(email,'signin') writes a SingleUseToken row and returns a
// signed URL; output {member_signin_urls:[{member_id,url}]}, no cache header.
export namespace blog::handlers::members::signin_urls::workflow {
namespace support = blog::api::members::members_support;

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"members_signin_urls_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const std::string id = support::segment_before(in.get_path(), "/signin_urls");
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body", support::invalid("id is required.", "member_signin_urls", "id")}}
            };
        }
        const auto member = support::member_row(ctx, id);
        if (!member) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"}, {"body", support::not_found("Member not found.")}}
            };
        }
        const auto email = support::scalar(&*member, "email");
        const auto r = s_ops->member_signin_urls(id, support::encode(*member));
        // The token row from Ghost's SingleUseTokenProvider is observable through the
        // outbox; the context-local token marker remains as the durable stand-in.
        support::emit_event(
            "SingleUseTokenCreated",
            std::format(R"({{"member_id":"{}","email":"{}","type":"signin"}})", id, email),
            id
        );
        return core::workflow::StepResponse<Output>{
            {{"status", std::to_string(r.first)}, {"body", r.second}}
        };
    }

    static blog::api::members::MembersOperations* s_ops;

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::api::members::MembersOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::members::signin_urls::workflow
