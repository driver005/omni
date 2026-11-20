export module blog_handlers:settings_settings_routes_yaml_post_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import :settings_browse_step;

export namespace blog::handlers::settings::settings_routes_yaml_post::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=support::Result;
 static constexpr std::string_view name{"settings_settings_routes_yaml_post_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  using namespace support;
  const auto part=multipart_part(in.body_as_string(),
      in.find_header(interfaces::io::types::Token::CONTENT_TYPE), "routes");

  // Ghost apiMw.upload.validation({type:'routes'}) — missing/invalid file.
  if (part.filename.empty() || part.content.empty())
   return core::workflow::StepResponse<Output>{fail(400, validation_error("Please select a YAML file."))};
  const auto dot=part.filename.rfind('.');
  const auto ext=dot==std::string::npos?std::string{}:part.filename.substr(dot);
  if (!iequal(ext,".yaml") && !iequal(ext,".yml"))
   return core::workflow::StepResponse<Output>{fail(400, ghost_error("Please select a valid YAML file to import.","UnsupportedMediaTypeError"))};

  // Ghost DynamicRoutingService.upload -> parseYaml + parseRouteSettings.
  if (!valid_yaml(part.content))
   return core::workflow::StepResponse<Output>{fail(400, validation_error("Unable to parse routes.yaml", "routes"))};

  // routeSettings store.replace + bridge.reloadFrontend (stand-ins on outbox).
  auto& rs=ctx.store().resources();
  for (const auto& rec : blog::api::parse_array(rs.list("routes_yaml"))) {
   const auto itid=rec.find("id");
   if (itid==rec.end()) continue;
   const auto id=core_serde::JsonCodec{}.decode<std::string>(itid->second).value_or(itid->second);
   rs.destroy("routes_yaml", id);
  }
  rs.create("routes_yaml", R"({"content":)" + core_serde::JsonCodec{}.encode(part.content) + "})");
  emit_transaction({
   {.type="routes.yaml.uploaded", .payload=R"({"filename":)"+quote(part.filename)+"}", .aggregate_id="routes_yaml"},
   {.type="routes.reloaded", .payload="{}", .aggregate_id="routes_yaml"},
   {.type="cache.invalidate", .payload=R"({"url":"/*"})", .aggregate_id="routes_yaml"}});

  Result result{{"status","200"},{"body","{}"},{"cache_invalidate","/*"}};
  return core::workflow::StepResponse<Output>{result};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
