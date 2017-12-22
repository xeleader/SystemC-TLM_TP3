CC=gcc
INCLUDE=-I. -I../../
CFLAGS=$(INCLUDE)

main.o: ../main.c ../../address_map.h hal.h
	$(CC) -o main.o -c ../main.c $(CFLAGS)

.PHONY: clean realclean
clean:
	$(RM) main.o

realclean: clean
	-$(RM) *~
