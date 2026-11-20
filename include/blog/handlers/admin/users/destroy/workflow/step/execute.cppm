export module blog_handlers:users_destroy_step;
import :domain_all;
import :users_dto_subresources;
import std;
import core_workflow;
import interfaces;
import :outbox_shim;

export namespace blog::handlers::users::destroy::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"users_destroy_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  const auto id=last_segment(in.get_path());
  if (id.empty()) {
   return core::workflow::StepResponse<Output>{{{"status","422"},
    {"body",blog::api::ApiError{}.validation("Validation error","An id is required","id")}}};
  }

  // Ghost `usersService.destroyUser` cascade, recorded on the shared outbox in
  // the same order as users-service.ts#destroyUser.
  auto& outbox=blog::handlers::outbox::global_outbox();
  outbox.begin_transaction();
  const auto emit=[&](std::string_view type, std::string_view action) {
   outbox.emit(blog::handlers::outbox::Event{
    .type=std::string{type},
    .payload=std::format(R"({{"id":{},"action":{}}})",
     blog::api::JsonBody{}.quote(id), blog::api::JsonBody{}.quote(action)),
    .aggregate_id=id});
  };
  emit("user.post_authors_locked","lock");
  emit("user.revision_authors_nulled","null");
  emit("user.posts_tagged","tag");
  emit("user.posts_reassigned","reassign");

  // ApiKey rows are deleted in the same transaction (NotFoundError tolerated).
  auto& rs=ctx.store().resources();
  const auto keys=blog::api::parse_array(ctx.store().json_for("api_keys", {{"filter","user_id:"+id}}));
  for (const auto& key : keys) {
   const auto it=key.find("id");
   if (it!=key.end() && !it->second.empty()) rs.destroy("api_keys", it->second);
  }
  emit("user.api_keys_destroyed","destroy");

  const auto mutation=ctx.store().destroy("users", id);
  if (!mutation.ok()) {
   outbox.rollback();
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(mutation.status())},
    {"body",blog::api::ApiError{}.validation(
     "You do not have permission to perform this action","Forbidden","","NoPermissionError")}}};
  }
  emit("user.destroyed","destroy");
  outbox.commit();

  // Ghost answers 204 with an empty body; the native store still answers 200.
  return core::workflow::StepResponse<Output>{{{"status","204"},{"body",""}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 static std::string last_segment(std::string_view path) {
  path=path.substr(0,path.find('?'));
  while (path.size()>1 && path.back()=='/') path.remove_suffix(1);
  const auto slash=path.find_last_of('/');
  return std::string{slash==std::string_view::npos?path:path.substr(slash+1)};
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
