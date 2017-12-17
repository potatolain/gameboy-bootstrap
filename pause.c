#include "main.h"
#include "game_config.h"
#include "variables.h"
#include "constants.h"
#include <gb/gb.h>


void show_pause() {
    gameState = GAME_STATE_PAUSED;
    
    // Clear the second row of hud
    for (i = 0; i != 20; ++i)
        buffer[i] = 0;
    set_win_tiles(0U, 1U, 20U, 1U, buffer);
    // Wait a frame for that change to apply, as the following functions also use buffer.
    wait_vbl_done();

    // We know the rest of buffer is still 0, so no need to fill it; we'll get it for free.
    // Fill the first row with PAUSE
    buffer[6] = '-' + WINDOW_TEXT_OFFSET;
    buffer[7] = 0;
    buffer[8] = 'P' + WINDOW_TEXT_OFFSET;
    buffer[9] = 'a' + WINDOW_TEXT_OFFSET;
    buffer[10] = 'u' + WINDOW_TEXT_OFFSET;
    buffer[11] = 's' + WINDOW_TEXT_OFFSET;
    buffer[12] = 'e' + WINDOW_TEXT_OFFSET;
    buffer[13] = 'd' + WINDOW_TEXT_OFFSET;
    buffer[14] = 0;
    buffer[15] = '-' + WINDOW_TEXT_OFFSET;
    
    set_win_tiles(0U, 0U, 20u, 1U, buffer);

}

void do_pause() {
    oldBtns = btns;
	btns = joypad();

    if (btns & J_START && !(oldBtns & J_START)) {

        // Clear eentire hud again - both rows
        for (i = 0; i != 20; i++)
            buffer[i] = 0;
        set_win_tiles(0U, 0U, 20U, 1U, buffer);

        // Wait a frame for that change to apply, as the following functions also use buffer.
        wait_vbl_done();
        
        // Same story, clear second row, wait another frame
        set_win_tiles(0U, 1U, 20U, 1U, buffer);
        wait_vbl_done();

        // Get the HUD back to normal
        update_money();
        // After updating money, wait a frame for that change to apply - otherwise we will overwrite buffer and only
        // end up updating health; ignoring money altogether.
        wait_vbl_done();

        update_health();
        gameState = GAME_STATE_RUNNING;
    }
}