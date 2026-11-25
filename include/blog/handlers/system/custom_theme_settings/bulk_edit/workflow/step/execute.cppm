export module blog_handlers:custom_theme_settings_bulk_edit_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::custom_theme_settings::bulk_edit::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"custom_theme_settings_bulk_edit_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const auto body =
            in.body_as_string().empty() ? std::string{"{}"} : std::string{in.body_as_string()};
        auto settings = unwrap(body);
        if (!settings) {
            settings = serde::Value::Array{};
        }

        // Ghost validates every key against the active theme's setting definitions
        // (`_activeThemeSettings`, service.js:100-146) before saving. In the C++
        // stand-in the definition fields live on the context-local rows; there is no
        // theme-activation step in these endpoints.
        const auto defs = definitions(ctx.store().json_for("custom_theme_settings", {}));
        for (const auto& element: *settings) {
            const auto obj = element.to_object();
            if (!obj) {
                return invalid(
                    "Validation error, cannot save custom theme settings.",
                    "key and value are required",
                    "custom_theme_settings"
                );
            }
            const auto key = scalar(*obj, "key");
            const auto* value = lookup(*obj, "value");
            if (key.empty() || value == nullptr) {
                return invalid(
                    "Validation error, cannot save custom theme settings.",
                    "key and value are required",
                    "custom_theme_settings"
                );
            }
            const auto it = defs.find(key);
            if (it == defs.end()) {
                return invalid("Unknown setting: " + key + ".", "", "custom_theme_settings");
            }
            const auto message = validate(key, it->second.object, *value);
            if (!message.empty()) {
                return invalid(message, "", "custom_theme_settings");
            }
        }

        // Ghost does repository.read + save(null) only when the value changed
        // (service.js:149-159): update each existing record in place by its key.
        // No bulk_edit envelope, no create of duplicate rows.
        for (const auto& element: *settings) {
            const auto obj = element.to_object();
            if (!obj) {
                continue;
            }
            const auto& def = defs.at(scalar(*obj, "key"));
            const auto* value = lookup(*obj, "value");
            const auto value_json = core_serde::JsonCodec{}.encode(*value);
            const bool updated = !def.id.empty() && ctx.store().resources().edit(
                                                        "custom_theme_settings",
                                                        def.id,
                                                        "{\"value\":" + value_json + "}"
                                                    );
            if (!updated) {
                ctx.store().resources().create(
                    "custom_theme_settings",
                    core_serde::JsonCodec{}.encode(element)
                );
            }
        }

        const auto rows = ctx.store().json_for("custom_theme_settings", {});
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", envelope(rows)}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    // Ghost output serializer: rows under docName, no pagination meta.
    static std::string envelope(std::string_view rows)
    {
        while (!rows.empty() && std::isspace(static_cast<unsigned char>(rows.front()))) {
            rows.remove_prefix(1);
        }
        while (!rows.empty() && std::isspace(static_cast<unsigned char>(rows.back()))) {
            rows.remove_suffix(1);
        }
        const auto data = rows.size() >= 2 && rows.front() == '[' && rows.back() == ']'
                              ? std::string{rows}
                              : std::string{"[]"};
        return "{\"custom_theme_settings\":" + data + "}";
    }

    struct Definition
    {
        std::string id;
        serde::Value::Object object;
    };

    static const serde::Value* lookup(const serde::Value::Object& obj, std::string_view key)
    {
        for (const auto& [k, v]: obj) {
            if (k == key) {
                return &v;
            }
        }
        return nullptr;
    }

    static std::string scalar(const serde::Value::Object& obj, std::string_view key)
    {
        const auto* v = lookup(obj, key);
        if (v == nullptr) {
            return {};
        }
        if (const auto* text = std::get_if<std::string>(&v->get())) {
            return *text;
        }
        return {};
    }

    static std::map<std::string, Definition> definitions(std::string_view rows)
    {
        std::map<std::string, Definition> out;
        try {
            const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(std::string{rows});
            if (!parsed) {
                return out;
            }
            const auto arr = parsed->to_array();
            if (!arr) {
                return out;
            }
            for (const auto& elem: *arr) {
                auto obj = elem.to_object();
                if (!obj) {
                    continue;
                }
                const auto key = scalar(*obj, "key");
                if (key.empty()) {
                    continue;
                }
                out.insert_or_assign(key, Definition{scalar(*obj, "id"), std::move(*obj)});
            }
        } catch (...) {
        }
        return out;
    }

    static std::string
    validate(std::string_view key, const serde::Value::Object& def, const serde::Value& value)
    {
        const auto type = scalar(def, "type");
        if (type == "select") {
            std::optional<serde::Value::Array> arr;
            if (const auto* options = lookup(def, "options")) {
                if (auto parsed = options->to_array()) {
                    arr = std::optional<serde::Value::Array>{std::move(*parsed)};
                }
            }
            const auto* text = std::get_if<std::string>(&value.get());
            std::string allowed;
            bool found = false;
            if (arr && text) {
                for (const auto& opt: *arr) {
                    const auto* item = std::get_if<std::string>(&opt.get());
                    if (item == nullptr) {
                        continue;
                    }
                    if (!allowed.empty()) {
                        allowed += ", ";
                    }
                    allowed += *item;
                    if (*item == *text) {
                        found = true;
                    }
                }
            }
            if (!found) {
                return "Unallowed value for '" + std::string{key} +
                       "'. Allowed values: " + allowed + ".";
            }
        } else if (type == "boolean") {
            if (std::get_if<bool>(&value.get()) == nullptr) {
                return "Unallowed value for '" + std::string{key} +
                       "'. Allowed values: true, false.";
            }
        } else if (type == "color") {
            const auto* text = std::get_if<std::string>(&value.get());
            bool valid = text != nullptr && text->size() == 7 && (*text)[0] == '#';
            if (valid) {
                for (std::size_t i = 1; i < 7; ++i) {
                    if (!std::isxdigit(static_cast<unsigned char>((*text)[i]))) {
                        valid = false;
                        break;
                    }
                }
            }
            if (!valid) {
                return "Invalid value for '" + std::string{key} +
                       "'. The value must follow this format: #1234AF.";
            }
        }
        return {};
    }

    static std::optional<serde::Value::Array> unwrap(std::string_view body)
    {
        try {
            const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(std::string{body});
            if (!parsed) {
                return std::nullopt;
            }
            const auto root = parsed->to_object();
            if (!root) {
                return std::nullopt;
            }
            const auto* data = lookup(*root, "data");
            if (data == nullptr) {
                return std::nullopt;
            }
            const auto dataObj = data->to_object();
            if (!dataObj) {
                return std::nullopt;
            }
            const auto* setting = lookup(*dataObj, "custom_theme_settings");
            if (setting == nullptr) {
                return std::nullopt;
            }
            auto arr = setting->to_array();
            if (!arr) {
                return std::nullopt;
            }
            return std::optional<serde::Value::Array>{std::move(*arr)};
        } catch (...) {
            return std::nullopt;
        }
    }

    static core::workflow::StepResponse<Output>
    invalid(std::string_view message, std::string_view context, std::string_view property)
    {
        return core::workflow::StepResponse<Output>{
            {{"status", "422"},
             {"body", blog::api::ApiError{}.validation(message, context, property)}}
        };
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::custom_theme_settings::bulk_edit::workflow
