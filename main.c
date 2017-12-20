// FIXME: Switch example back to arcade tileset - readme specifies that and it's PD, so harder to mess up if someone wanted to get angry!
#include "main.h"
#include "constants.h"
#include "sprite.h"
#include "title.h"
#include "pause.h"
#include "game_config.h"
#include "title_config.h"
#include "scroll_anim.h"
#include <gb/gb.h>
#include <rand.h>

UBYTE temp1, temp2, temp3, temp4, temp5, temp6, i, j;
UBYTE playerWorldPos, playerX, playerY, btns, oldBtns, playerXVel, playerYVel, gameState, playerVelocityLock, cycleCounter;
UBYTE playerHealth, playerMoney;
UBYTE buffer[32U];
UBYTE playerInvulnTime;
UINT16 temp16, temp16b, playerWorldTileStart;
UBYTE* bankedCurrentMap;
UBYTE* bankedNextMap;
UBYTE* * * currentMapSprites; // Triple pointer, so intense!!
UBYTE* tempPointer; 
UBYTE lockScrollToBottom;

UBYTE worldState[64U];
UBYTE bitLookup[6U];
UBYTE currentMap[0x80];
UBYTE nextMap[0x80];

struct SPRITE mapSprites[6];

enum SPRITE_DIRECTION playerDirection;

void init_vars() {
	gameState = GAME_STATE_RUNNING;
	
	playerWorldPos = 0U;
	btns = oldBtns = 0U;
	playerXVel = playerYVel = 0U;
	playerX = playerY = 32U;
	
	playerHealth = STARTING_HEALTH;
	playerMoney = STARTING_MONEY;
	playerVelocityLock = 0U;
	playerInvulnTime = 0u;
	lockScrollToBottom = 0U;
	
	// TODO: Convert this to a const array, or document why it isn't one.
	bitLookup[0] = 1U;
	bitLookup[1] = 2U;
	bitLookup[2] = 4U;
	bitLookup[3] = 8U;
	bitLookup[4] = 16U;
	bitLookup[5] = 32U;
	
	// Doesn't matter for first run, but if we restart (game over, etc) we don't want to leave things in a funky state.
	for (i = 0; i != 64; i++)
		worldState[i] = 0U;

}

// Used in a vblank method to make sure 
void set_xscroll_zero() {
	SCX_REG = 0;
}

// Call during vblank to keep the scroll at the stop position for the lower screen, so we can redraw the screen
// without weird artifacts
void vbl() {
	if (lockScrollToBottom == SCROLL_DIRECTION_RIGHT) {
		move_bkg(SCREEN_WIDTH, SCREEN_HEIGHT);
	} else if (lockScrollToBottom == SCROLL_DIRECTION_LEFT) {
		move_bkg(LEFT_SCROLL_STOP, SCREEN_HEIGHT);
	}
}

