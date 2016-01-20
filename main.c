#include "main.h"
#include <gb/gb.h>

#define BANK_GRAPHICS	1
#define BANK_MAP		2
#define PLAYER_MOVE_DISTANCE 3

#define MAP_TILE_SIZE 80U
#define WORLD_MAX_TILE 64U
#define WORLD_ROW_HEIGHT 8U
#define SPRITE_SIZE 12U
#define SPRITE_LEFT_BUFFER 2U
#define SPRITE_TOP_BUFFER 2U

#define SCREEN_WIDTH 160U
#define SCREEN_HEIGHT 128U // 144 - 16px status bar
#define STATUS_BAR_HEIGHT 16U

#define MAP_TILES_DOWN 8U
#define MAP_TILES_ACROSS 10U
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
	playerX = playerY = 32U;
	
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
	// Leave this as it was - we depend on it elsewhere.
	playerWorldTileStart = (UINT16)playerWorldPos * (UINT16)MAP_TILE_SIZE;

}

UBYTE test_collision(UBYTE x, UBYTE y) {

	// This offsets us by one tile to get us in line with 0-7= tile 0, 8-f = tile 1, etc...
	x -= (8U - SPRITE_LEFT_BUFFER);
	y += SPRITE_TOP_BUFFER; // Add a buffer to the top of our sprite - we made it < 16px tall 
	
	temp16 = playerWorldTileStart + (MAP_TILES_ACROSS * (((UINT16)y>>4U) - 1U)) + (((UINT16)x)>>4U);
	temp3 = currentMap[temp16];
	
	if (temp3 >= FIRST_SOLID_TILE) {
		return 1U;
	}
	return 0U;
}

void handle_input() {
	
	oldBtns = btns;
	btns = joypad();
	
	// Special case for the trial to let us switch screens by holding select. 
	// Remove this if you do not want that functionality.
	if (btns & J_SELECT) {
		if (btns != oldBtns) {
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
		} // Else do nothing... just lock the controller.
		
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
	
	temp1 = playerX + playerXVel;
	temp2 = playerY + playerYVel;
	
	if (playerXVel != 0) {
		if (temp1 + SPRITE_SIZE >= SCREEN_WIDTH) {
			playerX = 8U + PLAYER_MOVE_DISTANCE;
			playerWorldPos++;
			load_map();
			return;
		} else if (temp1 <= 8U) {
			playerX = SCREEN_WIDTH - SPRITE_SIZE - PLAYER_MOVE_DISTANCE;
			playerWorldPos--;
			load_map();
			return;
		} else {
			if (playerXVel == PLAYER_MOVE_DISTANCE) {
				if (test_collision(temp1 + SPRITE_SIZE, temp2) || test_collision(temp1 + SPRITE_SIZE, temp2 + SPRITE_SIZE)) {
					temp1 = playerX;
				}
			} else {
				if (test_collision(temp1-1U, temp2) || test_collision(temp1-1U, temp2 + SPRITE_SIZE)) {
					temp1 = playerX;
				}
			}
		}
	}
	
	if (playerYVel != 0) {
		if (temp2 + SPRITE_SIZE >= SCREEN_HEIGHT) {
			playerY = SPRITE_SIZE + PLAYER_MOVE_DISTANCE;
			playerWorldPos += WORLD_ROW_HEIGHT;
			load_map();
			return;
		} else if (temp2 <= 8U) {
			playerY = (SCREEN_HEIGHT - STATUS_BAR_HEIGHT) - PLAYER_MOVE_DISTANCE;
			playerWorldPos -= WORLD_ROW_HEIGHT;
			load_map();
			return;
		} else {
			if (playerYVel <= PLAYER_MOVE_DISTANCE) {
				if (test_collision(temp1, temp2 + SPRITE_SIZE) || test_collision(temp1 + SPRITE_SIZE, temp2 + SPRITE_SIZE)) {
					temp2 = playerY;
				}
			} else {
				if (test_collision(temp1, temp2) || test_collision(temp1 + SPRITE_SIZE, temp2)) {
					temp2 = playerY;
				}
			}
		}
	}
	
	playerX = temp1;
	playerY = temp2;

	
	for (i = 0U; i < 4U; i++) {
		#if MAIN_CHARACTER_SPRITE_TYPE == MAIN_CHARACTER_SPRITE_SINGLE
			// This code will only be used if your game only has a single static sprite for the main character.
			if (playerXVel + playerYVel != 0U)
				set_sprite_tile(i, i);
		#elif MAIN_CHARACTER_SPRITE_TYPE == MAIN_CHARACTER_SPRITE_DIRECTIONAL
			// This code will only be used if your game has a multi-directional non-animated main character sprite.
			if (playerXVel + playerYVel != 0U)
				set_sprite_tile(i, ((playerDirection-1U)<<2U) + i);

		#elif MAIN_CHARACTER_SPRITE_TYPE == MAIN_CHARACTER_SPRITE_ANIMATED_DIRECTIONAL
			// This code will only be used if your game has a multi-directional animated main character sprite.
			if (playerXVel + playerYVel != 0U)
				set_sprite_tile(i, (((sys_time & PLAYER_ANIM_INTERVAL) >> PLAYER_ANIM_SHIFT)<<2U) + ((playerDirection-1U)<<3U) + i);
		#else
			#error "Unknown main character sprite type. Check the definition for MAIN_CHARACTER_SPRITE_TYPE"
		#endif

		move_sprite(i, playerX + (i%2U)*8U, playerY + (i/2U)*8U);
	}
	
	if (playerVelocityLock > 0)
		playerVelocityLock--;

}

void init_screen() {
	
	disable_interrupts();
	DISPLAY_OFF;
	
	SWITCH_ROM_MBC1(BANK_GRAPHICS);
	set_bkg_data(0U, 128U, TILES);
	set_win_data(0U, 128U, TILES);
	set_sprite_data(0U, 128U, SPRITES);

	load_map();
	
	scroll_bkg(0U, 0U);
	SPRITES_8x8;
	
	// Initialize main character sprite to something sane.
	for (i = 0; i != 4; i++)
		set_sprite_tile(i, i);
	
	SHOW_BKG;
	SHOW_SPRITES;

	move_win(0, 128);
	SHOW_WIN;
	
	DISPLAY_ON;
	enable_interrupts();

}

void update_health() {
	for (i = 0; i < playerHealth; i++)
		buffer[i] = WINDOW_TILE_HEALTH_FULL;

	for (; i < MAX_HEALTH; i++)
		buffer[i] = WINDOW_TILE_HEALTH_EMPTY;
	
	set_win_tiles(1U, 0U, MAX_HEALTH, 1U, buffer);

}

void main() {
	init_vars();
	
	// TODO: Built-in title screen?
	//SWITCH_ROM_MBC1(BANK_TITLE);
	//show_title();
	
	init_screen();
	
	update_health();
	
	while(1) {
		cycleCounter++;
		SWITCH_ROM_MBC1(BANK_MAP);
		handle_input();
		
		// Limit us to not-batnose-crazy speeds
		wait_vbl_done();
	}

}
