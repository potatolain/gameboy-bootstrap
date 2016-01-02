ifeq ($(OS),Windows_NT)
# Nasty little trick to get us a backslash.
	DS := $(shell echo \)
	NULL = nul
else 
	DS = /
	NULL = /dev/null
endif

CC  = gbdk$(DS)bin$(DS)lcc
VBA = tools$(DS)vba$(DS)VisualBoyAdvance
BGB = tools$(DS)bgb$(DS)bgb

# End of easily-user-editable section.

# Tell make these aren't real targets. 
.PHONY: all clean
.SECONDARY: main-build

# Get all of our graphics/map files listed.
GRAPHICS_FILES := $(wildcard graphics/*.s)
MAPS_FILES := $(wildcard maps/*.s)

all: pre-build main-build

# Do a little pre-work to set up the environment before compiling things.
pre-build: 
	@test -d bin || mkdir bin
# Give some useful output if the user didn't grab gbdk yet.	
	@test -d gbdk$(DS)bin || echo "GBDK toolkit not installed in tools/gbdk. Please download and extract it into tools/gbdk."


# Build everything.
main-build:
	$(CC) -c -o bin/main.o main.c
# This grabs everything in maps/* and compiles it, then puts the results into bin/ (And bank 1, based on -Wf-bo1)
	$(foreach FILE, $(MAPS_FILES), $(shell $(CC) -Wa-l -Wf-bo1 -o $(FILE:maps/%.s=bin/%.o) -c $(FILE)))
# This grabs everything in graphics/* and does the same thing with bank 2.
	$(foreach FILE, $(GRAPHICS_FILES), $(shell $(CC) -Wa-l -Wf-bo2 -o $(FILE:graphics/%.s=bin/%.o) -c $(FILE)))
# Compile everything in bin into main.gb
	$(CC) -Wl-yt1 -Wl-yo8 -o main.gb bin/*.o
	
emu: 
	$(BGB) main.gb 
vba:
	$(VBA) main.gb
	
clean:
	-rm -rf bin *.gb 2> $(NULL)
