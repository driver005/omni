module;

export module core_serde;

export import :core;
export import :json_codec;
export import :converter;
export import :cache;
export import :jsonld;
export import :error_kind;
export import :error;
export import :serializer;

import interfaces;
#ifdef CONGELADO_TEST
import boost.ut;
import io_layer_http2;
#endif


#ifdef CONGELADO_TEST
namespace serde::tests {

// Minimal ISerdeFormat fixture — real JSON round-trip via rfl::json, no plugin machinery.
class MockJsonFormat final : public interfaces::ISerdeFormat
{
public:
    [[nodiscard]] std::string_view content_type() const noexcept override
    {
        return "application/json";
    }

    [[nodiscard]] std::string_view format_name() const noexcept override
    {
        return "mock-json";
    }

    [[nodiscard]] std::expected<std::string, std::string>
    encode(const rfl::Generic& value) const override
    {
        return rfl::json::write(value);
    }

    [[nodiscard]] std::expected<rfl::Generic, std::string>
    decode(std::string_view data) const override
    {
        auto result = rfl::json::read<rfl::Generic>(data);
        if (!result) {
            return std::unexpected{result.error().what()};
        }
        return *result;
    }
};

// Minimal ISerializable fixture with a numeric field, so from_map has a parse-failure path.
class NumericTestRecord
{
public:
    NumericTestRecord() = default;

    void set_count(int count)
    {
        m_count = count;
    }

    [[nodiscard]] int get_count() const noexcept
    {
        return m_count;
    }

private:
    int m_count{0};
};

} // namespace serde::tests

template<>
struct serde::Serializable<serde::tests::NumericTestRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "count",
                &serde::tests::NumericTestRecord::get_count,
                &serde::tests::NumericTestRecord::set_count>{},
        };
    }
};

