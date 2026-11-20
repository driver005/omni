export module blog_handlers:members_members_metafields_values_step;
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
export namespace blog::handlers::members::members_metafields_values::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string, std::string>;
 static constexpr std::string_view name{"members_members_metafields_values_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  namespace support=blog::api::members::metafields_support;
  std::string path=std::string{in.get_path()};
  const auto q=path.find('?');
  const auto at=path.find("/metafields/");
  const auto key_at=at==std::string::npos?std::string::npos:path.find('/', at+12);
  const std::string ns=at==std::string::npos?std::string{}
    :path.substr(at+12,(key_at==std::string::npos?(q==std::string::npos?path.size():q):key_at)-at-12);
  std::string key;
  if (key_at!=std::string::npos) {
    const auto stop=path.find('/', key_at+1);
    const auto end=(stop==std::string::npos?(q==std::string::npos?path.size():q):stop);
    key=path.substr(key_at+1,end-key_at-1);
  }
  if (!support::valid_segment(ns))
   return core::workflow::StepResponse<Output>{{{"status","422"},{"body",support::invalid(support::kNamespaceMessage,"namespace")}}};
  if (!support::valid_segment(key))
   return core::workflow::StepResponse<Output>{{{"status","422"},{"body",support::invalid(support::kKeyMessage,"key")}}};
  const auto r=s_ops->metafields_held_values(*s_ctx, ns, key);
  return core::workflow::StepResponse<Output>{{{"status",std::to_string(r.first)},{"body",r.second}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx; static blog::api::members::MembersOperations* s_ops;
};
blog::api::members::MembersOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
