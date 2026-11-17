# Ghost Blog Plugin

A full-featured blog plugin for Ghost built with C++20 modules, using the core router abstraction and database connector from the main include directory.

## Architecture

```
plugins/blog/
├── api/                 # Blog API module
│   ├── blog_api.cppm           # Main API implementation
│   ├── blog_types.cppm         # Type definitions (Post, Tag, Author, etc.)
│   ├── post_controller.cppm    # HTTP route handlers using core router
│   ├── json_serializer.cppm    # JSON serialization utilities
│   └── BUILD.bazel
├── database/            # Database abstraction module
│   ├── database.cppm           # Database interface and implementations
│   ├── post_repository.cppm    # Post data access
│   ├── tag_repository.cppm     # Tag data access
│   ├── author_repository.cppm  # Author data access
│   ├── migration_manager.cppm  # Schema migrations
│   └── BUILD.bazel
├── plugin/              # Congelado plugin module
│   ├── plugin.cppm             # Main plugin class
│   ├── plugin_routes.cppm      # Route registration
│   ├── plugin_entry.cppm       # Plugin entry point
│   ├── plugin.yaml             # Plugin manifest
│   └── BUILD.bazel
└── BUILD.bazel
```

## Key Features

- **C++20 Modules**: Uses `.cppm` module files throughout
- **Core Router Integration**: Uses `core::router::RouterContext` and `RouteBuilder` from `include/core/router/`
- **Database Abstraction**: Uses `connector::Sql` from `include/connector/` for type-safe queries
- **HTTP/2 Server**: Uses `io::layer::http2::Server` from `include/io/layer/http2/`
- **Congelado Plugin**: Implements the congelado plugin interface for dynamic loading

## API Endpoints

### Posts
- `GET /api/v1/posts` - List posts (with pagination, filtering)
- `POST /api/v1/posts` - Create post
- `GET /api/v1/posts/{id}` - Get post by ID
- `GET /api/v1/posts/slug/{slug}` - Get post by slug
- `PUT /api/v1/posts/{id}` - Update post
- `DELETE /api/v1/posts/{id}` - Delete post
- `POST /api/v1/posts/{id}/publish` - Publish post
- `POST /api/v1/posts/{id}/unpublish` - Unpublish post
- `POST /api/v1/posts/{id}/schedule` - Schedule post

### Tags
- `GET /api/v1/tags` - List tags
- `POST /api/v1/tags` - Create tag
- `GET /api/v1/tags/{id}` - Get tag
- `PUT /api/v1/tags/{id}` - Update tag
- `DELETE /api/v1/tags/{id}` - Delete tag
- `GET /api/v1/tags/{id}/posts` - Get posts by tag

### Authors
- `GET /api/v1/authors` - List authors
- `GET /api/v1/authors/{id}` - Get author
- `GET /api/v1/authors/{id}/posts` - Get posts by author

### Search & Stats
- `GET /api/v1/search?q={query}` - Search posts
- `GET /api/v1/stats` - Get blog statistics

### Health
- `GET /health` - Health check endpoint

## Building

```bash
# Build the blog API
bazel build //plugins/blog/api:blog_api

# Build the database module
bazel build //plugins/blog/database:blog_database

# Build the plugin
bazel build //plugins/blog/plugin:blog_plugin

# Run tests
bazel test //plugins/blog/api:blog_api_test
bazel test //plugins/blog/database:blog_database_test
bazel test //plugins/blog/plugin:blog_plugin_test
```

## Dependencies

- `//include/core/router` - Core router abstraction
- `//include/connector` - Database connector
- `//include/io/layer/http2` - HTTP/2 server
- `//include/interfaces` - IO interfaces
- `//include/shared` - Shared utilities
- `@nlohmann_json` - JSON library
- `@boost` - Boost.Asio, System, Thread
- `@congelado` - Plugin framework

## Configuration

The plugin accepts the following configuration:

```yaml
host: "0.0.0.0"
port: 8080
cert_path: ""
key_path: ""
database_url: "postgresql://user:pass@localhost/ghost_blog"
log_level: "info"
enable_tls: false
worker_threads: 0
```

## Database Schema

The plugin creates the following tables:
- `posts` - Blog posts
- `tags` - Tags
- `authors` - Authors
- `post_tags` - Many-to-many relationship
- `post_revisions` - Post revisions
- `schema_migrations` - Migration tracking

Migrations are managed by `MigrationManager` and run automatically on startup.