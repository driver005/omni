module;
#define UUID_SYSTEM_GENERATOR
#include <rfl.hpp>
#include <uuid.h>

export module core_serde:core;

import std;
import interfaces;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace serde {

inline constexpr std::string_view json_content_type{"application/json"};

// ─── StringLiteral ────────────────────────────────────────────────────────────
// Structural NTTP wrapper so string literals can be template parameters.
// Usage: template <StringLiteral S> — caller writes MyTemplate<"application/json">.

template<std::size_t N>
struct StringLiteral
{
    /**
     * @brief Copies a string literal's characters (including the trailing NUL) into `value`
     * at compile time — this is the whole trick that lets a literal ride along as a
     * template parameter, no cap.
     * @param source the string literal being wrapped, e.g. `"application/json"`.
     */
    consteval StringLiteral(const char (&source)[N]) noexcept
    {
        std::copy_n(source, N, m_value);
    }

    char m_value[N]{};

    /**
     * @brief Gets a view over the literal, trailing NUL excluded.
     * @return a `string_view` of length `N - 1` over `m_value`.
     */
    [[nodiscard]] constexpr std::string_view string_view() const noexcept
    {
        return {m_value, N - 1};
    }

    /**
     * @brief Compares two StringLiterals for equality, char by char — different lengths are
     * an instant false, same length falls through to a straight scan.
     * @tparam M the other StringLiteral's length (including its trailing NUL).
     * @param other the StringLiteral being compared against.
     * @return true if both literals hold the same characters.
     */
    template<std::size_t M>
    [[nodiscard]] constexpr bool operator==(const StringLiteral<M>& other) const noexcept
    {
        // Different lengths can never be equal — bail before touching a single char.
        if constexpr (N != M) {
            return false;
        }
        // Same length, so straight scan char by char, first mismatch is an instant false.
        for (std::size_t index = 0; index < N; ++index) {
            if (m_value[index] != other.m_value[index]) {
                return false;
            }
        }
        return true;
    }
};

// ─── MFPTraits ────────────────────────────────────────────────────────────────

template<typename MFP>
struct MFPTraits;

template<typename C, typename R>
struct MFPTraits<R (C::*)() const>
{
    using class_t = C;
    using value_t = std::remove_cvref_t<R>;
};

template<typename C, typename R>
struct MFPTraits<R (C::*)() const noexcept>
{
    using class_t = C;
    using value_t = std::remove_cvref_t<R>;
};

/** @brief Data-member pointers, for reflectors built from public members. */
template<typename C, typename R>
    requires std::is_member_object_pointer_v<R C::*>
struct MFPTraits<R C::*>
{
    using class_t = C;
    using value_t = std::remove_cvref_t<R>;
};

// ─── Wire operations ──────────────────────────────────────────────────────────

/** @brief CRUD/list operations a field may appear on. */
enum class WireOp : std::uint8_t
{
    create,
    read,
    update,
    list,
    del
};

/** @brief Single-bit mask for one WireOp. */
[[nodiscard]] constexpr std::uint8_t wire_op_bit(WireOp op) noexcept
{
    return static_cast<std::uint8_t>(1u << static_cast<unsigned>(op));
}

/** @brief True when `Fd` is enabled for `op` by its FieldOptions wire mask. */
template<typename Fd>
[[nodiscard]] constexpr bool enabled_on_wire(WireOp op) noexcept
{
    return (Fd::options.m_wire_ops & wire_op_bit(op)) != 0;
}

// ─── Wire views ───────────────────────────────────────────────────────────────

/**
 * @brief Bitmask tag for one wire shape. Bits from the CRUD and REST sections may be OR'd
 *        together on one field; v1/v2 select a version variant. Version bits match exactly
 *        at lookup time, so one state bit can carry several shapes.
 */
enum class WireView : std::uint32_t
{
    // ── CRUD section ───────────────────────────────────────────────────────────
    create = 1u << 0,
    find = 1u << 1,
    update = 1u << 2,
    remove = 1u << 3,
    all_crud = create | find | update | remove,

    // ── REST section ───────────────────────────────────────────────────────────
    post = 1u << 4,
    get = 1u << 5,
    put = 1u << 6,
    patch = 1u << 7,
    query = 1u << 8,
    del = 1u << 9,
    list = 1u << 10,
    filters = 1u << 11,
    aggregate = 1u << 12,
    reverse = 1u << 13,
    all_rest = post | get | put | patch | query | del | list | filters | aggregate | reverse,

    // ── versions ───────────────────────────────────────────────────────────────
    v1 = 1u << 14,
    v2 = 1u << 15,
};

/** @brief Combination of WireView bits. */
using WireViewMask = std::uint32_t;

/** @brief Mask covering the two version bits. */
inline constexpr WireViewMask wire_version_mask =
    static_cast<WireViewMask>(WireView::v1) | static_cast<WireViewMask>(WireView::v2);

/** @brief Mask covering every state bit (CRUD + REST). */
inline constexpr WireViewMask wire_state_mask = ~wire_version_mask;

/** @brief Converts one WireView tag to its mask value. */
[[nodiscard]] constexpr WireViewMask wire_view_bit(WireView view) noexcept
{
    return static_cast<WireViewMask>(view);
}

/** @brief Combines two view bits (or groups) into one structural mask value. */
[[nodiscard]] constexpr WireView operator|(WireView left, WireView right) noexcept
{
    return static_cast<WireView>(wire_view_bit(left) | wire_view_bit(right));
}

// ─── FieldOptionsDb ───────────────────────────────────────────────────────────

class FieldOptionsDb
{
public:
    bool m_primary_key = false;
    bool m_unique = false;
    bool m_nullable = true;
    bool m_skip_insert = false;
    bool m_skip_update = false;
    const char* m_ref_table = nullptr;
    const char* m_ref_column = nullptr;

    /// @brief Builds a default-initialized FieldOptionsDb — the fluent-chain starting point.
    /// @return a fresh FieldOptionsDb with every flag at its default (nullable, no PK/unique).
    [[nodiscard]] static constexpr FieldOptionsDb init()
    {
        return {};
    }

    /**
     * @brief Returns a copy with `m_primary_key` flipped on — chain this on `init()` to mark
     * a column as the table's PK, bet.
     * @return the modified copy; the original is untouched.
     */
    [[nodiscard]] constexpr FieldOptionsDb pk() const
    {
        auto opt = *this;
        opt.m_primary_key = true;
        return opt;
    }

    /**
     * @brief Returns a copy with `m_nullable` flipped off.
     * @return the modified copy; the original is untouched.
     */
    [[nodiscard]] constexpr FieldOptionsDb not_null() const
    {
        auto opt = *this;
        opt.m_nullable = false;
        return opt;
    }

    /**
     * @brief Returns a copy with `m_skip_insert` flipped on, so Sql::build_insert_sql /
     * build_insert_many_sql leave this column out of generated INSERTs.
     * @return the modified copy; the original is untouched.
     */
    [[nodiscard]] constexpr FieldOptionsDb no_insert() const
    {
        auto opt = *this;
        opt.m_skip_insert = true;
        return opt;
    }

    /**
     * @brief Returns a copy with `m_skip_update` flipped on, so Sql::build_update_sql leaves
     * this column out of generated UPDATE SET clauses.
     * @return the modified copy; the original is untouched.
     */
    [[nodiscard]] constexpr FieldOptionsDb no_update() const
    {
        auto opt = *this;
        opt.m_skip_update = true;
        return opt;
    }

    /**
     * @brief Returns a copy with a foreign-key reference attached, rendered as
     * `REFERENCES table(column)` by Sql::build_create_sql.
     * @param table_name the referenced table's name.
     * @param column_name the referenced column's name.
     * @warning Both pointers are stashed raw, no copy made — pass string literals (as every
     * call site does) or `table_name`/`column_name` must outlive every FieldOptionsDb copied
     * from this one. Pass a temporary `std::string::c_str()` and it's a dangling-pointer L
     * waiting to happen.
     * @return the modified copy; the original is untouched.
     */
    [[nodiscard]] constexpr FieldOptionsDb
    references(const char* table_name, const char* column_name) const
    {
        auto opt = *this;
        opt.m_ref_table = table_name;
        opt.m_ref_column = column_name;
        return opt;
    }
};

// ─── Query metadata ───────────────────────────────────────────────────────────

/**
 * @brief Role a reflected field plays in a Behavior query builder.
 * @note `none` is the default and means an ordinary data field; `ignore` also stays an
 * ordinary data field but is explicitly never consulted by the query builder.
 */
enum class QueryRole : std::uint8_t
{
    none,
    filter,
    limit,
    offset,
    order,
    ignore
};

/** @brief How sibling `filter` fields are combined by the query builder. */
enum class QueryJoin : std::uint8_t
{
    And,
    Or
};

// ─── FieldOptions ─────────────────────────────────────────────────────────────

/**
 * @brief Structural string-view carrier for a field's wire-name override.
 * @warning `std::string_view` cannot be stored in `FieldOptions`: `FieldOptions` rides as a
 * class-type NTTP on `FieldDesc`, and `std::string_view` has private data members, so it is
 * not a structural type. This public-member twin keeps `FieldOptions` structural while
 * behaving like a view for every reader.
 * @warning Points at caller-owned storage — pass string literals or static storage, same
 * contract as `FieldOptionsDb::references`.
 */
class FieldWireName
{
public:
    /// @brief Fixed inline capacity keeps the type structural for use as an NTTP.
    static constexpr std::size_t capacity = 64;

    char m_data[capacity]{};
    std::size_t m_size{0};

    constexpr FieldWireName() = default;

    constexpr FieldWireName(std::string_view value) noexcept :
        m_size{value.size() <= capacity ? value.size() : capacity}
    {
        for (std::size_t i = 0; i < m_size; ++i) {
            m_data[i] = value[i];
        }
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return m_size == 0;
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return m_size;
    }

    [[nodiscard]] constexpr const char* data() const noexcept
    {
        return m_data;
    }

    [[nodiscard]] constexpr std::string_view view() const noexcept
    {
        return {m_data, m_size};
    }

    [[nodiscard]] constexpr operator std::string_view() const noexcept
    {
        return view();
    }

    [[nodiscard]] constexpr bool operator==(std::string_view other) const noexcept
    {
        return view() == other;
    }
};

/**
 * @brief One field's participation in a wire shape: views, order, and optional key override.
 */
struct WireUse
{
    WireView views{};
    std::uint8_t order{};
    FieldWireName name{};
};

/**
 * @brief True when a WireUse contributes to the queried mask.
 * @details State bits intersect; version bits must match exactly. A query with no state bits
 *          matches every state, so one state bit can carry several shapes.
 */
[[nodiscard]] constexpr bool wire_use_matches(WireUse use, WireView mask) noexcept
{
    const auto views = wire_view_bit(use.views);
    const auto wanted = wire_view_bit(mask);
    const auto use_versions = views & wire_version_mask;
    if (use_versions != (wanted & wire_version_mask)) {
        return false;
    }
    const auto use_states = views & wire_state_mask;
    const auto wanted_states = wanted & wire_state_mask;
    return wanted_states == 0 ? use_states != 0 : (use_states & wanted_states) != 0;
}

/**
 * @brief Structural carrier for a field's validation pattern.
 * @warning Same storage contract as FieldWireName: points at caller-owned static/literal text.
 */
class FieldPattern
{
public:
    const char* m_data{nullptr};
    std::size_t m_size{0};

    constexpr FieldPattern() = default;

    constexpr FieldPattern(std::string_view value) noexcept :
        m_data{value.data()},
        m_size{value.size()}
    {
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return m_size == 0;
    }

    [[nodiscard]] constexpr std::string_view view() const noexcept
    {
        return m_data != nullptr ? std::string_view{m_data, m_size} : std::string_view{};
    }

    [[nodiscard]] constexpr operator std::string_view() const noexcept
    {
        return view();
    }
};

class FieldOptions
{
public:
    FieldOptionsDb m_db{};

    /// @brief Wire-key override; empty means fall back to `FieldDesc::name`.
    FieldWireName m_wire_name{};

    /// @brief Bitmask of WireOp bits; 0xff means every operation.
    std::uint8_t m_wire_ops{0xFF};

    /// @brief Optional regex pattern checked against string-like field values on validation.
    FieldPattern m_pattern{};

    /// @brief Maximum number of wire-shape entries one field may declare.
    static constexpr std::size_t max_wire_uses = 12;

    /// @brief Structural wire-shape entries; unused slots stay default-initialized.
    std::array<WireUse, max_wire_uses> m_wire_uses{};

    /// @brief Number of populated entries in m_wire_uses.
    std::uint8_t m_wire_use_count{0};

    /// @brief Reject a missing (disengaged optional / null pointer) value on validation.
    bool m_required{false};

    /// @brief Reject an empty (empty string/container/optional) value on validation.
    bool m_non_empty{false};

    /// @brief Optional per-field predicate; receives the address of the field's value.
    bool (*m_validator)(const void*){nullptr};

    /// @brief Query-builder role. `none` keeps this a plain data field.
    QueryRole m_query_role{QueryRole::none};

    /// @brief Join operator for sibling `filter` fields (And by default).
    QueryJoin m_query_join{QueryJoin::And};

    /// @brief Include this field's value in the Behavior cache key.
    bool m_cache_key{false};

    /// @brief Fill with the current epoch string before insert.
    bool m_created_at{false};

    /// @brief Fill with the current epoch string before insert and update.
    bool m_updated_at{false};

    /// @brief Increment this numeric field before update.
    bool m_version{false};

    /// @brief Soft-delete flag: `remove()` flips it and find/list exclude set rows.
    bool m_soft_delete{false};

    /// @brief Cache TTL in seconds. Zero leaves the caller's TTL untouched.
    std::uint64_t m_ttl_seconds{0};

    /// @brief Builds a default-initialized FieldOptions — the fluent-chain starting point.
    /// @return a fresh FieldOptions with a default-constructed `m_db` block.
    [[nodiscard]] static constexpr FieldOptions init()
    {
        return {};
    }

    /**
     * @brief Returns a copy with `m_db` swapped in — this is how FieldDesc's `Opts` template
     * param picks up its DB-column metadata (PK, nullability, FK, etc).
     * @param db_options the FieldOptionsDb to attach.
     * @return the modified copy; the original is untouched.
     */
    [[nodiscard]] constexpr FieldOptions with_db(FieldOptionsDb db_options) const
    {
        auto opt = *this;
        opt.m_db = db_options;
        return opt;
    }

    /// @brief Returns a copy with a wire-key override attached; empty keeps `FieldDesc::name`.
    /// @param name the alternate wire key — pass literal/static storage.
    [[nodiscard]] constexpr FieldOptions wire_name(std::string_view name) const
    {
        auto opt = *this;
        opt.m_wire_name = FieldWireName{name};
        return opt;
    }

    /// @brief Returns a copy with only the listed wire ops enabled.
    [[nodiscard]] constexpr FieldOptions only(std::initializer_list<WireOp> ops) const
    {
        auto opt = *this;
        std::uint8_t mask = 0;
        for (const WireOp op: ops) {
            mask = static_cast<std::uint8_t>(mask | wire_op_bit(op));
        }
        opt.m_wire_ops = mask;
        return opt;
    }

    /// @brief Returns a copy with the listed wire ops disabled.
    [[nodiscard]] constexpr FieldOptions not_on(std::initializer_list<WireOp> ops) const
    {
        auto opt = *this;
        for (const WireOp op: ops) {
            opt.m_wire_ops = static_cast<std::uint8_t>(opt.m_wire_ops & ~wire_op_bit(op));
        }
        return opt;
    }

    /// @brief Returns a copy with this field excluded from every wire operation.
    [[nodiscard]] constexpr FieldOptions none_on_wire() const
    {
        auto opt = *this;
        opt.m_wire_ops = 0;
        return opt;
    }

    /**
     * @brief Returns a copy declaring this field's wire-shape memberships.
     * @tparam Uses structural WireUse entries; first match wins when a query selects several.
     */
    template<WireUse... Uses>
    [[nodiscard]] constexpr FieldOptions wire() const
    {
        static_assert(sizeof...(Uses) <= max_wire_uses, "too many wire-shape entries");
        auto opt = *this;
        opt.m_wire_uses = {};
        std::size_t index = 0;
        ((opt.m_wire_uses[index++] = Uses), ...);
        opt.m_wire_use_count = static_cast<std::uint8_t>(sizeof...(Uses));
        return opt;
    }

    /// @brief Returns a copy with a validation pattern attached (literal/static storage).
    [[nodiscard]] constexpr FieldOptions pattern(std::string_view value) const
    {
        auto opt = *this;
        opt.m_pattern = FieldPattern{value};
        return opt;
    }

    // ─── FieldOptionsDb convenience wrappers ───────────────────────────────────

    [[nodiscard]] constexpr FieldOptions pk() const
    {
        return with_db(m_db.pk());
    }

    [[nodiscard]] constexpr FieldOptions not_null() const
    {
        return with_db(m_db.not_null());
    }

    [[nodiscard]] constexpr FieldOptions no_insert() const
    {
        return with_db(m_db.no_insert());
    }

    [[nodiscard]] constexpr FieldOptions no_update() const
    {
        return with_db(m_db.no_update());
    }

    [[nodiscard]] constexpr FieldOptions references(const char* table, const char* column) const
    {
        return with_db(m_db.references(table, column));
    }

    /// @brief Returns a copy marking this field as required.
    [[nodiscard]] constexpr FieldOptions required() const
    {
        auto opt = *this;
        opt.m_required = true;
        return opt;
    }

    /// @brief Returns a copy marking this field as non-empty.
    [[nodiscard]] constexpr FieldOptions non_empty() const
    {
        auto opt = *this;
        opt.m_non_empty = true;
        return opt;
    }

    /// @brief Returns a copy with a per-field validator function attached.
    /// @param validator predicate taking the address of the field value; nullptr clears it.
    [[nodiscard]] constexpr FieldOptions with_validator(bool (*validator)(const void*)) const
    {
        auto opt = *this;
        opt.m_validator = validator;
        return opt;
    }

    /// @brief Marks the field as an AND-ed equality filter in a Behavior query.
    [[nodiscard]] constexpr FieldOptions query_filter_and() const
    {
        auto opt = *this;
        opt.m_query_role = QueryRole::filter;
        opt.m_query_join = QueryJoin::And;
        return opt;
    }

    /// @brief Marks the field as an OR-group source; `std::vector<T>` fields carry one group
    /// per element and groups are OR-ed by executing one query each.
    [[nodiscard]] constexpr FieldOptions query_filter_or() const
    {
        auto opt = *this;
        opt.m_query_role = QueryRole::filter;
        opt.m_query_join = QueryJoin::Or;
        return opt;
    }

    /// @brief Marks the field as supplying the query's LIMIT.
    [[nodiscard]] constexpr FieldOptions query_limit() const
    {
        auto opt = *this;
        opt.m_query_role = QueryRole::limit;
        return opt;
    }

    /// @brief Marks the field as supplying the query's OFFSET.
    [[nodiscard]] constexpr FieldOptions query_offset() const
    {
        auto opt = *this;
        opt.m_query_role = QueryRole::offset;
        return opt;
    }

    /// @brief Marks the field as the ORDER BY column; its value carries the direction.
    [[nodiscard]] constexpr FieldOptions query_order_by() const
    {
        auto opt = *this;
        opt.m_query_role = QueryRole::order;
        return opt;
    }

    /// @brief Explicitly keeps the field out of the query builder while leaving it data.
    [[nodiscard]] constexpr FieldOptions query_ignore() const
    {
        auto opt = *this;
        opt.m_query_role = QueryRole::ignore;
        return opt;
    }

    /// @brief Includes this field's value in the Behavior cache key.
    [[nodiscard]] constexpr FieldOptions cache_key() const
    {
        auto opt = *this;
        opt.m_cache_key = true;
        return opt;
    }

    /// @brief Overrides the cache TTL (in seconds) used by `Behavior::cache_put`.
    [[nodiscard]] constexpr FieldOptions ttl(std::uint64_t seconds) const
    {
        auto opt = *this;
        opt.m_ttl_seconds = seconds;
        return opt;
    }

    /// @brief Marks the field for epoch stamping on insert.
    [[nodiscard]] constexpr FieldOptions created_at() const
    {
        auto opt = *this;
        opt.m_created_at = true;
        return opt;
    }

    /// @brief Marks the field for epoch stamping on insert and update.
    [[nodiscard]] constexpr FieldOptions updated_at() const
    {
        auto opt = *this;
        opt.m_updated_at = true;
        return opt;
    }

    /// @brief Marks the field as a version counter incremented before update.
    [[nodiscard]] constexpr FieldOptions version() const
    {
        auto opt = *this;
        opt.m_version = true;
        return opt;
    }

    /// @brief Marks the field as the soft-delete flag.
    [[nodiscard]] constexpr FieldOptions soft_delete() const
    {
        auto opt = *this;
        opt.m_soft_delete = true;
        return opt;
    }
};

// ─── FieldDesc ────────────────────────────────────────────────────────────────

/** @brief Default encode normalizer — leaves the stored value untouched. */
struct identity_normalizer
{
    template<typename T>
    [[nodiscard]] constexpr T&& operator()(T&& value) const noexcept
    {
        return std::forward<T>(value);
    }
};

/** @brief A pre-rendered JSON fragment, emitted verbatim by the wire converter. */
struct RawJson
{
    std::string json{};
};

template<
    rfl::internal::StringLiteral Name,
    auto Getter,
    auto Setter,
    FieldOptions Opts = FieldOptions{},
    auto Normalize = identity_normalizer{}>
struct FieldDesc
{
    // FIXME(clang-tidy): readability-identifier-naming — name/getter/setter/options have
    // external callers across 20+ files (e.g. include/model/**, include/serde/sql.cppm,
    // include/serde/cache.cppm, sdk/client/**, plugins/engine/handler/**); rename needs a
    // repo-wide sweep.
    static constexpr auto name =
        Name; // NOLINT(readability-identifier-naming) — shared field name with 20+ external
              // call sites across model/serde/sdk/plugins, rename out of scope
    static constexpr auto getter =
        Getter; // NOLINT(readability-identifier-naming) — shared field name with 20+ external
                // call sites across model/serde/sdk/plugins, rename out of scope
    static constexpr auto setter =
        Setter; // NOLINT(readability-identifier-naming) — shared field name with 20+ external
                // call sites across model/serde/sdk/plugins, rename out of scope
    static constexpr FieldOptions options =
        Opts; // NOLINT(readability-identifier-naming) — shared field name with 20+ external
              // call sites across model/serde/sdk/plugins, rename out of scope
    static constexpr auto normalize =
        Normalize; // NOLINT(readability-identifier-naming) — encode-time field normalizer
    using ClassType = MFPTraits<decltype(Getter)>::class_t;
    using ValueType = MFPTraits<decltype(Getter)>::value_t;
};

/**
 * @brief Reflection descriptor for a public data member — the compile-time mirror of
 * FieldDesc for models whose fields need no hand-written getters/setters.
 * @note Consumers should use `field_value<Fd>()`/`set_field_value<Fd>()`; those dispatch on
 * the presence of `member` so legacy FieldDesc descriptors keep working unchanged.
 */
template<
    rfl::internal::StringLiteral Name,
    auto MemberPtr,
    FieldOptions Opts = FieldOptions{},
    auto Normalize = identity_normalizer{}>
struct DataFieldDesc
{
    static constexpr auto name = Name;
    static constexpr auto member = MemberPtr;
    static constexpr FieldOptions options = Opts;
    static constexpr auto normalize = Normalize;
    using ClassType = MFPTraits<decltype(MemberPtr)>::class_t;
    using ValueType = MFPTraits<decltype(MemberPtr)>::value_t;
};

/** @brief One row of a model-declared `field_metadata()` tuple. */
template<
    rfl::internal::StringLiteral Name,
    auto MemberPtr,
    FieldOptions Opts = FieldOptions{},
    auto Normalize = identity_normalizer{}>
struct FieldMeta
{
    using Descriptor = DataFieldDesc<Name, MemberPtr, Opts, Normalize>;
    static constexpr auto descriptor = Descriptor{};
    static constexpr auto name = Name;
    static constexpr auto member = MemberPtr;
    static constexpr FieldOptions options = Opts;
    using ClassType = typename Descriptor::ClassType;
    using ValueType = typename Descriptor::ValueType;
};

/**
 * @brief Converts `T::field_metadata()` (a tuple of FieldMeta) into the descriptor tuple serde
 * consumes everywhere FieldDesc used to be required.
 */
template<typename T>
constexpr auto generated_fields()
{
    using Metadata = decltype(T::field_metadata());
    return []<std::size_t... I>(std::index_sequence<I...>)
    {
        return std::tuple<typename std::tuple_element_t<I, Metadata>::Descriptor...>{};
    }(std::make_index_sequence<std::tuple_size_v<Metadata>>{});
}

/**
 * @brief First matching WireUse for a descriptor under a mask, if any.
 */
template<typename Meta>
[[nodiscard]] constexpr const WireUse* matched_wire_use(WireView mask) noexcept
{
    for (std::size_t i = 0; i < Meta::options.m_wire_use_count; ++i) {
        const WireUse& use = Meta::options.m_wire_uses[i];
        if (wire_use_matches(use, mask)) {
            return &use;
        }
    }
    return nullptr;
}

/**
 * @brief Builds the captured wire key list for one view from T::field_metadata().
 * @details Fields are emitted in the order stored on their first matching WireUse; equal
 *          orders fall back to field_metadata() declaration order.
 */
template<typename T, WireView Mask>
consteval auto wire_fields()
{
    using Metadata = decltype(T::field_metadata());
    constexpr std::size_t total = std::tuple_size_v<Metadata>;

    struct Entry
    {
        std::string_view name;
        std::uint8_t order;
        std::uint8_t declaration;
    };

    constexpr std::size_t count = []<std::size_t... I>(std::index_sequence<I...>)
    {
        return (
            std::size_t{0} + ... +
            (matched_wire_use<std::tuple_element_t<I, Metadata>>(Mask) ? 1u : 0u)
        );
    }(std::make_index_sequence<total>{});

    std::array<Entry, count> entries{};
    std::size_t written = 0;
    [&]<std::size_t... I>(std::index_sequence<I...>)
    {
        (
            [&]
            {
                using Meta = std::tuple_element_t<I, Metadata>;
                if (const auto* use = matched_wire_use<Meta>(Mask)) {
                    entries[written++] = Entry{
                        use->name.empty() ? Meta::name.string_view()
                                          : static_cast<std::string_view>(use->name),
                        use->order,
                        static_cast<std::uint8_t>(I)
                    };
                }
            }(),
            ...);
    }(std::make_index_sequence<total>{});

    for (std::size_t i = 1; i < count; ++i) {
        Entry key = entries[i];
        std::size_t j = i;
        while (j > 0 && (key.order < entries[j - 1].order ||
                         (key.order == entries[j - 1].order &&
                          key.declaration < entries[j - 1].declaration))) {
            entries[j] = entries[j - 1];
            --j;
        }
        entries[j] = key;
    }

    std::array<std::string_view, count> result{};
    for (std::size_t i = 0; i < count; ++i) {
        result[i] = entries[i].name;
    }
    return result;
}

/** @brief True for descriptors backed by a public data-member pointer. */
template<typename Fd>
concept IDataFieldDesc = requires {
    Fd::member;
    typename Fd::ValueType;
};

/** @brief Either legacy getter/setter FieldDesc or data-member DataFieldDesc. */
template<typename Fd>
concept IFieldDescriptor = requires {
    Fd::name;
    Fd::options;
    Fd::normalize;
    typename Fd::ClassType;
    typename Fd::ValueType;
} && (IDataFieldDesc<Fd> || requires {
                               Fd::getter;
                               Fd::setter;
                           });

/** @brief Reads (and normalizes) a field through either descriptor kind. */
template<IFieldDescriptor Fd, typename T>
[[nodiscard]] constexpr decltype(auto) field_value(const T& object)
{
    if constexpr (IDataFieldDesc<Fd>) {
        return Fd::normalize(object.*Fd::member);
    } else {
        return Fd::normalize((object.*Fd::getter)());
    }
}

/** @brief Writes a field through either descriptor kind. */
template<IFieldDescriptor Fd, typename T, typename V>
constexpr void set_field_value(T& object, V&& value)
{
    if constexpr (IDataFieldDesc<Fd>) {
        object.*Fd::member = std::forward<V>(value);
    } else {
        (object.*Fd::setter)(std::forward<V>(value));
    }
}

// ─── Serializable<T> + concepts ───────────────────────────────────────────────

template<typename T>
struct Serializable;

template<typename T>
concept ISerializable = requires {
    { Serializable<T>::fields() };
};

template<typename T>
concept IConnectable = ISerializable<T> && requires {
    { Serializable<T>::table_name() } -> std::convertible_to<std::string_view>;
};

// ─── Format concepts ──────────────────────────────────────────────────────────
// IAnyFormat: a format class declares a content_type (usable in template packs).
// IFormat<F,T>: full wire-format contract — encode T → string, decode string → T.

template<typename F>
concept IAnyFormat = requires {
    { F::content_type } -> std::convertible_to<std::string_view>;
};

template<typename F, typename T>
concept IFormat =
    IAnyFormat<F> && ISerializable<T> && requires(const T& value, std::string_view data) {
        { F::encode(value) } -> std::same_as<std::string>;
        { F::decode(data) } -> std::same_as<std::expected<T, std::string>>;
    };

// ─── Generic value-kind classification / field-reflection helpers ────────────
//
// Deliberately not SQL-specific: value_kind_of<VT>()/pk_column_name<T>() just classify a
// reflected field's C++ type into a generic tag and find a type's PK field name — any
// consumer needing that (SQL codegen, or anything else) reaches for these, not a
// dialect-flavored string. Mapping a ValueKind to an actual column-type string (e.g.
// Postgres's "TEXT"/"BIGINT"/"JSONB") is a dialect's own opinion, not serde's.

enum class ValueKind : std::uint8_t
{
    STRING,
    BOOLEAN,
    INT64,
    UINT64,
    INT32,
    UINT32,
    DOUBLE,
    FLOAT,
    TIMESTAMP,
    UUID,
    OTHER
};

/**
 * @brief Classifies a reflected field's C++ value type into a generic kind tag.
 * @tparam VT the field's declared value type.
 * @warning Anything that doesn't match a known kind falls through to `ValueKind::OTHER` — no
 * compile error, no cap. A dialect plugin decides what that fallback actually means (e.g.
 * Postgres treats it as `JSONB`).
 * @return the value's generic kind.
 */
template<typename VT>
constexpr ValueKind value_kind_of()
{
    if constexpr (
        std::same_as<VT, std::string> || std::same_as<VT, std::optional<std::string>> ||
        std::is_enum_v<VT>
    ) {
        return ValueKind::STRING;
    } else if constexpr (std::same_as<VT, bool>) {
        return ValueKind::BOOLEAN;
    } else if constexpr (
        std::same_as<VT, std::chrono::system_clock::time_point> ||
        std::same_as<VT, std::optional<std::chrono::system_clock::time_point>>
    ) {
        return ValueKind::TIMESTAMP;
    } else if constexpr (std::same_as<VT, std::int64_t>) {
        return ValueKind::INT64;
    } else if constexpr (std::same_as<VT, std::uint64_t>) {
        return ValueKind::UINT64;
    } else if constexpr (std::same_as<VT, std::int32_t>) {
        return ValueKind::INT32;
    } else if constexpr (std::same_as<VT, std::uint32_t>) {
        return ValueKind::UINT32;
    } else if constexpr (std::same_as<VT, double>) {
        return ValueKind::DOUBLE;
    } else if constexpr (std::same_as<VT, float>) {
        return ValueKind::FLOAT;
    } else if constexpr (
        std::same_as<VT, uuids::uuid> || std::same_as<VT, std::optional<uuids::uuid>>
    ) {
        return ValueKind::UUID;
    } else {
        return ValueKind::OTHER;
    }
}

/**
 * @brief Finds the column name of `T`'s primary-key field by scanning every reflected field for
 * `options.m_db.m_primary_key`.
 * @tparam T the connectable type whose PK field is being looked up.
 * @warning If no field is marked `primary_key`, this quietly returns an empty `string_view`
 * instead of erroring — every caller then splices that empty string into whatever it's
 * building, a footgun for any `T` that forgot `.pk()` on its `FieldOptionsDb`.
 * @return the primary key's field name, or empty if no field is marked as one.
 */
template<IConnectable T>
std::string_view pk_column_name()
{
    std::string_view result;
    // Fold over every reflected field looking for the one flagged primary_key — linear
    // scan wearing a fold-expression trenchcoat.
    std::apply(
        [&](auto... fields)
        {
            ((fields.options.m_db.m_primary_key ? (result = fields.name.string_view())
                                                : std::string_view{}),
             ...);
        },
        Serializable<T>::fields()
    );
    return result;
}

// ─── Cache / SQL concepts ─────────────────────────────────────────────────────

template<typename F, typename T>
concept ICacheHelper = IConnectable<T> && requires(const T& value, std::string_view pk_value) {
    { F::pk_string(value) } -> std::same_as<std::string>;
    { F::cache_key(value) } -> std::same_as<std::string>;
    { F::template cache_key<T>(pk_value) } -> std::same_as<std::string>;
    { F::cache_value(value) } -> std::same_as<std::string>;
};

template<typename F, typename T>
concept ISqlBuilder = IConnectable<T> && requires(
                                             const T& value,
                                             std::string_view key,
                                             std::span<const std::string> keys,
                                             std::span<const T> values
                                         ) {
    { F::template build_create_sql<T>() } -> std::same_as<std::string>;
    { F::template build_select_sql<T>(key) } -> std::same_as<std::string>;
    { F::template build_select_many_sql<T>(keys) } -> std::same_as<std::string>;
    { F::template build_select_all_sql<T>() } -> std::same_as<std::string>;
    { F::template build_insert_sql<T>(value) } -> std::same_as<std::string>;
    { F::template build_insert_many_sql<T>(values) } -> std::same_as<std::string>;
    { F::template build_update_sql<T>(value) } -> std::same_as<std::string>;
    { F::template build_upsert_sql<T>(value) } -> std::same_as<std::string>;
    { F::template build_delete_sql<T>(key) } -> std::same_as<std::string>;
    { F::template build_delete_many_sql<T>(keys) } -> std::same_as<std::string>;
};

} // namespace serde

