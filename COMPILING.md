# Compiling

So, you've decided you want to create your own Gameboy game using this too, huh? Here's some information...

## Prerequisites

1. A Windows machine.
2. Gow : https://github.com/bmatzelle/gow (Cygwin should work as well, but is untested!)
3. Gameboy Bootstrap code on your filesystem (Source zip from the Game Builder utility, or git checkout)

Why Windows? Unfortunately, GBDK seems to have never been very well tested or supported on Linux. There is a newer 2.96
build of it that is available for Linux, however 2.96 was never built for windows, and the two seem incompatible. The
results of compiling the same game on Windows and Linux seems to differ  with the same exact code.
It's painful to debug, and it's painful to work with.

As a result, I am only supporting Windows at this time, sorry.

## Environment setup

1. Extract gbdk 2.95 to the ./gbdk folder. Get that from: https://sourceforge.net/projects/gbdk/files/gbdk-win32/2.95-3/
2. Get a copy of BGB from http://bgb.bircd.org/ and put it into the ./tools/bgb folder.
3. _Optional_: Get a copy of VisualBoyAdvance and put it into the ./tools/vba folder.

## Compiling

Compiling the game should be as simple as typing `make` into a cmd window in the gameboy_bootstrap home folder.

To clean up old files you can use `make clean`.

## Running

You can run your game in any emulator you like. If you followed the instructions above, you can also run your rom with
BGB by typing `make emu`. You can launch VisualBoyAdvance using `make vba`.

_Please feel free to improve these instructions with a PR. This is everything I can think of, but it feels very bare._
