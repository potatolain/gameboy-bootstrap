// A list of a bunch of things that will never change that are shared amongst a few modules.
#ifndef CONSTANTS_H
#define CONSTANTS_H

#define BANK_GRAPHICS		1
#define BANK_MAP			2
#define BANK_TITLE			3
#define BANK_SPRITES		4
#define PLAYER_MOVE_DISTANCE 2
#define DAMAGE_COLLISION_LOCK_TIME 25U

#define MAP_TILE_SIZE 80U
#define WORLD_MAX_TILE 64U
#define WORLD_ROW_HEIGHT 8U
#define SPRITE_WIDTH 11U
#define HALF_SPRITE_WIDTH 6U
#define SPRITE_HEIGHT 12U
#define HALF_SPRITE_HEIGHT 6U
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

#endif