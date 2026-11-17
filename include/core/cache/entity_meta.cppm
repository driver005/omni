export module core_cache:entity_meta;

import std;
import :field;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cache::reflect {

/**
 * @brief Metadata trait discovered by ADL on `core_cache_reflect_meta(E*)`,
 * emitted by CORE_CACHE_REFLECT in the entity's own namespace. Works anywhere,
 * no cross-module specialization needed.
 */
template<typename E>
struct CacheEntityMeta
{
private:
    using MetaTuple = decltype(core_cache_reflect_meta(static_cast<E*>(nullptr)));

public:
    static constexpr std::string_view prefix =
        std::get<0>(core_cache_reflect_meta(static_cast<E*>(nullptr)));
    using fields = std::tuple_element_t<1, MetaTuple>;
    using entity_type = E;
};

template<typename E>
concept ReflectedCacheEntity = requires {
    core_cache_reflect_meta(static_cast<E*>(nullptr));
} && requires {
    typename CacheEntityMeta<E>::fields;
    { CacheEntityMeta<E>::prefix } -> std::convertible_to<std::string_view>;
    typename CacheEntityMeta<E>::entity_type;
} && std::same_as<typename CacheEntityMeta<E>::entity_type, E>;

template<ReflectedCacheEntity E>
inline constexpr std::size_t field_count_v =
    std::tuple_size_v<typename CacheEntityMeta<E>::fields>;

template<ReflectedCacheEntity E, std::size_t I>
using field_at_t = std::tuple_element_t<I, typename CacheEntityMeta<E>::fields>;

/** @brief Member pointer type stored on a reflected field (cv stripped). */
template<ReflectedCacheEntity E, std::size_t I>
using field_member_t = std::remove_cv_t<decltype(field_at_t<E, I>::member)>;

template<ReflectedCacheEntity E, auto MemberPointer>
inline constexpr bool is_entity_member_v = []<std::size_t... I>(std::index_sequence<I...>) {
    return (... || std::same_as<decltype(MemberPointer), field_member_t<E, I>>);
}(std::make_index_sequence<field_count_v<E>>{});

/** @brief Field name for a member pointer, validated at compile time. */
template<ReflectedCacheEntity E, auto MemberPointer>
consteval std::string_view name_of()
{
    std::string_view result;
    [&]<std::size_t... I>(std::index_sequence<I...>) {
        ((std::same_as<decltype(MemberPointer), field_member_t<E, I>>
              ? (result = std::string_view{field_at_t<E, I>::name}, 0)
              : 0),
         ...);
    }(std::make_index_sequence<
      std::tuple_size_v<typename CacheEntityMeta<E>::fields>>{});
    if (result.empty()) {
        throw "CORE_CACHE: member is not a reflected field";
    }
    return result;
}

} // namespace core::cache::reflect

/** @brief Declares a plain struct as a reflected cache entity. */
#define CORE_CACHE_FIELD(Name, MemberPointer, ...)                              \
    ::core::cache::reflect::CacheField<                                         \
        Name, MemberPointer __VA_OPT__(, ) __VA_ARGS__>

#define CORE_CACHE_ENTITY(Type, Prefix, ...)                                     \
    consteval auto core_cache_reflect_meta(Type*) {                              \
        return ::std::make_tuple(                                                \
            ::std::string_view{Prefix},                                          \
            ::std::tuple<__VA_ARGS__>{}                                          \
        );                                                                       \
    }                                                                            \
    static_assert(                                                               \
        ::core::cache::reflect::ReflectedCacheEntity<Type>,                      \
        "CORE_CACHE_ENTITY: " #Type " failed entity validation"                  \
    )

#ifdef CONGELADO_TEST
namespace core::cache::reflect::tests {

struct MetaSample
{
    std::int64_t id{};
    std::string name{};
};
CORE_CACHE_ENTITY(MetaSample, "samples",
    CORE_CACHE_FIELD("id", &MetaSample::id, FieldRole::KEY),
    CORE_CACHE_FIELD("name", &MetaSample::name, FieldRole::TAG));

using namespace boost::ut;

suite<"core_cache_cache_entity_meta"> core_cache_cache_entity_meta_suite = []
{
    "ADL metadata exposes prefix, fields and lookup"_test = []
    {
        static_assert(ReflectedCacheEntity<MetaSample>);
        static_assert(field_count_v<MetaSample> == 2);
        static_assert(is_entity_member_v<MetaSample, &MetaSample::id>);
        static_assert(field_at_t<MetaSample, 0>::is_key);
        static_assert(field_at_t<MetaSample, 1>::is_tag);
        expect(CacheEntityMeta<MetaSample>::prefix == "samples");
        expect(name_of<MetaSample, &MetaSample::name>() == "name");
    };
};

} // namespace core::cache::reflect::tests
#endif
