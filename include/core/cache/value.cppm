export module core_cache:value;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cache {

/** @brief Tag stored in a CacheValue to represent a lookup miss. */
struct Miss
{
    constexpr bool operator==(const Miss&) const noexcept = default;
};

/** @brief The only value type crossing the abstraction boundary. */
using CacheValue = std::variant<
    Miss,
    std::int64_t,
    std::uint64_t,
    double,
    bool,
    std::string,
    std::vector<std::uint8_t>,
    std::vector<std::string>>;

/** @brief Normalizes a C++ value into a CacheValue. */
template<typename T>
[[nodiscard]] CacheValue to_cache_value(T&& value)
{
    using U = std::remove_cvref_t<T>;
    if constexpr (std::same_as<U, Miss>) {
        return CacheValue{Miss{}};
    } else if constexpr (std::same_as<U, bool>) {
        return CacheValue{value};
    } else if constexpr (std::integral<U> && std::is_signed_v<U>) {
        return CacheValue{static_cast<std::int64_t>(value)};
    } else if constexpr (std::integral<U>) {
        return CacheValue{static_cast<std::uint64_t>(value)};
    } else if constexpr (std::floating_point<U>) {
        return CacheValue{static_cast<double>(value)};
    } else if constexpr (std::same_as<U, std::string> ||
                         std::convertible_to<U, std::string_view>) {
        return CacheValue{std::string{std::string_view{value}}};
    } else {
        return CacheValue{std::forward<T>(value)};
    }
}

/** @brief Extracts a concrete value from a CacheValue, or a fallback on miss/mismatch. */
template<typename T>
[[nodiscard]] T value_as(const CacheValue& value, T fallback = T{})
{
    if constexpr (std::same_as<T, bool>) {
        if (const auto* p = std::get_if<bool>(&value)) {
            return *p;
        }
    } else if constexpr (std::integral<T>) {
        if (const auto* p = std::get_if<std::int64_t>(&value)) {
            return static_cast<T>(*p);
        }
        if (const auto* p = std::get_if<std::uint64_t>(&value)) {
            return static_cast<T>(*p);
        }
    } else if constexpr (std::floating_point<T>) {
        if (const auto* p = std::get_if<double>(&value)) {
            return static_cast<T>(*p);
        }
    } else if constexpr (std::same_as<T, std::string>) {
        if (const auto* p = std::get_if<std::string>(&value)) {
            return *p;
        }
    }
    return fallback;
}

/** @brief Instance functor detecting a cache miss value. */
class IsMiss
{
public:
    [[nodiscard]] bool operator()(const CacheValue& value) const noexcept
    {
        return std::holds_alternative<Miss>(value);
    }
};

} // namespace core::cache

#ifdef CONGELADO_TEST
namespace core::cache::tests {
using namespace boost::ut;

suite<"core_cache_value"> core_cache_value_suite = []
{
    "normalizes arithmetic and strings"_test = []
    {
        expect(std::get<std::int64_t>(to_cache_value(7)) == 7);
        expect(std::get<double>(to_cache_value(2.5f)) == 2.5);
        expect(std::get<std::string>(to_cache_value(std::string{"ada"})) == "ada");
    };

    "value_as converts and falls back on miss"_test = []
    {
        const CacheValue v = to_cache_value(std::int64_t{42});
        expect(value_as<std::int64_t>(v) == 42);
        expect(value_as<std::string>(v, std::string{"fallback"}) == "fallback");
        expect(IsMiss{}(CacheValue{Miss{}}));
        expect(!IsMiss{}(v));
    };
};

} // namespace core::cache::tests
#endif
