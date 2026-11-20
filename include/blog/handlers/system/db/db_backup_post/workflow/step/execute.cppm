module;
#include <ctime>
export module blog_handlers:db_db_backup_post_step;
import :domain_all;
import std;
import core_query;
import core_serde;
import core_workflow;
import interfaces;

export namespace blog::handlers::db::db_backup_post::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"db_db_backup_post_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  constexpr std::array<std::string_view,2> kOptions{"include","filename"};
  const auto q=core::query::parse(in.get_path(), std::span<const std::string_view>{kOptions});

  // Unknown include values are silently dropped by Ghost's shared validator
  // (validators/input/all.ts:89-94); only the filename basename guard is a 422.
  std::vector<std::string_view> includes;
  for (const auto& name : q.include) if (backup_table(name)) includes.push_back(name);

  const auto fit=q.options.find("filename");
  std::string requested;
  if (fit!=q.options.end()&&!fit->second.empty()) {
   if (fit->second.find('/')!=std::string::npos||fit->second.find('\\')!=std::string::npos)
    return core::workflow::StepResponse<Output>{{{"status","422"},
     {"body",blog::api::ApiError{}.validation("Export filename must not contain path separators",
                                              "Export filename must not contain path separators","filename")}}};
   requested=fit->second;
  }
  const auto filename=disposition_filename(ctx, requested);

  const std::vector<std::string_view> tables=includes.empty()
    ?std::vector<std::string_view>{kDefaultTables.begin(),kDefaultTables.end()}:includes;
  std::string data="{";
  bool first=true;
  for (const auto table : tables) {
   if (!first) data+=",";
   first=false;
   data+=quote(table)+":"+ctx.store().json_for(table, {});
  }
  data+="}";
  const auto ms=std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
  const std::string snapshot="{\"meta\":{\"exported_on\":"+std::to_string(ms)
    +",\"version\":\"5.0.0\"},\"data\":"+data+"}";
  (void)ctx.store().create("db", "{\"filename\":"+quote(filename)+",\"data\":"+snapshot+"}");

  // output/db.js.backupContent -> {db:[{filename}]}; cacheInvalidate:false (no header).
  return core::workflow::StepResponse<Output>{{{"status","200"},
    {"body","{\"db\":[{\"filename\":"+quote(filename)+"}]}"}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 static constexpr std::array<std::string_view,88> kBackupTables{
  "actions","api_keys","automated_email_recipients","brute","donation_payment_events",
  "email_design_settings","emails","integrations","invites","labels","machine_payment_events",
  "members","members_labels","members_products","members_stripe_customers",
  "members_stripe_customers_subscriptions","migrations","migrations_lock","permissions",
  "permissions_roles","permissions_users","rate_limits","webhooks","tokens","sessions",
  "user_accounts","user_sessions","verifications","user_two_factors","subscriptions",
  "mobiledoc_revisions","post_revisions","email_batches","email_recipients",
  "email_recipient_failures","members_cancel_events","members_payment_events",
  "members_login_events","members_email_change_events","members_metafield_change_events",
  "members_status_events","members_paid_subscription_events","members_subscribe_events",
  "members_product_events","members_created_events","members_current_subscription",
  "members_subscription_created_events","members_newsletters","members_metafields",
  "members_metafield_values","members_metafield_bindings","mentions","comments",
  "comment_likes","comment_reports","jobs","redirects","members_click_events",
  "members_feedback","suppressions","email_spam_complaint_events","milestones","collections",
  "collections_posts","recommendations","recommendation_click_events",
  "recommendation_subscribe_events","outbox","jobs_outbox","gift_deliveries","gifts",
  "gift_links","app_installations","app_installation_manifests","post_gift_links",
  "automations","automation_actions","automation_action_edges","automation_action_revisions",
  "automation_run_steps","automation_runs","automation_trigger_rules",
  "welcome_email_automation_runs","welcome_email_automated_emails","tinybird_syncs",
  "stripe_checkout_config","tier_requirements","tier_requirement_tiers"
 };
 static constexpr std::array<std::string_view,20> kDefaultTables{
  "posts","posts_authors","posts_meta","posts_tags","roles","roles_users","settings",
  "custom_theme_settings","tags","users","products","stripe_products","stripe_prices",
  "posts_products","newsletters","benefits","products_benefits","offers",
  "offer_redemptions","snippets"
 };
 [[nodiscard]] static bool backup_table(std::string_view value) {
  return std::ranges::find(kBackupTables,value)!=std::ranges::end(kBackupTables);
 }
 [[nodiscard]] static std::string quote(std::string_view value) {
  return core_serde::JsonCodec{}.encode(std::string{value});
 }
 [[nodiscard]] static std::string unquote(std::string_view value) {
  const auto decoded=core_serde::JsonCodec{}.decode<std::string>(value);
  return decoded?*decoded:std::string{value};
 }
 [[nodiscard]] static std::string basename(std::string_view value) {
  const auto pos=value.find_last_of("/\\");
  return std::string{pos==std::string_view::npos?value:value.substr(pos+1)};
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
 [[nodiscard]] static std::string disposition_filename(blog::BlogContext& ctx, std::string_view requested) {
  // exporter.fileName(): basename(filename)+'.json' or '<title>.ghost.<date>.json'
  // (export-filename.js:10-36).
  return requested.empty()?site_title(ctx)+".ghost."+stamp("%Y-%m-%d-%H-%M-%S")+".json"
                          :basename(requested)+".json";
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
