LDFLAGS = -lIphlpapi -lws2_32 -lpsapi
OBJECTS = src/tcpsocket.o src/timer.o

loquat.dll : $(OBJECTS) src/loquat.o deps/uv/uv.a deps/lua/src/liblua.a
	gcc src/loquat.o $(OBJECTS) -shared -o loquat.dll \
		-DBUILDDLL \
		deps/lua/src/liblua.a deps/uv/uv.a \
		$(LDFLAGS)
		
src/loquat.o: src/loquat.c src/loquat.h src/*.h
	gcc -c -o src/loquat.o src/loquat.c \
		-DBUILDDLL \
		-Ideps/uv/include -Ideps/lua/src \
		
$(OBJECTS): %.o: %.c
	gcc -c -o $@ $< \
		-Ideps/uv/include -Ideps/lua/src \
		
src/tcpsocket.c: src/tcpsocket.h

deps/uv/uv.a:
	$(MAKE) -C deps/uv
	
deps/lua/src/liblua.a:
	$(MAKE) -C deps/lua mingw
	
clean:
	rm src/*.o
	rm loquat.dll