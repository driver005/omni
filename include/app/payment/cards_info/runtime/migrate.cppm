export module app_payment:cards_info_runtime_migrate;
import core_behavior;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_model;
import app_store;

import :cards_info_input;
import :cards_info_result;
import :cards_info_runtime_common;

export namespace app::handlers::cards_info {

class MigrateCardsInfoRuntime
{
public:
    explicit MigrateCardsInfoRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: migrate_cards_info ─────────────────────────
    // core/cards_info.rs:127-345,428-450, wrapped by
    // api::server_wrap(Flow::CardsInfoMigrate, ...). No counter in server_wrap;
    // CARDS_INFO_MIGRATED is incremented by the core itself.

    [[nodiscard]] core::behavior::Outcome<
        std::vector<::app::model::cards_info::CardInfoMigrationResponse>>
    migrate(const core::behavior::CallContext& ctx) const
    {
        const auto input = MigrateCardsInfoData{ctx};
        std::vector<CardInfoMigrationResponse> result{};
        result.reserve(input.records.size());
        std::size_t migrated{0};
        for (const auto& record: input.records) {
            auto flow = card_info_flow(record);
            if (flow) {
                ++migrated;
            }
            result.push_back(migration_response_from(std::move(flow), record));
        }
        m_context.logger()
            .counter("CARDS_INFO_MIGRATED")
            .increment(static_cast<std::uint64_t>(migrated));
        m_context.logger().record_flow("CardsInfoMigrate");
        return std::vector<::app::model::cards_info::CardInfoMigrationResponse>{std::move(result)};
    }

private:
    // ── core/cards_info.rs:179-196 typestate markers ────────────────────────────
    // State/TransitionTo are represented as a local enum + builder; the legal
    // transitions are Fetch->Add/Update and Add/Update->Response.

    enum class MigrateState
    {
        Fetch,
        Add,
        Update,
        Response,
    };

    class CardInfoBuilder
    {
    public:
        CardInfoBuilder() = default;

        [[nodiscard]] CardInfoBuilder set_card_info(CardInfoMigrateResponseRecord record) const
        {
            return CardInfoBuilder{MigrateState::Update, std::move(record)};
        }

        [[nodiscard]] CardInfoBuilder transition() const
        {
            return CardInfoBuilder{MigrateState::Add, std::nullopt};
        }

        [[nodiscard]] CardInfoBuilder
        set_updated_card_info(CardInfoMigrateResponseRecord record) const
        {
            return CardInfoBuilder{MigrateState::Response, std::move(record)};
        }

        [[nodiscard]] CardInfoBuilder
        set_added_card_info(CardInfoMigrateResponseRecord record) const
        {
            return CardInfoBuilder{MigrateState::Response, std::move(record)};
        }

        [[nodiscard]] CardInfoMigrateResponseRecord build() const
        {
            return card_info.value_or(CardInfoMigrateResponseRecord{});
        }

        [[nodiscard]] MigrateState state() const noexcept
        {
            return this->state_value;
        }

    private:
        explicit CardInfoBuilder(
            MigrateState value,
            std::optional<CardInfoMigrateResponseRecord> record
        ) :
            state_value{value},
            card_info{std::move(record)}
        {
        }

        MigrateState state_value{MigrateState::Fetch};
        std::optional<CardInfoMigrateResponseRecord> card_info{};
    };

    // ── core/cards_info.rs:199-287 CardInfoMigrateExecutor ──────────────────────

    class CardInfoMigrateExecutor
    {
    public:
        CardInfoMigrateExecutor(
            ::app::context::AppContext& context,
            const CardsInfoRuntimeSupport& support,
            const CardInfoUpdateRequest& record
        ) :
            m_context{context},
            m_support{support},
            m_record{record}
        {
        }

        [[nodiscard]] std::expected<std::optional<CardInfoRecord>, AppError> fetch_card_info() const
        {
            // core/cards_info.rs:212-219; storage error maps to InvalidCardIin.
            return m_context.repository().find_card_info_by_string(m_record.get_card_iin());
        }

