export module blog_handlers:invites_read_by_id_step;
import :domain_all;
import :invites_dto_admin_resources;
import std;
import core_workflow;
import core_query;
import interfaces;

export namespace blog::handlers::invites::read_by_id::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"invites_read_by_id_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;

  // Ghost data:['id','email'], options:['include'] (invites.js:38-44).
  static constexpr std::string_view kOptions[]{"include","email"};
  const auto q=core::query::parse(in.get_path(), kOptions);
  if (!q.include.empty()) return invalid("include","include is not supported");

  const std::string id=last_segment(in.get_path());
  const auto email_it=q.options.find("email");
  const std::string email=email_it==q.options.end()?std::string{}:email_it->second;
  if (id.empty() && email.empty()) return invalid("id","An id or email is required");

  // findOne(frame.data, frame.options) - search by id, else by email (invites.js:47).
  core::query::ParsedQuery filter;
  if (!id.empty()) filter.filters.emplace_back("id", id);
  else filter.filters.emplace_back("email", email);
  const auto rows=ctx.store().json_for("invites", {});
  const auto matched=core::query::apply(support::clean(rows), filter, 15);
  const auto items=blog::api::parse_array(matched);
  if (!items.empty()) {
   // default output serializer: {"invites":[<one model>]}, token stripped by clean().
   return core::workflow::StepResponse<Output>{{{"status","200"},
    {"body",support::wrap(blog::api::record_to_json(items.front()))}}};
  }
  return core::workflow::StepResponse<Output>{{{"status","404"},
   {"body",blog::api::ApiError{}.json("Invite not found.","Not Found")}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 static std::string last_segment(std::string_view path) {
  auto clean_path=path.substr(0,path.find('?'));
  while (clean_path.size()>1 && clean_path.back()=='/') clean_path.remove_suffix(1);
  const auto slash=clean_path.find_last_of('/');
  return std::string{slash==std::string_view::npos?clean_path:clean_path.substr(slash+1)};
 }
 static core::workflow::StepResponse<Output> invalid(std::string_view property, std::string_view context) {
  return core::workflow::StepResponse<Output>{{{"status","422"},
   {"body",blog::api::ApiError{}.validation("Validation error",context,property)}}};
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
