#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h> // hardware-specific library
#include <SPI.h>
#include <mapAndDraw.h>
#include <enemyStruct.h>

// Color definitions
#define ENEMY     0xCC42

const int speed = 4000;

// spawn an enemy at a location
void drawEnemy(enemy theEnemies[15], uint8_t numEn, Adafruit_ST7735 *tft) {
  for (uint8_t i = 0; i < numEn; i++) {
    if(theEnemies[i].active){
      drawCircle(tft, theEnemies[i].x, theEnemies[i].y, ENEMY);
    }
  }
}

void changeDirection(enemy *bob) {
  uint8_t i;
  do {
    i = random(4);
  } while(bob->dir == i);
  bob->dir = i;
}


// move an enemy randomly
void moveEnemy(uint8_t map[20][16], enemy *bob) {
  uint8_t x = bob->x;
  uint8_t y = bob->y;

  if (bob->dir == 0) {
    x = constrain(x + 1, 0, 15);
  } else if (bob->dir == 1) {
    x = constrain(x - 1, 0, 15);
  } else if (bob->dir == 2) {
    y = constrain(y - 1, 0, 19);
  } else if (bob->dir == 3) {
    y = constrain(y + 1, 0, 19);
  }
  if ((map[y][x] == 0 || map[y][x] == 1) && (x != bob->x || y != bob->y)) {
    bob->y = y;
    bob->x = x;
  } else {
    changeDirection(bob);
    moveEnemy(map,bob);
  }
}

// move an enemy towards player;
void moveSmartEnemy(uint8_t map[20][16], enemy *bob) {

}

// move enemies
void moveAllEnemies(uint8_t map[20][16], enemy theEnemies[15], Adafruit_ST7735 *tft, uint8_t numEn) {
  for (uint8_t i = 0; i < numEn; i++) {
    if(theEnemies[i].active && (theEnemies[i].timeOfLastMove + speed) <= millis()){
      theEnemies[i].timeOfLastMove = millis();
      drawTile(tft, map, theEnemies[i].x, theEnemies[i].y);
      moveEnemy(map, &(theEnemies[i]));
    }
  }
  drawEnemy(theEnemies, numEn, tft);
}

// check if player is touching any enemy
bool touchingAnyEnemy(enemy theEnemies[15], uint8_t playerX, uint8_t playerY, uint8_t numEn) {
  for (uint8_t i = 0; i < numEn; i++) {
    if(theEnemies[i].isTouching(playerX,playerY)){
      return true;
    }
  }
  return false;
}

void createEnemies(enemy theEnemies[15], uint8_t numEn, uint8_t map[20][16]) {
  for (uint8_t i = 0; i < numEn; i++) {
    uint8_t y, x;
    do {
      x = random(15);//range: 0-15
      y = random(14)+6;// range: 6-19
    } while(map[y][x] != 0);//do until no tree at this location
    theEnemies[i].setXY(x,y);
    theEnemies[i].dir = random(4);
    theEnemies[i].setStatus(true);
    theEnemies[i].timeOfLastMove = millis();
  }
}

void killAllEnemies(enemy theEnemies[15], uint8_t numEn) {
  for (uint8_t i = 0; i < numEn; i++) {
    theEnemies[i].setStatus(false);
  }
}

void killEnemies(enemy theEnemies[15], uint8_t total, uint8_t x, uint8_t y, uint8_t* numEn, uint8_t explo_size) {
  for (uint8_t t = 0; t < total; t++) {
    if (theEnemies[t].isTouching(x,y)){
      theEnemies[t].setStatus(false);
      (*numEn)--;
    }
    for (uint8_t i = 1; i <= explo_size; i++) {
      if (theEnemies[t].isTouching(x+i,y)){
        theEnemies[t].setStatus(false);
        (*numEn)--;
      }

      if (theEnemies[t].isTouching(x-i, y)) {
        theEnemies[t].setStatus(false);
        (*numEn)--;
      }

      if (theEnemies[t].isTouching(x, y+i)){
        theEnemies[t].setStatus(false);
        (*numEn)--;
      }

      if (theEnemies[t].isTouching(x,y-i)) {
        theEnemies[t].setStatus(false);
        (*numEn)--;
      }
    }
  }
}
