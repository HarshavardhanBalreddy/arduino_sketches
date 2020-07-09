/*
   LoRa Duplex communication
   Sends a message every half second, and polls continually
   for new incoming messages. Implements a one-byte addressing scheme,
   with 0xFF as the broadcast address.
   Uses readString() from Stream class to read payload. The Stream class'
   timeout may affect other functuons, like the radio's callback. For an
   created 28 April 2017
   by Tom Igoe
 */
//including libraries for GPS
#include <SPI.h>              // include libraries
#include <TinyGPS++.h>
//including libraries for LORA
#include <SoftwareSerial.h>
#include <LoRa.h>
//basic string library
#include<string.h>
//defining function to get gps data in human readable format
//initialising pins for GPS
// Choose two Arduino pins to use for software serial
const int RXPin = 4;
const int TXPin = 3;

int GPSBaud=9600;
//initialising pins for LoRa
const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;       // LoRa radio reset
const int irqPin = 1;         // change for your board; must be a hardware interrupt pin
const int buttonPin = 2; 
const int redLED = A0;
const int greenLED = A1;
const int blueLED = A3;

//constant character array to be sent using lora
const char gps_data[40]="";
// Create a TinyGPS++ object
TinyGPSPlus gps;

// Create a software serial port called "gpsSerial"
SoftwareSerial gpsSerial(RXPin, TXPin);

//outgoing message initialisation for LoRa
String outgoing;              // outgoing message

byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device
byte destination = 0xFF;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

void setup() {
  Serial.begin(9600);  
  while (!Serial);
  //starting gpd baud
  gpsSerial.begin(GPSBaud);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  pinMode(buttonPin,INPUT_PULLUP);

  Serial.println("Starting lora.....");
  Serial.println("LoRa Duplex");

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin

  if (!LoRa.begin(915E6)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.......");
    while (true);                       // if failed, do nothing
  }

  Serial.println("LoRa init succeeded.....");
  if (gpsSerial.available() > 0){
    if (gps.encode(gpsSerial.read())){
        int i=10;
        while(i>0){
          displayInfo();
          i--;
          delay(1000);
        }}
}}

void loop() {
 digitalWrite(redLED,HIGH);
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
        delay(5000);
        Serial.println("Ready to hit the button");
        digitalWrite(blueLED,HIGH);
        int buttonState=digitalRead(buttonPin);
        int lastButtonState=NULL;
          if(buttonState != lastButtonState){
            if((buttonState == LOW)){
              Serial.print("GPS State:");
              Serial.println(get_gpsdata(gps_data));
              if(get_gpsdata(gps_data)==0){
                  for(int i=0;i<10;i++){
                    if (millis() - lastSendTime > interval) {
                    String message = String(random(100000)+gps_data);   // send a message
                    sendMessage(message);
                    Serial.println("Sending " + message);
                    lastSendTime = millis();            // timestamp the message
                    interval = random(2000) + 1000;    // 2-3 seconds
                    }
                    digitalWrite(greenLED,HIGH);
                    // parse for a packet, and call onReceive with the result:
                    onReceive(LoRa.parsePacket());
                  }
              }
            }
          lastButtonState=buttonState;
          }
        Serial.println(gps_data);   
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
  


void sendMessage(String outgoing){
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void onReceive(int packetSize){
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";

  while (LoRa.available()){
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}


void displayInfo(){
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
    Serial.print(gps.date.month());
    Serial.print("/");
    Serial.print(gps.date.day());
    Serial.print("/");
    Serial.println(gps.date.year());
  }
  else
  {
    Serial.println("Not Available");
  }

  Serial.print("Time: ");
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(":");
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(":");
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(".");
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.println(gps.time.centisecond());
  }
  else
  {
    Serial.println("Not Available");
  }

  Serial.println();
  Serial.println();
  delay(1000);
}



int get_gpsdata(String c){
  Serial.println("Fetching location data");
  if (gps.location.isValid()||gps.date.isValid()||gps.time.isValid())
  {
    (String)c=String(gps.location.lat(),6);
    (String)c=String(gps.location.lng(),6);
    (String)c=String(gps.altitude.meters(),6);
	  c=c+gps.date.value();
    c=c+gps.time.value();
    
    return 0;
  }
  else
  {
    return 1;
  }
  delay(1000);
}
