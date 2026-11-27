module;
#include <cxxabi.h> // Itanium ABI demangling for model_name<T>() fallback

export module core_behavior:behavior;

import std;
import core_serde;
import core_database;
import core_cache;

// ─── Module-internal helpers ────────────────────────────────────────────────────────────────
// Nothing in this namespace is exported. Behavior<T> is the only route in.

namespace behavior_detail {

/** @brief DB storage fields (db_fields() when a model declares one, fields() otherwise). */
template<class T>
consteval auto storage_fields()
{
    if constexpr (requires { serde::Serializable<T>::db_fields(); }) {
        return serde::Serializable<T>::db_fields();
    } else {
        return serde::Serializable<T>::fields();
    }
}

template<class V>
inline constexpr bool is_std_vector_v = false;

template<class E, class A>
inline constexpr bool is_std_vector_v<std::vector<E, A>> = true;

template<class Fd>
using field_value_t = typename serde::MFPTraits<std::remove_cvref_t<decltype(Fd::getter)>>::value_t;

/** @brief True for a `std::vector<T>` filter field: one nested OR group per element. */
template<class Fd, class T>
consteval bool is_query_group()
{
    using V = field_value_t<Fd>;
    if constexpr (is_std_vector_v<V>) {
        return std::same_as<std::remove_cvref_t<typename V::value_type>, T>;
    } else {
        return false;
    }
}

/** @brief Normalizes a reflected value into a DbValue, unwrapping optional/pointer. */
template<class V>
[[nodiscard]] core::db::DbValue field_db_value(const V& value)
{
    if constexpr (requires { value.has_value(); }) {
        return value.has_value() ? core::db::to_db_value(*value)
                                 : core::db::DbValue{core::db::Null{}};
    } else if constexpr (std::is_pointer_v<V>) {
        return value != nullptr ? core::db::to_db_value(*value)
                                : core::db::DbValue{core::db::Null{}};
    } else {
        return core::db::to_db_value(value);
    }
}

[[nodiscard]] inline std::string lower_copy(std::string_view text)
{
    std::string out{text};
    std::ranges::transform(
        out,
        out.begin(),
        [](unsigned char c)
        {
            return static_cast<char>(std::tolower(c));
        }
    );
    return out;
}

template<class V>
[[nodiscard]] core::db::SortOrder order_direction(const V& value)
{
    using U = std::remove_cvref_t<V>;
    if constexpr (std::same_as<U, core::db::SortOrder>) {
        return value;
    } else if constexpr (std::same_as<U, bool>) {
        return value ? core::db::SortOrder::DESC : core::db::SortOrder::ASC;
    } else if constexpr (std::convertible_to<U, std::string_view>) {
        return lower_copy(std::string_view{value}).starts_with("desc") ? core::db::SortOrder::DESC
                                                                       : core::db::SortOrder::ASC;
    } else {
        return core::db::SortOrder::ASC;
    }
}

template<class V>
[[nodiscard]] std::size_t as_count(const V& value)
{
    using U = std::remove_cvref_t<V>;
    if constexpr (std::integral<U>) {
        return static_cast<std::size_t>(value);
    } else if constexpr (std::convertible_to<U, std::string_view>) {
        const auto text = std::string_view{value};
        std::size_t parsed{0};
        const auto result = std::from_chars(text.data(), text.data() + text.size(), parsed);
        return result.ec == std::errc{} ? parsed : std::size_t{0};
    } else {
        return std::size_t{0};
    }
}

/** @brief Stable string form of a reflected value for cache keys. */
template<class V>
[[nodiscard]] std::string key_part(const V& value)
{
    using U = std::remove_cvref_t<V>;
    if constexpr (requires { value.has_value(); }) {
        return value.has_value() ? key_part(*value) : std::string{"null"};
    } else if constexpr (std::same_as<U, bool>) {
        return value ? "true" : "false";
    } else if constexpr (std::is_enum_v<U>) {
        return std::format("{}", static_cast<std::underlying_type_t<U>>(value));
    } else if constexpr (std::convertible_to<U, std::string_view>) {
        return std::string{std::string_view{value}};
    } else if constexpr (std::is_arithmetic_v<U>) {
        return std::format("{}", value);
    } else {
        return "list"; // containers are not cache-key scalars
    }
}

[[nodiscard]] inline std::string epoch_string()
{
    using clock = std::chrono::system_clock;
    const auto count =
        std::chrono::duration_cast<std::chrono::seconds>(clock::now().time_since_epoch()).count();
    return std::to_string(count);
}

template<class T, class Setter>
void set_epoch(T& model, Setter setter, const std::string& epoch)
{
    if constexpr (requires(T& m, const std::string& s) { (m.*setter)(s); }) {
        (model.*setter)(epoch);
    } else if constexpr (requires(T& m, std::int64_t n) { (m.*setter)(n); }) {
        (model.*setter)(static_cast<std::int64_t>(std::stoll(epoch)));
    }
}

template<class T>
void apply_timestamps(T& model, bool inserting)
{
    const std::string now = epoch_string();
    std::apply(
        [&](auto... field)
        {
            (
                [&]
                {
                    using Fd = decltype(field);
                    if constexpr (Fd::options.m_created_at) {
                        if (inserting) {
                            set_epoch(model, Fd::setter, now);
                        }
                    }
                    if constexpr (Fd::options.m_updated_at) {
                        set_epoch(model, Fd::setter, now);
                    }
                }(),
                ...);
        },
        storage_fields<T>()
    );
}

template<class T>
void bump_version(T& model)
{
    std::apply(
        [&](auto... field)
        {
            (
                [&]
                {
                    using Fd = decltype(field);
                    if constexpr (Fd::options.m_version) {
                        auto current = (model.*Fd::getter)();
                        using V = std::remove_cvref_t<decltype(current)>;
                        if constexpr (std::integral<V>) {
                            if constexpr (requires(T& m, V v) { (m.*Fd::setter)(v); }) {
                                (model.*Fd::setter)(static_cast<V>(current + 1));
                            }
                        } else if constexpr (std::same_as<V, std::string>) {
                            if constexpr (requires(T& m, std::string s) { (m.*Fd::setter)(s); }) {
                                (model.*Fd::setter)(std::to_string(std::stoll(current) + 1));
                            }
                        }
                    }
                }(),
                ...);
        },
        storage_fields<T>()
    );
}

template<class T>
consteval bool has_soft_delete()
{
    return []<std::size_t... I>(std::index_sequence<I...>)
    {
        return (
            ... || std::tuple_element_t<I, decltype(storage_fields<T>())>::options.m_soft_delete
        );
    }(std::make_index_sequence<std::tuple_size_v<decltype(storage_fields<T>())>>{});
}

template<class T>
[[nodiscard]] std::string_view soft_delete_column()
{
    std::string_view result;
    std::apply(
        [&](auto... field)
        {
            (
                [&]
                {
                    using Fd = decltype(field);
                    if constexpr (Fd::options.m_soft_delete) {
                        result = Fd::name.string_view();
                    }
                }(),
                ...);
        },
        storage_fields<T>()
    );
    return result;
}

template<class T>
[[nodiscard]] bool is_soft_deleted(const T& model)
{
    if constexpr (has_soft_delete<T>()) {
        bool deleted = false;
        std::apply(
            [&](auto... field)
            {
                (
                    [&]
                    {
                        using Fd = decltype(field);
                        if constexpr (Fd::options.m_soft_delete) {
                            deleted = static_cast<bool>((model.*Fd::getter)());
                        }
                    }(),
                    ...);
            },
            storage_fields<T>()
        );
        return deleted;
    } else {
        return false;
    }
}

template<class T>
void set_soft_deleted(T& model)
{
    std::apply(
        [&](auto... field)
        {
            (
                [&]
                {
                    using Fd = decltype(field);
                    if constexpr (Fd::options.m_soft_delete) {
                        if constexpr (requires(T& m, bool b) { (m.*Fd::setter)(b); }) {
                            (model.*Fd::setter)(true);
                        }
                    }
                }(),
                ...);
        },
        storage_fields<T>()
    );
}

/** @brief Primary-key column + normalized value, discovered from DB storage metadata. */
template<class T>
[[nodiscard]] std::pair<std::string, core::db::DbValue> primary_key(const T& model)
{
    std::string column;
    core::db::DbValue value{core::db::Null{}};
    std::apply(
        [&](auto... field)
        {
            (
                [&]
                {
                    using Fd = decltype(field);
                    if constexpr (Fd::options.m_db.m_primary_key) {
                        if (column.empty()) {
                            column = std::string{Fd::name.string_view()};
                            value = field_db_value((model.*Fd::getter)());
                        }
                    }
                }(),
                ...);
        },
        storage_fields<T>()
    );
    return {std::move(column), std::move(value)};
}

/** @brief Smallest positive `ttl()` among storage fields; zero means "keep caller TTL". */
template<class T>
[[nodiscard]] std::uint64_t ttl_seconds()
{
    std::uint64_t seconds{0};
    std::apply(
        [&](auto... field)
        {
            (
                [&]
                {
                    using Fd = decltype(field);
                    if constexpr (Fd::options.m_ttl_seconds != 0) {
                        if (seconds == 0) {
                            seconds = Fd::options.m_ttl_seconds;
                        }
                    }
                }(),
                ...);
        },
        storage_fields<T>()
    );
    return seconds;
}

template<class T>
[[nodiscard]] std::string build_cache_key(const T& model)
{
    std::vector<std::string> parts;
    const auto collect = [&](auto predicate)
    {
        std::apply(
            [&](auto... field)
            {
                (
                    [&]
                    {
                        using Fd = decltype(field);
                        if constexpr (requires { predicate(Fd{}); }) {
                            if (predicate(Fd{})) {
                                parts.push_back(key_part((model.*Fd::getter)()));
                            }
                        }
                    }(),
                    ...);
            },
            storage_fields<T>()
        );
    };
    collect(
        []<class Fd>(Fd)
        {
            return Fd::options.m_cache_key;
        }
    );
    if (parts.empty()) {
        collect(
            []<class Fd>(Fd)
            {
                return Fd::options.m_db.m_primary_key;
            }
        );
    }
    std::string key;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i != 0) {
            key += ':';
        }
        key += parts[i];
    }
    return key;
}

