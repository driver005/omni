export module blog_handlers:newsletters_create_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_serde;
import interfaces;
import :outbox_shim;
import :newsletters_browse_step;

export namespace blog::handlers::newsletters::create::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"newsletters_create_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        auto& ctx = *s_ctx;

        constexpr std::array<std::string_view, 2> kOptions{"include", "opt_in_existing"};
        auto q = core::query::parse(in.get_path(), kOptions);
        if (!valid_include(q)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body", validation("Validation error", "Invalid include", "include")}}
            };
        }

        // Controller passes frame.data.newsletters[0] to service.add.
        Object attrs;
        const auto raw = std::string{in.body_as_string()};
        const auto parsed = raw.empty() ? std::nullopt : decode(raw);
        if (parsed) {
            const auto* root = std::get_if<Object>(&parsed->get());
            if (root != nullptr) {
                const auto* data = value_for(root, "data");
                const auto* source = root;
                if (data != nullptr) {
                    if (const auto* d = std::get_if<Object>(&data->get())) {
                        source = d;
                    }
                }
                const auto* list = value_for(source, "newsletters");
                if (list != nullptr) {
                    if (const auto* array = std::get_if<Array>(&list->get())) {
                        if (!array->empty()) {
                            if (const auto* item = std::get_if<Object>(&array->front().get())) {
                                attrs = *item;
                            }
                        }
                    }
                }
            }
        }

        // prepAttrsForEmailVerification: validate sender addresses first.
        for (const auto property:
             {std::string_view{"sender_email"}, std::string_view{"sender_reply_to"}}) {
            const std::string email = field(&attrs, property);
            if (email.empty()) {
                continue;
            }
            if (property == "sender_reply_to" && (email == "newsletter" || email == "support")) {
                continue;
            }
            if (!valid_email(email)) {
                const std::string message =
                    property == "sender_email"
                        ? "You cannot set the sender email address to " + email
                        : "You cannot set the reply-to email address to " + email;
                return core::workflow::StepResponse<Output>{
                    {{"status", "422"}, {"body", validation(message, "", property)}}
                };
            }
        }
        const std::string name = field(&attrs, "name");
        if (!name.empty() && name_taken(ctx, name)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body", validation("A newsletter with the same name already exists", "", "name")}}
            };
        }

        // Managed-email policy: strip the properties that require verification, then
        // send a magic-link mail (observable as an outbox event) for each one.
        std::vector<std::pair<std::string, std::string>> to_verify;
        const bool managed = managed_email_enabled(ctx);
        if (managed) {
            for (const auto property:
                 {std::string_view{"sender_email"}, std::string_view{"sender_reply_to"}}) {
                const std::string email = field(&attrs, property);
                if (email.empty()) {
                    continue;
                }
                if (property == "sender_reply_to" && email == field(&attrs, "sender_email")) {
                    continue;
                }
                to_verify.emplace_back(property, email);
                Object kept;
                for (const auto& entry: attrs) {
                    if (entry.first != property) {
                        kept[entry.first] = entry.second;
                    }
                }
                attrs = std::move(kept);
            }
        }

        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        const auto result = ctx.store().create("newsletters", encode(serde::Value{attrs}));
        const std::string id = first_id(result.body());
        if (!id.empty()) {
            // getNextAvailableSortOrder() -> max(sort_order)+1, stored as a JSON number.
            static_cast<void>(ctx.store().edit(
                "newsletters",
                id,
                "{\"sort_order\":" + std::to_string(next_sort_order(ctx)) + "}"
            ));
        }
        for (const auto& [property, email]: to_verify) {
            outbox.emit(
                blog::handlers::outbox::Event{
                    "email.send_verification",
                    "{\"property\":\"" + escape(property) + "\",\"email\":\"" + escape(email) +
                        "\",\"subject\":\"Verify email address\"}",
                    id
                }
            );
            static_cast<void>(issue_verification_token(ctx, id, property, email));
        }
        outbox.emit(
            blog::handlers::outbox::Event{"newsletter.added", "{\"id\":\"" + escape(id) + "\"}", id}
        );
        outbox.commit();
        mail_side_effect::ctx_ptr() = &ctx;
        mail_side_effect::ensure();
        [[maybe_unused]] const auto drained = outbox.drain();
        mail_side_effect::ctx_ptr() = nullptr;

        // opt_in_existing -> MemberModel.fetchAllSubscribed + subscribeMembersById.
        std::size_t opted_in{0};
        const auto opt_it = q.options.find("opt_in_existing");
        const bool opted = opt_it != q.options.end() && opt_it->second == "true";
        if (opted) {
            core::query::ParsedQuery subscribed;
            subscribed.raw_filter = "subscribed:true";
            subscribed.limit = std::numeric_limits<std::size_t>::max();
            opted_in =
                count_rows(core::query::apply(ctx.store().json_for("members", {}), subscribed));
            outbox.emit(
                blog::handlers::outbox::Event{
                    "newsletter.members_subscribed",
                    "{\"id\":\"" + escape(id) + "\",\"count\":" + std::to_string(opted_in) + "}",
                    id
                }
            );
            [[maybe_unused]] const auto extra = outbox.drain();
        }

        // Service reloads findOne({id}, require:true) before returning.
        std::string model;
        const auto reloaded = ctx.store().find_by_resource("newsletters", id);
        if (reloaded) {
            model = *reloaded;
        } else {
            model = first_row(result.body()).empty()
                        ? std::string{"{}"}
                        : encode(serde::Value{first_row(result.body())});
        }

        std::string meta;
        if (opted) {
            meta += ",\"opted_in_member_count\":" + std::to_string(opted_in);
        }
        if (!to_verify.empty()) {
            meta += ",\"sent_email_verification\":[";
            for (std::size_t i = 0; i < to_verify.size(); ++i) {
                meta += std::string{i ? "," : ""} + "\"" + to_verify[i].first + "\"";
            }
            meta += "]";
        }
        if (!meta.empty()) {
            meta = "{" + meta.substr(1) + "}";
        }
        const std::string body = wrap("newsletters", model, meta);
        return core::workflow::StepResponse<Output>{
            {{"status", std::to_string(result.status())}, {"body", body}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static bool name_taken(blog::BlogContext& ctx, std::string_view name)
    {
        const auto parsed = support::decode(ctx.store().json_for("newsletters", {}));
        if (!parsed) {
            return false;
        }
        if (const auto* list = std::get_if<support::Array>(&parsed->get())) {
            for (const auto& item: *list) {
                if (const auto* object = std::get_if<support::Object>(&item.get())) {
                    std::string other = support::field(object, "name");
                    std::ranges::transform(
                        other,
                        other.begin(),
                        [](unsigned char c)
                        {
                            return static_cast<char>(std::tolower(c));
                        }
                    );
                    std::string wanted{name};
                    std::ranges::transform(
                        wanted,
                        wanted.begin(),
                        [](unsigned char c)
                        {
                            return static_cast<char>(std::tolower(c));
                        }
                    );
                    if (other == wanted) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::newsletters::create::workflow
