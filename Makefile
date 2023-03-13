CXXOPTS = -std=c++17 -ggdb3
SHARED = -fPIC -shared


all: libclock.so libfifo.so libiad.so liblru.so 

libfifo.so: fifo.cc
	g++ $(CXXOPTS) $^ -o $@ $(SHARED) -O3

libiad.so: iad.c
	gcc $^ -o $@ $(SHARED) -O3

liblru.so: lru.cc
	g++ $(CXXOPTS) $^ -o $@ $(SHARED) -O3

liblru2.so: lru2.cc
	g++ $(CXXOPTS) $^ -o $@ $(SHARED) -O3

libclock.so: clock.cc
	g++ $(CXXOPTS) $^ -o $@ $(SHARED) -O3

libqfifo.so: qfifo.cc
	g++ $(CXXOPTS) $^ -o $@ $(SHARED) -O3

lsms.so: lsm.c lsm2.c
	g++ $(CXXOPTS) $^ -o $@ $(SHARED) -O3

