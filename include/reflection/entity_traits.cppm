export module reflection.entity_traits;

import std;
import database.types;

export namespace reflection {

template<size_t N>
struct StringLiteral {
    constexpr StringLiteral(const char (&str)[N]) {
        for (size_t i = 0; i < N; ++i) value[i] = str[i];
    }
    char value[N];
    static constexpr size_t size = N - 1;
    
    constexpr operator std::string_view() const { return {value, size}; }
};

template<StringLiteral Name, database::ColumnType Type, bool PrimaryKey = false, bool Nullable = true, bool Unique = false>
struct Column {
    static constexpr std::string_view name = Name;
    static constexpr database::ColumnType type = Type;
    static constexpr bool primary_key = PrimaryKey;
    static constexpr bool nullable = Nullable;
    static constexpr bool unique = Unique;
    static constexpr std::string_view default_value = "";
    static constexpr std::string_view check_constraint = "";
    static constexpr std::string_view foreign_table = "";
    static constexpr std::string_view foreign_column = "";
};

template<StringLiteral Name, database::ColumnType Type, bool PrimaryKey, bool Nullable, bool Unique, StringLiteral Default>
struct ColumnWithDefault {
    static constexpr std::string_view name = Name;
    static constexpr database::ColumnType type = Type;
    static constexpr bool primary_key = PrimaryKey;
    static constexpr bool nullable = Nullable;
    static constexpr bool unique = Unique;
    static constexpr std::string_view default_value = Default;
    static constexpr std::string_view check_constraint = "";
    static constexpr std::string_view foreign_table = "";
    static constexpr std::string_view foreign_column = "";
};

template<StringLiteral Name, database::ColumnType Type, bool PrimaryKey, bool Nullable, bool Unique, StringLiteral ForeignTable, StringLiteral ForeignColumn>
struct ColumnWithFK {
    static constexpr std::string_view name = Name;
    static constexpr database::ColumnType type = Type;
    static constexpr bool primary_key = PrimaryKey;
    static constexpr bool nullable = Nullable;
    static constexpr bool unique = Unique;
    static constexpr std::string_view default_value = "";
    static constexpr std::string_view check_constraint = "";
    static constexpr std::string_view foreign_table = ForeignTable;
    static constexpr std::string_view foreign_column = ForeignColumn;
};

template<StringLiteral Name, StringLiteral... Columns>
struct Index {
    static constexpr std::string_view name = Name;
    static constexpr std::array<std::string_view, sizeof...(Columns)> columns = {Columns...};
    static constexpr bool unique = false;
    static constexpr std::string_view method = "btree";
    static constexpr std::string_view where_clause = "";
};

template<StringLiteral Name, StringLiteral... Columns>
struct UniqueIndex {
    static constexpr std::string_view name = Name;
    static constexpr std::array<std::string_view, sizeof...(Columns)> columns = {Columns...};
    static constexpr bool unique = true;
    static constexpr std::string_view method = "btree";
    static constexpr std::string_view where_clause = "";
};

template<StringLiteral Name, StringLiteral Expression>
struct Check {
    static constexpr std::string_view name = Name;
    static constexpr std::string_view expression = Expression;
};

template<StringLiteral Name, StringLiteral Schema = "public", typename... Columns>
struct Table {
    static constexpr std::string_view name = Name;
    static constexpr std::string_view schema = Schema;
    using columns_tuple = std::tuple<Columns...>;
    static constexpr size_t column_count = sizeof...(Columns);
};

struct EntityBase {
    virtual ~EntityBase() = default;
    virtual const char* entity_name() const = 0;
    virtual const char* table_name() const = 0;
};

template<typename Entity>
struct EntityTraits {
    static_assert(false, "EntityTraits not specialized for this type. Use MEDUSA_ENTITY macro.");
};

template<typename Tuple, size_t... I>
constexpr std::array<database::ColumnDef, sizeof...(I)> 
make_columns_impl(const Tuple&, std::index_sequence<I...>) {
    return {database::ColumnDef{
        std::string(std::tuple_element_t<I, Tuple>::name),
        std::tuple_element_t<I, Tuple>::type,
        std::tuple_element_t<I, Tuple>::primary_key,
        std::tuple_element_t<I, Tuple>::nullable,
        std::tuple_element_t<I, Tuple>::unique,
        std::tuple_element_t<I, Tuple>::default_value.empty() ? std::nullopt : std::optional<std::string>(std::string(std::tuple_element_t<I, Tuple>::default_value)),
        std::tuple_element_t<I, Tuple>::check_constraint.empty() ? std::nullopt : std::optional<std::string>(std::string(std::tuple_element_t<I, Tuple>::check_constraint)),
        (!std::tuple_element_t<I, Tuple>::foreign_table.empty() && !std::tuple_element_t<I, Tuple>::foreign_column.empty()) 
            ? std::optional<std::pair<std::string, std::string>>({std::string(std::tuple_element_t<I, Tuple>::foreign_table), std::string(std::tuple_element_t<I, Tuple>::foreign_column)})
            : std::nullopt,
        ""
    }...};
}

template<typename Entity>
constexpr database::TableDef generate_table_def() {
    using Traits = EntityTraits<Entity>;
    using ColumnsTuple = typename Traits::table_type::columns_tuple;
    
    constexpr auto columns = make_columns_impl(ColumnsTuple{}, std::make_index_sequence<std::tuple_size_v<ColumnsTuple>>{});
    
    database::TableDef def;
    def.name = std::string(Traits::table_type::name);
    def.schema = std::string(Traits::table_type::schema);
    def.columns.assign(columns.begin(), columns.end());
    
    for (const auto& col : def.columns) {
        if (col.primary_key) def.primary_keys.push_back(col.name);
    }
    
    return def;
}

template<typename Entity>
constexpr std::string generate_create_sql() {
    return generate_table_def<Entity>().create_sql();
}

template<typename Entity>
constexpr std::string generate_drop_sql() {
    return generate_table_def<Entity>().drop_sql();
}

template<typename T>
struct TypeToColumnType;

template<> struct TypeToColumnType<int64_t> { static constexpr database::ColumnType value = database::ColumnType::BIGINT; };
template<> struct TypeToColumnType<int32_t> { static constexpr database::ColumnType value = database::ColumnType::INTEGER; };
template<> struct TypeToColumnType<int16_t> { static constexpr database::ColumnType value = database::ColumnType::SMALLINT; };
template<> struct TypeToColumnType<double> { static constexpr database::ColumnType value = database::ColumnType::DOUBLE_PRECISION; };
template<> struct TypeToColumnType<float> { static constexpr database::ColumnType value = database::ColumnType::REAL; };
template<> struct TypeToColumnType<bool> { static constexpr database::ColumnType value = database::ColumnType::BOOLEAN; };
template<> struct TypeToColumnType<std::string> { static constexpr database::ColumnType value = database::ColumnType::TEXT; };
template<> struct TypeToColumnType<std::vector<uint8_t>> { static constexpr database::ColumnType value = database::ColumnType::BYTEA; };
template<> struct TypeToColumnType<std::chrono::system_clock::time_point> { static constexpr database::ColumnType value = database::ColumnType::TIMESTAMPTZ; };

struct FieldDescriptor {
    std::string name;
    database::ColumnType type;
    bool primary_key;
    bool nullable;
    bool unique;
    std::optional<std::string> default_value;
    std::optional<std::string> check_constraint;
    std::optional<std::pair<std::string, std::string>> foreign_key;
    size_t offset;
    size_t size;
    const std::type_info* cpp_type;
};

template<typename Entity>
struct EntityMetadata {
    static const std::string name;
    static const std::string table_name;
    static const std::string schema;
    static const std::vector<FieldDescriptor> fields;
    static const std::vector<database::IndexDef> indexes;
    static const std::vector<database::CheckConstraint> checks;
    static const std::string create_sql;
    static const std::string drop_sql;
};

} // namespace reflection

