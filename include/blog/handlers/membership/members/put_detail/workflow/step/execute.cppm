export module blog_handlers:members_put_detail_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import core_serde;
import :members_operations;

// Strict-parity PUT /members/:id/subscriptions/:subscription_id (editSubscription).
// Ghost requires both ids, cancel_at_period_end and status∈{canceled}; Stripe
// cancel/update + _MemberCancelEvent then re-read or 404 (M:188-245).
export namespace blog::handlers::members::put_detail::workflow {
namespace support = blog::api::members::members_support;

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"members_put_detail_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const auto parts = support::segments(in.get_path());
        if (parts.size() < 2) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body", support::invalid("Validation error, cannot save member.", "", "id")}}
            };
        }
        const std::string id = parts[1], subscriptionId = parts.back();
        const auto body = std::string{in.body_as_string()};
        const auto parsed = support::decode(body);
        const auto* object = parsed ? std::get_if<support::Object>(&parsed->get()) : nullptr;
        if (object == nullptr) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  support::invalid(
                      "Validation error, cannot save member.",
                      "",
                      "cancel_at_period_end"
                  )}}
            };
        }
        const auto status = support::scalar(object, "status");
        if (!status.empty() && status != "canceled") {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  support::invalid("Validation error, cannot save member.", status, "status")}}
            };
        }
        const auto cancel = support::value_for(object, "cancel_at_period_end");
        if (cancel == nullptr) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  support::invalid(
                      "Validation error, cannot save member.",
                      "",
                      "cancel_at_period_end"
                  )}}
            };
        }
        const auto member = ctx.store().find_by_resource("members", id);
        if (!member) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"}, {"body", support::not_found("Member not found.")}}
            };
        }
        const auto r = s_ops->member_subscription_edit(id, subscriptionId, body);
        support::emit(
            status == "canceled" ? std::string{"stripe.cancel_subscription"}
                                 : std::string{"stripe.update_subscription"},
            std::format(
                R"({{"id":"{}","subscription_id":"{}","status":"{}"}})",
                id,
                subscriptionId,
                status
            ),
            id
        );
        return core::workflow::StepResponse<Output>{
            {{"status", std::to_string(r.first)},
             {"body", support::member_with_subscriptions(*member, r.second)}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
    static blog::api::members::MembersOperations* s_ops;
};

blog::api::members::MembersOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::members::put_detail::workflow
