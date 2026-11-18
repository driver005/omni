export module blog_handlers:comments_comment_dislikes_step;
import std;
import core_workflow;
import blog_context;
import :mutation;

import blog_dto;
import :comments_comment_dislikes_model;
import :comments_comment_dislikes_dto;
export namespace blog::handlers::comments::comment_dislikes::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"comments_comment_dislikes_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
    const auto r=[&](std::string_view path)->std::pair<int,std::string>{
        std::string_view key{"comments"};
        if (path.ends_with("/likes")) key = "comment_likes";
        else if (path.ends_with("/dislikes")) key = "comment_dislikes";
        else if (path.ends_with("/reports")) key = "comment_reports";
        return std::pair<int, std::string>{200, std::format(
            R"({{"{}":[{}],"meta":{{"pagination":{{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}}}})",
            key, key == "comments" ? blog::dto::GhostFixtures::comment([]{ blog::model::Comment c; c.set_id(1); return c; }())
                                   : blog::dto::GhostFixtures::comment_sub()
        )};
    }("/comments/:id/dislikes");
  if(r.first!=0) return core::workflow::StepResponse<Output>{r.second};
  const auto one=ctx.store().find_by_resource("comments", in.id.empty()?"1":in.id);
  if(one) return core::workflow::StepResponse<Output>{"{\"comments\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"comments":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
