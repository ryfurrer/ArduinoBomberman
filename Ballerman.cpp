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
#include <enemies.h>

// standard U of A library settings, assuming Atmel Mega SPI pins
#define SD_CS    5  // Chip select line for SD card
#define TFT_CS   6  // Chip select line for TFT display
#define TFT_DC   7  // Data/command line for TFT
#define TFT_RST  8  // Reset line for TFT (or connect to +5V)

// Screen Dimensions
#define TFT_HEIGHT 160
#define TFT_WIDTH 128

#define JOY_SEL 9
#define JOY_VERT_ANALOG 0
#define JOY_HORIZ_ANALOG 1

#define JOY_DEADZONE 128 // Only care about joystick movement if
// position is JOY_CENTRE +/- JOY_DEADZONE
#define JOY_CENTRE 512
#define MILLIS_PER_FRAME 120

// Color definitions
#define BLACK    0x0000
#define RED      0xF800
#define WHITE    0xFFFF


//consants
const uint8_t maxBombCount = 5;
const int explodeTime = 1000; // how long it takes for bombs to explode in ms (tweak later)
const uint8_t explosionDuration = 250; // how long an explosion lasts for

// For point calculations
unsigned long startTime = 0;
unsigned int totalPoints = 0;
unsigned long enemiesKilled = 0;

// bombs
struct bomb{
  uint8_t x;
  uint8_t y;
  unsigned long time;
  unsigned long timeOfExplosion;
  bool active;
  bool exploding;
};

tree trees[40];
enemy theEnemies[15];
uint8_t maxEnemies = 0;
uint8_t numEn = 0;

// Map Arrays
// 0 = Grass
// 1 = Dirt
// 2 = Water
// 3 = Tree
// 4 = STONE
// 5 = Tree with hidden item
// 6 = powerup
uint8_t map1[20][16];

// Important Variables
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);


uint8_t selection = 0; // To highlight which the selected restaurant in the menu
uint8_t oldSelection = 0; // To store which restaurant was previously selected

uint8_t playerPosX = 0;
uint8_t playerPosY = 0;
uint8_t oldPlayerPosX = 0;
uint8_t oldPlayerPosY = 0;
uint8_t bombCount = 0;
bool isAlive = true;


bomb activeBombs[5]; // store the player's bombs in here
uint8_t explo_size = 1;         //make power up to increase radius



bool getPlayerInput();
void playerMove();
void setup();
void bombExplode(int x, int y);
void setBomb(int x, int y);
void updateBombs();
void removeExplosion(int x, int y);
void menuScroll(int vert);
void changeHighlightedItem();
void gameEnded();
bool explodePlayer(int playerPosX, int playerPosY, int explosionPosX, int explosionPosY);
void checkPowerUps();
void calculatePoints (unsigned long enemiesKilled, unsigned long clearTime);
bool touchingAnyEnemy(uint8_t playerX, uint8_t playerY);

// General functions

// check if player is touching any enemy
bool touchingAnyEnemy(uint8_t playerX, uint8_t playerY) {
  if(map1[playerY][playerX] > 9){
    return true;
  }
  return false;
}

void updateBombs() {
  unsigned long currentTime;

  for (int i = 0; i < maxBombCount; i++) {
    currentTime = millis();
    // check if the bombs are not exploding
    if (activeBombs[i].active && !(activeBombs[i].exploding)) {
      // check if the bombs are going to explode
      if ((currentTime - activeBombs[i].time) > explodeTime) {
        Serial.print("Bomb "); Serial.print(i); Serial.println(" exploding.");
        Serial.print("Time since set: "); Serial.println(currentTime - activeBombs[i].time);
        // explode the bomb at its current position
        bombExplode(activeBombs[i].x, activeBombs[i].y);
        // set the bomb's condition to exploding, and store its time of Explosion
        activeBombs[i].exploding = true;
        activeBombs[i].timeOfExplosion = millis();
      }
      // if not going to explode, draw the bomb at that location
      else {
        drawBomb(activeBombs[i].x, activeBombs[i].y);
      }
    }
    // if the bomb is exploding
    else if (activeBombs[i].exploding) {
      // check if it should still be exploding
      if ((currentTime - activeBombs[i].timeOfExplosion) > explosionDuration) {
        // if the explosion duration is over, destroy surrounding trees
        activeBombs[i].exploding = false;
        activeBombs[i].active = false;
        removeTrees(map1, activeBombs[i].x, activeBombs[i].y, explo_size);
        removeExplosion(activeBombs[i].x, activeBombs[i].y);
      }
      else {
        drawExplosion(activeBombs[i].x, activeBombs[i].y, explo_size);
        killEnemies(map1, theEnemies, maxEnemies, activeBombs[i].x, activeBombs[i].y, &numEn, explo_size);
        explodePlayer(playerPosX, playerPosY, activeBombs[i].x, activeBombs[i].y);
      }
    }
  }
}

