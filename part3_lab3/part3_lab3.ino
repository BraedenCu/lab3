#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>

// OLED dimensions
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64

// SPI pin assignments
#define OLED_CS    10
#define OLED_DC     9
#define OLED_RST    8

// create display object on the hardware SPI bus
Adafruit_SSD1306 disp(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &SPI,
  OLED_DC,
  OLED_RST,
  OLED_CS
);

// Encoder pins (DT = Data, CLK = Clock)
const int encoderDT  = 2;
const int encoderCLK = 3;

// State for encoder decoding
volatile int8_t   lastEnc    = 0;
volatile int16_t  encoderVal = 128;  // 0–255, wraps

// Ball state
int x  = 64, y  = 32;
int vx = 0,  vy = 0;
const int R      = 4;
const int maxVel = 10;    // max ticks per frame → max speed

// track previous frame’s encoderVal for delta
int16_t prevEnc = 128;

// ISR to update encoderVal with wrap-around
void updateEncoder() {
  int msb     = digitalRead(encoderCLK);
  int lsb     = digitalRead(encoderDT);
  int encoded = (msb << 1) | lsb;
  int sum     = (lastEnc << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 ||
      sum == 0b0010 || sum == 0b1011) {
    // forward
    encoderVal++;
    if (encoderVal > 255) encoderVal = 0;
  }
  if (sum == 0b1110 || sum == 0b0111 ||
      sum == 0b0001 || sum == 0b1000) {
    // reverse
    encoderVal--;
    if (encoderVal < 0) encoderVal = 255;
  }
  lastEnc = encoded;
}

void setup() {
  Serial.begin(9600);

  // OLED init
  if (!disp.begin(SSD1306_SWITCHCAPVCC)) {
    while (1);
  }
  disp.clearDisplay();

  // encoder inputs + interrupts
  pinMode(encoderDT,  INPUT_PULLUP);
  pinMode(encoderCLK, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderDT),  updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderCLK), updateEncoder, CHANGE);
}

void loop() {
  // compute ticks since last frame
  int16_t delta = encoderVal - prevEnc;
  // handle wrap in delta
  if (delta > 128)  delta -= 256;
  if (delta < -128) delta += 256;
  prevEnc = encoderVal;

  // velocity = direction * magnitude, capped at maxVel
  int speed = constrain(delta, -maxVel, maxVel);
  vx = speed;
  vy = speed;

  // X collision
  int nextX = x + vx;
  if (nextX <= R) {
    x  = R;
    vx = -vx;
  } else if (nextX >= SCREEN_WIDTH - R) {
    x  = SCREEN_WIDTH - R;
    vx = -vx;
  } else {
    x = nextX;
  }

  // Y collision
  int nextY = y + vy;
  if (nextY <= R) {
    y  = R;
    vy = -vy;
  } else if (nextY >= SCREEN_HEIGHT - R) {
    y  = SCREEN_HEIGHT - R;
    vy = -vy;
  } else {
    y = nextY;
  }

  // render
  disp.clearDisplay();
  disp.fillCircle(x, y, R, SSD1306_WHITE);
  disp.display();

  // debug
  Serial.print("Enc: ");
  Serial.print(encoderVal);
  Serial.print("  Δticks: ");
  Serial.print(delta);
  Serial.print("  Speed: ");
  Serial.println(speed);

  delay(50);  // ~20 FPS
}
