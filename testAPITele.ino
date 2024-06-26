R3_REGISTRATION_CODE="3571692A-EB69-5189-ACB2-7D80EB803363" sh -c "$(curl -L https://downloads.remote.it/remoteit/install_agent.sh)"
https://www.tomshardware.com/how-to/fix-cannot-currently-show-desktop-error-raspberry-pi

  
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <Wire.h>
#include <ESP32Servo.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <NewPing.h>
#include <HX711.h>

const char* ssid      = "Rumah sakit";
const char* password  = "k0stput1h";

#define BOTtoken      "token"
#define CHAT_ID       "chat id"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay   = 1000;
unsigned long lastTimeBotRan;

// Configuration pin
const int capacitiveProximityPin  = 34;
const int inductiveProximityPin   = 35;
const int servo1Pin               = 17;
const int servo2Pin               = 5;
const int trigPin                 = 13;
const int echoPin                 = 12;

#define CLK           26
#define DOUT          25
#define MAX_DISTANCE  200

HX711 scale;
Servo myServo1;
Servo myServo2;
LiquidCrystal_I2C lcd(0x27, 20, 4);
NewPing sonar(trigPin, echoPin, MAX_DISTANCE);


void setup() {
  Serial.begin(115200);

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");
    client.setTrustAnchors(&cert);
  #endif

  //lcd.init();
  lcd.begin();
  lcd.backlight();

  // Initialize sensor pins
  pinMode(capacitiveProximityPin, INPUT);
  pinMode(inductiveProximityPin, INPUT);

  // Initialize Load Cell
  scale.begin(DOUT, CLK);
  scale.set_scale();
  scale.tare();

  // Initialize Servo
  myServo1.attach(servo1Pin);
  myServo2.attach(servo2Pin);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  #endif

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
    lcd.setCursor(0,1);
    lcd.print("    Connecting...   ");
  }
  
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print(" WiFi: " + String(ssid));
  lcd.setCursor(0,2);
  lcd.print("      ALL CLEAR     ");
  delay(2000);
  lcd.clear();
}


void loop() {
  int capacitiveValue = digitalRead(capacitiveProximityPin);
  int inductiveValue = digitalRead(inductiveProximityPin);
  unsigned int distance = sonar.ping_cm();
  float weight = scale.get_units(10);

  String message = "LAPORAN STATUS\n\n";
  message += "LOADCELL  : " + String(weight) + " g\n";
  message += "PROX CAPA : " + String(capacitiveValue) + "\n";
  message += "PROX INDU : " + String(inductiveValue) + "\n";
  message += "Keterangan : -";
  bot.sendMessage(CHAT_ID, message, "");

  Serial.print("Capacitive: ");
  Serial.print(capacitiveValue);
  Serial.print("\tInductive: ");
  Serial.print(inductiveValue);
  Serial.print("\tDistance: ");
  Serial.print(distance);
  Serial.print(" cm\tWeight: ");
  Serial.print(weight);
  Serial.println(" g");

  // Display values on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Capacitive: " + String(capacitiveValue));

  lcd.setCursor(0, 1);
  lcd.print("Inductive: " + String(inductiveValue));
  
  lcd.setCursor(0, 2);
  lcd.print("Distance: " + String(distance) + " cm");
  
  lcd.setCursor(0, 3);
  lcd.print("Weight: " + String(weight) + " g");

  if (distance < 10) {
    for(int posDegrees1 = 0; posDegrees1 <= 180; posDegrees1++) {
      myServo1.write(posDegrees1);
      Serial.println(posDegrees1);
      delay(20);
    }

    for(int posDegrees1 = 180; posDegrees1 >= 0; posDegrees1--) {
      myServo1.write(posDegrees1);
      Serial.println(posDegrees1);
      delay(20);
    }
  }

  if (capacitiveValue = 1) {
    for(int posDegrees2 = 0; posDegrees2 <= 180; posDegrees2++) {
      myServo2.write(posDegrees2);
      Serial.println(posDegrees2);
      delay(20);
    }

    for(int posDegrees2 = 180; posDegrees2 >= 0; posDegrees2--) {
      myServo2.write(posDegrees2);
      Serial.println(posDegrees2);
      delay(20);
    }
  }

  if (inductiveValue = 0) {
    for(int posDegrees2 = 0; posDegrees2 <= 180; posDegrees2++) {
      myServo2.write(posDegrees2);
      Serial.println(posDegrees2);
      delay(20);
    }

    for(int posDegrees2 = 180; posDegrees2 >= 0; posDegrees2--) {
      myServo2.write(posDegrees2);
      Serial.println(posDegrees2);
      delay(20);
    }
  }

  sendData();
}

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n\n";
      welcome += "SYSTEM MENU. \n";
      welcome += "Welcome everyone \n";
      bot.sendMessage(chat_id, welcome, "");
    }

  }
  
}

void sendData() {
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
