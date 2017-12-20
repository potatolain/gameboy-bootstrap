#include <gb/gb.h>
#include <rand.h>
#include "variables.h"
#include "main.h"
#include "sprite.h"
#include "game_config.h"
#include "constants.h"

// Remove all non-player sprites from the screen so we can have a clean transition
// NOTE: this duplicates some behavior from the sprite file, but reproducing it is easier than swapping banks here.
// Additionally, it's easier to understand and potentially even faster. So, yeah, we're gonna do that.
void clear_non_player_sprites() {
    temp2 = 0;
    while (temp2 != MAX_SPRITES) {
        for (i = 0U; i < 4U; i++) {
            move_sprite(WORLD_SPRITE_START + (temp2 << 2U) + i, SPRITE_OFFSCREEN, SPRITE_OFFSCREEN);
        }

        mapSprites[temp2].type = SPRITE_TYPE_NONE;
        mapSprites[temp2].x = mapSprites[temp2].y = SPRITE_OFFSCREEN;
        mapSprites[temp2].size = 0U;    
        temp2++;
    }

}

// Scroll the screen to whatever is in nextMap using the direction given.
void do_scroll_anim(UBYTE direction) {
    switch (direction) {
        case SCROLL_DIRECTION_UP:
        case SCROLL_DIRECTION_DOWN:
            // Up/down are the easy case - we've got enough room to draw the entire map below our current map, so we
            // just do it all at once then scroll down, then redraw over the top and reset scroll. Super simple.
            if (direction == SCROLL_DIRECTION_UP) {
                playerDirection = SPRITE_DIRECTION_UP;
            } else {
                playerDirection = SPRITE_DIRECTION_DOWN;
            }

            for (i = 0; i != MAP_TILES_DOWN; i++) {
                for (j = 0; j != MAP_TILES_ACROSS; j++) {
                    buffer[j*2] = nextMap[(i<<4) + j] << 2;
                    buffer[j*2 + 1] = buffer[j*2] + 1;
                }
                set_bkg_tiles(0, (MAP_TILES_DOWN << 1) + (i << 1U), 20, 1, buffer);

                for (j = 0; j != MAP_TILES_ACROSS*2; j++) {
                    buffer[j] += 2;
                }
                set_bkg_tiles(0, (MAP_TILES_DOWN << 1) + (i << 1U)+1, 20, 1, buffer);
            }

            // Ok, the screen's loaded. Let's scroll it!
            
            // hardcode the player's speed to make sure we trigger the walk animation
            temp2 = playerYVel;
            playerYVel = 55;

            clear_non_player_sprites();

            if (direction == SCROLL_DIRECTION_DOWN) {
                for (j = 0; j != SCREEN_HEIGHT; j+=2) {
                    wait_vbl_done();
                    move_bkg(0, j);
                    for(i = 0; i != 4; ++i) {
                        move_sprite(i, playerX + (i%2U)*8U, (SCREEN_HEIGHT - j - VERTICAL_SCREEN_BUFFER_DOWN) + (j >> 3) + (i/2U)*8U);
                        draw_sprite_anim_state();                        
                    }
                }  
            } else {
                // Uh, guess I'll go find myself then.
                for (j = 0; j != SCREEN_HEIGHT - 2; j-=2) {
                    wait_vbl_done();
                    move_bkg(0, j);
                    for(i = 0; i != 4; ++i) {
                        move_sprite(i, playerX + (i%2U)*8U, (0u - j - VERTICAL_SCREEN_BUFFER_UP) + (j >> 3) + (i/2U)*8U);
                        draw_sprite_anim_state();                        
                    }
                }  

            }


            // Restore the walk speed from before we messed with it.
            playerYVel = temp2;

            break;
        case SCROLL_DIRECTION_LEFT:
            break;
        case SCROLL_DIRECTION_RIGHT:
            break;
        default: 
            break;
    }
}