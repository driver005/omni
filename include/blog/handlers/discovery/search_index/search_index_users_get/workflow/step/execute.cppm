export module blog_handlers:search_index_search_index_users_get_step;
import :domain_all;
import :search_index_dto_more_resources;
import std;
import core_workflow;
import interfaces;
import blog_handlers_middleware;

export namespace blog::handlers::search_index::search_index_users_get::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"search_index_search_index_users_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; auto& mutations = ctx.store();
    const auto r=[&](std::string_view kind)->std::pair<int,std::string>{
        if (kind == "pages") {
            blog::dto::search_index::SearchPagesResponse r; r.pages.push_back(blog::dto::search_index::RecordDto{}); return {200, blog::handlers::middleware::encode(r)};
        }
        if (kind == "posts") {
            blog::dto::search_index::SearchPostsResponse r; r.posts.push_back(blog::dto::search_index::RecordDto{}); return {200, blog::handlers::middleware::encode(r)};
        }
        if (kind == "tags") {
            blog::dto::search_index::SearchTagsResponse r; r.tags.push_back(blog::dto::search_index::RecordDto{}); return {200, blog::handlers::middleware::encode(r)};
        }
        if (kind == "authors") {
            blog::dto::search_index::SearchAuthorsResponse r;
            r.authors.push_back(blog::dto::search_index::RecordDto{});
            return {200, blog::handlers::middleware::encode(r)};
        }
        blog::dto::search_index::SearchUsersResponse r; r.users.push_back(blog::dto::search_index::RecordDto{}); return {200, blog::handlers::middleware::encode(r)};
    }("users");
  if(r.first!=0) return core::workflow::StepResponse<Output>{r.second};
  const auto rows=ctx.store().json_for("search-index", {});
  return core::workflow::StepResponse<Output>{"{\"search-index\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}}"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
