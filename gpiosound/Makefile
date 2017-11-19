CC = gcc

COMMON_CFLAGS = -std=gnu11 -W -Wall -Wextra -Werror \
  -Wduplicated-cond \
  -Wlogical-op \
  -Wnull-dereference \
  -Wjump-misses-init \
  -Wdouble-promotion \
  -Wshadow \
  -Wformat=2 \
  -Wundef \
  -Wpointer-arith \
  -Wcast-align \
  -Wstrict-prototypes \
  -Wstrict-overflow=5 \
  -Wwrite-strings \
  -Waggregate-return \
  -Wcast-qual \
  -Wswitch-default \
  -Wswitch-enum \
  -Wconversion \
  -Wunreachable-code

COMMON_LDFLAGS = -lrt -lwiringPi -lm

# "make DEBUG=1" => debug build
ifeq ($(DEBUG),1)
  CFLAGS = $(COMMON_CFLAGS) -g
  LDFLAGS = $(COMMON_LDFLAGS)
else
  CFLAGS = $(COMMON_CFLAGS) -DNDEBUG -O3
  LDFLAGS = $(COMMON_LDFLAGS)
endif

TONESRV_TARGET = gpiosound

TONESRV_OBJS = \
  main.o \
  optparse.o \
  tone.o \
  noise.o

WRITEFLOAT_TARGET = writefloat

WRITEFLOAT_OBJS = \
  writefloat.o

all: $(TONESRV_TARGET) $(WRITEFLOAT_TARGET) $(NOISESRV_TARGET)

$(TONESRV_TARGET): $(TONESRV_OBJS)
	$(CC) $(LDFLAGS) -o $(TONESRV_TARGET) $(TONESRV_OBJS)

$(WRITEFLOAT_TARGET): $(WRITEFLOAT_OBJS)
	$(CC) $(LDFLAGS) -o $(WRITEFLOAT_TARGET) $(WRITEFLOAT_OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o *~ $(TONESRV_TARGET) $(WRITEFLOAT_TARGET) $(NOISESRV_TARGET)

test: $(TONESRV_TARGET) $(WRITEFLOAT_TARGET)
	(./writefloat 440.0; sleep 5) | ./gpiosound -o 18
