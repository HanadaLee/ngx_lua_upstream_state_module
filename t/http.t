# vim:set ft= ts=4 sw=4 et:

use Cwd qw(abs_path);
use FindBin;

BEGIN {
    $ENV{TEST_NGINX_MODULE_LUALIB} =
        abs_path("$FindBin::Bin/../lualib");
    $ENV{TEST_NGINX_RESTY_LUALIB} ||= "/usr/local/openresty/lualib";
    $ENV{TEST_NGINX_INIT_BY_LUA} =
        "package.path = '$ENV{TEST_NGINX_RESTY_LUALIB}/?.lua;' "
        . ".. (package.path or ''); require 'resty.core'";
}

use Test::Nginx::Socket::Lua;

repeat_each(1);
plan tests => repeat_each() * blocks() * 3;

no_long_string();
run_tests();

__DATA__

=== TEST 1: push and update HTTP upstream state
--- http_config
    lua_package_path '$TEST_NGINX_MODULE_LUALIB/?.lua;$TEST_NGINX_RESTY_LUALIB/?.lua;;';
--- config
    location = /t {
        content_by_lua_block {
            local state = require "resty.upstream.state"

            assert(state.push_upstream_state({
                addr = "127.0.0.1:8080",
                status = 200,
                connect_time = 12,
                header_time = 23,
                response_time = 34,
                response_length = 456,
                bytes_received = 567,
                bytes_sent = 78,
            }))

            assert(state.update_upstream_state({
                addr = "127.0.0.2:8081",
                status = 201,
                response_time = 45,
                response_length = 678,
                bytes_received = 789,
                bytes_sent = 89,
            }))

            ngx.say(ngx.var.upstream_addr)
            ngx.say(ngx.var.upstream_status)
            ngx.say(ngx.var.upstream_connect_time)
            ngx.say(ngx.var.upstream_header_time)
            ngx.say(ngx.var.upstream_response_time)
            ngx.say(ngx.var.upstream_response_length)
            ngx.say(ngx.var.upstream_bytes_received)
            ngx.say(ngx.var.upstream_bytes_sent)
        }
    }
--- request
GET /t
--- response_body
127.0.0.2:8081
201
0.012
0.023
0.045
678
789
89
--- no_error_log
[error]



=== TEST 2: multiple HTTP upstream attempts retain their order
--- http_config
    lua_package_path '$TEST_NGINX_MODULE_LUALIB/?.lua;$TEST_NGINX_RESTY_LUALIB/?.lua;;';
--- config
    location = /t {
        content_by_lua_block {
            local state = require "resty.upstream.state"

            assert(state.push_upstream_state({
                addr = "127.0.0.1:8001",
                status = 502,
                response_time = 10,
            }))
            assert(state.push_upstream_state({
                addr = "127.0.0.1:8002",
                status = 200,
                response_time = 20,
            }))

            ngx.say(ngx.var.upstream_addr)
            ngx.say(ngx.var.upstream_status)
            ngx.say(ngx.var.upstream_response_time)
        }
    }
--- request
GET /t
--- response_body
127.0.0.1:8001, 127.0.0.1:8002
502, 200
0.010, 0.020
--- no_error_log
[error]



=== TEST 3: HTTP setters and input validation
--- http_config
    lua_package_path '$TEST_NGINX_MODULE_LUALIB/?.lua;$TEST_NGINX_RESTY_LUALIB/?.lua;;';
--- config
    location = /t {
        content_by_lua_block {
            local state = require "resty.upstream.state"
            local ok_update, update_err = state.update_upstream_state({})
            local ok_opts, opts_err = state.push_upstream_state("bad")
            local ok_addr, addr_err = state.push_upstream_state({ addr = 1 })

            ngx.say(ok_update == nil)
            ngx.say(update_err)
            ngx.say(ok_opts == nil)
            ngx.say(opts_err)
            ngx.say(ok_addr == nil)
            ngx.say(addr_err)
            ngx.say(assert(state.set_upstream_scheme("https")))
            ngx.say(assert(state.set_upstream_method("POST")))
            ngx.say(ngx.var.upstream_method)
        }
    }
--- request
GET /t
--- response_body
true
error while updating upstream state
true
opts must be a table
true
addr must be a string
true
true
POST
--- no_error_log
[error]
