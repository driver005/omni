export module blog_handlers:automated_emails_automated_emails_id_test_post_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import :outbox_shim;
import interfaces;

export namespace blog::handlers::automated_emails::automated_emails_id_test_post::workflow {

/** Outbox adapter standing in for Ghost's #sendTransactionalEmail (SMTP). The
 *  rendered test message is recorded in the context-local `emails` resource and an
 *  in-process log so the send is observable without live credentials. */
namespace test_mail_side_effect {
    using Event = blog::handlers::outbox::Event;

    inline std::vector<Event>& log()
    {
        static std::vector<Event> v;
        return v;
    }

    inline blog::BlogContext*& ctx_ptr()
    {
        static blog::BlogContext* p{nullptr};
        return p;
    }

    class Adapter final : public blog::handlers::outbox::Adapter
    {
    public:
        void dispatch(const Event& event) override
        {
            if (event.type != "email.send_test") {
                return;
            }
            log().push_back(event);
            if (ctx_ptr() != nullptr) {
                ctx_ptr()->store().resources().create("emails", event.payload);
            }
        }
    };

    inline void ensure()
    {
        static const bool registered = []
        {
            blog::handlers::outbox::register_adapter(std::make_shared<Adapter>());
            return true;
        }();
        static_cast<void>(registered);
    }
} // namespace test_mail_side_effect

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{
        "automated_emails_automated_emails_id_test_post_execute"
    };

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const std::string id = parent_segment(in.get_path());
        if (id.empty()) {
            return invalid("id is required", "id");
        }

        std::string raw{in.body_as_string()};
        if (raw.empty()) {
            raw = "{}";
        }
        Object data;
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(raw);
        if (parsed) {
            if (const auto* root = std::get_if<Object>(&parsed->get())) {
                const auto* node = value_for(root, "data");
                if (node != nullptr) {
                    if (const auto* dobj = std::get_if<Object>(&node->get())) {
                        data = *dobj;
                    }
                }
                if (data.empty()) {
                    data = *root;
                }
            }
        }
        const std::string email = field(&data, "email");
        if (!valid_email(email)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}
                      .validation("The server did not receive a valid email", "", "email")}}
            };
        }
        const std::string subject = field(&data, "subject");
        const std::string lexical = field(&data, "lexical");
        if (subject.empty()) {
            return invalid("Subject is required", "subject");
        }
        if (lexical.empty()) {
            return invalid("Email content is required", "lexical");
        }
        if (!core_serde::JsonCodec{}.decode<serde::Value>(lexical)) {
            return invalid("Lexical must be a valid JSON string", "lexical");
        }

        const auto rows = ctx.store().json_for("automated_emails", {{"filter", "id:" + id}});
        const auto parsed_rows = core_serde::JsonCodec{}.decode<serde::Value>(rows);
        bool found = false;
        if (parsed_rows) {
            if (const auto* list = std::get_if<serde::Value::Array>(&parsed_rows->get())) {
                for (const auto& item: *list) {
                    if (const auto* object = std::get_if<Object>(&item.get())) {
                        if (field(object, "id") == id) {
                            found = true;
                        }
                    }
                }
            }
        }
        if (!found) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body", blog::api::ApiError{}.json("No member welcome email found", "Not Found")}}
            };
        }

        // #sendTestAutomationEmail: render with the submitted member email, refresh the
        // sender options, then send with subject "[Test] <rendered subject>".
        const std::string rendered_subject = render_subject(subject, email);
        const auto [html, plaintext] = render_lexical(lexical);
        const auto enc = [](std::string_view value)
        {
            return core_serde::JsonCodec{}.encode(serde::Value{std::string{value}});
        };
        const std::string payload = "{\"to\":" + enc(email) +
                                    ",\"subject\":" + enc("[Test] " + rendered_subject) +
                                    ",\"html\":" + enc(html) + ",\"plaintext\":" + enc(plaintext) +
                                    ",\"force_text_content\":true}";
        test_mail_side_effect::ensure();
        test_mail_side_effect::ctx_ptr() = &ctx;
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.emit(blog::handlers::outbox::Event{"email.send_test", payload, id});
        [[maybe_unused]] const auto drained = outbox.drain();
        test_mail_side_effect::ctx_ptr() = nullptr;

        // Ghost statusCode:204, no response body.
        return core::workflow::StepResponse<Output>{{{"status", "204"}, {"body", ""}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;
    using Array = serde::Value::Array;

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

    static std::string parent_segment(std::string_view path)
    {
        const auto clean = path.substr(0, path.find('?'));
        const auto last = clean.rfind('/');
        if (last == std::string_view::npos || last == 0) {
            return {};
        }
        const auto before = clean.substr(0, last);
        const auto prev = before.rfind('/');
        return std::string{before.substr(prev == std::string_view::npos ? 0 : prev + 1)};
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

    static std::string replace_all(std::string text, std::string_view token, std::string_view value)
    {
        for (;;) {
            const auto pos = text.find(token);
            if (pos == std::string::npos) {
                break;
            }
            text.replace(pos, token.size(), value);
        }
        return text;
    }

    static std::string render_subject(std::string subject, std::string_view email)
    {
        subject = replace_all(subject, "%%{first_name}%%", "Jamie");
        subject = replace_all(subject, "%%{name}%%", "Jamie Larson");
        subject = replace_all(subject, "%%{email}%%", email);
        subject = replace_all(subject, "%%{uuid}%%", "00000000-0000-4000-8000-000000000000");
        return subject;
    }

    static void collect_text(const serde::Value& node, std::vector<std::string>& out)
    {
        if (const auto* object = std::get_if<Object>(&node.get())) {
            if (field(object, "type") == "text") {
                const std::string text = field(object, "text");
                if (!text.empty()) {
                    out.push_back(text);
                }
            }
            for (const auto& [key, value]: *object) {
                if (key == "children" || key == "root") {
                    collect_text(value, out);
                }
            }
        } else if (const auto* array = std::get_if<Array>(&node.get())) {
            for (const auto& item: *array) {
                collect_text(item, out);
            }
        }
    }

    static std::pair<std::string, std::string> render_lexical(std::string_view lexical)
    {
        std::vector<std::string> parts;
        if (const auto doc = core_serde::JsonCodec{}.decode<serde::Value>(lexical)) {
            collect_text(*doc, parts);
        }
        std::string plaintext;
        for (std::size_t i = 0; i < parts.size(); ++i) {
            plaintext += (i ? "\n" : "") + parts[i];
        }
        std::string html;
        for (const auto& part: parts) {
            html += "<p>" + part + "</p>";
        }
        if (html.empty()) {
            html = "<p></p>";
        }
        return {html, plaintext};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::automated_emails::automated_emails_id_test_post::workflow
