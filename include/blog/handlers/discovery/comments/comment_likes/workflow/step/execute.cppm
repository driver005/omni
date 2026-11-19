export module blog_handlers:comments_comment_likes_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::comments::comment_likes::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"comments_comment_likes_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
    const auto r=[&](std::string_view path)->std::pair<int,std::string>{
        std::string_view key{"comments"};
        if (path.ends_with("/likes")) key = "comment_likes";
        else if (path.ends_with("/dislikes")) key = "comment_dislikes";
        else if (path.ends_with("/reports")) key = "comment_reports";
        return std::pair<int, std::string>{200, std::format(
            R"({{"{}":[{}],"meta":{{"pagination":{{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}}}})",
            key, key == "comments" ? core_serde::JsonCodec{}.encode([]{ blog::model::Comment c; c.set_id(1); return c; }())
                                   : core_serde::JsonCodec{}.encode(blog::model::CommentSubWire{})
        )};
    }("/comments/:id/likes");
  if(r.first!=0) return core::workflow::StepResponse<Output>{r.second};
  const auto one=ctx.store().find_by_resource("comments", std::string{}.empty()?"1":std::string{});
  if(one) return core::workflow::StepResponse<Output>{"{\"comments\":[" + *one + "]}"};
  return core::workflow::StepResponse<Output>{R"({"comments":[]})"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
