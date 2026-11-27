module;
export module app_connector:model_bluecode_payments;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payments {

class BluecodeQrRedirect : public serde::Serializer<BluecodeQrRedirect>
{
public:
};

} // namespace app::model::payments

template<>
struct serde::Serializable<app::model::payments::BluecodeQrRedirect>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "bluecode_qr_redirect";
    }
};
