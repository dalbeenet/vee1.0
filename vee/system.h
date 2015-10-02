#ifndef _VEE_SYSTEM_H_
#define _VEE_SYSTEM_H_

#include <vee/exception.h>
#include <vee/string.h>

namespace vee {
namespace system {

enum class error_code: int
{
    success = 0,
    stream_already_connected, //TODO: NET �������̽����� ����
    stream_connection_failure,
    server_accept_failure,
    stream_send_failure,
    stream_read_failure,
    stream_disconnected_by_host,
    closed_stream,
    rfc6455_handshake_failure,
    rfc6455_websocket_heartbeat,
    win32_busy_named_pipe,
    win32_close_handle_failure,
    invalid_parameter,
    buffer_is_too_small,
};

struct operation_result
{
    error_code error;
    string     desc;
};

} // namespace system
} // namespace vee

#endif // !_VEE_SYSTEM_H_
