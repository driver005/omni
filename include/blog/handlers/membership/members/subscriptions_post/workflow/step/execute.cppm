export module blog_handlers:members_subscriptions_post_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import core_serde;
import :members_operations;

// Strict-parity POST /members/:id/subscriptions/ (createSubscription). Ghost
// requires id + stripe_price_id and calls Stripe createSubscription then
// re-reads the member with metafieldsFor:ADMIN (M:252-288, MR:2734-2790).
export namespace blog::handlers::members::subscriptions_post::workflow {
namespace support = blog::api::members::members_support;

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"members_subscriptions_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const std::string raw{in.get_path()};
        const auto marker = raw.find("/subscriptions");
        const auto cut = marker == std::string::npos ? raw.find('?') : marker;
        const auto clean = raw.substr(0, cut == std::string::npos ? raw.size() : cut);
        const auto slash = clean.find_last_of('/');
        const std::string id{slash == std::string::npos ? std::string{} : clean.substr(slash + 1)};
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body", support::invalid("Validation error, cannot save member.", "", "id")}}
            };
        }
        const auto body = std::string{in.body_as_string()};
        const auto parsed = support::decode(body);
        const auto price = parsed ? nested_field(*parsed, "stripe_price_id") : std::string{};
        if (price.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  support::invalid("Validation error, cannot save member.", "", "stripe_price_id")}}
            };
        }
        const auto member = ctx.store().find_by_resource("members", id);
        if (!member) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"}, {"body", support::not_found("Member not found.")}}
            };
        }
        const auto r = s_ops->member_subscription_create(id, body);
        // Stripe has no allowed primitive: record the outbound call in the outbox.
        support::emit(
            "stripe.create_subscription",
            std::format(R"({{"id":"{}","stripe_price_id":"{}"}})", id, price),
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

private:
    static std::string nested_field(const support::Value& value, std::string_view key)
    {
        const auto* object = std::get_if<support::Object>(&value.get());
        if (object == nullptr) {
            return {};
        }
        auto found = support::scalar(object, key);
        if (!found.empty()) {
            return found;
        }
        for (const auto& [name, nested]: *object) {
            if (const auto* inner = std::get_if<support::Object>(&nested.get())) {
                found = support::scalar(inner, key);
                if (!found.empty()) {
                    return found;
                }
            }
        }
        return {};
    }
};

blog::api::members::MembersOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::members::subscriptions_post::workflow