// Move the player around the screen
void updatePlayer() {
  drawTile(map1, oldPlayerPosX, oldPlayerPosY);
  playerMove();
}

//
void bombExplode(int x, int y) {
  drawTile(map1, x, y);
  bombCount--;
  Serial.print("bombCount: "); Serial.println(bombCount);
}

// Redraws the map over an explosion
void removeExplosion(int x, int y) {
  for (int i = x - explo_size; i <= x+(explo_size); i++) {
    drawTile(map1, i, y);
  }
  for (int i = y-explo_size; i <= y+(explo_size); i++) {
    drawTile(map1, x, i);
  }
}


// Player functions

// Call this when the player dies.
void gameEnded() {

  for (int i = 0; i < maxBombCount; i++) {
    activeBombs[i].exploding = false;
    activeBombs[i].active = false;
  }
}

// Checks if the player is within an explosion
bool explodePlayer(int playerPosX, int playerPosY, int explosionPosX, int explosionPosY) {
  // If the player is within the explosion radius, call playerDead()
  if (((explosionPosX - explo_size) <= playerPosX) && (playerPosX <= (explosionPosX + explo_size)) && (playerPosY == explosionPosY)) {
    isAlive = false;
    return false;
  }
  else if (((explosionPosY - explo_size) <= playerPosY) && (playerPosY <= (explosionPosY + explo_size)) && (playerPosX == explosionPosX)) {
    isAlive = false;
    return false;
  }
  return true;
}

bool getPlayerInput() {
  int vert = analogRead(JOY_VERT_ANALOG);
  int horiz = analogRead(JOY_HORIZ_ANALOG);
  int select = digitalRead(JOY_SEL);

  // Check for select
  if (select == 0) {
    if (bombCount < maxBombCount) {
      Serial.println("Setting bomb");
      setBomb(playerPosX, playerPosY);
      return true;
    }
  }

  // Check for Joystick movement
  if (abs(horiz - JOY_CENTRE) > JOY_DEADZONE) {
    int temp;
    if (horiz > JOY_CENTRE) {
      temp = constrain(playerPosX + 1, 0, 15);
    }
    else if (horiz < JOY_CENTRE) {
      temp = constrain(playerPosX - 1, 0, 15);
    }
    if (map1[playerPosY][temp] % 10 == 0 || map1[playerPosY][temp] % 10 == 1 || map1[playerPosY][temp] % 10 == 6) {       //constrain movement to ground
      playerPosX = temp;
      return true; // return so that they can't go diagonal
    }
  }

  if (abs(vert - JOY_CENTRE) > JOY_DEADZONE) { // not "else if" in case horiz movement is blocked
    int temp;
    if (vert > JOY_CENTRE) {
      temp = constrain(playerPosY + 1, 0, 19);
    }
    else if (vert < JOY_CENTRE) {
      temp = constrain(playerPosY - 1, 0, 19);
    }

    if (map1[temp][playerPosX] % 10 == 0 || map1[temp][playerPosX] % 10 == 1 || map1[temp][playerPosX] % 10 == 6) {       //constrain movement to ground
      playerPosY = temp;
      return true;
    }
  }

  return false;
}

