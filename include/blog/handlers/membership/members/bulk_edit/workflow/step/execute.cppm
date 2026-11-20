export module blog_handlers:members_bulk_edit_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_serde;
import interfaces;
import :members_operations;

// Strict-parity PUT /members/bulk (bulkEdit). Ghost requires action in
// {unsubscribe,addLabel,removeLabel} plus all/filter/search (M:349-361);
// serializer bulkAction overrides the query shape with
// {bulk:{action,meta:{stats:{successful,unsuccessful},errors,unsuccessfulData}}}
// (OS:18,63-77).
export namespace blog::handlers::members::bulk_edit::workflow {
namespace support=blog::api::members::members_support;
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"members_bulk_edit_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  static constexpr std::string_view kOptions[]{"all","filter","search"};
  auto q=core::query::parse(in.get_path(), kOptions);
  if (core::query::has_restricted_field(q.raw_filter))
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",support::bad_request("Restricted fields cannot be used in a filter.")}}};

  std::string action, label;
  const auto parsed=support::decode(in.body_as_string());
  const auto* root=parsed ? std::get_if<support::Object>(&parsed->get()) : nullptr;
  if (root != nullptr) {
   action=support::scalar(root, "action");
   const auto* meta=support::value_for(root, "meta");
   if (meta != nullptr) {
    if (const auto* meta_object=std::get_if<support::Object>(&meta->get()))
     label=support::scalar(meta_object, "name");
   }
  }
  if (action!="unsubscribe" && action!="addLabel" && action!="removeLabel")
   return core::workflow::StepResponse<Output>{{{"status","422"},
     {"body",support::invalid("Validation error, cannot bulk edit members.", action, "action")}}};
  const bool guarded=q.options.contains("all") || !q.raw_filter.empty() ||
                     q.options.contains("search");
  if (!guarded)
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",support::incorrect_usage("Bulk edit requires all, filter or search.")}}};

  std::string available=ctx.store().json_for("members", {});
  try { available=core::query::apply(available, q); }
  catch (...) {
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",support::bad_request("The NQL filter you passed was invalid.")}}};
  }
  auto ids=extract_ids(available);
  std::string id_list;
  for (const auto& id : ids) { if (!id_list.empty()) id_list += ','; id_list += id; }
  const auto result=ctx.store().bulk_edit("members",
    std::format(R"({{"ids":"{}","action":"{}","label":"{}"}})", id_list, action, label));
  const int successful=result.status()==200?static_cast<int>(ids.size()):0;
  const auto body=std::format(
    R"({{"bulk":{{"action":"{}","meta":{{"stats":{{"successful":{},"unsuccessful":0}},)"
    R"("errors":[],"unsuccessfulData":[]}}}}}}}})", action, successful);
  return core::workflow::StepResponse<Output>{{{"status",std::to_string(result.status())},{"body",body}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 static std::vector<std::string> extract_ids(std::string_view rows) {
  std::vector<std::string> out;
  for (const auto& item : support::rows_of(rows)) {
   const auto* object=std::get_if<support::Object>(&item.get());
   if (object==nullptr) continue;
   const auto id=support::scalar(object, "id");
   if (!id.empty()) out.push_back(id);
  }
  return out;
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
