LIBRARY=libsvmtool++
PROGRAMS=SVMTeval SVMTlearn SVMTagger

all: $(LIBRARY).a $(LIBRARY).so $(PROGRAMS)

SVMTeval: src/SVMTeval.static.o $(LIBRARY).a $(LIBRARY).so
	g++ $(LDFLAGS) $(EXTRA_LDFLAGS) $< -o $@ -L. -lsvmtool++ $(LIBS)

SVMTlearn: src/SVMTlearn.static.o $(LIBRARY).a $(LIBRARY).so
	g++ $(LDFLAGS) $(EXTRA_LDFLAGS) $< -o $@ -L. -lsvmtool++ $(LIBS)

SVMTagger: src/SVMTagger.static.o $(LIBRARY).a $(LIBRARY).so
	g++ $(LDFLAGS) $(EXTRA_LDFLAGS) $< -o $@ -L. -lsvmtool++ $(LIBS)

MAJOR=0
MINOR=0

SOURCES = src/learner.cc \
          src/tagger.cc \
          src/swindow.cc \
          src/mapping.cc \
          src/dict.cc \
          src/list.cc \
          src/weight.cc \
          src/hash.cc \
          src/stack.cc \
          src/er.cc \
          src/common.cc

INCLUDE_DIR = include

SHARED_OBJS = $(SOURCES:.cc=.shared.o)
STATIC_OBJS = $(SOURCES:.cc=.static.o)

EXTRA_CFLAGS=-I$(INCLUDE_DIR)
STATIC_CFLAGS= -O2 -g -ansi -Wall -Wno-unused-parameter $(EXTRA_CFLAGS)
SHARED_CFLAGS= $(STATIC_CFLAGS) -fPIC

LDFLAGS= -Wl,-z,defs -Wl,--as-needed -Wl,--no-undefined
EXTRA_LDFLAGS=
LIBS= 

$(LIBRARY).so.$(MAJOR).$(MINOR): $(SHARED_OBJS)
	g++ $(LDFLAGS) $(EXTRA_LDFLAGS) -shared \
		-Wl,-soname,$(LIBRARY).so.$(MAJOR) \
		-o $(LIBRARY).so.$(MAJOR).$(MINOR) \
		$+ -o $@ $(LIBS)

$(LIBRARY).so: $(LIBRARY).so.$(MAJOR).$(MINOR)
	rm -f $@.$(MAJOR)
	ln -s $@.$(MAJOR).$(MINOR) $@.$(MAJOR)
	rm -f $@
	ln -s $@.$(MAJOR) $@

$(LIBRARY).a: $(STATIC_OBJS)
	ar cru $@ $+

%.shared.o: %.cpp
	g++ -o $@ -c $+ $(SHARED_CFLAGS)

%.shared.o: %.cc
	g++ -o $@ -c $+ $(SHARED_CFLAGS)

%.shared.o: %.c
	gcc -o $@ -c $+ $(SHARED_CFLAGS)

%.so : %.o
	g++ $(LDFLAGS) $(EXTRA_LDFLAGS) -shared $^ -o $@

%.static.o: %.cpp
	g++ -o $@ -c $+ $(STATIC_CFLAGS)

%.static.o: %.cc
	g++ -o $@ -c $+ $(STATIC_CFLAGS)

%.static.o: %.c
	gcc -o $@ -c $+ $(STATIC_CFLAGS)

clean:
	rm -fv $(SHARED_OBJS)
	rm -fv $(STATIC_OBJS)
	rm -fv $(FIXED_OBJS)
	rm -fv $(STATIC_FIXED_OBJS)
	rm -fv $(PROGRAMS)
	rm -fv *.so *.so* *.a *~

DESTDIR=

install: $(LIBRARY).a $(LIBRARY).so
	mkdir -p "$(DESTDIR)/usr/lib/"
	cp -a *.a "$(DESTDIR)/usr/lib/"
	cp -a *.so* "$(DESTDIR)/usr/lib/"
	mkdir -p "$(DESTDIR)/usr/include/"

	find $(INCLUDE_DIR) -name "*.h" | \
		while read f; do \
			t=`echo "$$f" | sed -e 's|^$(INCLUDE_DIR)/|/usr/include/%PACKAGE%/|'` ; \
			d=`dirname "$$t"`; \
			mkdir -p "$(DESTDIR)/$$d" ; \
			cp "$$f" "$(DESTDIR)/$$t"; \
		done
