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
volatile uint16_t encoderVal = 128;  // range 0–255

// Ball state
int x  = 64, y  = 32;
int vx = 1,  vy = 1;
const int R      = 4;
const int maxVel = 5;

// ISR to update encoder value
void updateEncoder() {
  int msb     = digitalRead(encoderCLK);
  int lsb     = digitalRead(encoderDT);
  int encoded = (msb << 1) | lsb;
  int sum     = (lastEnc << 2) | encoded;

  // Gray-code decoding
  if (sum == 0b1101 || sum == 0b0100 ||
      sum == 0b0010 || sum == 0b1011) {
    encoderVal++;
  }
  if (sum == 0b1110 || sum == 0b0111 ||
      sum == 0b0001 || sum == 0b1000) {
    encoderVal--;
  }
  encoderVal = constrain(encoderVal, 0, 255);
  lastEnc    = encoded;
}

void setup() {
  Serial.begin(9600);

  // initialize OLED
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
  // map 0–255 → –maxVel…+maxVel
  int speed = map(encoderVal, 0, 255, -maxVel, maxVel);
  vx = speed;
  vy = speed;

  // update ball
  x += vx;  if (x < R || x > SCREEN_WIDTH  - R) vx = -vx;
  y += vy;  if (y < R || y > SCREEN_HEIGHT - R) vy = -vy;

  // render
  disp.clearDisplay();
  disp.fillCircle(x, y, R, SSD1306_WHITE);
  disp.display();

  // debug output
  Serial.print("Encoder Value: ");
  Serial.print(encoderVal);
  Serial.print("   Speed: ");
  Serial.println(speed);

  delay(50);  // ~20 FPS
}
