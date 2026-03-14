#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------------------
// Arduino Mole - Joystick Edition (Quantum Input)
// ---------------------------

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Buzzer
const int buzzerPin = 9;

// Joystick Pins
const int joyXPin = A6;
const int joyYPin = A7;
const int joyBtnPin = 3;

// Mole Definitions
const int moleX[5] = {64, 64, 44, 84, 64};
const int moleY[5] = {9, 45, 27, 27, 27};

// Game variables
unsigned long nextMoleSpawnDelay = 900; // base delay before next mole
int activeMole = -1;
int score = 0;
int lives = 3;
unsigned long moleSpawnTime = 0;
unsigned long moleVisibleDuration = 900;
int animYOffset = 0;
bool isAnimating = false;
bool isPopUp = true;
int animFrame = 0;
bool inputLock = false; // Requires return to center before next input

// ---------------------------
// Mole bitmap
// ---------------------------
const unsigned char epd_bitmap_pixilart_drawing [] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc0,
    0x04, 0x20, 0x08, 0x10, 0x0a, 0x50, 0x0a, 0x50,
    0x08, 0x10, 0x09, 0x90, 0x0a, 0x50, 0x09, 0x90,
    0x08, 0x10, 0x08, 0x10, 0x1c, 0x38, 0x2a, 0x54
};

// ---------------------------
// Buzzer
// ---------------------------
void beep(int freq, int durationMs) {
  tone(buzzerPin, freq, durationMs);
  delay(durationMs);
  noTone(buzzerPin);
}

void playButtonBeep() { beep(700, 50); }
void playMoleSpawnBeep() { beep(600, 60); }
void playLoseBeep() { beep(250, 250); }

// ---------------------------
// Input helpers
// ---------------------------
int checkInputs() {
  int rawX = analogRead(joyXPin);
  int rawY = analogRead(joyYPin);
  int btnVal = digitalRead(joyBtnPin);

  int diffX = rawX - 512;
  int diffY = rawY - 512;
  int absX = abs(diffX);
  int absY = abs(diffY);
  int threshold = 350; 

  bool isCentered = (absX < threshold && absY < threshold && btnVal == HIGH);
  if (isCentered) { inputLock = false; return -1; }
  if (inputLock) return -1;

  inputLock = true;
  if (btnVal == LOW) return 4;
  if (absX > absY) return (diffX < 0 ? 1 : 0);
  else return (diffY < 0 ? 2 : 3);
}

// ---------------------------
// Setup
// ---------------------------
void setup() {
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) while(1);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  pinMode(joyBtnPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  startScreen();
}

// ---------------------------
// Start screen
// ---------------------------
void startScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(20,10); display.println("Quantum");
  display.setCursor(44,32); display.println("Mole");
  display.setTextSize(1);
  display.setCursor(30,55); display.println("Move Joystick!");
  display.display();
  delay(2000);
  display.clearDisplay();
}

// ---------------------------
// Draw lives + score
// ---------------------------
void drawLivesAndScore() {
  display.setTextSize(1);
  display.setCursor(0,56);
  display.print("Lives:");
  int heartStartX = 36;
  for(int i=0;i<lives;i++) display.fillCircle(heartStartX + i*6, 60, 2, SSD1306_WHITE);
  display.setCursor(heartStartX + lives*6 + 4, 56);
  display.print("| Score: "); display.print(score);
}

// ---------------------------
// Draw mole
// ---------------------------
void drawBitmapMole(int x, int yBase, int yOffset, int frame) {
  int w = 16, h = 16;
  if(isPopUp){
    if(frame==0){w=20; h=12;}
    if(frame==1||frame==2){w=16; h=18;}
  }else{
    if(frame==0||frame==1){w=16; h=18;}
    if(frame==2||frame==3){w=18; h=12;}
  }
  display.fillRect(x-w/2, yBase-yOffset, w, h, SSD1306_BLACK);
  display.drawBitmap(x-8, yBase-yOffset, epd_bitmap_pixilart_drawing, 16, 16, SSD1306_WHITE);
}

void drawMoles() {
  display.clearDisplay();
  drawLivesAndScore();
  for(int i=0;i<5;i++){
    display.drawCircle(moleX[i], moleY[i], 8, SSD1306_WHITE);
    if(i==activeMole) drawBitmapMole(moleX[i], moleY[i], animYOffset, animFrame);
  }
}

// ---------------------------
// Animate mole
// ---------------------------
void animateMole() {
  if(!isAnimating) return;
  static unsigned long lastFrameTime = 0;
  unsigned long now = millis();
  if(now - lastFrameTime >= 30){
    int step = 16/4;
    animFrame = (animFrame + 1) % 4;
    if(isPopUp){
      animYOffset += step;
      if(animYOffset >= 16){ animYOffset=16; isAnimating=false;}
    } else {
      animYOffset -= step;
      if(animYOffset <=0){ animYOffset=0; isAnimating=false; activeMole=-1;}
    }
    lastFrameTime = now;
  }
}

// ---------------------------
// Game Over
// ---------------------------
void gameOverScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(14,20); display.println("GAME OVER");
  display.setTextSize(1);
  display.setCursor(44,50); display.print("Score: "); display.print(score);
  display.display();
  playLoseBeep();
  delay(1500);
  score=0; lives=3; activeMole=-1; moleSpawnTime=0; animYOffset=0; isAnimating=false;
  startScreen();
}

// ---------------------------
// Main loop
// ---------------------------
void loop() {
  unsigned long now = millis();

  // ---------------------------
  // Read quantum mole index from Python
  // ---------------------------
 if (Serial.available() > 0) {
    String line = Serial.readStringUntil('\n');
    int mole = line.toInt();
    if(mole >=0 && mole <=4 && mole != activeMole){
        activeMole = mole;
        moleSpawnTime = now;
        isAnimating = true;
        isPopUp = true;
        animYOffset = 0;
        animFrame = 0;
        playMoleSpawnBeep();

        // ---------------------------
        // Adjust game speed based on score
        // ---------------------------
        // After a mole is hit or missed, adjust the delays
        moleVisibleDuration = max(250, 900 - score * 50); // stays visible shorter
        nextMoleSpawnDelay = max(150, 900 - score * 50);  // spawns faster

    }
}


  // ---------------------------
  // Timeout for mole pop-down
  // ---------------------------
  if(activeMole == -1 && now - moleSpawnTime >= nextMoleSpawnDelay) {
    isAnimating = true;
    isPopUp = false;
    animFrame = 0;
}


  animateMole();
  drawMoles();
  display.display();

  // ---------------------------
  // Check inputs
  // ---------------------------
  int input = checkInputs();
  if(input != -1){
    if(input == activeMole) score++;
    else{
      lives--;
      if(lives <=0) gameOverScreen();
    }
    isAnimating=true;
    isPopUp=false;
    animFrame=0;
    playButtonBeep();
    
    // Send current score to Python
    Serial.print("SCORE:");
    Serial.println(score);

  }
}
