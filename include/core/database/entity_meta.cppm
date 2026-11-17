export module core_database:entity_meta;

import std;
import :field;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db::reflect {

/**
 * @brief Primary metadata trait. It discovers a class's reflected fields via
 * ADL on `core_db_reflect_meta(E*)`, emitted by CORE_DB_REFLECT in whatever
 * namespace the entity lives in. No namespace open/close tricks, no explicit
 * specialization across module boundaries — the macro works anywhere.
 */
template<typename E>
struct EntityMeta
{
private:
    using MetaTuple = decltype(core_db_reflect_meta(static_cast<E*>(nullptr)));

public:
    static constexpr std::string_view table =
        std::get<0>(core_db_reflect_meta(static_cast<E*>(nullptr)));
    using fields = std::tuple_element_t<1, MetaTuple>;
    using entity_type = E;
};

/** @brief True for models described only by serde::Serializable. */
template<typename E>
concept SerdeReflected = serde::ISerializable<E> &&
    !requires { core_db_reflect_meta(static_cast<E*>(nullptr)); };

template<typename T>
consteval ColumnType serde_column_type()
{
    if constexpr (std::same_as<T, std::string>) return ColumnType::TEXT;
    else if constexpr (std::same_as<T, bool>) return ColumnType::BOOLEAN;
    else if constexpr (std::is_integral_v<T>) return ColumnType::BIGINT;
    else if constexpr (std::is_floating_point_v<T>) return ColumnType::DOUBLE;
    else return ColumnType::TEXT;
}

template<typename Descriptor>
consteval bool serde_pk()
{
    if constexpr (requires { Descriptor::options.m_db.m_primary_key; }) {
        return Descriptor::options.m_db.m_primary_key;
    } else {
        return false;
    }
}

/** @brief EntityMeta bridge: DB field descriptors built from serde FieldDesc. */
template<SerdeReflected E>
struct EntityMeta<E>
{
private:
    template<typename... Descriptors>
    static consteval auto make_fields(std::tuple<Descriptors...>)
    {
        return std::tuple<
            SerdeField<
                Descriptors::name,
                Descriptors::getter,
                Descriptors::setter,
                serde_column_type<typename Descriptors::ValueType>(),
                serde_pk<Descriptors>()
            >...
        >{};
    }

public:
    static constexpr std::string_view table = serde::Serializable<E>::table_name();
    using fields = decltype(make_fields(serde::Serializable<E>::fields()));
    using entity_type = E;
};

/** @brief True when `E` was registered with CORE_DB_REFLECT. */
template<typename E>
concept ReflectedEntity = (requires {
    core_db_reflect_meta(static_cast<E*>(nullptr));
} || SerdeReflected<E>) && requires {
    typename EntityMeta<E>::fields;
    { EntityMeta<E>::table } -> std::convertible_to<std::string_view>;
    typename EntityMeta<E>::entity_type;
} && std::same_as<typename EntityMeta<E>::entity_type, E>;

/** @brief Number of reflected fields on `E`. */
template<ReflectedEntity E>
inline constexpr std::size_t field_count_v =
    std::tuple_size_v<typename EntityMeta<E>::fields>;

/** @brief Descriptor for the I-th field of `E`. */
template<ReflectedEntity E, std::size_t I>
using field_at_t = std::tuple_element_t<I, typename EntityMeta<E>::fields>;

/** @brief Member-pointer type stored on a reflected field (cv stripped). */
template<ReflectedEntity E, std::size_t I>
using field_member_t = std::remove_cv_t<decltype(field_at_t<E, I>::member)>;

/** @brief Compile-time check that a member pointer belongs to `E`. */
template<ReflectedEntity E, auto MemberPointer>
inline constexpr bool is_entity_member_v = []<std::size_t... I>(std::index_sequence<I...>) {
    return (... || std::same_as<decltype(MemberPointer), field_member_t<E, I>>);
}(std::make_index_sequence<field_count_v<E>>{});

/** @brief Index of the field named `Name`, or a compile-time error. */
template<ReflectedEntity E, StringLiteral Name>
consteval std::size_t field_index()
{
    constexpr std::size_t COUNT = std::tuple_size_v<typename EntityMeta<E>::fields>;
    std::size_t found = COUNT;
    [&]<std::size_t... I>(std::index_sequence<I...>) {
        ((field_at_t<E, I>::name == Name ? (found = I, 0) : 0), ...);
    }(std::make_index_sequence<COUNT>{});
    if (found == COUNT) {
        throw "CORE_DB: unknown reflected column";
    }
    return found;
}

/** @brief Column name for a member pointer, validated at compile time. */
template<ReflectedEntity E, auto MemberPointer>
consteval std::string_view column_of()
{
    std::string_view result;
    [&]<std::size_t... I>(std::index_sequence<I...>) {
        (([&] {
            if constexpr (std::same_as<decltype(MemberPointer), field_member_t<E, I>>) {
                if (MemberPointer == field_at_t<E, I>::member) {
                    result = std::string_view{field_at_t<E, I>::name};
                }
            }
        }()), ...);
    }(std::make_index_sequence<
      std::tuple_size_v<typename EntityMeta<E>::fields>>{});
    if (result.empty()) {
        throw "CORE_DB: member is not a reflected column";
    }
    return result;
}

} // namespace core::db::reflect

