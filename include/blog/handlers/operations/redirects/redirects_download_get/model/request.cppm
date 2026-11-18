export module blog_operations:redirects_redirects_download_get_model;

import std;

export namespace blog::handlers::operations::redirects::redirects_download_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