        [[nodiscard]] std::expected<CardInfoRecord, AppError> add_card_info() const
        {
            // core/cards_info.rs:221-232; duplicate -> GenericDuplicateError.
            auto added =
                m_context.repository().create_card_info(m_support.to_store_record(m_record));
            if (!added) {
                return std::unexpected{m_support.generic_duplicate_error(
                    "CardInfo with given key already exists in our records"
                )};
            }
            m_support.cache_response(m_record.get_card_iin(), m_support.build_response(m_record));
            return *added;
        }

        [[nodiscard]] std::expected<CardInfoRecord, AppError> update_card_info() const
        {
            // core/cards_info.rs:234-286; last_updated=now(), miss -> GenericNotFoundError.
            auto repository = m_context.repository();
            auto stored = repository.find_card_info_by_string(m_record.get_card_iin());
            if (!stored) {
                return std::unexpected{m_support.generic_not_found_error(
                    "Card info with given key does not exist in our records"
                )};
            }
            m_support.apply_update(*stored, m_record);
            if (!repository.update_card_info(*stored)) {
                return std::unexpected{
                    m_support.internal_server_error("Failed while updating card info")
                };
            }
            m_support.cache_response(m_record.get_card_iin(), m_support.build_response(m_record));
            return *stored;
        }

    private:
        ::app::context::AppContext& m_context;
        const CardsInfoRuntimeSupport& m_support;
        const CardInfoUpdateRequest& m_record;
    };

    // ── core/cards_info.rs:289-345 CardInfoBuilder::build ───────────────────────

    [[nodiscard]] CardInfoMigrateResponseRecord
    to_migrate_record(const CardInfoResponse& response) const
    {
        using namespace ::app::model::cards_info;
        CardInfoMigrateResponseRecord out;
        if (!response.get_card_iin().empty()) {
            out.set_card_iin(response.get_card_iin());
        }
        out.set_card_issuer(response.get_card_issuer());
        out.set_card_network(response.get_card_network());
        out.set_card_type(response.get_card_type());
        out.set_card_sub_type(response.get_card_sub_type());
        out.set_card_issuing_country(response.get_card_issuing_country());
        out.set_country_code(response.get_country_code());
        out.set_funding_source(response.get_funding_source());
        out.set_card_iin_type(response.get_card_iin_type());
        out.set_virtual_card(response.get_virtual_card());
        out.set_gambling_blocked(response.get_gambling_blocked());
        out.set_co_badged_card_networks(response.get_co_badged_card_networks());
        out.set_card_segment_type(response.get_card_segment_type());
        out.set_numeric_country_code(response.get_numeric_country_code());
        out.set_prepaid(response.get_prepaid());
        out.set_regulated(response.get_regulated());
        out.set_issuer_phone(response.get_issuer_phone());
        out.set_issuer_url(response.get_issuer_url());
        out.set_regulated_name(response.get_regulated_name());
        out.set_reloadable_prepaid(response.get_reloadable_prepaid());
        out.set_account_updater(response.get_account_updater());
        out.set_account_level_management(response.get_account_level_management());
        out.set_domestic_only(response.get_domestic_only());
        out.set_level_two_supported(response.get_level_two_supported());
        out.set_level_three_supported(response.get_level_three_supported());
        out.set_issuer_currency(response.get_issuer_currency());
        out.set_combo_card(response.get_combo_card());
        out.set_authentication(response.get_authentication());
        out.set_cost(response.get_cost());
        out.set_issuer_supports_tokenization(response.get_issuer_supports_tokenization());
        out.set_billpay_enabled(response.get_billpay_enabled());
        out.set_ecom_enabled(response.get_ecom_enabled());
        out.set_flexible_credential_supported(response.get_flexible_credential_supported());
        out.set_card_subtype_code(response.get_card_subtype_code());
        out.set_multi_account_access_indicator(response.get_multi_account_access_indicator());
        return out;
    }

    [[nodiscard]] CardInfoMigrateResponseRecord
    migrate_record_from_store(const CardInfoRecord& record) const
    {
        auto fallback = m_support.cached_response(record.get_iin()).value_or(CardInfoResponse{});
        return to_migrate_record(
            m_support.response_from_store_enriched(record, std::move(fallback))
        );
    }

