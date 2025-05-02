const int photocellPin = A0;
const int blueLEDPin  = 11;

const int trigPin = 7;
const int echoPin = 6;

const float maxDist = 200.0;  // cm; adjust based on sensor range

void setup() {
  Serial.begin(9600);

  pinMode(blueLEDPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

float measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); 
  // timeout 30 ms → ~5 m max
  if (duration == 0) return maxDist;  
  return min(duration / 58.0, maxDist);
}

void loop() {
  // Part1: brightness
  int raw = analogRead(photocellPin);
  int bright = map(raw, 0, 1023, 0, 255);

  // Part2: distance → color
  float dist = measureDistance();         
  int colorValue = map(dist, 0, maxDist, 255, 0);  
    // 0 cm → 255; far → 0

  int red   = 255 - colorValue;  // far → red high
  int green = colorValue;        // close → green high

  // Apply to LED
  analogWrite(9,  red);
  analogWrite(10, green);
  analogWrite(blueLEDPin, bright);

  // Debug
  Serial.print("Dist(cm): ");
  Serial.print(dist,1);
  Serial.print("  R:");
  Serial.print(red);
  Serial.print(" G:");
  Serial.print(green);
  Serial.print(" B:");
  Serial.println(bright);

  delay(1000);
}
