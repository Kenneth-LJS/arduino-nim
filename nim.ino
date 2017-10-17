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
// int x[] = {HIGH, LOW, HIGH, LOW};
// int y[] = {LOW, HIGH, LOW, HIGH};
int on[] = {HIGH, HIGH, HIGH, HIGH};
int off[] = {LOW, LOW, LOW, LOW};
int player[] = {LOW, HIGH, HIGH, LOW};
int end[] = {HIGH, LOW, LOW, HIGH};

// Misc constants
const int ledArrayIntervalMicros = 1;
const unsigned long ledBlinkIntervalMillis = 1000;
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
    // already decided which config to show
    // now need to decide whether 2 is on or off
    for (int i = 0; i < 4; i++) {
      config[i] = ledColConfig[i] == 1 ? HIGH : LOW;
    }
    if (millis() - prevLedBlinkTimeMillis > ledBlinkIntervalMillis) {
      ledBlinkOn = !ledBlinkOn;
      prevLedBlinkTimeMillis = millis();
      for (int i = 0; i < 4; i++) {
        if (ledColConfig[i] == 2) {
          config[i] = ledBlinkOn ? HIGH : LOW;
        }
      }
      // problem, not blinking
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

void oldlightup(int x[], int y[]) {
  lightsOff();
  digitalWrite(X, LOW);
  digitalWrite(Y, HIGH);
  digitalWrite(A, x[0]);
  digitalWrite(B, x[1]);
  digitalWrite(C, x[2]);
  digitalWrite(D, x[3]);
  delay(1);
  lightsOff();
  digitalWrite(X, HIGH);
  digitalWrite(Y, LOW);
  digitalWrite(A, y[0]);
  digitalWrite(B, y[1]);
  digitalWrite(C, y[2]);
  digitalWrite(D, y[3]);
  delay(1);
}

void lightsOff() {
  digitalWrite(X, HIGH);
  digitalWrite(Y, HIGH);
  digitalWrite(A, LOW);
  digitalWrite(B, LOW);
  digitalWrite(C, LOW);
  digitalWrite(D, LOW);
}

void lightFirstPlayer() {
  if (firstPlayer == PLAYER_HUMAN) {
    lightup(player, off);
  } else {
    lightup(off, player);
  }
}

bool isHeld(int buttonPort) {
  return (1 - digitalRead(buttonPort)) == 1;
}

int convertPort(int portOrIndex) {
  return OFFSET_BUTTON_ARRAY - portOrIndex;
}

bool isPressed(int buttonPort) {
  return buttonPushed[convertPort(buttonPort)];
}

bool isIn(int state) {
  return currentState == state;
}

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

void stateIdleRoutine() {
  lightsOff();
  if (isPressed(BUTTON_SELECT)) {
    initGame();
    currentState = STATE_CHOOSE_FIRST_PLAYER;
  }

  // debugging only
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

void stateChooseFirstPlayerRoutine() {
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

void lightGame() {
  lightup(gameState[0], gameState[1]);
}

void stateGameIdleRoutine() {
  if (currentPlayer == PLAYER_AI) {
    playAI();
    if (hasGameEnded()) return;
    currentPlayer = PLAYER_HUMAN;
  }
  lightGame();
  if (isPressed(BUTTON_LEFT)) {
    removeFrom = X;
    currentState = STATE_GAME_REMOVAL;

  } else if (isPressed(BUTTON_RIGHT)) {
    removeFrom = Y;
    currentState = STATE_GAME_REMOVAL;

  } else if (isPressed(BUTTON_CANCEL)) {
    currentState = STATE_IDLE;
  }
}

int availableForRemoval() {
  return availableForRemoval(removeFrom);
}

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

void cycleRemoveCount() {
  removeCount++;
  if (removeCount > availableForRemoval()) {
    removeCount = 1;
  }
}

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

bool hasGameEnded() {
  if (availableForRemoval(X) == 0 && availableForRemoval(Y) == 0) {
    currentState = STATE_GAME_END;
    return true;
  }
  return false;
}

void stateGameRemovalRoutine() {
  int btn = removeFrom == X ? BUTTON_LEFT : BUTTON_RIGHT;
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

bool isAnyPressed() {
  return isPressed(BUTTON_LEFT) || isPressed(BUTTON_RIGHT) || isPressed(BUTTON_CANCEL) || isPressed(BUTTON_SELECT);
}

void stateGameEndRoutine() {
  if (currentPlayer == PLAYER_AI) {
    lightup(end, off);
  } else {
    lightup(off, end);
  }
  if (isAnyPressed()) {
    currentState = STATE_IDLE;
  }
}

bool isButtonPushed(int buttonPort) {
  return buttonPushed[convertPort(buttonPort)];
}

void checkButtons() {
  for (int i = 0; i < 4; i++) {
    int buttonPort = convertPort(i);
    if (isButtonPushed(buttonPort)) {
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
  printVars();
  if (isIn(STATE_IDLE)) {
    stateIdleRoutine();

  } else if (isIn(STATE_CHOOSE_FIRST_PLAYER)) {
    stateChooseFirstPlayerRoutine();

  } else if (isIn(STATE_GAME_IDLE)) {
    stateGameIdleRoutine();

  } else if (isIn(STATE_GAME_REMOVAL)) {
    stateGameRemovalRoutine();

  } else if (isIn(STATE_GAME_END)) {
    stateGameEndRoutine();

  }
  clearButtons();
}
