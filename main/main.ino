#define DEBUG 0

int highCurrentOut = 5; // D1 to 100k R
int lowCurrentOut = 4;  // D2 to 1M R
int sensingDelay = 10; // ms
int averageWindow = 50;

float voltageHigh = 0;
float voltageLow = 0;
float delta = 0;
float runningAverage = 0;

void setup() {
  pinMode(highCurrentOut, OUTPUT);
  pinMode(lowCurrentOut, OUTPUT);
  pinMode(A0, INPUT);
  Serial.begin(9600);
  digitalWrite(highCurrentOut, LOW);
  digitalWrite(lowCurrentOut, LOW);
}

void loop() {
  // measure high
  digitalWrite(highCurrentOut, HIGH);
  delay(sensingDelay);
  voltageHigh = analogRead(A0) * (5000.0 / 1023.0);
  digitalWrite(highCurrentOut, LOW);
  if (DEBUG) {
    Serial.print("V_high: ");
    Serial.println(voltageHigh);
  }
  delay(sensingDelay);

  // measure low
  digitalWrite(lowCurrentOut, HIGH);
  delay(sensingDelay);
  voltageLow = analogRead(A0) * (5000.0 / 1023.0);
  digitalWrite(lowCurrentOut, LOW);
  if (DEBUG) {
    Serial.print("V_low: ");
    Serial.println(voltageLow);
  }
  delay(sensingDelay);
  
  delta = voltageHigh - voltageLow;

  // smoothing
  runningAverage =  runningAverage + delta - runningAverage / averageWindow;
  
  if (DEBUG) {
    Serial.print("delta: ");
    Serial.print(voltageHigh - voltageLow);
    Serial.println();
  }
  Serial.println(runningAverage / averageWindow);
}
