#include <WiFi.h> // Библиотека работы с Wi-Fi
#include <PubSubClient.h> //  Библиотеку, реализующая клиента по протоколу MQTT (Pubsubclient)
#include "DHT.h"

const char* ssid = "ssid"; // Имя точки доступа
const char* password = "password"; // Пароль от точки доступа
const char* mqtt_server = "srv2.clusterfly.ru"; // Имя сервера MQTT
#define mqtt_port 9991 // Порт для подключения к серверу MQTT
#define MQTT_USER "user_123456789" // Логин от сервера
#define MQTT_PASSWORD "pass_12345" // Пароль от сервера
#define DHTPIN 33 //here we use pin IO33 of ESP32 to read data
#define DHTTYPE DHT11 //our sensor is DHT11 type

WiFiClient wifiClient;

PubSubClient client(wifiClient); // Создаём экземпляр библиотеки

DHT dht(DHTPIN, DHTTYPE);

//Подсоединяемся к Wi-Fi
void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

// Подключаемся к MQTT-серверу
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "user_123456789_led_";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) {
      Serial.println("connected");
      //Once connected, publish an announcement...
      client.publish("user_123456789/data", "hello world");
      // ... and resubscribe
      client.subscribe("user_123456789/led");
      client.subscribe("user_123456789/relay");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Функция для приема данных
void callback(char* topic, byte *payload, unsigned int length) {
    Serial.println("-------new message from broker-----");
    Serial.print("channel:");
    Serial.println(topic);
    Serial.print("data:");  
    Serial.write(payload, length);

    // Проверяем из какого топика пришли данные
    if(String(topic) == "user_123456789/led")
    {
     switch (*payload)
      {
        case '0': digitalWrite(2, 0); break;
        case '1': digitalWrite(2, 1); break;
      }
    }
    if(String(topic) == "user_123456789/relay")
    {
      switch (*payload)
      {
        case '0': digitalWrite(32, 0); break;
        case '1': digitalWrite(32, 1); break;
      }
    }
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(500);// Set time out for 
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
  pinMode(2, OUTPUT);
  pinMode(DHTPIN, INPUT);
  pinMode(32, OUTPUT);
  Serial.println("DHT11 sensor!");
  //call begin to start sensor
  dht.begin();
}

char temp[3];
char hum[3];

void loop() {
   client.loop();

  //use the functions which are supplied by library.
  int h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  int t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
    }
   
   String temp_str = String(t);
   temp_str.toCharArray(temp, temp_str.length() + 1);
   client.publish("user_123456789/temp", temp);
   String hum_str = String(h);
   hum_str.toCharArray(hum, hum_str.length() + 1);
   client.publish("user_123456789/hum", hum);
   delay(1000);
 }
