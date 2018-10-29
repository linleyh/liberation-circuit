for file in *.c; do
  echo "${file%.*}.o"
done | xargs redo-ifchange
gcc -O3 -fwrapv -o $3 *.o -lallegro -lm -lallegro_audio -lallegro_acodec -lallegro_font -lallegro_image -lallegro_primitives -lallegro_dialog -lallegro_main
