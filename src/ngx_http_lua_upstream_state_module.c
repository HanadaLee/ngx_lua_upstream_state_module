#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_http_lua_upstream_state_module.h"


static ngx_http_module_t  ngx_http_lua_upstream_state_module_ctx = {
    NULL,                                          /* preconfiguration */
    NULL,                                          /* postconfiguration */

    NULL,                                          /* create main conf */
    NULL,                                          /* init main conf */

    NULL,                                          /* create srv conf */
    NULL,                                          /* merge srv conf */

    NULL,                                          /* create loc conf */
    NULL                                           /* merge loc conf */
};


ngx_module_t  ngx_http_lua_upstream_state_module = {
    NGX_MODULE_V1,
    &ngx_http_lua_upstream_state_module_ctx,       /* module context */
    NULL,                                          /* module directives */
    NGX_HTTP_MODULE,                               /* module type */
    NULL,                                          /* init master */
    NULL,                                          /* init module */
    NULL,                                          /* init process */
    NULL,                                          /* init thread */
    NULL,                                          /* exit thread */
    NULL,                                          /* exit process */
    NULL,                                          /* exit master */
    NGX_MODULE_V1_PADDING
};


static void
ngx_http_lua_set_upstream_state_fields(ngx_http_upstream_state_t *state,
    ngx_msec_int_t start_msec, ngx_msec_int_t ssl_start_msec,
    ngx_msec_int_t send_start_msec, ngx_msec_int_t send_end_msec,
    ngx_msec_int_t connect_time_ms, ngx_msec_int_t header_time_ms,
    ngx_msec_int_t response_time_ms, off_t response_length,
    off_t bytes_received, off_t bytes_sent)
{
#if (NGX_RESTY_EXT)
    if (start_msec >= 0) {
        state->start_msec = (ngx_msec_t) start_msec;
    }

#if (NGX_HTTP_SSL)
    if (ssl_start_msec >= 0) {
        state->ssl_start_msec = (ngx_msec_t) ssl_start_msec;
    }
#endif

    if (send_start_msec >= 0) {
        state->send_start_msec = (ngx_msec_t) send_start_msec;
    }

    if (send_end_msec >= 0) {
        state->send_end_msec = (ngx_msec_t) send_end_msec;
    }
#endif

    if (connect_time_ms >= 0) {
        state->connect_time = (ngx_msec_t) connect_time_ms;
    }

    if (header_time_ms >= 0) {
        state->header_time = (ngx_msec_t) header_time_ms;
    }

    if (response_time_ms >= 0) {
        state->response_time = (ngx_msec_t) response_time_ms;
    }

    if (response_length >= 0) {
        state->response_length = response_length;
    }

    if (bytes_received >= 0) {
        state->bytes_received = bytes_received;
    }

    if (bytes_sent >= 0) {
        state->bytes_sent = bytes_sent;
    }
}


ngx_int_t
ngx_http_lua_push_upstream_state(ngx_http_request_t *r,
    const u_char *addr, size_t addr_len, ngx_int_t status,
    ngx_msec_int_t connect_time_ms, ngx_msec_int_t header_time_ms,
    ngx_msec_int_t start_msec, ngx_msec_int_t ssl_start_msec,
    ngx_msec_int_t send_start_msec, ngx_msec_int_t send_end_msec,
    ngx_msec_int_t response_time_ms, off_t response_length,
    off_t bytes_received, off_t bytes_sent)
{
    ngx_http_upstream_state_t  *state;
    ngx_str_t                  *peer;
    u_char                     *p;

    if (r->upstream_states == NULL) {
        r->upstream_states = ngx_array_create(r->pool, 1,
                                            sizeof(ngx_http_upstream_state_t));
        if (r->upstream_states == NULL) {
            return NGX_ERROR;
        }
    }

    state = ngx_array_push(r->upstream_states);
    if (state == NULL) {
        return NGX_ERROR;
    }

    ngx_memzero(state, sizeof(ngx_http_upstream_state_t));

    /* unset timings render as "-" in logs, not "0.000" */
#if (NGX_RESTY_EXT)
    state->start_msec = (ngx_msec_t) -1;

#if (NGX_HTTP_SSL)
    state->ssl_start_msec = (ngx_msec_t) -1;
#endif

    state->send_start_msec = (ngx_msec_t) -1;
    state->send_end_msec = (ngx_msec_t) -1;
#endif

    state->response_time = (ngx_msec_t) -1;
    state->connect_time = (ngx_msec_t) -1;
    state->header_time = (ngx_msec_t) -1;

    if (addr != NULL && addr_len > 0) {
        peer = ngx_palloc(r->pool, sizeof(ngx_str_t));
        if (peer == NULL) {
            return NGX_ERROR;
        }

        p = ngx_pnalloc(r->pool, addr_len);
        if (p == NULL) {
            return NGX_ERROR;
        }

        ngx_memcpy(p, addr, addr_len);
        peer->data = p;
        peer->len = addr_len;
        state->peer = peer;
    }

    state->status = status;

    ngx_http_lua_set_upstream_state_fields(state,
        start_msec, ssl_start_msec, send_start_msec, send_end_msec,
        connect_time_ms, header_time_ms, response_time_ms,
        response_length, bytes_received, bytes_sent);

    return NGX_OK;
}


