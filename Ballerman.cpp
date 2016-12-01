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

#define JOY_DEADZONE 64 // Only care about joystick movement if
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

// General functions

void updateBombs() {      //confusing to look at
  unsigned long currentTime;

  for (int i = 0; i < maxBombCount; i++) {
    currentTime = millis();
    if (activeBombs[i].active && !(activeBombs[i].exploding)) {
      if ((currentTime - activeBombs[i].time) > explodeTime) {
        Serial.print("Bomb "); Serial.print(i); Serial.println(" exploding.");
        Serial.print("currentTime: "); Serial.println(currentTime);
        Serial.print("Bomb set at time "); Serial.println(activeBombs[i].time);
        Serial.print("time difference: "); Serial.println(currentTime - activeBombs[i].time);
        bombExplode(activeBombs[i].x, activeBombs[i].y);
        activeBombs[i].exploding = true;
        activeBombs[i].timeOfExplosion = millis();
      }
      else {
        drawBomb(&tft, activeBombs[i].x, activeBombs[i].y);
      }
    }
    else if (activeBombs[i].exploding) {
      if ((currentTime - activeBombs[i].timeOfExplosion) > explosionDuration) {
        activeBombs[i].exploding = false;
        activeBombs[i].active = false;
        removeTrees(map1, activeBombs[i].x, activeBombs[i].y, explo_size);
        removeExplosion(activeBombs[i].x, activeBombs[i].y);
      }
      else {
        drawExplosion(&tft, activeBombs[i].x, activeBombs[i].y, explo_size);
        killEnemies(theEnemies, maxEnemies, activeBombs[i].x, activeBombs[i].y, &numEn, explo_size);
        if (explodePlayer(playerPosX, playerPosY, activeBombs[i].x, activeBombs[i].y)){
          Serial.println("Exploding Player");
          break;
        }
      }
    }
  }
}

// Move the player around the screen
void updatePlayer() {
  drawTile(&tft, map1, oldPlayerPosX, oldPlayerPosY);
  playerMove();
}

//
void bombExplode(int x, int y) {
  drawTile(&tft, map1, x, y);
  bombCount--;
  Serial.print("bombCount: "); Serial.println(bombCount);
}

// Redraws the map over an explosion
void removeExplosion(int x, int y) {
  for (int i = x - explo_size; i < x+(2*explo_size); i++) {
    drawTile(&tft, map1, i, y);
  }
  for (int i = y-explo_size; i < y+(explo_size*2); i++) {
    drawTile(&tft, map1, x, i);
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
    if (map1[playerPosY][temp] == 0 || map1[playerPosY][temp] == 1 || map1[playerPosY][temp] == 6) {       //constrain movement to ground
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

    if (map1[temp][playerPosX] == 0 || map1[temp][playerPosX] == 1 || map1[temp][playerPosX] == 6) {       //constrain movement to ground
      playerPosY = temp;
      return true;
    }
  }

  return false;
}

void setBomb(int x, int y) {
  bombCount++;
  Serial.print("bombCount: "); Serial.println(bombCount);
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
  drawTile(&tft, playerPosX, playerPosY, RED);
  oldPlayerPosX = playerPosX;
  oldPlayerPosY = playerPosY;
}
//


void setup() {
  init();
  Serial.begin(9600);
  setupTFT(&tft);

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
      tft.setTextColor(ST7735_BLACK, WHITE);    //Show tile scren
      printStartItem(&tft, 0);
      tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
      printStartItem(&tft, -1);
      printStartItem(&tft, 1);

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
    else if (state == htp)//TODO:MAKE THIS
    {
      state = start;
    }
    else if (state == init_game)
    {
      playerPosY = 0;
      playerPosX = 0;
      bombCount = 0;
      isAlive = true;
      // init_map(map1);
      if (level == newLevel) {
        if (maxEnemies < 13) {
          maxEnemies += 4;
        }

        init_map(map1);
        addGridOfBlocks(map1,2, 2);
        addTrees(trees,map1);
      } else {
        maxEnemies = 5;
        explo_size = 1;
        resetTrees(trees,map1);
        level = newLevel;
      }
      numEn = maxEnemies;
      loadMap(&tft, map1);
      Serial.println("Map loaded");
      createEnemies(theEnemies, maxEnemies, map1);
      drawEnemy(theEnemies, maxEnemies, &tft);
      updatePlayer();
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

      moveAllEnemies(map1, theEnemies, &tft, maxEnemies);
      if(touchingAnyEnemy(theEnemies, playerPosX, playerPosY, maxEnemies)){
        isAlive = false;
      }

      if (!isAlive) {
        gameEnded();
        printDeath(&tft);
        level = retry;
        state = end;
        continue;
      }

      if (numEn == 0) {
        gameEnded();
        printWin(&tft);
        level = newLevel;
        state = end;
        continue;
      }

      int t = millis();
      // If the the amount of time passed since before the screen updated til now
      // is less than milliseconds per frame, delay until that MILLIS_PER_FRAME
      // has passed. i.e wait until the frame finishes before updating again.
      if (t - prevTime < MILLIS_PER_FRAME) {
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
  if (map1[playerPosY][playerPosX] == 6) {
    map1[playerPosY][playerPosX] = 0;
    explo_size++;
  }
}

void changeHighlightedItem(){
  //overwrite old selection
  (tft).setTextColor(ST7735_WHITE, ST7735_BLACK);
  printStartItem(&tft, oldSelection);
  //highlight new selection
  (tft).setTextColor(ST7735_BLACK, ST7735_WHITE);
  printStartItem(&tft, selection);
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