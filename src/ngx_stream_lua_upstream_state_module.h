#ifndef _NGX_STREAM_LUA_UPSTREAM_STATE_MODULE_H_INCLUDED_
#define _NGX_STREAM_LUA_UPSTREAM_STATE_MODULE_H_INCLUDED_


#include <ngx_stream.h>
#include <ngx_stream_lua_api.h>


ngx_int_t ngx_stream_lua_push_upstream_state(ngx_stream_lua_request_t *r,
    const u_char *addr, size_t addr_len, ngx_int_t status,
    ngx_msec_int_t connect_time_ms, ngx_msec_int_t first_byte_time_ms,
    ngx_msec_int_t start_msec, ngx_msec_int_t ssl_start_msec,
    ngx_msec_int_t send_start_msec, ngx_msec_int_t send_end_msec);
ngx_int_t ngx_stream_lua_update_upstream_state(ngx_stream_lua_request_t *r,
    ngx_msec_int_t session_time_ms, off_t bytes_sent, off_t bytes_received);


#endif /* _NGX_STREAM_LUA_UPSTREAM_STATE_MODULE_H_INCLUDED_ */
