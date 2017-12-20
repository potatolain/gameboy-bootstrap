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
                temp5 = SCREEN_HEIGHT - VERTICAL_SCREEN_BUFFER_DOWN;
                for (j = 0; j != SCREEN_HEIGHT; j+=2) {
                    wait_vbl_done();
                    move_bkg(0, j);
                    temp5 -= 2;
                    // This doesn't quite go down far enough, but not enough to adjust every frame. So... tweak it
                    // slightly every 16 frames.
                    if (j % 16 == 0) {
                        temp5 += 1;
                    }
                    for(i = 0; i != 4; ++i) {
                        move_sprite(i, playerX + (i%2U)*8U, temp5 + (j >> 3) + (i/2U)*8U);
                        draw_sprite_anim_state();                        
                    }
                }  
            } else {
                temp5 = 0 - VERTICAL_SCREEN_BUFFER_UP;
                // Uh, guess I'll go find myself then.
                for (j = 0; j != SCREEN_HEIGHT - 2; j-=2) {
                    wait_vbl_done();
                    move_bkg(0, j);
                    temp5 += 2;
                    for(i = 0; i != 4; ++i) {
                        move_sprite(i, playerX + (i%2U)*8U, temp5 + (j >> 3) + (i/2U)*8U);
                        draw_sprite_anim_state();                        
                    }
                }  

            }


            // Restore the walk speed from before we messed with it.
            playerYVel = temp2;

            break;
        case SCROLL_DIRECTION_RIGHT:
            // Left and right are a little more complicated - Our screens are 160px wide, but the window is only
            // 256 px. So, we have to get a bit clever, draw half the screen, scroll halfway, then draw the rest.
            
            // Copy everything to right below this map, including the first sliver of the next room over
            for (i = 0; i != MAP_TILES_DOWN; i++) {
                for (j = 0; j != MAP_TILES_ACROSS; j++) {
                    buffer[j*2] = currentMap[(i<<4) + j] << 2;
                    buffer[j*2 + 1] = buffer[j*2] + 1;
                }
                for (; j != MAP_TILES_ACROSS + 6; j++) {
                    buffer[j*2] = nextMap[(i<<4) + j-MAP_TILES_ACROSS] << 2;
                    buffer[j*2 + 1] = buffer[j*2] + 1;
                }
                set_bkg_tiles(0, (MAP_TILES_DOWN << 1) + (i << 1U), 32, 1, buffer);

                for (j = 0; j != 32; j++) {
                    buffer[j] += 2;
                }
                set_bkg_tiles(0, (MAP_TILES_DOWN << 1) + (i << 1U)+1, 32, 1, buffer);
            }

            // hardcode the player's speed to make sure we trigger the walk animation
            temp2 = playerYVel;
            playerYVel = 55;

            clear_non_player_sprites();

            temp5 = SCREEN_WIDTH - HORIZONTAL_SCREEN_BUFFER_RIGHT;
            // Now, scroll to halfway across
            for (j = 0; j != SCREEN_WIDTH>>1; j+=2) {
                wait_vbl_done();
                temp5 -= 2;
                move_bkg(j, SCREEN_HEIGHT);
                for(i = 0; i != 4; ++i) {
                    move_sprite(i, temp5 + (j >> 3) + (i%2U)*8U, playerY + (i/2U)*8U);
                    draw_sprite_anim_state();                        
                }
            }
            temp4 = j;

            // Draw the second sliver over the first bits
            for (i = 0; i != MAP_TILES_DOWN; i++) {
                for (j = 0; j != 4; j++) {
                    buffer[j*2] = nextMap[(i<<4) + j + (MAP_TILES_ACROSS>>1) + 1] << 2;
                    buffer[j*2 + 1] = buffer[j*2] + 1;
                }
                set_bkg_tiles(0, (MAP_TILES_DOWN << 1) + (i << 1U), 8, 1, buffer);

                for (j = 0; j != 8; j++) {
                    buffer[j] += 2;
                }
                set_bkg_tiles(0, (MAP_TILES_DOWN << 1) + (i << 1U)+1, 8, 1, buffer);
                if (i % 4 == 0) {
                    // Do this regularly to avoid stuttering
                    wait_vbl_done();
                    temp4 += 2;
                    temp5 -= 2;
                    move_bkg(temp4, SCREEN_HEIGHT);
                }
            }

            // Scroll the rest of the way
            for (j = temp4; j != SCREEN_WIDTH; j+=2) {
                wait_vbl_done();
                move_bkg(j, SCREEN_HEIGHT);
                temp5 -= 2;
                for(i = 0; i != 4; ++i) {
                    move_sprite(i, temp5 + (j >> 3) + (i%2U)*8U, playerY + (i/2U)*8U);
                    draw_sprite_anim_state();                        
                }
            } 
            move_bkg(SCREEN_WIDTH, SCREEN_HEIGHT);


            // Put back velocity from earlier.
            playerYVel = temp2; 

            break;
        case SCROLL_DIRECTION_LEFT:
            // Do something similar to the above, but in the reverse order so we can scroll to the left instead!

            // Copy everything to right below this map, including the first sliver of the next room over
            for (i = 0; i != MAP_TILES_DOWN; i++) {
                for (j = 0; j != 6; j++) {
                    buffer[j*2] = nextMap[(i<<4) + j+4] << 2;
                    buffer[j*2 + 1] = buffer[j*2] + 1;
                }

                for (; j != MAP_TILES_ACROSS + 6; j++) {
                    buffer[j*2] = currentMap[(i<<4) + j - 6] << 2;
                    buffer[j*2 + 1] = buffer[j*2] + 1;
                }
                set_bkg_tiles(0, (MAP_TILES_DOWN << 1) + (i << 1U), 32, 1, buffer);

                for (j = 0; j != 32; j++) {
                    buffer[j] += 2;
                }
                set_bkg_tiles(0, (MAP_TILES_DOWN << 1) + (i << 1U)+1, 32, 1, buffer);
            }

            // hardcode the player's speed to make sure we trigger the walk animation
            temp2 = playerYVel;
            playerYVel = 55;

            clear_non_player_sprites();

            // Now, scroll to halfway across
            temp5 = 0 + HORIZONTAL_SCREEN_BUFFER_LEFT;
            for (j = SCREEN_WIDTH>>1; j != 0; j -= 2) {
                wait_vbl_done();
                move_bkg(j, SCREEN_HEIGHT);
                temp5 += 2;
                for(i = 0; i != 4; ++i) {
                    move_sprite(i, temp5 - (temp5 >> 4) + (i%2U)*8U, playerY + (i/2U)*8U);
                    draw_sprite_anim_state();                        
                }
            }
            temp4 = j;

            // Draw the second sliver over the first bits
            for (i = 0; i != MAP_TILES_DOWN; i++) {
                for (j = 0; j != 4; j++) {
                    buffer[j*2] = nextMap[(i<<4) + j] << 2;
                    buffer[j*2 + 1] = buffer[j*2] + 1;
                }
                set_bkg_tiles(24, (MAP_TILES_DOWN << 1) + (i << 1U), 8, 1, buffer);

                for (j = 0; j != 8; j++) {
                    buffer[j] += 2;
                }
                set_bkg_tiles(24, (MAP_TILES_DOWN << 1) + (i << 1U)+1, 8, 1, buffer);
                if (i % 4 == 0) {
                    // Do this regularly to avoid stuttering
                    wait_vbl_done();
                    temp4 -= 2;
                    temp5 += 2;
                    move_bkg(temp4, SCREEN_HEIGHT);
                }
            }

            // Scroll the rest of the way
            for (j = temp4; j != LEFT_SCROLL_STOP; j-=2) {
                wait_vbl_done();
                move_bkg(j, SCREEN_HEIGHT);
                temp5 += 2;
                for(i = 0; i != 4; ++i) {
                    move_sprite(i, temp5 - (temp5 >> 4) + (i%2U)*8U, playerY + (i/2U)*8U);
                    draw_sprite_anim_state();                        
                }
            } 
            move_bkg(LEFT_SCROLL_STOP, SCREEN_HEIGHT);


            // Put back velocity from earlier.
            playerYVel = temp2; 
            break;
        default: 
            break;
    }
}