export module core_cache:entity_codec;

import std;
import :value;
import :entry;
import :key_builder;
import :entity_meta;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cache::reflect {


template<typename T>
class EncodeField
{
public:
    [[nodiscard]] std::string operator()(const T& value) const
    {
        using U = std::remove_cvref_t<T>;
        if constexpr (std::same_as<U, bool>) {
            return value ? "true" : "false";
        } else if constexpr (std::is_arithmetic_v<U>) {
            return std::format("{}", value);
        } else if constexpr (
            std::same_as<U, std::string> || std::convertible_to<U, std::string_view>
        ) {
            return std::string{std::string_view{value}};
        } else {
            return std::format("{}", value);
        }
    }
};

template<typename T>
class DecodeField
{
public:
    [[nodiscard]] std::optional<T> operator()(std::string_view text) const
    {
        using U = std::remove_cvref_t<T>;
        if constexpr (std::same_as<U, std::string>) {
            return std::string{text};
        } else if constexpr (std::same_as<U, bool>) {
            return text == "true" || text == "1";
        } else if constexpr (std::is_integral_v<U>) {
            long long parsed = 0;
            const auto [end, error] =
                std::from_chars(text.data(), text.data() + text.size(), parsed);
            if (error != std::errc{} || end != text.data() + text.size()) {
                return std::nullopt;
            }
            return static_cast<U>(parsed);
        } else if constexpr (std::is_floating_point_v<U>) {
            double parsed = 0.0;
            const auto [end, error] =
                std::from_chars(text.data(), text.data() + text.size(), parsed);
            if (error != std::errc{} || end != text.data() + text.size()) {
                return std::nullopt;
            }
            return static_cast<U>(parsed);
        } else {
            return std::nullopt;
        }
    }
};

/** @brief Encodes/decodes reflected entities into backend-neutral cache entries. */
class EntityCodec
{
public:
    template<ReflectedCacheEntity E>
    [[nodiscard]] CacheValue encode_value(const CacheEntry& entry)
    {
        // The raw value is the encoded key itself; typed round-trips rely on
        // the attributes below, while raw gets still see a meaningful value.
        return to_cache_value(entry.key);
    }

    template<ReflectedCacheEntity E>
    [[nodiscard]] CacheEntry encode(const E& entity)
    {
        CacheEntry entry;
        entry.key = KeyBuilder{}.build<E>(entity);
        [&]<std::size_t... I>(std::index_sequence<I...>)
        {
            auto append = [&](auto descriptor)
            {
                using F = decltype(descriptor);
                const std::string name{std::string_view{F::name}};
                const std::string encoded =
                    EncodeField<std::remove_cvref_t<decltype(entity.*F::member)>>{}(
                        entity.*F::member
                    );
                entry.attributes.emplace(name, encoded);
                if constexpr (F::is_tag) {
                    entry.tags.push_back(encoded);
                }
            };
            (append(field_at_t<E, I>{}), ...);
        }(std::make_index_sequence<field_count_v<E>>{});
        entry.value = encode_value<E>(entry);
        return entry;
    }

    template<ReflectedCacheEntity E>
    [[nodiscard]] std::optional<E> decode(const CacheEntry& entry)
    {
        E entity{};
        bool complete = true;
        [&]<std::size_t... I>(std::index_sequence<I...>)
        {
            auto read = [&](auto descriptor)
            {
                using F = decltype(descriptor);
                const auto it = entry.attributes.find(std::string{std::string_view{F::name}});
                if (it == entry.attributes.end()) {
                    return;
                }
                if (auto parsed = DecodeField<typename F::value_type>{}(it->second)) {
                    entity.*F::member = *parsed;
                } else {
                    complete = false;
                }
            };
            (read(field_at_t<E, I>{}), ...);
        }(std::make_index_sequence<field_count_v<E>>{});
        if (!complete) {
            return std::nullopt;
        }
        return entity;
    }
};

} // namespace core::cache::reflect

#ifdef CONGELADO_TEST
namespace core::cache::reflect::tests {

struct CodecSample
{
    std::int64_t id{};
    std::string name{};
    bool active{};
};

CORE_CACHE_ENTITY(
    CodecSample,
    "users",
    CORE_CACHE_FIELD("id", &CodecSample::id, FieldRole::KEY),
    CORE_CACHE_FIELD("name", &CodecSample::name, FieldRole::TAG),
    CORE_CACHE_FIELD("active", &CodecSample::active)
);

using namespace boost::ut;

suite<"core_cache_entity_codec"> core_cache_entity_codec_suite = []
{
    "round trips an entity through a cache entry"_test = []
    {
        const CodecSample original{7, "Ada", true};
        const CacheEntry entry = EntityCodec{}.encode(original);
        expect(entry.key == "users:v1:7");
        expect(entry.attributes.at("name") == "Ada");
        expect(entry.has_tag("Ada"));
        const auto decoded = EntityCodec{}.decode<CodecSample>(entry);
        expect(decoded.has_value());
        expect(decoded->id == 7);
        expect(decoded->name == "Ada");
        expect(decoded->active);
    };
};

} // namespace core::cache::reflect::tests
#endif
