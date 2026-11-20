export module blog_handlers:integrations_edit_step;
import :domain_all;
import :integrations_dto_more_resources;
import :outbox_shim;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::integrations::edit::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"integrations_edit_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  namespace support = blog::handlers::integrations::support;
  auto& ctx = *s_ctx;
  const std::string_view path{in.get_path()};

  const auto options = support::query_options(path, {"id", "keyid", "include"});
  const std::string include = options.contains("include") ? options.at("include") : std::string{};
  if (!support::valid_include(include)) {
   return core::workflow::StepResponse<Output>{{{"status", "422"},
    {"body", blog::api::ApiError{}.validation("Validation error", "Invalid include value", "include")}}};
  }

  const std::string id = support::tail_id(path);
  std::string record = ctx.store().resources().get("integrations", id);
  if (id.empty() || record.empty()) return not_found();

  const std::string body = in.body_as_string().empty() ? std::string{"{}"}
                                                       : std::string{in.body_as_string()};
  const auto parsed = support::object(body);
  std::string changed = record;
  bool has_webhooks = false;
  std::string webhooks;
  for (auto key : {"name", "icon_image", "description"}) {
   if (const auto it = parsed.find(key); it != parsed.end()) {
    changed = support::with_field(changed, key, it->second);
   }
  }
  if (const auto it = parsed.find("name"); it != parsed.end()) {
   changed = support::with_field(changed, "slug",
                                 support::quote(support::slugify(support::unquote(it->second))));
  }
  if (const auto it = parsed.find("webhooks"); it != parsed.end()) {
   has_webhooks = true;
   webhooks = it->second;
  }
  changed = support::with_field(changed, "updated_at", support::quote(support::now_iso()));
  ctx.store().resources().edit("integrations", id, changed);

  auto& outbox = blog::handlers::outbox::global_outbox();
  outbox.begin_transaction();
  std::string hook_rows{"[]"};
  if (has_webhooks) hook_rows = support::replace_webhooks(ctx.store(), id, webhooks);
  outbox.emit(blog::handlers::outbox::Event{
      .type = "integration.edited",
      .payload = std::format(R"({{"id":{},"name":{},"webhooks":{}}})",
                             support::quote(id), support::field(changed, "name"), hook_rows),
      .aggregate_id = id});
  outbox.commit();

  record = ctx.store().resources().get("integrations", id);
  record = support::embed_relations(ctx.store(), record, include);
  record = support::prefix_admin_secrets(record);
  return core::workflow::StepResponse<Output>{{{"status", "200"},
   {"body", support::wrap("integrations", record)}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 static core::workflow::StepResponse<Output> not_found() {
  return core::workflow::StepResponse<Output>{{{"status", "404"},
   {"body", blog::handlers::integrations::support::not_found_integration()}}};
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
