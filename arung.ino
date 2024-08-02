#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>
#include <elapsedMillis.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>

#define BLYNK_TEMPLATE_ID "TMPL6J2PkRluT"
#define BLYNK_TEMPLATE_NAME "hydronutrimixer"
#define BLYNK_AUTH_TOKEN "145FqDQu66qq4idt_8_7G6iX8L1tlVFP"

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "MarsRaftel";
char pass[] = "marsraftel21";

// Configuration TDS Sensor
#define VREF 5.0
#define SCOUNT 30
int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 25;

// Component pins
#define TdsSensorPin  A0
#define waterLevelPin D7
#define relayDCPin    D5 // IN1
#define relayPoAirPin D4 // IN2
#define relayPoABmPin D3 // IN3
#define relayPoACPin  D2 // IN4
#define ledPin        D5

// Variable Public Blynk
int ppmValue;

BLYNK_WRITE(V0) {
  ppmValue = param.asInt();
  Serial.print("V0: ");
  Serial.println(ppmValue);
}

BLYNK_WRITE(V2) {
  switch (param.asInt()) {
    case 0: { // Item 1
      Serial.println("OFF");
      digitalWrite(relayDCPin, HIGH);
      digitalWrite(relayPoAirPin, HIGH);
      digitalWrite(relayPoABmPin, HIGH);
      digitalWrite(relayPoACPin, HIGH);
      break;
    }
    case 1: { // Item 2
      Serial.println("ON");
      digitalWrite(relayDCPin, LOW);
      break;
    }    
  }
}

LiquidCrystal_I2C lcd(0x27, 20, 4);

elapsedMillis tdsSampleTimer;
elapsedMillis tdsPrintTimer;
elapsedMillis relayControlTimer;

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  
  //lcd.init();
  lcd.begin();
  lcd.backlight();

  // Input output initialization
  pinMode(relayDCPin, OUTPUT);
  pinMode(relayPoAirPin, OUTPUT);
  pinMode(relayPoABmPin, OUTPUT);
  pinMode(relayPoACPin, OUTPUT);
  pinMode(waterLevelPin, INPUT);
  pinMode(ledPin, OUTPUT);

  // Relay Low
  digitalWrite(ledPin, HIGH);
  digitalWrite(relayDCPin, HIGH);
  digitalWrite(relayPoAirPin, HIGH);
  digitalWrite(relayPoABmPin, HIGH);
  digitalWrite(relayPoACPin, HIGH);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
    lcd.setCursor(0, 1);
    lcd.print("   Connecting....   ");
  }
  Serial.println("Connecting to wifi");

  lcd.setCursor(0, 1);
  lcd.print("                    ");
  lcd.print("  Pengaduk Nutrisi  ");
  lcd.setCursor(0, 2);
  lcd.print("   Sistem Irigasi   ");
  delay(2000);
  lcd.clear();
}

void loop() {
  Blynk.run();
  int waterLevelSensorValue = digitalRead(waterLevelPin);
  Blynk.virtualWrite(V4, waterLevelSensorValue);

  if (tdsSampleTimer > 40) {
    tdsSampleTimer = 0;
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT) {
      analogBufferIndex = 0;
    }
  }

  if (tdsPrintTimer > 800) {
    tdsPrintTimer = 0;
    for(copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    }
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0; 
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);    
    float compensationVolatge = averageVoltage / compensationCoefficient;  
    tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; 
    Serial.print(tdsValue, 0);
    Serial.println("ppm");
      
    lcd.setCursor(0, 1);
    lcd.print("PPM Nutrisi: "); 
    lcd.print(tdsValue, 0);

    Blynk.virtualWrite(V3, tdsValue);
  }

  if (relayControlTimer > 3000) {
    relayControlTimer = 0;
    if (tdsValue < ppmValue - 50) {
      digitalWrite(relayPoABmPin, LOW);
      delay(3000);
      digitalWrite(relayPoABmPin, HIGH);
    } else if (tdsValue > ppmValue + 50) {
      digitalWrite(relayPoAirPin, LOW);
      delay(3000);
      digitalWrite(relayPoAirPin, HIGH);
    } else if (tdsValue >= ppmValue + 50 && tdsValue <= ppmValue - 50) {
      digitalWrite(relayDCPin, HIGH);
      digitalWrite(relayPoACPin, LOW);
    }
  }

  lcd.setCursor(0, 0);
  lcd.print("PPM: " + String(tdsValue));

  if (waterLevelSensorValue == HIGH) {
    lcd.setCursor(0, 1);
    lcd.print("   Tangki  Penuh    ");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("   Tangki  Kosong   ");
  }
}

int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++) {
    bTab[i] = bArray[i];
  }
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0) {
    bTemp = bTab[(iFilterLen - 1) / 2];
  } else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

