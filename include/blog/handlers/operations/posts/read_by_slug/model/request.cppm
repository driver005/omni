export module blog_operations:posts_read_by_slug_model;

import std;

export namespace blog::handlers::operations::posts::read_by_slug::model {

struct RequestData { std::string id{}; std::string body{}; };

} // namespace
