const int photocellPin = A0;
const int blueLEDPin  = 11;

void setup() {
  Serial.begin(9600);
  pinMode(blueLEDPin, OUTPUT);
}

void loop() {
  int raw = analogRead(photocellPin);             // 0–1023
  int bright = map(raw, 0, 1023, 0, 255);          // 0–255
  analogWrite(blueLEDPin, bright);                // PWM (Timer2)
  Serial.print("Brightness: ");
  Serial.println(bright);
  delay(1000);
}
