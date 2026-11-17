export module core_database:entity_mapper;

import std;
import :row;
import :entity_meta;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db::reflect {

/** @brief Converts between reflected entities and backend-neutral rows/values. */
class EntityMapper
{
public:
    /** @brief Builds an entity from a row using reflected member pointers. */
    template<ReflectedEntity E>
    [[nodiscard]] E from_row(const Row& row)
    {
        E entity{};
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            auto assign = [&](auto descriptor) {
                using F = decltype(descriptor);
                if (row.contains(std::string_view{F::name})) {
                    entity.*F::member =
                        row.get<typename F::value_type>(std::string_view{F::name});
                }
            };
            (assign(field_at_t<E, I>{}), ...);
        }(std::make_index_sequence<field_count_v<E>>{});
        return entity;
    }

    /** @brief Flattens an entity into named DbValues via reflected members. */
    template<ReflectedEntity E>
    [[nodiscard]] std::map<std::string, DbValue> to_values(const E& entity)
    {
        std::map<std::string, DbValue> values;
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            auto append = [&](auto descriptor) {
                using F = decltype(descriptor);
                values.emplace(
                    std::string{std::string_view{F::name}},
                    to_db_value(entity.*F::member)
                );
            };
            (append(field_at_t<E, I>{}), ...);
        }(std::make_index_sequence<field_count_v<E>>{});
        return values;
    }
};

} // namespace core::db::reflect

#ifdef CONGELADO_TEST
namespace core::db::reflect::tests {

struct MapSample
{
    std::int64_t id{};
    std::string name{};
    bool active{};
};
CORE_DB_REFLECT(MapSample, "map_samples",
    CORE_DB_FIELD("id", &MapSample::id, ColumnType::BIGINT, FieldFlags::PRIMARY_KEY),
    CORE_DB_FIELD("name", &MapSample::name, ColumnType::TEXT),
    CORE_DB_FIELD("active", &MapSample::active, ColumnType::BOOLEAN));

using namespace boost::ut;

suite<"core_db_entity_mapper"> core_db_entity_mapper_suite = []
{
    "round trips an entity through a Row"_test = []
    {
        Row row;
        row.cells["id"] = std::int64_t{7};
        row.cells["name"] = std::string{"Ada"};
        row.cells["active"] = true;

        const MapSample entity = EntityMapper{}.from_row<MapSample>(row);
        expect(entity.id == 7);
        expect(entity.name == "Ada");
        expect(entity.active);

        const auto values = EntityMapper{}.to_values(entity);
        expect(values.at("name") == DbValue{std::string{"Ada"}});
        expect(values.at("active") == DbValue{true});
    };
};

} // namespace core::db::reflect::tests
#endif
