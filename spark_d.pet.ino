#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDRESS  0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BUZZER_PIN  27
#define SDA_PIN     26
#define SCL_PIN     25
#define SOUND_PIN   32

int leftEyeX  = 38, leftEyeY  = 32;
int rightEyeX = 90, rightEyeY = 32;
int eyeW = 20, eyeH = 26;

enum Mood { NORMAL, BLINK, LOOK_LEFT, LOOK_RIGHT, HAPPY, SURPRISED, SLEEPY, WINK, ANGRY, HEART, SAD, DIZZY, CROSSED };
Mood currentMood = NORMAL;

unsigned long lastMoodChange  = 0;
unsigned long moodDuration    = 2000;
unsigned long lastActivity    = 0;
unsigned long lastSound       = 0;
unsigned long lastClapTime    = 0;
unsigned long silenceStart    = 0;
int clapCount                 = 0;
bool silenceTracking          = false;

void drawEyes(int lx, int ly, int rx, int ry, int w, int h);
void drawHappyEyes();
void drawSurprisedEyes();
void drawSleepyEyes(int stage);
void drawAngryEyes();
void drawWinkEyes();
void drawBlinkEyes(int openAmount);
void drawHeartEyes();
void drawSadEyes();
void drawDizzyEyes();
void drawCrossedEyes();
void beep(int freq, int dur);
void playFriendsTheme();
void playWimoweh();
void playSadTune();
void playAngryTune();
void playFunnyTune();
void pickRandomMood();
void runMood();
void sleepMode();

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SOUND_PIN, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED not found!");
    while (true);
  }

  display.clearDisplay();
  display.display();
  delay(500);

  playFriendsTheme();

  for (int h = 0; h <= eyeH; h += 3) {
    display.clearDisplay();
    drawEyes(leftEyeX, leftEyeY, rightEyeX, rightEyeY, eyeW, h);
    display.display();
    delay(40);
  }

  lastActivity    = millis();
  lastMoodChange  = millis();
  silenceStart    = millis();
}

void loop() {
  unsigned long now = millis();

  if (digitalRead(SOUND_PIN) == LOW && now - lastSound > 150) {
    lastSound       = now;
    lastActivity    = now;
    silenceStart    = now;
    silenceTracking = true;

    if (now - lastClapTime > 1500) {
      clapCount = 0;
    }
    clapCount++;
    lastClapTime = now;

    delay(400);

    if (clapCount == 1) {
      currentMood    = HAPPY;
      moodDuration   = 2000;
      lastMoodChange = now;
      playWimoweh();
    } else if (clapCount == 2) {
      currentMood    = DIZZY;
      moodDuration   = 2000;
      lastMoodChange = now;
      playFunnyTune();
    } else if (clapCount >= 3) {
      currentMood    = ANGRY;
      moodDuration   = 2000;
      lastMoodChange = now;
      playAngryTune();
      clapCount = 0;
    }
    return;
  }

  if (silenceTracking && now - silenceStart > 5000 && currentMood != SAD) {
    currentMood     = SAD;
    moodDuration    = 3000;
    lastMoodChange  = now;
    playSadTune();
    silenceTracking = false;
  }

  if (now - lastActivity > 300000UL) {
    sleepMode();
    return;
  }

  if (now - lastMoodChange > moodDuration) {
    pickRandomMood();
    lastMoodChange = now;
    lastActivity   = now;
  }

  runMood();
}

// ── TUNES ────────────────────────────────────────────────────

void playFriendsTheme() {
  // So no one told you life was gonna be this way...
  beep(392, 150); delay(30);
  beep(392, 150); delay(30);
  beep(392, 150); delay(30);
  beep(392, 150); delay(30);
  delay(100); delay(100); delay(100); delay(100);

  beep(349, 150); delay(30);
  beep(392, 150); delay(30);
  beep(440, 300); delay(50);
  beep(392, 150); delay(30);
  beep(349, 150); delay(30);
  beep(330, 400); delay(80);

  beep(294, 150); delay(30);
  beep(330, 150); delay(30);
  beep(349, 300); delay(50);
  beep(330, 150); delay(30);
  beep(294, 150); delay(30);
  beep(262, 600);
}

