export module app_routing:blocklist_result;

import std;
import core_serde;
import app_model;

export namespace app::handlers::blocklist {

// The removed Result wrappers each delegated Serializable<Result>::fields() to
// the underlying response DTO. This constant captures that exact field list;
// handlers pass it to Serializer<T>::serialize so emitted objects keep the
// original wire keys and declaration order.
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

} // namespace app::handlers::blocklist
