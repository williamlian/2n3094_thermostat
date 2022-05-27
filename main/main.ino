#define DEBUG 0

int highCurrentOut = 5; // D1 to 100k R
int lowCurrentOut = 4;  // D2 to 1M R
int sensingDelay = 1;  // ms
int averageWindow = 100;
int histogramWindow = 200;

float voltageHigh = 0;
float voltageLow = 0;
float delta = 0;
float runningAverage = 0;

int histogramCounter = 0;
int histogramPeak = 0;
float histogram[5000];

void setup() {
  pinMode(highCurrentOut, OUTPUT);
  pinMode(lowCurrentOut, OUTPUT);
  pinMode(A0, INPUT);
  Serial.begin(9600);
  digitalWrite(highCurrentOut, LOW);
  digitalWrite(lowCurrentOut, LOW);
  Serial.println("\nMovingAverage,HistogramPeak");
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
  
  // smoothing
  delta = voltageHigh - voltageLow;
  runningAverage =  runningAverage + delta - runningAverage / averageWindow;

  // Histogram
  histogramCounter++;
  histogram[(int)(runningAverage / averageWindow)]++;
  if(histogramCounter >= histogramWindow) {
    histogramPeak = getHistogramPeak();
    histogramCounter = 0;
    clearHistogram();
  }
  
  if (DEBUG) {
    Serial.print("delta: ");
    Serial.print(voltageHigh - voltageLow);
    Serial.println();
  }
  Serial.print(runningAverage / averageWindow);
  Serial.print(',');
  Serial.println(histogramPeak);
}

int getHistogramPeak() {
  int peakVal = histogram[0];
  int peak = 0;
  for(int i = 1; i < 5000; i++) {
   if (histogram[i] > peakVal) {
    peakVal = histogram[i];
    peak = i;
   }
  }
  return peak;
}

void clearHistogram() {
  for(int i = 0; i < 5000; i++) {
   histogram[i] = 0;
  }
}