void playWimoweh() {
  beep(1047, 100); delay(20);
  beep(988,  60);  delay(20);
  beep(1047, 100); delay(20);
  beep(988,  60);  delay(20);
  beep(1047, 100); delay(20);
  beep(988,  60);  delay(20);
  beep(1047, 200); delay(40);
  beep(1047, 100); delay(20);
  beep(988,  60);  delay(20);
  beep(1047, 100); delay(20);
  beep(988,  60);  delay(20);
  beep(1047, 100); delay(20);
  beep(988,  60);  delay(20);
  beep(784,  400);
}

void playSadTune() {
  beep(494, 200); delay(50);
  beep(440, 200); delay(50);
  beep(392, 200); delay(50);
  beep(349, 400); delay(50);
  beep(330, 600);
}

void playAngryTune() {
  beep(150, 80); delay(30);
  beep(120, 80); delay(30);
  beep(100, 80); delay(30);
  beep(150, 80); delay(30);
  beep(80,  400);
}

void playFunnyTune() {
  beep(1000, 60); delay(30);
  beep(500,  60); delay(30);
  beep(1500, 60); delay(30);
  beep(300,  60); delay(30);
  beep(2000, 60); delay(30);
  beep(200,  100);
}

// ── MOOD PICKER ──────────────────────────────────────────────

void pickRandomMood() {
  int r = random(0, 100);
  if      (r < 25) { currentMood = BLINK;      moodDuration = 200; }
  else if (r < 36) { currentMood = LOOK_LEFT;  moodDuration = 1200; }
  else if (r < 47) { currentMood = LOOK_RIGHT; moodDuration = 1200; }
  else if (r < 55) { currentMood = HAPPY;      moodDuration = 1500; playWimoweh(); }
  else if (r < 63) { currentMood = SURPRISED;  moodDuration = 900;  beep(1500,60); }
  else if (r < 70) { currentMood = WINK;       moodDuration = 800; }
  else if (r < 77) { currentMood = HEART;      moodDuration = 1200; beep(1000,60); delay(40); beep(1400,60); }
  else if (r < 84) { currentMood = CROSSED;    moodDuration = 1000; playFunnyTune(); }
  else if (r < 91) { currentMood = SLEEPY;     moodDuration = 2000; }
  else             { currentMood = NORMAL;     moodDuration = random(1500,3500); }
}

// ── MOOD RUNNER ──────────────────────────────────────────────

void runMood() {
  display.clearDisplay();

  switch (currentMood) {
    case NORMAL:     drawEyes(leftEyeX, leftEyeY, rightEyeX, rightEyeY, eyeW, eyeH); break;
    case LOOK_LEFT:  drawEyes(leftEyeX-8, leftEyeY, rightEyeX-8, rightEyeY, eyeW, eyeH); break;
    case LOOK_RIGHT: drawEyes(leftEyeX+8, leftEyeY, rightEyeX+8, rightEyeY, eyeW, eyeH); break;
    case HAPPY:      drawHappyEyes();     break;
    case SURPRISED:  drawSurprisedEyes(); break;
    case SLEEPY:     drawSleepyEyes(1);   break;
    case WINK:       drawWinkEyes();      break;
    case ANGRY:      drawAngryEyes();     break;
    case HEART:      drawHeartEyes();     break;
    case SAD:        drawSadEyes();       break;
    case DIZZY:      drawDizzyEyes();     break;
    case CROSSED:    drawCrossedEyes();   break;
    case BLINK: {
      for (int h = eyeH; h >= 0; h -= 5) {
        display.clearDisplay(); drawBlinkEyes(h); display.display(); delay(20);
      }
      delay(60);
      for (int h = 0; h <= eyeH; h += 5) {
        display.clearDisplay(); drawBlinkEyes(h); display.display(); delay(20);
      }
      currentMood  = NORMAL;
      moodDuration = random(1500, 3000);
      return;
    }
  }
  display.display();
  delay(50);
}

// ── SLEEP MODE ───────────────────────────────────────────────

