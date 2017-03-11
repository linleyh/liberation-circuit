#!/bin/sh
for file in *.c; do
  redo-ifchange ${file%.*}.o
done
gcc -O3 -fwrapv -o $3 *.o -lallegro -lm -lallegro_audio -lallegro_acodec -lallegro_font -lallegro_image -lallegro_primitives -lallegro_dialog
