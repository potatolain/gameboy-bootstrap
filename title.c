// Font is from: http://opengameart.org/content/a-package-of-8-bit-fonts-for-grafx2-and-linux
#include "title.h"
#include "main.h"
#include "variables.h"
#include <gb/gb.h>

void clear_buffer() {
	for (i = 0; i < 20; i++) {
		buffer[i] = BLANK_TILE;
	}
}

void clear_to_border() {
	for (i = 0; i < 20; i++) {
		buffer[i] = BORDER_TILE_START+1;
	}
	buffer[0] = BORDER_TILE_START;
	buffer[19] = BORDER_TILE_START+2;
	set_bkg_tiles(0U, 0U, 20U, 1U, buffer);
	
	clear_buffer();
	buffer[0] = BORDER_TILE_START+3;
	buffer[19] = BORDER_TILE_START+4;
	for (i = 1; i < SCREEN_HEIGHT_TILES-1; i++) {
		set_bkg_tiles(0U, i, 20U, 1U, buffer);
	}

	for (i = 0; i < 20; i++) {
		buffer[i] = BORDER_TILE_START+6;
	}
	buffer[0] = BORDER_TILE_START+5;
	buffer[19] = BORDER_TILE_START+7;
	set_bkg_tiles(0U, SCREEN_HEIGHT_TILES-1, 20U, 1U, buffer);
}

void init_title() {
	
	disable_interrupts();
	DISPLAY_OFF;
	HIDE_SPRITES;
	HIDE_WIN;
	
	set_bkg_data(0U, 0, title_tiles);

	clear_to_border();
	
	clear_buffer();
	for (i = 0; i < 16; i++) {
		if (TITLE_LINE_1[i] == NULL)
			break;
		
		buffer[i] = TITLE_LINE_1[i] - 32U;
	}
	set_bkg_tiles(2U, 6U, 16U, 1U, buffer);
	
	clear_buffer();
	for (i = 0; i < 16; i++) {
		if (TITLE_LINE_2[i] == NULL)
			break;
		
		buffer[i] = TITLE_LINE_2[i] - 32U;
	}
	set_bkg_tiles(2U, 7U, 16U, 1U, buffer);

	clear_buffer();
	for (i = 0; i < 16; i++) {
		if (TITLE_LINE_3[i] == NULL)
			break;
		
		buffer[i] = TITLE_LINE_3[i] - 32U;
	}
	set_bkg_tiles(2U, 8U, 16U, 1U, buffer);

	clear_buffer();
	for (i = 0; i < 16; i++) {
		if (TITLE_LINE_4[i] == NULL)
			break;
		
		buffer[i] = TITLE_LINE_4[i] - 32U;
	}
	set_bkg_tiles(2U, 9U, 16U, 1U, buffer);

	
	scroll_bkg(0U, 0U);
		
	SHOW_BKG;
	
	DISPLAY_ON;
	enable_interrupts();
}

void init_game_over() {
	disable_interrupts();
	DISPLAY_OFF;
	HIDE_SPRITES;
	HIDE_WIN;
	
	set_bkg_data(0U, 0, title_tiles);

	clear_to_border();
	
	clear_buffer();
	for (i = 0; i < 16; i++) {
		if (GAME_OVER[i] == NULL)
			break;
		
		buffer[i] = GAME_OVER[i] - 32U;
	}
	set_bkg_tiles(2U, 6U, 16U, 1U, buffer);
	
	clear_buffer();
	for (i = 0; i < 16; i++) {
		if (GAME_OVER_PRESS_START[i] == NULL)
			break;
		
		buffer[i] = GAME_OVER_PRESS_START[i] - 32U;
	}
	set_bkg_tiles(2U, 9U, 16U, 1U, buffer);
	
	scroll_bkg(0U, 0U);
		
	SHOW_BKG;
	
	DISPLAY_ON;
	enable_interrupts();
}

void do_input_loop() {
	waitpadup();
	oldBtns = btns;
	btns = joypad();
	while (!(!(oldBtns & J_START) && btns & J_START)) {
		oldBtns = btns;
		btns = joypad();
		wait_vbl_done();
	}
}

void show_title() {
	init_title();
	do_input_loop();
}

void show_game_over() {
	init_game_over();
	do_input_loop();
}