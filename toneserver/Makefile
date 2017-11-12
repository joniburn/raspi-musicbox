CC = gcc

CFLAGS = -W -Wall -Wextra -Werror -O3 -std=gnu11
LDFLAGS = -lrt -lwiringPi

TONESRV_TARGET = toneserver

TONESRV_OBJS = \
  toneserver.o \
	optparse.o \
	tone.o

all: $(TONESRV_TARGET)

$(TONESRV_TARGET): $(TONESRV_OBJS)
	$(CC) $(LDFLAGS) -o $(TONESRV_TARGET) $(TONESRV_OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o *~ $(TONESRV_TARGET)
