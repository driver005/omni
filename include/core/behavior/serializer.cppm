export module core_behavior:serializer;

import std;
import core_serde;
import :behavior;

export namespace core::behavior {

/**
 * @brief Drop-in replacement for `serde::Serializer<T>` with Behavior mixed in.
 *
 * Both bases keep their own APIs: wire/HTTP methods come from `serde::Serializer<T>`, DB,
 * cache, metrics and audit methods come from `Behavior<T>`. Nothing overrides anything, so
 * existing `serialize()` / `validate()` / `to_response()` call sites keep their behavior.
 */
template<class T>
class Serializer : public serde::Serializer<T>, public Behavior<T>
{};

} // namespace core::behavior