void sleepMode() {
  for (int h = eyeH; h >= 4; h -= 2) {
    display.clearDisplay(); drawBlinkEyes(h); display.display(); delay(60);
  }
  while (true) {
    display.clearDisplay();
    drawSleepyEyes(2);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(100, 10); display.print("z");
    display.setCursor(108, 4);  display.print("Z");
    display.display();
    delay(1500);
    beep(200, 60);
    delay(2000);
    if (digitalRead(SOUND_PIN) == LOW) break;
    if (random(0, 100) < 5) break;
  }
  beep(900,80); delay(60); beep(1200,80);
  for (int h = 4; h <= eyeH; h += 2) {
    display.clearDisplay(); drawBlinkEyes(h); display.display(); delay(40);
  }
  lastActivity    = millis();
  lastMoodChange  = millis();
  silenceStart    = millis();
  currentMood     = HAPPY;
  moodDuration    = 1500;
}

// ── DRAW FUNCTIONS ───────────────────────────────────────────

void drawEyes(int lx, int ly, int rx, int ry, int w, int h) {
  if (h <= 0) return;
  display.fillEllipse(lx, ly, w, h, SSD1306_WHITE);
  display.fillEllipse(rx, ry, w, h, SSD1306_WHITE);
  display.fillCircle(lx, ly, 6, SSD1306_BLACK);
  display.fillCircle(rx, ry, 6, SSD1306_BLACK);
  display.fillCircle(lx-5, ly-5, 2, SSD1306_WHITE);
  display.fillCircle(rx-5, ry-5, 2, SSD1306_WHITE);
}

void drawBlinkEyes(int openAmount) {
  drawEyes(leftEyeX, leftEyeY, rightEyeX, rightEyeY, eyeW, openAmount);
}

void drawHappyEyes() {
  display.fillEllipse(leftEyeX,  leftEyeY,  eyeW, eyeH, SSD1306_WHITE);
  display.fillEllipse(rightEyeX, rightEyeY, eyeW, eyeH, SSD1306_WHITE);
  display.fillRect(leftEyeX-eyeW,  leftEyeY,  eyeW*2, eyeH, SSD1306_BLACK);
  display.fillRect(rightEyeX-eyeW, rightEyeY, eyeW*2, eyeH, SSD1306_BLACK);
  display.fillCircle(leftEyeX-18,  leftEyeY+18,  5, SSD1306_WHITE);
  display.fillCircle(rightEyeX+18, rightEyeY+18, 5, SSD1306_WHITE);
}

void drawSurprisedEyes() {
  display.fillCircle(leftEyeX,  leftEyeY,  eyeW, SSD1306_WHITE);
  display.fillCircle(rightEyeX, rightEyeY, eyeW, SSD1306_WHITE);
  display.fillCircle(leftEyeX,  leftEyeY,  5, SSD1306_BLACK);
  display.fillCircle(rightEyeX, rightEyeY, 5, SSD1306_BLACK);
  display.fillCircle(leftEyeX-5,  leftEyeY-5,  2, SSD1306_WHITE);
  display.fillCircle(rightEyeX-5, rightEyeY-5, 2, SSD1306_WHITE);
}

void drawSleepyEyes(int stage) {
  int h = (stage == 1) ? eyeH/2 : eyeH/4;
  display.fillEllipse(leftEyeX,  leftEyeY+eyeH/4, eyeW, h, SSD1306_WHITE);
  display.fillEllipse(rightEyeX, rightEyeY+eyeH/4, eyeW, h, SSD1306_WHITE);
}

void drawWinkEyes() {
  display.fillEllipse(leftEyeX, leftEyeY, eyeW, eyeH, SSD1306_WHITE);
  display.fillCircle(leftEyeX, leftEyeY, 6, SSD1306_BLACK);
  display.fillCircle(leftEyeX-5, leftEyeY-5, 2, SSD1306_WHITE);
  display.drawLine(rightEyeX-eyeW, rightEyeY,   rightEyeX+eyeW, rightEyeY,   SSD1306_WHITE);
  display.drawLine(rightEyeX-eyeW, rightEyeY+1, rightEyeX+eyeW, rightEyeY+1, SSD1306_WHITE);
}

void drawAngryEyes() {
  display.fillEllipse(leftEyeX,  leftEyeY,  eyeW, eyeH, SSD1306_WHITE);
  display.fillEllipse(rightEyeX, rightEyeY, eyeW, eyeH, SSD1306_WHITE);
  display.fillCircle(leftEyeX,  leftEyeY,  6, SSD1306_BLACK);
  display.fillCircle(rightEyeX, rightEyeY, 6, SSD1306_BLACK);
  display.drawLine(leftEyeX-eyeW,  leftEyeY-eyeH-2,  leftEyeX+eyeW,  leftEyeY-eyeH-8,  SSD1306_WHITE);
  display.drawLine(rightEyeX-eyeW, rightEyeY-eyeH-8, rightEyeX+eyeW, rightEyeY-eyeH-2, SSD1306_WHITE);
}

