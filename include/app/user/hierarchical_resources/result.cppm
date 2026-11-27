export module app_user:hierarchical_resources_result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::hierarchical_resources {

// Wire field lists captured from the former Result Serializable specializations.
// Each Result reused the full FieldDesc tuple of its underlying model, so these
// lists hold the exact declared field names in Serializable<T>::fields() order.

inline constexpr std::array<std::string_view, 2> kGenerateHierarchicalResourceResponseWireFields{
    "id",
    "data",
};

inline constexpr std::array<std::string_view, 2> kUploadCertificateResponseWireFields{
    "id",
    "created_at",
};

inline constexpr std::array<std::string_view, 1> kListHierarchicalResourcesResponseWireFields{
    "resources",
};

inline constexpr std::array<std::string_view, 1> kLinkHierarchicalResourceResponseWireFields{
    "id",
};

} // namespace app::handlers::hierarchical_resources
