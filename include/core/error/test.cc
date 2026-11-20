import std;
import core_error;
import core_serde;

int main()
{
    const auto error = core::error::AppErrorBuilder{}.operator()(
        core::error::AppErrorKind::INVALID_JSON,
        "invalid JSON"
    );
    const auto text = serde::Ser::encode_json(error);
    const auto expected = std::string{R"({"error":"INVALID_JSON","message":"invalid JSON"})"};
    return text == expected ? 0 : 1;
}
