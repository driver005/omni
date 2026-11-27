module;
export module app_connector:model_alipay_payments;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payments {

class AliPayHkRedirection : public serde::Serializer<AliPayHkRedirection>
{
public:
};

class AliPayRedirection : public serde::Serializer<AliPayRedirection>
{
public:
};

} // namespace app::model::payments

template<>
struct serde::Serializable<app::model::payments::AliPayRedirection>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "ali_pay_redirection";
    }
};

template<>
struct serde::Serializable<app::model::payments::AliPayHkRedirection>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "ali_pay_hk_redirection";
    }
};
