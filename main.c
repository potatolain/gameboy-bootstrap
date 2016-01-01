#include "main.h"
#include <gb/gb.h>

#define BANK_GRAPHICS	1
#define BANK_MAP		2

#define MAP_TILES_DOWN 8
#define MAP_TILES_ACROSS 10
UBYTE temp1, temp2, temp3, temp4, temp5, temp6, i, j;
UBYTE playerWorldPos, playerX, playerY, btns, oldBtns, playerXVel, playerYVel, gameState, playerVelocityLock, cycleCounter;
UBYTE playerHealth;
UBYTE buffer[20U];
UINT16 temp16, temp16b, playerWorldTileStart;
UBYTE* currentMap;
UBYTE* tempPointer; 

enum SPRITE_DIRECTION playerDirection;

void init_vars() {
	playerWorldPos = 0U;
	btns = oldBtns = 0U;
	playerXVel = playerYVel = 0U;
	
	playerHealth = 5U;
	// gameState = GAME_STATE_RUNNING;
	playerVelocityLock = 0U;

}

void load_map() {
	SWITCH_ROM_MBC1(BANK_MAP);
	currentMap = MAP;
	playerWorldTileStart = ((8*10)*8);
	
	for (i = 0U; i != MAP_TILES_DOWN; i++) {
		for (j = 0U; j != MAP_TILES_ACROSS; j++) {
			buffer[j*2U] = currentMap[playerWorldTileStart + j] << 2U; 
			buffer[j*2U+1U] = buffer[j*2U]+1U;
		}
		set_bkg_tiles(0U, i << 1U, 20U, 1U, buffer);
		
		for (j = 0U; j != MAP_TILES_ACROSS*2; j++) {
			buffer[j]+=2;
		}
		set_bkg_tiles(0U, i*2U+1U, 20U, 1U, buffer);
		playerWorldTileStart += MAP_TILES_ACROSS;

	}
}

void init_screen() {
	
	disable_interrupts();
	DISPLAY_OFF;
	
	SWITCH_ROM_MBC1(BANK_GRAPHICS);
	set_bkg_data(0U, 128U, TILES);
	set_win_data(0U, 128U, TILES);
	set_sprite_data(0U, 64U, SPRITES);

	load_map();
	
	scroll_bkg(0U, 0U);
	SPRITES_8x8;
		
	SHOW_BKG;
	SHOW_SPRITES;

	move_win(0, 128);
	SHOW_WIN;
	
	DISPLAY_ON;
	enable_interrupts();

}

void main() {
	init_vars();
	
	// TODO: Built-in title screen?
	//SWITCH_ROM_MBC1(BANK_TITLE);
	//show_title();
	
	init_screen();
	
	while(1) {
		cycleCounter++;
	}

}
