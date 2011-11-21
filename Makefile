#
# Makefile for systems with pkg-config
#

PKG_FLAGS = `pkg-config --cflags --libs opencv`

digimag: main.c ini.c config.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(PKG_FLAGS) $^ -o $@

clean:
	rm -f digimag
