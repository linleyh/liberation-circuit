ALLEGRO_MODULES = allegro-5 allegro_audio-5 allegro_acodec-5 allegro_dialog-5 allegro_font-5 allegro_image-5 allegro_primitives-5
CFLAGS = $$(pkg-config --cflags $(ALLEGRO_MODULES)) -Wall
LIBS = -lm $$(pkg-config --libs $(ALLEGRO_MODULES))
HEADERS = $(shell find src/ -name '*.h')
OBJECTS = $(shell find src/ -name '*.c' | sed -e 's/\.c$$/.o/g')

.PHONY: all clean

all: bin/lc

clean:
	rm -f bin/lc src/*.o

bin/lc: $(OBJECTS)
	gcc $(CFLAGS) -o $@ $? $(LIBS)

%.o: %.c $(HEADERS)
	gcc $(CFLAGS) -c -o $@ $<
