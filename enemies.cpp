/*
Tim Tran
Ryan Furrer
CMPUT 274 EA1
*/

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
void drawEnemy(enemy theEnemies[15], uint8_t numEn) {
  for (uint8_t i = 0; i < numEn; i++) {
    if(theEnemies[i].active){
      drawCircle(theEnemies[i].getX(), theEnemies[i].getY(), ENEMY);
    }
  }
}

void changeDirection(enemy *bob) {
  uint8_t i, t = 0;
  // keeps looping until bob gets a different direction than his current one
  do {
    i = random(4);
    t++;
  } while(bob->dir == i && t <= 4);
  // set bob's direction to the new one
  bob->dir = i;
}


// move an enemy randomly
void moveEnemy(uint8_t map[20][16], enemy *bob, uint8_t recursions) {
  // x and y are the tentative coordinates for bob to move to.
  uint8_t x = bob->getX();
  uint8_t y = bob->getY();

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
  if ((map[y][x] == 0 || map[y][x] == 1) && (x != bob->getX() || y != bob->getY())) {
    map[bob->getY()][bob->getX()] -= 10;
    bob->setXY(x,y);
    map[bob->getY()][bob->getX()] += 10;
  } else if(recursions < 6){
    recursions++;
    changeDirection(bob);
    moveEnemy(map, bob, recursions);
  }
}

// move an enemy towards player;
void moveSmartEnemy(uint8_t map[20][16], enemy *bob) {

}

// move enemies
void moveAllEnemies(uint8_t map[20][16], enemy theEnemies[15], uint8_t numEn) {
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
        theEnemies[i].timeToNextMove = random(2000,4000);
        theEnemies[i].spacesToMove = random(1,5);
        // changeDirection(&(theEnemies[i]));
      }
      // Draw over his old position
      drawTile(map, theEnemies[i].getX(), theEnemies[i].getY());
      // Get his new coordinates to move to.
      moveEnemy(map, &(theEnemies[i]), 0);
    }
  }
  // Move the enemy to his new location.
  drawEnemy(theEnemies, numEn);
}

void createEnemies(enemy theEnemies[15], uint8_t numEn, uint8_t map[20][16]) {
  for (uint8_t i = 0; i < numEn; i++) {
    uint8_t y, x;
    do {
      x = random(15);//range: 0-15
      y = random(14)+6;// range: 6-19
    } while(map[y][x] % 10 != 0);//do until no tree at this location
    map[y][x] += 10;
    theEnemies[i].setXY(x,y);
    theEnemies[i].dir = random(4);
    theEnemies[i].setStatus(true);
    theEnemies[i].timeOfLastMove = millis();
    theEnemies[i].timeToNextMove = random(2000,5000);
    theEnemies[i].spacesToMove = random(1,6);
  }
}

void killAllEnemies(uint8_t map[20][16], enemy theEnemies[15], uint8_t numEn) {
  for (uint8_t i = 0; i < numEn; i++) {
    if (theEnemies[i].active) {
      theEnemies[i].setStatus(false);
      map[theEnemies[i].getY()][theEnemies[i].getX()] -= 10;
    }
  }
}

//O(nlog(n))
// void qsort(enemy theEnemies[15]) {
//
// }

// Swap two enemies of enemy struct
void swap_enemies(enemy *ptr_enemy1, enemy *ptr_enemy2) {
  enemy tmp = *ptr_enemy1;
  *ptr_enemy1 = *ptr_enemy2;
  *ptr_enemy2 = tmp;
}


int pick_pivot(enemy theEnemies[15], int len) {
  return len/2;
}


int partition(enemy theEnemies[15], int len, int pivot_idx) {
  swap_enemies(&theEnemies[pivot_idx], &theEnemies[len-1]);
  int pivotVal = theEnemies[len-1].getYXandStat();
  int lower = 0;
  int upper = len-2;
  while (lower < upper) {
    if (theEnemies[lower].getYXandStat() <= pivotVal) {
      lower++;
    }
    else if (theEnemies[upper].getYXandStat() > pivotVal) {
      upper--;
    }
    else {
      swap_enemies(&theEnemies[lower], &theEnemies[upper]);
    }
  }// theEnemies[upper] will be the first value greater than the pivot as long as
  //the pivot isn't the highest value after leaving this loop

  // if statement to check if the index is the highest value
  if (theEnemies[len-1].getYXandStat() < theEnemies[upper].getYXandStat()) {
    swap_enemies(&theEnemies[upper], &theEnemies[len-1]);
    pivot_idx = upper;
  }
  else {
    pivot_idx = len-1;
  }
  return pivot_idx;
}

