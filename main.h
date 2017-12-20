// Some functions in main that need to be available to everyone
#include <gb/gb.h>
void update_health();
void update_money();
UBYTE test_collision(UBYTE x, UBYTE y);
void load_next_map(UINT16 id);
void draw_sprite_anim_state();