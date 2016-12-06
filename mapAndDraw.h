#ifndef _map_func_
#define  _map_func_

#include <treeStruct.h>

void loadMap(uint8_t map[20][16]);
void init_map(uint8_t map[20][16]);
void setupTFT();

void printStartItem(uint8_t i);
void printInstructions();
void printDeath(unsigned long totalPoints);
void printWin();

void drawTile(uint8_t x, uint8_t y, int color);
void drawTile(uint8_t map[20][16], uint8_t x, uint8_t y);
void drawBomb(uint8_t x, uint8_t y);
void drawExplosion(uint8_t x, uint8_t y, uint8_t explo_size);
void drawTree(uint8_t treeTile[8][8], uint8_t x, uint8_t y);
void drawCircle(uint8_t x, uint8_t y, int color);

void addGridOfBlocks(uint8_t map[20][16], uint8_t space, uint8_t num);
void addSemiRandomBlocks(uint8_t map[20][16], uint8_t type);

void addTrees(tree trees[40], uint8_t map[20][16]);
void resetTrees(tree trees[40], uint8_t map[20][16]);
void removeTree(uint8_t map[20][16], uint8_t y, uint8_t x, uint8_t num);
void removeTrees(uint8_t map[20][16], uint8_t x, uint8_t y, uint8_t explo_size);

uint8_t getRandom(uint8_t max);

#endif
