export module interfaces:response_sink;

import std;
import :io_response;

export namespace interfaces::io {

/**
 * @brief Bundles an IResponse with the callback that flushes it, so handlers can mutate and
 * send a response without threading `(res, send)` through every call.
 */
class ResponseSink
{
public:
    ResponseSink(IResponse& res, std::function<void()> send) noexcept :
        m_res{res}, m_send{std::move(send)}
    {
    }

    void set_status(types::Status status)
    {
        m_res.set_status(status);
    }

    void set_body(std::vector<std::byte> body)
    {
        m_res.set_body(std::move(body));
    }

    void set_header(types::Token token, std::string_view value)
    {
        m_res.set_header(token, value);
    }

    void add_header(std::string_view name, std::string_view value)
    {
        m_res.add_header(name, value);
    }

    void send()
    {
        m_send();
    }

private:
    IResponse& m_res;
    std::function<void()> m_send;
};

} // namespace interfaces::io
