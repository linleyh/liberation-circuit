# Liberation Circuit

This is the release version of Liberation Circuit, an RTS/programming
game.

To play the pre-built binaries on Windows, [download the latest
release](https://github.com/linleyh/liberation-circuit/releases) and run
`LibCirc.exe`. There are also pre-built binaries for all platforms
available on [itch.io](https://linleyh.itch.io/liberation-circuit).

Vanilla has set up a [Discord server](https://discord.gg/8q7DFaM) to
discuss strategy and things.

## Screenshots

![a screenshot](https://i.imgur.com/pPIJ03I.png)
![another screenshot](https://i.imgur.com/QKWzkqA.png)

## Dependencies

The following libraries are required to play Liberation Circuit.

* Allegro5
* Allegro5 acodec
* Allegro5 audio
* Allegro5 dialog
* Allegro5 image

## Compiling

It should compile on any OS supported by Allegro 5 - to build, compile
the c files in the source directory and link with Allegro 5. More
detailed instructions are below. More detail about the source file
structure is at the start of `m_main.c`.

The executable should go in the "bin" subdirectory (the same directory as
the "init.txt" file). The game requires write access to this directory to
save mission progress. If this isn't okay, you can specify a path in the
fopen calls at about lines 2808 and 2860 of `h_story.c`. Don't try to
compile the `.c` files in the /proc or /story subdirectories! They are
code used by the game itself.

- [Manual.html](bin/Manual.html) has extensive detail about the game,
  including documentation for the in-game API.
- Edit [init.txt](bin/init.txt) to set screen resolution and other
  options (fullscreen, sound volume, key rebinding, colourblind mode
etc).


## Compiling on Linux

To build on a Linux system, there are two options available.

- `make`
- `do`

Note that `do` always compiles all source files; if you want to rebuild
targets only when relevant source files have changed, you should use
`redo` instead. A version of `redo` can be obtained from
<http://news.dieweltistgarnichtso.net/bin/redo-sh.html> (written in
Bourne shell) or <http://jdebp.eu./Softwares/redo/> (written in C++).

## Compiling on macOS

To build on macOS (Sierra (10.12) with latest Homebrew and Xcode)

```sh
git clone https://github.com/linleyh/liberation-circuit.git
cd liberation-circuit
brew install allegro
./do
cd bin
libcirc
```

If you are using a Retina screen, you may want to set the double_fonts
option to make the text larger (edit init.txt to do this).

## Thanks to:

- [Nils Dagsson Moskopp](https://github.com/erlehmann) for very useful
  feedback on the alpha and beta versions.
- zugz (from the tigsource forum) for very useful feedback on the beta.
- Serge Zaitsev's [cucu](https://zserge.com/posts/cucu-part1/) for a very clear explanation of how to write a simple C compiler.
- Batuhan Bozkurt's [otomata](http://www.earslap.com/page/otomata.html) for the basis of the cellular automata-based procedural music generation.