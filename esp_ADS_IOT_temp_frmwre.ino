/*Dieses Sketch, ist der Prototyp für meinen Sensor für Temperatur / Luftfeuchte / Stromabfrage (Solar)
   und ggf. andere analoge oder Digitale Sensoren, wie Feuchte Sensor für Plfanzen oder Helligkeitssenor.
   Es soll als Firmware für alle ESP8266 Boards im Haus dienen.

   ToDo:
         Wenn SSID und PW noch nicht vorhanden, dann ebenfalls abfragen und in EPROM schreiben

*/


#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <PubSubClient.h>
#include "DHT.h"

#define FORCE_DEEPSLEEP
#define WINDOW_SIZE 128
#define DHTTYPE DHT22 

/*
   Include wird benötigt für ESP WIFI, WIRE, Analog Sensor Extender und MQTT
*/

//Adafruit_ADS1115 ads(0x48);  /* Analog Sensor Extender 16bit Version */
#define   ADC_16BIT_MAX   65536
 

//Be Aware, change HOST and MQTT Name
const char* host = "Arbeitszimmer";
const char* ssid = "";
const char* password = "";
const char* MQTTServer = "192.168.0.31";

const char* mqqtpw = "openhab";
const char* mqqtuser = "openhabian";

//Beaware change also IP

float messtoleranz_volt = 0;

//Wird benötigt um Grad Celcius zu berechnen, ACHTUNG nur wegen ADC
float ads_bit_Voltage;
float lm35_constant;

//Wird benötigt für Voltage abfrage
int16_t results[WINDOW_SIZE] = {0};
int current_result = 0;



DHT dht(5, DHTTYPE);
WiFiClient net;
PubSubClient client(net);

int timeSinceLastRead = 0;


void setup() {

  Serial.begin(115200);   //Setup Serial Com
  Serial.print("Connecting to ");     //Connection MSg
  Serial.println(ssid);
  Serial.println("Booting Sketch...");
  client.setServer(MQTTServer, 1883);

  connect();
  
  
  Serial.printf("ready!");
  dht.begin();
  //ads.setGain(GAIN_TWOTHIRDS);
  //ads.begin();
  delay(2000);
  client.loop();

  Serial.println("Send Data");

  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    //return;
   }
  
  // Compute heat index in Fahrenheit (the default)
  float hic = dht.computeHeatIndex(t, h, false);
  // Compute heat index in Celsius (isFahreheit = false)
  
  Serial.println(t);
  
  client.publish("EBSHome/Arbeitszimmer/Temp", String(t).c_str());
  //client.publish("EBSHome/Arbeitszimmer/VoltageBat", String(getVoltage()).c_str());
  client.publish("EBSHome/Arbeitszimmer/Hum", String(h).c_str());
  client.publish("EBSHome/Arbeitszimmer/HeatIndex", String(hic).c_str());  
  client.publish("EBSHome/Arbeitszimmer/resetReason", String(ESP.getResetReason()).c_str());

  delay(2000);
  
  Serial.println("Going into deep sleep for 20 minutes");
  ESP.deepSleep(1200e6); // 20e6 is 20 microseconds
  
  //ESP.deepSleep(20e6); // 20e6 is 20 microseconds
  
   delay(100);

}

void loop(void) {

}


void connect() {
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password); 
    Serial.println("WiFi failed, retrying.");
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  while (!client.connect(host, mqqtuser, mqqtpw)) {
    Serial.print(".");
  }
  Serial.println("\nconnected!");
}



//Function for ADS and Temp Sensor LM35
/*float gettemp()  {

  //Funktion von https://github.com/agaelema/ADS1115_and_LM35/blob/master/Arduino/ADS1115_LM35_01/ADS1115_LM35_01.ino

  float ads_InputRange = 6.144f;
  ads_bit_Voltage = (ads_InputRange * 2) / (ADC_16BIT_MAX - 1);
  lm35_constant = 10.0f / 1000;



  int16_t ads_ch0 = 0;
  int16_t nano_ch0_0 = 0;           // usando referencia de Vcc (5V)
  int16_t nano_ch0_1 = 0;           // usando referencia interna (1.1V)
  float ads_Voltage_ch0 = 0.0f;
  float ads_Temperature_ch0 = 0.0f;

  ads_ch0 = ads.readADC_SingleEnded(0);
  ads_Voltage_ch0 = ads_ch0 * ads_bit_Voltage;
  ads_Temperature_ch0 = ads_Voltage_ch0 / lm35_constant;

  /* imprime os resultados 
  Serial.print(ads_ch0);    Serial.print("\t\t");   Serial.print(ads_Temperature_ch0, 3);       Serial.print("\t\t");
  Serial.println();
  
  return ads_Temperature_ch0;

}
*/

float getVoltage() {

    //Idee von http://henrysbench.capnfatz.com/henrys-bench/arduino-voltage-measurements/arduino-25v-voltage-sensor-module-user-manual/
  
    float vout,vin;
    float value;
    int val11,val2;
    float R1 = 30000.0; //  
    float R2 = 7500.0; // 
   
    value = analogRead(0);
    //Berechnung kommt Henrys Bench und ist auch nur mit dem Voltage Sensor B25 funktionstüchtig
    //Value des AnalogPins * 3 Volt (spannung esp) 
    vout = (value * 3.0) / 1024.0; // see text
    vin = vout / (R2/(R1+R2)); 
    Serial.print(value);    Serial.print("\t\t");   Serial.print(vin);       Serial.print("\t\t");
    if(vin == -messtoleranz_volt) {
       return 0.00;    
    }
    else  {
      return vin;
    }  
    
}
