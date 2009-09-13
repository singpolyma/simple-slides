view: view.o

.PHONY: clean
clean:
	$(RM) view view.o

%.o: %.c
	$(CC) $^ `sdl-config --cflags` $(CFLAGS) -c -o $@

%: %.o
# Statically link, but holy crap is this icky! There should be a better way. Also, do we need to statically link more? less?
	$(CC) $^ $(LDFLAGS) -Wl,-Bstatic -lSDL_image `sdl-config --static-libs` -lpng12 -lz -ltiff -ljpeg -lasound -laudio -lesd -Wl,-Bdynamic `directfb-config --libs` -lpulse-simple -lcaca -laa -ldl -o $@
