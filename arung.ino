#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>

#define BLYNK_TEMPLATE_ID "TMPL6J2PkRluT"
#define BLYNK_TEMPLATE_NAME "hydronutrimixer"
#define BLYNK_AUTH_TOKEN "145FqDQu66qq4idt_8_7G6iX8L1tlVFP"

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "UGMURO-INET";
char pass[] = "Gepuk15000";

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
#define BTN1          D6
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

BLYNK_WRITE(V1) {
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

LiquidCrystal_I2C lcd(0x26, 20, 4);

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
  pinMode(BTN1, OUTPUT);

  digitalWrite(ledPin, HIGH);
  
  // Relay Low
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
  // Blynk.run();
  int waterLevelSensorValue = digitalRead(waterLevelPin);
  Blynk.virtualWrite(V3, waterLevelSensorValue);

  SensTDS();

  lcd.setCursor(0, 0);
  lcd.print("PPM: " + String(tdsValue));

  if (waterLevelSensorValue == HIGH) {
    lcd.setCursor(0, 1);
    lcd.print("   Tangki  Penuh    ");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("   Tangki  Kosong   ");
  }

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

void SensTDS() {
  static unsigned long analogSampleTimepoint = millis();
  if(millis()-analogSampleTimepoint > 40U) {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT)
      analogBufferIndex = 0;
  }
  static unsigned long printTimepoint = millis();
  if(millis()-printTimepoint > 800U) {
    printTimepoint = millis();
    for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
      analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
    averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; 
    float compensationCoefficient=1.0+0.02*(temperature-25.0);    
    float compensationVolatge=averageVoltage/compensationCoefficient;  
    tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; 
    Serial.print(tdsValue,0);
    Serial.println("ppm");
      
    lcd.setCursor(0, 1);
    lcd.print("PPM Nutrisi: "); 
    lcd.print(tdsValue,0);

    Blynk.virtualWrite(V2, tdsValue);    
  }
}

int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
    bTab[i] = bArray[i];
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
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}

/*void componentCheck() {
  lcd.setCursor(3, 0);
  lcd.print("Component State");
  
  int flowSensorValue = digitalRead(flowSensorPin);
  int waterLevelSensorValue = digitalRead(waterLevelSensorPin);
  
  if (flowSensorValue == HIGH) {
    lcd.setCursor(0, 1);
    lcd.print("Flow Sensor: OK  ");
    digitalWrite(ledPin, HIGH);
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Flow Sensor: LOW");
    digitalWrite(ledPin, LOW);
  }
  
  if (waterLevelSensorValue == HIGH) {
    lcd.setCursor(0, 2);
    lcd.print("Water Level: OK  ");
    digitalWrite(ledPin, HIGH);
  } else {
    lcd1.setCursor(0, 2);
    lcd1.print("Water Level: LOW ");
    digitalWrite(ledPin, LOW);
  }
}

void openSolenoidValve() {
  digitalWrite(solenoidValvePin, HIGH);
  lcd2.clear();
  lcd2.print("Solenoid Valve");
  lcd2.setCursor(0, 1);
  lcd2.print("Terbuka");
  delay(1000);
}

void closeSolenoidValve() {
  digitalWrite(solenoidValvePin, LOW);
  lcd2.clear();
  lcd2.print("Solenoid Valve");
  lcd2.setCursor(0, 1);
  lcd2.print("Tertutup");
  delay(1000);
}

void turnOnAgitator() {
  digitalWrite(relayV1Pin, LOW);
  digitalWrite(relayV2Pin, LOW);
  digitalWrite(relayV3Pin, LOW);
  digitalWrite(relayMPin, LOW);
  lcd2.clear();
  lcd2.print("Pengaduk Aktif");
  delay(3000);
}

void turnOffAgitator() {
  digitalWrite(relayV1Pin, HIGH);
  digitalWrite(relayV2Pin, HIGH);
  digitalWrite(relayV3Pin, HIGH);
  digitalWrite(relayMPin, HIGH);
  lcd2.clear();
  lcd2.print("Pengaduk Mati");
  delay(3000);
}

void showNutrientValues() {
  lcd2.clear();
  lcd2.print("Nutrisi 1 : ");
  lcd2.print(nutrisi1);
  lcd2.setCursor(0, 1);
  lcd2.print("Nutrisi 2 : ");
  lcd2.print(nutrisi2);
  lcd2.setCursor(0, 2);
  lcd2.print("Nutrisi 3 : ");
  lcd2.print(nutrisi3);
  lcd2.setCursor(0, 3);
  lcd2.print("Air       : ");
  lcd2.print(air);
  delay(5000);
}

void resetNutrientValues() {
  nutrisi1 = 0;
  nutrisi2 = 0;
  nutrisi3 = 0;
  air = 0;
  lcd2.clear();
  lcd2.print("Nilai Nutrisi");
  lcd2.setCursor(0, 1);
  lcd2.print("dan air direset");
  delay(5000);
}*/


/*LOGIC SKRIPSI PENGADUK NUTRISI
1. Masukkan PPM yang diinginkan melalui Blynk dan PPM yang ditargetkan akan terdisplay
  di LCD
2. Info pembacaan sensor terdisplay di Blynk dan LCD
3. System ON (pada blynk) akan memulai sistem dimana
    - motor DC (in1 relay) aktif mengaduk
    - jika ppm kurang 50 point dari target (<50), in3 relay (double pump AB-MIX 
      aktif 3 detik)
    - jika ppm lebih 50 point dari target (>50), in2 relay (pompa 5v air aktif 3 detik)
    - jika ppm target terpenuhi, motor DC pengaduk mati, pompa AC distribusi
    (in4 relay) aktif
4. SYSTEM OFF (pada blynk) mematikan seluruh komponen output relay (in1, in2, in3,
  in4 mati semua)
*/
