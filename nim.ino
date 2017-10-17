// Pin labels
const int A = 13;
const int B = 12;
const int C = 11;
const int D = 10;
const int X = 9;
const int Y = 8;
const int BUTTON_LEFT = 7;
const int BUTTON_RIGHT = 6;
const int BUTTON_SELECT = 5;
const int BUTTON_CANCEL = 4;
const int OFFSET_BUTTON_ARRAY = BUTTON_LEFT;

// State labels
const int STATE_IDLE = 0;
const int STATE_GAME_IDLE = 1;
const int STATE_CHOOSE_FIRST_PLAYER = 2;
const int STATE_GAME_REMOVAL = 3;
const int STATE_GAME_END = 4;

// Misc labels
const int PLAYER_HUMAN = 99;
const int PLAYER_AI = 98;

// Fixed patterns
int on[] = {HIGH, HIGH, HIGH, HIGH};
int off[] = {LOW, LOW, LOW, LOW};
int player[] = {0, 2, 2, 0};
int end[] = {2, 0, 0, 2};

// Misc constants
const int ledArrayIntervalMicros = 1;
const unsigned long ledBlinkIntervalMillis = 200;
const int buttonResponseIntervalMillis = 50;
const int printIntervalMillis = 500;

// Variables: general
int currentState = STATE_IDLE;
int currentLedCol = X; // for multiplexing the array lighting
unsigned long lastPrintTimeMillis = 0;
bool ledBlinkOn = false;
unsigned long prevLedBlinkTimeMillis = 0;

// Variables: buttons
unsigned long prevLedArrayTimeMicros = 0;
unsigned long prevButtonTimeMillis[] = {0,0,0,0};
bool buttonPushed[] = {false,false,false,false};
bool buttonPushedTemp[] = {false,false,false,false};

// Variables: game
int gameState[2][4] = {{1,1,1,1},{1,1,1,1}};
int firstPlayer = PLAYER_HUMAN;
int currentPlayer = firstPlayer;
int removeFrom = X;
int removeCount = 1;

// Function declarations
void lightup();
void playAI();

// display a solid light
void lightup(int x[], int y[]) {
  if (micros() - prevLedArrayTimeMicros > ledArrayIntervalMicros) {
    int* ledColConfig;
    if (currentLedCol == Y) {
      ledColConfig = x;
      currentLedCol = X;
    } else {
      ledColConfig = y;
      currentLedCol = Y;
    }
    lightsOff();
    digitalWrite(currentLedCol, LOW);
    digitalWrite(A, ledColConfig[0]);
    digitalWrite(B, ledColConfig[1]);
    digitalWrite(C, ledColConfig[2]);
    digitalWrite(D, ledColConfig[3]);
    prevLedArrayTimeMicros = micros();
  }
}

// takes in a config array for each column
// blinks when config[i] is 2, solid when 1, off when 0
void blinkup(int x[], int y[]) {
  if (micros() - prevLedArrayTimeMicros > ledArrayIntervalMicros) {
    int* ledColConfig;
    if (currentLedCol == Y) {
      ledColConfig = x;
      currentLedCol = X;
    } else {
      ledColConfig = y;
      currentLedCol = Y;
    }
    int config[] = {LOW, LOW, LOW, LOW};
    for (int i = 0; i < 4; i++) {
      if (ledColConfig[i] == 2) {
        config[i] = ledBlinkOn ? HIGH : LOW;
      } else {
        config[i] = ledColConfig[i] == 1 ? HIGH : LOW;
      }
    }
    if (millis() - prevLedBlinkTimeMillis > ledBlinkIntervalMillis) {
      ledBlinkOn = !ledBlinkOn;
      prevLedBlinkTimeMillis = millis();
    }
    lightsOff();
    digitalWrite(currentLedCol, LOW);
    digitalWrite(A, config[0]);
    digitalWrite(B, config[1]);
    digitalWrite(C, config[2]);
    digitalWrite(D, config[3]);
    prevLedArrayTimeMicros = micros();
  }
}

// turns off all lights
void lightsOff() {
  digitalWrite(X, HIGH);
  digitalWrite(Y, HIGH);
  digitalWrite(A, LOW);
  digitalWrite(B, LOW);
  digitalWrite(C, LOW);
  digitalWrite(D, LOW);
}

