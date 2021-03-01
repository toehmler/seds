OFLAGS=-Wall -pedantic -g -c -fPIC
SOFLAGS=-shared

OBJS = msg.o queue.o net.o serv.o sfs.o frmt.o rbuf.o

libseds.so: $(OBJS)
	gcc $(SOFLAGS) -o $@ $^

%.o: %.c
	gcc $(OFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f $(OBJS) libseds.so

