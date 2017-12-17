ifeq ($(OS),Windows_NT)
# Nasty little trick to get us a backslash.
	DS := $(shell echo \)
	NULL = nul
	SET_ENVIRONMENT = set GBDKDIR=gbdk$(DS)
else 
	DS = /
	NULL = /dev/null
	SET_ENVIRONMENT = GBDKDIR=gbdk$(DS) 
endif

CC  = $(SET_ENVIRONMENT)&& gbdk$(DS)bin$(DS)lcc
VBA = tools$(DS)vba$(DS)VisualBoyAdvance
BGB = tools$(DS)bgb$(DS)bgb

# End of easily-user-editable section.

# Tell make these aren't real targets. 
.PHONY: all clean
.SECONDARY: main-build

# Get all of our graphics/map files listed.
GRAPHICS_FILES := $(wildcard graphics/*.s)
MAPS_FILES_A := $(wildcard maps/*.s)
MAPS_FILES_C := $(wildcard maps/*.c)

all: pre-build main-build

# Do a little pre-work to set up the environment before compiling things.
pre-build: 
	@test -d bin || mkdir bin
# Give some useful output if the user didn't grab gbdk yet.	
	@test -d gbdk$(DS)bin || echo "GBDK toolkit not installed in tools/gbdk. Please download and extract it into tools/gbdk."


# Build everything.
main-build:
	$(CC) -c -o bin/main.o main.c
# This grabs everything in maps/* and compiles it, then puts the results into bin/ (And bank 2, based on -Wf-bo1)
# Assembly files
	$(foreach FILE, $(MAPS_FILES_A), $(shell $(CC) -Wa-l -Wf-bo2 -o $(FILE:maps/%.s=bin/%.o) -c $(FILE)))
# C files	
	$(foreach FILE, $(MAPS_FILES_C), $(shell $(CC) -Wa-l -Wf-bo2 -o $(FILE:maps/%.c=bin/%.o) -c $(FILE)))
	
# This grabs everything in graphics/* and does the same thing with bank 1.
	$(foreach FILE, $(GRAPHICS_FILES), $(shell $(CC) -Wa-l -Wf-bo1 -o $(FILE:graphics/%.s=bin/%.o) -c $(FILE)))
	
# Grab title tiles and source, and jam it all into bank 3
	$(CC) -Wa-l -Wf-bo3 -o bin/title.o -c title.c
	$(CC) -Wa-l -Wf-bo3 -o bin/pause.o -c pause.c
	$(CC) -Wa-l -Wf-bo3 -o bin/title_tiles.o -c title_graphics/title_tiles.s
	
# Separated sprite code, too!
	$(CC) -Wa-l -Wf-bo4 -o bin/sprite.o -c sprite.c

# Compile everything in bin into main.gb
	$(CC) -Wl-yt1 -Wl-yo8 -o main.gb bin/*.o
	
emu: 
	$(BGB) main.gb 
vba:
	$(VBA) main.gb
	
clean:
	-rm -rf bin *.gb 2> $(NULL)
