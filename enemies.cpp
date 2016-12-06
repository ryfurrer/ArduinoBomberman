#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h> // hardware-specific library
#include <SPI.h>
#include <mapAndDraw.h>
#include <enemyStruct.h>

// Color definitions
#define ENEMY     0xCC42

const int invSpeed = 500; // The inverse of the enemy's speed (ms/tile).
int points = 0;

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
  // keeps looping until bob gets a different direction than his current one
  do {
    i = random(4);
  } while(bob->dir == i);
  // set bob's direction to the new one
  bob->dir = i;
}


// move an enemy randomly
void moveEnemy(uint8_t map[20][16], enemy *bob) {
  // x and y are the tentative coordinates for bob to move to.
  uint8_t x = bob->x;
  uint8_t y = bob->y;

  // Depending on bob's direction, set the tentative coordinates of his
  // next position to 1 space in that direction.
  // Constrain this value within the boundaries of the map.
  if (bob->dir == 0) {
    x = constrain(x + 1, 0, 15);
  } else if (bob->dir == 1) {
    x = constrain(x - 1, 0, 15);
  } else if (bob->dir == 2) {
    y = constrain(y - 1, 0, 19);
  } else if (bob->dir == 3) {
    y = constrain(y + 1, 0, 19);
  }

  // If bob is about to move to a valid tile (grass or dead tree), AND bob's
  // next position is not his current position, then apply bob's new coords.
  // Else, get bob a new direction and try again, until he recieves valid
  // coordinates to move to.
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
  // for each of the remaining enemies
  for (uint8_t i = 0; i < numEn; i++) {
    // if the enemy is active AND is ready to move
    if(theEnemies[i].active && (theEnemies[i].timeOfLastMove + theEnemies[i].timeToNextMove) <= millis()){
      // reset his timeOfLastMove, and decrease the amount of spaces to move.
      theEnemies[i].timeOfLastMove = millis();
      theEnemies[i].spacesToMove--;
      // Check if the enemy still needs to move after this move.
      if (theEnemies[i].spacesToMove > 0) {
        // To calculate when the enemy will next move : t = d/v = 1 tile / speed
        // = 1 tile * invSpeed
        theEnemies[i].timeToNextMove = 1 * invSpeed;
      }
      // If the enemy has no more spacesToMove, make him wait between 2 and 5
      // seconds before he moves again.
      // Also, set the amount of spaces he will move when he moves again
      // and give him a new direction to move in.
      else if (theEnemies[i].spacesToMove <= 0) {
        theEnemies[i].timeToNextMove = random(2000,5000);
        theEnemies[i].spacesToMove = random(1,6);
        Serial.print("Waiting for: "); Serial.println(theEnemies[i].timeToNextMove);
        Serial.print("Spaces to Move: "); Serial.println(theEnemies[i].spacesToMove);
        // changeDirection(&(theEnemies[i]));
      }
      // Draw over his old position
      drawTile(tft, map, theEnemies[i].x, theEnemies[i].y);
      // Get his new coordinates to move to.
      moveEnemy(map, &(theEnemies[i]));
    }
  }
  // Move the enemy to his new location.
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
    theEnemies[i].timeToNextMove = random(2000,5000);
    theEnemies[i].spacesToMove = random(1,6);
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
      points += 100;
    }
    for (uint8_t i = 1; i <= explo_size; i++) {
      if (theEnemies[t].isTouching(x+i,y)){
        theEnemies[t].setStatus(false);
        (*numEn)--;
        points += 100;
      }

      if (theEnemies[t].isTouching(x-i, y)) {
        theEnemies[t].setStatus(false);
        (*numEn)--;
        points += 100;
      }

      if (theEnemies[t].isTouching(x, y+i)){
        theEnemies[t].setStatus(false);
        (*numEn)--;
        points += 100;
      }

      if (theEnemies[t].isTouching(x,y-i)) {
        theEnemies[t].setStatus(false);
        (*numEn)--;
        points += 100;
      }
    }
  }
}