/** @brief Declared names of fields that are data, not query-only inputs. */
template<class T>
[[nodiscard]] std::vector<std::string_view> data_field_names()
{
    std::vector<std::string_view> names;
    std::apply(
        [&](auto... field)
        {
            (
                [&]
                {
                    using Fd = decltype(field);
                    if constexpr (
                        Fd::options.m_query_role == serde::QueryRole::none ||
                        Fd::options.m_query_role == serde::QueryRole::ignore
                    ) {
                        names.push_back(Fd::name.string_view());
                    }
                }(),
                ...);
        },
        serde::Serializable<T>::fields()
    );
    return names;
}

/** @brief Serializes the data-only projection — query fields never reach wire or cache. */
template<class T>
[[nodiscard]] std::string serialize_model(const T& model)
{
    const auto bytes = serde::Ser::serialize(
        serde::json_content_type,
        serde::Serializer<T>::build_value(model, data_field_names<T>())
    );
    return std::string{reinterpret_cast<const char*>(bytes.data()), bytes.size()};
}

template<class T>
[[nodiscard]] std::optional<T> deserialize_model(std::string_view data)
{
    auto parsed = serde::Ser::deserialize<T>(serde::json_content_type, data);
    if (!parsed) {
        return std::nullopt;
    }
    return std::move(*parsed);
}

