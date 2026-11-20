export module blog_handlers:members_members_metafields_create_step;
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
import core_workflow;
import core_query;
import interfaces;
import :members_operations;
export namespace blog::handlers::members::members_metafields_create::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"members_members_metafields_create_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  namespace support=blog::api::members::metafields_support;
  std::string path=std::string{in.get_path()};
  const auto q=path.find('?');
  const auto at=path.find("/metafields/");
  const std::string ns=at==std::string::npos?std::string{}
    :path.substr(at+12,(q==std::string::npos?path.size():q)-at-12);
  if (!support::valid_segment(ns))
   return core::workflow::StepResponse<Output>{{{"status","422"},{"body",support::invalid(support::kNamespaceMessage,"namespace")}}};
  // The whole data.members_metafields batch is passed through: Ghost validates
  // every item and writes all of them in one request (member-metafields.ts:19-25).
  const auto r=s_ops->metafields_create(*s_ctx, ns, in.body_as_string());
  return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::members::MembersOperations* s_ops;
};
blog::api::members::MembersOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
