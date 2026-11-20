export module blog_handlers:authors_read_by_slug_step;
import :domain_all;
import :authors_browse_step;
import std;
import core_workflow;
import core_query;
import interfaces;

export namespace blog::handlers::authors::read_by_slug::workflow {

namespace local_mapping {
[[nodiscard]] inline std::string wrap(std::string_view resource, std::string_view rows,
                                      std::string_view meta = {})
{
    return "{\"" + std::string{resource} + "\":" + std::string{rows} +
           (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
}
[[nodiscard]] inline std::string clean(std::string_view json, std::string_view) { return std::string{json}; }
[[nodiscard]] inline std::string pagination(std::size_t total, std::size_t limit, std::size_t page,
                                            std::size_t pages, std::string_view prev,
                                            std::string_view next)
{
    return std::format(R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
                       total, limit, page, pages,
                       prev.empty() ? std::string{"null"} : std::string{prev},
                       next.empty() ? std::string{"null"} : std::string{next});
}
} // namespace local_mapping
class ExecuteStep {
public:
  using Input=interfaces::io::IRequest&;
  using Output=std::map<std::string,std::string>;
  static constexpr std::string_view name{"authors_read_by_slug_execute"};
  [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
   auto& ctx=*s_ctx;
   namespace shared=blog::handlers::authors::browse::workflow::detail;

   const auto full=std::string{in.get_path()};
   const auto qpos=full.find('?');
   auto path=std::string_view{full}.substr(0,qpos);
   while(path.size()>1 && path.back()=='/') path.remove_suffix(1);
   const auto slash=path.find_last_of('/');
   const std::string slug{slash==std::string_view::npos?std::string{path}:std::string{path.substr(slash+1)}};

   const auto q=core::query::parse(full, shared::kAllowedReadOptions);
   for (const auto& include : q.include) {
    if (include!="count.posts") {
     return core::workflow::StepResponse<Output>{
      {{"status","400"},{"body",blog::api::ApiError{}.validation(
         "Invalid include","Bad Request","include","ValidationError")}}};
    }
   }
   if (slug.empty()) {
    return core::workflow::StepResponse<Output>{
     {{"status","400"},{"body",blog::api::ApiError{}.validation(
        "An author id or slug is required.","Bad Request","slug","ValidationError")}}};
   }

   auto find=q;
   shared::sanitize(find);
   // Ghost read: pick(frame.data, ['id','slug']) -> the /slug route supplies only `slug`.
   find.filters.emplace_back("slug", slug);

   const auto raw=ctx.store().json_for("authors");
   const bool with_count=find.include.contains("count.posts");
   const std::size_t post_count=with_count?ctx.store().total("posts"):std::size_t{0};
   const auto wire=shared::author_rows(raw,with_count,post_count,in.get_scheme(),in.get_authority());
   const auto rows=core::query::apply(wire,find);
   const auto items=blog::api::parse_array(rows);
   if (items.empty()) {
    return core::workflow::StepResponse<Output>{
     {{"status","404"},{"body",blog::api::ApiError{}.json(
        "Author not found.","Author not found")}}};
   }
   return core::workflow::StepResponse<Output>{
    {{"status","200"},
     {"body",local_mapping::wrap("authors",blog::api::record_to_json(items.front()))}}};
  }
  void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
  static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
