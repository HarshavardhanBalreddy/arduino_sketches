//including libraries for GPS
#include <SPI.h>              // include libraries
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
//basic string library
#include<string.h>
//defining function to get gps data in human readable format
String get_gpsdata(String c);
//initialising pins for GPS
// Choose two Arduino pins to use for software serial
int RXPin = 4;
int TXPin = 3;

int GPSBaud=9600;
//constant character array to be sent using lora
char gps_data[75]="";
// Create a TinyGPS++ object
TinyGPSPlus gps;

// Create a software serial port called "gpsSerial"
SoftwareSerial gpsSerial(RXPin, TXPin);

void setup() {
  Serial.begin(9600);                   // initialize serial
  while (!Serial);
  //starting gpd baud
  gpsSerial.begin(GPSBaud);
  
}

void loop() {
  //to do change the light colour to yellow and blink every 2 sec only on a button click
  //until the callibration of gps completes
  //fetching gps data
  if (gpsSerial.available() > 0){
    if (gps.encode(gpsSerial.read())){
        int i=10;
        while(i>0){
          displayInfo();
          i--;
          delay(1000);
        }
    Serial.println("GPS check complete");
    Serial.println(get_gpsdata(gps_data));
    Serial.println(gps_data);
    //to do change the light colour to yellow and blink every 2 sec based on the return value from the fucntion get_gps
    }
    
  }
  // If 5000 milliseconds pass and there are no characters coming in
  // over the software serial port, show a "No GPS detected" error
  else if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println("No GPS detected");
    while(true);
  }
  
}


void displayInfo()
{
  if (gps.location.isValid())
  {
    Serial.print("Latitude: ");
    Serial.println(gps.location.lat(), 6);
    Serial.print("Longitude: ");
    Serial.println(gps.location.lng(), 6);
    Serial.print("Altitude: ");
    Serial.println(gps.altitude.meters());
  }
  else
  {
    Serial.println("Location: Not Available");
  }
  
  Serial.print("Date: ");
  if (gps.date.isValid())
  {
    Serial.print(gps.date.value());
    
  }
  else
  {
    Serial.println("Not Available");
  }

  Serial.print("Time: ");
  if (gps.time.isValid())
  {
    Serial.println(gps.time.value());
  }
  else
  {
    Serial.println("Not Available");
  }

  Serial.println();
  Serial.println();
  delay(1000);
}



String get_gpsdata(String c)
{
  Serial.println("Fetching location data");
  if (gps.location.isValid()||gps.date.isValid()||gps.time.isValid())
  {
    (String)c=String(gps.location.lat(),6);
    (String)c=String(gps.location.lng(),6);
    (String)c=String(gps.altitude.meters(),6);
	  c=c+gps.date.value();
    c=c+gps.time.value();
    
    return c;
  }
  else
  {
    String a="Waiting for location";
    return a;
  }
  delay(1000);
}
