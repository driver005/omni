//! OpenAPI-derived HTTP client surface for the Rust SDK.
//!
//! The `Client` type in this module is generated at compile time by `progenitor` from
//! `sdk/openapi/openapi.json`. The document itself is produced from registered C++ route
//! metadata by `//plugins/generator/openapi_generator:openapi_dump`, so request paths and
//! methods stay in sync with the server automatically.
//!
//! Hand-written business logic must live in `sdk/domain`, never here.

progenitor::generate_api!(
    spec = {
        path = "sdk/openapi/openapi.json",
        relative_to = OutDir,
    },
);
