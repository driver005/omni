export module blog_operations:exports_exports_download_get_model;

import std;

export namespace blog::handlers::operations::exports::exports_download_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
