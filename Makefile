CC = gcc

CFLAGS = -W -Wall -Wextra -Werror -g
LDFLAGS = -lrt

TONESRV_TARGET = toneserver

TONESRV_OBJS = \
  toneserver.o

all: $(TONESRV_TARGET)

$(TONESRV_TARGET): $(TONESRV_OBJS)
	$(CC) $(LDFLAGS) -o $(TONESRV_TARGET) $(TONESRV_OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $<