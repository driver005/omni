module;
#include <uuid.h>

export module app_model:refund;

import std;
import core_serde;
import core_behavior;
import core_database;
import core_error;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model {

/** @brief Upstream common_enums RefundStatus aliases. */
enum class RefundStatus
{
    Succeeded,
    Failed,
    Pending,
    Review,
};

/** @brief Upstream common_enums RefundType. */
enum class RefundType
{
    Scheduled,
    Instant,
};

/**
 * @brief The single refund model: store row, request, response, filter and aggregate shapes.
 *
 * Plain public data only. `field_metadata()` is the reflection source for
 * `serde::generated_fields<Refund>()`; every wire shape is derived from the same entries by
 * serde::wire_fields<T, WireViewMask>().
 */
class Refund : public core::behavior::Serializer<Refund>
{
public:
    // ── identity ───────────────────────────────────────────────────────────────
    uuids::uuid id{};
    uuids::uuid payment_id{};
    std::string refund_id{};

    // ── upstream refund columns ────────────────────────────────────────────────
    std::string merchant_id{};
    std::string merchant_reference_id{};
    std::int64_t amount{};
    std::int64_t net_amount{};
    std::int64_t surcharge_refund_amount{};
    std::string currency{};
    std::string status{};
    std::string reason{};
    std::string metadata{};
    std::string feature_metadata{};
    std::string connector{};
    std::string created_at{};
    std::string updated_at{};
    std::string error_code{};
    std::string error_message{};
    std::string unified_code{};
    std::string unified_message{};
    std::string issuer_error_code{};
    std::string issuer_error_message{};
    std::string connector_refund_id{};
    std::string attempt_id{};
    std::string profile_id{};
    std::string merchant_connector_id{};
    std::string split_refunds{};
    std::string raw_connector_response{};
    std::string processor_merchant_id{};
    std::string internal_reference_id{};
    std::string description{};
    std::string refund_type{"Instant"};

    // ── request / filter / aggregate shapes ────────────────────────────────────
    std::string cancellation_reason{};
    std::string payment_method_data{};
    std::string merchant_connector_details{};
    std::optional<std::string> input_amount{};
    std::optional<std::string> time_range{};
    std::optional<std::string> amount_filter{};
    std::optional<std::string> start_amount{};
    std::optional<std::string> end_amount{};
    std::optional<std::string> refund_status{};
    std::optional<std::string> limit{};
    std::optional<std::string> offset{};
    std::optional<std::string> start_time{};
    std::optional<std::string> end_time{};
    std::optional<std::string> reason_update{};
    std::optional<std::string> metadata_update{};
    std::optional<bool> force_sync{};
    std::optional<bool> all_keys_required{};
    std::optional<bool> return_raw_connector_response{};

    // ── list / aggregate documents ─────────────────────────────────────────────
    std::int64_t count{};
    std::int64_t total_count{};
    std::unordered_map<std::string, std::int64_t> status_with_count{};
    std::vector<std::string> connectors{};
    std::vector<std::string> currencies{};
    std::string data{};

    /** @brief Reflection source; every entry names a plain public data member. */
    [[nodiscard]] static constexpr auto field_metadata()
    {
        return std::tuple{
            serde::FieldMeta<"id", &Refund::id, serde::FieldOptions::init().pk().cache_key()>{},
            serde::FieldMeta<
                "payment_id",
                &Refund::payment_id,
                serde::FieldOptions::init()
                    .required()
                    .wire<
                        serde::WireUse{serde::WireView::get, 1},
                        serde::WireUse{serde::WireView::create | serde::WireView::v1, 0},
                        serde::WireUse{serde::WireView::create | serde::WireView::v2, 0},
                        serde::WireUse{serde::WireView::list, 0},
                        serde::WireUse{serde::WireView::list | serde::WireView::v1, 1},
                        serde::WireUse{serde::WireView::get | serde::WireView::v1, 1}>()>{},
            serde::FieldMeta<
                "refund_id",
                &Refund::refund_id,
                serde::FieldOptions::init()
                    .query_filter_and()
                    .wire<
                        serde::WireUse{serde::WireView::get, 0},
                        serde::WireUse{serde::WireView::create | serde::WireView::v1, 1},
                        serde::WireUse{serde::WireView::find, 0},
                        serde::WireUse{serde::WireView::update, 0},
                        serde::WireUse{serde::WireView::reverse, 0},
                        serde::WireUse{serde::WireView::update | serde::WireView::v1, 0},
                        serde::WireUse{serde::WireView::list, 1},
                        serde::WireUse{serde::WireView::list | serde::WireView::v1, 2},
                        serde::WireUse{serde::WireView::get | serde::WireView::v1, 0}>()>{},
            serde::FieldMeta<
                "merchant_id",
                &Refund::merchant_id,
                serde::FieldOptions::init()
                    .query_filter_and()
                    .wire<
                        serde::WireUse{serde::WireView::create | serde::WireView::v1, 2},
                        serde::WireUse{serde::WireView::create | serde::WireView::v2, 2},
                        serde::WireUse{serde::WireView::update | serde::WireView::v1, 1},
                        serde::WireUse{serde::WireView::get | serde::WireView::v1, 2}>()>{},
            serde::FieldMeta<
                "merchant_reference_id",
                &Refund::merchant_reference_id,
                serde::FieldOptions::init()
                    .wire<serde::WireUse{serde::WireView::create | serde::WireView::v2, 1}>()>{},
            serde::FieldMeta<
                "amount",
                &Refund::amount,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::get, 2},
                        serde::WireUse{serde::WireView::create | serde::WireView::v1, 3},
                        serde::WireUse{serde::WireView::create | serde::WireView::v2, 3},
                        serde::WireUse{
                            serde::WireView::get | serde::WireView::v1,
                            9,
                            serde::FieldWireName{"refund_amount"}
                        }>()>{},
            serde::FieldMeta<
                "net_amount",
                &Refund::net_amount,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::get, 3},
                        serde::WireUse{
                            serde::WireView::get | serde::WireView::v1,
                            10,
                            serde::FieldWireName{"total_amount"}
                        }>()>{},
            serde::FieldMeta<
                "surcharge_refund_amount",
                &Refund::surcharge_refund_amount,
                serde::FieldOptions::init().wire<serde::WireUse{serde::WireView::get, 4}>()>{},
            serde::FieldMeta<
                "currency",
                &Refund::currency,
                serde::FieldOptions::init()
                    .query_filter_and()
                    .wire<
                        serde::WireUse{serde::WireView::get, 5},
                        serde::WireUse{serde::WireView::list, 9},
                        serde::WireUse{serde::WireView::list | serde::WireView::v1, 11},
                        serde::WireUse{serde::WireView::get | serde::WireView::v1, 11},
                        serde::WireUse{serde::WireView::filters, 1}>()>{},
            serde::FieldMeta<
                "status",
                &Refund::status,
                serde::FieldOptions::init()
                    .query_filter_and()
                    .wire<
                        serde::WireUse{serde::WireView::get, 6},
                        serde::WireUse{serde::WireView::update | serde::WireView::v1, 3}>()>{},
            serde::FieldMeta<
                "reason",
                &Refund::reason,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::get, 7},
                        serde::WireUse{serde::WireView::create | serde::WireView::v1, 4},
                        serde::WireUse{serde::WireView::create | serde::WireView::v2, 4},
                        serde::WireUse{serde::WireView::update, 1},
                        serde::WireUse{serde::WireView::update | serde::WireView::v2, 0},
                        serde::WireUse{
                            serde::WireView::get | serde::WireView::v1,
                            13,
                            serde::FieldWireName{"refund_reason"}
                        }>()>{},
            serde::FieldMeta<
                "metadata",
                &Refund::metadata,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::get, 8},
                        serde::WireUse{serde::WireView::create | serde::WireView::v1, 6},
                        serde::WireUse{serde::WireView::create | serde::WireView::v2, 6},
                        serde::WireUse{serde::WireView::update, 2},
                        serde::WireUse{serde::WireView::update | serde::WireView::v2, 1}>()>{},
            serde::FieldMeta<
                "feature_metadata",
                &Refund::feature_metadata,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::get, 23},
                        serde::WireUse{serde::WireView::create | serde::WireView::v1, 11},
                        serde::WireUse{serde::WireView::create | serde::WireView::v2, 9}>()>{},
            serde::FieldMeta<
                "connector",
                &Refund::connector,
                serde::FieldOptions::init()
                    .query_filter_and()
                    .wire<
                        serde::WireUse{serde::WireView::get, 15},
                        serde::WireUse{serde::WireView::list, 7},
                        serde::WireUse{serde::WireView::list | serde::WireView::v1, 9},
                        serde::WireUse{serde::WireView::get | serde::WireView::v1, 5},
                        serde::WireUse{serde::WireView::filters, 0}>()>{},
            serde::FieldMeta<
                "created_at",
                &Refund::created_at,
                serde::FieldOptions::init()
                    .created_at()
                    .wire<
                        serde::WireUse{serde::WireView::get, 13},
                        serde::WireUse{serde::WireView::get | serde::WireView::v1, 15}>()>{},
            serde::FieldMeta<
                "updated_at",
                &Refund::updated_at,
                serde::FieldOptions::init()
                    .updated_at()
                    .wire<
                        serde::WireUse{serde::WireView::get, 14},
                        serde::WireUse{
                            serde::WireView::get | serde::WireView::v1,
                            16,
                            serde::FieldWireName{"modified_at"}
                        }>()>{},
            serde::FieldMeta<
                "error_code",
                &Refund::error_code,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::get, 10},
                        serde::WireUse{serde::WireView::update | serde::WireView::v1, 4}>()>{},
            serde::FieldMeta<
                "error_message",
                &Refund::error_message,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::get, 9},
                        serde::WireUse{serde::WireView::update | serde::WireView::v1, 5}>()>{},
            serde::FieldMeta<
                "unified_code",
                &Refund::unified_code,
                serde::FieldOptions::init().wire<serde::WireUse{serde::WireView::get, 11}>()>{},
            serde::FieldMeta<
                "unified_message",
                &Refund::unified_message,
                serde::FieldOptions::init().wire<serde::WireUse{serde::WireView::get, 12}>()>{},
            serde::FieldMeta<
                "issuer_error_code",
                &Refund::issuer_error_code,
                serde::FieldOptions::init().wire<serde::WireUse{serde::WireView::get, 19}>()>{},
            serde::FieldMeta<
                "issuer_error_message",
                &Refund::issuer_error_message,
                serde::FieldOptions::init().wire<serde::WireUse{serde::WireView::get, 20}>()>{},
            serde::FieldMeta<
                "connector_refund_id",
                &Refund::connector_refund_id,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::get, 22},
                        serde::WireUse{serde::WireView::update | serde::WireView::v1, 2},
                        serde::WireUse{serde::WireView::get | serde::WireView::v1, 7}>()>{},
            serde::FieldMeta<
                "attempt_id",
                &Refund::attempt_id,
                serde::FieldOptions::init()
                    .wire<serde::WireUse{serde::WireView::get | serde::WireView::v1, 8}>()>{},
            serde::FieldMeta<
                "profile_id",
                &Refund::profile_id,
                serde::FieldOptions::init()
                    .query_filter_and()
                    .wire<
                        serde::WireUse{serde::WireView::get, 16},
                        serde::WireUse{serde::WireView::list, 2},
                        serde::WireUse{serde::WireView::list | serde::WireView::v1, 3},
                        serde::WireUse{serde::WireView::get | serde::WireView::v1, 4}>()>{},
            serde::FieldMeta<
                "merchant_connector_id",
                &Refund::merchant_connector_id,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::get, 17},
                        serde::WireUse{serde::WireView::list, 8},
                        serde::WireUse{serde::WireView::list | serde::WireView::v1, 10},
                        serde::WireUse{serde::WireView::get | serde::WireView::v1, 6}>()>{},
            serde::FieldMeta<
                "split_refunds",
                &Refund::split_refunds,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::get, 18},
                        serde::WireUse{serde::WireView::create | serde::WireView::v1, 8}>()>{},
            serde::FieldMeta<
                "raw_connector_response",
                &Refund::raw_connector_response,
                serde::FieldOptions::init().wire<serde::WireUse{serde::WireView::get, 21}>()>{},
            serde::FieldMeta<
                "processor_merchant_id",
                &Refund::processor_merchant_id,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::list | serde::WireView::v1, 0},
                        serde::WireUse{serde::WireView::get | serde::WireView::v1, 3}>()>{},
            serde::FieldMeta<
                "internal_reference_id",
                &Refund::internal_reference_id,
                serde::FieldOptions::init()>{},
            serde::FieldMeta<
                "description",
                &Refund::description,
                serde::FieldOptions::init()
                    .wire<serde::WireUse{serde::WireView::get | serde::WireView::v1, 14}>()>{},
            serde::FieldMeta<
                "refund_type",
                &Refund::refund_type,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::create | serde::WireView::v1, 5},
                        serde::WireUse{serde::WireView::create | serde::WireView::v2, 5}>()>{},
            serde::FieldMeta<
                "cancellation_reason",
                &Refund::cancellation_reason,
                serde::FieldOptions::init().wire<serde::WireUse{serde::WireView::reverse, 1}>()>{},
            serde::FieldMeta<
                "payment_method_data",
                &Refund::payment_method_data,
                serde::FieldOptions::init()
                    .wire<serde::WireUse{serde::WireView::create | serde::WireView::v1, 10}>()>{},
            serde::FieldMeta<
                "merchant_connector_details",
                &Refund::merchant_connector_details,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::create | serde::WireView::v1, 7},
                        serde::WireUse{serde::WireView::create | serde::WireView::v2, 7},
                        serde::WireUse{serde::WireView::find, 2},
                        serde::WireUse{serde::WireView::find | serde::WireView::v2, 1}>()>{},
            serde::FieldMeta<"input_amount", &Refund::input_amount, serde::FieldOptions::init()>{},
            serde::FieldMeta<
                "time_range",
                &Refund::time_range,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::list, 5},
                        serde::WireUse{serde::WireView::list | serde::WireView::v1, 6}>()>{},
            serde::FieldMeta<
                "amount_filter",
                &Refund::amount_filter,
                serde::FieldOptions::init().wire<serde::WireUse{serde::WireView::list, 6}>()>{},
            serde::FieldMeta<
                "start_amount",
                &Refund::start_amount,
                serde::FieldOptions::init()
                    .wire<serde::WireUse{serde::WireView::list | serde::WireView::v1, 7}>()>{},
            serde::FieldMeta<
                "end_amount",
                &Refund::end_amount,
                serde::FieldOptions::init()
                    .wire<serde::WireUse{serde::WireView::list | serde::WireView::v1, 8}>()>{},
            serde::FieldMeta<
                "refund_status",
                &Refund::refund_status,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::list, 10},
                        serde::WireUse{serde::WireView::list | serde::WireView::v1, 12},
                        serde::WireUse{serde::WireView::get | serde::WireView::v1, 12},
                        serde::WireUse{serde::WireView::filters, 2}>()>{},
            serde::FieldMeta<
                "limit",
                &Refund::limit,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::list, 3},
                        serde::WireUse{serde::WireView::list | serde::WireView::v1, 4}>()>{},
            serde::FieldMeta<
                "offset",
                &Refund::offset,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::list, 4},
                        serde::WireUse{serde::WireView::list | serde::WireView::v1, 5}>()>{},
            serde::FieldMeta<"start_time", &Refund::start_time, serde::FieldOptions::init()>{},
            serde::FieldMeta<"end_time", &Refund::end_time, serde::FieldOptions::init()>{},
            serde::
                FieldMeta<"reason_update", &Refund::reason_update, serde::FieldOptions::init()>{},
            serde::FieldMeta<
                "metadata_update",
                &Refund::metadata_update,
                serde::FieldOptions::init()>{},
            serde::FieldMeta<
                "force_sync",
                &Refund::force_sync,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::find, 1},
                        serde::WireUse{serde::WireView::find | serde::WireView::v1, 0},
                        serde::WireUse{serde::WireView::find | serde::WireView::v2, 0}>()>{},
            serde::FieldMeta<
                "all_keys_required",
                &Refund::all_keys_required,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::create | serde::WireView::v1, 9},
                        serde::WireUse{serde::WireView::find, 3},
                        serde::WireUse{serde::WireView::find | serde::WireView::v1, 1}>()>{},
            serde::FieldMeta<
                "return_raw_connector_response",
                &Refund::return_raw_connector_response,
                serde::FieldOptions::init()
                    .wire<
                        serde::WireUse{serde::WireView::create | serde::WireView::v2, 8},
                        serde::WireUse{serde::WireView::find | serde::WireView::v2, 2}>()>{},
            serde::FieldMeta<
                "count",
                &Refund::count,
                serde::FieldOptions::init()
                    .wire<serde::WireUse{serde::WireView::get | serde::WireView::v2, 0}>()>{},
            serde::FieldMeta<
                "total_count",
                &Refund::total_count,
                serde::FieldOptions::init()
                    .wire<serde::WireUse{serde::WireView::get | serde::WireView::v2, 1}>()>{},
            serde::FieldMeta<
                "status_with_count",
                &Refund::status_with_count,
                serde::FieldOptions::init()
                    .wire<serde::WireUse{serde::WireView::aggregate, 0}>()>{},
            serde::FieldMeta<"connectors", &Refund::connectors, serde::FieldOptions::init()>{},
            serde::FieldMeta<"currencies", &Refund::currencies, serde::FieldOptions::init()>{},
            serde::FieldMeta<
                "data",
                &Refund::data,
                serde::FieldOptions::init()
                    .wire<serde::WireUse{serde::WireView::get | serde::WireView::v2, 2}>()>{}
        };
    }
};

