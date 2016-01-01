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


all:
	@test -d bin || mkdir bin
# Give some useful output if the user didn't grab gbdk yet.	
	@test -d gbdk$(DS)bin || echo "GBDK toolkit not installed in tools/gbdk. Please download and extract it into tools/gbdk."
	
	$(CC) -c -o bin/main.o main.c
	$(CC) -Wa-l -Wf-bo1 -c -o bin/map.o graphics/map.s
	$(CC) -Wa-l -Wf-bo2 -c -o bin/tiles.o graphics/tiles.s
	$(CC) -Wl-yt1 -Wl-yo8 -o main.gb bin/*.o
	
emu: 
	$(BGB) main.gb 
vba:
	$(VBA) main.gb
	
clean:
	-rm -rf bin *.gb 2> $(NULL)
