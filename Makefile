server.exe : deps/uv/uv.a deps/lua/?
	gcc \
		-Ideps/uv/include -Ideps/lua/src lua/src/liblua.a libuv/uv.a -lws2_32
	
libuv/uv.a:
	$(MAKE) -C deps/uv
	
lua/src/liblua.a:
	$(MAKE) -C deps/lua mingw