// ── captured wire shapes, re-derived from field_metadata() ─────────────
static_assert(
    serde::wire_fields<Refund, serde::WireView::get>() == std::array<std::string_view, 24>{
                                                              "refund_id",
                                                              "payment_id",
                                                              "amount",
                                                              "net_amount",
                                                              "surcharge_refund_amount",
                                                              "currency",
                                                              "status",
                                                              "reason",
                                                              "metadata",
                                                              "error_message",
                                                              "error_code",
                                                              "unified_code",
                                                              "unified_message",
                                                              "created_at",
                                                              "updated_at",
                                                              "connector",
                                                              "profile_id",
                                                              "merchant_connector_id",
                                                              "split_refunds",
                                                              "issuer_error_code",
                                                              "issuer_error_message",
                                                              "raw_connector_response",
                                                              "connector_refund_id",
                                                              "feature_metadata"
                                                          }
);
static_assert(
    serde::wire_fields<Refund, serde::WireView::create | serde::WireView::v1>() ==
    std::array<std::string_view, 12>{
        "payment_id",
        "refund_id",
        "merchant_id",
        "amount",
        "reason",
        "refund_type",
        "metadata",
        "merchant_connector_details",
        "split_refunds",
        "all_keys_required",
        "payment_method_data",
        "feature_metadata"
    }
);
static_assert(
    serde::wire_fields<Refund, serde::WireView::create | serde::WireView::v2>() ==
    std::array<std::string_view, 10>{
        "payment_id",
        "merchant_reference_id",
        "merchant_id",
        "amount",
        "reason",
        "refund_type",
        "metadata",
        "merchant_connector_details",
        "return_raw_connector_response",
        "feature_metadata"
    }
);
static_assert(
    serde::wire_fields<Refund, serde::WireView::find>() == std::array<std::string_view, 4>{
                                                               "refund_id",
                                                               "force_sync",
                                                               "merchant_connector_details",
                                                               "all_keys_required"
                                                           }
);
static_assert(
    serde::wire_fields<Refund, serde::WireView::find | serde::WireView::v1>() ==
    std::array<std::string_view, 2>{"force_sync", "all_keys_required"}
);
static_assert(
    serde::wire_fields<Refund, serde::WireView::find | serde::WireView::v2>() ==
    std::array<std::string_view, 3>{
        "force_sync",
        "merchant_connector_details",
        "return_raw_connector_response"
    }
);
static_assert(
    serde::wire_fields<Refund, serde::WireView::update>() ==
    std::array<std::string_view, 3>{"refund_id", "reason", "metadata"}
);
static_assert(
    serde::wire_fields<Refund, serde::WireView::reverse>() ==
    std::array<std::string_view, 2>{"refund_id", "cancellation_reason"}
);
static_assert(
    serde::wire_fields<Refund, serde::WireView::update | serde::WireView::v1>() ==
    std::array<std::string_view, 6>{
        "refund_id",
        "merchant_id",
        "connector_refund_id",
        "status",
        "error_code",
        "error_message"
    }
);
static_assert(
    serde::wire_fields<Refund, serde::WireView::update | serde::WireView::v2>() ==
    std::array<std::string_view, 2>{"reason", "metadata"}
);
static_assert(
    serde::wire_fields<Refund, serde::WireView::list>() == std::array<std::string_view, 11>{
                                                               "payment_id",
                                                               "refund_id",
                                                               "profile_id",
                                                               "limit",
                                                               "offset",
                                                               "time_range",
                                                               "amount_filter",
                                                               "connector",
                                                               "merchant_connector_id",
                                                               "currency",
                                                               "refund_status"
                                                           }
);
static_assert(
    serde::wire_fields<Refund, serde::WireView::get | serde::WireView::v2>() ==
    std::array<std::string_view, 3>{"count", "total_count", "data"}
);
static_assert(
    serde::wire_fields<Refund, serde::WireView::list | serde::WireView::v1>() ==
    std::array<std::string_view, 13>{
        "processor_merchant_id",
        "payment_id",
        "refund_id",
        "profile_id",
        "limit",
        "offset",
        "time_range",
        "start_amount",
        "end_amount",
        "connector",
        "merchant_connector_id",
        "currency",
        "refund_status"
    }
);
static_assert(
    serde::wire_fields<Refund, serde::WireView::get | serde::WireView::v1>() ==
    std::array<std::string_view, 17>{
        "refund_id",
        "payment_id",
        "merchant_id",
        "processor_merchant_id",
        "profile_id",
        "connector",
        "merchant_connector_id",
        "connector_refund_id",
        "attempt_id",
        "refund_amount",
        "total_amount",
        "currency",
        "refund_status",
        "refund_reason",
        "description",
        "created_at",
        "modified_at"
    }
);
static_assert(
    serde::wire_fields<Refund, serde::WireView::filters>() ==
    std::array<std::string_view, 3>{"connector", "currency", "refund_status"}
);
static_assert(
    serde::wire_fields<Refund, serde::WireView::aggregate>() ==
    std::array<std::string_view, 1>{"status_with_count"}
);

