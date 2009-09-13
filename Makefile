view: view.o

.PHONY: clean
clean:
	$(RM) view view.o

%.o: %.c
	$(CC) $^ `sdl-config --cflags` $(CFLAGS) -c -o $@
# For use with mingw32 cross-compiler:
#	$(CC) $^ -I/usr/i586-mingw32msvc/include/SDL -D_REENTRANT $(CFLAGS) -c -o $@

%: %.o
# Statically link, but holy crap is this icky! There should be a better way. Also, do we need to statically link more? less?
	$(CC) $^ $(LDFLAGS) -Wl,-Bstatic -lSDL_image `sdl-config --libs` -lpng12 -lz -ltiff -ljpeg -lasound -laudio -lesd -Wl,-Bdynamic `directfb-config --libs` -lpulse-simple -lcaca -laa -ldl -mwindows -o $@
# For use with mingw32 cross-compiler:
#	$(CC) $^ $(LDFLAGS) -lmingw32 -Wl,-Bstatic -lSDL_image -lSDLmain -Wl,-Bdynamic -lSDL -mwindows -o $@
