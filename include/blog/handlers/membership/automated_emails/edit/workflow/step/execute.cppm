export module blog_handlers:automated_emails_edit_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import :outbox_shim;
import interfaces;

export namespace blog::handlers::automated_emails::edit::workflow {

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
    static constexpr std::string_view name{"automated_emails_edit_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const std::string path{in.get_path()};
        const auto qpos = path.find('?');
        const std::string clean = qpos == std::string::npos ? path : path.substr(0, qpos);
        const std::string id{clean.substr(clean.rfind('/') + 1)}; // options.id (required)
        if (id.empty()) {
            return invalid("id is required", "id");
        }

        std::string raw{in.body_as_string()};
        if (raw.empty()) {
            raw = "{}";
        }
        Object automation;
        Object email;
        Object sender;
        std::string slug;
        bool has_slug = false;
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(raw);
        if (parsed) {
            if (const auto* root = std::get_if<Object>(&parsed->get())) {
                const auto* data = value_for(root, "data");
                if (data != nullptr) {
                    if (const auto* dobj = std::get_if<Object>(&data->get())) {
                        const auto* arr = value_for(dobj, "automated_emails");
                        if (arr != nullptr) {
                            if (const auto* list = std::get_if<serde::Value::Array>(&arr->get())) {
                                if (!list->empty()) {
                                    if (const auto* item =
                                            std::get_if<Object>(&list->front().get())) {
                                        automation = project(item, {"status", "name", "slug"});
                                        email = project(
                                            item,
                                            {"subject", "lexical", "email_design_setting_id"}
                                        );
                                        sender = project(
                                            item,
                                            {"sender_name", "sender_email", "sender_reply_to"}
                                        );
                                        slug = field(item, "slug");
                                        has_slug = value_for(item, "slug") != nullptr;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        // validators/input/automated_emails.js#edit: slug is optional but restricted.
        if (!field(&automation, "name").empty() && !valid_name(field(&automation, "name"))) {
            return invalid(
                "Name must be one of: Free member welcome flow, Paid member welcome flow",
                "name"
            );
        }
        const std::string status = field(&automation, "status");
        if (!status.empty() && status != "inactive" && status != "active") {
            return invalid("Status must be one of: inactive, active", "status");
        }
        if (has_slug && !valid_slug(slug)) {
            return invalid(
                "Slug must be one of: member-welcome-email-free, member-welcome-email-paid",
                "slug"
            );
        }
        const std::string lexical = field(&email, "lexical");
        if (!lexical.empty() && !valid_json(lexical)) {
            return invalid("Lexical must be a valid JSON string", "lexical");
        }

        // Automation.findOne({id}) -> NotFoundError.
        const Object row =
            first_row(ctx.store().json_for("automated_emails", {{"filter", "id:" + id}}));
        if (row.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body", blog::api::ApiError{}.json("Automated email not found.", "Not Found")}}
            };
        }

        // Status-change guard: an undeleted automation_actions row means this welcome
        // email was converted to an automation.
        if (value_for(&automation, "status") != nullptr && status != field(&row, "status") &&
            has_action(ctx.store(), id)) {
            return invalid(
                "Welcome email has been converted to an automation. Change its status through the "
                "automations API.",
                "status"
            );
        }
        // Slug immutability.
        if (has_slug && slug != field(&row, "slug")) {
            return invalid("Automated email slug cannot be changed.", "slug");
        }

        const Object design = first_row(ctx.store().json_for(
            "email_design_settings",
            {{"filter", "slug:default-automated-email"}}
        ));
        const std::string design_id = [&]
        {
            const std::string own = field(&row, "email_design_setting_id");
            return own.empty() ? field(&design, "id") : own;
        }();
        // getChangedSenderData(): validate+persist only fields that actually changed.
        Object changed;
        for (const auto key: {"sender_name", "sender_email", "sender_reply_to"}) {
            const auto* value = value_for(&sender, key);
            if (value == nullptr) {
                continue;
            }
            const std::string text = field(&sender, key);
            if (text == field(&design, key)) {
                continue;
            }
            if ((std::string{key} == "sender_email" || std::string{key} == "sender_reply_to") &&
                !text.empty() && !valid_email(text)) {
                return core::workflow::StepResponse<Output>{
                    {{"status", "422"},
                     {"body",
                      blog::api::ApiError{}.validation(
                          "You cannot set " + std::string{key} + " to " + text,
                          "",
                          key
                      )}}
                };
            }
            changed[std::string{key}] = *value;
        }

        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        (void)ctx.store()
            .edit("automations", id, core_serde::JsonCodec{}.encode(serde::Value{automation}));
        const std::string email_id = [&]
        {
            const std::string linked = field(&row, "welcome_email_automated_email_id");
            return linked.empty() ? id : linked;
        }();
        if (!email.empty()) {
            (void)ctx.store().edit(
                "welcome_email_automated_emails",
                email_id,
                core_serde::JsonCodec{}.encode(serde::Value{email})
            );
        }
        if (!changed.empty() && !design_id.empty()) {
            (void)ctx.store().edit(
                "email_design_settings",
                design_id,
                core_serde::JsonCodec{}.encode(serde::Value{changed})
            );
        }
        outbox.emit(
            blog::handlers::outbox::Event{"automation.edited", "{\"id\":\"" + id + "\"}", id}
        );
        outbox.commit();
        [[maybe_unused]] const auto drained = outbox.drain();

        const Object after =
            first_row(ctx.store().json_for("automated_emails", {{"filter", "id:" + id}}));
        Object flat;
        if (!after.empty()) {
            flat["id"] = after.at("id");
            flat["status"] = value_or_null(&after, "status");
            flat["name"] = value_or_null(&after, "name");
            flat["slug"] = value_or_null(&after, "slug");
            flat["subject"] = value_or_null(&email, "subject");
            flat["lexical"] = value_or_null(&email, "lexical");
            for (const auto key: {"sender_name", "sender_email", "sender_reply_to"}) {
                flat[key] = value_for(&changed, key) != nullptr ? changed.at(std::string{key})
                                                                : value_or_null(&design, key);
            }
            flat["email_design_setting_id"] = serde::Value{design_id};
            flat["created_at"] = value_or_null(&after, "created_at");
            flat["updated_at"] = value_or_null(&after, "updated_at");
        }
        return core::workflow::StepResponse<Output>{
            {{"status", "200"},
             {"body",
              local_mapping::wrap(
                  "automated_emails",
                  core_serde::JsonCodec{}.encode(serde::Value{flat})
              )}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;

    static core::workflow::StepResponse<Output>
    invalid(std::string_view message, std::string_view property)
    {
        return core::workflow::StepResponse<Output>{
            {{"status", "422"}, {"body", blog::api::ApiError{}.validation(message, "", property)}}
        };
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

    static serde::Value value_or_null(const Object* object, std::string_view key)
    {
        const auto* value = value_for(object, key);
        return value != nullptr ? *value : serde::Value{};
    }

    static Object project(const Object* object, std::initializer_list<std::string_view> keys)
    {
        Object out;
        for (const auto key: keys) {
            const auto* value = value_for(object, key);
            if (value != nullptr) {
                out[std::string{key}] = *value;
            }
        }
        return out;
    }

    static bool valid_name(std::string_view name)
    {
        return name == "Free member welcome flow" || name == "Paid member welcome flow";
    }

    static bool valid_slug(std::string_view slug)
    {
        return slug == "member-welcome-email-free" || slug == "member-welcome-email-paid";
    }

    static bool valid_json(std::string_view text)
    {
        return core_serde::JsonCodec{}.decode<serde::Value>(text).has_value();
    }

    static bool valid_email(std::string_view value)
    {
        const auto at = value.find('@');
        if (at == std::string_view::npos || at == 0 || at + 1 >= value.size()) {
            return false;
        }
        if (value.find(' ') != std::string_view::npos) {
            return false;
        }
        const auto domain = value.substr(at + 1);
        return domain.find('.') != std::string_view::npos && domain.front() != '.' &&
               domain.back() != '.';
    }

    static Object first_row(std::string_view json)
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

    static bool has_action(blog::api::ApiDataStore& store, std::string_view automation_id)
    {
        const auto rows = store.resources().list("automation_actions");
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(rows);
        if (!parsed) {
            return false;
        }
        if (const auto* list = std::get_if<serde::Value::Array>(&parsed->get())) {
            for (const auto& item: *list) {
                if (const auto* object = std::get_if<Object>(&item.get())) {
                    if (field(object, "automation_id") != automation_id) {
                        continue;
                    }
                    const std::string deleted = field(object, "deleted_at");
                    if (deleted.empty() || deleted == "null") {
                        return true;
                    }
                }
            }
        }
        return false;
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::automated_emails::edit::workflow