#define MEDUSA_ENTITY(EntityName, TableName, SchemaName, ...) \
    struct EntityName : reflection::EntityBase { \
        __VA_ARGS__ \
        const char* entity_name() const override { return #EntityName; } \
        const char* table_name() const override { return #TableName; } \
        using TableMeta = reflection::Table<#TableName, #SchemaName, __VA_ARGS__>; \
    }; \
    namespace reflection { \
        template<> \
        struct EntityTraits<EntityName> { \
            using table_type = typename EntityName::TableMeta; \
        }; \
    }

#define MEDUSA_COLUMN(Name, Type) \
    reflection::Column<#Name, database::ColumnType::Type>

#define MEDUSA_PK(Name, Type) \
    reflection::Column<#Name, database::ColumnType::Type, true, false, false>

#define MEDUSA_COLUMN_NULLABLE(Name, Type) \
    reflection::Column<#Name, database::ColumnType::Type, false, true, false>

#define MEDUSA_COLUMN_UNIQUE(Name, Type) \
    reflection::Column<#Name, database::ColumnType::Type, false, false, true>

#define MEDUSA_COLUMN_DEFAULT(Name, Type, Default) \
    reflection::ColumnWithDefault<#Name, database::ColumnType::Type, false, true, false, #Default>

#define MEDUSA_FK(Name, Type, ForeignTable, ForeignColumn) \
    reflection::ColumnWithFK<#Name, database::ColumnType::Type, false, true, false, #ForeignTable, #ForeignColumn>

#define MEDUSA_PK_FK(Name, Type, ForeignTable, ForeignColumn) \
    reflection::ColumnWithFK<#Name, database::ColumnType::Type, true, false, false, #ForeignTable, #ForeignColumn>

#define MEDUSA_INDEX(Name, ...) \
    reflection::Index<#Name, __VA_ARGS__>

#define MEDUSA_UNIQUE_INDEX(Name, ...) \
    reflection::UniqueIndex<#Name, __VA_ARGS__>

#define MEDUSA_CHECK(Name, Expression) \
    reflection::Check<#Name, #Expression>