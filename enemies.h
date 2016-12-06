#ifndef SYMBOL
#define  value

void drawEnemy(enemy theEnemies[15], uint8_t numEn);
void changeDirection(enemy *bob);
void moveEnemy(uint8_t map[20][16], enemy *bob);
void moveSmartEnemy(uint8_t map[20][16], enemy *bob);
void moveAllEnemies(uint8_t map[20][16], enemy theEnemies[15], uint8_t numEn);
bool touchingAnyEnemy(enemy theEnemies[15], uint8_t playerX, uint8_t playerY, uint8_t numEn);
void createEnemies(enemy theEnemies[15], uint8_t numEn, uint8_t map[20][16]);
void killAllEnemies(uint8_t map[20][16], enemy theEnemies[15], uint8_t numEn);
void killEnemies(uint8_t map[20][16], enemy theEnemies[15], uint8_t maxEnemies, uint8_t x, uint8_t y, uint8_t* numEn, uint8_t explo_size);
void showPoints();

int binarySearch(enemy theEnemies[15], uint8_t x, uint8_t y, uint8_t numEn);
void qsort(enemy theEnemies[15], int len);
int partition(enemy theEnemies[15], int len, int pivot_idx);
int pick_pivot(enemy theEnemies[15], int len);
void swap_enemies(enemy *ptr_enemy1, enemy *ptr_enemy2);

#endif
