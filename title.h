#include <gb/gb.h>

#define SCREEN_WIDTH_TILES 20
#define SCREEN_HEIGHT_TILES 18
#define BLANK_TILE 0
#define BORDER_TILE_START 95
extern UBYTE title_tiles[];

void show_title();
void show_game_over();
void show_winner_screen();