ngx_int_t
ngx_http_lua_update_upstream_state(ngx_http_request_t *r,
    const u_char *addr, size_t addr_len, ngx_int_t status,
    ngx_msec_int_t connect_time_ms, ngx_msec_int_t header_time_ms,
    ngx_msec_int_t start_msec, ngx_msec_int_t ssl_start_msec,
    ngx_msec_int_t send_start_msec, ngx_msec_int_t send_end_msec,
    ngx_msec_int_t response_time_ms, off_t response_length,
    off_t bytes_received, off_t bytes_sent)
{
    ngx_http_upstream_state_t  *state;
    ngx_str_t                  *peer;
    u_char                     *p;

    if (r->upstream_states == NULL || r->upstream_states->nelts == 0) {
        return NGX_ERROR;
    }

    state = (ngx_http_upstream_state_t *) r->upstream_states->elts
            + (r->upstream_states->nelts - 1);

    if (addr != NULL && addr_len > 0) {
        if (state->peer == NULL
            || state->peer->len != addr_len
            || ngx_memcmp(state->peer->data, addr, addr_len) != 0)
        {
            peer = ngx_palloc(r->pool, sizeof(ngx_str_t));
            if (peer == NULL) {
                return NGX_ERROR;
            }

            p = ngx_pnalloc(r->pool, addr_len);
            if (p == NULL) {
                return NGX_ERROR;
            }

            ngx_memcpy(p, addr, addr_len);
            peer->data = p;
            peer->len = addr_len;
            state->peer = peer;
        }
    }

    if (status != 0) {
        state->status = status;
    }

    ngx_http_lua_set_upstream_state_fields(state,
        start_msec, ssl_start_msec, send_start_msec, send_end_msec,
        connect_time_ms, header_time_ms, response_time_ms,
        response_length, bytes_received, bytes_sent);

    return NGX_OK;
}


#if (NGX_RESTY_EXT)

ngx_int_t
ngx_http_lua_set_upstream_scheme(ngx_http_request_t *r,
    const u_char *scheme, size_t scheme_len)
{
    u_char                *p;
    ngx_http_upstream_t   *u;

    if (r->upstream == NULL) {
        if (ngx_http_upstream_create(r) != NGX_OK) {
            return NGX_ERROR;
        }
    }

    u = r->upstream;

    /* scheme + "://" */
    p = ngx_pnalloc(r->pool, scheme_len + 3);
    if (p == NULL) {
        return NGX_ERROR;
    }

    ngx_memcpy(p, scheme, scheme_len);
    ngx_memcpy(p + scheme_len, "://", 3);

    u->schema.data = p;
    u->schema.len = scheme_len + 3;

    return NGX_OK;
}


ngx_int_t
ngx_http_lua_set_upstream_method(ngx_http_request_t *r,
    const u_char *method, size_t method_len)
{
    ngx_http_upstream_t  *u;

    if (r->upstream == NULL) {
        if (ngx_http_upstream_create(r) != NGX_OK) {
            return NGX_ERROR;
        }
    }

    u = r->upstream;

    u->method.data = ngx_pnalloc(r->pool, method_len);
    if (u->method.data == NULL) {
        return NGX_ERROR;
    }

    ngx_memcpy(u->method.data, method, method_len);
    u->method.len = method_len;

    return NGX_OK;
}

#else

ngx_int_t
ngx_http_lua_set_upstream_scheme(ngx_http_request_t *r,
    const u_char *scheme, size_t scheme_len)
{
    return NGX_OK;
}


ngx_int_t
ngx_http_lua_set_upstream_request_method(ngx_http_request_t *r,
    const u_char *method, size_t method_len)
{
    return NGX_OK;
}

#endif