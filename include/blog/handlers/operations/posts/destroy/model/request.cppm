export module blog_operations:posts_destroy_model;

import std;

export namespace blog::handlers::operations::posts::destroy::model {

struct RequestData { std::string id{}; std::string body{}; };

} // namespace
