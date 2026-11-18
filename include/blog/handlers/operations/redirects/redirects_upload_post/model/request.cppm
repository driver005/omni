export module blog_operations:redirects_redirects_upload_post_model;

import std;

export namespace blog::handlers::operations::redirects::redirects_upload_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
