export module app_handlers_cards_info:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_card_iin_info;
import :runtime_create;
import :runtime_update;
import :runtime_migrate;

export namespace app::handlers::cards_info {

class CardsInfoRuntime
{
public:
    explicit CardsInfoRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_card_iin_info{context},
        m_create{context},
        m_update{context},
        m_migrate{context}
    {
    }

    [[nodiscard]] std::expected<CardIinInfoResult, core::error::AppError>
    card_iin_info(const CardIinInfoInput& input) const
    {
        return m_card_iin_info.card_iin_info(input);
    }

    [[nodiscard]] std::expected<CreateCardsInfoResult, core::error::AppError>
    create(const CreateCardsInfoInput& input) const
    {
        return m_create.create(input);
    }

    [[nodiscard]] std::expected<UpdateCardsInfoResult, core::error::AppError>
    update(const UpdateCardsInfoInput& input) const
    {
        return m_update.update(input);
    }

    [[nodiscard]] std::expected<MigrateCardsInfoResult, core::error::AppError>
    migrate(const MigrateCardsInfoInput& input) const
    {
        return m_migrate.migrate(input);
    }

private:
    app::context::AppContext& m_context;
    CardIinInfoRuntime m_card_iin_info;
    CreateCardsInfoRuntime m_create;
    UpdateCardsInfoRuntime m_update;
    MigrateCardsInfoRuntime m_migrate;
};

} // namespace app::handlers::cards_info