void load_map() {
	SWITCH_ROM_MBC1(BANK_MAP);
	bankedCurrentMap = MAP;
	currentMapSprites = MAP_SPRITES;
	playerWorldTileStart = (UINT16)playerWorldPos * (UINT16)MAP_TILE_SIZE;
	temp1 = 0;

	for (i = 0U; i != MAP_TILES_DOWN; i++) {
		for (j = 0U; j != MAP_TILES_ACROSS; j++) {
			buffer[j*2U] = bankedCurrentMap[playerWorldTileStart + j] << 2U; 
			buffer[j*2U+1U] = buffer[j*2U]+1U;
			currentMap[temp1] = bankedCurrentMap[playerWorldTileStart + j];
			temp1++;
		}
		temp1 += 6; // 6 bytes of padding per row to simplify math. May find a use for this later.

		set_bkg_tiles(0U, i << 1U, 20U, 1U, buffer);
		
		for (j = 0U; j != MAP_TILES_ACROSS*2; j++) {
			buffer[j]+=2;
		}
		set_bkg_tiles(0U, i*2U+1U, 20U, 1U, buffer);
		playerWorldTileStart += MAP_TILES_ACROSS;

	}
	
	tempPointer = currentMapSprites[playerWorldPos];
	temp1 = 0x00; // Generic data
	temp2 = 0U; // Position
	// Not added to the sprite file, because this depends on map data in a separate bank, and we can only have one bank loaded at a time. 
	// There are ways to do this - could have map lookups in the primary file, but that would get more confusing and weird.
	while(temp2 != MAX_SPRITES) {
		temp1 = tempPointer++[0];
		// 255 indicates the end of the array. Bail out!
		if (temp1 == 255U)
			break;
		
		// If this sprite exists in worldState, move on.
		temp3 = worldState[playerWorldPos];
		temp4 = bitLookup[temp2];
		if ((temp3 & temp4) == 0U) {

			// Type is really the id of the sprite... could do with improving this...
			mapSprites[temp2].type = tempPointer++[0];
			mapSprites[temp2].direction = SPRITE_DIRECTION_STOP;
			
			SWITCH_ROM_MBC1(BANK_SPRITES);
			load_sprite(); // WARNING: load_sprite destroys temp1 -- the data in it is GONE past this point.
			SWITCH_ROM_MBC1(BANK_MAP);
		} else {
			// Clean up anything left behind.
			for (i = 0U; i < 4U; i++)
				move_sprite(WORLD_SPRITE_START + (temp2 << 2U) + i, SPRITE_OFFSCREEN, SPRITE_OFFSCREEN);
			// Advance the pointer...
			tempPointer++;
			
			mapSprites[temp2].type = SPRITE_TYPE_NONE;

		}
		temp2++;
	}
	
	SWITCH_ROM_MBC1(BANK_SPRITES);
	clear_sprites_from_temp2();
	
	// Leave this as it was - we depend on it elsewhere.
	playerWorldTileStart = (UINT16)playerWorldPos * (UINT16)MAP_TILE_SIZE;

}

void load_next_map(UINT16 id) {
	UINT16 tempid = id * (UINT16)MAP_TILE_SIZE;
	SWITCH_ROM_MBC1(BANK_MAP);
	bankedNextMap = MAP;
	temp1 = 0;

	for (i = 0U; i != MAP_TILES_DOWN; i++) {
		for (j = 0U; j != MAP_TILES_ACROSS; j++) {
			nextMap[temp1] = bankedNextMap[tempid + j];
			temp1++;
		}
		temp1 += 6; // 6 bytes of padding per row to simplify math. May find a use for this later.
		tempid += MAP_TILES_ACROSS;

	}
	
}


UBYTE test_collision(UBYTE x, UBYTE y) {

	// This offsets us by one tile to get us in line with 0-7= tile 0, 8-f = tile 1, etc...
	x -= (8U - SPRITE_LEFT_BUFFER);
	y += SPRITE_TOP_BUFFER; // Add a buffer to the top of our sprite - we made it < 16px tall 
	
	temp3 = currentMap[((y & 0xf0) - 16) + (x>>4)];

	
	if (temp3 >= FIRST_DAMAGE_TILE) {
		temp3 = COLLISION_TYPE_DAMAGE;
	} else if (temp3 >= FIRST_SOLID_TILE) {
		temp3 = COLLISION_TYPE_SOLID;
	} else {
		temp3 = COLLISION_TYPE_NONE;
	}
	return temp3;
}

