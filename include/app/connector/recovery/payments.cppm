module;
export module app_connector:model_recovery_payments;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payments {

class RecoveryConnectorDetailsValue : public serde::Serializer<RecoveryConnectorDetailsValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class RecoveryConnectorValue : public serde::Serializer<RecoveryConnectorValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

} // namespace app::model::payments
