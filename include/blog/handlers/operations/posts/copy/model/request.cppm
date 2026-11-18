export module blog_operations:posts_copy_model;

import std;

export namespace blog::handlers::operations::posts::copy::model {

struct RequestData { std::string id{}; std::string body{}; };

} // namespace