void setBomb(int x, int y) {
  bombCount++;
  // Serial.print("bombCount: "); Serial.println(bombCount);
  for (int i = 0; i < maxBombCount; i++) {
    if (activeBombs[i].active == false) {
      Serial.print("Bomb "); Serial.print(i); Serial.println(" is active.");
      activeBombs[i].active = true;
      activeBombs[i].x = x;
      activeBombs[i].y = y;
      activeBombs[i].time = millis();
      break;
    }
  }
}

void playerMove() {
  drawTile(playerPosX, playerPosY, RED);
  oldPlayerPosX = playerPosX;
  oldPlayerPosY = playerPosY;
}
//

/*
  Tests if qsort and binarySearch are working as expected.
*/
void test() {
  createEnemies(theEnemies, 10, map1);
  for (int i = 0; i < 5; i++) {
    theEnemies[i].setXY(i,i);
  }

  if (binarySearch(theEnemies, 2,2, 5) == 2) {
  } else {
    Serial.println("ERROR! binarySearch");
  }
  if (binarySearch(theEnemies, 2,3, 5) == -1) {
  } else {
    Serial.println("ERROR! binarySearch");
  }
  if (binarySearch(theEnemies, 3, 3, 5) == 3) {
  } else {
    Serial.println("ERROR! binarySearch");
  }

  for (int i = 0; i < 5; i++) {
    theEnemies[i].setXY(i,10-i);
  }

  Serial.println("Start qsort");
  qsort(theEnemies, 5);
  Serial.println("End of qsort");

  if (binarySearch(theEnemies, 0,10, 5) != 4) {
    Serial.println("ERROR! 0");
  }
  if (binarySearch(theEnemies, 2,8, 5) != 2) {
    Serial.println("ERROR! 2");
  }
  if (binarySearch(theEnemies, 4, 6, 5) != 0) {
    Serial.println("ERROR! 4");
  }

  theEnemies[1].setStatus(false);
  theEnemies[3].setStatus(false);

  qsort(theEnemies, 5);

  if (binarySearch(theEnemies, 0,10, 5) != 2) {
    Serial.println("ERROR! 0");
    Serial.println(binarySearch(theEnemies, 0, 10, 5));
  }
  if (binarySearch(theEnemies, 2,8, 5) != 1) {
    Serial.println("ERROR! 2");
    Serial.println(binarySearch(theEnemies, 2,8, 5));
  }
  if (binarySearch(theEnemies, 4, 6, 5) != 0) {
    Serial.println("ERROR! 4");
  }
  for (int i = 0; i < 5; i++) {
    Serial.println(theEnemies[i].getY());
  }

  // Serial.println("Good");
}

void setup() {
  init();
  Serial.begin(9600);
  setupTFT();

  // Init joystick
  pinMode(JOY_SEL, INPUT);
  digitalWrite(JOY_SEL, HIGH); // enables pull-up resistor
  Serial.println("initialized!");
}

