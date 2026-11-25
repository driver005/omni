export module blog_handlers:settings_bulk_edit_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import :settings_browse_step;

export namespace blog::handlers::settings::bulk_edit::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = support::Result;
    static constexpr std::string_view name{"settings_bulk_edit_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        using namespace support;
        const std::string body = in.body_as_string();
        const auto entries = settings_array(body);
        if (!entries) {
            return core::workflow::StepResponse<Output>{
                fail(400, validation_error("Settings are required", "settings"))
            };
        }

        const auto stored = overrides(ctx);
        std::vector<std::pair<std::string, std::string>> edits;
        bool email_changed = false;
        bool automations_enabled = false;
        bool stripe_connect = false;

        for (const auto& entry: *entries) {
            const auto* item = as_object(&entry);
            if (item == nullptr) {
                continue;
            }
            const auto key = string_field(item, "key");
            if (key.empty()) {
                return core::workflow::StepResponse<Output>{
                    fail(400, validation_error("Setting key is required", "key"))
                };
            }

            // Ghost input serializer: drop non-allowlisted keys for non-internal frames.
            if (!contains(kEditable, key)) {
                continue;
            }
            // Ghost BREAD service: drop stripe-connect token/keys and obfuscated secrets.
            if (contains(kSecretFiltered, key)) {
                if (key == "stripe_connect_integration_token" &&
                    !string_field(item, "value").empty()) {
                    stripe_connect = true;
                }
                continue;
            }

            for (const auto& def: kSettings) {
                if (def.key != key) {
                    continue;
                }
                if (def.group == "core") {
                    return core::workflow::StepResponse<Output>{fail(
                        403,
                        no_permission("Attempted to access core setting from external request")
                    )};
                }
            }

            auto value = raw_field(item, "value", "null");
            const auto raw = string_field(item, "value");

            // Validator: array settings, email, announcement allowed values, labs flags.
            if (contains(kArray, key) && !value.starts_with('[')) {
                return core::workflow::StepResponse<Output>{
                    fail(400, validation_error("Please send a valid array", key))
                };
            }
            if (key == "members_support_address" && !raw.empty() && !valid_email(raw)) {
                return core::workflow::StepResponse<Output>{
                    fail(400, validation_error("Please enter a valid email address.", key))
                };
            }
            if (key == "announcement_visibility" && raw != "public" && raw != "members" &&
                raw != "paid" && raw != "public_free" && raw != "public_paid" &&
                raw != "public_only" && raw != "free_members" && raw != "none") {
                return core::workflow::StepResponse<Output>{fail(
                    400,
                    validation_error("Please enter a valid announcement visibility value", key)
                )};
            }
            if (key == "announcement_background" && raw != "#ffffff" && raw != "#000000" &&
                !raw.empty()) {
                return core::workflow::StepResponse<Output>{fail(
                    400,
                    validation_error("Please enter a valid announcement background value", key)
                )};
            }

            if (key == "labs") {
                const auto labs = decode(value);
                Object filtered;
                if (labs) {
                    if (const auto* obj = as_object(&*labs)) {
                        for (const auto& [flag, flag_value]: *obj) {
                            if (contains(kLabsWritable, flag)) {
                                filtered[flag] = flag_value;
                            }
                        }
                    }
                }
                value = encode(serde::Value{filtered});
                const auto before = setting_value("labs", stored);
                for (const auto& [flag, flag_value]: filtered) {
                    const auto* enabled = std::get_if<bool>(&flag_value.get());
                    if (enabled != nullptr && *enabled &&
                        before.find("\"" + flag + "\":true") == std::string::npos) {
                        automations_enabled = true;
                    }
                }
            }

            value = coerce(key, value);
            if (key == "members_support_address") {
                email_changed = !raw.empty();
            }
            edits.emplace_back(key, value);
        }

        // Unknown-setting guard (Ghost NotFoundError "Problem finding setting: {key}").
        for (const auto& [key, value]: edits) {
            (void)value;
            bool known = false;
            for (const auto& def: kSettings) {
                if (def.key == key) {
                    known = true;
                    break;
                }
            }
            if (!known) {
                return core::workflow::StepResponse<Output>{
                    fail(404, not_found("Problem finding setting: " + key))
                };
            }
        }

        // SettingsModel.edit — dedup write-through of every accepted setting.
        for (const auto& [key, value]: edits) {
            write_setting(ctx, key, value);
        }

        // Side effects (observable on the shared outbox, CONTRACTS.md §4).
        if (!edits.empty()) {
            std::vector<blog::handlers::outbox::Event> events;
            for (const auto& [key, value]: edits) {
                events.push_back(
                    {.type = "settings.edited",
                     .payload = R"({"key":)" + quote(key) + R"(,"value":)" + value + "}",
                     .aggregate_id = "settings"}
                );
            }
            if (stripe_connect) {
                events.push_back(
                    {.type = "stripe.connected", .payload = "{}", .aggregate_id = "settings"}
                );
            }
            if (automations_enabled) {
                events.push_back(
                    {.type = "automations.beta_flag_enabled",
                     .payload = "{}",
                     .aggregate_id = "settings"}
                );
            }
            if (email_changed) {
                events.push_back(
                    {.type = "email.send_verification", .payload = "{}", .aggregate_id = "settings"}
                );
            }
            events.push_back(
                {.type = "cache.invalidate",
                 .payload = R"({"url":"/*"})",
                 .aggregate_id = "settings"}
            );
            emit_transaction(events);
        }

        // Ghost: full browse for the return + `meta` from the edit result.
        auto result = support::browse(ctx, {});
        std::string meta = "{}";
        if (email_changed) {
            meta = R"({"sent_email_verification":true})";
        }
        result["body"] =
            std::format(R"({{"settings":{},"meta":{}}})", settings_only(result["body"]), meta);
        if (!edits.empty()) {
            result["cache_invalidate"] = "/*";
        }
        return core::workflow::StepResponse<Output>{result};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::settings::bulk_edit::workflow
