export module blog_operations:posts_read_by_id_model;

import std;

export namespace blog::handlers::operations::posts::read_by_id::model {

struct RequestData { std::string id{}; std::string body{}; };

} // namespace
