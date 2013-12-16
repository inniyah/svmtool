LIBRARY=svntool-lang-guesser
LIBFILENAME=lib$(LIBRARY)
PROGRAM=guesser

all: $(LIBFILENAME).a $(LIBFILENAME).so $(PROGRAM)

MAJOR=0
MINOR=1

MAPSDIR=./maps

LIB_SOURCES = guesser.c crc32.c utils.c
PRG_SOURCES = main.c

SHARED_OBJS = $(LIB_SOURCES:.c=.shared.o)
STATIC_OBJS = $(LIB_SOURCES:.c=.static.o)

EXTRA_CFLAGS= -DMAPSDIR=\"$(MAPSDIR)\" -DVERSION=\"0.4\"
STATIC_CFLAGS= -O2 -g -Wall -I. $(EXTRA_CFLAGS)
SHARED_CFLAGS= $(STATIC_CFLAGS) -fPIC

LDFLAGS= -Wl,-z,defs -Wl,--as-needed -Wl,--no-undefined
EXTRA_LDFLAGS=

LIB_LIBS= -lm
PRG_LIBS= -lm -L. -l$(LIBRARY)

$(PROGRAM): $(PRG_SOURCES:.c=.static.o)
	g++ -o $@ $+ $(PRG_LIBS)

$(LIBFILENAME).so.$(MAJOR).$(MINOR): $(SHARED_OBJS)
	g++ $(LDFLAGS) $(EXTRA_LDFLAGS) -shared \
		-Wl,-soname,$(LIBFILENAME).so.$(MAJOR) \
		-o $(LIBFILENAME).so.$(MAJOR).$(MINOR) \
		$+ -o $@ $(LIB_LIBS)

$(LIBFILENAME).so: $(LIBFILENAME).so.$(MAJOR).$(MINOR)
	rm -f $@.$(MAJOR)
	ln -s $@.$(MAJOR).$(MINOR) $@.$(MAJOR)
	rm -f $@
	ln -s $@.$(MAJOR) $@

$(LIBFILENAME).a: $(STATIC_OBJS)
	ar cru $@ $+

%.shared.o: %.cpp
	g++ -o $@ -c $+ $(SHARED_CFLAGS)

%.shared.o: %.c
	gcc -o $@ -c $+ $(SHARED_CFLAGS)

%.so : %.o
	g++ $(LDFLAGS) $(EXTRA_LDFLAGS) -shared $^ -o $@

%.static.o: %.cpp
	g++ -o $@ -c $+ $(STATIC_CFLAGS)

%.static.o: %.c
	gcc -o $@ -c $+ $(STATIC_CFLAGS)

clean:
	rm -vf $(SHARED_OBJS)
	rm -vf $(STATIC_OBJS) main.static.o
	rm -vf $(PROGRAM)
	rm -vf *.so *.so* *.a *~

install: $(LIBFILENAME).a $(LIBFILENAME).so $(PROGRAM)
	mkdir -p "$(DESTDIR)/usr/lib/"
	cp -a *.a "$(DESTDIR)/usr/lib/"
	cp -a *.so* "$(DESTDIR)/usr/lib/"
	mkdir -p "$(DESTDIR)/usr/include/"
	cp guesser.h "$(DESTDIR)/usr/include/"
	mkdir -p "$(DESTDIR)/usr/bin/"
	cp guesser "$(DESTDIR)/usr/bin/"
