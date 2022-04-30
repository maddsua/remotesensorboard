//	2022 Daniel Strebkov
//
//	https://gitlab.com/maddsua/

//	Wireless version with HC-12 UART module

//	Line template:
//	TIME;TEMP;HUM;UV;LUX;AIRQ;FLAM;GX;GY;GZ

#include <Wire.h>
#include <dht11.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <SoftwareSerial.h>


#define DHT11_PIN	(4)
#define action_pin	(2)
#define pin_buzz	(3)
#define pin_uv		(A0)
#define pin_lux		(A1)
#define pin_poll	(A2)
#define pin_flam	(A3)

//	calibration constants
const float g_offset = -1.57;
const short airqui_offset = -360;
const short airqui_divider = 1;

const short lpg_offset = -850;
const short lpg_divider = 1;

const short light_offset = 1024;
const short light_divider = 100;

const short uvsense_offset = 0;
const short uvsense_divider = 100;

const short motionsence_timeout = 5;


	//	class inits
	SoftwareSerial Wireless(10, 9);
	dht11 DHT;
	Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
	
//	global vars
short motionsence_delay;
bool motionsence_a, motionsence_b;
bool mov_stable = true;
bool table_mode = true;


void setup() {
	
	//	uart
	Serial.begin(9600);
	Wireless.begin(9600);
	
	//pin modes
	pinMode(pin_buzz, OUTPUT);
	
	//	start ADXL345
	accel.begin();
	accel.setRange(ADXL345_RANGE_16_G);

	//beep soung notification
	tone(pin_buzz, 980);
	delay(100);
	noTone(pin_buzz);
	digitalWrite(pin_buzz, LOW);

}

void loop() {

	//unsigned int device_uptime = millis()/1000;
	float dev_ut_dms = millis()/1000.0;
	
	//analog sensors read
	short uvlevel = (analogRead(pin_uv) + uvsense_offset)/uvsense_divider;
	short brightness = (light_offset - analogRead(pin_lux))/light_divider;
	
	short airqui = (analogRead(pin_poll) + airqui_offset)/airqui_divider;
	short flamable = (analogRead(pin_flam) + lpg_offset)/lpg_divider;

	//vibration
	if(!mov_stable){
		
		motionsence_delay--;
		if(motionsence_delay <= 0){
			mov_stable = true;
		}
	}
	else{
		motionsence_a = digitalRead(action_pin);
	}


	//dht11
	DHT.read(DHT11_PIN);
	short air_hum = (DHT.humidity);
	short air_temp = (DHT.temperature);

	//accelerometer
	sensors_event_t event; 
	accel.getEvent(&event);
	float accelx = (event.acceleration.x);
	float accely = (event.acceleration.y);
	float accelz = (event.acceleration.z);
	
	
	//check and fix "out of range"
	if(flamable < 0){
		flamable = 0;
	}
	else if(flamable > 100){
		flamable = 100;
	}
	
	if(airqui < 0){
		airqui = 0;
	}
	else if(airqui > 100){
		airqui = 100;
	}

	//output
	String output = String(dev_ut_dms) + ";" + String(air_temp) + ";" + String(air_hum) + ";" + String(uvlevel) + ";";
	
		if(brightness > 9){
			output += String(brightness) + "+;";
		}
		else{
			output += String(brightness) + ";";
		}
		
		output += String(airqui) + ";" + String(flamable) + ";";
		
		if(motionsence_a == motionsence_b){
			output += "STC;";
		}
		else{
			output += "MOV;";
		}
		
		output += String(accelx+g_offset) + ";" + String(accely+g_offset) + ";" + String(accelz+g_offset) + "\n";
		
		Serial.print(output);
		Wireless.print(output);

	//	vibration stage 2
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
