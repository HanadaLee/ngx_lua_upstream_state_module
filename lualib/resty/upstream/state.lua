local ffi = require("ffi")
local base = require("resty.core.base")
local get_request = base.get_request
local C = ffi.C
local ngx = ngx
local ngx_var = ngx.var
local NGX_ERROR = ngx.ERROR
local subsystem = ngx.config.subsystem
local is_http = subsystem == "http"
local is_stream = subsystem == "stream"


local ngx_lua_push_upstream_state
local ngx_lua_update_upstream_state
local ngx_lua_set_upstream_scheme
local ngx_lua_set_upstream_method
if is_http then
    ffi.cdef[[
    typedef intptr_t        ngx_int_t;
    ngx_int_t ngx_http_lua_push_upstream_state(ngx_http_request_t *r,
        const char *addr, size_t addr_len, ngx_int_t status,
        ngx_int_t connect_time_ms, ngx_int_t header_time_ms,
        ngx_int_t start_msec, ngx_int_t ssl_start_msec,
        ngx_int_t send_start_msec, ngx_int_t send_end_msec,
        ngx_int_t response_time_ms, intptr_t response_length,
        intptr_t bytes_received, intptr_t bytes_sent);
    ngx_int_t ngx_http_lua_update_upstream_state(ngx_http_request_t *r,
        const char *addr, size_t addr_len, ngx_int_t status,
        ngx_int_t connect_time_ms, ngx_int_t header_time_ms,
        ngx_int_t start_msec, ngx_int_t ssl_start_msec,
        ngx_int_t send_start_msec, ngx_int_t send_end_msec,
        ngx_int_t response_time_ms, intptr_t response_length,
        intptr_t bytes_received, intptr_t bytes_sent);
    ngx_int_t ngx_http_lua_set_upstream_scheme(ngx_http_request_t *r,
        const char *scheme, size_t scheme_len);
    ngx_int_t ngx_http_lua_set_upstream_method(ngx_http_request_t *r,
        const char *method, size_t method_len);
    ]]

    ngx_lua_push_upstream_state = C.ngx_http_lua_push_upstream_state
    ngx_lua_update_upstream_state = C.ngx_http_lua_update_upstream_state
    ngx_lua_set_upstream_scheme = C.ngx_http_lua_set_upstream_scheme
    ngx_lua_set_upstream_method = C.ngx_http_lua_set_upstream_method

elseif is_stream then
    ffi.cdef[[
    typedef intptr_t        ngx_int_t;
    ngx_int_t ngx_stream_lua_push_upstream_state(ngx_stream_lua_request_t *r,
        const char *addr, size_t addr_len, ngx_int_t status,
        ngx_int_t connect_time_ms, ngx_int_t first_byte_time_ms,
        ngx_int_t start_msec, ngx_int_t ssl_start_msec,
        ngx_int_t send_start_msec, ngx_int_t send_end_msec,
        ngx_int_t session_time_ms, intptr_t response_length,
        intptr_t bytes_received, intptr_t bytes_sent);
    ngx_int_t ngx_stream_lua_update_upstream_state(ngx_stream_lua_request_t *r,
        const char *addr, size_t addr_len, ngx_int_t status,
        ngx_int_t connect_time_ms, ngx_int_t first_byte_time_ms,
        ngx_int_t start_msec, ngx_int_t ssl_start_msec,
        ngx_int_t send_start_msec, ngx_int_t send_end_msec,
        ngx_int_t session_time_ms, intptr_t response_length,
        intptr_t bytes_received, intptr_t bytes_sent);
    ]]

    ngx_lua_push_upstream_state = C.ngx_stream_lua_push_upstream_state
    ngx_lua_update_upstream_state = C.ngx_stream_lua_update_upstream_state
    -- not supported in stream subsystem
    ngx_lua_set_upstream_scheme = nil
    ngx_lua_set_upstream_method = nil
end


local _M = {}


local function _call_upstream_state(c_func, opts, action)
    if type(opts) ~= "table" then
        return nil, "opts must be a table"
    end

    local r = get_request()
    if not r then
        return nil, "no request found"
    end

    local addr = opts.addr
    if addr ~= nil and type(addr) ~= "string" then
        return nil, "addr must be a string"
    end

    local addr_len = addr and #addr or 0
    local header_time_ms = is_http and opts.header_time or opts.first_byte_time
    local response_time = opts.response_time or opts.session_time

    local rc = c_func(
        r,
        addr, addr_len,
        opts.status or 0,
        opts.connect_time or -1,
        header_time_ms or -1,
        opts.start_msec or -1,
        opts.ssl_start_msec or -1,
        opts.send_start_msec or -1,
        opts.send_end_msec or -1,
        response_time or -1,
        opts.response_length or -1,
        opts.bytes_received or -1,
        opts.bytes_sent or -1)

    if rc == NGX_ERROR then
        return nil, "error while " .. action .. " upstream state"
    end

    return true
end


function _M.push_upstream_state(opts)
    return _call_upstream_state(ngx_lua_push_upstream_state, opts, "pushing")
end


function _M.update_upstream_state(opts)
    return _call_upstream_state(ngx_lua_update_upstream_state, opts, "updating")
end


-- must add config 'set $upstream_scheme "";' before use this function in stream subsystem
function _M.set_upstream_scheme(scheme)
    if type(scheme) ~= "string" then
        return nil, "scheme must be a string"
    end

    if is_stream then
        ngx_var.upstream_scheme = scheme
        return true
    end

    local r = get_request()
    if not r then
        return nil, "no request found"
    end

    if not ngx_lua_set_upstream_scheme then
        return nil, "not supported in stream subsystem"
    end

    local rc = ngx_lua_set_upstream_scheme(r, scheme, #scheme)

    if rc == NGX_ERROR then
        return nil, "error while setting upstream scheme"
    end

    return true
end


-- must add config 'set $upstream_method "";' before use this function in stream subsystem
function _M.set_upstream_method(method)
    if type(method) ~= "string" then
        return nil, "method must be a string"
    end

    if is_stream then
        ngx_var.upstream_method = method
        return true
    end

    local r = get_request()
    if not r then
        return nil, "no request found"
    end

    if not ngx_lua_set_upstream_method then
        return nil, "not supported in stream subsystem"
    end

    local rc = ngx_lua_set_upstream_method(r, method, #method)

    if rc == NGX_ERROR then
        return nil, "error while setting upstream method"
    end

    return true
end


return _M