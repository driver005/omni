module;
#include <ctime>
export module blog_handlers:db_create_step;
import :domain_all;
import std;
import core_serde;
import core_workflow;
import interfaces;

export namespace blog::handlers::db::create::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"db_create_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  const std::string raw=in.body_as_string().empty()?std::string{"{}"}:in.body_as_string();
  const auto decoded=decode_import(raw);
  if (!decoded)
   return core::workflow::StepResponse<Output>{{{"status","422"},
    {"body",blog::api::ApiError{}.validation("Please select a database file to import.",
                                             "Please select a database file to import.","importfile")}}};

  // Ghost resolves the site timezone and owner email for the queued import job
  // (db.js:94-109). The in-memory job record stands in for ImportManager.
  std::string timezone=decoded->timezone;
  if (timezone.empty()) timezone=setting(ctx,"timezone","Etc/UTC");
  std::string owner=decoded->owner;
  if (owner.empty()) owner=owner_email(ctx);
  const std::string import_tag="#Import "+utc_stamp("%Y-%m-%d %H:%M");
  const std::string job="{"
    +quote("kind")+":"+quote("importContent")+","
    +quote("importfile")+":"+quote(decoded->file)+","
    +quote("timezone")+":"+quote(timezone)+","
    +quote("ownerEmail")+":"+quote(owner)+","
    +quote("importTag")+":"+quote(import_tag)+"}";
  (void)ctx.store().create("db", job);

  // output/db.js.importContent always sets db:[] (truthy), so statusCode() is 200
  // on every successful import (db.js:82-88, output/db.js:20-28).
  return core::workflow::StepResponse<Output>{{{"status","200"},
    {"body",R"({"db":[],"problems":[]})"}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 struct Import { std::string file; std::string timezone; std::string owner; };
 [[nodiscard]] static std::string quote(std::string_view value) {
  return core_serde::JsonCodec{}.encode(std::string{value});
 }
 [[nodiscard]] static std::string unquote(std::string_view value) {
  const auto decoded=core_serde::JsonCodec{}.decode<std::string>(value);
  return decoded?*decoded:std::string{value};
 }
 [[nodiscard]] static std::string utc_stamp(std::string_view format) {
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
 [[nodiscard]] static std::string setting(blog::BlogContext& ctx, std::string_view key,
                                          std::string_view fallback) {
  const auto needle=quote(key);
  for (const auto& row : blog::api::parse_array(ctx.store().json_for("settings", {}))) {
   const auto k=row.find("key"); const auto v=row.find("value");
   if (k!=row.end()&&v!=row.end()&&k->second==needle) return unquote(v->second);
  }
  return std::string{fallback};
 }
 [[nodiscard]] static std::string owner_email(blog::BlogContext& ctx) {
  const auto users=blog::api::parse_array(ctx.store().json_for("users", {}));
  if (!users.empty()) {
   const auto email=users.front().find("email");
   if (email!=users.front().end()) return unquote(email->second);
  }
  return {};
 }
 [[nodiscard]] static std::optional<Import> decode_import(std::string_view raw) {
  const auto parsed=core_serde::JsonCodec{}.decode<serde::Value>(raw);
  if (!parsed) return std::nullopt;
  const auto* object=std::get_if<serde::Value::Object>(&parsed->get());
  if (object==nullptr) return std::nullopt;
  Import result;
  for (const auto& [key,value] : *object) {
   const auto* text=std::get_if<std::string>(&value.get());
   if (text==nullptr) continue;
   if (key=="importfile"||key=="file") result.file=*text;
   else if (key=="timezone") result.timezone=*text;
   else if (key=="ownerEmail"||key=="email") result.owner=*text;
  }
  if (result.file.empty()) return std::nullopt;
  return result;
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