template<class T>
[[nodiscard]] std::string model_name()
{
    if constexpr (serde::IConnectable<T>) {
        return std::string{serde::Serializable<T>::table_name()};
    } else {
        const char* raw = typeid(T).name();
        int status = 0;
        std::unique_ptr<char, decltype(&std::free)> demangled{
            abi::__cxa_demangle(raw, nullptr, nullptr, &status),
            &std::free
        };
        return status == 0 && demangled != nullptr ? std::string{demangled.get()}
                                                   : std::string{raw};
    }
}

// ─── QuerySpec construction ─────────────────────────────────────────────────────────────────

template<class T>
void append_filter_predicates(core::db::QuerySpec& spec, const T& model);

/** @brief AND-only pass used for nested groups and for the base spec's scalar filters. */
template<class T>
void append_predicates_only(core::db::QuerySpec& spec, const T& model)
{
    std::apply(
        [&](auto... field)
        {
            (
                [&]
                {
                    using Fd = decltype(field);
                    if constexpr (
                        Fd::options.m_query_role == serde::QueryRole::filter &&
                        Fd::options.m_query_join == serde::QueryJoin::And
                    ) {
                        if constexpr (is_query_group<Fd, T>()) {
                            for (const auto& nested: (model.*Fd::getter)()) {
                                append_filter_predicates(spec, nested);
                            }
                        } else {
                            spec.predicates.push_back(
                                core::db::Predicate{
                                    .column = std::string{Fd::name.string_view()},
                                    .comparison = core::db::Compare::EQ,
                                    .value = field_db_value((model.*Fd::getter)()),
                                }
                            );
                        }
                    }
                }(),
                ...);
        },
        serde::Serializable<T>::fields()
    );
}