#ifdef CONGELADO_TEST
namespace serde::tests {

// Minimal IConnectable fixture for pk_column_name<T>() — one field, flagged as the PK.
class CoreTestRecord
{
public:
    CoreTestRecord() = default;

    void set_id(std::string id)
    {
        m_id = std::move(id);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return m_id;
    }

private:
    std::string m_id;
};

// Minimal metadata-only fixture for generated_fields<T>() and data-member descriptors.
class GeneratedTestRecord
{
public:
    std::string code{};
    std::int64_t count{};

    static constexpr auto field_metadata()
    {
        return std::tuple{
            FieldMeta<"code", &GeneratedTestRecord::code, FieldOptions::init().required()>{},
            FieldMeta<
                "count",
                &GeneratedTestRecord::count,
                FieldOptions::init().only({WireOp::create, WireOp::read})>{},
        };
    }
};

} // namespace serde::tests

template<>
struct serde::Serializable<serde::tests::CoreTestRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &serde::tests::CoreTestRecord::get_id,
                &serde::tests::CoreTestRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
        };
    }

    static constexpr std::string_view table_name()
    {
        return "core_test_records";
    }
};

namespace serde::tests {
using namespace boost::ut;

suite<"StringLiteral"> string_literal_suite = []
{
    "string_view excludes the trailing NUL"_test = []
    {
        constexpr StringLiteral literal{"hello"};

        expect(literal.string_view() == "hello");
        expect(literal.string_view().size() == 5);
    };

    "equal literals of the same length compare equal"_test = []
    {
        constexpr StringLiteral first{"abc"};
        constexpr StringLiteral second{"abc"};

        expect(first == second);
    };

    "same-length literals with different content compare unequal"_test = []
    {
        constexpr StringLiteral first{"abc"};
        constexpr StringLiteral second{"abd"};

        expect(not(first == second));
    };

    "different-length literals compare unequal"_test = []
    {
        constexpr StringLiteral first{"abc"};
        constexpr StringLiteral second{"abcd"};

        expect(not(first == second));
    };
};

suite<"FieldOptionsDb"> field_options_db_suite = []
{
    "init defaults to nullable, no PK, no unique, no skip flags"_test = []
    {
        constexpr auto options = FieldOptionsDb::init();

        expect(not options.m_primary_key);
        expect(not options.m_unique);
        expect(options.m_nullable);
        expect(not options.m_skip_insert);
        expect(not options.m_skip_update);
    };

    "pk/not_null/no_insert/no_update chain without mutating the original"_test = []
    {
        constexpr auto base = FieldOptionsDb::init();
        constexpr auto derived = base.pk().not_null().no_insert().no_update();

        expect(derived.m_primary_key);
        expect(not derived.m_nullable);
        expect(derived.m_skip_insert);
        expect(derived.m_skip_update);
        // Original stays untouched — every mutator on FieldOptionsDb returns a modified copy.
        expect(not base.m_primary_key);
        expect(base.m_nullable);
    };

    "references stores the referenced table/column pointers"_test = []
    {
        constexpr auto derived = FieldOptionsDb::init().references("users", "id");

        expect(std::string_view{derived.m_ref_table} == "users");
        expect(std::string_view{derived.m_ref_column} == "id");
    };
};

suite<"FieldOptions"> field_options_suite = []
{
    "with_db attaches the db options block without mutating the original"_test = []
    {
        constexpr auto db_options = FieldOptionsDb::init().pk();
        constexpr auto options = FieldOptions::init().with_db(db_options);

        expect(options.m_db.m_primary_key);
    };

    "query metadata defaults to none/And with every behavior flag off"_test = []
    {
        constexpr auto options = FieldOptions::init();

        expect(options.m_query_role == QueryRole::none);
        expect(options.m_query_join == QueryJoin::And);
        expect(not options.m_cache_key);
        expect(not options.m_created_at);
        expect(not options.m_updated_at);
        expect(not options.m_version);
        expect(not options.m_soft_delete);
        expect(options.m_ttl_seconds == 0u);
    };

    "query/cache/timestamp wrappers return copies without mutating the original"_test = []
    {
        constexpr auto base = FieldOptions::init();
        constexpr auto filtered = base.query_filter_or()
                                      .cache_key()
                                      .ttl(30)
                                      .created_at()
                                      .updated_at()
                                      .version()
                                      .soft_delete();

        expect(filtered.m_query_role == QueryRole::filter);
        expect(filtered.m_query_join == QueryJoin::Or);
        expect(filtered.m_cache_key);
        expect(filtered.m_ttl_seconds == 30u);
        expect(filtered.m_created_at);
        expect(filtered.m_updated_at);
        expect(filtered.m_version);
        expect(filtered.m_soft_delete);
        expect(base.m_query_role == QueryRole::none);
        expect(base.m_query_join == QueryJoin::And);
        expect(not base.m_cache_key);
    };

    "every query role wrapper maps to its QueryRole"_test = []
    {
        expect(FieldOptions::init().query_filter_and().m_query_role == QueryRole::filter);
        expect(FieldOptions::init().query_limit().m_query_role == QueryRole::limit);
        expect(FieldOptions::init().query_offset().m_query_role == QueryRole::offset);
        expect(FieldOptions::init().query_order_by().m_query_role == QueryRole::order);
        expect(FieldOptions::init().query_ignore().m_query_role == QueryRole::ignore);
    };
};

suite<"wire options"> wire_options_suite = []
{
    "defaults enable every wire op and carry no pattern"_test = []
    {
        constexpr auto options = FieldOptions::init();

        expect(options.m_wire_ops == 0xFF);
        expect(options.m_pattern.empty());
    };

    "only/not_on/none_on_wire compose without mutating the original"_test = []
    {
        using Descriptor = DataFieldDesc<
            "count",
            &GeneratedTestRecord::count,
            FieldOptions::init().only({WireOp::create, WireOp::read})>;

        constexpr auto base = FieldOptions::init();
        constexpr auto enabled = base.only({WireOp::create, WireOp::read});
        constexpr auto narrowed = enabled.not_on({WireOp::create});

        expect(enabled_on_wire<Descriptor>(WireOp::create));
        expect(enabled_on_wire<Descriptor>(WireOp::read));
        expect(not enabled_on_wire<Descriptor>(WireOp::update));
        expect(narrowed.m_wire_ops == wire_op_bit(WireOp::read));
        expect(base.m_wire_ops == 0xFF);
        expect(FieldOptions::init().none_on_wire().m_wire_ops == 0);
    };
};

suite<"generated_fields"> generated_fields_suite = []
{
    "metadata rows become data-member descriptors usable by generic accessors"_test = []
    {
        using Fields = decltype(generated_fields<GeneratedTestRecord>());
        using First = std::tuple_element_t<0, Fields>;
        using Second = std::tuple_element_t<1, Fields>;

        static_assert(std::same_as<First::ValueType, std::string>);
        static_assert(std::same_as<Second::ValueType, std::int64_t>);

        GeneratedTestRecord record{"ok", 7};
        expect(field_value<First>(record) == "ok");
        set_field_value<First>(record, std::string{"changed"});
        expect(record.code == "changed");
    };
};

suite<"value_kind_of"> value_kind_of_suite = []
{
    "classifies every known C++ type into its ValueKind tag"_test = []
    {
        expect(value_kind_of<std::string>() == ValueKind::STRING);
        expect(value_kind_of<bool>() == ValueKind::BOOLEAN);
        expect(value_kind_of<std::int64_t>() == ValueKind::INT64);
        expect(value_kind_of<std::uint64_t>() == ValueKind::UINT64);
        expect(value_kind_of<std::int32_t>() == ValueKind::INT32);
        expect(value_kind_of<std::uint32_t>() == ValueKind::UINT32);
        expect(value_kind_of<double>() == ValueKind::DOUBLE);
        expect(value_kind_of<float>() == ValueKind::FLOAT);
        expect(value_kind_of<uuids::uuid>() == ValueKind::UUID);
        expect(value_kind_of<std::chrono::system_clock::time_point>() == ValueKind::TIMESTAMP);
    };

    "an unrecognized type falls back to OTHER"_test = []
    {
        expect(value_kind_of<std::vector<int>>() == ValueKind::OTHER);
    };
};

suite<"pk_column_name"> pk_column_name_suite = []
{
    "finds the field flagged primary_key in FieldOptionsDb"_test = []
    {
        expect(pk_column_name<CoreTestRecord>() == "id");
    };
};

} // namespace serde::tests
#endif