/** @brief Plain value object for refund list constraints. */
struct RefundQuery
{
    std::string payment_id{};
    std::string merchant_id{};
    std::string status{};
    std::size_t limit{0};
    std::size_t offset{0};
};

/**
 * @brief Refund persistence, owned by the model so the store layer stays generic.
 *        AppContext exposes one instance bound to its database handle.
 */
} // namespace app::model

template<>
struct serde::Serializable<::app::model::Refund>
{
    [[nodiscard]] static constexpr auto fields()
    {
        return serde::generated_fields<::app::model::Refund>();
    }

    [[nodiscard]] static constexpr auto db_fields()
    {
        using R = ::app::model::Refund;
        return std::tuple{
            serde::DataFieldDesc<"refund_id", &R::refund_id, serde::FieldOptions::init().pk()>{},
            serde::DataFieldDesc<"payment_id", &R::payment_id, serde::FieldOptions{}>{},
            serde::DataFieldDesc<"merchant_id", &R::merchant_id, serde::FieldOptions{}>{},
            serde::DataFieldDesc<"amount", &R::amount, serde::FieldOptions{}>{},
            serde::DataFieldDesc<"currency", &R::currency, serde::FieldOptions{}>{},
            serde::DataFieldDesc<"status", &R::status, serde::FieldOptions{}>{},
            serde::DataFieldDesc<"reason", &R::reason, serde::FieldOptions{}>{},
            serde::DataFieldDesc<"connector", &R::connector, serde::FieldOptions{}>{},
            serde::DataFieldDesc<"created_at", &R::created_at, serde::FieldOptions{}>{},
            serde::DataFieldDesc<"updated_at", &R::updated_at, serde::FieldOptions{}>{}
        };
    }

