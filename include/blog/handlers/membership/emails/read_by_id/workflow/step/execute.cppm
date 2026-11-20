export module blog_handlers:emails_read_by_id_step;
import :domain_all;
import :emails_dto_analytics_job;
import :emails_dto_analytics_status;
import :emails_dto_analytics_status_input;
import :emails_dto_scheduled_analytics_job;
import :emails_model_member;
import :emails_model_wire_fixtures;
import :emails_operations;
import std;
import core_query;
import core_serde;
import core_workflow;
import interfaces;

export namespace blog::handlers::emails::read_by_id::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"emails_read_by_id_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  const std::string path{in.get_path()};
  const std::string id=blog::api::emails::id_from_path(path);

  // Ghost options:['fields'] with validation fields ∈ {html,plaintext,subject}.
  static constexpr std::string_view kOptions[]{"fields"};
  std::string error;
  const auto q=blog::api::emails::parse_query(path, kOptions, error);
  if(!q) return core::workflow::StepResponse<Output>{{{"status","400"},{"body",error}}};

  static constexpr std::string_view kFields[]{"html","plaintext","subject"};
  for(const auto& name : q->fields) {
   if(std::ranges::find(kFields, name)==std::ranges::end(kFields)) {
    return core::workflow::StepResponse<Output>{{{"status","422"},
     {"body",blog::api::emails::validation_error("fields","Invalid fields value")}}};
   }
  }

  // Ghost Email.findOne(data, options) -> NotFoundError when absent.
  const auto found=blog::api::emails::find_email(ctx.store(), id);
  if(found.empty()) return core::workflow::StepResponse<Output>{{{"status","404"},
   {"body",blog::api::emails::not_found("Email not found.")}}};

  const auto objects=blog::api::emails::decode_objects("["+found+"]");
  if(objects.empty()) return core::workflow::StepResponse<Output>{{{"status","404"},
   {"body",blog::api::emails::not_found("Email not found.")}}};

  std::string row=blog::api::emails::encode_object(blog::api::emails::map_email(objects.front()));
  if(!q->fields.empty()) {
   core::query::ParsedQuery projection;
   projection.fields=q->fields;
   const auto projected=blog::api::emails::decode_objects(
    core::query::apply("["+row+"]", projection, 15));
   if(!projected.empty()) row=blog::api::emails::encode_object(projected.front());
  }

  // Ghost serializes a single model under the resource key only (no top-level
  // `data`, no pagination meta) — output/emails.js has no browse/read override
  // so the default api-framework serializer applies.
  const std::string body=blog::api::emails::envelope("emails", "["+row+"]");
  return core::workflow::StepResponse<Output>{{{"status","200"},{"body",body}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
