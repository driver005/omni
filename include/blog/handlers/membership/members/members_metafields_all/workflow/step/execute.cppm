export module blog_handlers:members_members_metafields_all_step;
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
export namespace blog::handlers::members::members_metafields_all::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"members_members_metafields_all_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  namespace support=blog::api::members::metafields_support;
  // Ghost: options ['namespace','filter'], namespace required and parsed by
  // NamespaceSchema (services/members-metafields/serializers.ts:8-16).
  std::string path=std::string{in.get_path()};
  const auto q=path.find('?');
  const auto at=path.find("/metafields/");
  const std::string ns=at==std::string::npos?std::string{}
    :path.substr(at+12,(q==std::string::npos?path.size():q)-at-12);
  if (!support::valid_segment(ns))
   return core::workflow::StepResponse<Output>{{{"status","422"},{"body",support::invalid(support::kNamespaceMessage,"namespace")}}};
  // NQL filter + publisher order live in core_query (CONTRACTS.md §1); the
  // full path is passed so filter syntax can be validated before parsing.
  const auto r=s_ops->metafields_browse(*s_ctx, ns, in.get_path());
  return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},
    {"body",r.second}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::members::MembersOperations* s_ops;
};
blog::api::members::MembersOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
