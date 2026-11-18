export module blog_operations:posts_export_model;

import std;

export namespace blog::handlers::operations::posts::export_op::model {

struct RequestData { std::string id{}; std::string body{}; };

} // namespace