    [[nodiscard]] static constexpr std::string_view table_name()
    {
        return "refunds";
    }
};

export namespace app::model {


class RefundRepository
{
public:
    explicit RefundRepository(core::db::Database& db) noexcept :
        m_db{db}
    {
    }

    Refund save_refund(Refund record)
    {
        return m_db.insert<Refund>(std::move(record));
    }

    [[nodiscard]] std::optional<Refund> find_refund(uuids::uuid id) const
    {
        return m_db.find<Refund>(core::db::DbValue{uuids::to_string(id)});
    }

    [[nodiscard]] std::optional<Refund> find_refund(std::int64_t id) const
    {
        return m_db.find<Refund>(core::db::DbValue{id});
    }

    [[nodiscard]] std::vector<Refund> list_refunds() const
    {
        return m_db.all<Refund>();
    }

    [[nodiscard]] std::optional<Refund> find_refund_by_string(std::string_view refund_id) const
    {
        const auto id = uuids::uuid::from_string(refund_id);
        if (!id) {
            return std::nullopt;
        }
        return m_db.find<Refund>(core::db::DbValue{uuids::to_string(*id)});
    }

    bool update_refund(const Refund& record)
    {
        return m_db.update_where_column<Refund>(
                   record,
                   "id",
                   core::db::Compare::EQ,
                   core::db::DbValue{uuids::to_string(record.id)}
               ) > 0;
    }