void drawHeartEyes() {
  display.fillCircle(leftEyeX-5,  leftEyeY-4,  7, SSD1306_WHITE);
  display.fillCircle(leftEyeX+5,  leftEyeY-4,  7, SSD1306_WHITE);
  display.fillTriangle(leftEyeX-12, leftEyeY-2, leftEyeX+12, leftEyeY-2, leftEyeX, leftEyeY+10, SSD1306_WHITE);
  display.fillCircle(rightEyeX-5, rightEyeY-4, 7, SSD1306_WHITE);
  display.fillCircle(rightEyeX+5, rightEyeY-4, 7, SSD1306_WHITE);
  display.fillTriangle(rightEyeX-12, rightEyeY-2, rightEyeX+12, rightEyeY-2, rightEyeX, rightEyeY+10, SSD1306_WHITE);
}

void drawSadEyes() {
  display.fillEllipse(leftEyeX,  leftEyeY,  eyeW, eyeH, SSD1306_WHITE);
  display.fillEllipse(rightEyeX, rightEyeY, eyeW, eyeH, SSD1306_WHITE);
  display.fillRect(leftEyeX-eyeW,  leftEyeY-eyeH, eyeW*2, eyeH, SSD1306_BLACK);
  display.fillRect(rightEyeX-eyeW, rightEyeY-eyeH, eyeW*2, eyeH, SSD1306_BLACK);
  display.drawLine(leftEyeX-eyeW,  leftEyeY-eyeH-8, leftEyeX+eyeW,  leftEyeY-eyeH-2,  SSD1306_WHITE);
  display.drawLine(rightEyeX-eyeW, rightEyeY-eyeH-2, rightEyeX+eyeW, rightEyeY-eyeH-8, SSD1306_WHITE);
  display.fillCircle(leftEyeX+5,  leftEyeY+eyeH+3,  3, SSD1306_WHITE);
  display.fillCircle(rightEyeX+5, rightEyeY+eyeH+3, 3, SSD1306_WHITE);
}

void drawDizzyEyes() {
  display.drawCircle(leftEyeX,  leftEyeY,  10, SSD1306_WHITE);
  display.drawCircle(leftEyeX,  leftEyeY,  6,  SSD1306_WHITE);
  display.drawCircle(leftEyeX,  leftEyeY,  2,  SSD1306_WHITE);
  display.drawCircle(rightEyeX, rightEyeY, 10, SSD1306_WHITE);
  display.drawCircle(rightEyeX, rightEyeY, 6,  SSD1306_WHITE);
  display.drawCircle(rightEyeX, rightEyeY, 2,  SSD1306_WHITE);
  display.drawLine(leftEyeX-4,  leftEyeY-4,  leftEyeX+4,  leftEyeY+4,  SSD1306_WHITE);
  display.drawLine(leftEyeX+4,  leftEyeY-4,  leftEyeX-4,  leftEyeY+4,  SSD1306_WHITE);
  display.drawLine(rightEyeX-4, rightEyeY-4, rightEyeX+4, rightEyeY+4, SSD1306_WHITE);
  display.drawLine(rightEyeX+4, rightEyeY-4, rightEyeX-4, rightEyeY+4, SSD1306_WHITE);
}

void drawCrossedEyes() {
  display.fillEllipse(leftEyeX,  leftEyeY,  eyeW, eyeH, SSD1306_WHITE);
  display.fillEllipse(rightEyeX, rightEyeY, eyeW, eyeH, SSD1306_WHITE);
  display.fillCircle(leftEyeX+7,  leftEyeY, 5, SSD1306_BLACK);
  display.fillCircle(rightEyeX-7, rightEyeY, 5, SSD1306_BLACK);
  display.fillCircle(leftEyeX+5,  leftEyeY-2, 2, SSD1306_WHITE);
  display.fillCircle(rightEyeX-5, rightEyeY-2, 2, SSD1306_WHITE);
}

void beep(int freq, int dur) {
  tone(BUZZER_PIN, freq, dur);
  delay(dur);
  noTone(BUZZER_PIN);
}