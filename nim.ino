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

// State labels
const int STATE_INACTIVE = 0;
const int STATE_ACTIVE = 1;
const int STATE_CHOOSE_OPPONENT = 2;
const int STATE_PLAYING = 3;

// Misc labels
const int OPPONENT_HUMAN = 99;
const int OPPONENT_AI = 98;

// Fixed patterns
int x[] = {HIGH, LOW, HIGH, LOW};
int y[] = {LOW, HIGH, LOW, HIGH};
int on[] = {HIGH, HIGH, HIGH, HIGH};
int off[] = {LOW, LOW, LOW, LOW};
int opp[] = {LOW, HIGH, HIGH, LOW};

// Variables
int currentState = STATE_INACTIVE;
int opponent = OPPONENT_HUMAN;

// Lights up the leds based on the x and y arrays
// takes 2 ms to multiplex through
void lightup(int x[], int y[]) {
  digitalWrite(X, LOW);
  digitalWrite(Y, HIGH);
  digitalWrite(A, x[0]);
  digitalWrite(B, x[1]);
  digitalWrite(C, x[2]);
  digitalWrite(D, x[3]);
  delay(1);
  digitalWrite(X, HIGH);
  digitalWrite(Y, LOW);
  digitalWrite(A, y[0]);
  digitalWrite(B, y[1]);
  digitalWrite(C, y[2]);
  digitalWrite(D, y[3]);
  delay(1);
}

void lightupFor(int x[], int y[], int ms) {
  for (int i = 0; i < ms/2; i++) {
    lightup(x, y);
  }
}

void lightsOff() {
  lightup(off, off);
}

void lightOpponent() {
  int blinkInterval = 70;
  if (opponent == OPPONENT_HUMAN) {
    lightupFor(opp, off, blinkInterval);
  } else {
    lightupFor(off, opp, blinkInterval);
  }
  lightupFor(off, off, blinkInterval);
}

int isPressed(int buttonPort) {
  return 1 - digitalRead(buttonPort);
}

bool isIn(int state) {
  return currentState == state;
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
  lightsOff();
  if (isIn(STATE_INACTIVE)) {
    lightsOff();
    if (isPressed(BUTTON_SELECT)) {
      currentState = STATE_CHOOSE_OPPONENT;
      opponent = OPPONENT_HUMAN;
    }

  } else if (isIn(STATE_CHOOSE_OPPONENT)) {
    if (isPressed(BUTTON_CANCEL)) {
      currentState = STATE_INACTIVE;
    } else if (isPressed(BUTTON_LEFT)) {
      opponent = OPPONENT_HUMAN;
    } else if (isPressed(BUTTON_RIGHT)) {
      opponent = OPPONENT_AI;
    }
    lightOpponent();

  } else if (isIn(STATE_ACTIVE)) {
    lightup(on, on);
    if (isPressed(BUTTON_CANCEL)) {
      currentState = STATE_INACTIVE;
    }

  }
}
