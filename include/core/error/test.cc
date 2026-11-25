import std;
import core_error;
import core_serde;
import interfaces;

int main()
{
    const auto error = core::error::AppErrorBuilder{}.operator()(
        core::error::AppErrorKind::INVALID_JSON,
        "invalid JSON"
    );
    const auto text = serde::Ser::encode_json(error);
    const auto expected = std::string{R"({"error":"INVALID_JSON","message":"invalid JSON"})"};
    const auto status_ok = error.get_status() == interfaces::io::types::Status::BAD_REQUEST;
    return text == expected && status_ok ? 0 : 1;
}
