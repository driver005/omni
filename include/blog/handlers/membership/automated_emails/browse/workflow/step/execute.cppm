export module blog_handlers:automated_emails_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_serde;
import interfaces;

export namespace blog::handlers::automated_emails::browse::workflow {

namespace local_mapping {
    [[nodiscard]] inline std::string
    wrap(std::string_view resource, std::string_view rows, std::string_view meta = {})
    {
        return "{\"" + std::string{resource} + "\":" + std::string{rows} +
               (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
    }

    [[nodiscard]] inline std::string clean(std::string_view json, std::string_view)
    {
        return std::string{json};
    }

    [[nodiscard]] inline std::string pagination(
        std::size_t total,
        std::size_t limit,
        std::size_t page,
        std::size_t pages,
        std::string_view prev,
        std::string_view next
    )
    {
        return std::format(
            R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
            total,
            limit,
            page,
            pages,
            prev.empty() ? std::string{"null"} : std::string{prev},
            next.empty() ? std::string{"null"} : std::string{next}
        );
    }
} // namespace local_mapping

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"automated_emails_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        // Ghost options: ['fields','limit','order','page'] + restrictAdminApiQueryOptions.
        // core::query::parse applies the allowlist / regex validation; run() enforces the
        // restricted-field guard (password|token|gift_link_token|secret) and throws on a
        // restricted order/filter, which the controller maps to a 400 BadRequestError.
        static constexpr std::string_view kOptions[]{"fields", "limit", "order", "page"};
        const auto q = core::query::parse(in.get_path(), kOptions);

        // Ghost getDefaultEmailDesignSettings(): NotFoundError when the shared default
        // design row is absent.
        const auto default_rows = ctx.store().json_for(
            "email_design_settings",
            {{"filter", "slug:default-automated-email"}}
        );
        const Object default_design = first_object(default_rows);
        if (default_design.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body",
                  blog::api::ApiError{}
                      .json("Default automated email design setting not found", "Not Found")}}
            };
        }

        // The fixture-backed `automated_emails` resource is the C++ facade over the
        // split Automation + WelcomeEmailAutomatedEmail tables. Flatten it to the
        // Ghost 12-field shape, falling back to the default design sender settings.
        const auto raw = ctx.store().json_for("automated_emails", {});
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(raw);
        std::string rows = "[]";
        if (parsed) {
            if (const auto* list = std::get_if<serde::Value::Array>(&parsed->get())) {
                rows = "[";
                bool first = true;
                for (const auto& item: *list) {
                    if (const auto* object = std::get_if<Object>(&item.get())) {
                        const std::string slug = field(object, "slug");
                        // MEMBER_WELCOME_EMAIL_FILTER: server-side fixed slug filter.
                        if (!slug.empty() && !welcome_slug(slug)) {
                            continue;
                        }
                        rows += (first ? "" : ",") + flatten(object, &default_design);
                        first = false;
                    }
                }
                rows += "]";
            }
        }

        std::string ran;
        try {
            ran = core::query::run(rows, q, 15);
        } catch (const std::exception&) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body",
                  blog::api::ApiError{}
                      .json("Restricted fields cannot be used in order.", "Bad Request")}}
            };
        }
        const auto parts = blog::api::JsonBody{}.parse_raw(ran);
        const auto meta = blog::api::JsonBody{}.parse_raw(
            parts.contains("meta") ? parts.at("meta") : std::string{"{}"}
        );
        const auto pagination_json =
            meta.contains("pagination") ? meta.at("pagination") : std::string{"{}"};
        const auto body = local_mapping::wrap(
            "automated_emails",
            parts.contains("data") ? parts.at("data") : std::string{"[]"},
            "{\"pagination\":" + pagination_json + "}"
        );
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;

    static bool welcome_slug(std::string_view slug)
    {
        return slug == "member-welcome-email-free" || slug == "member-welcome-email-paid";
    }

    static const serde::Value* value_for(const Object* object, std::string_view key)
    {
        if (object == nullptr) {
            return nullptr;
        }
        for (const auto& [name, value]: *object) {
            if (name == key) {
                return &value;
            }
        }
        return nullptr;
    }

    static std::string field(const Object* object, std::string_view key)
    {
        const auto* value = value_for(object, key);
        if (value == nullptr) {
            return {};
        }
        if (const auto* text = std::get_if<std::string>(&value->get())) {
            return *text;
        }
        return {};
    }

    static std::string raw_value(const Object* object, std::string_view key)
    {
        const auto* value = value_for(object, key);
        return value == nullptr ? std::string{"null"} : core_serde::JsonCodec{}.encode(*value);
    }

    static Object first_object(std::string_view json)
    {
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(json);
        if (parsed) {
            if (const auto* list = std::get_if<serde::Value::Array>(&parsed->get())) {
                if (!list->empty()) {
                    if (const auto* object = std::get_if<Object>(&list->front().get())) {
                        return *object;
                    }
                }
            }
        }
        return {};
    }

    static std::string flatten(const Object* automation, const Object* design)
    {
        const std::string design_id = field(design, "id");
        const std::string email_design_id = [&]
        {
            const std::string id = field(automation, "email_design_setting_id");
            return id.empty() ? design_id : id;
        }();
        const auto item = [&](std::string_view key, std::string value)
        {
            return "\"" + std::string{key} + "\":" + std::move(value);
        };
        return "{" + item("id", raw_value(automation, "id")) + "," +
               item("status", raw_value(automation, "status")) + "," +
               item("name", raw_value(automation, "name")) + "," +
               item("slug", raw_value(automation, "slug")) + "," +
               item("subject", raw_value(automation, "subject")) + "," +
               item("lexical", raw_value(automation, "lexical")) + "," +
               item("sender_name", raw_value(design, "sender_name")) + "," +
               item("sender_email", raw_value(design, "sender_email")) + "," +
               item("sender_reply_to", raw_value(design, "sender_reply_to")) + "," +
               item("email_design_setting_id", "\"" + email_design_id + "\"") + "," +
               item("created_at", raw_value(automation, "created_at")) + "," +
               item("updated_at", raw_value(automation, "updated_at")) + "}";
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::automated_emails::browse::workflow
