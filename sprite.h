#ifndef SPRITE_H
#define SPRITE_H
#include <gb/gb.h>

#define SPRITE_ANIM_INTERVAL 0x08U
#define SPRITE_ANIM_SHIFT 3U
#define MAX_SPRITES 6U
#define SPRITE_OFFSCREEN 255U
#define SPRITE_SPEED 2U

enum SPRITE_TYPE {
	SPRITE_TYPE_TBD = 0U,
	SPRITE_TYPE_NONE = 250U
};

enum SPRITE_DIRECTION {
	SPRITE_DIRECTION_STOP = 0U,
	SPRITE_DIRECTION_UP = 1U,
	SPRITE_DIRECTION_DOWN = 2U,
	SPRITE_DIRECTION_LEFT = 3U,
	SPRITE_DIRECTION_RIGHT = 4U
};

struct SPRITE {
	UBYTE x;
	UBYTE y;
	UBYTE size;
	UBYTE type;
	enum SPRITE_DIRECTION direction;
};

void clear_sprites_from_temp2();
void test_sprite_collision();
void move_enemy_sprite();
void directionalize_sprites();

#endif