namespace serde::tests {
using namespace boost::ut;

suite<"SerdeFormatRegistry"> serde_format_registry_suite = []
{
    "find returns nullptr when no format is registered for that content-type"_test = []
    {
        SerdeFormatRegistry registry;
        expect(registry.find("application/json") == nullptr);
    };

    "add_format registers a format findable by its content_type"_test = []
    {
        SerdeFormatRegistry registry;
        registry.add_format(std::make_shared<MockJsonFormat>());
        expect(registry.find("application/json") != nullptr);
        expect(registry.find("application/toml") == nullptr);
    };

    "add_format(nullptr) is a no-op"_test = []
    {
        SerdeFormatRegistry registry;
        registry.add_format(nullptr);
        expect(registry.get_formats().empty());
    };

    "get_formats reflects registration order and count"_test = []
    {
        SerdeFormatRegistry registry;
        registry.add_format(std::make_shared<MockJsonFormat>());
        expect(registry.get_formats().size() == 1);
    };

    "set_active/get_active round-trip, including clearing with nullptr"_test = []
    {
        SerdeFormatRegistry registry;
        expect(SerdeFormatRegistry::get_active() == nullptr);
        SerdeFormatRegistry::set_active(&registry);
        expect(SerdeFormatRegistry::get_active() == &registry);
        SerdeFormatRegistry::set_active(nullptr);
        expect(SerdeFormatRegistry::get_active() == nullptr);
    };
};

suite<"Ser"> ser_suite = []
{
    "serialize returns an error payload when no format is registered for accept"_test = []
    {
        SerdeFormatRegistry::set_active(nullptr);
        auto bytes = Ser::serialize("application/json", std::string{"hi"});
        std::string text(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        expect(text.contains("error"));
        expect(text.contains("no format plugin loaded"));
    };

    "serialize dispatches to the registered format's encode"_test = []
    {
        SerdeFormatRegistry registry;
        registry.add_format(std::make_shared<MockJsonFormat>());
        SerdeFormatRegistry::set_active(&registry);
        auto bytes = Ser::serialize("application/json", std::string{"hi"});
        std::string text(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        expect(text == R"("hi")");
        SerdeFormatRegistry::set_active(nullptr);
    };

    "deserialize returns an error when no format is registered for content_type"_test = []
    {
        SerdeFormatRegistry::set_active(nullptr);
        auto result = Ser::deserialize<std::string>("application/json", R"("hi")");
        expect(!result.has_value());
    };

    "deserialize dispatches to the registered format's decode"_test = []
    {
        SerdeFormatRegistry registry;
        registry.add_format(std::make_shared<MockJsonFormat>());
        SerdeFormatRegistry::set_active(&registry);
        auto result = Ser::deserialize<std::string>("application/json", R"("hi")");
        expect(result.has_value()) << fatal;
        expect(*result == "hi");
        SerdeFormatRegistry::set_active(nullptr);
    };

    "from_map populates fields present in the input map"_test = []
    {
        auto result = Ser::from_map<CoreTestRecord>({{"id", "abc123"}});
        expect(result.has_value()) << fatal;
        expect(result->get_id() == "abc123");
    };

    "from_map leaves fields absent from the input map at their default"_test = []
    {
        auto result = Ser::from_map<CoreTestRecord>({});
        expect(result.has_value()) << fatal;
        expect(result->get_id().empty());
    };

    "from_map fails on a field that doesn't parse as its declared type"_test = []
    {
        auto result = Ser::from_map<NumericTestRecord>({{"count", "not-a-number"}});
        expect(!result.has_value());
    };

    "from_value decodes a Value via rfl::from_generic"_test = []
    {
        Value value = rfl::json::read<rfl::Generic>(R"("hi")").value();
        auto result = Ser::from_value<std::string>(value);
        expect(result.has_value()) << fatal;
        expect(*result == "hi");
    };

    "encode_json JSON-encodes directly, bypassing the registry"_test = []
    {
        expect(Ser::encode_json(std::string{"hi"}) == R"("hi")");
    };

    "serialize_error wraps the message in a minimal error JSON payload"_test = []
    {
        auto bytes = Ser::serialize_error("application/json", "boom");
        std::string text(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        expect(text == R"({"error":"boom"})");
    };

    "serialize_raw passes text straight through to bytes"_test = []
    {
        auto bytes = Ser::serialize_raw("application/json", "raw-text");
        std::string text(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        expect(text == "raw-text");
    };
};

suite<"content_negotiation_middleware"> content_negotiation_middleware_suite = []
{
    "rejects a body with an unregistered Content-Type as 415"_test = []
    {
        SerdeFormatRegistry::set_active(nullptr);
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_header("content-type", "application/json");
        std::vector<std::byte> body{std::byte{1}};
        req.set_body(std::move(body));
        bool next_called = false;
        bool send_called = false;
        content_negotiation_middleware(
            req,
            res,
            [&](interfaces::io::IRequest&,
                interfaces::io::IResponse&,
                std::function<void()>) noexcept
            {
                next_called = true;
            },
            [&]
            {
                send_called = true;
            }
        );
        expect(!next_called);
        expect(send_called);
        expect(res.get_status() == interfaces::io::types::Status::UNSUPPORTED_MEDIA_TYPE);
    };

    "rejects an unregistered Accept as 406"_test = []
    {
        SerdeFormatRegistry registry;
        registry.add_format(std::make_shared<MockJsonFormat>());
        SerdeFormatRegistry::set_active(&registry);
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_header("accept", "application/toml");
        bool next_called = false;
        bool send_called = false;
        content_negotiation_middleware(
            req,
            res,
            [&](interfaces::io::IRequest&,
                interfaces::io::IResponse&,
                std::function<void()>) noexcept
            {
                next_called = true;
            },
            [&]
            {
                send_called = true;
            }
        );
        expect(!next_called);
        expect(send_called);
        expect(res.get_status() == interfaces::io::types::Status::NOT_ACCEPTABLE);
        SerdeFormatRegistry::set_active(nullptr);
    };

    "calls next when body-less request has a registered Accept"_test = []
    {
        SerdeFormatRegistry registry;
        registry.add_format(std::make_shared<MockJsonFormat>());
        SerdeFormatRegistry::set_active(&registry);
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_header("accept", "application/json");
        bool next_called = false;
        content_negotiation_middleware(
            req,
            res,
            [&](interfaces::io::IRequest&,
                interfaces::io::IResponse&,
                std::function<void()>) noexcept
            {
                next_called = true;
            },
            [] {}
        );
        expect(next_called);
        SerdeFormatRegistry::set_active(nullptr);
    };
};

} // namespace serde::tests
#endif
