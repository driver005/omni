export module blog_handlers:tiers_edit_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_serde;
import :outbox_shim;
import interfaces;

export namespace blog::handlers::tiers::edit::workflow {

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
    static constexpr std::string_view name{"tiers_edit_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        const auto q = core::query::parse(in.get_path(), kOptions);
        if (q.options.contains("id") && q.options.at("id").empty()) {
            return invalid("id", "id is required");
        }
        for (const auto& inc: q.include) {
            if (inc != "requirements") {
                return invalid("include", "Invalid include value");
            }
        }

        const std::string path_text{in.get_path()};
        const std::string_view path{path_text};
        const auto id = q.options.contains("id") && !q.options.at("id").empty() ? q.options.at("id")
                                                                                : path_tail(path);
        if (id.empty()) {
            return invalid("id", "id is required");
        }
        // TiersAPI.edit converts through ObjectID.createFromHexString (tiers-api.js:113).
        if (!is_object_id(id)) {
            return failure("400", "Invalid ObjectID");
        }
        if (!ctx.store().find_by_resource("tiers", id)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body", blog::api::ApiError{}.json("Resource not found", "Not Found")}}
            };
        }

        const auto raw = std::string{in.body_as_string()};
        if (raw.empty()) {
            return invalid("tiers", "Invalid tier payload");
        }
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(raw);
        if (!parsed) {
            return invalid("tiers", "Invalid tier payload");
        }
        const auto inner = convert(unwrap(*parsed));
        if (!inner) {
            return invalid("tiers", "Invalid tier payload");
        }
        if (const auto error = validate_domain(*inner); !error.empty()) {
            return invalid("tiers", error);
        }

        // TiersAPI.editableProperties + updatePricing (tiers-api.js:116-136).
        Object store;
        store["id"] = serde::Value{id};
        if (const auto* name = find(*inner, "name")) {
            store["name"] = *name;
        }
        if (const auto* slug = find(*inner, "slug")) {
            store["slug"] = *slug;
        }
        if (const auto* price = find(*inner, "monthlyPrice")) {
            store["price"] = *price;
        }

        const auto result =
            ctx.store().edit("tiers", id, core_serde::JsonCodec{}.encode(serde::Value{store}));
        const int status = result.status() == 201 ? 200 : result.status();
        const auto body = local_mapping::wrap("tiers", clean_collection(result.body()));

        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        for (const auto& [field, type]: kEvents) {
            if (find(*inner, field) != nullptr) {
                outbox.emit(
                    blog::handlers::outbox::Event{
                        .type = std::string{type},
                        .payload = core_serde::JsonCodec{}.encode(serde::Value{*inner}),
                        .aggregate_id = id
                    }
                );
            }
        }
        outbox.commit();

