Name
====
ngx_lua_upstream_state_module — Manage upstream state entries for Lua modules.


Table of Contents
=================

- [Name](#name)
- [Table of Contents](#table-of-contents)
- [Description](#description)
- [Install](#install)
- [Lua Methods](#lua-methods)
  - [push\_upstream\_state](#push_upstream_state)
  - [update\_upstream\_state](#update_upstream_state)
  - [set\_upstream\_scheme](#set_upstream_scheme)
  - [set\_upstream\_method](#set_upstream_method)
- [License](#license)


Description
===========
This module provides C functions for managing upstream peer state entries in Nginx's `upstream_states` array and some members of `upstream`. It is designed to be called from LuaJIT FFI by lua modules to populate timing, status, and peer address information for each upstream attempt. When using Lua consocket instead of nginx upstream for upstream processing with this module, Lua modules can populate the standard `$upstream_*` variables (e.g. `$upstream_addr`, `$upstream_status`, `$upstream_response_time`).

It supports both `http` and `stream` subsystems.

The original work of this patch came from: https://github.com/api7/apisix-nginx-module/commit/555c64492b4f7cd8442f79c87456c7f1fb675f73
Some fields are only available after using the nginx core patch: https://git.hanada.info/hanada/openresty/-/tree/main/patches. In the following description, it will be referred to as `nginx core patch`.

Install
=======
Some fields are only available after applying the nginx core patch:

```shell
cd /path/to/nginx-1.xx.x
patch -p1 < nginx-ext_1.xx.x+.patch
```

The patch is from https://git.hanada.info/hanada/openresty/-/tree/main/patches

This module can be installed just like any ordinary Nginx C module, using the `--add-module` configuration option:

```shell
./configure --add-module=/path/to/ngx_lua_upstream_state_module \
            ...
```
Copy the Lua library file into your OpenResty/Nginx Lua path:

```shell
cp lualib/resty/upstream/state.lua /path/to/openresty/lualib/resty/upstream/
```

Then require it in your Lua code:

```lua
local upstream_state = require("resty.upstream.state")
```


Lua Methods
=======

push\_upstream\_state
---------------------
**syntax:** *push_upstream_state(opts)*

**subsystems:** *http*, *stream*

Creates and pushes a new upstream state entry into the request's (or session's) `upstream_states` array. Call this at the beginning of each upstream attempt.

The `opts` parameter is a table with the following fields:

- `addr`: peer address string (e.g. `"192.168.1.1:8080"`).
- `status`: response status. Only effective in http subsystem or after applying the core patch. A value of `0` in `update` means "do not overwrite". Defaults to `0`.
- `connect_time`: time spent establishing the TCP/TLS connection, in milliseconds. Defaults to `-1`.
- `header_time` (http): time to receive header of the response, in milliseconds. Defaults to `-1`.
- `first_byte_time` (stream): time to receive the first byte of the connection, in milliseconds. Defaults to `-1`.
- `start_msec`: absolute start time in milliseconds (monotonic). Only effective after applying the core patch. Defaults to `-1`.
- `ssl_start_msec` (http): absolute time when SSL handshake started (monotonic). Only effective after applying the core patch. Defaults to `-1`.
- `send_start_msec` (http): absolute time when request send started (monotonic). Only effective after applying the core patch. Defaults to `-1`.
- `send_end_msec` (http): absolute time when request send finished (monotonic). Only effective after applying the core patch. Defaults to `-1`.
- `response_time` (http): total response time in milliseconds. Defaults to `-1`.
- `session_time` (stream): total session time in milliseconds. Defaults to `-1`.
- `response_length` (http): response body length in bytes. Defaults to `-1`.
- `bytes_received`: bytes received from the upstream. Defaults to `-1`.
- `bytes_sent`: bytes sent to the upstream. Defaults to `-1`.

All timing values default to `-1` (unknown), which renders as `"-"` in Nginx log templates.

All absolute time fields (`start_msec`, `ssl_start_msec`, `send_start_msec`, `send_end_msec`) use monotonic time — milliseconds elapsed since system boot, not epoch timestamps. Use `monotonic_msec()` from [lua-resty-core's resty.core.time](https://github.com/openresty/lua-resty-core/blob/master/lib/resty/core/time.md) to obtain this value.

[Back to TOC](#table-of-contents)


update\_upstream\_state
-----------------------
**syntax:** *update_upstream_state(opts)*

**subsystems:** *http*, *stream*

Updates the last upstream state entry. Any field can be set or updated at this stage. Call this after the upstream response has been fully received.

The `opts` parameter is a table with the same fields as `push_upstream_state`. Fields passed as `-1` (or empty `addr` / `status = 0`) are left unchanged.

[Back to TOC](#table-of-contents)


set\_upstream\_scheme
------------------------
**syntax:** *set_upstream_scheme(scheme)*

**subsystems:** *http*, *stream*

Sets the scheme (e.g. `"http"`, `"https"`, `mqtt`) on the upstream structure. The scheme string is automatically suffixed with `"://"`.

Parameters:

- `scheme`: scheme name (without `"://"`).

The function is only available after applying the core patch in http subsystem. Must add config `set $upstream_scheme "";` before use this function in stream subsystem.


set_upstream_method
------------------------
**syntax:** *set_upstream_method(method)*

**subsystems:** *http*, *stream*

Set the request method (e.g. `GET`, `POST`) on the upstream structure.

Parameters:

- `method`: method name.

The function is only available after applying the core patch in http subsystem. Must add config `set $upstream_method "";` before use this function in stream subsystem.

[Back to TOC](#table-of-contents)


License
=======

```
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

[Back to TOC](#table-of-contents)
