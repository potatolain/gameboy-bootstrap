#include "main.h"
#include <gb/gb.h>

#define BANK_GRAPHICS	1
#define BANK_MAP		2
#define PLAYER_MOVE_DISTANCE 3

#define MAP_TILE_SIZE 80U
#define WORLD_MAX_TILE 64
#define WORLD_ROW_HEIGHT 8

#define MAP_TILES_DOWN 8
#define MAP_TILES_ACROSS 10
UBYTE temp1, temp2, temp3, temp4, temp5, temp6, i, j;
UBYTE playerWorldPos, playerX, playerY, btns, oldBtns, playerXVel, playerYVel, gameState, playerVelocityLock, cycleCounter;
UBYTE playerHealth;
UBYTE buffer[20U];
UINT16 temp16, temp16b, playerWorldTileStart;
UBYTE* currentMap;
UBYTE* tempPointer; 

enum SPRITE_DIRECTION {
	SPRITE_DIRECTION_STOP = 0U,
	SPRITE_DIRECTION_UP = 1U,
	SPRITE_DIRECTION_DOWN = 2U,
	SPRITE_DIRECTION_LEFT = 3U,
	SPRITE_DIRECTION_RIGHT = 4U
};

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
	playerWorldTileStart = (UINT16)playerWorldPos * (UINT16)MAP_TILE_SIZE;
	
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

void handle_input() {
	
	oldBtns = btns;
	btns = joypad();
	
	// Special case for the trial to let us switch screens by holding select. 
	// Remove this if you do not want that functionality.
	if (btns & J_SELECT && btns != oldBtns) {
		if (btns & J_LEFT && playerWorldPos > 0) {
			playerWorldPos--; 
			load_map();
		} else if (btns & J_RIGHT && playerWorldPos < (WORLD_MAX_TILE-1U)) {
			playerWorldPos++;
			load_map();
		} else if (btns & J_UP && playerWorldPos >= WORLD_ROW_HEIGHT) {
			playerWorldPos -= WORLD_ROW_HEIGHT;
			load_map();
		} else if (btns & J_DOWN && playerWorldPos <= (WORLD_MAX_TILE-WORLD_ROW_HEIGHT)) {
			playerWorldPos += WORLD_ROW_HEIGHT;
			load_map();
		}
	} else if (!playerVelocityLock) {
		// General player movement  code.
		playerXVel = playerYVel = 0;

		if (btns & J_UP) {
			playerYVel = -PLAYER_MOVE_DISTANCE;
			playerDirection = SPRITE_DIRECTION_UP;
		} else if (btns & J_DOWN) {
			playerYVel = PLAYER_MOVE_DISTANCE;
			playerDirection = SPRITE_DIRECTION_DOWN;
		}
		
		if (btns & J_LEFT) {
			playerXVel = -PLAYER_MOVE_DISTANCE;
			playerDirection = SPRITE_DIRECTION_LEFT;
		} else if (btns & J_RIGHT) {
			playerXVel = PLAYER_MOVE_DISTANCE;
			playerDirection = SPRITE_DIRECTION_RIGHT;
		}
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
		handle_input();
	}

}
