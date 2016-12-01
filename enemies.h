#ifndef SYMBOL
#define  value

void drawEnemy(enemy theEnemies[15], uint8_t numEn, Adafruit_ST7735 *tft);
void changeDirection(enemy *bob);
void moveEnemy(uint8_t map[20][16], enemy *bob);
void moveSmartEnemy(uint8_t map[20][16], enemy *bob);
void moveAllEnemies(uint8_t map[20][16], enemy theEnemies[15], Adafruit_ST7735 *tft, uint8_t numEn);
bool touchingAnyEnemy(enemy theEnemies[15], uint8_t playerX, uint8_t playerY, uint8_t numEn);
void createEnemies(enemy theEnemies[15], uint8_t numEn, uint8_t map[20][16]);
void killAllEnemies(enemy theEnemies[15], uint8_t numEn);
void killEnemies(enemy theEnemies[15], uint8_t total, uint8_t x, uint8_t y, uint8_t* numEn, uint8_t explo_size);

#endif