// light up the game board during the selection of the first player
void lightFirstPlayer() {
  if (firstPlayer == PLAYER_HUMAN) {
    blinkup(player, off);
  } else {
    blinkup(off, player);
  }
}

// simple check to see if the button is currently being held down
bool isHeld(int buttonPort) {
  return (1 - digitalRead(buttonPort)) == 1;
}

// changes between the button port and the button array index
int convertPort(int portOrIndex) {
  return OFFSET_BUTTON_ARRAY - portOrIndex;
}

// check the isButtonPushed flag, not simply the hardware button
bool isPressed(int buttonPort) {
  return buttonPushed[convertPort(buttonPort)];
}

// checks the state that the system is in
bool isIn(int state) {
  return currentState == state;
}

// reset game variables
void initGame() {
  firstPlayer = PLAYER_HUMAN;
  removeFrom = X;
  removeCount = 1;
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++) {
      gameState[i][j] = 1;
    }
  }
}

// task for the idle state
void stateIdleTask() {
  lightsOff();
  if (isPressed(BUTTON_SELECT)) {
    initGame();
    currentState = STATE_CHOOSE_FIRST_PLAYER;
  }

  // debugging only. these buttons should have no effect otherwise.
  if (isHeld(BUTTON_LEFT)) {
    int x[] = {1, 0, 0, 0};
    int y[] = {0, 0, 0, 1};
    lightup(on, on);
  } else if (isHeld(BUTTON_RIGHT)) {
    // blink top 2, solid btm 1, dark btm 1
    int x[] = {0, 1, 2, 2};
    blinkup(x, x);
  }
}

// task for the choose first player state
void stateChooseFirstPlayerTask() {
  lightFirstPlayer();
  if (isPressed(BUTTON_CANCEL)) {
    currentState = STATE_IDLE;

  } else if (isPressed(BUTTON_LEFT)) {
    firstPlayer = PLAYER_HUMAN;

  } else if (isPressed(BUTTON_RIGHT)) {
    firstPlayer = PLAYER_AI;

  } else if (isPressed(BUTTON_SELECT)) {
    currentPlayer = firstPlayer;
    currentState = STATE_GAME_IDLE;
  }
}

// display the game board
void lightGame() {
  lightup(gameState[0], gameState[1]);
}

// task for the game-idle state
void stateGameIdleTask() {
  if (currentPlayer == PLAYER_AI) {
    playAI();
    if (hasGameEnded()) return;
    currentPlayer = PLAYER_HUMAN;
  }
  lightGame();
  if (isPressed(BUTTON_LEFT)) {
    if (availableForRemoval(X) > 0) {
      removeFrom = X;
      currentState = STATE_GAME_REMOVAL;
    }

  } else if (isPressed(BUTTON_RIGHT)) {
    if (availableForRemoval(Y) > 0) {
      removeFrom = Y;
      currentState = STATE_GAME_REMOVAL;
    }

  } else if (isPressed(BUTTON_CANCEL)) {
    currentState = STATE_IDLE;
  }
}

// returns how many objects can be removed from the selected heap
int availableForRemoval() {
  return availableForRemoval(removeFrom);
}

// returns how many objects can be removed from a specified heap
int availableForRemoval(int col) {
  int c = X - col;
  int remaining = 0;
  for (int i = 3; i >= 0; i--) {
    if (gameState[c][i] == 0) {
      return remaining;
    }
    remaining++;
  }
  return remaining;
}

// cycle through the number of possible objects to be removed
void cycleRemoveCount() {
  removeCount++;
  if (removeCount > availableForRemoval()) {
    removeCount = 1;
  }
}

// perform the removal and update the game state
void performRemoval() {
  int c = X - removeFrom;
  int toRemove = removeCount;
  removeCount = 1;
  for (int i = 0; i < 4; i++) {
    if (gameState[c][i] == 1 && toRemove > 0) {
      toRemove--;
      gameState[c][i] = 0;
    }
  }
}

// runs the logic for the AI
void playAI() {
  removeFrom = Y;
  removeCount = 1;
  if (availableForRemoval() == 0) {
    removeFrom = X;
    if (availableForRemoval() == 0) {
      return;
    }
  }
  performRemoval();
}

// check to see if the game has ended, set the system state if yes
bool hasGameEnded() {
  if (availableForRemoval(X) == 0 && availableForRemoval(Y) == 0) {
    currentState = STATE_GAME_END;
    return true;
  }
  return false;
}

