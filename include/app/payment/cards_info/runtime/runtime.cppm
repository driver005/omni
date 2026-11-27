export module app_payment:cards_info_runtime;

import std;
import core_error;
import app_context;

import :cards_info_input;
import :cards_info_result;
import :cards_info_runtime_common;
import :cards_info_runtime_card_iin_info;
import :cards_info_runtime_create;
import :cards_info_runtime_update;
import :cards_info_runtime_migrate;

export namespace app::handlers::cards_info {

class CardsInfoRuntime
{
public:
    explicit CardsInfoRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_card_iin_info{context},
        m_create{context},
        m_update{context},
        m_migrate{context}
    {
    }

    [[nodiscard]] std::expected<::app::model::cards_info::CardInfoResponse, core::error::AppError>
    card_iin_info(const CardIinInfoInput& input) const
    {
        return m_card_iin_info.card_iin_info(input);
    }

    [[nodiscard]] std::expected<::app::model::cards_info::CardInfoResponse, core::error::AppError>
    create(const CreateCardsInfoInput& input) const
    {
        return m_create.create(input);
    }

    [[nodiscard]] std::expected<::app::model::cards_info::CardInfoResponse, core::error::AppError>
    update(const UpdateCardsInfoInput& input) const
    {
        return m_update.update(input);
    }

    [[nodiscard]] std::expected<
        std::vector<::app::model::cards_info::CardInfoMigrationResponse>,
        core::error::AppError>
    migrate(const MigrateCardsInfoInput& input) const
    {
        return m_migrate.migrate(input);
    }

private:
    ::app::context::AppContext& m_context;
    CardIinInfoRuntime m_card_iin_info;
    CreateCardsInfoRuntime m_create;
    UpdateCardsInfoRuntime m_update;
    MigrateCardsInfoRuntime m_migrate;
};

} // namespace app::handlers::cards_info
