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

#define DeviceID    (2)
#define PIR         (14)
#define DIR_IN      true
#define DIR_OUT     false
#define device_state            "mandevices/deviceID_0002/$state"
#define device_name             "mandevices/deviceID_0002/$name"


WiFiClient espClient;
PubSubClient mqtt(espClient);
TaskHandle_t xMQTTTaskHandle;

const char* SSID = "Vento";
const char* PASS = "12345678";
const char* mqtt_server = "172.20.10.7";
// const char* mqtt_server = "test.mosquitto.org";
const int port = 1883;

uint32_t Period = 0;
uint8_t DeviceState = 0;
bool Direction = false;
bool isMotion = false;
bool isTimerRun = false;
char payload[50] = {0};

uint32_t timePub = 0;
uint32_t timeSec = 0;

bool MQTTConnect();
void callback(char* topic, uint8_t* payload, unsigned int length);
void WiFiConnect(void);

void MQTT_Task(void* parameter);


void setup()
{
  Serial.begin(9600);

  pinMode(PIR, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  mqtt.setServer(mqtt_server, port);
  mqtt.setCallback(callback);
  mqtt.setKeepAlive(120);

  timePub = millis();

  delay(1000);

  xTaskCreatePinnedToCore(
                MQTT_Task,
                "MQTT Task",
                1024 * 3,
                NULL,
                1,
                &xMQTTTaskHandle,
                1);
}

void loop()
{
  if ((digitalRead(PIR) == HIGH) && (isMotion == false))
  {
    Serial.println("Motion started!");
    digitalWrite(LED_BUILTIN, HIGH);
    if (Direction == DIR_OUT)
    {
      Serial.println("Im IN :)");
      Direction = DIR_IN;
      DeviceState = 1;
      memset(payload, '\0', sizeof(payload));
      sprintf(payload, "{\"DeviceID\":%d,\"State\":%d,\"Period\":%i}", DeviceID, DeviceState, Period);
      Serial.println(payload);
      mqtt.publish("mandevices/Human_Present", payload, false);
      Period = 0;
      timePub = millis();
    }
    else
    {
      Serial.println("Im OUT :p");
      Direction = DIR_OUT;
      DeviceState = 0;
      memset(payload, '\0', sizeof(payload));
      sprintf(payload, "{\"DeviceID\":%d,\"State\":%d,\"Period\":%i}", DeviceID, DeviceState, Period);
      Serial.println(payload);
      mqtt.publish("mandevices/Human_Present", payload, false);
      Period = 0;
      timePub = millis();
    }
    isMotion = true;
  }
  else if ((digitalRead(PIR) == LOW) && (isMotion == true))
  {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("Motion ended!");
    isMotion = false;
  }
  
  if ((millis() - timePub) >= 90000)
  {
    Serial.println("Regular message ping");
    mqtt.publish("mandevices","");
    timePub = millis();
  }
  
  if (((millis() - timeSec) >= 1000) && (Direction == DIR_IN))
  {
    Period++;
    Serial.println(Period);
    timeSec = millis();
  }
  else if (Direction == DIR_OUT)
  {
    timeSec = millis();
  }

  if (!mqtt.connected() || (WiFi.status() != WL_CONNECTED))
  {
    vTaskResume(xMQTTTaskHandle);
  }
}

void MQTT_Task(void* parameter)
{
  while(1)
  {
    if (!mqtt.connected() && (WiFi.status() == WL_CONNECTED))
    {
      MQTTConnect();
    }
    else if (WiFi.status() != WL_CONNECTED)
    {
      WiFiConnect();
      MQTTConnect();
    }
    vTaskSuspend(xMQTTTaskHandle);
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





