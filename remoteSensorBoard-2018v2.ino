// 2018 Daniel Strebkov
//
//	https://gitlab.com/maddsua/

// This version uses SD card to store data. No realtime transmissions

#include <dht11.h>
dht11 DHT;
#define DHT11_PIN 2

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

#include <SPI.h>
#include <SD.h>
int PIN_CHIP_SELECT = 5;

int pin_buzz = 10;
int pin_air = (A0);
int pin_smoke = (A1);
int echoPin = 4;
int trigPin = 3;

String startmsg = ">> [black box initialized]\nTIME, TEMP, HUM, ACL, SL, aX, aY, aZ, SNR";
String dwr = ">> [DATA WRITE ERROR!]";

void setup() {
  Serial.begin(9600);

  //pin modes
  pinMode(pin_buzz, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  accel.begin();
  accel.setRange(ADXL345_RANGE_16_G);

  //beep soung notification
  tone(pin_buzz, 980);
  delay(150);
  noTone(pin_buzz);
  digitalWrite(pin_buzz, LOW);
  Serial.println(startmsg);
  //writing data legend to sd card
  SD.begin(PIN_CHIP_SELECT);
  File dataFile = SD.open("flog.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println(startmsg);
    dataFile.close();
  }
  else {
    Serial.println(dwr);
  }

}

void loop() {

  int worktime = millis()/1000;
  
  //dht11
  DHT.read(DHT11_PIN);
  int hum = (DHT.humidity);
  int temp = (DHT.temperature);
  
  //analog sensors read
  int airclean = analogRead(pin_air)/10;
  int smokelevel = analogRead(pin_smoke)/10;

  //accelerometer
  sensors_event_t event; 
  accel.getEvent(&event);
  float accelx = (event.acceleration.x);
  float accely = (event.acceleration.y);
  float accelz = (event.acceleration.z);

  //sonar distanse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  int dist = pulseIn(echoPin, HIGH)/58;
  
  //creating string log
  String writetelemetry = String(worktime)+", "+String(temp)+", "+String(hum)+", "+String(airclean)+", "+String(smokelevel)+", "+String(accelx)+", "+String(accely)+", "+String(accelz)+", "+String(dist);

  
  Serial.println(writetelemetry);

  //write data to sd card
  File dataFile = SD.open("flog.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println(writetelemetry);
    dataFile.close();
    digitalWrite(pin_buzz, LOW);
  }
  else {
    Serial.println(dwr);
    tone(pin_buzz, 900);
    delay(150);
    noTone(pin_buzz);
    delay(150);
    tone(pin_buzz, 900);
    delay(150);
    noTone(pin_buzz);
    digitalWrite(pin_buzz, LOW);
  }
  
    if (dist > 30) {
    digitalWrite(pin_buzz, LOW);
  }
  else {
    Serial.println(dwr);
    tone(pin_buzz, 1200);
    delay(150);
    noTone(pin_buzz);
    delay(150);
    tone(pin_buzz, 1200);
    delay(150);
    noTone(pin_buzz);
    delay(150);
    tone(pin_buzz, 1200);
    delay(150);
    noTone(pin_buzz);
    delay(150);
    digitalWrite(pin_buzz, LOW);
  }


  
  delay(500);


//-------------
}
