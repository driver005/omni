export module blog_operations:posts_bulk_edit_model;

import std;

export namespace blog::handlers::operations::posts::bulk_edit::model {

struct RequestData { std::string id{}; std::string body{}; };

} // namespace
