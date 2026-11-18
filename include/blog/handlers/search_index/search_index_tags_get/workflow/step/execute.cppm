export module blog_handlers:search_index_search_index_tags_get_step;
import std;
import core_workflow;
import blog_context;
import :mutation;
import blog_handlers_middleware;

import blog_dto;
import :search_index_search_index_tags_get_model;
import :search_index_search_index_tags_get_dto;
export namespace blog::handlers::search_index::search_index_tags_get::workflow {
class ExecuteStep {
public:
 using Input=model::Request; using Output=std::string;
 static constexpr std::string_view name{"search_index_search_index_tags_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx; blog::api::MutationStore mutations{ctx.store()};
    const auto r=[&](std::string_view kind)->std::pair<int,std::string>{
        if (kind == "pages") {
            blog::dto::SearchPagesResponse r; r.pages.push_back(blog::dto::RecordDto{}); return {200, blog::handlers::middleware::encode(r)};
        }
        if (kind == "posts") {
            blog::dto::SearchPostsResponse r; r.posts.push_back(blog::dto::RecordDto{}); return {200, blog::handlers::middleware::encode(r)};
        }
        if (kind == "tags") {
            blog::dto::SearchTagsResponse r; r.tags.push_back(blog::dto::RecordDto{}); return {200, blog::handlers::middleware::encode(r)};
        }
        if (kind == "authors") {
            blog::dto::SearchAuthorsResponse r;
            r.authors.push_back(blog::dto::RecordDto{});
            return {200, blog::handlers::middleware::encode(r)};
        }
        blog::dto::SearchUsersResponse r; r.users.push_back(blog::dto::RecordDto{}); return {200, blog::handlers::middleware::encode(r)};
    }("tags");
  if(r.first!=0) return core::workflow::StepResponse<Output>{r.second};
  const auto rows=ctx.store().json_for("search-index", {});
  return core::workflow::StepResponse<Output>{"{\"search-index\":" + rows + ",\"meta\":{\"pagination\":{\"limit\":15,\"page\":1,\"pages\":1,\"prev\":null,\"next\":null,\"total\":0}}}}"};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
