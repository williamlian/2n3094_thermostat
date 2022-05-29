#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "secrets.h"

#define DEBUG 0
#define CHART 0
#define HISTOGRAM_SIZE 1024

int highCurrentOut = 5; // D1 to 100k R
int lowCurrentOut = 4;  // D2 to 1M R
int sensingDelay = 1;  // ms
int windowSize = 1000;

short voltageHigh = 0;
short voltageLow = 0;
short delta = 0;

struct Node {
  short value;
  Node* next;
};
Node* dataList = 0;
short histogram[HISTOGRAM_SIZE];
int webSendFrequency = 500;
int webSendCounter = 0;

ESP8266WiFiMulti WiFiMulti;

void setup() {
  pinMode(highCurrentOut, OUTPUT);
  pinMode(lowCurrentOut, OUTPUT);
  pinMode(A0, INPUT);
  Serial.begin(9600);
  digitalWrite(highCurrentOut, LOW);
  digitalWrite(lowCurrentOut, LOW);  
  dataList = createRing();
  Serial.println("\nMovingAverage");

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(SECRET_SSID, SECRET_PASS);
  Serial.println("Connected to WIFI.");
  if ((WiFiMulti.run() != WL_CONNECTED)) {
    Serial.println("WIFI Init failed");
    while(1) {}
  }
}

void loop() {
  // measure high
  digitalWrite(highCurrentOut, HIGH);
  delay(sensingDelay);
  voltageHigh = analogRead(A0);
  digitalWrite(highCurrentOut, LOW);
  if (DEBUG) {
    Serial.print("V_high: ");
    Serial.println(voltageHigh);
  }
  delay(sensingDelay);

  // measure low
  digitalWrite(lowCurrentOut, HIGH);
  delay(sensingDelay);
  voltageLow = analogRead(A0);
  digitalWrite(lowCurrentOut, LOW);
  if (DEBUG) {
    Serial.print("V_low: ");
    Serial.println(voltageLow);
  }
  delay(sensingDelay);
  
  delta = voltageHigh - voltageLow;
  float runningAverage = processHistogram(delta);
  if (DEBUG) {
    Serial.print("delta: ");
    Serial.print(delta);
    Serial.println();
  }
  if (CHART) {
    Serial.println(runningAverage);
  }
}

Node* createRing() {
  Node* head = new Node();
  Node* cur = head;
  for(int i = 1; i < windowSize; i++) {
    Node* next = new Node();
    cur->value = -1;
    cur->next = next;
    cur = next;
  }
  cur->value = -1;
  cur->next = head;
  return head;
}

short processHistogram(short delta) {
  Node* cur = dataList;
  float total = 0;
  int counter = 0;
  
  // reset histogram
  for(int i = 0; i < HISTOGRAM_SIZE; i++) {
    histogram[i] = 0;
  }
  
  // update queue, fill histogram and sum
  while(cur->next != dataList) {
    counter++;
    if(cur->value < 0) {
       cur->value = delta;
       total += delta;
       histogram[delta]++;
       break;
    }
    total += cur->value;
    histogram[cur->value]++;
    cur = cur->next;
  }
  if (cur->next == dataList) {
    cur->value = delta;
    total += delta;
    histogram[delta]++;
    counter++;
    dataList = cur;
  }
  // send histogram
  sendHistogram();
  
  // return answer
  return total / counter;
}

void sendHistogram() {
  webSendCounter++;
  if(webSendCounter >= webSendFrequency) {
    webSendCounter = 0;
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure); 
    client->setFingerprint(SECRET_FINGER_PRINT);
    HTTPClient https;
    https.begin(*client, SECRET_SERVER);
    Serial.println("Preparing Web Data");
    String data = "{\"data\":\"";
    for(int i = 0; i < HISTOGRAM_SIZE; i++) {
      data += String(i);
      data += ",";
      data += String(histogram[i]);
      data += ";"; 
    }
    data += "\"}";

    https.addHeader("Content-Type", "application/json");
    int httpCode = https.POST(data);
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
  }
}
