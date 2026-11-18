export module blog_operations:posts_upload_model;

import std;

export namespace blog::handlers::operations::posts::upload::model {

struct RequestData { std::string id{}; std::string body{}; };

} // namespace
