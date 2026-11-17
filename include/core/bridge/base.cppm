export module core_bridge;

// Public surface only. Storage/loader partitions are re-exported for toolchain
// correctness but their declarations are not exported — importers cannot reach
// a Poco::SharedLibrary or a registry implementation. Entry point:
// BridgeBuilder -> Bridge.

export import :status;
export import :service_key;
export import :service_registry;
export import :bridge;
export import :builder;

export import :plugin_module;
export import :runtime;
export import :context;
