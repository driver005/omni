export module app_routing:gsm_result;

import std;
import core_serde;

export namespace app::handlers::gsm {

// The removed Result wrappers reused GsmResponse / GsmDeleteResponse's FieldDesc
// tuples. This constant captures those exact wire field lists.
template<class T>
[[nodiscard]] consteval auto wire_fields()
{
    using fields_type = decltype(serde::Serializable<T>::fields());
    return []<std::size_t... I>(std::index_sequence<I...>)
    {
        return std::array<std::string_view, sizeof...(I)>{
            std::tuple_element_t<I, fields_type>::name.string_view()...
        };
    }(std::make_index_sequence<std::tuple_size_v<fields_type>>{});
}

} // namespace app::handlers::gsm
