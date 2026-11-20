#pragma once

// C++20 modules do not export macros. Include this header in the global module fragment
// of every app model partition, after importing core_database.

#define APP_FIELD(Name, MemberPointer, SqlType, ...)                              \
    ::core::db::reflect::Field<Name, MemberPointer, SqlType __VA_OPT__(, ) __VA_ARGS__>

#define APP_ENTITY(Type, Table, ...)                                              \
    consteval auto core_db_reflect_meta(Type*) {                                  \
        return ::std::make_tuple(                                                 \
            ::std::string_view{Table},                                            \
            ::std::tuple<__VA_ARGS__>{}                                           \
        );                                                                        \
    }                                                                             \
    static_assert(                                                                \
        ::core::db::reflect::ReflectedEntity<Type>,                               \
        "APP_ENTITY: " #Type " failed entity validation"                          \
    )
