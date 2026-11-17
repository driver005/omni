# core_bridge

C++20 module bridge connecting the `include/` core facades to the plugin layer.
The heavy lifting is done by **POCO** (C++ library, fetched from BCR), not by
hand-written platform code.

## What POCO does here

- `Poco::Any` — type-erased service storage. The host registers core services;
  plugins resolve them and may register their own — bidirectional by design.
- `Poco::SharedLibrary` — loads plugin `.so`/`.dll` files and resolves the
  `congelado_bridge_register` C entrypoint. No `dlopen` written by hand.

## Shape

`base.cppm` is the primary interface (`export module core_bridge;`). Loader and
runtime partitions (`plugin_module`, `bridge_runtime`, `bridge_context`) are
re-exported only for toolchain correctness; their declarations are not exported,
so importers cannot reach a `Poco::SharedLibrary` or the registry implementation.
`Bridge` owns its state through a private incomplete `Runtime` type (pImpl).

Each partition defines one class (or one cohesive value type) and one `boost.ut`
test suite under `CONGELADO_TEST`.

## Usage

```cpp
import core_bridge;

core::bridge::BridgeBuilder builder;
auto bridge = builder.option("base_dir", "./plugins").build();

// Host -> plugins: register a service (facades, config, loggers, ...).
const auto key = core::bridge::ServiceKey::make<MyService>("my_service");
bridge.register_service(key, MyService{});

// Plugins -> host: a plugin registers services through the same registry
// during its `congelado_bridge_register(void* context)` callback.
bridge.load_plugin("./plugins/libmy_plugin.so");

auto resolved = bridge.resolve<MyService>(key);
```

## Build

`BUILD` uses `congelado_module_library` with `base.cppm` as the primary
interface, with `deps = ["@poco//:PocoFoundation"]` (BCR). POCO headers are
`#include`d in the global module fragment of the two partitions that need them.
