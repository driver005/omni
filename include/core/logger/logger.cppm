module;

#include <cstdio>

export module core_logger;

import std;
import interfaces;

export import :registry;
export import :facade;
export import :metrics_logger;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

#ifdef CONGELADO_TEST
namespace core::logger::tests {
using namespace boost::ut;

class LoggerFacadeFakeLogger : public interfaces::ILogger
{
public:
    [[nodiscard]] std::string_view get_name() const noexcept override
    {
        return "fake";
    }

    void write(interfaces::LogLevel level, std::string_view message) noexcept override
    {
        m_last_level = level;
        m_last_message = std::string{message};
        ++m_write_count;
    }

    void error(std::string_view message) noexcept override
    {
        m_last_message = std::string{message};
        ++m_error_count;
    }

    interfaces::LogLevel m_last_level{interfaces::LogLevel::DEBUG};
    std::string m_last_message;
    int m_write_count{0};
    int m_error_count{0};
};

suite<"logger facade"> facade_suite = []
{
    "info routes through write() with a |name| prefix"_test = []
    {
        auto* previous = LoggerRegistry::get_active();
        LoggerRegistry registry;
        auto logger = std::make_shared<LoggerFacadeFakeLogger>();
        registry.add_logger(logger);
        LoggerRegistry::set_active(&registry);

        core::logger::info("engine", "started with {} workers", 3);

        expect(logger->m_write_count == 1);
        expect(logger->m_error_count == 0);
        expect(logger->m_last_level == interfaces::LogLevel::INFO);
        expect(logger->m_last_message == "|engine| started with 3 workers");

        LoggerRegistry::set_active(previous);
    };

    "error routes through error(), not write()"_test = []
    {
        auto* previous = LoggerRegistry::get_active();
        LoggerRegistry registry;
        auto logger = std::make_shared<LoggerFacadeFakeLogger>();
        registry.add_logger(logger);
        LoggerRegistry::set_active(&registry);

        core::logger::error("engine", "boom");

        expect(logger->m_error_count == 1);
        expect(logger->m_write_count == 0);
        expect(logger->m_last_message == "|engine| boom");

        LoggerRegistry::set_active(previous);
    };

    "every registered logger receives the message, fan-out style"_test = []
    {
        auto* previous = LoggerRegistry::get_active();
        LoggerRegistry registry;
        auto first = std::make_shared<LoggerFacadeFakeLogger>();
        auto second = std::make_shared<LoggerFacadeFakeLogger>();
        registry.add_logger(first);
        registry.add_logger(second);
        LoggerRegistry::set_active(&registry);

        core::logger::warning("engine", "careful");

        expect(first->m_write_count == 1);
        expect(second->m_write_count == 1);

        LoggerRegistry::set_active(previous);
    };

    "logging with no active registry falls back to stderr without throwing"_test = []
    {
        auto* previous = LoggerRegistry::get_active();
        LoggerRegistry::set_active(nullptr);

        expect(nothrow(
            []
            {
                core::logger::debug("engine", "no sink around");
            }
        ));

        LoggerRegistry::set_active(previous);
    };

    "unnamed::info skips the |name| prefix"_test = []
    {
        auto* previous = LoggerRegistry::get_active();
        LoggerRegistry registry;
        auto logger = std::make_shared<LoggerFacadeFakeLogger>();
        registry.add_logger(logger);
        LoggerRegistry::set_active(&registry);

        core::logger::unnamed::info("plain message");

        expect(logger->m_last_message == "plain message");

        LoggerRegistry::set_active(previous);
    };
};

} // namespace core::logger::tests
#endif