void init_screen() {
	
	disable_interrupts();
	DISPLAY_OFF;
	
	SWITCH_ROM_MBC1(BANK_GRAPHICS);
	// REMEMBER: Graphics are set up with weird overlaps - we could be smart about this, but for now we limit you to 128 tiles in each section.
	set_bkg_data(0U, 0, TILES);
	set_sprite_data(0U, 128U, SPRITES); // This is also why we limit ourselves to 128 sprites.

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
	
	// Always, ALWAYS force the window to stay at 0 to avoid glitchy movement.
	STAT_REG = 0x45;
	LYC_REG = SCREEN_HEIGHT;
	set_interrupts(VBL_IFLAG | LCD_IFLAG);
	add_LCD(set_xscroll_zero);
	add_VBL(vbl);

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

void update_money() {
	buffer[0] = WINDOW_TILE_MONEY;
	buffer[1] = WINDOW_TILE_NUMERIC_0 + (playerMoney / 10);
	buffer[2] = WINDOW_TILE_NUMERIC_0 + (playerMoney % 10);
	
	set_win_tiles(17U, 1U, 3U, 1U, buffer);
}

void damage_player(UBYTE amount) {
	if (amount >= playerHealth) {
		gameState = GAME_STATE_GAME_OVER;
		playerHealth = 0;
	} else {
		playerHealth -= amount;
	}
	update_health();
	
	if (playerXVel == 0U && playerYVel == 0) {
		// If the player wasn't "moving" give a sane velocity swap
		playerYVel = PLAYER_MOVE_DISTANCE;
	} else {
		// Otherwise reverse whatever they were doing.
		playerXVel = 0U-playerXVel;
		playerYVel = 0U-playerYVel;
	}
	playerVelocityLock = DAMAGE_COLLISION_LOCK_TIME;
	playerInvulnTime = DAMAGE_COLLISION_INVULN_TIME;
}

// NOTE: If we need kernel space, this could be moved to a separate bank as long as show_pause() and load_map were
// turned into separate gameStates called in the nmi method, rather than calling them directly here.
void handle_input() {
	
	oldBtns = btns;
	btns = joypad();

	if (btns & J_START && !(oldBtns & J_START)) {
		SWITCH_ROM_MBC1(BANK_PAUSE);
		show_pause();
	}
	
	if (!playerVelocityLock) {
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
		if (temp1 + HALF_SPRITE_WIDTH >= SCREEN_WIDTH) {
			playerX = 8U + PLAYER_MOVE_DISTANCE;
			playerWorldPos++;
			load_next_map(playerWorldPos);
			SWITCH_ROM_MBC1(BANK_SCROLL_ANIM);
			do_scroll_anim(SCROLL_DIRECTION_RIGHT);
			lockScrollToBottom = SCROLL_DIRECTION_RIGHT;
			load_map();
			lockScrollToBottom = 0;
			move_bkg(0, 0);
			return;
		} else if (temp1 <= 8U) {
			playerX = SCREEN_WIDTH - SPRITE_WIDTH - PLAYER_MOVE_DISTANCE;
			playerWorldPos--;
			load_next_map(playerWorldPos);
			SWITCH_ROM_MBC1(BANK_SCROLL_ANIM);
			do_scroll_anim(SCROLL_DIRECTION_LEFT);
			lockScrollToBottom = SCROLL_DIRECTION_LEFT;
			load_map();
			lockScrollToBottom = 0;
			move_bkg(0, 0);			return;
		} else {
			if (playerXVel == PLAYER_MOVE_DISTANCE) {
				if (test_collision(temp1 + SPRITE_WIDTH, temp2) || test_collision(temp1 + SPRITE_WIDTH, temp2 + SPRITE_HEIGHT)) {
					if (temp3 == COLLISION_TYPE_DAMAGE && !playerInvulnTime) {
						damage_player(1U);
					} else {
						temp1 = playerX;
					}
				}
			} else {
				if (test_collision(temp1-1U, temp2) || test_collision(temp1-1U, temp2 + SPRITE_HEIGHT)) {
					if (temp3 == COLLISION_TYPE_DAMAGE && !playerInvulnTime) {
						damage_player(1U);
					} else {
						temp1 = playerX;
					}
				}
			}
		}
	}
	
	if (playerYVel != 0) {
		if (temp2 + HALF_SPRITE_HEIGHT >= SCREEN_HEIGHT) {
			playerY = SPRITE_HEIGHT + PLAYER_MOVE_DISTANCE;
			playerWorldPos += WORLD_ROW_HEIGHT;
			load_next_map(playerWorldPos);
			SWITCH_ROM_MBC1(BANK_SCROLL_ANIM);
			do_scroll_anim(SCROLL_DIRECTION_DOWN);
			load_map();
			move_bkg(0, 0);
			return;
		} else if (temp2 <= 12U) {
			playerY = (SCREEN_HEIGHT - 4) - PLAYER_MOVE_DISTANCE;
			playerWorldPos -= WORLD_ROW_HEIGHT;
			load_next_map(playerWorldPos);
			SWITCH_ROM_MBC1(BANK_SCROLL_ANIM);
			do_scroll_anim(SCROLL_DIRECTION_UP);
			load_map();
			move_bkg(0, 0);
			return;
		} else {
			if (playerYVel <= PLAYER_MOVE_DISTANCE) {
				if (test_collision(temp1, temp2 + SPRITE_HEIGHT) || test_collision(temp1 + SPRITE_WIDTH, temp2 + SPRITE_HEIGHT)) {
					if (temp3 == COLLISION_TYPE_DAMAGE && !playerInvulnTime) {
						damage_player(1U);
					} else {
						temp2 = playerY;
					}				
				}
			} else {
				if (test_collision(temp1, temp2) || test_collision(temp1 + SPRITE_WIDTH, temp2)) {
					if (temp3 == COLLISION_TYPE_DAMAGE && !playerInvulnTime) {
						damage_player(1U);
					} else {
						temp2 = playerY;
					}				
				}
			}
		}
	}
	
	playerX = temp1;
	playerY = temp2;

	
	for (i = 0U; i < 4U; i++) {
		draw_sprite_anim_state();

		// This little bit of trickery flashes the sprite if the player is in the invulnerability period after getting hurt.
		if (!playerInvulnTime || playerInvulnTime & 0x04) {
			move_sprite(i, playerX + (i%2U)*8U, playerY + (i/2U)*8U);
		} else {
			move_sprite(i, SPRITE_OFFSCREEN, SPRITE_OFFSCREEN);
		}
	}
	
	if (playerVelocityLock > 0)
		--playerVelocityLock;

	if (playerInvulnTime > 0)
		--playerInvulnTime;

}

void draw_sprite_anim_state() {
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

}

void main() {
	
	// Wrap the entirety of our main in a loop, so when the user exits, we can put ourselves back into a "like-new" state.
	// Just set gameState to something, check in the inner for loop, and break if it's time to reboot.
	while (1) {
		init_vars();
				
		SWITCH_ROM_MBC1(BANK_TITLE);
		show_title();
		
		init_screen();
		
		update_health();
		update_money();
		initrand(sys_time);
		
		// Game loop. Does all the things.
		while(1) {

			if (gameState == GAME_STATE_RUNNING) {
				++cycleCounter;
				SWITCH_ROM_MBC1(BANK_MAP);
				handle_input();
				SWITCH_ROM_MBC1(BANK_SPRITES);
				move_sprites();
				
				if (!playerVelocityLock) {
					SWITCH_ROM_MBC1(BANK_SPRITES);
					test_sprite_collision();
				}
			} else if (gameState == GAME_STATE_PAUSED) {
				SWITCH_ROM_MBC1(BANK_PAUSE);
				do_pause();
			} else if (gameState == GAME_STATE_GAME_OVER) {				
				SWITCH_ROM_MBC1(BANK_TITLE);
				show_game_over();
				break; // Break out of the game loop and start this whole mess all over again...

			} else if (gameState == GAME_STATE_WINNER) {
				SWITCH_ROM_MBC1(BANK_TITLE);
				show_winner_screen();
				break; // Break out of the game loop and the let them play again.
			}
			
			// Limit us to not-batnose-crazy speeds
			wait_vbl_done();
		}
	}

}
