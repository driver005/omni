module;
export module app_connector:model_shared_cards_info;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::cards_info {

class CoBadgedCardNetworkMetadataValue : public serde::Serializer<CoBadgedCardNetworkMetadataValue>
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

} // namespace app::model::cards_info
