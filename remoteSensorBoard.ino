#include <Wire.h>
#include <dht11.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <SoftwareSerial.h>


#define DHT11_PIN 4
#define action_pin 2
#define pin_buzz 3
#define pin_uv A0
#define pin_lux A1
#define pin_poll A2
#define pin_flam A3


const float g_offset = -1.57;
const int airqui_offset = -360;
const int airqui_divider = 1;

const int lpg_offset = -810;
const int lpg_divider = 1;

const int light_offset = 1024;
const int light_divider = 100;

const int uvsense_offset = 0;
const int uvsense_divider = 100;

const int motionsence_timeout = 5;

SoftwareSerial Wireless(10, 9);

dht11 DHT;

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

unsigned int motionsence_a, motionsence_b, motionsence_delay;
bool mov_stable = true;
bool table_mode = true;

String buffer_master;

void setup() {
	Serial.begin(9600);
	Wireless.begin(9600);
	
	//pin modes
	pinMode(pin_buzz, OUTPUT);

	accel.begin();
	accel.setRange(ADXL345_RANGE_16_G);

	//beep soung notification
	tone(pin_buzz, 980);
	delay(100);
	noTone(pin_buzz);
	digitalWrite(pin_buzz, LOW);

}

void loop() {

	unsigned int device_uptime = millis()/1000;
	
	//analog sensors read
	int uvlevel = (analogRead(pin_uv) + uvsense_offset)/uvsense_divider;
	int brightness = (light_offset - analogRead(pin_lux))/light_divider;
	
	int airqui = (analogRead(pin_poll) + airqui_offset)/airqui_divider;
	int flamable = (analogRead(pin_flam) + lpg_offset)/lpg_divider;

	//vibration
	if(!mov_stable){
		motionsence_delay--;
		if(!motionsence_delay){
			mov_stable = true;
		}
	}
	
	if(mov_stable){	
		motionsence_a = digitalRead(action_pin);
	}
	
  
	//dht11
	DHT.read(DHT11_PIN);
	unsigned int air_hum = (DHT.humidity);
	unsigned int air_temp = (DHT.temperature);

	//accelerometer
	sensors_event_t event; 
	accel.getEvent(&event);
	float accelx = (event.acceleration.x);
	float accely = (event.acceleration.y);
	float accelz = (event.acceleration.z);
	
	
	
	//check and fix "out of range"
	if(flamable<0){
		flamable = 0;
	}
	else if(flamable>100){
		flamable = 100;
	}
	
	if(airqui<0){
		airqui = 0;
	}
	else if(airqui>100){
		airqui = 100;
	}

	//output

		Serial.print(String(device_uptime) + " s | ");
		Serial.print(String(air_temp) + "ºC, ");
		Serial.print(String(air_hum) + "%, ");
		Serial.print(String(uvlevel) + " UV, ");
	
		if(brightness>9){
			Serial.print(String(brightness) + "+ Cd, ");
		}
		else{
			Serial.print(String(brightness) + " Cd, ");
		}
	
		Serial.print(String(airqui) + "% AQI, ");
		Serial.print(String(flamable) + "% LPG | ");
	
		if(motionsence_a == motionsence_b){
			Serial.print("static, ");
		}
		else{
			Serial.print("moving, ");
		}
		
		Serial.println(String(accelx+g_offset) + "X " + String(accely+g_offset) + "Y " + String(accelz+g_offset) + "Z m/s^2 ");


		Wireless.print(String(air_temp) + ", ");
		Wireless.print(String(air_hum) + ", ");
		Wireless.print(String(uvlevel) + ", ");
		Wireless.print(String(brightness) + ", ");
		Wireless.print(String(airqui) + ", ");
		Wireless.print(String(flamable) + ", ");
		
		Wireless.println(String(accelx+g_offset) + ", " + String(accely+g_offset) + ", " + String(accelz+g_offset));

/*
	//receive commands
	if(Serial.available() > 0){
		
		//	read message
		while (Serial.available() > 0) {
			buffer_master += Serial.readStringUntil('\n');
		}
		
		//	ping
		if(buffer_master == "plot"){
			table_mode = false;
		}
		
		//	gate0 open command
		else if(buffer_master == "table"){
			table_mode = true;
		}
		
		//	received commad doesn't match any
		else {
			Serial.println("no such command");
		}
		
		//	clear hub buffer
		buffer_master.remove(0);
	}
*/

	if(mov_stable){
		motionsence_b = digitalRead(action_pin);
		
		if(motionsence_a != motionsence_b){
			mov_stable = false;
			motionsence_delay = motionsence_timeout;
		}
	}


//end of function
}
//end of source file
