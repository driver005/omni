export module blog_operations:posts_edit_model;

import std;

export namespace blog::handlers::operations::posts::edit::model {

struct RequestData { std::string id{}; std::string body{}; };

} // namespace
