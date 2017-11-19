CC = gcc

CFLAGS = -W -Wall -Wextra -Werror -O3 -std=gnu11
LDFLAGS = -lrt -lwiringPi

TONESRV_TARGET = toneserver

TONESRV_OBJS = \
  toneserver.o \
  optparse.o \
  tone.o

WRITE_D_TARGET = write_d

WRITE_D_OBJS = \
  write_d.o

NOISESRV_TARGET = noiseserver

NOISESRV_OBJS = \
  noiseserver.o \
  optparse.o \
  noise.o

all: $(TONESRV_TARGET) $(WRITE_D_TARGET) $(NOISESRV_TARGET)

$(TONESRV_TARGET): $(TONESRV_OBJS)
	$(CC) $(LDFLAGS) -o $(TONESRV_TARGET) $(TONESRV_OBJS)

$(WRITE_D_TARGET): $(WRITE_D_OBJS)
	$(CC) $(LDFLAGS) -o $(WRITE_D_TARGET) $(WRITE_D_OBJS)

$(NOISESRV_TARGET): $(NOISESRV_OBJS)
	$(CC) $(LDFLAGS) -o $(NOISESRV_TARGET) $(NOISESRV_OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o *~ $(TONESRV_TARGET) $(WRITE_D_TARGET) $(NOISESRV_TARGET)

test: $(TONESRV_TARGET) $(WRITE_D_TARGET)
	(./write_d; sleep 5) | ./toneserver -o 18
