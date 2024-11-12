#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <BH1750.h>
#include <Wire.h>

// Configurando los leds
#define LED_VERDE 4
#define LED_ROJO 16

// Configuracion del sensor DHT-11
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Configuración del sensor de luz BH1750
BH1750 lightMeter;

// Configuracion del sensor HC-SR04
#define TRIG_PIN 5
#define ECHO_PIN 17

// Configuracion del sensor YL-83
#define YL83_PIN 19  

// Configuracion de la red wifi
const char* ssid = "WIFI_UCC_ACADEMICA";
const char* password = "academica2023*";

// Configuracion del broker MQTT
const char* mqtt_server = "10.26.8.110";
const int mqtt_port = 1883;

// Variables para el control de tiempo
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 10000; 

// Variables globales para los sensores
float temperatura;
float humedad;
float radiacionSolar;
float precipitacion;

// Constantes para la medida del recipiente
const float areaRecipiente = 0.0; // aca va el area del recipiente

// Declaracion del servidor web y del cliente mqtt
AsyncWebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);

// Declaración de funciones
void reconnectMQTT();
void publishSensorData();
void callback(char* topic, byte* payload, unsigned int length);
float getDistance();

void setup() {
  Wire.begin(21, 22);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(YL83_PIN, INPUT);  
  Serial.begin(115200);
  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23)) {
    Serial.println("Sensor de luz BH1750 iniciado correctamente");
  } else {
    Serial.println("Error al iniciar el sensor de luz BH1750");
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando....");
  }
  Serial.println("");
  Serial.print("Conectado a ");
  Serial.println(ssid);
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  server.on("/sensores", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{";
    json += "\"Temperatura\":" + String(temperatura) + ",";
    json += "\"Humedad\":" + String(humedad) + ",";
    json += "\"Radiacion Solar\":" + String(radiacionSolar) + ",";
    json += "\"Precipitacion\":" + String(precipitacion);
    json += "}";
    request->send(200, "application/json", json);
  });

  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
  
  if (millis() - lastUpdateTime > updateInterval) {
    lastUpdateTime = millis(); 
    
    temperatura = dht.readTemperature();
    humedad = dht.readHumidity();
    radiacionSolar = lightMeter.readLightLevel();
    int estaLloviendo = digitalRead(YL83_PIN);
      
    if (isnan(temperatura) || isnan(humedad)) {
      Serial.println("Error al leer el sensor DHT");
      return;
    }
    Serial.println("************************* Actualizacion de sensores ****************************");

    if(estaLloviendo == 0){
      Serial.print("Esta LLoviendo");
      
      float distancia = getDistance();
      float alturaAgua = 0.0 - distancia;// altura del recipiente

      if (alturaAgua > 0){
        precipitacion = areaRecipiente * (alturaAgua / 100);  // Convertir cm a metros
      }else{
        Serial.println("No hay agua acumulada.");
      }
      
    }else{
      Serial.println("No está lloviendo.");
    }
    
    
    Serial.println("Temperatura: " + String(temperatura));
    Serial.println("Humedad: " + String(humedad));
    Serial.println("RadiacionSolar: "+ String(radiacionSolar));
    Serial.println("Precipitacion: " + String(precipitacion)); 
    publishSensorData();
  }
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Intentando conectar al broker MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado");
      client.subscribe("tu/tema/recibir");
    } else {
      Serial.print("Fallo, rc=");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 5 segundos...");
      delay(5000);
    }
  }
}

void publishSensorData() {
  StaticJsonDocument<200> doc;
  doc["temperatura"] = temperatura;
  doc["humedad"] = humedad;
  doc["radiacionSolar"] = radiacionSolar;
  doc["precipitacion"] = precipitacion; 
  String jsonString;
  serializeJson(doc, jsonString);
  client.publish("sensores", jsonString.c_str());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido en el tema: ");
  Serial.println(topic);
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println("Mensaje: " + message);
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2; 
  return distance;
}
