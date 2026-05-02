#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_stream.h>
#include <ngx_stream_lua_api.h>
#include "ngx_stream_lua_upstream_state_module.h"


static ngx_stream_module_t  ngx_stream_lua_upstream_state_module_ctx = {
    NULL,                                          /* preconfiguration */
    NULL,                                          /* postconfiguration */

    NULL,                                          /* create main conf */
    NULL,                                          /* init main conf */

    NULL,                                          /* create srv conf */
    NULL                                           /* merge srv conf */
};


ngx_module_t  ngx_stream_lua_upstream_state_module = {
    NGX_MODULE_V1,
    &ngx_stream_lua_upstream_state_module_ctx,     /* module context */
    NULL,                                          /* module directives */
    NGX_STREAM_MODULE,                             /* module type */
    NULL,                                          /* init master */
    NULL,                                          /* init module */
    NULL,                                          /* init process */
    NULL,                                          /* init thread */
    NULL,                                          /* exit thread */
    NULL,                                          /* exit process */
    NULL,                                          /* exit master */
    NGX_MODULE_V1_PADDING
};


ngx_int_t
ngx_stream_lua_push_upstream_state(ngx_http_request_t *r,
    const u_char *addr, size_t addr_len, ngx_int_t status,
    ngx_msec_int_t connect_time_ms, ngx_msec_int_t first_byte_time_ms,
    ngx_msec_int_t start_msec, ngx_msec_int_t ssl_start_msec,
    ngx_msec_int_t send_start_msec, ngx_msec_int_t send_end_msec)
{
    ngx_stream_session_t         *s;
    ngx_stream_upstream_state_t  *state;
    ngx_stream_upstream_t        *u;
    ngx_str_t                    *peer;
    u_char                       *p;

    s = r->session;
    if (s == NULL) {
        return NGX_ERROR;
    }

    if (s->upstream_states == NULL) {
        s->upstream_states = ngx_array_create(s->connection->pool, 1,
                                              sizeof(ngx_stream_upstream_state_t));
        if (s->upstream_states == NULL) {
            return NGX_ERROR;
        }
    }

    state = ngx_array_push(s->upstream_states);
    if (state == NULL) {
        return NGX_ERROR;
    }

    ngx_memzero(state, sizeof(ngx_stream_upstream_state_t));

    /* unset timings render as "-" in logs, not "0.000" */

#if (NGX_RESTY_EXT)
    state->start_msec = (ngx_msec_t) -1;
#endif

    state->response_time = (ngx_msec_t) -1;
    state->connect_time = (ngx_msec_t) -1;
    state->first_byte_time = (ngx_msec_t) -1;

    if (addr != NULL && addr_len > 0) {
        peer = ngx_palloc(s->connection->pool, sizeof(ngx_str_t));
        if (peer == NULL) {
            return NGX_ERROR;
        }

        p = ngx_pnalloc(s->connection->pool, addr_len);
        if (p == NULL) {
            return NGX_ERROR;
        }

        ngx_memcpy(p, addr, addr_len);
        peer->data = p;
        peer->len = addr_len;
        state->peer = peer;
    }

#if (NGX_RESTY_EXT)
    state->status = status;

    if (start_msec >= 0) {
        state->start_msec = (ngx_msec_t) start_msec;
    }
#endif

    if (connect_time_ms >= 0) {
        state->connect_time = (ngx_msec_t) connect_time_ms;
    }

    if (first_byte_time_ms >= 0) {
        state->first_byte_time = (ngx_msec_t) first_byte_time_ms;
    }

    return NGX_OK;
}


ngx_int_t
ngx_stream_lua_update_upstream_state(ngx_stream_lua_request_t *r,
    ngx_msec_int_t session_time_ms, off_t bytes_sent, off_t bytes_received)
{
    ngx_stream_session_t         *s;
    ngx_stream_upstream_state_t  *state;
    
    s = r->session;
    if (s == NULL) {
        return NGX_ERROR;
    }

    if (s->upstream_states == NULL || s->upstream_states->nelts == 0) {
        return NGX_ERROR;
    }

    state = (ngx_stream_upstream_state_t *) s->upstream_states->elts
            + (s->upstream_states->nelts - 1);

    if (session_time_ms >= 0) {
        state->response_time = (ngx_msec_t) session_time_ms;
    }

    if (bytes_sent >= 0) {
        state->bytes_sent = bytes_sent;
    }

    if (bytes_received >= 0) {
        state->bytes_received = bytes_received;
    }

    return NGX_OK;
}

