R3_REGISTRATION_CODE="3571692A-EB69-5189-ACB2-7D80EB803363" sh -c "$(curl -L https://downloads.remote.it/remoteit/install_agent.sh)"
https://www.tomshardware.com/how-to/fix-cannot-currently-show-desktop-error-raspberry-pi

  
#include <Wire.h>
#include <OneWire.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

#define BLYNK_TEMPLATE_ID "TMPL6SZ9sUosl"
#define BLYNK_TEMPLATE_NAME "HidroponikIoT"
#define BLYNK_AUTH_TOKEN "TNxaYdePGtw9eWyW3mp7AAbxkotRFK19"

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Rumah sakit";
char pass[] = "k0stput1h";
WidgetLCD lcdBlynk1(V0);
WidgetLCD lcdBlynk2(V3);

const int relayPinIN = D8;
const int relayPinOUT = D7;
const int turbidityPin = D4;
OneWire ds(D5);

// pH sensor
float calibration_value = 25.06;
int phval = 0;
float ph_act;
unsigned long int avgval;
int buffer_arr[10], temp;

// ds sensor
byte i;
byte present = 0;
byte type_s;
byte data[12];
byte addr[8];
float celsius, fahrenheit;

// turbidity sensor
int turbidity;

void dsSensor() {
  if ( !ds.search(addr)) {
    ds.reset_search();
    delay(250);
    return;
  }


  if (OneWire::crc8(addr, 7) != addr[7]) {
    Serial.println("CRC is not valid!");
    return;
  }
  Serial.println();

  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      type_s = 1;
      break;
    case 0x28:
      type_s = 0;
      break;
    case 0x22:
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  delay(1000);
  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {
    data[i] = ds.read();
  }

  // Convert the data to actual temperature
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
  }

  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Blynk.virtualWrite(V4, celsius);
  lcd.setCursor(0, 2);
  lcd.print("Suhu: " + String(celsius));

  if (celsius > 32.00) {
    lcdBlynk2.print(0, 1, "Pompa Nyala");
    lcd.setCursor(0, 3);
    lcd.print("Pompa Nyala");
    digitalWrite(relayPinIN, LOW);
    digitalWrite(relayPinOUT, HIGH);
    delay(2000);
    digitalWrite(relayPinIN, HIGH);
    digitalWrite(relayPinOUT, HIGH);
  } else {
    lcdBlynk2.print(0, 1, "Pompa Mati");
    lcd.setCursor(0, 3);
    lcd.print("Pompa Mati ");
    digitalWrite(relayPinIN, HIGH);
    digitalWrite(relayPinOUT, LOW);
    delay(2000);
    digitalWrite(relayPinIN, HIGH);
    digitalWrite(relayPinOUT, HIGH);
  }

  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
}

void pHsensor() {
  for (int i = 0; i < 10; i++)
  {
    buffer_arr[i] = analogRead(A0);
    delay(30);
  }
  for (int i = 0; i < 9; i++)
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buffer_arr[i] > buffer_arr[j])
      {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }
  avgval = 0;
  for (int i = 2; i < 8; i++)
    avgval += buffer_arr[i];
  float volt = (float)avgval * 5.0 / 1024 / 6;
  ph_act = -5.70 * volt + calibration_value;
  Blynk.virtualWrite(V1, ph_act);
  Serial.print("ph val:");
  Serial.println(ph_act);

  lcd.setCursor(0, 0);
  lcd.print("pH: " + String(ph_act));
}

void tubiditySensor() {
  analogWriteResolution(10);
  int sensorValue = analogRead(turbidityPin);
  //Serial.println(sensorValue);
  turbidity = map(sensorValue, 0, 750, 100, 0);
  delay(100);
  Serial.print("turbidity= ");
  Serial.println(turbidity);

  lcd.setCursor(0, 1);
  lcd.print("turbidity: " + String(turbidity));
  delay(100);
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);

  lcd.begin();
  lcd.backlight();

  pinMode(relayPinIN, OUTPUT);
  pinMode(relayPinOUT, OUTPUT);

  digitalWrite(relayPinIN, HIGH);
  digitalWrite(relayPinOUT, HIGH);

  WiFi.begin(ssid, pass);
  int wifi_ctr = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }
  Serial.println("Wifi Tersambung");

  lcd.setCursor(0, 0);
  lcd.print("SISTEM START:");
  delay(2000);
  lcd.clear();
}

void loop() {
  Blynk.run();

  pHsensor();
  tubiditySensor();
  dsSensor();

  lcdBlynk1.print(0, 0, "pH  : " + String(ph_act));
  lcdBlynk1.print(0, 1, "Turb: " + String(turbidity));

  lcdBlynk2.print(0, 0, "Suhu: " + String(celsius) + "°C");
}
