CFLAGS=-O0 -g
CPPFLAGS=-I/usr/pkg/include -I./include -DPOSIX_MAKE
LDFLAGS=-Wl,-rpath=/usr/pkg/lib
LIBS=-ljansson -ldb5
LIBPATH=-L/usr/pkg/lib
RM=rm
RMFLAGS=-f
INSTALL=install


IRCOBJS=src/IRCbot.o src/dbjson.o src/debug.o src/events.o src/ircwrap.o src/tokparse.o 

src/IRCbot: $(IRCOBJS)
	$(CC) $(LDFLAGS) $(LIBPATH) -o $@ $(IRCOBJS) $(LIBS)

.c:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LIBPATH) -o $@ $< $(LIBS)

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

clean:
	$(RM) $(RMFLAGS) src/IRCbot src/ghdecode $(IRCOBJS)

install:
	install src/IRCbot /usr/local/bin