int main() {
  setup();

  enum State {start, htp, init_game, game, end}; // htp = how to play
  State state = start;
  enum Levels {newLevel, retry};
  Levels level = newLevel;
  bool update = true;
  int prevTime = millis();
  maxEnemies = 3;

  // One loop = one frame
  while (true) {
    if (state == start) {
      tft.fillScreen(BLACK);
      tft.setTextColor(ST7735_BLACK, WHITE);    //Show tile scren
      printStartItem(0);
      tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
      printStartItem(-1);
      printStartItem(1);

      int vert;
      while(state == start) {               //handles screen selection
        vert = analogRead(JOY_VERT_ANALOG);
        if (abs(vert - JOY_CENTRE) > JOY_DEADZONE) {
          menuScroll(vert);
        }
        if (digitalRead(JOY_SEL) == 0 && selection == 0) {
          state = init_game;
        }

        if (digitalRead(JOY_SEL) == 0 && selection == 1) {
          state = htp;
        }
      }
    }
    else if (state == htp)
    {
      printInstructions();
      while (state == htp) {
        if (digitalRead(JOY_SEL) == 0) {
          state = start;
          selection = 0;
          oldSelection = 0;
        }
      }
    }
    else if (state == init_game)
    {
      playerPosY = 0;
      playerPosX = 0;
      bombCount = 0;
      isAlive = true;

      if (level == newLevel) {//change map design increase enemies
        if (maxEnemies < 13) {
          maxEnemies += 3;
        }

        init_map(map1);
        addGridOfBlocks(map1,2, 2);
        addTrees(trees,map1);
      } else {                //reset trees, max enemies, and player
        maxEnemies = 6;
        explo_size = 1;
        resetTrees(trees,map1);
        level = newLevel;
      }

      numEn = maxEnemies;
      loadMap(map1);
      Serial.println("Map loaded");
      createEnemies(theEnemies, maxEnemies, map1);
      drawEnemy(theEnemies, maxEnemies);
      updatePlayer();
      startTime = millis();
      state = game;
    }
    else if (state == game)
    {
      update = getPlayerInput();
      checkPowerUps();
      updateBombs();

      if (update) {
        updatePlayer();
      }

      moveAllEnemies(map1, theEnemies, maxEnemies);
      if(touchingAnyEnemy(playerPosX, playerPosY)){
        isAlive = false;
      }

      if (!isAlive) {
        gameEnded();
        killAllEnemies(map1, theEnemies, maxEnemies);
        enemiesKilled = maxEnemies-numEn;
        calculatePoints(enemiesKilled, millis() - startTime);
        printDeath(totalPoints);
        totalPoints = 0;
        level = retry;
        state = end;
        continue;
      }

      if (numEn == 0) {
        gameEnded();
        printWin();
        calculatePoints(maxEnemies, millis() - startTime);
        level = newLevel;
        state = end;
        continue;
      }

      int t = millis();
      // If the the amount of time passed since before the screen updated til now
      // is less than milliseconds per frame, delay until that MILLIS_PER_FRAME
      // has passed. i.e wait until the frame finishes before updating again.
      if (t - prevTime < MILLIS_PER_FRAME) {//code runs fast enough that this is always true
        delay(MILLIS_PER_FRAME - (t - prevTime));
      }
      prevTime = millis();

    } else { //End game state
      if (digitalRead(JOY_SEL) == 0) {
        tft.fillScreen(BLACK);
        if (level == retry) {
          state = start;
        } else{
          state = init_game;
        }
      }
    }

  }

  return 0;
}

void checkPowerUps(){
  if (map1[playerPosY][playerPosX] % 10 == 6) {
    map1[playerPosY][playerPosX] = 0;
    explo_size++;
  }
}

void changeHighlightedItem(){
  //overwrite old selection
  (tft).setTextColor(ST7735_WHITE, ST7735_BLACK);
  printStartItem(oldSelection);
  //highlight new selection
  (tft).setTextColor(ST7735_BLACK, ST7735_WHITE);
  printStartItem(selection);
  oldSelection = selection;
}

// Scroll through the menu
void menuScroll(int vert) {
      // scroll up by one
    if ((vert - JOY_CENTRE) < 0 && selection == 1) {
      // print old selection normally
      selection = 0;
      changeHighlightedItem();
    }
    // scroll down by one
    if ((vert - JOY_CENTRE) > 0 && selection == 0) {
      // print old selection normally
      selection = 1;
      changeHighlightedItem();
    }
    delay(150); // Delay so that you don't scroll too fast
}

// Point calculation: enemies killed * 200 / time to clear level (seconds)
void calculatePoints (unsigned long enemiesKilled, unsigned long clearTime) {
  Serial.print("Previous points: "); Serial.println(totalPoints);
  Serial.print("enemiesKilled: "); Serial.println(enemiesKilled);
  Serial.print("clearTime: "); Serial.println(clearTime);
  totalPoints += ((unsigned long) enemiesKilled * 200 * 1000) / clearTime;
  Serial.print("Total Points: "); Serial.println(totalPoints);
}
