export module blog_operations:files_files_upload_post_model;

import std;

export namespace blog::handlers::operations::files::files_upload_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
