CC := gcc
CFLAGS := -O2 -g
LDFLAGS := -lcrypto

all:
	$(CC) $(CFLAGS) -o signtos signtos.c $(LDFLAGS)
clean:
	rm signtos
