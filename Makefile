LDFLAGS = -lIphlpapi -lws2_32 -lpsapi

loquat.dll : loquat.o deps/uv/uv.a deps/lua/src/liblua.a
	gcc loquat.o -shared -o loquat.dll \
		-DBUILDDLL \
		deps/lua/src/liblua.a deps/uv/uv.a \
		$(LDFLAGS)
		
loquat.o: src/loquat.c src/loquat.h
	gcc -c src/loquat.c \
		-DBUILDDLL \
		-Ideps/uv/include -Ideps/lua/src \
	
deps/uv/uv.a:
	$(MAKE) -C deps/uv
	
deps/lua/src/liblua.a:
	$(MAKE) -C deps/lua mingw