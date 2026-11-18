export module blog_operations:posts_bulk_destroy_model;

import std;

export namespace blog::handlers::operations::posts::bulk_destroy::model {

struct RequestData { std::string id{}; std::string body{}; };

} // namespace