    bool remove_refund(uuids::uuid id)
    {
        return m_db.remove<Refund>(core::db::DbValue{uuids::to_string(id)}) > 0;
    }

    [[nodiscard]] std::vector<Refund> find_refunds_by_payment_id(std::string_view payment_id) const
    {
        core::db::QuerySpec spec = m_db.query<Refund>().spec();
        spec.predicates.push_back(
            core::db::Predicate{
                .column = "payment_id",
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{payment_id}}
            }
        );
        return m_db.collect<Refund>(spec);
    }

    [[nodiscard]] std::vector<Refund> find_refunds_by_constraints(const RefundQuery& query) const
    {
        core::db::QuerySpec spec = m_db.query<Refund>().spec();
        if (!query.payment_id.empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "payment_id",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.payment_id}
                }
            );
        }
        if (!query.merchant_id.empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "merchant_id",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.merchant_id}
                }
            );
        }
        if (!query.status.empty()) {
            spec.predicates.push_back(
                core::db::Predicate{
                    .column = "status",
                    .comparison = core::db::Compare::EQ,
                    .value = core::db::DbValue{query.status}
                }
            );
        }
        if (query.limit > 0) {
            spec.limit_count = query.limit;
        }
        spec.offset_count = query.offset;
        return m_db.collect<Refund>(spec);
    }

    bool update_refund_status(std::string_view refund_id, std::string status)
    {
        auto record = find_refund_by_string(refund_id);
        if (!record) {
            return false;
        }
        record->status = std::move(status);
        return update_refund(*record);
    }

    std::expected<Refund, core::error::AppError> create_refund(Refund record)
    {
        if (any_match("refunds", "refund_id", record.refund_id)) {
            return std::unexpected{already_exists("refund", record.refund_id)};
        }
        return m_db.insert<Refund>(std::move(record));
    }

