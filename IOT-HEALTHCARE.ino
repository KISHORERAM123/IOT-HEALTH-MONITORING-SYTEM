

#include <Wire.h>
#include "MAX30105.h"

#include "heartRate.h"

#include <ESP8266WiFi.h>;
#include <WiFiClient.h>;
#include <ThingSpeak.h>;

MAX30105 particleSensor;

const char* ssid = "Abi's Nord"; //Your Network SSID
const char* password = "abi@123456"; //Your Network Password
WiFiClient client;
unsigned long myChannelNumber = 1951295; //Your Channel Number (Without Brackets)
const char *myWriteAPIKey = "VJ8YEO9Y4PPASTF0"; //Your Write API Key


const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;


int period = 20000;
unsigned long time_now = 0;


void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
  particleSensor.enableDIETEMPRDY();

  WiFi.begin(ssid, password); //connect to wifi network
  ThingSpeak.begin(client);
}

void loop()
{
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }
  float temperatureF = particleSensor.readTemperatureF(); 
  
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);
  Serial.print(" temperatureF=");
  Serial.print(temperatureF, 2);
  if (irValue < 50000)
    Serial.print(" No finger?");

  Serial.println();
         if(millis() >= time_now + period){
            time_now += period;
           ThingSpeak.setField(1,irValue);
           ThingSpeak.setField(2,beatsPerMinute);
           ThingSpeak.setField(3,temperatureF);
           ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
         }
           
}