    // ── core/cards_info.rs:428-450 card_info_flow ───────────────────────────────

    [[nodiscard]] std::expected<CardInfoMigrateResponseRecord, std::string>
    card_info_flow(const CardInfoUpdateRequest& record) const
    {
        const auto builder = CardInfoBuilder{};
        const CardInfoMigrateExecutor executor{m_context, m_support, record};
        auto fetched = executor.fetch_card_info();
        if (!fetched) {
            return std::unexpected{fetched.error().get_message()};
        }
        if (*fetched) {
            // Fetch -> Update -> Response.
            const auto updated_builder =
                builder.set_card_info(migrate_record_from_store(**fetched));
            auto updated = executor.update_card_info();
            if (!updated) {
                return std::unexpected{updated.error().get_message()};
            }
            return updated_builder.set_updated_card_info(migrate_record_from_store(*updated))
                .build();
        }
        // Fetch -> Add -> Response.
        const auto added_builder = builder.transition();
        auto added = executor.add_card_info();
        if (!added) {
            return std::unexpected{added.error().get_message()};
        }
        return added_builder.set_added_card_info(migrate_record_from_store(*added)).build();
    }

    // ── api_models/cards_info.rs:349-406 CardInfoMigrationResponse::from ────────

    [[nodiscard]] CardInfoMigrationResponse migration_response_from(
        std::expected<CardInfoMigrateResponseRecord, std::string> outcome,
        const CardInfoUpdateRequest& record
    ) const
    {
        using namespace ::app::model::cards_info;
        CardInfoMigrationResponse out;
        out.set_card_iin(record.get_card_iin());
        out.set_line_number(record.get_line_number());
        if (outcome) {
            const auto& res = *outcome;
            out.set_card_issuer(res.get_card_issuer());
            out.set_card_network(res.get_card_network());
            out.set_card_type(res.get_card_type());
            out.set_card_sub_type(res.get_card_sub_type());
            out.set_card_issuing_country(res.get_card_issuing_country());
            out.set_country_code(res.get_country_code());
            out.set_funding_source(res.get_funding_source());
            out.set_card_iin_type(res.get_card_iin_type());
            out.set_virtual_card(res.get_virtual_card());
            out.set_gambling_blocked(res.get_gambling_blocked());
            out.set_co_badged_card_networks(res.get_co_badged_card_networks());
            out.set_card_segment_type(res.get_card_segment_type());
            out.set_numeric_country_code(res.get_numeric_country_code());
            out.set_prepaid(res.get_prepaid());
            out.set_regulated(res.get_regulated());
            out.set_issuer_phone(res.get_issuer_phone());
            out.set_issuer_url(res.get_issuer_url());
            out.set_regulated_name(res.get_regulated_name());
            out.set_reloadable_prepaid(res.get_reloadable_prepaid());
            out.set_account_updater(res.get_account_updater());
            out.set_account_level_management(res.get_account_level_management());
            out.set_domestic_only(res.get_domestic_only());
            out.set_level_two_supported(res.get_level_two_supported());
            out.set_level_three_supported(res.get_level_three_supported());
            out.set_issuer_currency(res.get_issuer_currency());
            out.set_combo_card(res.get_combo_card());
            out.set_authentication(res.get_authentication());
            out.set_cost(res.get_cost());
            out.set_issuer_supports_tokenization(res.get_issuer_supports_tokenization());
            out.set_billpay_enabled(res.get_billpay_enabled());
            out.set_ecom_enabled(res.get_ecom_enabled());
            out.set_flexible_credential_supported(res.get_flexible_credential_supported());
            out.set_card_subtype_code(res.get_card_subtype_code());
            out.set_multi_account_access_indicator(res.get_multi_account_access_indicator());
            out.set_migration_status(CardInfoMigrationStatus::Success);
        } else {
            // Err(e) -> default fields, migration_error = e, status Failed.
            out.set_migration_status(CardInfoMigrationStatus::Failed);
            out.set_migration_error(std::string{outcome.error()});
        }
        return out;
    }

    ::app::context::AppContext& m_context;
    CardsInfoRuntimeSupport m_support;
};

} // namespace app::handlers::cards_info
