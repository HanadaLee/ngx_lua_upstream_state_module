#ifndef _NGX_HTTP_LUA_UPSTREAM_STATE_MODULE_H_INCLUDED_
#define _NGX_HTTP_LUA_UPSTREAM_STATE_MODULE_H_INCLUDED_


#include <ngx_http.h>


ngx_int_t ngx_http_lua_push_upstream_state(ngx_http_request_t *r,
    const u_char *addr, size_t addr_len, ngx_int_t status,
    ngx_msec_int_t connect_time_ms, ngx_msec_int_t header_time_ms,
    ngx_msec_int_t start_msec, ngx_msec_int_t ssl_start_msec,
    ngx_msec_int_t send_start_msec, ngx_msec_int_t send_end_msec);
ngx_int_t ngx_http_lua_update_upstream_state(ngx_http_request_t *r,
    ngx_msec_int_t response_time_ms, off_t response_length,
    off_t bytes_received);
ngx_int_t ngx_http_lua_set_upstream_scheme(ngx_http_request_t *r,
    const u_char *scheme, size_t scheme_len);
ngx_int_t ngx_http_lua_set_upstream_request_method(ngx_http_request_t *r,
    const u_char *method, size_t method_len);


#endif /* _NGX_HTTP_LUA_UPSTREAM_STATE_MODULE_H_INCLUDED_ */