export module core_workflow;

// Public surface — mirrors core_router / core_database / core_cron.
export import :engine;
export import :value;
export import :state;
export import :step_response;
export import :step_options;
export import :step_outcome;
export import :context;
export import :expression;
export import :step;
export import :hook;
export import :definition;
export import :builder;
export import :checkpoint;
export import :execution;
export import :registry;
export import :workflow;

// Interface partitions re-exported for toolchain correctness only. Their
// declarations live in a non-exported namespace, so an importer of
// core_workflow cannot name a driver, backend or runtime.
export import :driver;
export import :runtime;
