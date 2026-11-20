export module blog_handlers:integrations_create_step;
import :domain_all;
import :integrations_dto_more_resources;
import :outbox_shim;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::integrations::create::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"integrations_create_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  namespace support = blog::handlers::integrations::support;
  auto& ctx = *s_ctx;
  const std::string_view path{in.get_path()};

  const auto options = support::query_options(path, {"include"});
  const std::string include = options.contains("include") ? options.at("include") : std::string{};
  if (!support::valid_include(include)) {
   return core::workflow::StepResponse<Output>{{{"status", "422"},
    {"body", blog::api::ApiError{}.validation("Validation error", "Invalid include value", "include")}}};
  }

  // data:['name','icon_image','description','webhooks'] (integrations.js:88).
  const std::string body = in.body_as_string().empty() ? std::string{"{}"}
                                                       : std::string{in.body_as_string()};
  const auto parsed = support::object(body);
  auto name_it = parsed.find("name");
  if (name_it == parsed.end() || support::unquote(name_it->second).empty()) {
   return core::workflow::StepResponse<Output>{{{"status", "422"},
    {"body", blog::api::ApiError{}.validation("Validation error, cannot save integration.",
                                             "name is required", "name")}}};
  }
  const std::string name = support::unquote(name_it->second);

  std::map<std::string, std::string> fields{
      {"name", support::quote(name)},
      {"slug", support::quote(support::slugify(name))},
      {"type", support::quote("custom")},
      {"created_at", support::quote(support::now_iso())},
      {"updated_at", support::quote(support::now_iso())}};
  for (auto key : {"icon_image", "description"}) {
   if (const auto it = parsed.find(key); it != parsed.end()) fields[key] = it->second;
  }
  const std::string webhooks = parsed.contains("webhooks") ? parsed.at("webhooks")
                                                           : std::string{"[]"};

  auto& outbox = blog::handlers::outbox::global_outbox();
  outbox.begin_transaction();

  // Integration.add + defaultRelations(api_keys) + injected api keys
  // (integrations.js:103-109; models/integration.js:29-33,40-61).
  const std::string id = ctx.store().resources().create("integrations", support::make_object(fields));
  const std::string api_keys = support::create_api_keys(ctx.store(), id);
  const std::string hook_rows = support::create_webhooks(ctx.store(), id, webhooks);

  outbox.emit(blog::handlers::outbox::Event{
      .type = "integration.added",
      .payload = std::format(R"({{"id":{},"name":{},"api_keys":{},"webhooks":{}}})",
                             support::quote(id), support::quote(name), api_keys, hook_rows),
      .aggregate_id = id});
  outbox.commit();

  std::string record = ctx.store().resources().get("integrations", id);
  record = support::embed_relations(ctx.store(), record, include);
  record = support::prefix_admin_secrets(record);
  return core::workflow::StepResponse<Output>{{{"status", "201"},
   {"body", support::wrap("integrations", record)}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
