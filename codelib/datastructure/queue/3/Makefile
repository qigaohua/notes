CC=gcc

INC+= -I.
LIB+= -pthread

CFLAGS+= -g -std=gnu99

TARGET= test
ALL:$(TARGET)

OBJ= rte_ring.o test.o

$(TARGET):$(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	rm -f *.o
	rm -f $(TARGET)

install:
