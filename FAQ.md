# FAQ

## Why did you make this?

The goal of this is to give people the tools to quickly create the start of a Gameboy game. It also was a good project
to hone my skills in a few areas. Note that this isn't a very serious project, but was really meant to be fun.

Hopefully this will be useful to someone - either as a starting place for a Game Jam, or even just a teaching tool.
(Though, there are some very bad anti-lessons in here. Please use with a critical eye!)

## Will my game run on a real Gameboy?

Yes. I have tested the output of this and the Classic Game Builder tool on a physical gameboy multiple times.

There are many tools availbale for this, however I have had best luck with this cartridge: http://store.kitsch-bent.com/product/usb-64m-smart-card

## I haven't worked with the Gameboy or GBDK before. What do I need to know?

There are a few things worth knowing, and some of this may be a little discouraging.

First, Gameboy programming is frustrating at times. C isn't the most forgiving language, and working with a system
like the Gameboy takes away some of the simplest tools you might use while debugging, such as logging. You may find
yourself spending hours on tiny problems, and inventing cursewords in foreign languages.

That said, it can also be very rewarding. Writing something you know you can use on a real gameboy
is really cool, if that's something you're into.

The documentation for GBDK is available here: http://gbdk.sourceforge.net/doc/html/book01.html. You'll likely find
yourself mainly looking at the Graphics and Joypad sections. It is not to be overlooked.

Finally, know that GBDK can be buggy. There is a chance you will find situations that defy logic when compiling your
game. I have heard tell of greater than/equal to signs not properly matching numbers, for example. I have also seen a
case where multiplying 1 by 32 (or bit shifting with the << operator) resulted in 64. (Workaround was to shift by 4, then
by 1.) Unfortunately, I don't have any great advice for what to do in these situations - just be aware that it is 
possible that the problem is with the compiler, not with your code.

## Seriously, why Windows?

I answer this in some amount of detail in the compiling document, but I'll try to explain a little further here.

When I started out on this project, I had every intention of supporting the big 3: Windows, Linux, and Mac OS. I made
the makefile in a way that could be used across all three systems and tested semi-frequently against a linux system.

For a while, all was fine and dandy.

At some point during the project, I found that my source would no longer even compile under Linux. I explored why, and
it seems like the perfectly valid triple pointer (yes, that's gross, but that is another story) caused the compiler to
choke. I was able to work around this by changing how I got at my data, but this failed to compile on Windows for a
similar reason. At this point, I realized that without serious refactoring, there was no way I could build something
this complex without maintaining two code paths in situations like this.

Some of the problem may also have stemmed from my usage of GBDK 2.96 on Linux and 2.95 on Windows, however no 2.96 build
was ever made for Windows, and I could not manage to get the code to compile to build it myself. Versions 2.92-2.95 on
linux immediately Segfault upon trying to compile anything. (Or run --version...)

Based on prior experience, I also fear that I would uncover more issues like this. In the interest of providing the most
stable experience possible, I have chosen to only support GBDK 2.95 on Windows. This way, I can be certain that all
roms generated from this code will behave in the same manner.
