module;
#include <ctime>
export module blog_handlers:db_bulk_destroy_step;
import :domain_all;
import std;
import core_serde;
import core_workflow;
import interfaces;

export namespace blog::handlers::db::bulk_destroy::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"db_bulk_destroy_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  const std::string posts=ctx.store().json_for("posts", {});
  const std::string tags=ctx.store().json_for("tags", {});

  // dbBackup.backup() runs before the delete transaction (db.js:143, backup.js:51-70).
  const auto ms=std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
  const std::string data="{\"meta\":{\"exported_on\":"+std::to_string(ms)
    +",\"version\":\"5.0.0\"},\"data\":{\"posts\":"+posts+",\"tags\":"+tags+"}}";
  const std::string filename=site_title(ctx)+".ghost."+stamp("%Y-%m-%d-%H-%M-%S")+".json";
  (void)ctx.store().create("db", "{\"filename\":"+quote(filename)+",\"data\":"+data+"}");

  // Ghost destroyAll removes every Post and Tag in a transaction with
  // context.internal + promisePool(...,100) (db.js:153-184). The shared
  // ResourceStore deletes by id, so enumerate all in-memory rows explicitly.
  (void)ctx.store().resources().bulk_destroy("posts", ids_body(posts));
  (void)ctx.store().resources().bulk_destroy("tags", ids_body(tags));

  // 204 with no body (db.js:140-142); X-Cache-Invalidate: /* is set by the handler.
  return core::workflow::StepResponse<Output>{{{"status","204"},{"body",""}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 [[nodiscard]] static std::string quote(std::string_view value) {
  return core_serde::JsonCodec{}.encode(std::string{value});
 }
 [[nodiscard]] static std::string unquote(std::string_view value) {
  const auto decoded=core_serde::JsonCodec{}.decode<std::string>(value);
  return decoded?*decoded:std::string{value};
 }
 [[nodiscard]] static std::string stamp(std::string_view format) {
  const auto now=std::chrono::system_clock::now();
  const auto seconds=std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
  std::time_t raw{static_cast<std::time_t>(seconds)};
  std::tm utc{};
#if defined(_WIN32)
  ::gmtime_s(&utc,&raw);
#else
  ::gmtime_r(&raw,&utc);
#endif
  char buffer[64]{};
  std::strftime(buffer,sizeof(buffer),std::string{format}.c_str(),&utc);
  return std::string{buffer};
 }
 [[nodiscard]] static std::string site_title(blog::BlogContext& ctx) {
  for (const auto& row : blog::api::parse_array(ctx.store().json_for("settings", {}))) {
   const auto key=row.find("key"); const auto value=row.find("value");
   if (key==row.end()||value==row.end()||key->second!=quote("title")) continue;
   std::string title=unquote(value->second);
   for (char& c : title) if (!std::isalnum(static_cast<unsigned char>(c))&&c!='.'&&c!='-'&&c!='_') c='-';
   return title;
  }
  return "ghost";
 }
 [[nodiscard]] static std::string ids_body(std::string_view rows) {
  std::string joined;
  for (const auto& row : blog::api::parse_array(rows)) {
   const auto id=row.find("id");
   if (id==row.end()) continue;
   if (!joined.empty()) joined+=",";
   joined+=unquote(id->second);
  }
  return "{\"ids\":"+quote(joined)+"}";
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