/*
  Sort enemies by their x and y coordinates and status
  Living, smallest yx to greatest yx then dead
  < O(nlog(n))
*/
void qsort(enemy theEnemies[15], int len) {
  if (len <= 1) return; // sorted already

  // choose the pivot
  int pivot_idx = pick_pivot(theEnemies, len);

  // partition around the pivot and get the new pivot position
  pivot_idx = partition(theEnemies, len, pivot_idx);

  // recurse on the halves before and after the pivot
  qsort(theEnemies, pivot_idx);

  // only the active ones need to be sorted.
  if (theEnemies[pivot_idx].active) { //only dead ones after if false
    qsort(theEnemies + pivot_idx + 1, len - pivot_idx - 1);
  }
}


//O(log(n))
int binarySearch(enemy theEnemies[15], uint8_t x, uint8_t y, uint8_t numEn){
  int yx = x + (y<<4);
  int low = 0;
  int high = numEn - 1;

  while (low <= high) {
    int mid = (low + high) / 2;

    if (theEnemies[mid].active && theEnemies[mid].yx == yx) {
      return mid;
    } else if (theEnemies[mid].yx > yx) {
      high = mid -1;
    } else if (theEnemies[mid].yx < yx) {
      low = mid +1;
    }
  }
  return -1;//should never happen since we only call this if we now an enemy is there
  //ie: logic ERROR
}

/*
  Removes enmies that are in explosions
  O(numEn+explo_size)*log(numEn))
*/
void killEnemies(uint8_t map[20][16], enemy theEnemies[15], uint8_t maxEnemies, uint8_t x, uint8_t y, uint8_t* numEn, uint8_t explo_size) {
  qsort(theEnemies, maxEnemies);

  if (map[y][x] > 9){
    int t = binarySearch(theEnemies, x, y, *numEn);
    if (t == -1) {
      Serial.println("Center Error: binary search = -1");
    }
    theEnemies[t].setStatus(false);
    (*numEn)--;
    points += 100;
    map[y][x] -= 10;
  }
  for (uint8_t i = 1; i <= explo_size; i++) {
    if(x+i<16){
      while (map[y][x+i] > 9){
        int t = binarySearch(theEnemies, x+i, y, *numEn);
        if (t == -1) {
          Serial.println("left Error: binary search = -1");//loops infinitly if true
        } else {
          theEnemies[t].setStatus(false);
          (*numEn)--;
          points += 100;
          map[y][x+i] -= 10;
        }
      }
    }

    if(x >= i){
      while (map[y][x-i] > 9) {
        int t = binarySearch(theEnemies, x-i, y, *numEn);
        if (t == -1) {
          Serial.println("right Error: binary search = -1");
        } else {
          theEnemies[t].setStatus(false);
          (*numEn)--;
          points += 100;
          map[y][x-i] -= 10;
        }
      }
    }

    if(y+i<20) {
      while (map[y+i][x] > 9){
        int t = binarySearch(theEnemies, x, y+i, *numEn);
        if (t == -1) {
          Serial.println("up Error: binary search = -1");
        } else {
          theEnemies[t].setStatus(false);
          (*numEn)--;
          points += 100;
          map[y+i][x] -= 10;
        }
      }
    }

    if(y>i){
      while (map[y-i][x] > 9) {
        int t = binarySearch(theEnemies, x, y-i, *numEn);
        if (t == -1) {
          Serial.println("down Error: binary search = -1");
        } else {
          theEnemies[t].setStatus(false);
          (*numEn)--;
          points += 100;
          map[y-i][x] -= 10;
        }
      }
    }
  }
}
