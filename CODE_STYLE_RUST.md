# Rust code style

All hand-written Rust in `sdk/` follows the
[Google Rust Style Guide](https://google.github.io/styleguide/rustguide.html).

Enforced locally with `make sdk-lint` using `sdk/rustfmt.toml`:

- `max_width = 100`
- `imports_granularity = "Module"` (one `use` per module path, no glob imports)
- lower-case hex literals, formatted macro matchers

Additional review rules applied by hand:

- one small crate per abstraction, flat file layout (no `mod.rs` chains)
- public enums marked `#[non_exhaustive]`; public items documented with `///`
- `expect` over `unwrap`; errors flow through `SdkError`
- borrow over clone; named fields over tuples
- macro-expanded code from `progenitor::generate_api!` is generated code and is not hand-edited
