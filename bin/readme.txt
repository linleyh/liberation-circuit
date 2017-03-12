
Liberation Circuit
Copyright 2017 Linley Henzell

version: 1.0

This the release version of Liberation Circuit, an RTS/programming game.



To play the prebuilt binaries, run LibCirc.exe.

- Manual.html has extensive detail about the game, including documentation for the in-game API.

- Edit init.txt to set screen resolution and other options (fullscreen, sound volume, key rebinding, colourblind mode etc).


--------------------------------------------------


To build the game yourself:

The game should compile on any OS supported by Allegro 5 - to build, compile the c files in the source directory and link with Allegro 5. 

You will need the data files from the most recent release (on github). The executable needs to be in the main directory (the same one as the init.txt file).

Don't try to compile the .c files in the /proc or /story subdirectories! They are code used by the game itself.

The game requires write access to its home directory to save mission completion data (the msn.dat file). If this isn't okay, you can specify a path in the fopen calls at about lines 2808 and 2860 of h_story.c.


--------------------------------------------------


To build using do/redo (using the .do scripts by Nils Dagsson Moskopp):

  To build Liberation Circuit on
  any Unix-like OS like GNU/Linux, execute the “do” script. 
  Note that “do” always compiles all source files; if you want
  to rebuild targets only when relevant source files have changed, you
  should use “redo” instead. A version of “redo” can be obtained from
  <http://news.dieweltistgarnichtso.net/bin/redo-sh.html> (written in
  Bourne shell) or <http://jdebp.eu./Softwares/redo/> (written in C++).

  Packages needed for Liberation Circuit on Debian GNU/Linux or Ubuntu:
  - liballegro-acodec5-dev
  - liballegro-audio5-dev
  - liballegro-dialog5-dev
  - liballegro-image5-dev
  - liballegro5-dev


--------------------------------------------------


To build using cmake (using the cmake scripts by Kyle Findlay; The following instructions are from u/JCanseco on reddit):


  I did compile it with ccmake ncurses frontend on Antergos (based on Arch Linux).

  mkdir build;cd build;ccmake ..

  Adding this line to CMAKE_EXE_LINKER_FLAGS was enough:

  -lallegro_image -lallegro_primitives -lallegro_color -lallegro_acodec -lallegro_audio -lallegro_dialog -lallegro_font -lallegro_main -lallegro -lm

  make -j4 and it compiled fine with Allegro 5.2.2. Extracted zip data on bin folder and it did run fine.



---------------------------------------------------


Thanks to:

Nils Dagsson Moskopp (erlehmann) <nils+liberationcircuit@dieweltistgarnichtso.net> for very useful feedback on the alpha and beta versions.

zugz (from the tigsource forum) for very useful feedback on the beta.

Serge Zaitsev's cucu (http://zserge.com/blog/cucu-part1.html) for a very clear explanation of how to write a simple C compiler.

Batuhan Bozkurt's otomata (http://www.earslap.com/page/otomata.html) for the basis of the cellular automata-based procedural music generation.