// display the objects selected for removal
void lightRemoval() {
  int c = X - removeFrom;
  int remCol[] = {0,0,0,0};
  int toRemove = removeCount;
  for (int i = 0; i < 4; i++) {
    remCol[i] = gameState[c][i];
    if (remCol[i] == 1 && toRemove > 0) {
      remCol[i] = 2;
      toRemove--;
    }
  }
  if (removeFrom == X) {
    blinkup(remCol, gameState[1]);
  } else {
    blinkup(gameState[0], remCol);
  }
}

// task for the game-removal state
void stateGameRemovalTask() {
  int btn = removeFrom == X ? BUTTON_LEFT : BUTTON_RIGHT;
  lightRemoval();
  if (isPressed(btn)) {
    cycleRemoveCount();

  } else if (isPressed(BUTTON_SELECT)) {
    performRemoval();
    if (hasGameEnded()) return;
    currentPlayer = PLAYER_AI;
    currentState = STATE_GAME_IDLE;

  } else if (isPressed(BUTTON_CANCEL)) {
    currentState = STATE_GAME_IDLE;
    removeCount = 1;
  }
}

// check if any button is pressed (checking flag)
bool isAnyPressed() {
  return isPressed(BUTTON_LEFT) || isPressed(BUTTON_RIGHT) || isPressed(BUTTON_CANCEL) || isPressed(BUTTON_SELECT);
}

// task for the game-end state
void stateGameEndTask() {
  if (currentPlayer == PLAYER_AI) {
    lightup(end, off);
  } else {
    lightup(off, end);
  }
  if (isAnyPressed()) {
    currentState = STATE_IDLE;
  }
}

// sets flag if button has been held down for more than some time
// and then let go. i.e. flag is set only when button goes down and then up again.
void checkButtons() {
  for (int i = 0; i < 4; i++) {
    int buttonPort = convertPort(i);
    if (isPressed(buttonPort)) {
      continue;
    }
    if (isHeld(buttonPort)) {
      if (!buttonPushedTemp[i]) {
        prevButtonTimeMillis[i] = millis();
      }
    } else {
      if (buttonPushedTemp[i]) {
        if (millis() - prevButtonTimeMillis[i] > buttonResponseIntervalMillis) {
          buttonPushed[i] = true;
        }
      }
    }
    buttonPushedTemp[i] = isHeld(buttonPort);
  }
}

// just print the button flags
void printButtons() {
  if (millis() - lastPrintTimeMillis < printIntervalMillis) {
    return;
  }
  lastPrintTimeMillis = millis();
  for (int i = 0; i < 4; i++) {
    Serial.print(buttonPushed[i]);
    Serial.print(" ");
  }
  Serial.println();
}

// construct the game state in string form for printing
String stateText() {
  switch(currentState) {
    case 0 :
    return "Idle";
    case 1 :
    return "Game Idle";
    case 2 :
    return "Choose 1P";
    case 3 :
    return "Game Removal";
    case 4 :
    return "Game End";
    default :
    return "Invalid State";
  }
}

// print some variables every few millis
void printVars() {
  if (millis() - lastPrintTimeMillis < 1000) {
    return;
  }
  lastPrintTimeMillis = millis();
  String gs = String("");
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++) {
      gs = gs + String(gameState[i][j]) + " ";
    }
    if (i == 0) gs = gs + "| ";
  }
  Serial.println(gs);
  Serial.println("Removes " + String(removeCount));
  Serial.println("Remaining " + String(availableForRemoval()));
  Serial.println(stateText());
}

// clears the button flags
void clearButtons() {
  for (int i = 0; i < 4; i++) {
    buttonPushed[i] = false;
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(X, OUTPUT);
  pinMode(Y, OUTPUT);

  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_CANCEL, INPUT_PULLUP);

  lightsOff();
}

void loop() {
  checkButtons();
  // printVars();
  if (isIn(STATE_IDLE)) {
    stateIdleTask();

  } else if (isIn(STATE_CHOOSE_FIRST_PLAYER)) {
    stateChooseFirstPlayerTask();

  } else if (isIn(STATE_GAME_IDLE)) {
    stateGameIdleTask();

  } else if (isIn(STATE_GAME_REMOVAL)) {
    stateGameRemovalTask();

  } else if (isIn(STATE_GAME_END)) {
    stateGameEndTask();

  }
  clearButtons();
}
