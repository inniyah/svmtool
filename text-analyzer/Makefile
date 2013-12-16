PROGRAM=atext

all: $(PROGRAM)

OBJS = example.o Ini.o IniReader.o LMMin.o LMCurve.o \
	UnivariateDensityDerivative.o UnivariateBandwidthEstimate.o \
	FastUnivariateDensityDerivative.o FastUnivariateBandwidthEstimate.o

#PKG_CONFIG=
#PKG_CONFIG_CFLAGS=`pkg-config --cflags $(PKG_CONFIG)`
#PKG_CONFIG_LIBS=`pkg-config --libs $(PKG_CONFIG)`

EXTRA_CFLAGS=-I../include/
#EXTRA_CFLAGS=-I. $(PKG_CONFIG_CFLAGS)
EXTRA_CXXFLAGS= $(EXTRA_CFLAGS)
CFLAGS= -O2 -g -Wall
CXXFLAGS= $(CFLAGS) -std=c++11

LDFLAGS= -Wl,-z,defs -Wl,--as-needed -Wl,--no-undefined
EXTRA_LDFLAGS=
LIBS=-L.. -lsvmtool++
#LIBS=$(PKG_CONFIG_LIBS)

$(PROGRAM): $(OBJS)
	g++ $(LDFLAGS) $(EXTRA_LDFLAGS) $+ -o $@ $(LIBS)

%.o: %.cpp
	g++ -o $@ -c $+ $(CXXFLAGS) $(EXTRA_CXXFLAGS)

%.o: %.c
	gcc -o $@ -c $+ $(CFLAGS) $(EXTRA_CFLAGS)

clean:
	rm -f $(OBJS)
	rm -f $(PROGRAM)
	rm -f *.o *~
