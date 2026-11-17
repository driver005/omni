export module core_bridge:service_key;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::bridge {

/** @brief A typed service key: a name plus the C++ type it resolves to. */
class ServiceKey
{
public:
    template<typename T>
    [[nodiscard]] static ServiceKey make(std::string_view name)
    {
        return ServiceKey{std::string{name}, &typeid(T)};
    }

    [[nodiscard]] const std::string& name() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] const std::type_info& type() const noexcept
    {
        return *m_type;
    }

    [[nodiscard]] bool matches(const std::type_info& other) const noexcept
    {
        return *m_type == other;
    }

private:
    ServiceKey(std::string name, const std::type_info* type) :
        m_name{std::move(name)},
        m_type{type}
    {
    }

    std::string m_name{};
    const std::type_info* m_type{&typeid(void)};
};

} // namespace core::bridge

#ifdef CONGELADO_TEST
namespace core::bridge::tests {
using namespace boost::ut;

suite<"core_bridge_service_key"> core_bridge_service_key_suite = []
{
    "keys carry a name and a type"_test = []
    {
        const auto key = ServiceKey::make<int>("counter");
        expect(key.name() == "counter");
        expect(key.matches(typeid(int)));
        expect(!key.matches(typeid(double)));
    };
};

} // namespace core::bridge::tests
#endif
