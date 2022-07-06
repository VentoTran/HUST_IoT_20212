/**
 * @file main.cpp
 * @author Vento (zseefvhu12345@gmail.com)
 * @brief 
 * @version 1.0
 * @date 2022-06-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <Arduino.h>
#include "WiFi.h"
#include "PubSubClient.h"

#define PIR   (14)
#define device_state            "mandevices/deviceID_0002/$state"
#define device_name             "mandevices/deviceID_0002/$name"


WiFiClient espClient;
PubSubClient mqtt(espClient);

const char* SSID = "T.1.1";
const char* PASS = "123123123";
const char* mqtt_server = "test.mosquitto.org";
const int port = 1883;

bool isMotion = false;

uint32_t timePub = 0;

bool MQTTConnect();
void callback(char* topic, uint8_t* payload, unsigned int length);
void WiFiConnect(void);


void setup()
{
  Serial.begin(9600);

  pinMode(PIR, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  delay(1000);

  mqtt.setServer(mqtt_server, port);
  mqtt.setCallback(callback);
  mqtt.setKeepAlive(120);

  WiFiConnect();
  timePub = millis();
}

void loop()
{
  // if (digitalRead(PIR) == HIGH)
  // {
  //   // if (isMotion == false)
  //   // {
  //     digitalWrite(LED_BUILTIN, HIGH);
  //     Serial.println("Motion detected!");
  //     isMotion = true;
  //   // }
  // }
  // else
  // {
  //   // if (isMotion == true)
  //   // {
  //     digitalWrite(LED_BUILTIN, LOW);
  //     Serial.println("Motion ended!");
  //     isMotion = false;
  //   // }
  // }
  if ((millis() - timePub) >= 15000)
  {
    Serial.println("regular message");
    mqtt.publish("mandevices/running", "1", false);
    timePub = millis();
  }
  
  if (!mqtt.connected() && (WiFi.status() == WL_CONNECTED))
  {
    MQTTConnect();
    delay(1000);
  }
}

//================================================================================================

void WiFiConnect(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  delay(100);

  Serial.println("Setup done, attemp to connect to WiFi!");

  WiFi.begin(SSID, PASS);
  int c = 0;
  while ((c < 20) && (WiFi.status() != WL_CONNECTED)) 
  {
    delay(500);
    Serial.print(".");
    c++;
  }
  if (c == 20)
  {
    Serial.println("WiFi Connect FAIL!");
    WiFi.disconnect();
  }
  else 
  {
    Serial.println("WiFi Connect OK!");
  }

}

void callback(char* topic, uint8_t* payload, unsigned int length)
{
  String payload_content;
  Serial.print ("Message arrived [");
  Serial.print (topic);
  Serial.print ("]");
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print ((char)payload[i]);
    payload_content += (char)payload[i];
  }
  Serial.println();
}

bool MQTTConnect()
{
  Serial.print("Attemp to connect MQTT server...");
  String clientID = "Stroke medical - ";
  clientID += String (random(0xffff), HEX);
  if (mqtt.connect(clientID.c_str(), "", "", device_state, 1, true, "lost"))
  {
    Serial.println("Connected");
    mqtt.publish(device_name, "stroke-medical", false);
    return true;
  }
  else 
  {
    Serial.print("failed, rc=");
    Serial.print(mqtt.state());
    Serial.println(" try again in 5 seconds");
    delay(5000);
    return false;
  }
}