template<class T>
void append_filter_predicates(core::db::QuerySpec& spec, const T& model)
{
    append_predicates_only(spec, model);
}

template<class T, class Fd>
void apply_query_field(core::db::QuerySpec& spec, const T& model, Fd)
{
    using FdT = Fd;
    if constexpr (FdT::options.m_query_role == serde::QueryRole::limit) {
        spec.limit_count = as_count((model.*FdT::getter)());
    } else if constexpr (FdT::options.m_query_role == serde::QueryRole::offset) {
        spec.offset_count = as_count((model.*FdT::getter)());
    } else if constexpr (FdT::options.m_query_role == serde::QueryRole::order) {
        spec.ordering = std::make_pair(
            std::string{FdT::name.string_view()},
            order_direction((model.*FdT::getter)())
        );
    }
}

template<class T>
[[nodiscard]] core::db::QuerySpec build_base_spec(const T& model)
{
    core::db::QuerySpec spec;
    spec.table = std::string{core::db::reflect::EntityMeta<T>::table};
    const std::string joined = core::db::reflect::SqlGenerator{}.select_columns<T>();
    for (const auto part: std::views::split(joined, ',')) {
        spec.columns.emplace_back(std::ranges::begin(part), std::ranges::end(part));
    }

    // Soft-deleted rows are never visible to find/list.
    if (const auto column = soft_delete_column<T>(); !column.empty()) {
        spec.predicates.push_back(
            core::db::Predicate{
                .column = std::string{column},
                .comparison = core::db::Compare::EQ,
                .value = core::db::to_db_value(false),
            }
        );
    }

    append_predicates_only(spec, model);
    std::apply(
        [&](auto... field)
        {
            (
                [&]
                {
                    apply_query_field(spec, model, field);
                }(),
                ...);
        },
        serde::Serializable<T>::fields()
    );
    return spec;
}

/** @brief One predicate group per `filter/Or` vector<T> element; groups are OR-ed by the caller. */
template<class T>
[[nodiscard]] std::vector<std::vector<core::db::Predicate>> or_groups(const T& model)
{
    std::vector<std::vector<core::db::Predicate>> groups;
    std::apply(
        [&](auto... field)
        {
            (
                [&]
                {
                    using Fd = decltype(field);
                    if constexpr (
                        Fd::options.m_query_role == serde::QueryRole::filter &&
                        Fd::options.m_query_join == serde::QueryJoin::Or && is_query_group<Fd, T>()
                    ) {
                        for (const auto& nested: (model.*Fd::getter)()) {
                            core::db::QuerySpec sub;
                            append_predicates_only(sub, nested);
                            if (!sub.predicates.empty()) {
                                groups.push_back(std::move(sub.predicates));
                            }
                        }
                    }
                }(),
                ...);
        },
        serde::Serializable<T>::fields()
    );
    return groups;
}

/** @brief Final specs to execute: base alone, or one base+group per OR group. */
template<class T>
[[nodiscard]] std::vector<core::db::QuerySpec> build_query_specs(const T& model)
{
    const core::db::QuerySpec base = build_base_spec(model);
    const auto groups = or_groups(model);
    if (groups.empty()) {
        return {base};
    }
    std::vector<core::db::QuerySpec> specs;
    specs.reserve(groups.size());
    for (const auto& group: groups) {
        core::db::QuerySpec spec = base;
        spec.predicates.insert(spec.predicates.end(), group.begin(), group.end());
        specs.push_back(std::move(spec));
    }
    return specs;
}

