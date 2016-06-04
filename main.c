#include "main.h"
#include "sprite.h"
#include "title.h"
#include "title_config.h"
#include <gb/gb.h>
#include <rand.h>

#define BANK_GRAPHICS		1
#define BANK_MAP			2
#define BANK_TITLE			3
#define PLAYER_MOVE_DISTANCE 2
#define DAMAGE_COLLISION_LOCK_TIME 25U

#define MAP_TILE_SIZE 80U
#define WORLD_MAX_TILE 64U
#define WORLD_ROW_HEIGHT 8U
#define SPRITE_WIDTH 11U
#define SPRITE_HEIGHT 12U
#define SPRITE_LEFT_BUFFER 2U
#define SPRITE_TOP_BUFFER 2U

#define SCREEN_WIDTH 160U
#define SCREEN_HEIGHT 128U // 144 - 16px status bar
#define STATUS_BAR_HEIGHT 16U

#define COLLISION_TYPE_NONE 0U
#define COLLISION_TYPE_SOLID 1U
#define COLLISION_TYPE_DAMAGE 2U

#define GAME_STATE_RUNNING 0U
#define GAME_STATE_GAME_OVER 1U
#define GAME_STATE_WINNER 50U

#define MAP_TILES_DOWN 8U
#define MAP_TILES_ACROSS 10U
UBYTE temp1, temp2, temp3, temp4, temp5, temp6, i, j;
UBYTE playerWorldPos, playerX, playerY, btns, oldBtns, playerXVel, playerYVel, gameState, playerVelocityLock, cycleCounter;
UBYTE playerHealth, playerMoney;
UBYTE buffer[20U];
UINT16 temp16, temp16b, playerWorldTileStart;
UBYTE* currentMap;
UBYTE* * * currentMapSprites; // Triple pointer, so intense!!
UBYTE* tempPointer; 

UBYTE worldState[64U];
UBYTE bitLookup[6U];

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

