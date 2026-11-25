export module blog_handlers:tiers_create_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_serde;
import :outbox_shim;
import interfaces;

export namespace blog::handlers::tiers::create::workflow {

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
    static constexpr std::string_view name{"tiers_create_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        // options=['include'], validation include∈['requirements'] (tiers.js:48-54).
        const auto q = core::query::parse(in.get_path(), kOptions);
        for (const auto& inc: q.include) {
            if (inc != "requirements") {
                return invalid("include", "Invalid include value");
            }
        }

        const auto raw = std::string{in.body_as_string()};
        if (raw.empty()) {
            return invalid("name", "name is required");
        }
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(raw);
        if (!parsed) {
            return invalid("tiers", "Invalid tier payload");
        }

        // input serializer add() (input/tiers.js:113-119): products[0] wins over
        // tiers[0]; convertTierInput renames the snake_case wire fields.
        const auto inner = convert(unwrap(*parsed));
        if (!inner) {
            return invalid("tiers", "Invalid tier payload");
        }

        const std::string name = string_field(*inner, "name");
        if (name.empty() || name.size() > 191) {
            return invalid("name", "Tier name must be a string with a maximum of 191 characters");
        }
        const std::string type = string_field(*inner, "type");
        if (!type.empty() && type != "paid" && type != "free") {
            return invalid("type", "Tier type must be either \"paid\" or \"free\"");
        }

        // TiersAPI.add: free tiers cannot be created (tiers-api.js:148-153).
        if (type == "free") {
            return failure("400", "Cannot create free Tier");
        }

        if (const auto error = validate_domain(*inner); !error.empty()) {
            return invalid("tiers", error);
        }

        Object store;
        store["name"] = serde::Value{name};
        store["slug"] = serde::Value{slug_for(*inner, name)};
        store["price"] = serde::Value{price_for(*inner)};

        const auto result =
            ctx.store().create("tiers", core_serde::JsonCodec{}.encode(serde::Value{store}));
        const auto body = local_mapping::wrap("tiers", clean_collection(result.body()));

        // Domain tier events are dispatched through the repository (tier-repository.js:184-186).
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        outbox.emit(
            blog::handlers::outbox::Event{
                .type = "TierCreatedEvent",
                .payload = core_serde::JsonCodec{}.encode(serde::Value{store}),
                .aggregate_id = std::string{name}
            }
        );
        outbox.emit(
            blog::handlers::outbox::Event{
                .type = "TierPriceChangeEvent",
                .payload = core_serde::JsonCodec{}.encode(serde::Value{store}),
                .aggregate_id = std::string{name}
            }
        );
        outbox.commit();

        return core::workflow::StepResponse<Output>{
            {{"status", std::to_string(result.status())}, {"body", body}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;
    static constexpr std::string_view kOptions[]{"include"};

    static const serde::Value* find(const Object& object, std::string_view key)
    {
        for (const auto& [k, v]: object) {
            if (k == key) {
                return &v;
            }
        }
        return nullptr;
    }

    /** @brief Apply the Ghost tier mapper to the tiers array of a store envelope. */
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
        const auto* tiers = find(*root, "tiers");
        if (tiers == nullptr) {
            return "[]";
        }
        const auto* array = std::get_if<serde::Value::Array>(&tiers->get());
        if (array == nullptr) {
            return "[]";
        }
        return local_mapping::clean(core_serde::JsonCodec{}.encode(*array), "tier");
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

    static std::int64_t int_field(const Object& object, std::string_view key)
    {
        const auto* value = find(object, key);
        if (value == nullptr) {
            return 0;
        }
        if (const auto* n = std::get_if<std::int64_t>(&value->get())) {
            return *n;
        }
        if (const auto* d = std::get_if<double>(&value->get())) {
            return static_cast<std::int64_t>(*d);
        }
        return 0;
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

    /** @brief convertTierInput (input/tiers.js:29-58): rename wire fields to the
     *  domain names and drop the originals. */
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

    /** @brief Tier.create validators (tier.js:310-527) at the Wire-shape level. */
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
        if (const auto* trial = find(in, "trialDays"); trial != nullptr) {
            const auto* days = std::get_if<std::int64_t>(&trial->get());
            if (days == nullptr) {
                const auto* number = std::get_if<double>(&trial->get());
                if (number == nullptr ||
                    *number != static_cast<double>(static_cast<std::int64_t>(*number))) {
                    return "Tier trials must be an integer greater than 0";
                }
            }
            if (int_field(in, "trialDays") < 0) {
                return "Tier trials must be an integer greater than 0";
            }
        }
        const auto monthly = int_field(in, "monthlyPrice");
        const auto yearly = int_field(in, "yearlyPrice");
        if (monthly < 0 || yearly < 0) {
            return "Tier prices must not be negative";
        }
        if (monthly > 9'999'999'999LL || yearly > 9'999'999'999LL) {
            return "Tier prices may not exceed 999999.99";
        }
        const auto* benefits = find(in, "benefits");
        if (benefits != nullptr && std::get_if<serde::Value::Array>(&benefits->get()) == nullptr) {
            return "Tier benefits must be an array of strings";
        }
        return {};
    }

    [[nodiscard]] static std::string slug_for(const Object& in, std::string_view name)
    {
        const auto slug = string_field(in, "slug");
        if (!slug.empty()) {
            return slug;
        }
        std::string out;
        bool dash = false;
        for (char c: name) {
            const unsigned char u = static_cast<unsigned char>(c);
            if (std::isalnum(u) != 0) {
                if (dash && !out.empty()) {
                    out += '-';
                }
                dash = false;
                out += static_cast<char>(std::tolower(u));
            } else {
                dash = true;
            }
        }
        while (!out.empty() && out.back() == '-') {
            out.pop_back();
        }
        return out;
    }

    [[nodiscard]] static std::int64_t price_for(const Object& in)
    {
        const auto* value = find(in, "monthlyPrice");
        if (value == nullptr) {
            return 500;
        }
        if (const auto* n = std::get_if<std::int64_t>(&value->get())) {
            return *n;
        }
        if (const auto* d = std::get_if<double>(&value->get())) {
            return static_cast<std::int64_t>(*d);
        }
        return 500;
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
} // namespace blog::handlers::tiers::create::workflow