/**
 * @brief Declares a plain struct as a reflected database entity.
 *
 * Usage:
 * @code
 * struct User { std::int64_t id{}; std::string name{}; };
 * CORE_DB_REFLECT(User, "users",
 *     CORE_DB_FIELD("id", &User::id, core::db::ColumnType::BIGINT,
 *                   core::db::FieldFlags::PRIMARY_KEY),
 *     CORE_DB_FIELD("name", &User::name, core::db::ColumnType::TEXT));
 * @endcode
 *
 * Works at any namespace scope: metadata is discovered by ADL on the entity's
 * own type, so no namespace open/close or cross-module specialization needed.
 */
#define CORE_DB_FIELD(Name, MemberPointer, SqlType, ...)                        \
    ::core::db::reflect::Field<Name, MemberPointer, SqlType __VA_OPT__(, ) __VA_ARGS__>

#define CORE_DB_REFLECT(Type, Table, ...)                                        \
    consteval auto core_db_reflect_meta(Type*) {                                 \
        return ::std::make_tuple(                                                \
            ::std::string_view{Table},                                           \
            ::std::tuple<__VA_ARGS__>{}                                          \
        );                                                                       \
    }                                                                            \
    static_assert(                                                               \
        ::core::db::reflect::ReflectedEntity<Type>,                              \
        "CORE_DB_REFLECT: " #Type " failed entity validation"                    \
    )

#ifdef CONGELADO_TEST
namespace core::db::reflect::tests {

struct MetaSample
{
    std::int64_t id{};
    std::string name{};
};
CORE_DB_REFLECT(MetaSample, "samples",
    CORE_DB_FIELD("id", &MetaSample::id, ColumnType::BIGINT, FieldFlags::PRIMARY_KEY),
    CORE_DB_FIELD("name", &MetaSample::name, ColumnType::TEXT));

using namespace boost::ut;

suite<"core_db_entity_meta"> core_db_entity_meta_suite = []
{
    "specialization exposes table, fields and compile-time lookup"_test = []
    {
        static_assert(ReflectedEntity<MetaSample>);
        static_assert(field_count_v<MetaSample> == 2);
        static_assert(is_entity_member_v<MetaSample, &MetaSample::id>);
        static_assert(field_index<MetaSample, "name">() == 1);
        expect(EntityMeta<MetaSample>::table == "samples");
        expect(column_of<MetaSample, &MetaSample::name>() == "name");
    };
};

} // namespace core::db::reflect::tests
#endif
