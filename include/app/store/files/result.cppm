export module app_store:files_result;

import std;

export namespace app::handlers::files {

// Exact wire field lists captured from the retired *Result Serializable field
// tuples. The create response keeps its DTO model; delete/retrieve are documents
// built in this order by the runtime.

[[nodiscard]] inline constexpr auto files_create_wire_fields()
{
    return std::array<std::string_view, 1>{"file_id"};
}

[[nodiscard]] inline constexpr auto files_delete_wire_fields()
{
    return std::array<std::string_view, 1>{"status"};
}

[[nodiscard]] inline constexpr auto files_retrieve_wire_fields()
{
    return std::array<std::string_view, 2>{"file_data", "content_type"};
}

} // namespace app::handlers::files
