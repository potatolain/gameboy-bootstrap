#include <gb/gb.h>

#define SCREEN_WIDTH_TILES 20
#define SCREEN_HEIGHT_TILES 18
#define BLANK_TILE 0
#define TITLE_LINE_1 "Some Random"
#define TITLE_LINE_2 "Gameboy Game"
#define TITLE_LINE_3 ""
#define TITLE_LINE_4 "By Some Dude"
#define GAME_OVER "   Game Over    "
#define GAME_OVER_PRESS_START "- Press Start -"
#define BORDER_TILE_START 120
extern UBYTE title_tiles[];

void show_title();
void show_game_over();