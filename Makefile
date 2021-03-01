LIBDIR=seds-lib

OFLAGS=-Wall -I$(LIBDIR) -pedantic -g -c -fPIC
SOFLAGS=-shared
LIBFLAGS=-L$(LIBDIR) -lseds

OBJS_TEST=tests/tests.o
OBJS_SIMPLE=simple-serv.o
OBJS_SEDS=seds.o
OBJS_QUEUES=worker-queues.o
BINS=simple-serv seds worker-queues

.PHONY: all
all: $(BINS)

libseds.so: $(LIBDIR)/$@
	cd $(LIBDIR) && $(MAKE) $@ && ln -f $@ ..

seds-tests: $(OBJS_TEST) libseds.so
	gcc -o $@ $(OBJS_TEST) $(LIBFLAGS)

simple-serv: $(OBJS_SIMPLE) libseds.so
	gcc -o $@ $(OBJS_SIMPLE) $(LIBFLAGS) -pthread

seds: $(OBJS_SEDS) libseds.so
	gcc -o $@ $(OBJS_SEDS) $(LIBFLAGS) -pthread

worker-queues: $(OBJS_QUEUES) libseds.so
	gcc -o $@ $(OBJS_QUEUES) $(LIBFLAGS) -pthread

%.o: %.c
	gcc $(OFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f $(OBJS_SINGLE) $(OBJS_MULTI) $(BINS) libseds.so 	
	cd $(LIBDIR) && $(MAKE) clean


