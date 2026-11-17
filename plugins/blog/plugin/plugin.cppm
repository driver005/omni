module;
#include <string>
#include <memory>
#include <string_view>
#include <span>
#include <cstdint>

export module blog_plugin:plugin;

import std;
import blog_api:types;
import core_router;
import io_layer_http2;

export namespace ghost::blog {

/**
 * Blog Plugin - Main plugin class that integrates with the host
 * Uses congelado plugin system and HTTP/2 server from include/
 */
class BlogPlugin {
public:
    BlogPlugin();
    ~BlogPlugin();
    
    // Non-copyable, non-movable
    BlogPlugin(const BlogPlugin&) = delete;
    BlogPlugin& operator=(const BlogPlugin&) = delete;
    BlogPlugin(BlogPlugin&&) = delete;
    BlogPlugin& operator=(BlogPlugin&&) = delete;
    
    // Plugin lifecycle (congelado plugin interface)
    void on_load(const void* host_callbacks, const void* config_view);
    void on_ready() noexcept;
    void on_unload() noexcept;
    void on_shutdown_requested() noexcept;
    
    // Get the server for the host (protocol_get capability)
    void* protocol_get() noexcept;
    
    // Get plugin metadata
    std::string_view get_name() const noexcept { return "blog"; }
    std::string_view get_version() const noexcept { return "1.0.0"; }
    std::string_view get_unique_type() const noexcept { return "protocol"; }
    uint32_t capabilities() const noexcept { return 0x01; } // CONGELADO_CAP_PROTOCOL
    
    // Required dependencies
    std::span<const std::string_view> get_requires() const noexcept;
    
    // Configuration
    struct Config {
        std::string host = "0.0.0.0";
        uint16_t port = 8080;
        std::string cert_path = "";
        std::string key_path = "";
        std::string database_url = "";
        std::string log_level = "info";
        bool enable_tls = false;
        int worker_threads = 0;
    };
    
    const Config& get_config() const noexcept { return config_; }
    void set_config(const Config& config) { config_ = config; }
    
    // Status
    bool is_loaded() const noexcept { return loaded_; }
    bool is_ready() const noexcept { return ready_; }
    std::string get_status() const;

private:
    Config config_;
    std::unique_ptr<io::layer::http2::Server> server_;
    core::router::RouterContext<256, 64, 10>* router_ctx_{nullptr};
    bool loaded_{false};
    bool ready_{false};
    
    // Internal initialization
    bool initialize_database();
    bool initialize_router();
    bool initialize_server();
    void register_routes();
    void register_middleware();
    
    // Cleanup
    void cleanup();
};

} // namespace ghost::blog