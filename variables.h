// Yep... these are all globals. Limited memory, gameboy, poor compiler support for local vars, etc.
// If this is grossing you out, please stop thinking about it like C. Think about it like the assembly behind it,
// and also keep in mind that we're working in a very limited system. Tricks will be necessary.
#include <gb/gb.h>
#ifndef VARIABLES_H
#define VARIABLES_H
extern UBYTE temp1, temp2, temp3, temp4, temp5, temp6, i, j;
extern UBYTE playerWorldPos, playerX, playerY, btns, oldBtns, playerXVel, playerYVel, gameState, playerVelocityLock, cycleCounter;
extern UBYTE playerHealth;
extern UBYTE playerHealth, playerMoney;
extern UBYTE buffer[20U];
extern UBYTE playerInvulnTime;
extern UINT16 temp16, temp16b, playerWorldTileStart;
extern UBYTE* bankedCurrentMap;
extern UBYTE* * currentMapSprites; // Triple pointer, so intense!!
extern UBYTE* tempPointer; 

extern UBYTE worldState[64U];
extern UBYTE bitLookup[6U];

extern UBYTE currentMap[80];

extern struct SPRITE mapSprites[6];
#endif