module;
#include <congelado/plugin.h>
#include <memory>

export module blog_plugin:entry;

import std;
import blog_plugin:plugin;

namespace {

class BlogPluginEntry final : public congelado::Plugin {
public:
    BlogPluginEntry() : plugin_(std::make_unique<ghost::blog::BlogPlugin>()) {}

    [[nodiscard]] std::string_view get_name() const noexcept override {
        return plugin_->get_name();
    }

    [[nodiscard]] std::string_view get_version() const noexcept override {
        return plugin_->get_version();
    }

    [[nodiscard]] std::string_view get_unique_type() const noexcept override {
        return plugin_->get_unique_type();
    }

    [[nodiscard]] uint32_t capabilities() const noexcept override {
        return plugin_->capabilities();
    }

    [[nodiscard]] std::span<const std::string_view> get_requires() const noexcept override {
        return plugin_->get_requires();
    }

    void on_load(const CongeladoHostCallbacks& host, const CongeladoConfigView& cfg_view) override {
        plugin_->on_load(&host, &cfg_view);
    }

    void on_ready() noexcept override {
        plugin_->on_ready();
    }

    void on_unload() noexcept override {
        plugin_->on_unload();
    }

    void on_shutdown_requested() noexcept override {
        plugin_->on_shutdown_requested();
    }

    void* protocol_get() noexcept override {
        return plugin_->protocol_get();
    }

private:
    std::unique_ptr<ghost::blog::BlogPlugin> plugin_;
};

} // namespace

CONGELADO_PLUGIN(BlogPluginEntry)