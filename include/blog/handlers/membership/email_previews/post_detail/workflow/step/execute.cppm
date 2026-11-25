export module blog_handlers:email_previews_post_detail_step;
import :domain_all;
import :email_previews_dto_subresources;
import :outbox_shim;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::email_previews::post_detail::workflow {

/** Outbox side effect standing in for Ghost's `SendingService.send` ->
 *  `MailgunEmailProvider.send`. The rendered test message is dispatched to a local
 *  adapter and recorded in the context-local `emails` resource so the send is
 *  observable without live credentials. */
namespace test_mail_side_effect {
    using Event = blog::handlers::outbox::Event;

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
            if (event.type != "email_previews.send_test") {
                return;
            }
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
    using PreviewInput = blog::dto::email_previews::preview_support::Input;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"email_previews_post_detail_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        using namespace blog::dto::email_previews::preview_support;

        const std::string path{in.get_path()};
        const std::string id = last_segment(path);
        if (id.empty()) {
            return fail("422", blog::api::ApiError{}.validation("id is required", "", "id"));
        }

        // Same service-layer option checks as GET (frame.options or frame.data).
        PreviewInput options;
        parse_options(path, options);
        const std::string body{in.body_as_string()};
        const auto document = parse_document(body);
        parse_data_options(data_object(document), options);
        if (const auto error = validate(options, path)) {
            return fail(
                "422",
                blog::api::ApiError{}.validation(error->message, "", error->property)
            );
        }
        fold_legacy_segment(options);

        // Ghost controller: emails = frame.data.emails ?? []; exactly one address.
        // Invalid addresses are filtered by validator.isEmail downstream, not here.
        const auto emails = data_emails(document);
        if (emails.empty()) {
            return fail(
                "422",
                blog::api::ApiError{}.validation("No emails provided.", "", "emails")
            );
        }
        if (emails.size() > 1) {
            return fail(
                "422",
                blog::api::ApiError{}.validation(
                    "Too many emails provided. Maximum of 1 test email can be sent at once.",
                    "",
                    "emails"
                )
            );
        }

        const auto post = select_post(ctx.store().json_for("posts", {{"filter[id]", id}}), id);
        if (!post) {
            return fail("404", blog::api::ApiError{}.json("Post not found.", ""));
        }
        std::map<std::string, std::string> newsletter_params;
        if (options.newsletter) {
            newsletter_params["filter[slug]"] = *options.newsletter;
        }
        const auto newsletters = ctx.store().json_for("newsletters", newsletter_params);
        const auto newsletter = select_newsletter(newsletters, options.newsletter.value_or(""));
        if (options.newsletter && !newsletter) {
            return fail("404", blog::api::ApiError{}.json("Newsletter not found.", ""));
        }

        // EmailService.sendTestEmail: example member + `[TEST]` subject, no DB write.
        std::string html = field_string(&*post, "html");
        std::string plaintext = field_string(&*post, "plaintext");
        if (plaintext.empty()) {
            plaintext = field_string(&*post, "excerpt");
        }
        if (plaintext.empty()) {
            plaintext = field_string(&*post, "custom_excerpt");
        }
        if (html.empty() || plaintext.empty()) {
            const auto [rendered_html, rendered_plaintext] = render_body(&*post);
            if (html.empty()) {
                html = rendered_html;
            }
            if (plaintext.empty()) {
                plaintext = rendered_plaintext;
            }
        }
        const auto subject = "[TEST] " + subject_of(&*post, newsletter ? &*newsletter : nullptr);
        html = replace_definitions(html, "", "", emails.front());
        plaintext = replace_definitions(plaintext, "", "", emails.front());

        const auto enc = [](std::string_view value)
        {
            return encode(value);
        };
        const std::string payload =
            "{\"to\":" + enc(emails.front()) + ",\"subject\":" + enc(subject) +
            ",\"html\":" + enc(html) + ",\"plaintext\":" + enc(plaintext) +
            ",\"message_id\":" + enc(message_id()) +
            ",\"tags\":[\"bulk-email\"],\"is_test_email\":true," +
            "\"click_tracking_enabled\":false,\"open_tracking_enabled\":false}";
        test_mail_side_effect::ensure();
        test_mail_side_effect::ctx_ptr() = &ctx;
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.emit(blog::handlers::outbox::Event{"email_previews.send_test", payload, id});
        [[maybe_unused]] const auto drained = outbox.drain();
        test_mail_side_effect::ctx_ptr() = nullptr;

        // Ghost `statusCode:204` with an empty body.
        return core::workflow::StepResponse<Output>{{{"status", "204"}, {"body", ""}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;
    using Array = serde::Value::Array;

    static core::workflow::StepResponse<Output> fail(std::string_view status, std::string body)
    {
        return core::workflow::StepResponse<Output>{
            {{"status", std::string{status}}, {"body", std::move(body)}}
        };
    }

    static std::string last_segment(std::string_view path)
    {
        const auto bare = path.substr(0, path.find('?'));
        auto tail = bare;
        while (tail.size() > 1 && tail.back() == '/') {
            tail.remove_suffix(1);
        }
        const auto slash = tail.find_last_of('/');
        return std::string{slash == std::string_view::npos ? tail : tail.substr(slash + 1)};
    }

    static std::optional<serde::Value> parse_document(std::string_view body)
    {
        if (body.empty()) {
            return std::nullopt;
        }
        return core_serde::JsonCodec{}.decode<serde::Value>(std::string{body});
    }

    static const Object* as_object(const serde::Value* value)
    {
        return value != nullptr ? std::get_if<Object>(&value->get()) : nullptr;
    }

    static const Object* data_object(const std::optional<serde::Value>& document)
    {
        if (!document) {
            return nullptr;
        }
        const auto* root = as_object(&*document);
        if (root == nullptr) {
            return nullptr;
        }
        const auto* data =
            as_object(blog::dto::email_previews::preview_support::value_for(root, "data"));
        return data != nullptr ? data : root;
    }

    static std::vector<std::string> data_emails(const std::optional<serde::Value>& document)
    {
        std::vector<std::string> out;
        const auto* root = document ? as_object(&*document) : nullptr;
        if (root == nullptr) {
            return out;
        }
        const auto* data =
            as_object(blog::dto::email_previews::preview_support::value_for(root, "data"));
        const auto* emails = blog::dto::email_previews::preview_support::value_for(
            data != nullptr ? data : root,
            "emails"
        );
        if (data != nullptr && emails == nullptr) {
            emails = blog::dto::email_previews::preview_support::value_for(root, "emails");
        }
        const auto* array = emails != nullptr ? std::get_if<Array>(&emails->get()) : nullptr;
        if (array == nullptr) {
            return out;
        }
        for (const auto& item: *array) {
            if (const auto* text = std::get_if<std::string>(&item.get())) {
                out.push_back(*text);
            }
        }
        return out;
    }

    /** Ghost test emails get a fresh random per-recipient Message-Id. */
    static std::string message_id()
    {
        static std::size_t counter{0};
        static thread_local std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<int> digit{0, 15};
        const auto hex = [&](int count)
        {
            std::string out;
            out.reserve(static_cast<std::size_t>(count));
            for (int i = 0; i < count; ++i) {
                out.push_back("0123456789abcdef"[digit(rng)]);
            }
            return out;
        };
        const auto uuid = hex(8) + "-" + hex(4) + "-4" + hex(3) + "-a" + hex(3) + "-" + hex(12);
        return uuid + "." + hex(32) + "@example.com";
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::email_previews::post_detail::workflow
