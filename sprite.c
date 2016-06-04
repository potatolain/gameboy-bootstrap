// Sprite functions are broken out here.
// Important note: This runs in a non-primary bank! 

#include <gb/gb.h>
#include <rand.h>
#include "variables.h"
#include "sprite.h"
#include "main.h"
#include "constants.h"

void clear_sprites_from_temp2() {
	while (temp2 != MAX_SPRITES) {
		// Fill in the rest -- both in actual sprites and in our structs.
		for (i = 0U; i < 4U; i++)
			move_sprite(WORLD_SPRITE_START + (temp2 << 2U) + i, SPRITE_OFFSCREEN, SPRITE_OFFSCREEN);
		
		mapSprites[temp2].type = SPRITE_TYPE_NONE;
		mapSprites[temp2].x = mapSprites[temp2].y = SPRITE_OFFSCREEN;
		mapSprites[temp2].size = 0U;
		temp2++;
	}
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