private:
    template<class Record>
    [[nodiscard]] std::optional<Record>
    find_by_string(std::string_view column, std::string_view value) const
    {
        core::db::QuerySpec spec = m_db.query<Record>().spec();
        spec.predicates.push_back(
            core::db::Predicate{
                .column = std::string{column},
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{value}}
            }
        );
        spec.limit_count = 1;
        std::vector<Record> records = m_db.collect<Record>(spec);
        if (records.empty()) {
            return std::nullopt;
        }
        return std::move(records.front());
    }

    [[nodiscard]] bool
    any_match(std::string_view table, std::string_view column, std::string_view value) const
    {
        core::db::QuerySpec spec;
        spec.table = std::string{table};
        spec.predicates.push_back(
            core::db::Predicate{
                .column = std::string{column},
                .comparison = core::db::Compare::EQ,
                .value = core::db::DbValue{std::string{value}}
            }
        );
        spec.limit_count = 1;
        return !m_db.execute(spec).rows.empty();
    }

    [[nodiscard]] core::error::AppError
    already_exists(std::string_view entity, std::string_view key) const
    {
        return core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::ALREADY_EXISTS,
            std::format("{} already exists: {}", entity, key)
        );
    }

    core::db::Database& m_db;
};

} // namespace app::model


#ifdef CONGELADO_TEST
namespace app::model::tests {

using namespace boost::ut;

suite<"app_model_refund"> refund_model_suite = []
{
    "carries uuid identity and upstream wire names"_test = []
    {
        Refund refund{};
        refund.id = uuids::uuid::from_string("00000000-0000-4000-8000-000000000001").value();
        refund.refund_id = "rf_test";
        refund.amount = 900;
        expect(refund.id != uuids::uuid{});
        expect(serde::Serializable<Refund>::table_name() == "refunds");
        expect(serde::wire_fields<Refund, serde::WireView::get>().size() == 24);
        expect(std::tuple_size_v<decltype(Refund::field_metadata())> == 56);
    };
};

} // namespace app::model::tests
#endif
