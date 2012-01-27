loquat.dll : deps/uv/uv.a deps/lua/src/liblua.a
	gcc -shared -o loquat.dll \
		-Ideps/uv/include -Ideps/lua/src deps/lua/src/liblua.a deps/uv/uv.a -lws2_32
	
deps/uv/uv.a:
	$(MAKE) -C deps/uv
	
deps/lua/src/liblua.a:
	$(MAKE) -C deps/lua mingw