        return core::workflow::StepResponse<Output>{
            {{"status", std::to_string(status)}, {"body", body}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;
    static constexpr std::string_view kOptions[]{"id", "include"};
    static constexpr std::pair<std::string_view, std::string_view> kEvents[]{
        {"name", "TierNameChangeEvent"},
        {"monthlyPrice", "TierPriceChangeEvent"},
        {"status", "TierStatusChangeEvent"},
        {"active", "TierStatusChangeEvent"}
    };

    static const serde::Value* find(const Object& object, std::string_view key)
    {
        for (const auto& [k, v]: object) {
            if (k == key) {
                return &v;
            }
        }
        return nullptr;
    }

    [[nodiscard]] static std::string clean_collection(std::string_view body)
    {
        auto value = core_serde::JsonCodec{}.decode<serde::Value>(body);
        if (!value) {
            return "[]";
        }
        const auto* root = std::get_if<Object>(&value->get());
        if (root == nullptr) {
            return "[]";
        }
        for (const auto& [key, val]: *root) {
            if (key == "tiers") {
                return local_mapping::clean(core_serde::JsonCodec{}.encode(val), "tier");
            }
        }
        return "[]";
    }

    static std::string string_field(const Object& object, std::string_view key)
    {
        const auto* value = find(object, key);
        if (value == nullptr) {
            return {};
        }
        const auto* text = std::get_if<std::string>(&value->get());
        return text == nullptr ? std::string{} : *text;
    }

    static const serde::Value* unwrap(const serde::Value& parsed)
    {
        const auto* root = std::get_if<Object>(&parsed.get());
        if (root == nullptr) {
            return nullptr;
        }
        const auto* tiers = find(*root, "products");
        if (tiers == nullptr) {
            tiers = find(*root, "tiers");
        }
        if (tiers == nullptr) {
            const auto* data = find(*root, "data");
            if (data != nullptr) {
                const auto* data_object = std::get_if<Object>(&data->get());
                if (data_object != nullptr) {
                    tiers = find(*data_object, "tiers");
                }
            }
        }
        if (tiers == nullptr) {
            return nullptr;
        }
        const auto* array = std::get_if<serde::Value::Array>(&tiers->get());
        if (array == nullptr || array->empty()) {
            return nullptr;
        }
        return &array->front();
    }

    static std::optional<Object> convert(const serde::Value* source)
    {
        if (source == nullptr) {
            return std::nullopt;
        }
        const auto* object = std::get_if<Object>(&source->get());
        if (object == nullptr) {
            return std::nullopt;
        }
        static constexpr std::pair<std::string_view, std::string_view> kRenames[]{
            {"active", "status"},
            {"welcome_page_url", "welcomePageURL"},
            {"trial_days", "trialDays"},
            {"monthly_price", "monthlyPrice"},
            {"yearly_price", "yearlyPrice"}
        };
        Object out;
        for (const auto& [key, val]: *object) {
            bool renamed = false;
            for (const auto& [from, to]: kRenames) {
                if (key == from) {
                    out[std::string{to}] = val;
                    renamed = true;
                    break;
                }
            }
            if (!renamed) {
                out[key] = val;
            }
        }
        return out;
    }

    [[nodiscard]] static std::string validate_domain(const Object& in)
    {
        const auto description = string_field(in, "description");
        if (description.size() > 191) {
            return "Tier description must be a string with a maximum of 191 characters";
        }
        const auto status = string_field(in, "status");
        if (!status.empty() && status != "active" && status != "archived") {
            return "Tier status must be either \"active\" or \"archived\"";
        }
        const auto visibility = string_field(in, "visibility");
        if (!visibility.empty() && visibility != "public" && visibility != "none") {
            return "Tier visibility must be either \"public\" or \"none\"";
        }
        const auto currency = string_field(in, "currency");
        if (!currency.empty() && currency.size() != 3) {
            return "Tier currency must be a 3 letter ISO currency code";
        }
        return {};
    }

    [[nodiscard]] static bool is_object_id(std::string_view id) noexcept
    {
        if (id.size() != 24) {
            return false;
        }
        for (char c: id) {
            const bool digit = c >= '0' && c <= '9';
            const bool hex = (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
            if (!digit && !hex) {
                return false;
            }
        }
        return true;
    }

    static std::string path_tail(std::string_view path)
    {
        auto bare = path.substr(0, path.find('?'));
        while (bare.size() > 1 && bare.back() == '/') {
            bare.remove_suffix(1);
        }
        const auto slash = bare.find_last_of('/');
        return std::string{slash == std::string_view::npos ? bare : bare.substr(slash + 1)};
    }

    static core::workflow::StepResponse<Output>
    invalid(std::string_view property, std::string_view context)
    {
        return core::workflow::StepResponse<Output>{
            {{"status", "422"},
             {"body", blog::api::ApiError{}.validation("Validation error", context, property)}}
        };
    }

    static core::workflow::StepResponse<Output>
    failure(std::string_view code, std::string_view message)
    {
        return core::workflow::StepResponse<Output>{
            {{"status", std::string{code}},
             {"body", blog::api::ApiError{}.json(message, "Bad Request")}}
        };
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::tiers::edit::workflow