void load_map() {
	SWITCH_ROM_MBC1(BANK_MAP);
	currentMap = MAP;
	currentMapSprites = MAP_SPRITES;
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
	
	tempPointer = currentMapSprites[playerWorldPos];
	temp1 = 0x00; // Generic data
	temp2 = 0U; // Position
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

			// Apply it to the 2x2 big sprites (encompasses both enemy sprites and the endgame sprites)
			if (mapSprites[temp2].type <= LAST_DOOR_SPRITE) {
				mapSprites[temp2].size = 16U;

				// Temp1 is our position.. convert to x/y
				mapSprites[temp2].x = ((temp1 % MAP_TILES_ACROSS) << 4U) + 8U; // add 8 to deal with offset by 1.
				mapSprites[temp2].y = ((temp1 / MAP_TILES_ACROSS) << 4U) + 16U; // Add 16 so the first tile = 16

				for (i = 0; i != 4U; i++) {
					set_sprite_tile(WORLD_SPRITE_START + (temp2 << 2U) + i, ENEMY_SPRITE_START + (mapSprites[temp2].type << 2U) + i);
					move_sprite(WORLD_SPRITE_START + (temp2 << 2U) + i, mapSprites[temp2].x + ((i%2U) << 3U), mapSprites[temp2].y + ((i/2U) << 3U));
				}
			} else if (mapSprites[temp2].type <= LAST_MONEY_SPRITE) { // And also apply it to the rest of our smaller sprites - health, and money.
				mapSprites[temp2].size = 8U;
				
				// Temp1 is our position.. convert to x/y
				mapSprites[temp2].x = ((temp1 % MAP_TILES_ACROSS) << 4U) + 12U; // Add 8 to deal with offset by 1, and center
				mapSprites[temp2].y = ((temp1 / MAP_TILES_ACROSS) << 4U) + 20U; // Add 16 so the first tile = 16, then add 4 to center.
				// Put our sprite on the map!
				set_sprite_tile(WORLD_SPRITE_START + (temp2 << 2U), HEALTH_SPRITE_START + (mapSprites[temp2].type - FIRST_8PX_SPRITE));
				move_sprite(WORLD_SPRITE_START + (temp2 << 2U), mapSprites[temp2].x, mapSprites[temp2].y);
				for (i = 1; i != 4U; i++) {
					move_sprite(WORLD_SPRITE_START + (temp2 << 2U) + i, SPRITE_OFFSCREEN, SPRITE_OFFSCREEN);
				}
			}
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
	
	while (temp2 != MAX_SPRITES) {
		// Fill in the rest -- both in actual sprites and in our structs.
		for (i = 0U; i < 4U; i++)
			move_sprite(WORLD_SPRITE_START + (temp2 << 2U) + i, SPRITE_OFFSCREEN, SPRITE_OFFSCREEN);
		
		mapSprites[temp2].type = SPRITE_TYPE_NONE;
		mapSprites[temp2].x = mapSprites[temp2].y = SPRITE_OFFSCREEN;
		mapSprites[temp2].size = 0U;
		temp2++;
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
}

void test_sprite_collision() {
	UBYTE spriteWidth, spriteHeight;
	for (i = 0U; i < MAX_SPRITES; i++) {
		spriteWidth = mapSprites[i].size;
		spriteHeight = mapSprites[i].size;
		// Offset from center, used for mini sprites.
		temp1 = 0;

		if (playerX < mapSprites[i].x + spriteWidth && playerX + SPRITE_WIDTH > mapSprites[i].x && 
				playerY < mapSprites[i].y + spriteHeight && playerY + SPRITE_HEIGHT > mapSprites[i].y) {
			
			if (mapSprites[i].type <= LAST_ENEMY_SPRITE) {
				if (playerHealth < 2) {
					gameState = GAME_STATE_GAME_OVER;
					return;
				}

				playerHealth--;
				update_health();
				playerVelocityLock = DAMAGE_COLLISION_LOCK_TIME;
				if (playerXVel == 0 && playerYVel == 0) {
					playerYVel = PLAYER_MOVE_DISTANCE;
				} else {
					playerYVel = 0U-playerYVel;
					playerXVel = 0U-playerXVel;
				}
				return;
			} else if (mapSprites[i].type <= LAST_ENDGAME_SPRITE) {
				gameState = GAME_STATE_WINNER;
			} else if (mapSprites[i].type <= LAST_DOOR_SPRITE) {
				if (playerMoney >= DOOR_COST) {
					playerMoney -= DOOR_COST;
					mapSprites[i].type = SPRITE_TYPE_NONE;
					for(j = 0; j != 4; j++) {
						move_sprite(WORLD_SPRITE_START + (i << 2U) + j, SPRITE_OFFSCREEN, SPRITE_OFFSCREEN);
					}
					worldState[playerWorldPos] |= 1U << i;
					update_money();
				} else {
					playerVelocityLock = 1;
					if (playerXVel == 0 && playerYVel == 0) {
						// Should never happen!
						playerYVel = PLAYER_MOVE_DISTANCE;
					} else {
						playerYVel = 0U-playerYVel;
						playerXVel = 0U-playerXVel;
					}
				}
			} else if (mapSprites[i].type <= LAST_HEALTH_SPRITE) {
				if (playerHealth < MAX_HEALTH)
					playerHealth++;
				
				mapSprites[i].type = SPRITE_TYPE_NONE;
				move_sprite(WORLD_SPRITE_START + (i << 2U), SPRITE_OFFSCREEN, SPRITE_OFFSCREEN);
				worldState[playerWorldPos] |= 1U << i;

				update_health();
			} else if (mapSprites[i].type <= LAST_MONEY_SPRITE) {
				if (playerMoney < MAX_MONEY) 
					playerMoney++;
				mapSprites[i].type = SPRITE_TYPE_NONE;
				move_sprite(WORLD_SPRITE_START + (i << 2U), SPRITE_OFFSCREEN, SPRITE_OFFSCREEN);
				worldState[playerWorldPos] |= 1U << i;

				update_money();
			}
			
		}
	}
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
		if (temp1 + SPRITE_WIDTH >= SCREEN_WIDTH) {
			playerX = 8U + PLAYER_MOVE_DISTANCE;
			playerWorldPos++;
			load_map();
			return;
		} else if (temp1 <= 8U) {
			playerX = SCREEN_WIDTH - SPRITE_WIDTH - PLAYER_MOVE_DISTANCE;
			playerWorldPos--;
			load_map();
			return;
		} else {
			if (playerXVel == PLAYER_MOVE_DISTANCE) {
				if (test_collision(temp1 + SPRITE_WIDTH, temp2) || test_collision(temp1 + SPRITE_WIDTH, temp2 + SPRITE_HEIGHT)) {
					if (temp3 == COLLISION_TYPE_DAMAGE) {
						damage_player(1U);
					} else {
						temp1 = playerX;
					}
				}
			} else {
				if (test_collision(temp1-1U, temp2) || test_collision(temp1-1U, temp2 + SPRITE_HEIGHT)) {
					if (temp3 == COLLISION_TYPE_DAMAGE) {
						damage_player(1U);
					} else {
						temp1 = playerX;
					}
				}
			}
		}
	}
	
	if (playerYVel != 0) {
		if (temp2 + SPRITE_HEIGHT >= SCREEN_HEIGHT) {
			playerY = SPRITE_HEIGHT + PLAYER_MOVE_DISTANCE;
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
				if (test_collision(temp1, temp2 + SPRITE_HEIGHT) || test_collision(temp1 + SPRITE_WIDTH, temp2 + SPRITE_HEIGHT)) {
					if (temp3 == COLLISION_TYPE_DAMAGE) {
						damage_player(1U);
					} else {
						temp2 = playerY;
					}				
				}
			} else {
				if (test_collision(temp1, temp2) || test_collision(temp1 + SPRITE_WIDTH, temp2)) {
					if (temp3 == COLLISION_TYPE_DAMAGE) {
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

void directionalize_sprites() {
	// Kind of bizarre, but it gives us a good variation.
	if (cycleCounter % 60U < MAX_SPRITES) {
		temp3 = rand() % 32U;
		if (temp3 > SPRITE_DIRECTION_DOWN) {
			if (temp3 < 9) {
				temp3 = SPRITE_DIRECTION_STOP;
			} else if (temp3 < 21) {
				temp4 = playerX + (mapSprites[temp1].size/2U);
				temp5 = mapSprites[temp1].x + (mapSprites[temp1].size/2U);
				if (temp3 % 2 == 0)
					temp3 = SPRITE_DIRECTION_LEFT;
				else
					temp3 = SPRITE_DIRECTION_RIGHT;
			} else {
				temp4 = playerY + (mapSprites[temp1].size/2U);
				temp5 = mapSprites[temp1].y + (mapSprites[temp1].size/2U);
				if (temp3 % 2 == 0)
					temp3 = SPRITE_DIRECTION_UP;
				else
					temp3 = SPRITE_DIRECTION_DOWN;

			}
		}
		mapSprites[temp1].direction = temp3;
	}
	
	temp4 = mapSprites[temp1].x;
	temp5 = mapSprites[temp1].y;
	switch (mapSprites[temp1].direction) {
		case SPRITE_DIRECTION_LEFT: 
			temp4 -= SPRITE_SPEED;
			break;
		case SPRITE_DIRECTION_RIGHT:
			temp4 += SPRITE_SPEED;
			break;
		case SPRITE_DIRECTION_UP:
			temp5 -= SPRITE_SPEED;
			break;
		case SPRITE_DIRECTION_DOWN:
			temp5 += SPRITE_SPEED;
			break;
	}
	
}

void move_enemy_sprite() {
	mapSprites[temp1].x = temp4;
	mapSprites[temp1].y = temp5;
	
	for (i = 0; i != 4U; i++) {
		// set_sprite_tile(WORLD_SPRITE_START + (temp1 << 2U) + i, ENEMY_SPRITE_START + (mapSprites[temp1].type << 2U) + ((sys_time & SPRITE_ANIM_INTERVAL) >> SPRITE_ANIM_SHIFT) + i);
		// TODO: We have no sprite animation yet, so we can't do the anim thing
		set_sprite_tile(WORLD_SPRITE_START + (temp1 << 2U) + i, ENEMY_SPRITE_START + (mapSprites[temp1].type << 2U) + i);
		move_sprite(WORLD_SPRITE_START + (temp1 << 2U) + i, temp4 + ((i%2) << 3), temp5 + ((i/2) << 3));
	}

}


void move_sprites() {
	temp1 = cycleCounter % MAX_SPRITES;
	if (mapSprites[temp1].type == SPRITE_TYPE_NONE || mapSprites[temp1].type > LAST_ENEMY_SPRITE)
		return;
	directionalize_sprites();
		
	SWITCH_ROM_MBC1(BANK_MAP);	
	// Now, we test collision with our temp4 and temp5
		
	if (mapSprites[temp1].direction == SPRITE_DIRECTION_STOP)
		return;

	// mapSprites[temp1].size is our sprite width.
	if (mapSprites[temp1].direction == SPRITE_DIRECTION_LEFT || mapSprites[temp1].direction == SPRITE_DIRECTION_RIGHT) {
		if (temp4+mapSprites[temp1].size >= SCREEN_WIDTH || temp4 <= 4U) {
			temp4 = mapSprites[temp1].x;
		} else {
			if (mapSprites[temp1].direction == SPRITE_DIRECTION_RIGHT) {
				if (test_collision(temp4+mapSprites[temp1].size, temp5) || test_collision(temp4 + mapSprites[temp1].size, temp5+mapSprites[temp1].size)) {
					temp4 = mapSprites[temp1].x;
				}
			} else {
				if (test_collision(temp4-1U, temp5) || test_collision(temp4-1U, temp5+mapSprites[temp1].size)) {
					temp4 = mapSprites[temp1].x;
				}
			}
		}
	}
	
	if (mapSprites[temp1].direction == SPRITE_DIRECTION_UP || mapSprites[temp1].direction == SPRITE_DIRECTION_DOWN) {
		if (temp5+mapSprites[temp1].size >= SCREEN_HEIGHT || temp5 <= 4U) {
			temp5 = mapSprites[temp1].y;
		} else {
			if (mapSprites[temp1].direction == SPRITE_DIRECTION_DOWN) {
				if (test_collision(temp4, temp5+mapSprites[temp1].size) || test_collision(temp4+mapSprites[temp1].size, temp5 + mapSprites[temp1].size)) {
					temp5 = mapSprites[temp1].y;
				}
			} else {
				if (test_collision(temp4, temp5) || test_collision(temp4 + mapSprites[temp1].size, temp5)) {
					temp5 = mapSprites[temp1].y;
				}
			}
		}
	}
	
	// Okay, you can move.
	move_enemy_sprite();
	

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
			cycleCounter++;
			SWITCH_ROM_MBC1(BANK_MAP);
			handle_input();
			move_sprites();
			
			if (!playerVelocityLock) {
				test_sprite_collision();
			}
			
			if (gameState == GAME_STATE_GAME_OVER) {				
				SWITCH_ROM_MBC1(BANK_TITLE);
				show_game_over();
				break; // Break out of the game loop and start this whole mess all over again...

			}
			
			if (gameState == GAME_STATE_WINNER) {
				SWITCH_ROM_MBC1(BANK_TITLE);
				show_winner_screen();
				break; // Break out of the game loop and the let them play again.
			}
			
			// Limit us to not-batnose-crazy speeds
			wait_vbl_done();
		}
	}

}