/** @brief Dedupe key for OR-group merge — rendered PK value, stable across groups. */
template<class T>
[[nodiscard]] std::string entity_key(const T& entity)
{
    return core::db::DbValueLiteral{}(primary_key(entity).second);
}

template<class T>
[[nodiscard]] std::uint64_t remove_entity(core::db::Database& db, T& model)
{
    if constexpr (has_soft_delete<T>()) {
        set_soft_deleted(model);
        bump_version(model);
        apply_timestamps(model, false);
        const auto [column, value] = primary_key(model);
        return db.update_where_column(model, column, core::db::Compare::EQ, value);
    } else {
        const auto [column, value] = primary_key(model);
        (void)column;
        return db.remove<T>(value);
    }
}

} // namespace behavior_detail

// ─── Public surface ──────────────────────────────────────────────────────────────────────────

export namespace core::behavior {

/**
 * @brief A metrics sink exposes `counter(name).increment()` plus `record_flow(name)` — either
 * directly on the sink (the app's usual shape) or on the counter handle returned by `counter`.
 */
template<class Sink>
concept MetricSink = requires(Sink& sink, std::string_view name) {
    sink.counter(name).increment();
} && (
    requires(Sink& sink, std::string_view name) { sink.record_flow(name); } ||
    requires(Sink& sink, std::string_view name) { sink.counter(name).record_flow(name); }
);

/** @brief An audit sink exposes `audit(op, payload)`. */
template<class Sink>
concept AuditSink = requires(Sink& sink, std::string_view op, std::string_view payload) {
    sink.audit(op, payload);
};

/**
 * @brief Behavior mixin for a reflected model `T`.
 *
 * Inherit through `core::behavior::Serializer<T>`, not directly. Every method casts `this`
 * back to `T`, so CRTP discovery of FieldOptions metadata works without virtual dispatch.
 * DB writes use the model's `db_fields()` projection; query-only fields (QueryRole::filter/
 * limit/offset/order) live only in `Serializable::fields()` and never reach the DB, wire or
 * cache projection produced here.
 */
template<class T>
class Behavior
{
public:
    Behavior() = default;
    Behavior(const Behavior&) = default;
    Behavior& operator=(const Behavior&) = default;
    Behavior(Behavior&&) = default;
    Behavior& operator=(Behavior&&) = default;
    ~Behavior() = default;

    // ── Database CRUD ────────────────────────────────────────────────────────────────────────

    [[nodiscard]] T insert(core::db::Database& db)
    {
        return insert_impl(db, [](std::string_view, std::string_view) {});
    }

    template<AuditSink Sink>
    [[nodiscard]] T insert(core::db::Database& db, Sink& sink)
    {
        return insert_impl(
            db,
            [&sink](std::string_view op, std::string_view payload)
            {
                sink.audit(op, payload);
            }
        );
    }

    std::uint64_t update(core::db::Database& db)
    {
        return update_core(db);
    }

    template<AuditSink Sink>
    std::uint64_t update(core::db::Database& db, Sink& sink)
    {
        const std::uint64_t affected = update_core(db);
        sink.audit("update", behavior_detail::serialize_model(self()));
        return affected;
    }

    std::uint64_t remove(core::db::Database& db)
    {
        return remove_impl(db, [](std::string_view, std::string_view) {});
    }

    template<AuditSink Sink>
    std::uint64_t remove(core::db::Database& db, Sink& sink)
    {
        return remove_impl(
            db,
            [&sink](std::string_view op, std::string_view payload)
            {
                sink.audit(op, payload);
            }
        );
    }

    [[nodiscard]] std::optional<T> find(core::db::Database& db, const core::db::DbValue& id) const
    {
        auto found = db.find<T>(id);
        if (found && behavior_detail::is_soft_deleted(*found)) {
            return std::nullopt;
        }
        return found;
    }

