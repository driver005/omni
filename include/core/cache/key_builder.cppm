export module core_cache:key_builder;

import std;
import :cache_value;
import :cache_entity_meta;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cache::reflect {

/**
 * @brief Builds cache keys from reflected KEY members at compile time.
 * Format: "prefix:v1:<role-value>:..." e.g. "users:v1:7".
 */
class KeyBuilder
{
public:
    template<ReflectedCacheEntity E, typename Entity>
    [[nodiscard]] static std::string build(const Entity& entity)
    {
        std::string out{std::string{CacheEntityMeta<E>::prefix} + ":v1"};
        append_keys<E>(entity, out);
        return out;
    }

    /** @brief Fully qualified key for a role value (e.g. "users:v1:7"). */
    template<typename Value>
    [[nodiscard]] static std::string composite(
        std::string_view prefix,
        std::string_view role,
        const Value& value
    )
    {
        return std::string{prefix} + ":v1:" + std::string{role} + ":" +
               key_part(to_cache_value(value));
    }

private:
    template<ReflectedCacheEntity E, typename Entity>
    static void append_keys(const Entity& entity, std::string& out)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            auto append = [&](auto descriptor) {
                using F = decltype(descriptor);
                if constexpr (F::is_key) {
                    out += ":" + key_part(to_cache_value(entity.*F::member));
                }
            };
            (append(field_at_t<E, I>{}), ...);
        }(std::make_index_sequence<field_count_v<E>>{});
    }

    [[nodiscard]] static std::string key_part(const CacheValue& value)
    {
        return std::visit(
            [](const auto& v) -> std::string {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::same_as<T, Miss>) {
                    return "miss";
                } else if constexpr (std::same_as<T, std::vector<std::uint8_t>>) {
                    return "blob";
                } else if constexpr (std::same_as<T, std::vector<std::string>>) {
                    return "list";
                } else {
                    return std::format("{}", v);
                }
            },
            value
        );
    }
};

} // namespace core::cache::reflect

#ifdef CONGELADO_TEST
namespace core::cache::reflect::tests {

struct KeySample
{
    std::int64_t id{};
    std::string name{};
};
CORE_CACHE_ENTITY(KeySample, "users",
    CORE_CACHE_FIELD("id", &KeySample::id, FieldRole::KEY),
    CORE_CACHE_FIELD("name", &KeySample::name, FieldRole::TAG));

using namespace boost::ut;

suite<"core_cache_key_builder"> core_cache_key_builder_suite = []
{
    "builds keys from reflected KEY members"_test = []
    {
        const KeySample entity{7, "Ada"};
        expect(KeyBuilder::build<KeySample>(entity) == "users:v1:7");
        expect(KeyBuilder::composite("users", "id", std::int64_t{7}) == "users:v1:id:7");
    };
};

} // namespace core::cache::reflect::tests
#endif