    [[nodiscard]] std::vector<T> list(core::db::Database& db) const
    {
        const auto specs = behavior_detail::build_query_specs(self());
        std::vector<T> merged;
        std::set<std::string> seen;
        for (const auto& spec: specs) {
            for (auto& entity: db.collect<T>(spec)) {
                if (behavior_detail::is_soft_deleted(entity)) {
                    continue;
                }
                if (!seen.insert(behavior_detail::entity_key(entity)).second) {
                    continue;
                }
                merged.push_back(std::move(entity));
            }
        }
        return merged;
    }

    // ── Batch database ops ───────────────────────────────────────────────────────────────────

    [[nodiscard]] std::vector<T> insert_all(core::db::Database& db, std::vector<T> values) const
    {
        std::vector<T> saved;
        saved.reserve(values.size());
        for (auto& value: values) {
            behavior_detail::apply_timestamps(value, true);
            saved.push_back(db.insert(std::move(value)));
        }
        return saved;
    }

    std::uint64_t update_all(core::db::Database& db, std::vector<T> values) const
    {
        std::uint64_t total{0};
        for (auto& value: values) {
            behavior_detail::apply_timestamps(value, false);
            behavior_detail::bump_version(value);
            const auto [column, pk] = behavior_detail::primary_key(value);
            total += db.update_where_column(value, column, core::db::Compare::EQ, pk);
        }
        return total;
    }

    std::uint64_t remove_all(core::db::Database& db, std::vector<T> values) const
    {
        std::uint64_t total{0};
        for (auto& value: values) {
            total += behavior_detail::remove_entity(db, value);
        }
        return total;
    }

    // ── Cache ─────────────────────────────────────────────────────────────────────────────────

    [[nodiscard]] std::string cache_key() const
    {
        return behavior_detail::build_cache_key(self());
    }

    void cache_put(core::cache::Cache& cache, core::cache::Ttl ttl = {}) const
    {
        const auto seconds = behavior_detail::ttl_seconds<T>();
        if (seconds != 0) {
            ttl = core::cache::Ttl{}.seconds(static_cast<std::int64_t>(seconds));
        }
        cache.put(
            cache_key(),
            core::cache::to_cache_value(behavior_detail::serialize_model(self())),
            ttl
        );
    }

    [[nodiscard]] std::optional<T> cache_get(core::cache::Cache& cache) const
    {
        const auto result = cache.get(cache_key());
        if (!result.hit) {
            return std::nullopt;
        }
        return behavior_detail::deserialize_model<T>(
            core::cache::value_as<std::string>(result.value)
        );
    }

    bool cache_remove(core::cache::Cache& cache) const
    {
        return cache.erase(cache_key());
    }

    // ── Observability ─────────────────────────────────────────────────────────────────────────

    template<MetricSink Sink>
    void log_metrics(Sink& sink, std::string_view op) const
    {
        const std::string name =
            std::string{behavior_detail::model_name<T>()} + "." + std::string{op};
        sink.counter(name).increment();
        if constexpr (requires { sink.record_flow(name); }) {
            sink.record_flow(name);
        } else {
            sink.counter(name).record_flow(name);
        }
    }

private:
    [[nodiscard]] T& self() noexcept
    {
        return static_cast<T&>(*this);
    }

    [[nodiscard]] const T& self() const noexcept
    {
        return static_cast<const T&>(*this);
    }

    std::uint64_t update_core(core::db::Database& db)
    {
        behavior_detail::apply_timestamps(self(), false);
        behavior_detail::bump_version(self());
        const auto [column, pk] = behavior_detail::primary_key(self());
        return db.update_where_column(self(), column, core::db::Compare::EQ, pk);
    }

    template<class Emit>
    [[nodiscard]] T insert_impl(core::db::Database& db, Emit&& emit)
    {
        behavior_detail::apply_timestamps(self(), true);
        T saved = db.insert(self());
        self() = saved;
        emit("insert", behavior_detail::serialize_model(saved));
        return saved;
    }

    template<class Emit>
    std::uint64_t remove_impl(core::db::Database& db, Emit&& emit)
    {
        // remove_entity() performs the soft-delete UPDATE inline, so no nested audit fires.
        const std::uint64_t affected = behavior_detail::remove_entity(db, self());
        emit("remove", behavior_detail::serialize_model(self()));
        return affected;
    }
};

} // namespace core::behavior
